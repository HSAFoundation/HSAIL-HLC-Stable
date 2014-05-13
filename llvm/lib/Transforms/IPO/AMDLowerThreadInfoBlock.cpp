//===- AMDLowerThreadInfoBlock.cpp - lower TIB usage ----------------------===//
//
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//
//
// This pass replaces all calls to get_thread_info_block() with an argument that
// passed down from the kernel's stub function to the calling function.
// Only the WorkItem functions call get_thread_info_block().
//
// The function get_thread_info_block() retrieves a pointer to the structure,
// of type clk_thread_info_block_t*, that defines the WorkItem info.
//
// This pass also add new code, in the beginning of the kernel, to increment
// the thread_info_block->local_id (by calling inc_local_id()). This ensures
// the next call to the kernel with the same thread_info_block will process the
// next WorkItem (This is required for WorkGroup Level Execution).
// The call to inc_local_id() is emitted in a new kernel delegate function,
// called from the kernel's stub.
//
//
// Example of the transformation:
// ------------------ OpenCL C Program ------------------
//
// __kernel foo1(__global size_t *x) {
//   size_t id = get_group_id(0);
//   x[id] = 0;
// }
//
// __kernel foo2(__global size_t *x) {
//   size_t id = get_local_id(0);
//   x[id] = 0;
// }
//
// ------------------------------------------------------
// ---------------- LLVM Module - Before ----------------
//
// void __OpenCL_foo1_kernel(__global size_t *x) {
//   size_t id = get_group_id(0);
//   x[id] = 0;
// }
//
// void __OpenCL_foo1_stub(void *stubArgs) {
//   __global size_t *x = *((__global size_t **)stubArgs);
//   __OpenCL_foo1_kernel(x);
// }
//
//
// void __OpenCL_foo2_kernel(__global size_t *x) {
//   size_t id = get_local_id(0);
//   x[id] = 0;
// }
//
// void __OpenCL_foo2_stub(void *stubArgs) {
//   __global size_t *x = *((__global size_t **)stubArgs);
//   __OpenCL_foo2_kernel(x);
// }
//
// ------------------------------------------------------
// ---------------- LLVM Module - After -----------------
//
// void __OpenCL_foo1_kernel(clk_thread_info_block_t *tib, __global size_t *x) {
//   size_t id = get_group_id(tib, 0);
//   x[id] = 0;
// }
// 
// void delegate@__OpenCL_foo1_stub(clk_thread_info_block_t *tib,
//                                  __global size_t *x) {
//   __OpenCL_foo1_kernel(tib, x);
// }
//
// void __OpenCL_foo1_stub(void *stubArgs) {
//   __global size_t *x = *((__global size_t **)stubArgs);
//   clk_thread_info_block_t *tib = *((clk_thread_info_block_t **)
//                           ((byte*)stubArgs + sizeof(__global size_t *)));
//   delegate@__OpenCL_foo1_stub(tib, x);
// }
//
//
// void __OpenCL_foo2_kernel(clk_thread_info_block_t *tib,
//                           size_t *localIDs,
//                           __global size_t *x) {
//   size_t id = get_local_id(localIDs, 0);
//   x[id] = 0;
// }
//
// void delegate@__OpenCL_foo2_stub(clk_thread_info_block_t *tib,
//                                  __global size_t *x) {
//   size_t localIDs[4];
//   localIDs[0] = get_tib_local_id(tib, 0);
//   localIDs[1] = get_tib_local_id(tib, 1);
//   localIDs[2] = get_tib_local_id(tib, 2);
//   localIDs[3] = get_tib_local_id(tib, 3);
//   inc_local_id(tib);
//   __OpenCL_foo2_kernel(tib, localIDs, x);
// }
//
// void __OpenCL_foo2_stub(void *stubArgs) {
//   __global size_t *x = *((__global size_t **)stubArgs);
//   clk_thread_info_block_t *tib = *((clk_thread_info_block_t **)
//                           ((byte*)stubArgs + sizeof(__global size_t *)));
//   delegate@__OpenCL_foo2_stub(tib, x);
// }
//
// ------------------------------------------------------
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "amdlowertib"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Instructions.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/DataLayout.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallBitVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/AMDArgumentUtils.h"
#include "llvm/Transforms/Utils/AMDWorkGroupUtils.h"
#include "llvm/Analysis/AMDOpenCLSymbols.h"
using namespace llvm;

namespace {

# define SARG_LOCALIDS_NAME  DEBUG_RESERVE_NAME("localids")

# define FUNC_GET_TIB_NAME        DEBUG_RESERVE_NAME("get_thread_info_block")
# define FUNC_INC_LOCALID_NAME    DEBUG_RESERVE_NAME("inc_local_id")

