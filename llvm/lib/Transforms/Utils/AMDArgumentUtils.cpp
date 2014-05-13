//===- AMDArgumentUtils.cpp - Argument Utils ------------------------------===//
//
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/AMDArgumentUtils.h"
#include "llvm/Support/CallSite.h"
#include "llvm/Function.h"
#include "llvm/Module.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Constants.h"
#include "llvm/DataLayout.h"
#include "llvm/Value.h"
using namespace llvm;

static AttrListPtr shiftAttributes(LLVMContext &Context,
                                   const AttrListPtr& PAL,
                                   unsigned NumParams,
                                   unsigned Shift)
{
  SmallVector<AttributeWithIndex, 8> AttrVec;

  // Add any return attributes.
  if (PAL.getRetAttributes().hasAttributes())
    AttrVec.push_back(AttributeWithIndex::get(AttrListPtr::ReturnIndex,
                                              PAL.getRetAttributes()));

  for (unsigned Idx = 1; Idx <= NumParams; ++Idx)
    if (PAL.getParamAttributes(Idx).hasAttributes())
      AttrVec.push_back(AttributeWithIndex::get(Idx + Shift, PAL.getParamAttributes(Idx)));

  // Add any function attributes.
  if (PAL.getFnAttributes().hasAttributes())
    AttrVec.push_back(AttributeWithIndex::get(AttrListPtr::FunctionIndex,
                                              PAL.getFnAttributes()));

  return AttrListPtr::get(Context, AttrVec);
}

Function *llvm::CreateFunctionWithNewArguments(Function* F,
                                               ArrayRef<NewArgInfo> NArgs,
                                               bool TakeBlocks)
{
  unsigned NumNArgs = NArgs.size();
  // Create a new function type
  FunctionType *NFTy = GetFunctionTypeWithNewArguments(F->getFunctionType(),
                                                       NArgs);

  // Create the new function
  Function *NF = Function::Create(NFTy, F->getLinkage());
  F->getParent()->getFunctionList().insert(F, NF);

  Function::arg_iterator NI = NF->arg_begin();
  for (unsigned Idx = 0; Idx != NumNArgs; ++Idx, ++NI)
    if (const char *NName = NArgs[Idx].Name)
      NI->setName(NName);

  // Since we have now created the new function, splice the body of the old
  // function right into the new function, leaving the old rotting hulk of the
  // function empty.
  if (TakeBlocks) {
    NF->takeName(F);
    NF->getBasicBlockList().splice(NF->begin(), F->getBasicBlockList());

    // Loop over the argument list, transferring uses of the old arguments over
    // to the new arguments, also transferring over the names as well.
    for (Function::arg_iterator I = F->arg_begin(), E = F->arg_end();
         I != E; ++I, ++NI) {
      I->replaceAllUsesWith(NI);
      NI->takeName(I);
    }
  }
  else {
    if (F->hasName())
      NF->setName(F->getName());

    for (Function::arg_iterator I = F->arg_begin(), E = F->arg_end();
         I != E; ++I, ++NI) {
      if (I->hasName())
        NI->setName(I->getName());
    }
  }

  NF->GlobalValue::copyAttributesFrom(F);
  NF->setAttributes(shiftAttributes(F->getParent()->getContext(),
                                    F->getAttributes(),
                                    (unsigned)F->arg_size(),
                                    NumNArgs));

  NF->setCallingConv(F->getCallingConv());
  if (F->hasGC())
    NF->setGC(F->getGC());
  else
    NF->clearGC();

  return NF;
}

static inline Type *getType(Type* Ty) { return Ty; }
static inline Type *getType(const NewArgInfo &AI) { return AI.Ty; }

