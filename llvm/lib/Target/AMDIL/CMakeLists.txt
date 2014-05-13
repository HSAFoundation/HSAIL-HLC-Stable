include_directories(../../../include macrodb)


option(AMD_LLVM_INTERNAL "Enable internal stuff" OFF)

if(AMD_LLVM_INTERNAL)
  add_definitions(-DAMD_LLVM_INTERNAL)
  include_directories(Internal)
endif()

include_directories(AsmPrinter)
add_definitions(-DAMD_LLVM_INTERNAL)
include_directories(Internal)

set(LLVM_TARGET_DEFINITIONS AMDIL.td)

tablegen(LLVM AMDILGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM AMDILGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM AMDILGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM AMDILGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM AMDILGenCallingConv.inc -gen-callingconv)
tablegen(LLVM AMDILGenSubtarget.inc -gen-subtarget)
tablegen(LLVM AMDILGenEDInfo.inc -gen-enhanced-disassembly-info)
tablegen(LLVM AMDILGenIntrinsics.inc -gen-tgt-intrinsic)

add_public_tablegen_target(AMDILCommonTableGen)

add_llvm_target(AMDILCodeGen
  AMDILAsmBackend.cpp
  AMDILAsmPrinter.cpp
  AMDILBarrierDetect.cpp
  AMDILCFGStructurizer.cpp
  AMDILEBBPass.cpp
  AMDILEGIOExpansion.cpp
  AMDILFrameLowering.cpp
  AMDILInliner.cpp
  AMDILInstrInfo.cpp
  AMDILIntrinsicInfo.cpp
  AMDILIOExpansion.cpp
  AMDILInstPrinter.cpp # Not implemented
  AMDILISelDAGToDAG.cpp
  AMDILISelLowering.cpp
  AMDILKernelManager.cpp
  AMDILLiteralManager.cpp
  AMDILMachineDCE.cpp
  AMDILMachineDCE.h
  AMDILMachineEBB.cpp
  AMDILMachineEBB.h
  AMDILMachineFunctionInfo.cpp
  AMDILMachinePeephole.cpp
  AMDILMachineValue.cpp
  AMDILMachineValue.h
  AMDILMCAsmInfo.cpp
  AMDILMCCodeEmitter.cpp
  AMDILModuleInfo.cpp
  AMDILPeepholeOptimizer.cpp
  AMDILPointerManager.cpp
  AMDILPrintfConvert.cpp
  AMDILRegisterInfo.cpp
  AMDILSubtarget.cpp
  AMDILSwizzleEncoder.cpp
  AMDILTargetMachine.cpp
  AMDILUtilityFunctions.cpp
  AMDILSIPointerManager.cpp
  AMDILSIIOExpansion.cpp
  AMDILCompilerErrors.cpp
  AMDILCompilerWarnings.cpp
  AMDILCIPointerManager.cpp
  AMDILRegisterUseValidate.cpp
  AMDILCreateKernelStub.cpp
  AMDILCIIOExpansion.cpp
  AMDILRenumberRegister.cpp
  )

add_dependencies(LLVMAMDILCodeGen intrinsics_gen)
if( CMAKE_CL_64 )
  # A workaround for a bug in cmake 2.8.3. See PR 8885.
  if( CMAKE_VERSION STREQUAL "2.8.3" )
    include(CMakeDetermineCompilerId)
  endif()
  # end of workaround.
endif()

#add_subdirectory(AsmParser)
#add_subdirectory(Disassembler)
add_subdirectory(TargetInfo)
add_subdirectory(macrodb)
add_subdirectory(MDParser)
if(AMD_LLVM_INTERNAL)
  add_subdirectory(Internal)

  # TODO: This is normally handled by LLVMBuild
  target_link_libraries(LLVMAMDILCodeGen LLVMAMDILInternal)
endif()
#add_subdirectory(MCTargetDesc)

