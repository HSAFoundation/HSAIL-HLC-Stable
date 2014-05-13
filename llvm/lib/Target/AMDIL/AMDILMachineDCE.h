//==------ AMDILMachineDCE.h : MachineFunction Dead Code Elimination header ---===//
//
// Copyright (c) 2012, Advanced Micro Devices, Inc.
// All rights reserved.
//
//==---------------------------------------------------------------------------===//

//==--------------------------------------------------------------------------===//
//
// API function to MachineFunction Dead Code Elimination.
//
//===-------------------------------------------------------------------------===//

#ifndef _AMDILMACHINEDCE_H_
#define _AMDILMACHINEDCE_H_

#include "AMDILTargetMachine.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

bool doMachineFunctionDCE(const TargetMachine* aTM, MachineFunction *aMF);

}

#endif
