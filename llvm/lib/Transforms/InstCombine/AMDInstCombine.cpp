//
// Copyright (c) 2011 Advanced Micro Devices, Inc. All rights reserved.
//

#define DEBUG_TYPE "instcombine"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IRBuilder.h"
#include "llvm/DataLayout.h"
#include "llvm/Constants.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"
#include "llvm/LLVMContext.h"
#include "llvm/AMDLLVMContextHook.h"
#include "AMDInstCombine.h"

using namespace llvm;

void AMDInstCombiner::initialize(Function *aFunc, DataLayout *aDL)
{
  FP = aFunc;
  DL = aDL;

  LLVMContext &ctx = FP->getContext();
  AMDLLVMContextHook *amdhook = 
    static_cast<AMDLLVMContextHook*>(ctx.getAMDLLVMContextHook());

  // caching options to reuse them
  IsGPU = amdhook ? amdhook->amdoptions.IsGPU : false;
  EnableFDiv2FMul = amdhook ? amdhook->amdoptions.EnableFDiv2FMul
                              : false;

  MyIRBuilder = new IRBuilder<>(ctx);
}

void AMDInstCombiner::finalize()
{
  FP = NULL;
  DL = NULL;
  if (MyIRBuilder) {
    delete MyIRBuilder;
    MyIRBuilder = NULL;
  }
}

Value *AMDInstCombiner::SimplifyFDiv(BinaryOperator *BI)
{
  // So far, only handle float, not double fdiv
  if ( (FP == NULL) || !IsGPU || !EnableFDiv2FMul ||
       !BI->getType()->isFloatTy() ) {
    return NULL;
  }

  Value *Opr0 = BI->getOperand(0), *Opr1 = BI->getOperand(1);
  BasicBlock *BB = BI->getParent();
  MyIRBuilder->SetInsertPoint(BB, BI);

  // Convert f / c ===> f * (1/c)
  if (ConstantFP *CF = dyn_cast<ConstantFP>(Opr1)) {
    if (CF->isZero() || isInfinity(CF) || isFloatSubnormal(CF)) {
      return NULL;
    }
    double divisor = (double)CF->getValueAPF().convertToFloat();
    double res = 1.0 / divisor;
    Constant *NewCF = ConstantFP::get(BI->getType(), res);
    Value *nval = MyIRBuilder->CreateFMul(Opr0, NewCF, "_divC2mulC");

    DEBUG(errs() << "AMDIC: op0 / " << (float)divisor 
                 << " ==> op0 * " << (float)res << "\n");

    return nval;
  }
  return NULL; 
}

