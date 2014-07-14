//==- AMDLowerEnqueueKernel.cpp - Lower enqueue_kernel() calls  -*- C++ -*-===//
//
// Copyright(c) 2014 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// \brief Lower all the enqueue_kernel() calls to the enqueue_kernel_internal()
///  function defined in the builtins library. This pass inserts calls a new
///  function call to allocate the memory for the context structure in the
///  kernarg segment.
//
///  This pass also replaces the first argument of all the block invocation
///  function from i8* to corresponding context structure type.
//
//===----------------------------------------------------------------------===//

#define DEBUGTYPE "AMDLowerEnqueueKernel"

#include "llvm/ADT/StringRef.h"
#include "llvm/Attributes.h"
#include "llvm/DataLayout.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/Intrinsics.h"
#include "llvm/IRBuilder.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/AMDSPIRUtils.h"

using namespace llvm;

namespace llvm {
class AMDLowerEnqueueKernel : public ModulePass {

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<DataLayout>();
  }

public:
  static char ID;

  AMDLowerEnqueueKernel() : ModulePass(ID) {
    initializeAMDLowerEnqueueKernelPass(*PassRegistry::getPassRegistry());
  }

  virtual bool runOnModule(Module &M);
};
}

INITIALIZE_PASS(AMDLowerEnqueueKernel, "amd-opencl-lower-enqueue-kernel",
                "Lower enqueue_kernel() OpenCL builtin", false, false);

char AMDLowerEnqueueKernel::ID = 0;

namespace {
// Following are the number of arguments in the enqueue_kernel()
// overrloaded fucntions.
// TODO: Note that the ndrange_t argument is split into 4 arguments.
// Change the number of arguments when the ndrange_t argument is
// is fixed.
const unsigned int NumEnqFuncEventArgs = 10;
const unsigned int NumEnqFuncArgs = 7;

// NumDefaultArgsToKernel - default number of arguments to any
// OpenCL kernel: global offsets (3), printf, vqueue pointer, aqlWrap pointer.
const unsigned int NumDefaultArgsToKernel = 6;

}

namespace llvm {
ModulePass *createAMDLowerEnqueueKernelPass() {
  return new AMDLowerEnqueueKernel();
}
}

/// \brief  Find whether enqueue_kernel has event related arguments.
/// enqueue_kernel() with event arguments will have name mangling
/// for clk_event_t type.
/// TODO: Return based on type instead of depending on SPIR mangling.
static bool hasEventArgs(StringRef FnName) {
  return FnName.find("13ocl_clk_event") != StringRef::npos;
}

/// \brief The "local void*" are present if the number of arguments
/// for enqueue_kernel is more than and 10 and 7, based on the event
/// args. This function returns the numbe of such arguments present
/// in the enqueue_kernel().
static unsigned int getNumLocalArgs(Function *Fn) {

  bool EventArgs = hasEventArgs(Fn->getName());
  if (EventArgs)
    return (Fn->arg_size() - NumEnqFuncEventArgs);
  return (Fn->arg_size() - NumEnqFuncArgs);
}

/// \brief Find the position of the block argument in the enqueue_kernel
/// overloaded function.
static unsigned int computeBlockStructPos(Function *EnqFn) {

  bool EventArgs = hasEventArgs(EnqFn->getName());
  if (EventArgs)
    return NumEnqFuncEventArgs - 1;
  return NumEnqFuncArgs - 1;
}

/// \brief Construct the enqueue_kernel internal function name.
static std::string getInternalFnName(Function *Fn) {
  std::string FnName = "__enqueue_internal_";
  unsigned int LocalArgs = getNumLocalArgs(Fn);
  bool EventArgs = hasEventArgs(Fn->getName());
  assert(LocalArgs >= 0);
  APInt LocalArgsInt = APInt(sizeof(unsigned int), LocalArgs);
  FnName += LocalArgsInt.toString(10, false);
  if (EventArgs)
    FnName += "_events";
  return FnName;
}