  //===--------------------------------------------------------------------===//
  // AMDLowerThreadInfoBlock pass implementation.
  class AMDLowerThreadInfoBlock : public ModulePass {
  private:
    enum WorkItemFunctionType {
      WIF_GetGlobalSize,
      WIF_GetGlobalId,
      WIF_GetGlobalLinearId,
      WIF_GetLocalSize,
      WIF_GetEnqueuedLocalSize,
      WIF_GetLocalId,
      WIF_GetLocalLinearId,
      WIF_GetNumGroups,
      WIF_GetGroupId,
      WIF_GetGlobalOffset,
      WIF_GetWorkDim,
      WIF_GetLocalMemAddr,
      WIF_GetTableBase,
      WIF_GetScratch,
      WIF_Printf,
      WIF_Barrier,
      WIF_WorkGroupBarrier,

      NUM_WIFTypes
    };

    typedef DenseMap<Function *, Function *> FuncsMap;
    typedef DenseMap<GlobalAlias *, GlobalAlias *> AliasMap;
    typedef DenseMap<User *, SmallBitVector> UsersMap;

    UsersMap Users;
    UsersMap Stubs;
    
    union {
      Function       *WorkItemFunctions[NUM_WIFTypes];
      SmallBitVector *WorkItemUseBits[NUM_WIFTypes];
    };

    Function *FGetTIB;
    Function *FGetTIBLocalId;
    Function *FInclocalId;
    PointerType *TIBPtrTy;
    PointerType *localIdsPtrTy;

    OpenCLSymbols *OCLS;
    DataLayout *DL;

  public:
    static char ID; // Pass identification, replacement for typeid
    AMDLowerThreadInfoBlock() : ModulePass(ID),
                               FGetTIB(0), FGetTIBLocalId(0), FInclocalId(0),
                               TIBPtrTy(0), localIdsPtrTy(0), OCLS(0), DL(0)
    {
      initializeAMDLowerThreadInfoBlockPass(*PassRegistry::getPassRegistry());
      memset(WorkItemUseBits, 0, sizeof(WorkItemUseBits));
    }

    virtual bool runOnModule(Module& M);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<OpenCLSymbols>();
      AU.addRequired<DataLayout>();
      AU.addPreserved<DataLayout>();
    }

  private:
    void findFunctionUsers(Function &F, unsigned &FlagIdx);

    bool initializeWorkItemFunctions(Module &M);
    void initializeWorkItemFunction(unsigned &FlagIdx,
                                    WorkItemFunctionType WIFType,
                                    StringRef FuncName, Module &M);

    bool isDependentOnWorkItemFunction(
             const SmallBitVector &UseBits, WorkItemFunctionType WIFType) const;

    unsigned findLocalIdOffsetInTIB() const;

    Function *createNewGetTIBFunction() const;
    Function *createNewGetTIBLocalIdFunction() const;
    Function *createIncLocalIdFunction() const;
    Function *createFunctionWithTIB(Function *F, bool TakeBlocks = true) const;
    Function *createFunctionWithSpecialArguments(Function *F,
                                                 bool TakeBlocks = true) const;

    Value *emitStoreLocalIds(Function *F, Function *FStub) const;

    LoadInst *emitLoadTIBInStub(Function *FStub);

    void createKernelDelegate(Function *FStub, LoadInst *LITIB, FuncsMap &ModFuncs);
    unsigned getNextOffsetInStubArgs(Function *F, Value *StubArgsAddr) const;
  };
} // end anonymous namespace

char AMDLowerThreadInfoBlock::ID = 0;
INITIALIZE_PASS(AMDLowerThreadInfoBlock, "amdlowertib",
                "AMD Lower TIB", false, false)

Pass *llvm::createAMDLowerThreadInfoBlockPass() {
  return new AMDLowerThreadInfoBlock();
}

// returns the number of parameters of a function
static inline size_t getNumParams(const Function& F)
{
  Type* FuncType = F.getType()->getElementType();
  FunctionType* FuncFType = dyn_cast<FunctionType>(FuncType);
  assert(FuncFType && "Aliasee not function type");
  return FuncFType->getNumParams();
}

