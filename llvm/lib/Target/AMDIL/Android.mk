LOCAL_PATH := $(call my-dir)

#amd_il_TBLGEN_TABLES := \
#  ARMGenRegisterInfo.inc \
#  ARMGenInstrInfo.inc \
#  ARMGenCodeEmitter.inc \
#  ARMGenMCCodeEmitter.inc \
#  ARMGenMCPseudoLowering.inc \
#  ARMGenAsmWriter.inc \
#  ARMGenAsmMatcher.inc \
#  ARMGenDAGISel.inc \
#  ARMGenFastISel.inc \
#  ARMGenCallingConv.inc \
#  ARMGenSubtargetInfo.inc \
#  ARMGenDisassemblerTables.inc

amd_il_SRC_FILES := \
  AMDILAsmBackend.cpp \
  AMDILAsmPrinter.cpp \
  AMDILBarrierDetect.cpp \
  AMDILCFGStructurizer.cpp \
  AMDILCIIOExpansion.cpp \
  AMDILCIPointerManager.cpp \
  AMDILCompilerErrors.cpp \
  AMDILCompilerWarnings.cpp \
  AMDILEBBPass.cpp \
  AMDILEGIOExpansion.cpp \
  AMDILFrameLowering.cpp \
  AMDILInliner.cpp \
  AMDILInstPrinter.cpp \
  AMDILInstrInfo.cpp \
  AMDILIntrinsicInfo.cpp \
  AMDILIOExpansion.cpp \
  AMDILISelDAGToDAG.cpp \
  AMDILISelLowering.cpp \
  AMDILKernelManager.cpp \
  AMDILLiteralManager.cpp \
  AMDILMachineDCE.cpp \
  AMDILMachineEBB.cpp \
  AMDILMachineFunctionInfo.cpp \
  AMDILMachinePeephole.cpp \
  AMDILMachineValue.cpp \
  AMDILMCAsmInfo.cpp \
  AMDILMCCodeEmitter.cpp \
  AMDILModuleInfo.cpp \
  AMDILPeepholeOptimizer.cpp \
  AMDILPointerManager.cpp \
  AMDILPrintfConvert.cpp \
  AMDILRegisterInfo.cpp \
  AMDILRegisterUseValidate.cpp \
  AMDILRenumberRegister.cpp \
  AMDILSIIOExpansion.cpp \
  AMDILSIPointerManager.cpp \
  AMDILSubtarget.cpp \
  AMDILSwizzleEncoder.cpp \
  AMDILTargetMachine.cpp \
  AMDILUtilityFunctions.cpp

# For the host
# =====================================================
include $(CLEAR_VARS)
include $(CLEAR_TBLGEN_VARS)

LOCAL_MODULE:= AMDlibLLVMAMDIL
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(amd_il_SRC_FILES)
#LOCAL_C_INCLUDES := $(LOCAL_PATH)/MCTargetDesc
TBLGEN_TABLES := $(amd_il_TBLGEN_TABLES)

#include $(LLVM_HOST_BUILD_MK)
#include $(LLVM_TBLGEN_RULES_MK)
#include $(LLVM_GEN_INTRINSICS_MK)
#include $(BUILD_HOST_STATIC_LIBRARY)

# For the device only
# =====================================================
ifeq ($(TARGET_ARCH),arm)
include $(CLEAR_VARS)
include $(CLEAR_TBLGEN_VARS)

LOCAL_MODULE:= AMDlibLLVMAMDIL
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(amd_il_SRC_FILES)
#LOCAL_C_INCLUDES := $(LOCAL_PATH)/MCTargetDesc
TBLGEN_TABLES := $(amd_il_TBLGEN_TABLES)

include $(LLVM_DEVICE_BUILD_MK)
include $(LLVM_TBLGEN_RULES_MK)
include $(LLVM_GEN_INTRINSICS_MK)
include $(BUILD_STATIC_LIBRARY)
endif