/// \brief Get the alignment of the structure.
static unsigned int getStructAlign(const DataLayout &TD,  StructType *STy) {
  const StructLayout *STyLayout = TD.getStructLayout(STy);
  return STyLayout->getAlignment();
}

/// \brief Get the offset for the Context structure in the kernarg segment.
unsigned getContextStructOffset(const DataLayout &TD,
                                unsigned NumLocalArgs,
                                StructType *ContextStructTy) {

  unsigned int ContextAlign = getStructAlign(TD, ContextStructTy);
  unsigned ArgsOffset =
      (NumLocalArgs + NumDefaultArgsToKernel) * TD.getPointerSize();
  // calculate the start position of the context structure.
  unsigned TotalOffset = (ArgsOffset + ContextAlign - 1U) & -ContextAlign;
  return TotalOffset;
}

/// \brief Add the extra padding bytes to make sure that access of the
/// element is fine in the kernarg segment.
static void constructKernelArgs(StructType* STy, std::vector<Type*> &Args,
                           std::vector<unsigned int> &ArgPos, const DataLayout &TD) {

 unsigned int NumLocalArgs = Args.size();
 unsigned int TotalOffset = getContextStructOffset(TD, Args.size(), STy);
 unsigned ArgsOffset =
     (NumLocalArgs + NumDefaultArgsToKernel) * TD.getPointerSize();
 unsigned int PadBytesForSTy = TotalOffset - ArgsOffset;
 Type *I8Ty = IntegerType::getInt8Ty(STy->getContext());

 // TODO: Add types larger than i8, if paddnig bytes is too high.
 for (unsigned Num = 0; Num < PadBytesForSTy; Num++) {
     Args.push_back(I8Ty);
 }

 unsigned int ContextAlign = getStructAlign(TD, STy);
 const StructLayout *STyLayout = TD.getStructLayout(STy);
 for (unsigned Idx = 0; Idx  < STy->getNumElements(); Idx++) {
   Type *Ty = STy->getElementType(Idx);
   // Push the args.
   Args.push_back(Ty);
   // record the position of the field in args
   ArgPos.push_back(Args.size() - 1);
   if (Idx == STy->getNumElements() - 1)
     break;

   unsigned int CurrOffset =
        STyLayout->getElementOffset(Idx) + TD.getTypeAllocSize(Ty);
   unsigned PaddingBytes = STyLayout->getElementOffset(Idx + 1) - CurrOffset;

   // Fill the number of padding bytes.
   // TODO: Add types larger than i8, if paddnig bytes is too high.
   for (unsigned Num = 0; Num < PaddingBytes; Num++) {
     Args.push_back(I8Ty);
   }
 }
}