template <class TArg>
static inline FunctionType *getFunctionTypeWithNewArguments(FunctionType *FTy,
                                                            ArrayRef<TArg> NArgs)
{
  unsigned NumNArgs = NArgs.size();
  SmallVector<Type *, 16> ArgTypes;
  ArgTypes.reserve(NumNArgs + FTy->getNumParams());

  unsigned Idx;
  for (Idx = 0; Idx != NumNArgs; ++Idx)
    ArgTypes.push_back(getType(NArgs[Idx]));

  ArgTypes.insert(ArgTypes.end(), FTy->param_begin(), FTy->param_end());

  
  return FunctionType::get(FTy->getReturnType(), ArgTypes, FTy->isVarArg());
}

FunctionType *llvm::GetFunctionTypeWithNewArguments(FunctionType *FTy,
                                                    ArrayRef<NewArgInfo> NArgs)
{
  return getFunctionTypeWithNewArguments(FTy, NArgs);
}

FunctionType *llvm::GetFunctionTypeWithNewArguments(FunctionType *FTy,
                                                    ArrayRef<Type*> NTypes)
{
  return getFunctionTypeWithNewArguments(FTy, NTypes);
}

void llvm::AddArgumentsToCallSite(CallSite &CS, SmallVectorImpl<Value*> &Args,
                                  Value *Callee)
{
  Instruction *Call = CS.getInstruction();
  unsigned NumNArgs = (unsigned)Args.size();
  
  Args.insert(Args.end(), CS.arg_begin(), CS.arg_end());

  AttrListPtr NPAL = shiftAttributes(CS.getCaller()->getParent()->getContext(),
                                     CS.getAttributes(),
                                      (unsigned)CS.arg_size(), NumNArgs);

  Instruction *NCall;
  if (InvokeInst *II = dyn_cast<InvokeInst>(Call)) {
    InvokeInst *NII = InvokeInst::Create(Callee, II->getNormalDest(),
                                         II->getUnwindDest(),
                                         Args, "", Call);
    NII->setCallingConv(II->getCallingConv());
    NII->setAttributes(NPAL);
      
    NCall = NII;
  } else {
    CallInst *CI = cast<CallInst>(Call);
    CallInst *NCI = CallInst::Create(Callee, Args, "", Call);
    NCI->setCallingConv(CI->getCallingConv());
    NCI->setAttributes(NPAL);
    if (CI->isTailCall())
      NCI->setTailCall();

    NCall = NCI;
  }
  NCall->setDebugLoc(Call->getDebugLoc());

  if (!Call->use_empty())
    Call->replaceAllUsesWith(NCall);

  NCall->takeName(Call);

  // Remove the old call from the program, reducing the use-count of F.
  Call->eraseFromParent();
}

static void transferCastFunctionUses(ConstantExpr *CE,
                                     Function* F, Function *NF)
{
  if (!CE->isCast() || !isa<PointerType>(CE->getType()))
    return;

  Type *T = cast<PointerType>(CE->getType())->getElementType();
  if (!isa<FunctionType>(T) || T == F->getFunctionType())
    return;

  unsigned NumNArgs = (unsigned)(NF->arg_size() - F->arg_size());
  Type **NTypes = (Type **)alloca(sizeof(Type **) * NumNArgs);

  FunctionType *NFTy = NF->getFunctionType();
  for (unsigned Idx = 0; Idx != NumNArgs; ++Idx)
    NTypes[Idx] = NFTy->getParamType(Idx);
  
  NFTy = GetFunctionTypeWithNewArguments(cast<FunctionType>(T),
                                         makeArrayRef(NTypes, NumNArgs));

  Constant *NCE = ConstantExpr::getCast(CE->getOpcode(), NF,
                                        PointerType::getUnqual(NFTy));


  SmallVector<CallSite, 8> CSites;
  for (ConstantExpr::use_iterator I = CE->use_begin(), E = CE->use_end();
       I != E; ++I) {
    if (isa<CallInst>(*I))
      CSites.push_back(*I);
  }

  bool ReplaceRest = CE->getNumUses() > (unsigned)CSites.size();

  SmallVector<Value*, 16> Args;
  for (SmallVector<CallSite, 8>::iterator I = CSites.begin(), E = CSites.end();
       I != E; ++I) {
    CallSite &CS = *I;
    Function *Caller = CS.getCaller();
    Function::arg_iterator IA = Caller->arg_begin();
    for (unsigned Idx = 0; Idx != NumNArgs; ++Idx)
      Args.push_back(IA++);

    AddArgumentsToCallSite(CS, Args, NCE);
    Args.clear();
  }

  if (ReplaceRest)
    CE->replaceAllUsesWith(NCE);
}

