//===---------- AMDUtils.cpp - General Utils ------------------------------===//
//
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//

#include "llvm/Function.h"
#include "llvm/Transforms/AMDSPIRUtils.h"

#include "cxxabi.h"

namespace llvm {

/// \brief Check whether the function is a builtin.
/// If the function is mangled, then it should be an OpenCL builtin.
/// FIXME: Needs to handle mangled user-level functions.
bool isOpenCLBuiltinFunction(Function *Fn) {
  int status = 0;
  std::string MangledName = Fn->getName().str();
  const char *DemangledName =
      __cxxabiv1::__cxa_demangle(MangledName.c_str(), 0, 0, &status);

  if (status || !DemangledName)
    return false;
  return true;
}
}
