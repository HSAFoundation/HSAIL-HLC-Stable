//
// Copyright (c) 2011 Advanced Micro Devices, Inc. All rights reserved.
//

// This file defines an interface to hook-up objects specific to AMD OpenCL.
// AMD OpenCL creates a new context for each OpenCL build, and different builds
// have different contexts.
//
#ifndef _AMD_LLVM_CONTEXT_HOOK_H_
#define _AMD_LLVM_CONTEXT_HOOK_H_
#include <cstdio>
#include <string>
#include <set>

#include "llvm/ADT/Triple.h"
#include "llvm/BasicBlock.h"
#include "llvm/Function.h"
#include "llvm/Module.h"

namespace {

char CStringBuf[16];

}

namespace llvm {

/*
   We can expose the OpenCL runtime option object(amd::options::Options) to LLVM so
   that all build options are accessible from LLVM.  This will work when invoking LLVM
   from the OpenCL runtime,  but will not work for other LLVM tools like opt, llc, etc.

   To make option interface consistant to all LLVM tools and the OpenCL runtime, we keep
   in this file a copy of OpenCL options that are used by LLVM.  Doing so will allow any
   LLVM tool to set up those options.
*/
class AMDOptions {
public:

  AMDOptions() :
    IsGPU(false),
    IsPreLinkOpt(false),
    WholeProgram(false),
    GPUArch(0),
    OptLiveness(true),
    OptPrintLiveness(false),
    NumAvailGPRs(128),
    OptSimplifyLibCall(true),
    EnableFDiv2FMul(true),
    OptMem2reg(true),
    UseJIT(false),
    OptLICM(true),
    AAForBarrier(true),
    UnsafeMathOpt(false),
    NoSignedZeros(false),
    FiniteMathOnly(false),
    FastRelaxedMath(false),
    LUThreshold (100),
    LUCount(0),
    LUAllowPartial(true),
    UnrollScratchThreshold(-1),
    OptMemCombineMaxVecGen(16),
    SRThreshold(128),
    SRAEThreshold(1024),
    APThreshold(3),
    OptInlineAll(false)
  {}

  ~AMDOptions() {}

  bool      IsGPU;
  bool      IsPreLinkOpt;  // true: PreLinkOpt; false : post-link opt
  bool      WholeProgram;
  uint32_t  GPUArch;

  // OpenCL options that are exposed to LLVM
  bool      OptLiveness;
  bool      OptPrintLiveness;
  uint32_t  NumAvailGPRs;
  bool      OptSimplifyLibCall;
  bool      EnableFDiv2FMul;
  bool      OptMem2reg;
  bool      UseJIT;
  bool      OptLICM;
  bool      AAForBarrier;
    // math-related options
  bool      UnsafeMathOpt;
  bool      NoSignedZeros;
  bool      FiniteMathOnly;
  bool      FastRelaxedMath;

  // For Loop unrolling
  int       LUThreshold;
  int       LUCount;
  bool      LUAllowPartial;
  int       UnrollScratchThreshold;

  unsigned  OptMemCombineMaxVecGen;
  unsigned  SRThreshold;
  unsigned  SRAEThreshold;

  // For argument promotion
  unsigned  APThreshold;

  // For AMDIL function support
  bool      AmdilUseDefaultResId;
  bool      OptInlineAll;

  // Check that we are on the HSAIL target
  static inline bool isTargetHSAIL(const std::string &sTriple)
  {
    Triple t(sTriple);
    return t.getArch() == Triple::hsail || t.getArch() == Triple::hsail_64;
  }

  static inline bool isTargetHSAIL(const BasicBlock *BB)
  {
    const Function *F = BB->getParent();
    assert(F);
    return isTargetHSAIL(F->getParent()->getTargetTriple());
  }

  static inline bool isTargetHSAIL(const Instruction *Instr)
  {
    const BasicBlock *BB = Instr->getParent();
    assert(BB);
    return isTargetHSAIL(BB);
  }
};

// The AMDLLVMContextHook object is owned by the driver, so the driver (linker.cpp)
// should release the memory after use.
class  AMDLLVMContextHook {
public:
    std::string  *LLVMBuildLog;
    std::set<std::string> *amdrtFunctions;

    // amd::option::Options *options;

    AMDOptions amdoptions;

    AMDLLVMContextHook()
      : LLVMBuildLog(0),
        amdrtFunctions(0),
        amdoptions()
    { }

    ~AMDLLVMContextHook() {}

    // Utitily
    static const char* getAsCString(int IVal, unsigned radix = 10) {
      if (radix == 16) {
        sprintf(&CStringBuf[0],"%x%c", IVal, '\0');
      }
      else { // radix = 10
        sprintf(&CStringBuf[0],"%d%c", IVal, '\0');
      }
      return &CStringBuf[0];
    }
};

}
#endif // _AMD_LLVM_CONTEXT_HOOKH_
