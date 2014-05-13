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

  bool isOpenCLBuiltinFunction(Function *Fn);
} // End llvm namespace

#endif
