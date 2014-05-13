set(LLVM_LINK_COMPONENTS linker bitreader bitwriter asmparser ipo scalaropts)

add_llvm_tool(llvm-link
  bclinker.cpp
  AMDFixUpModule.cpp
  llvm-link.cpp
  )
