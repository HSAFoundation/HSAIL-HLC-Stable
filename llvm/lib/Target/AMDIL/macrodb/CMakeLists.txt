
add_llvm_library(LLVMAMDILMacroDB macrodata.cpp)

add_dependencies(LLVMAMDILMacroDB AMDILCommonTableGen)