/// \brief Convert first argument of block invoke function argument as
/// block invoke structure and convert the function to kernel, if possible.
/// Returns new kernel on success, NULL on failure.
static Function *CreateKernelFromBlockInvokeFn(Function *OldFn,
                                               const DataLayout &TD,
                                               std::string FuncName) {
#ifndef ANDROID
  llvm::Value *ContextArg = next(OldFn->arg_begin(), OldFn->arg_size() - 1 );
  unsigned int NumUses = ContextArg->getNumUses();
#else
  llvm::Value *ContextArg = OldFn->arg_begin();
  unsigned int NumUses = OldFn->arg_size() - 1;
#endif
  // Block doesn't capture anything. So, there is no need to create
  // kernel. OR if the return type is not void, it can't be kernel.
  // TODO: Find the right criteria to create kernels only for blocks
  // used in enqueue_kernel call.
  if (NumUses == 0 || !OldFn->getReturnType()->isVoidTy())
    return NULL;

  assert( NumUses == 1);
  BitCastInst *FirstUse = cast<BitCastInst>(*ContextArg->use_begin());
  PointerType *PtrTy = cast<PointerType>(FirstUse->getType());
  StructType *ContextStructTy = cast<StructType>(PtrTy->getElementType());

  std::vector<Type *> Args;
  std::vector<unsigned int> ArgPos;

  Function::arg_iterator AI = OldFn->arg_begin();
  // Push all the arguments except the last one.
  for (unsigned int Num = 1; Num < OldFn->arg_size(); ++AI, ++Num) {
    Args.push_back(AI->getType());
  }

  constructKernelArgs(ContextStructTy, Args, ArgPos, TD);

  FunctionType *NewFnTy =
   FunctionType::get(OldFn->getReturnType(), Args, OldFn->isVarArg());
  Function *NewFn = Function::Create(NewFnTy, OldFn->getLinkage());

  NewFn->setName("__OpenCL_" + FuncName + "_kernel");
  NewFn->setCallingConv(CallingConv::SPIR_KERNEL);
  NewFn->addFnAttr(Attributes::NoUnwind);

  // Backend expects name for every kernel argument.
  for (Function::arg_iterator AI = NewFn->arg_begin(), E = NewFn->arg_end();
       AI != E; ++AI) {
	// context structure field names are updated later.
    AI->setName("pad");
  }

  // Retain the names for the local pointer arguments.
  AI = OldFn->arg_begin();
  Function::arg_iterator NewAI = NewFn->arg_begin();
  for (unsigned int Num = 1; Num < OldFn->arg_size(); ++AI, ++NewAI, ++Num) {
     NewAI->setName(AI->getName());
  }

  // Create the new basic block which allocates a temp structure to hold the
  // elements of the type required call the actual block invocatio function.
  BasicBlock *EntryBlock = BasicBlock::Create(NewFnTy->getContext(),
                                              "entry", NewFn);
  IRBuilder<> Builder(EntryBlock);

  //Create the Entry Block.
  Value *STyAlloca = Builder.CreateAlloca(ContextStructTy);
  for ( unsigned Num = 0; Num < ArgPos.size(); Num++) {
#ifndef ANDROID
    Value *ArgN = next(NewFn->arg_begin(), ArgPos[Num]);
#else
	Value *ArgN = NewFn->arg_begin();
#endif
    APInt StructFieldNum = APInt(sizeof(unsigned int), Num);
    std::string FieldName = "ctx_struct_fld_";
	FieldName += StructFieldNum.toString(10, false);
	ArgN->setName(FieldName);
    Value *NthEle = Builder.CreateStructGEP(STyAlloca, Num, "context.field");
    StoreInst *StoreNthEle = Builder.CreateStore(ArgN, NthEle);
    StoreNthEle->setAlignment(TD.getABITypeAlignment(ArgN->getType()));
  }
  Value *VoidPtr = Builder.CreateBitCast(STyAlloca, AI->getType());

  SmallVector<Value*, 4> FnCallArgs;
  //Construct call to the block invoke function arguments.
  Function::arg_iterator NewArgs = NewFn->arg_begin();
  for (unsigned int Num = 1; Num < OldFn->arg_size(); ++NewArgs, ++Num) {
    FnCallArgs.push_back(NewArgs);
  }
  FnCallArgs.push_back(VoidPtr);

  CallInst *CallFunc = Builder.CreateCall(OldFn, FnCallArgs);
  CallFunc->setCallingConv(CallingConv::SPIR_FUNC);
  Builder.CreateRetVoid();

#if defined(LD_KERNARG_APPROACH_NEEDS_HSAIL_BACKEND_FIX) && 0
  // Get the body of the old function.
  NewFn->getBasicBlockList().splice(NewFn->begin(), OldFn->getBasicBlockList());


  // Generate the correct offset for the the context structure in the
  // kernarg segment.
  unsigned int Offset = getContextStructOffset(TD, OldFn->arg_size() - 1,
                                               ContextStructTy);
  LLVMContext &TheContext = OldFn->getParent()->getContext();
  APInt OffsetInfo(32, Offset);
  Constant *OffsetVal = Constant::getIntegerValue(
                           Type::getInt32Ty(TheContext), OffsetInfo);

  // Create pointer to kernarg segment.
  PointerType *PtrKernargPtr = PointerType::get(ContextStructTy, 8);

  // FIXME: Create the loads for new address space by accessing the GEPs.
  IntToPtrInst *ContextAddr =
     new IntToPtrInst(OffsetVal, PtrKernargPtr,"context.addr", FirstUse);

  FirstUse->replaceAllUsesWith(ContextAddr);
  FirstUse->eraseFromParent();

  Function::arg_iterator OldAI = OldFn->arg_begin(), NewAI = NewFn->arg_begin();

  // Replace all the uses of old function arguments to new one.
  for (unsigned int Num = 1; Num < OldFn->arg_size();
       ++OldAI, ++NewAI, ++Num) {
    OldAI->replaceAllUsesWith(NewAI);
  }
#endif
  return NewFn;
}

