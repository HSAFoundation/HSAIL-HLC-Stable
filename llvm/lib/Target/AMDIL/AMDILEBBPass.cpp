//===-- AMDILEBBPass.cpp - AMDIL EBB Optimizations -*- C++ -*---------===//
//
// Copyright (c) 2011, Advanced Micro Devices, Inc.
// All rights reserved.
//
//==------------------------------------------------------------------===//

#define DEBUG_TYPE "ebb"
#include "llvm/Support/Debug.h"
#include "AMDIL.h"
#include "AMDILSubtarget.h"
#include "AMDILTargetMachine.h"
#include "AMDILUtilityFunctions.h"
#include "AMDILMachineEBB.h"
#include "AMDILInternal.h"
#include "llvm/Pass.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

namespace {
  class AMDILMachineEBB : public MachineFunctionPass {
  const AMDILTargetMachine* TM;
  public:
    static char ID;
    AMDILMachineEBB(const AMDILTargetMachine* aTM = NULL)
      : MachineFunctionPass(ID),
        TM(aTM)
      {}

    virtual bool runOnMachineFunction(MachineFunction &MF);

    virtual const char *getPassName() const {
      return "AMD Extended Basic Block Optimization Pass";
    }
  };
  char AMDILMachineEBB::ID = 0;
}

bool AMDILMachineEBB::runOnMachineFunction(MachineFunction &MF)
{
  EBBOptimizer EBBOpt(TM, &MF);

  return EBBOpt.run();
}

INITIALIZE_PASS(AMDILMachineEBB, "ebb",
                "Machine Extended Basic Block Optimization", false, false)

FunctionPass *llvm::createAMDILMachineEBBPass(const AMDILTargetMachine& tm)
{
  return new AMDILMachineEBB(&tm);
}