bool AMDLowerThreadInfoBlock::runOnModule(Module& M)
{
  OCLS = &getAnalysis<OpenCLSymbols>();
  DL = &getAnalysis<DataLayout>();

  if (!initializeWorkItemFunctions(M))
    return false;

  // Run through all of the WorkItem functions users and add the required
  // arguments (for lowering get_thread_info_block()).
  //
  FuncsMap ModifiedFuncsMap;
  for (UsersMap::iterator I = Users.begin(), E = Users.end(); I != E; ++I) {
    User *U = I->first;
    if (Function *F = dyn_cast<Function>(U))
      ModifiedFuncsMap[F] = (localIdsPtrTy ?
        createFunctionWithSpecialArguments(F) : createFunctionWithTIB(F));
  }

   SetVector<GlobalAlias *> ModifiedAliases;

   for (Module::alias_iterator I = M.alias_begin(), E = M.alias_end();
        I != E; ++I) {
     const Function *Fun = dyn_cast<Function>(I->resolveAliasedGlobal(false));
     if (Fun && ModifiedFuncsMap.count(const_cast<Function*>(Fun)))
       ModifiedAliases.insert(I);
   }

   AliasMap ModifiedAliasesMap;

   for (unsigned i = 0; i < ModifiedAliases.size(); i++) {
     GlobalAlias *Alias = ModifiedAliases[i];
     const Function *Fun = dyn_cast<Function>(Alias->resolveAliasedGlobal(false));
     Function *NewFunction = ModifiedFuncsMap[const_cast<Function*>(Fun)];
     Constant *NewConstant = NewFunction;
     // If aliasee was a bitcast, add the new args to the alias type's arg list,
     // create a new bitcast that cast the new function to the new alias type
     const Constant* Aliasee = Alias->getAliasee();
     assert(Aliasee && "Empty aliasee"); 
     if (!isa<GlobalValue>(Aliasee)
       && cast<ConstantExpr>(Aliasee)->getOpcode() == Instruction::BitCast) {
       // Calculate how many new args were added to the new function
       size_t NumNewArgs = getNumParams(*NewFunction) - getNumParams(*Fun);

       // add the new args to the alias type's arg list,
       Type* OldAliasType = Alias->getType()->getElementType();
       FunctionType* OldAliasFType = dyn_cast<FunctionType>(OldAliasType);
       assert(OldAliasFType && "Alias not function type");
       FunctionType* NewFuncType
         = dyn_cast<FunctionType>(NewFunction->getType()->getElementType());
       SmallVector<Type*, 2> NewArgs;
       for (size_t j = 0; j < NumNewArgs; ++j) {
         NewArgs.push_back(NewFuncType->getParamType(j));
       }
       FunctionType* NewAliasFType
         = GetFunctionTypeWithNewArguments(OldAliasFType, NewArgs);
       Type* NewAliasPType = PointerType::getUnqual(NewAliasFType);

       // create a new bitcast that cast the new function to the new alias type
       NewConstant = ConstantExpr::getBitCast(NewFunction, NewAliasPType);
     }
     GlobalAlias *NewAlias = new GlobalAlias(NewConstant->getType(),
                                             Alias->getLinkage(),
                                             Alias->getName(),
                                             NewConstant,
                                             &M);
     ModifiedAliasesMap[Alias] = NewAlias;
   }

   for (AliasMap::iterator I = ModifiedAliasesMap.begin(),
        E = ModifiedAliasesMap.end(); I != E; ++I) {
     GlobalAlias *AOld = I->first, *ANew = I->second;
     TransferAliasUses(AOld, ANew);
     AOld->eraseFromParent();
   }

  if (FGetTIBLocalId) {
    // Old uses of get_local_id() should use the local local IDs stored on the
    // stack, instead of the ones in the thread_info_block, as it has already
    // been incremented by now, and therefore - invalid for the current WorkItem.
    TransferFunctionUses(FGetTIBLocalId, createNewGetTIBLocalIdFunction());
  }

  // Lower get_thread_info_block().
  //
  TransferFunctionUses(FGetTIB, createNewGetTIBFunction());
  FGetTIB->eraseFromParent();
  FGetTIB = 0;

  // Create all kernels' delegates.
  //
  for (UsersMap::iterator I = Stubs.begin(), E = Stubs.end(); I != E; ++I) {
    Function *FStub = cast<Function>(I->first);
    LoadInst *LI = emitLoadTIBInStub(FStub);
    createKernelDelegate(FStub, LI, ModifiedFuncsMap);
  }

  // Finalize the transformation of the modified functions.
  //
  for (FuncsMap::iterator I = ModifiedFuncsMap.begin(), E = ModifiedFuncsMap.end();
       I != E; ++I) {
    Function *FOld = I->first, *FNew = I->second;
    TransferFunctionUses(FOld, FNew);
    FOld->eraseFromParent();
  }

  return true;
}

// createNewGetTIBFunction - Returns a newly created function, which only
// receive the thread_info_block as an argument and retrieves it.
Function *AMDLowerThreadInfoBlock::createNewGetTIBFunction() const
{
  Function *NF = createFunctionWithTIB(FGetTIB, false);
  LLVMContext &C = NF->getContext();
  BasicBlock *BB = BasicBlock::Create(C, "", NF);
  ReturnInst::Create(C, NF->arg_begin(), BB);
  return NF;
}

