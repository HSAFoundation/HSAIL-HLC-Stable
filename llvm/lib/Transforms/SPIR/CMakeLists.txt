add_llvm_library(LLVMSPIR
  AMDSPIRLoader.cpp
  AMDSPIRUtils.cpp
  SPIRVerifier.cpp
  AMDSPIRMutator.cpp
  cxa_demangle.cpp
  khrext.cpp
  )

add_dependencies(LLVMSPIR intrinsics_gen)
