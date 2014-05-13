include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/.. )

add_llvm_library(LLVMHSAILAsmPrinter
  HSAILInstPrinter.cpp
  HSAILInstComments.cpp
  )

add_dependencies(LLVMHSAILAsmPrinter
  LLVMMC
  LLVMSupport
  )
add_dependencies(LLVMHSAILAsmPrinter HSAILCommonTableGen)