// createNewGetTIBLocalIdFunction - Returns a newly created function that
// emulate the functionality of the original get_local_id(), but extracts the
// information from the local IDs array received as an argument, instead of
// the original thread_info_block.
Function *AMDLowerThreadInfoBlock::createNewGetTIBLocalIdFunction() const
{
  Function *NF = createFunctionWithSpecialArguments(FGetTIBLocalId, false);
  LLVMContext &C = NF->getContext();

  AttrBuilder B;
  B.addAttribute(Attributes::NoInline);
  NF->removeFnAttr(Attributes::get(C, B));
  NF->addFnAttr(Attributes::AlwaysInline);

  BasicBlock *BB = BasicBlock::Create(C, "", NF);

  Function::arg_iterator Args = NF->arg_begin();
  Value *Idx = llvm::next(Args, (unsigned)NUM_SPECIAL_ARGS_FUNC + 1);
  Instruction *I;
  I = GetElementPtrInst::Create(llvm::next(Args, (unsigned)SARG_LOCALIDS_IDX),
                                Idx, "", BB);
  I = new LoadInst(I, 0, BB);//TODO: fix alignment?
  ReturnInst::Create(C, I, BB);
  return NF;
}

// createFunctionWithTIB - Returns a copy of F but with a pointer to the
// thread_info_block as a new argument at the beginning. The basic blocks of F
// may be transferred to the the new function (TakeBlocks = true), or the new
// function may be empty (TakeBlocks = false).
Function *AMDLowerThreadInfoBlock::createFunctionWithTIB(Function *F,
                                                         bool TakeBlocks) const
{
  NewArgInfo TIBArg = { TIBPtrTy, SARG_TIB_NAME };
  return CreateFunctionWithNewArguments(F, TIBArg, TakeBlocks);
}

// createFunctionWithSpecialArguments - Returns a copy of F but with a pointer
// to the thread_info_block and the local IDs array as new arguments at the
// beginning. The basic blocks of F may be transferred to the the new function
// (TakeBlocks = true), or the new function may be empty (TakeBlocks = false).
Function *AMDLowerThreadInfoBlock::createFunctionWithSpecialArguments(
                                             Function *F, bool TakeBlocks) const
{
  NewArgInfo NArgs[] = {
    { TIBPtrTy, SARG_TIB_NAME },
    { localIdsPtrTy, SARG_LOCALIDS_NAME }
  };
  return CreateFunctionWithNewArguments(F, NArgs, TakeBlocks);
}

// getCallToKernelInStub - Returns the call to FStub's kernel.
static CallInst *getCallToKernelInStub(Function &FStub)
{
  // The call to kernel in stub is always the last call in the function.
  //
  BasicBlock::InstListType &InstList = FStub.getEntryBlock().getInstList();
  for (BasicBlock::InstListType::reverse_iterator I = InstList.rbegin(),
       E = InstList.rend(); I != E; ++I) {
    if (CallInst *CI = dyn_cast<CallInst>(&*I))
      return CI;
  }
  return 0;
}

// createKernelDelegate - Creates a new function that delegates the call to
// kernel. This function includes 
//
// Motivation for creating a delegate:
//   In case kernel A is calling kernel B in a loop. B calls get_local_id().
//   If we added the call to inc_local_id() in the beginning of B, we would
//   increment the local ID in a loop, but only need it once! Therefore, we
//   introduce the delegate function. B's delegate increments the local ID once,
//   as well as A's delegate. This way, we may call kernel B as many times as we
//   want (as we call the kernel itself and not the delegate).
//   We may emit the call to inc_local_id() in the stub, but this way we will
//   not be able to wrap the incrementation code into the WorkGroup loop of the
//   later WorkGroup Level Execution optimization pass.
void AMDLowerThreadInfoBlock::createKernelDelegate(Function *FStub,
                                                   LoadInst *LITIB,
                                                   FuncsMap &ModFuncs)
{
  CallInst *CIKernel = getCallToKernelInStub(*FStub);
  assert(CIKernel && "Stub function has no call to kernel");

  Function *FKernel = ModFuncs[CIKernel->getCalledFunction()];
  assert(FKernel && "Kernel has not been modified");

  NewArgInfo NArgs = { TIBPtrTy, SARG_TIB_NAME };

  Function *FDelegate =
    CreateFunctionWithNewArguments(CIKernel->getCalledFunction(), NArgs, false);
  FDelegate->setName(GetKernelDelegateName(FStub));

  LLVMContext &C = FStub->getContext();

  // We want to manipulate this function without contaminating it with the
  // stub's code, by preventing it from inlining it into the stub.
  AttrBuilder B;
  B.addAttribute(Attributes::AlwaysInline);
  FDelegate->removeFnAttr(Attributes::get(C, B));
  FDelegate->addFnAttr(Attributes::NoInline);
  FDelegate->setLinkage(GlobalValue::InternalLinkage);

  // Replace the call to kernel, in stub, with the delegate
  SmallVector<Value*, 16> Args;
  Args.reserve(1 + CIKernel->getNumArgOperands());
  Args.push_back(LITIB);
  CallSite CSKernel(CIKernel);
  AddArgumentsToCallSite(CSKernel, Args, FDelegate);

  ReturnInst *RI = ReturnInst::Create(C, BasicBlock::Create(C, "", FDelegate));

  // Store LocalIds
  //
  Args.clear();
  Args.reserve(FKernel->arg_size());
  Function::arg_iterator IA = FDelegate->arg_begin(), EA = FDelegate->arg_end();
  Args.push_back(IA);
  if (Value *VLocalIdsPtr = emitStoreLocalIds(FDelegate, FStub))
    Args.push_back(VLocalIdsPtr);
  while (++IA != EA)
    Args.push_back(IA);

  // Call to kernel
  CIKernel = CallInst::Create(FKernel, Args, "", RI);
  CIKernel->setCallingConv(FKernel->getCallingConv());
}

