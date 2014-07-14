//
// Copyright (c) 2008 Advanced Micro Devices, Inc. All rights reserved.
//
#ifndef _AMD_PRELINK_OPT_H_
#define _AMD_PRELINK_OPT_H_

namespace llvm {
  class Module;

  void AMDPrelinkOpt(Module *M, bool Whole, bool DisableSimplifyLibCalls,
                     bool UnsafeFPMath, const char* UseNative,
                     bool lowerToPreciseFunctions);
}; // namespace llvm
#endif // _AMD_PRELINK_OPT_H_
