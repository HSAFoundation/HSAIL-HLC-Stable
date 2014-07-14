//===--Transforms/AMDSPIRUtils.h - General SPIR Utils -----------*- C++ -*-===//
//
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
//===----------------------------------------------------------------------===//
//
// This file contains the list of utility functions which are commonly used in
// optimizer and backends.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_UTILS_AMDUTILS_H
#define LLVM_TRANSFORMS_UTILS_AMDUTILS_H

namespace llvm {

  class Function;
  class Module;

  bool isOpenCLBuiltinFunction(Function *Fn);
  bool isOpenCL20Module(Module &M);
//OpenCL addresspaces
  namespace OpenCLLangAS {
   enum OpenCLAddrSpace {
     opencl_private = 0, // 0
     opencl_global ,     // 1
     opencl_local ,      // 2
     opencl_constant ,   // 3
     opencl_generic      // 4
   };
  };
} // End llvm namespace

#endif