/// \brief Create new function type with the block argument replaced to the
/// type declared in the original block call.
static Function *createNewEnqFunc(Function *EnqFunc, FunctionType* BlockFnTy,
                                  unsigned int BlockStructPos) {
  SmallVector<Type*, 8> EnqFnArgs;
  FunctionType * EnqFnTy = EnqFunc->getFunctionType();
  for (unsigned int ArgNum = 0; ArgNum < EnqFnTy->getNumParams(); ++ArgNum) {
    if (ArgNum == BlockStructPos)
      EnqFnArgs.push_back(PointerType::get(BlockFnTy,0));
    else
      EnqFnArgs.push_back(EnqFnTy->getParamType(ArgNum));
  }

  FunctionType *NewEnqFnTy = FunctionType::get(EnqFnTy->getReturnType(),
                                               EnqFnArgs,
                                               EnqFnTy->isVarArg());
  Function *NewEnqFn = Function::Create(NewEnqFnTy,
                                        GlobalVariable::ExternalLinkage,
                                        EnqFunc->getName(),
                                        EnqFunc->getParent());
  NewEnqFn->addFnAttr(Attributes::AlwaysInline);
  NewEnqFn->addFnAttr(Attributes::NoUnwind);

  return NewEnqFn;
}

/// \brief Return Alignment for Nth local argument in the enqueue_kernel
/// call.
static Value* getAlignmentForNthLocalArg(Value *BlockArg, unsigned ArgPos,
                                         const DataLayout &TD,
                                          LLVMContext &TheContext) {
  PointerType *PtrTy = cast<PointerType>(BlockArg->getType());
  FunctionType *BlockFnTy = cast<FunctionType>(PtrTy->getElementType());
  PointerType *ArgPtrTy = cast<PointerType>(BlockFnTy->getParamType(ArgPos));
  unsigned int Align = TD.getABITypeAlignment(ArgPtrTy->getElementType());

  // if the type is "local i8*", then assign the maximum alignment for this
  // argument. Note that, even char and uchar local pointers are aligned to 128
  // byte boundary.
  if (Align == 1) Align = 128;

  APInt AlignInfo(32, Align);
  Constant *AlignVal = Constant::getIntegerValue(
                       Type::getInt32Ty(TheContext), AlignInfo);
  return AlignVal;
}


