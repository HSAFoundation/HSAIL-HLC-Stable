//===- AMDOpenCLIRTransform - Transform non-OpenCL-style IRs into OpenCL-style IRS ---===//
//
//                    The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===---------------------------------------------------------------------------------===//
// Transform  ARM-ABI based LLVM IR binary into SPIR.
//
//===---------------------------------------------------------------------------------===//

#ifndef _AMD_OpenCLIR_TRANSFORM_
#define _AMD_OpenCLIR_TRANSFORM_

namespace llvm {
  class Function;
  class Module;
};

#include "llvm/Pass.h"

namespace openclir {
// Per SPIR spec.
typedef enum {
  AS_PRIVATE = 0,
  AS_GLOBAL = 1,
  AS_CONSTANT = 2,
  AS_LOCAL = 3
} AddrSpace;

class OpenCLIRTransform : public llvm::ModulePass {
private:
  llvm::Module *curModule; // scratch
public:
  OpenCLIRTransform();
  virtual ~OpenCLIRTransform() {}
  virtual bool runOnModule(llvm::Module &M);
  static char ID; // Pass identification
private:
  bool parseMetaData(llvm::Module &M);
  // Emit SPIR metadata for kernels.
  void emitSPIR(llvm::Function *);
  void addGlobalId(llvm::Function *);
}; // end OpenCLIRTransform class
}; // end namespace openclir

#endif // _AMD_OpenCLIR_TRANSFORM_