// emitStoreLocalIds - Emit the code to save the current local IDs array, into
// the kernel delegate function. We also emit the call to inc_local_id() after
// the save. The function returns the local IDs array.
Value *AMDLowerThreadInfoBlock::emitStoreLocalIds(Function *F,
                                                   Function *FStub) const
{
  if (!FGetTIBLocalId)
    return 0;

  const unsigned SIZE_ARRAY = 4;

  TerminatorInst *TI = F->getEntryBlock().getTerminator();

  AllocaInst *LocalIdsArray =
    new AllocaInst(ArrayType::get(localIdsPtrTy->getElementType(), SIZE_ARRAY),
                   "", &F->getEntryBlock().front());

  IntegerType *Int32Ty = Type::getInt32Ty(FStub->getContext());
  CallInst *CI;

  // Fill-up the allocated array
  //
  Value *Idxs[2] = { ConstantInt::get(Int32Ty, 0), 0 };
  Value *Args[2] = { F->arg_begin(), 0 };
  for (unsigned DimIdx = 0; DimIdx < SIZE_ARRAY; ++DimIdx) {
    Args[1] = Idxs[1] = ConstantInt::get(Int32Ty, DimIdx);
    Value *GEP = GetElementPtrInst::Create(LocalIdsArray, Idxs, "", TI);
    CI = CallInst::Create(FGetTIBLocalId, Args, "", TI);
    CI->setCallingConv(FGetTIBLocalId->getCallingConv());
    new StoreInst(CI, GEP, TI);
  }

  // After saving the old local ID we can increment it
  //
  CI = CallInst::Create(FInclocalId, Args[0], "", TI);
  CI->setCallingConv(FInclocalId->getCallingConv());

  BasicBlock::iterator IRet = LocalIdsArray;
  while (!isa<GetElementPtrInst>(++IRet))
    ;
  return IRet;
}

// getNextOffsetInStubArgs - Returns the offset of the next aggregated argument
// to load from StubArgsAddr, in bytes.
unsigned AMDLowerThreadInfoBlock::getNextOffsetInStubArgs(
                                         Function *F, Value *StubArgsAddr) const
{
  BasicBlock::InstListType &InstList = F->getEntryBlock().getInstList();
  for (BasicBlock::InstListType::reverse_iterator I = InstList.rbegin(),
       E = InstList.rend(); I != E; ++I) {
    if (!isa<GetElementPtrInst>(&*I))
      continue;

    const GetElementPtrInst *GEP = cast<GetElementPtrInst>(&*I);
    Type *Ty = (dyn_cast<PointerType>(GEP->getPointerOperandType()))->getElementType();
    unsigned TSize = (unsigned)DL->getTypeAllocSize(Ty);

    unsigned ElemSize = 0;
    while (++I != E) {
      if (const LoadInst *LI = dyn_cast<LoadInst>(&*I)) {
        if (LI->getPointerOperand() == StubArgsAddr) {
          ElemSize = DataLayout::RoundUpAlignment(TSize, LI->getAlignment());
          break;
        }
      }
      else if (isa<GetElementPtrInst>(&*I)) {
        // This is a complex type. The buffer is converted to a bytes array.
        GEP = cast<GetElementPtrInst>(&*I);
        ElemSize = 1;
        break;
      }
    }

    assert(ElemSize && "Invalid element size!");

    ConstantInt *CI = (ConstantInt*)((*GEP->idx_begin()).get());
    unsigned NumElems = (unsigned)CI->getZExtValue();
    return TSize + (ElemSize * NumElems);
  }
  return 0;
}

// getStubArgsAddr - Returns the stub's argument address.
static Value *getStubArgsAddr(Function *FStub)
{
  BasicBlock &BB = FStub->getEntryBlock();
  for (BasicBlock::iterator I = BB.begin(), E = BB.end(); I != E; ++I) {
    if (isa<AllocaInst>(I))
      return I;
  }
  return 0;
}