/// \brief Define the enqueue_kernel functions. The enqueue_kernel
/// function is lowered to the internal library functions.
//
// The prototype of the internel functions are as follows.
//   int __enqueue_prep_{0,1..,10} (queue_t Queue, size_t CtxSize,
//                                  int CtxAlign, global void *context,
//                                  global void** AQLWrap);
//
//  int
//  __enqueue_internal_{0,1,.,10}[_events] (
//    queue_t q,
//    int flags,
//    int dims, size_t goff[3], size_t gsize[3], size_t lsize[3],
//    __global void * something_like_function_pointer,
//    __global void * wrap_ptr_from_prep
//    [, uint size0, uint align0
//    [, uint size1, uint align1
//    [, uint size2, uint align2
//    [, uint size3, uint align3
//    ...]]]]]] );
//
//  The enqueue_kernel OpenCL builtin will be defined as follows.
//
//  int enqueue_kernel(...) {
//
//    int status =  __enqueue_prep_{0,1..,10} (...);
//    if (status != CLK_SUCCESS) return status;
//    return __enqueue_internal_0,1,.,10}[_events] (...);
//  }
static void defineEnqueueKernelFn(Function *EnqFn, const DataLayout &TD) {

  unsigned int BlockArgPos = computeBlockStructPos(EnqFn);
  EnqFn->addFnAttr(Attributes::AlwaysInline);
  Module *M = EnqFn->getParent();
  LLVMContext &TheContext = M->getContext();

  BasicBlock *EntryBlock = BasicBlock::Create(M->getContext(), "entry", EnqFn);
  BasicBlock *IFBlock = BasicBlock::Create(M->getContext(), "if.then", EnqFn);
  BasicBlock *ElseBlock = BasicBlock::Create(M->getContext(), "else", EnqFn);
  IRBuilder<> IFBuilder(IFBlock), ElseBuilder(ElseBlock), Builder(EntryBlock);

  // TODO: Add addr space for pointers as required.
  IntegerType *ContextSize = TD.getIntPtrType(TheContext);
  PointerType *VoidPtrTy = PointerType::get(Type::getInt8Ty(TheContext), 0);
  PointerType *VoidGPtrTy = PointerType::get(Type::getInt8Ty(TheContext), 1);
  StructType *BlockStructTy = StructType::get(VoidGPtrTy, ContextSize,
                                              Type::getInt32Ty(TheContext),
                                              NULL);
  PointerType *BlockPtrTy = PointerType::get(BlockStructTy, 1);
  StructType *SPIRStructTy = StructType::get(BlockPtrTy, VoidPtrTy, NULL);
  PointerType *SPIRPtrTy = PointerType::get(SPIRStructTy, 0);
#ifndef ANDROID
  llvm::Value *BlockArg = next(EnqFn->arg_begin(), BlockArgPos);
#else
  llvm::Value *BlockArg = EnqFn->arg_begin();
#endif

  Value *BlockStruct = Builder.CreateBitCast(BlockArg, SPIRPtrTy);
  Value *FirstField = Builder.CreateStructGEP(BlockStruct, 0);
  Value *LoadGlobalStruct = Builder.CreateLoad(FirstField, "global_ptr");

  // Prepare args for first call to builtins library to get memory for
  // context structure.
  Value *QueueT = EnqFn->arg_begin();

  Value *ContextSizeAddr = Builder.CreateStructGEP(LoadGlobalStruct, 1);
  LoadInst *ContextStrSize =
      Builder.CreateLoad(ContextSizeAddr, "context_size");
  Type *ContextSizeType = ContextStrSize->getType(); // i32 or i64
  ContextStrSize->setAlignment(TD.getABITypeAlignment(ContextSizeType));

  Value *ContextAlignAddr = Builder.CreateStructGEP(LoadGlobalStruct, 2);
  LoadInst *ContextStrAlign =
      Builder.CreateLoad(ContextAlignAddr, "context_align");
  Type *ContextAlignType = ContextStrAlign->getType(); // i32
  ContextStrAlign->setAlignment(TD.getABITypeAlignment(ContextAlignType));

  Value *SecondSPIRFld = Builder.CreateStructGEP(BlockStruct, 1);
  LoadInst *LoadContextStruct =
      Builder.CreateLoad(SecondSPIRFld, "context_str");
  LoadContextStruct->setAlignment(
      TD.getABITypeAlignment(LoadContextStruct->getType()));

  PointerType *VoidDoublePtr = PointerType::get(VoidGPtrTy, 0);
  AllocaInst *AQLWrap = Builder.CreateAlloca(VoidGPtrTy, 0, "aql_wrap_ret");
  AQLWrap->setAlignment(TD.getABITypeAlignment(VoidGPtrTy));

  // Arrange arg types for first call.
  SmallVector<Type *, 4> Args;
  Args.push_back(QueueT->getType());
  Args.push_back(ContextSizeType);
  Args.push_back(ContextAlignType);
  Args.push_back(VoidPtrTy);
  Args.push_back(VoidDoublePtr);

  IntegerType *I32Ty = IntegerType::getInt32Ty(TheContext);
  FunctionType *FirstFnType = FunctionType::get(I32Ty, Args, false);

  // Generate the function name for first call.
  std::string EnqPrepName = "__enqueue_prep_";
  unsigned int LocalArgs = getNumLocalArgs(EnqFn);
  assert(LocalArgs >= 0);
  APInt LocalArgsInt = APInt(sizeof(unsigned int), LocalArgs);
  EnqPrepName += LocalArgsInt.toString(10, false);

  Function *FirstFn =
    cast<Function>(M->getOrInsertFunction(EnqPrepName, FirstFnType));
  FirstFn->setCallingConv(CallingConv::SPIR_FUNC);
  FirstFn->addFnAttr(Attributes::AlwaysInline);
  FirstFn->addFnAttr(Attributes::NoUnwind);

  CallInst *CallFirstFn =
      Builder.CreateCall5(FirstFn, QueueT, ContextStrSize,ContextStrAlign,
                          LoadContextStruct, AQLWrap, "status");
  CallFirstFn->setCallingConv(CallingConv::SPIR_FUNC);

  llvm::Value *CompareNULL =
      Builder.CreateICmpNE(CallFirstFn, ConstantInt::get(I32Ty,
                                                         0/*CLK_SUCCESS*/));
  llvm::Value *Branch = Builder.CreateCondBr(CompareNULL, IFBlock, ElseBlock);

  // if (status != CLK_SUCCESS ) return status;
  IFBuilder.CreateRet(CallFirstFn);

  // "else", fill the context structure and call __enqueue_internal

  SmallVector<Type *, 4> SecondCallArgsTypes;
  SmallVector<Value *, 4> SecondCallArgs;

  unsigned I = 0;
  for (Function::arg_iterator AI = EnqFn->arg_begin();
       I < BlockArgPos; ++AI, ++I) {
    SecondCallArgsTypes.push_back(AI->getType());
    SecondCallArgs.push_back(AI);
  }
#if defined(TEMP_FIX_FOR_CONFORMANCE_TESTS) || 1
  Value *BlockFnPtr = ElseBuilder.CreateStructGEP(LoadGlobalStruct, 0);
  LoadInst *LoadPtr = ElseBuilder.CreateLoad(BlockFnPtr, "block.invoke.ptr");
  LoadPtr->setAlignment(TD.getABITypeAlignment(LoadPtr->getType()));

  // Push Block invoke function Pointer.
  SecondCallArgsTypes.push_back(LoadPtr->getType());
  SecondCallArgs.push_back(LoadPtr);
#else
  // FIXME: Push the hard-coded kernel index for now. This should be fine for
  // the conformance tests.

  APInt KernelIndex(32, 1);
  Constant *KIdxVal = Constant::getIntegerValue(
                                     Type::getInt32Ty(TheContext), KernelIndex);
  llvm::Value* IdxToPtr =
      ElseBuilder.CreateIntToPtr(KIdxVal, VoidGPtrTy, "kernel_index");
  SecondCallArgsTypes.push_back(IdxToPtr->getType());
  SecondCallArgs.push_back(IdxToPtr);
#endif
  // Push the AQLWrap argument obtained from first call
  LoadInst *LoadAQL = ElseBuilder.CreateLoad(AQLWrap, "aql_wrap_ptr");
  LoadAQL->setAlignment(TD.getABITypeAlignment(LoadAQL->getType()));
  SecondCallArgsTypes.push_back(LoadAQL->getType());
  SecondCallArgs.push_back(LoadAQL);

  // Push local args size and alginments, if present.
#ifndef ANDROID
  Function::arg_iterator LocalAI = next(EnqFn->arg_begin(), BlockArgPos + 1);
#else
  Function::arg_iterator LocalAI = EnqFn->arg_begin();
#endif
  for (; I < BlockArgPos + LocalArgs; ++I,++LocalAI) {
    SecondCallArgsTypes.push_back(LocalAI->getType()); // For size argument
    SecondCallArgsTypes.push_back(LocalAI->getType()); // For alignment
    SecondCallArgs.push_back(LocalAI);

    Value *LocalPtrAlign = getAlignmentForNthLocalArg(BlockArg,
                                                      I - BlockArgPos, TD,
                                                      TheContext);
    SecondCallArgs.push_back(LocalPtrAlign);
  }

  FunctionType *SecondFnType =
      FunctionType::get(EnqFn->getReturnType(), SecondCallArgsTypes, false);
  std::string EnqKernelInternalName = getInternalFnName(EnqFn);
  Function *SecondFn =
    cast<Function>(M->getOrInsertFunction(EnqKernelInternalName, SecondFnType));
  SecondFn->setCallingConv(CallingConv::SPIR_FUNC);
  SecondFn->addFnAttr(Attributes::AlwaysInline);
  SecondFn->addFnAttr(Attributes::NoUnwind);

  CallInst *SecondCall = ElseBuilder.CreateCall(SecondFn, SecondCallArgs);
  SecondCall->setCallingConv(CallingConv::SPIR_FUNC);
  ElseBuilder.CreateRet(SecondCall);
}

