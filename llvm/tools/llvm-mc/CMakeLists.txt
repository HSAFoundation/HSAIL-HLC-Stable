set(LLVM_LINK_COMPONENTS ${LLVM_TARGETS_TO_BUILD} support MC MCParser MCDisassembler SPIRUtils)

add_llvm_tool(llvm-mc
  llvm-mc.cpp
  Disassembler.cpp
  )