// emitLoadTIBInStub - Emit the code required for loading the thread_info_block
// from the aggregated arguments addressed by the stub's single argument.
// Returns the load instruction.
LoadInst *AMDLowerThreadInfoBlock::emitLoadTIBInStub(Function *FStub)
{
  Value *StubArgsAddr = getStubArgsAddr(FStub);
  Instruction *InsertBefore = getCallToKernelInStub(*FStub);

  unsigned Offset = getNextOffsetInStubArgs(FStub, StubArgsAddr);
  Offset = DataLayout::RoundUpAlignment(Offset, 
                  std::max(DL->getPointerABIAlignment(), DL->getPointerSize()));

  NewArgInfo TIBArg = { TIBPtrTy, SARG_TIB_NAME };

  LoadInst *LI;
  EmitLoadAggregateArguments(FStub->arg_begin(), TIBArg, LI, Offset,
                             *DL, *InsertBefore);

  // Save the next offset as a metadata for future uses, in other passes, even
  // after some optimizations applied.
  // * Required for WorkGroup Level Execution optimization pass.
  //
  LLVMContext &C = FStub->getContext();
  Module *M = FStub->getParent();
  NamedMDNode *MD = M->getOrInsertNamedMetadata(FStub->getName().str() + ".noffset");
  MD->addOperand(MDNode::get(C, ConstantInt::get(Type::getInt32Ty(C), Offset)));
  return LI;
}

// findFunctionUsers - Find all F's users (implicitly and explicitly), tagging
// them with a bit vector that represents all the functions it depends on,
// starting with FlagIdx.
void AMDLowerThreadInfoBlock::findFunctionUsers(Function &F, unsigned &FlagIdx)
{
  SmallVector<User *, 64> Pending;

  DEBUG(dbgs() << "Indirect/direct " << F.getName() << " users:\n");

  SmallBitVector *UseFlag = &Users[&F];
  UseFlag->resize(FlagIdx + 1);
  UseFlag->set(FlagIdx++);

  Pending.push_back(&F);
  do {
    User *U = Pending.pop_back_val();
    if (U->use_empty())
      continue;

    bool IsFirst = true;
    for (Value::use_iterator I = U->use_begin(), E = U->use_end();
         I != E; ++I) {
      User *UI = *I;
      Function *Caller = 0;
      if (isa<CallInst>(UI) || isa<InvokeInst>(UI)) {
        Caller = cast<Instruction>(UI)->getParent()->getParent();
        UI = Caller;
      }
      // Only allowed non Call/Invoke instruction user is Constant.
      else if (isa<Constant>(UI)) {
        if (!isa<GlobalAlias>(UI) && isa<GlobalValue>(UI))
          continue;
      }

      assert((Caller || isa<Constant>(UI)) &&
             "Indirect function calls are not allowed in OpenCL!");

      DEBUG(dbgs() << "    " << (UI->hasName() ? UI->getName() : "N/A") << "\n");
      if (Caller && OCLS->isStub(Caller)) {
        Stubs[Caller] |= Users[U];
      }
      else {
        SmallBitVector &Deps = Users[UI];
        if (Deps.empty()) {
          if (!IsFirst) {
            Deps.resize(FlagIdx + 1);
            Deps |= Users[U];
            Deps.set(FlagIdx++);
          }
          else {
            IsFirst = false;
            Deps = Users[U];
          }

          Pending.push_back(UI);
        }
        else {
          Users[&F] |= Deps;
        }
      }
    }
  } while (!Pending.empty());
}

