//===-- AMDILInternal.h - AMDIL header for internal code -*- C++ -*-------===//
//
// Copyright (c) 2011, Advanced Micro Devices, Inc.
// All rights reserved.
//
//==----------------------------------------------------------------------===//

#ifndef _AMDILINTERNAL_H_
#define _AMDILINTERNAL_H_

namespace llvm {

class AMDILTargetMachine;

FunctionPass *createAMDILMachineEBBPass(const AMDILTargetMachine& tm);

}

#endif
