//===- AMDSPIRLoader.hpp - Load a SPIR binary for the device ----*- C++ -*-===//
//
// Load a SPIR binary for the given target triple and materialize into
// a valid LLVMIR binary.
//
//===----------------------------------------------------------------------===//
#ifndef _AMD_SPIR_LOADER_HPP_
#define _AMD_SPIR_LOADER_HPP_

#include "llvm/ADT/Triple.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
namespace llvm {
  class AnalysisUsage;
  class DataLayout;
  class StructType;
  class Type;
} // end llvm namespace

namespace spir {

#define SPIR_METADATA_KERNEL_QUALIFIERS       "opencl.kernels"
#define SPIR_METADATA_KERNEL_ARG_ADDR_SPACE   "kernel_arg_addr_space"
#define SPIR_METADATA_KERNEL_ARG_ACCESS_QUAL  "kernel_arg_access_qual"
#define SPIR_METADATA_KERNEL_ARG_TYPE         "kernel_arg_type"
#define SPIR_METADATA_KERNEL_ARG_BASE_TYPE    "kernel_arg_base_type"
#define SPIR_METADATA_KERNEL_ARG_TYPE_QUAL    "kernel_arg_type_qual"
#define SPIR_METADATA_KERNEL_ARG_NAME         "kernel_arg_name"

  class SPIRLoader : public llvm::ModulePass {
    public:
      SPIRLoader();
      SPIRLoader(bool demangleBuiltin);
      virtual ~SPIRLoader() {}

      virtual bool runOnModule(llvm::Module &M);

      static char ID; // Pass identification
    private:
      bool mDemangleBuiltin;
  }; // end SPIRLoader class

  bool isSPIRType(llvm::Type **type_table, llvm::Type *cmp, uint32_t &x);

} // end namespace spir

#endif // _AMD_SPIR_LOADER_HPP_