// initializeWorkItemFunctions - Initialize all WorkItem functions' related
// information.
bool AMDLowerThreadInfoBlock::initializeWorkItemFunctions(Module &M)
{
  unsigned FlagIdx = 0;
  // TODO: The following macro should be removed when Clang becomes the
  // default compiler for building x86 built-ins.Only the check for the
  // mangled names should be retained.
#ifdef BUILD_X86_WITH_CLANG
  // Search for the mangled names also. Note that only builtins are mangled. All the
  // other internal AMD functions will retain have original names itself.
  initializeWorkItemFunction(FlagIdx, WIF_GetGlobalSize,    "_Z15get_global_sizej", M);
  initializeWorkItemFunction(FlagIdx, WIF_GetGlobalId,      "_Z13get_global_idj", M);
  initializeWorkItemFunction(FlagIdx, WIF_GetLocalSize,     "_Z14get_local_sizej", M);
  initializeWorkItemFunction(FlagIdx, WIF_GetLocalId,       "_Z12get_local_idj", M);
  initializeWorkItemFunction(FlagIdx, WIF_GetNumGroups,     "_Z14get_num_groupsj", M);
  initializeWorkItemFunction(FlagIdx, WIF_GetGroupId,       "_Z12get_group_idj", M);
  initializeWorkItemFunction(FlagIdx, WIF_GetGlobalOffset,  "_Z17get_global_offsetj", M);
  initializeWorkItemFunction(FlagIdx, WIF_GetWorkDim,       "_Z12get_work_dimv", M);
  initializeWorkItemFunction(FlagIdx, WIF_Barrier,          "_Z7barrierj", M);

  // TODO: Add the mangled names for the OpenCL 2.0 functions.
  //initializeWorkItemFunction(FlagIdx, WIF_GetGlobalLinearId,"get_global_linear_id", M);
  //initializeWorkItemFunction(FlagIdx, WIF_GetEnqueuedLocalSize,"get_enqueued_local_size", M);
  //initializeWorkItemFunction(FlagIdx, WIF_GetLocalLinearId, "get_local_linear_id", M);
  //initializeWorkItemFunction(FlagIdx, WIF_WorkGroupBarrier, "work_group_barrier", M);
#else
  initializeWorkItemFunction(FlagIdx, WIF_GetGlobalSize,    "get_global_size", M);
  initializeWorkItemFunction(FlagIdx, WIF_GetGlobalId,      "get_global_id", M);
  initializeWorkItemFunction(FlagIdx, WIF_GetGlobalLinearId,"get_global_linear_id", M);
  initializeWorkItemFunction(FlagIdx, WIF_GetEnqueuedLocalSize,"get_enqueued_local_size", M);
  initializeWorkItemFunction(FlagIdx, WIF_GetLocalSize,     "get_local_size", M);
  initializeWorkItemFunction(FlagIdx, WIF_GetLocalId,       "get_local_id", M);
  initializeWorkItemFunction(FlagIdx, WIF_GetLocalLinearId, "get_local_linear_id", M);
  initializeWorkItemFunction(FlagIdx, WIF_GetNumGroups,     "get_num_groups", M);
  initializeWorkItemFunction(FlagIdx, WIF_GetGroupId,       "get_group_id", M);
  initializeWorkItemFunction(FlagIdx, WIF_GetGlobalOffset,  "get_global_offset", M);
  initializeWorkItemFunction(FlagIdx, WIF_GetWorkDim,       "get_work_dim", M);
  initializeWorkItemFunction(FlagIdx, WIF_GetLocalMemAddr,  "__amd_get_local_mem_addr", M);
  initializeWorkItemFunction(FlagIdx, WIF_GetTableBase,     "__get_table_base", M);
  initializeWorkItemFunction(FlagIdx, WIF_GetScratch,       "__get_scratch", M);
  initializeWorkItemFunction(FlagIdx, WIF_Printf,           "__amd_get_builtin_fptr", M);
  initializeWorkItemFunction(FlagIdx, WIF_Barrier,          "barrier", M);
  initializeWorkItemFunction(FlagIdx, WIF_WorkGroupBarrier, "work_group_barrier", M);
#endif
  // We update the WorkItemUseBits only after we constructed all of the UsersMap,
  // as we get references to the SmallBitVector which may change while
  // constructing the map.
  //
  for (unsigned i = 0; i < NUM_WIFTypes; ++i) {
    if (Function *F = WorkItemFunctions[i])
      WorkItemUseBits[i] = &Users[F];
  }
  return 0 != TIBPtrTy;
}

// getFirstCalledFunction - Returns the first call with NumArgs arguments in F.
static Function *getFirstCalledFunction(Function *F, unsigned NumArgs)
{
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
    if (CallInst *CI = dyn_cast<CallInst>(&*I)) {
      if (Function *Callee = CI->getCalledFunction())
        if ((unsigned)Callee->arg_size() == NumArgs)
          return Callee;
    }

  return 0;
}

// initializeWorkItemFunction - Initialize all WorkItem function WIFType related
// information.
void AMDLowerThreadInfoBlock::initializeWorkItemFunction(
  unsigned &FlagIdx, WorkItemFunctionType WIFType, StringRef FuncName, Module &M)
{
  Function *F = M.getFunction(FuncName);
  WorkItemFunctions[WIFType] = F;
  if (!F)
    return;

  if (!TIBPtrTy) {
    FGetTIB = getFirstCalledFunction(F, 0);
    assert(FGetTIB && "get_thread_info_block() has not been found");

    TIBPtrTy = dyn_cast<PointerType>(FGetTIB->getReturnType());
    FGetTIB->setName(FUNC_GET_TIB_NAME);

    AttrBuilder B;
    B.addAttribute(Attributes::NoInline);
    FGetTIB->removeFnAttr(Attributes::get(M.getContext(), B));
    FGetTIB->addFnAttr(Attributes::AlwaysInline);
  }

  if ((WIFType == WIF_GetGlobalId || WIFType == WIF_GetLocalId
       || WIFType == WIF_GetLocalLinearId || WIFType == WIF_GetGlobalLinearId)
      && !FInclocalId) {
    FGetTIBLocalId = getFirstCalledFunction(F, 2);
    assert(FGetTIBLocalId && "get_tib_local_id() has not been found");

    FGetTIBLocalId->setName(FUNC_GET_TIB_LOCALID_NAME);

    localIdsPtrTy = PointerType::getUnqual(FGetTIBLocalId->getReturnType());

    FInclocalId = createIncLocalIdFunction();
    assert(FInclocalId && "Failed to create inc_local_id()");
  }

  findFunctionUsers(*F, FlagIdx);
}