/// \brief This function will fix the enqueue_kernel call to have the right
/// block pointer argument. i.e., convert void* arguments to the right type
/// given by the user.
static void handleEnqKernelCall(Function *F, const DataLayout &TD) {
  // Record the different enqueue_kernel functions, which needs
  // to be defined.
  SmallVector<Function*, 4> EnqKernelFns;

  for (Function::iterator FI = F->begin(), E = F->end();
       FI != E; ++FI) {
    for (BasicBlock::iterator I = FI->begin(), IE = FI->end();
         I != IE;) {
      CallInst *CI = dyn_cast<CallInst>(I++);
      if (!CI)
        continue;

      Function *EnqFunc = CI->getCalledFunction();
      if (!(EnqFunc &&
            EnqFunc->empty() &&
            EnqFunc->getName().find("enqueue_kernel") != StringRef::npos &&
            isOpenCLBuiltinFunction(EnqFunc)))
        continue;

      if (!getNumLocalArgs(EnqFunc)) {
        EnqKernelFns.push_back(EnqFunc);
        continue;
      }

      unsigned int BlockStructPos = computeBlockStructPos(EnqFunc);
      BitCastInst *CastBlockPtr =
          dyn_cast<BitCastInst>(CI->getOperand(BlockStructPos));
      // The arguments specified by the user itself is void*/char*.
      if (!CastBlockPtr) {
        EnqKernelFns.push_back(EnqFunc);
        continue;
      }

      PointerType *FuncPtrTy =
          cast<PointerType>(CastBlockPtr->getOperand(0)->getType());
	  FunctionType* BlockFnTy = dyn_cast<FunctionType>(FuncPtrTy->getElementType());

	  // All the operands are void* itself in the block declaration.
	  // No need to create new function definition.
	  if (!BlockFnTy) {
        EnqKernelFns.push_back(EnqFunc);
        continue;
	  }

      Function *NewEnqFn = createNewEnqFunc(EnqFunc, BlockFnTy, BlockStructPos);
      EnqKernelFns.push_back(NewEnqFn);
      SmallVector<Value*, 8> NewCallArgs;

      for (unsigned int ArgNum = 0; ArgNum < CI->getNumArgOperands();
            ++ArgNum) {
        if (ArgNum == BlockStructPos)
          NewCallArgs.push_back(CastBlockPtr->getOperand(0));
        else
          NewCallArgs.push_back(CI->getArgOperand(ArgNum));
      }
      CallInst *NewEnqCall = CallInst::Create(NewEnqFn, NewCallArgs,
                                              "new_enq_call", CI);
      CI->replaceAllUsesWith(NewEnqCall);
      CI->eraseFromParent();
      CastBlockPtr->eraseFromParent();
    }
  }

 for (SmallVector<Function*,4>::iterator FI = EnqKernelFns.begin(),
      E = EnqKernelFns.end(); FI != E; ++FI) {
   if (!(*FI)->empty()) continue;
   defineEnqueueKernelFn(*FI, TD);
 }
}

