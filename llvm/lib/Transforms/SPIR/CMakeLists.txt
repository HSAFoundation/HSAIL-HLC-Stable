add_llvm_library(LLVMSPIRUtils
  AMDMetadataUtils.cpp
  AMDSPIRLoader.cpp
  AMDSPIRMutator.cpp
  AMDSPIRUtils.cpp
  cxa_demangle.cpp
  SPIRVerifier.cpp
  khrext.cpp
  )

add_dependencies(LLVMSPIRUtils intrinsics_gen)
