set(LLVM_LINK_COMPONENTS linker bitreader bitwriter asmparser ScalarOpts SPIRUtils)

add_llvm_tool(llvm-link
  llvm-link.cpp
  AMDFixUpModule.cpp
  )