static void transferCastAliasUses(ConstantExpr *CE,
                                  GlobalAlias* A, GlobalAlias *NA)
{
  if (!CE->isCast() || !isa<PointerType>(CE->getType()))
    return;

  PointerType *OldPType = dyn_cast<PointerType>(A->getType());
  PointerType *NewPType = dyn_cast<PointerType>(NA->getType());
  assert(OldPType && "Old Alias type not a pointer type");
  assert(NewPType && "New Alias type not a pointer type");
  FunctionType *OldType = dyn_cast<FunctionType>(OldPType->getElementType());
  FunctionType *NewType = dyn_cast<FunctionType>(NewPType->getElementType());
  assert(OldType && "Old Alias type not a function pointer type");
  assert(NewType && "New Alias type not a function pointer type");

  Type *T = cast<PointerType>(CE->getType())->getElementType();
  if (!isa<FunctionType>(T) || T == NewType)
    return;

  unsigned NumNArgs = (unsigned)(NewType->getNumParams() -
                                 OldType->getNumParams());
  Type **NTypes = (Type **)alloca(sizeof(Type **) * NumNArgs);

  FunctionType *NFTy = NewType;
  for (unsigned Idx = 0; Idx != NumNArgs; ++Idx)
    NTypes[Idx] = NFTy->getParamType(Idx);
  
  NFTy = GetFunctionTypeWithNewArguments(cast<FunctionType>(T),
                                         makeArrayRef(NTypes, NumNArgs));

  Constant *NCE = ConstantExpr::getCast(CE->getOpcode(), NA,
                                        PointerType::getUnqual(NFTy));


  SmallVector<CallSite, 8> CSites;
  for (ConstantExpr::use_iterator I = CE->use_begin(), E = CE->use_end();
       I != E; ++I) {
    if (isa<CallInst>(*I))
      CSites.push_back(*I);
  }

  bool ReplaceRest = CE->getNumUses() > (unsigned)CSites.size();

  SmallVector<Value*, 16> Args;
  for (SmallVector<CallSite, 8>::iterator I = CSites.begin(), E = CSites.end();
       I != E; ++I) {
    CallSite &CS = *I;
    Function *Caller = CS.getCaller();
    Function::arg_iterator IA = Caller->arg_begin();
    for (unsigned Idx = 0; Idx != NumNArgs; ++Idx)
      Args.push_back(IA++);

    AddArgumentsToCallSite(CS, Args, NCE);
    Args.clear();
  }

  if (ReplaceRest)
    CE->replaceAllUsesWith(NCE);
}

