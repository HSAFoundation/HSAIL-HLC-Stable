//
// Copyright (c) 2011 Advanced Micro Devices, Inc. All rights reserved.
//

// This file adds a dummy move (dbgmove) of a stored value for any store that is going
// to be removed by mem2reg pass. Hopefully, this will keep the debug info more 
// accurate. 
// 

#define DEBUG_TYPE "dbgmove"
#include "llvm/Constants.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Instructions.h"
#include "llvm/Function.h"
#include "llvm/ADT/Statistic.h"

using namespace llvm;

namespace {

  // This pass is for gpu debug only.
  //
  // It first calculate the alloca instructions that are safe for
  // promotion (mimic mem2reg()), then it seaches all StoreInst to
  // those allocas.  If any one stores a constant (int, float, double)
  // value,  add a dummy move (add) instruction right before the StoreInst.
  // In doing so,  dbg.value will have a dgbmove as its value after
  // mem2reg() removes StoreInst. 
  // For example:
  //     
  //    %i = alloca i32, align 4
  //    call void @llvm.dbg.declare(metadata !{i32* %i}, metadata !10), !dbg !13
  //    store i32 0, i32* %i, align 4, !dbg !14
  //
  // will be transferred into:
  //    %i = alloca i32, align 4
  //    call void @llvm.dbg.declare(metadata !{i32* %i}, metadata !10), !dbg !13
  //    %dbgmove = add i32 0, 0, !dbg !14
  //    store i32 %dbgmove, i32* %i, align 4, !dbg !14
  //
  // And after mem2reg(), it will be
  //   %dbgmove = add i32 0, 0, !dbg !14
  //   call void @llvm.dbg.value(metadata !{i32 %dbgmove}, i64 0, metadata !10), !dbg !14
  //
  // Without this pass,  the above will be
  //   call void @llvm.dbg.value(metadata !9, i64 0, metadata !10), !dbg !14
  //
  //   where !9 is 0. This causes the loss of the above dbg.value inforation.
  //
  class AMDDbgmove : public FunctionPass {

  public:
    static char ID; // Pass identification, replacement for typeid

    AMDDbgmove() : FunctionPass(ID) { 
      initializeAMDDbgmovePass(*PassRegistry::getPassRegistry());
    }

    // runOnFunction - To run this pass, first we calculate the alloca
    // instructions that are safe for promotion, then we search all
    // store instructions to those alloca. If any one stores a constant
    // value, add a dummy move and store dbgmove in the original alloca.
    // Make sure that the DebugLoc is copied.
    virtual bool runOnFunction(Function &F);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesCFG();
    }
  };

}

char AMDDbgmove::ID = 0;
INITIALIZE_PASS(AMDDbgmove, "dbgmove",
                "Add a move (add x, 0) for debuggin purpose",
                false, false)

// Public interface
FunctionPass *llvm::createAMDDbgmovePass() {
  return new AMDDbgmove();
}


bool AMDDbgmove::runOnFunction(Function &F)
{
  std::vector<AllocaInst*> Allocas;
  BasicBlock &BB = F.getEntryBlock();  // Get the entry node for the function
  bool Changed  = false;

  // Mimic mem2reg for finding all allocas that are safe to promote
  for (BasicBlock::iterator I = BB.begin(), E = --BB.end(); I != E; ++I)
    if (AllocaInst *AI = dyn_cast<AllocaInst>(I))
      if (isAllocaPromotable(AI))
        Allocas.push_back(AI);

  if (Allocas.empty())
    return false;

  // Find StoreInst what stores a constant.
  for(unsigned i=0; i < Allocas.size(); ++i) {
    AllocaInst *AI = Allocas[i];
    for (Value::use_iterator UI = AI->use_begin(), E = AI->use_end();
         UI != E;)  {
      Instruction *User = cast<Instruction>(*UI++);

      if (StoreInst *SI = dyn_cast<StoreInst>(User)) {
        Value *StoredValue = SI->getOperand(0);
        if (isa<ConstantInt>(StoredValue) ||
            isa<ConstantFP>(StoredValue)) {
          Value *ValZero = ConstantInt::getNullValue(StoredValue->getType());
          Instruction::BinaryOps Opcode = isa<ConstantInt>(StoredValue)
                                            ? Instruction::Add
                                            : Instruction::FAdd;
          Instruction *dbgmove = BinaryOperator::Create(
            Opcode, StoredValue, ValZero, "dbgmove", SI);
          DebugLoc SIDL = SI->getDebugLoc();
          if (!SIDL.isUnknown()) {
            dbgmove->setDebugLoc(SIDL);
          }
          SI->setOperand(0, dbgmove);

          Changed = true;
        }
      }
    }
  }

  return Changed;
}

