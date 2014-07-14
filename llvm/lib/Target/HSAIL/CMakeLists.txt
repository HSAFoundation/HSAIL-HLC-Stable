set(LLVM_TARGET_DEFINITIONS HSAIL.td)

tablegen(LLVM HSAILGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM HSAILGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM HSAILGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM HSAILGenCodeEmitter.inc -gen-emitter)
tablegen(LLVM HSAILGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM HSAILGenSubtarget.inc -gen-subtarget)
tablegen(LLVM HSAILGenCallingConv.inc -gen-callingconv)
tablegen(LLVM HSAILGenIntrinsics.inc -gen-tgt-intrinsic)
tablegen(LLVM HSAILGenBrigWriter.inc -gen-brig-writer)
tablegen(LLVM HSAILGenVecMap.inc -gen-hsail-vec-map)

# llvm_tablegen(HSAILGenAsmMatcher.inc -gen-asm-matcher)
# llvm_tablegen(HSAILGenAsmWriter.inc -gen-asm-writer)
# llvm_tablegen(HSAILGenCallingConv.inc -gen-callingconv)
# llvm_tablegen(HSAILGenDAGISel.inc -gen-dag-isel)
# llvm_tablegen(HSAILGenEDInfo.inc -gen-enhanced-disassembly-info)
# llvm_tablegen(HSAILGenFastISel.inc -gen-fast-isel)
# llvm_tablegen(HSAILGenInstrNames.inc -gen-instr-enums)
# llvm_tablegen(HSAILGenInstrInfo.inc -gen-instr-desc)
# llvm_tablegen(HSAILGenIntrinsics.inc -gen-tgt-intrinsic)
# llvm_tablegen(HSAILGenRegisterInfo.inc -gen-register-desc)
# llvm_tablegen(HSAILGenRegisterNames.inc -gen-register-enums)
# llvm_tablegen(HSAILGenSubtarget.inc -gen-subtarget)

add_public_tablegen_target(HSAILCommonTableGen)

set(sources
  BRIGAsmPrinter.cpp
  BRIGDwarfCompileUnit.cpp
  BRIGDwarfDebug.cpp
  HSAILAsmBackend.cpp
  BRIGDwarfCompileUnit.cpp
  BRIGDwarfDebug.cpp
  BRIGDwarfStreamer.cpp 
  HSAILDevice.cpp
  HSAILDeviceInfo.cpp
  HSAILEarlyCFGOpts.cpp
  HSAILELFObjectWriter.cpp
  HSAILELFTargetObjectFile.cpp
  HSAILELFWriterInfo.cpp
  HSAILFrameLowering.cpp
  HSAILInstrInfo.cpp
  HSAILIntrinsicInfo.cpp
  HSAILISelDAGToDAG.cpp
  HSAILISelLowering.cpp
  HSAILKernelManager.cpp
  HSAILMachineFunctionInfo.cpp
  HSAILMCAsmInfo.cpp
  HSAILMCCodeEmitter.cpp
  HSAILModuleInfo.cpp
  HSAILOptimizeMemoryOps.cpp
  HSAILParamManager.cpp
  HSAILPropImageOps.cpp
  HSAILRegisterInfo.cpp
  HSAILSelectionDAGInfo.cpp
  HSAILSubtarget.cpp
  HSAILSyntaxCleanupPass.cpp
  HSAILTargetMachine.cpp
  HSAILUtilityFunctions.cpp
  RawVectorOstream.cpp
  HSAILGlobalOffsetInsertionPass.cpp
  HSAILControlDependencyAnalysis.cpp
  HSAILResizePointer.cpp
  HSAILOpaqueTypes.cpp
  HSAILUniformOperations.cpp
  HSAILParseMetadata.cpp
  HSAILLowerSPIRSamplers.cpp
  HSAILFastISel.cpp
  HSAILMergeLoadStores.cpp
  HSAILAddrSpaceCast.cpp
  HSAILKernelArgInsertion.cpp
  HSAILInsertKernelIndexMetadata.cpp
  HSAILPrintfRuntimeBindingMetadata.cpp
  HSAILNullPtrInsertion.cpp
  )

if( CMAKE_CL_64 )
  # A workaround for a bug in cmake 2.8.3. See PR 8885.
  if( CMAKE_VERSION STREQUAL "2.8.3" )
    include(CMakeDetermineCompilerId)
  endif()
  # end of workaround.
#  enable_language(ASM_MASM)
#  ADD_CUSTOM_COMMAND(
#    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/HSAILCompilationCallback_Win64.obj
#    MAIN_DEPENDENCY HSAILCompilationCallback_Win64.asm
#    COMMAND ${CMAKE_ASM_MASM_COMPILER} /Fo ${CMAKE_CURRENT_BINARY_DIR}/HSAILCompilationCallback_Win64.obj /c ${CMAKE_CURRENT_SOURCE_DIR}/HSAILCompilationCallback_Win64.asm
#   )
#   set(sources ${sources} ${CMAKE_CURRENT_BINARY_DIR}/HSAILCompilationCallback_Win64.obj)
endif()

add_llvm_target(HSAILCodeGen ${sources})

add_dependencies(LLVMHSAILCodeGen
  LLVMAnalysis
  LLVMAsmPrinter
  LLVMCodeGen
  LLVMCore
  LLVMMC
  LLVMSelectionDAG
  LLVMSupport
  LLVMTarget
  LLVMHSAILAsmPrinter
  )
#add_subdirectory(AsmParser)
#add_subdirectory(Disassembler)
#add_subdirectory(InstPrinter)
add_subdirectory(TargetInfo)