/// \brief This function creates the new child kernel for every
/// block invocation function in the Module.
static void handleBlockInvocationFunction(Function *BlockFn,
                                          const DataLayout &TD) {
  Module *TheModule =  BlockFn->getParent();
  LLVMContext &Context = TheModule->getContext();
  Function *BlockKernel =
      CreateKernelFromBlockInvokeFn(BlockFn, TD, BlockFn->getName().str());
  // No need to create kernel for the current block invoke function.
  if (!BlockKernel) return;
  BlockFn->getParent()->getFunctionList().insert(BlockFn, BlockKernel);
  //  BlockFnCopy->eraseFromParent();

  //Add kernel metadata to "opencl.kernels"
   SmallVector <llvm::Value*, 4> kernelMDArgs;
   kernelMDArgs.push_back(BlockKernel);
   NamedMDNode *OpenCLMD =
   TheModule->getOrInsertNamedMetadata("opencl.kernels");

   // FIXME: Add the dummy kernel metadata for now.
   MDNode *KD = cast<MDNode>(OpenCLMD->getOperand(0));
   for (unsigned e = KD->getNumOperands(), Count = 1;
         Count != e; ++Count) {
      kernelMDArgs.push_back(cast<MDNode>(KD->getOperand(Count)));
   }
   MDNode *kernelMDNode = MDNode::get(Context, kernelMDArgs);
   OpenCLMD->addOperand(kernelMDNode);
}