// isDependentOnWorkItemFunction - Returns true if UseBits intersects with
// WIFType's use bits, meaning they have common dependency.
bool AMDLowerThreadInfoBlock::isDependentOnWorkItemFunction(
              const SmallBitVector &UseBits, WorkItemFunctionType WIFType) const
{
  const SmallBitVector *WIBits = WorkItemUseBits[WIFType];
  if (!WIBits)
    return false;

  return (UseBits & *WIBits).any();
}

// createIncLocalIdFunction - Creates the function:
//
// void inc_local_id(clk_thread_info_block_t *tib)
// {
//   ++tib->local_id[0];
//   if (tib->local_id[0] >= tib->local_size[0]) {
//     tib->local_id[0] = 0;
//
//     ++tib->local_id[1];
//     if (tib->local_id[1] >= tib->local_size[1]) {
//       tib->local_id[1] = 0;
//
//       ++tib->local_id[2];
//       if (tib->local_id[2] >= tib->local_size[2]) {
//         tib->local_id[2] = 0;
//       }
//     }
//   }
// }
//
Function *AMDLowerThreadInfoBlock::createIncLocalIdFunction() const
{
  unsigned TIBLocalIdOffset = findLocalIdOffsetInTIB();
  if (TIBLocalIdOffset == unsigned(-1))
    return 0;

  LLVMContext &C = TIBPtrTy->getContext();
  Module *M = FGetTIBLocalId->getParent();
  FunctionType *FTy = FunctionType::get(Type::getVoidTy(C), TIBPtrTy, false);
  Function *F = Function::Create(FTy, Function::InternalLinkage,
                                 FUNC_INC_LOCALID_NAME, M);
  F->addFnAttr(Attributes::NoUnwind);
  F->addFnAttr(Attributes::AlwaysInline);

  IntegerType *Int32Ty = Type::getInt32Ty(C);
  IntegerType *IDValTy = cast<IntegerType>(FGetTIBLocalId->getReturnType());
  ConstantInt *Zero = ConstantInt::get(IDValTy, 0);
  ConstantInt *One  = ConstantInt::get(IDValTy, 1);
  ConstantInt *IdxLocalId   = ConstantInt::get(Int32Ty, TIBLocalIdOffset);
  ConstantInt *IdxLocalSize = ConstantInt::get(Int32Ty, TIBLocalIdOffset - 1);
  Value *Idxs[3] = { ConstantInt::get(Int32Ty, 0) };

  BasicBlock *BB = BasicBlock::Create(C, "entry", F);
  BasicBlock *Exit = BasicBlock::Create(C, "exit", F);
  ReturnInst::Create(C, Exit);

  Argument *TIBPtr = F->arg_begin();
  unsigned Dim = 0;
  do {
    Idxs[1] = IdxLocalId;
    Idxs[2] = ConstantInt::get(Int32Ty, Dim);
    Value *LocalIDPtr = GetElementPtrInst::Create(TIBPtr, Idxs, "", BB);
    Instruction *I = new LoadInst(LocalIDPtr, 0, BB);
    Value *VIncID = BinaryOperator::Create(Instruction::Add, I, One, "", BB);
    new StoreInst(VIncID, LocalIDPtr, BB);

    Idxs[1] = IdxLocalSize;
    I = GetElementPtrInst::Create(TIBPtr, Idxs, "", BB);
    I = new LoadInst(I, 0, BB);

    BasicBlock *BBNext = BasicBlock::Create(C, "", F);
    new StoreInst(Zero, LocalIDPtr, BBNext);

    I = new ICmpInst(*BB, ICmpInst::ICMP_ULT, VIncID, I);
    BranchInst::Create(Exit, BBNext, I, BB);
    BB = BBNext;
    ++Dim;
  } while (Dim < 3);

  BranchInst::Create(Exit, BB);

  return F;
}

unsigned AMDLowerThreadInfoBlock::findLocalIdOffsetInTIB() const
{
  if (!FGetTIBLocalId)
    return unsigned(-1);

  for (inst_iterator I = inst_begin(FGetTIBLocalId), E = inst_end(FGetTIBLocalId);
       I != E; ++I) {
    if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(&*I))
      if (GEP->getNumIndices() == 3)
        if (ConstantInt *CI = dyn_cast<ConstantInt>(next(GEP->idx_begin())))
        return CI->getZExtValue();
  }

  return unsigned(-1);
}