void llvm::TransferAliasUses(GlobalAlias* A, GlobalAlias *NA)
{
  PointerType *OldPType = dyn_cast<PointerType>(A->getType());
  PointerType *NewPType = dyn_cast<PointerType>(NA->getType());
  assert(OldPType && "Old Alias type not a pointer type");
  assert(NewPType && "New Alias type not a pointer type");
  FunctionType *OldType = dyn_cast<FunctionType>(OldPType->getElementType());
  FunctionType *NewType = dyn_cast<FunctionType>(NewPType->getElementType());
  assert(OldType && "Old Alias type not a function pointer type");
  assert(NewType && "New Alias type not a function pointer type");

  unsigned NumNArgs = (unsigned)(NewType->getNumParams() -
                                 OldType->getNumParams());

  // Loop over all of the callers of the function, transforming the call sites
  // to pass in a smaller number of arguments into the new function.
  SmallVector<Value*, 16> Args;
  Args.reserve(NewType->getNumParams());
  while (!A->use_empty()) {
    User* U = A->use_back();
    CallSite CS(U);
    if (!CS || !CS.getInstruction()) {
      Use &UF = A->use_begin().getUse();

      if (isa<GlobalAlias>(U)) {
        // We can zero this out since the alias is going to be deleted anyway
        UF.set(NULL);
        U = U->use_back();
      }

      
      if (isa<Constant>(U) && !isa<GlobalValue>(U)) {
        Constant *C = cast<Constant>(U);
        
        if (isa<ConstantExpr>(C)) {
          transferCastAliasUses(cast<ConstantExpr>(C), A, NA);
        }

        if (C->use_empty())
          C->destroyConstant();
        else
          C->replaceUsesOfWithOnConstant(A, NA, NULL);

      }
      continue;
    }

    Function *Caller = CS.getCaller();
    Function::arg_iterator I = Caller->arg_begin();
    for (unsigned Idx = 0; Idx != NumNArgs; ++Idx)
      Args.push_back(I++);

    AddArgumentsToCallSite(CS, Args, NA);
    Args.clear();
  }
}

void llvm::TransferFunctionUses(Function* F, Function *NF)
{
  unsigned NumNArgs = (unsigned)(NF->arg_size() - F->arg_size());

  // Loop over all of the callers of the function, transforming the call sites
  // to pass in a smaller number of arguments into the new function.
  SmallVector<Value*, 16> Args;
  Args.reserve(NF->arg_size());
  while (!F->use_empty()) {
    User* U = F->use_back();
    CallSite CS(U);
    if (!CS || !CS.getInstruction()) {
      Use &UF = F->use_begin().getUse();
      
      if (isa<Constant>(U) && !isa<GlobalValue>(U)) {
        Constant *C = cast<Constant>(U);
        
        if (isa<ConstantExpr>(C))
          transferCastFunctionUses(cast<ConstantExpr>(C), F, NF);
        
        if (C->use_empty())
          C->destroyConstant();
        else 
          C->replaceUsesOfWithOnConstant(F, NF, NULL);
      }
      
      continue;
    }

    Function *Caller = CS.getCaller();
    Function::arg_iterator I = Caller->arg_begin();
    for (unsigned Idx = 0; Idx != NumNArgs; ++Idx)
      Args.push_back(I++);

    AddArgumentsToCallSite(CS, Args, NF);
    Args.clear();
  }
}

void llvm::AddArgumentsToFunction(Function* &F, ArrayRef<NewArgInfo> NArgs)
{
  Function *FN = CreateFunctionWithNewArguments(F, NArgs, true);
  TransferFunctionUses(F, FN);
  F->eraseFromParent();
  F = FN;
}

void llvm::EmitLoadAggregateArguments(Value *AggregateArg,
                                      ArrayRef<NewArgInfo> NArgs,
                                      MutableArrayRef<LoadInst*> Loads,
                                      unsigned &Offset, DataLayout &DL,
                                      Instruction &InsertBefore)
{
  IntegerType *Int32Ty = Type::getInt32Ty(AggregateArg->getContext());
  Instruction *I;
  LoadInst *LI = 0;
  for (unsigned Idx = 0, NumNArgs = NArgs.size(); Idx != NumNArgs; ++Idx) {
    Type *Ty = NArgs[Idx].Ty;
    unsigned TySize = DL.getTypeStoreSize(Ty);
    unsigned TyAlign = DL.getABITypeAlignment(Ty);
    Offset = DataLayout::RoundUpAlignment(Offset, TyAlign);

    I = new BitCastInst(AggregateArg, PointerType::getUnqual(Ty),
      "", &InsertBefore);
    I = GetElementPtrInst::Create(I, ConstantInt::get(Int32Ty, Offset / TySize),
      "", &InsertBefore);
    LI = new LoadInst(I, NArgs[Idx].Name, &InsertBefore);
    LI->setAlignment(TyAlign);
    Loads[Idx] = LI;
    Offset += TySize;
  }
}
