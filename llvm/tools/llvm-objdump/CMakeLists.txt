set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  DebugInfo
  MC
  MCParser
  MCDisassembler
  Object
  SPIRUtils
  )

add_llvm_tool(llvm-objdump
  llvm-objdump.cpp
  MachODump.cpp
  MCFunction.cpp
  )
