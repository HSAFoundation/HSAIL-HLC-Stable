//===---------- AMDUtils.cpp - General Utils ------------------------------===//
//
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/AMDSPIRUtils.h"

#include "llvm/Constants.h"
#include "llvm/Function.h"
#include "llvm/Module.h"

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

/// \brief Check whether the Module has OpenCL 2.0 metadata.
bool isOpenCL20Module(Module &M) {

  NamedMDNode *OCLVersion = M.getNamedMetadata("opencl.ocl.version");

  if (!OCLVersion || OCLVersion->getNumOperands() < 1)
    return false;

  MDNode *Ver = OCLVersion->getOperand(0);
  if (Ver->getNumOperands() != 2)
    return false;

  ConstantInt *Major = dyn_cast<ConstantInt>(Ver->getOperand(0));
  ConstantInt *Minor = dyn_cast<ConstantInt>(Ver->getOperand(1));
  if (!(Major && Minor))
    return false;

  if (Major->getZExtValue() == 2)
    return true;

  return false;
}
}
