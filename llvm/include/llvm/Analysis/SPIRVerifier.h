//===- llvm/Analysis/SPIRVerifier.h - SPIR IR Verifier ----------*- C++ -*-===//
//
// Copyright (c) 2012 The Khronos Group Inc.  All rights reserved.
//
// NOTICE TO KHRONOS MEMBER:
//
// AMD has assigned the copyright for this object code to Khronos.
// This object code is subject to Khronos ownership rights under U.S. and
// international Copyright laws.
//
// Permission is hereby granted, free of charge, to any Khronos Member
// obtaining a copy of this software and/or associated documentation files
// (the "Materials"), to use, copy, modify and merge the Materials in object
// form only and to publish, distribute and/or sell copies of the Materials
// solely in object code form as part of conformant OpenCL API implementations,
// subject to the following conditions:
//
// Khronos Members shall ensure that their respective ICD implementation,
// that is installed over another Khronos Members' ICD implementation, will
// continue to support all OpenCL devices (hardware and software) supported
// by the replaced ICD implementation. For the purposes of this notice, "ICD"
// shall mean a library that presents an implementation of the OpenCL API for
// the purpose routing API calls to different vendor implementation.
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Materials.
//
// KHRONOS AND AMD MAKE NO REPRESENTATION ABOUT THE SUITABILITY OF THIS
// SOURCE CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR
// IMPLIED WARRANTY OF ANY KIND.  KHRONOS AND AMD DISCLAIM ALL WARRANTIES
// WITH REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
// IN NO EVENT SHALL KHRONOS OR AMD BE LIABLE FOR ANY SPECIAL, INDIRECT,
// INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
// FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
// NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH
// THE USE OR PERFORMANCE OF THIS SOURCE CODE.
//
// U.S. Government End Users.   This source code is a "commercial item" as
// that term is defined at 48 C.F.R. 2.101 (OCT 1995), consisting of
// "commercial computer software" and "commercial computer software
// documentation" as such terms are used in 48 C.F.R. 12.212 (SEPT 1995)
// and is provided to the U.S. Government only as a commercial end item.
// Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
// 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the
// source code with only those rights set forth herein.
//
//===----------------------------------------------------------------------===//
//
// This file defines the function verifier interface, that can be used for some
// sanity checking of input to the system, and for checking that transformations
// haven't done something bad.
//
// This does not provide LLVM style verification. It instead assumes that the
// LLVM verifier has already been run and the IR is well formed.
//
// To see what specifically is checked, look at the top of SPIRVerifier.cpp
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_SPIR_VERIFIER_H
#define LLVM_ANALYSIS_SPIR_VERIFIER_H

#include "llvm/PassRegistry.h"
#include "Verifier.h"
#include <string>

namespace llvm {
  extern void initializeSPIRLoaderPass(llvm::PassRegistry&);
  extern void initializeSPIRVerifierPass(llvm::PassRegistry&);
}

namespace llvm {
  typedef struct _spir_state {
    std::string CoreFeat; ///< Comma-delimited list of core features.
    std::string KhrFeat; ///< Comma-delimited list of khr features.
    unsigned SPIRMajor, SPIRMinor; ///< Supported major/minor spir version.
    unsigned OCLMajor, OCLMinor; ///< Supported major/minor ocl version.
  } SPIRState;

/// @brief Create a verifier pass.
///
/// Check a Lightweight SPIR module for compatibility.  When the pass is used, the
/// action indicated by the \p action argument will be used if errors are
/// found. This pass checks to make sure that the features and version
/// are valid for the specific vendor.
FunctionPass *createLightweightSPIRVerifierPass(
  VerifierFailureAction action, ///< Action to take
  SPIRState &state ///< State for the SPIR verifier.
  );

/// Check a Heavyweight SPIR module for compatibility.  When the pass is used, the
/// action indicated by the \p action argument will be used if errors are
/// found. This pass checks to make sure that the features and version
/// are valid for the specific vendor. This pass also checks that the
/// SPIR binary complies to the SPIR specification.
FunctionPass *createHeavyweightSPIRVerifierPass(
  VerifierFailureAction action, ///< Action to take
  SPIRState &state ///< State for the SPIR verifier
  );

bool verifySPIRModule(
    const Module &M, ///< The module to be verified.
    VerifierFailureAction action, ///< Action to take
    SPIRState &state, ///< State for the SPIR verifier
    bool lw, ///< Lightweight mode on true, heavyweight otherwise
    std::string *ErrorInfo = 0 ///< Information about failures.
    );
} // End llvm namespace

#endif // LLVM_ANALYSIS_SPIR_VERIFIER_H_