/// \brief Define Kernel Query functions to call the internal
/// library implementations for the same.
static void defineKernelQueryFunctions(Function* Fn) {

  LLVMContext &Context = Fn->getParent()->getContext();
  BasicBlock *EntryBlock = BasicBlock::Create(Context, "entry", Fn);
  IRBuilder <> Builder(EntryBlock);

  std::string NewFuncName;
  StringRef FnName = Fn->getName();
  if (FnName.find("preferred") == StringRef::npos)
    NewFuncName = "__get_kernel_work_group_size_internal";
  else
    NewFuncName = "__get_kernel_preferred_work_group_size_multiple_internal";
  FunctionType *FnTy = FunctionType::get(Fn->getReturnType(), false);
  Function *NewFn =
    cast<Function>(Fn->getParent()->getOrInsertFunction(NewFuncName, FnTy ));
  NewFn->setCallingConv(CallingConv::SPIR_FUNC);

  CallInst *RetVal = Builder.CreateCall(NewFn);
  RetVal->setCallingConv(CallingConv::SPIR_FUNC);
  Fn->addFnAttr(Attributes::AlwaysInline);
  Fn->addFnAttr(Attributes::NoUnwind);

  NewFn->addFnAttr(Attributes::AlwaysInline);
  NewFn->addFnAttr(Attributes::NoUnwind);
  Builder.CreateRet(RetVal);
}

bool AMDLowerEnqueueKernel::runOnModule(Module &M) {
  DEBUG(dbgs() << "AMD Lower enqueue_kernel() OpenCL builtin pass");
  bool Modified = false;
  const DataLayout &TD = getAnalysis<DataLayout>();
  for (Module::iterator FI = M.begin(), E = M.end(); FI != E;) {

    Function *F = FI++;

    StringRef FuncName = F->getName();

    if (F->empty() && isOpenCLBuiltinFunction(F)) {

      if (FuncName.find("get_kernel_work_group_size") == StringRef::npos &&
          FuncName.find("get_kernel_preferred_work_group_size_multiple")
             == StringRef::npos)
        continue;

      Modified = true;
      defineKernelQueryFunctions(F);
      continue;
    }

    // TODO: Check whether one of the uses of the function is a CallInst
    // if yes, then we also need to handle the call with first
    // argument as pointer to context structure.(?)
    if (FuncName.startswith("__amd_blocks_func_")) {
      Modified = true;
      handleBlockInvocationFunction(F, TD);
    } else {
      // FIXME: This is a temporary fix for handling the local non-void* args
      // types to enqueue_kernel call. This should be implemented in Clang
      Modified = true;
      handleEnqKernelCall(F, TD);
    }
  }
  return Modified;
}
