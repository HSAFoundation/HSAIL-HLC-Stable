LOCAL_PATH := $(call my-dir)

amd_il_parser_TBLGEN_TABLES := \
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

amd_il_parser_SRC_FILES := \
  AMDILMDTypes.cpp 

# For the host
# =====================================================
include $(CLEAR_VARS)
include $(CLEAR_TBLGEN_VARS)

LOCAL_MODULE:= AMDlibLLVMAMDILMDParser
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(amd_il_parser_SRC_FILES)
#LOCAL_C_INCLUDES := $(LOCAL_PATH)/MCTargetDesc
TBLGEN_TABLES := $(amd_il_parser_TBLGEN_TABLES)

#include $(LLVM_HOST_BUILD_MK)
#include $(LLVM_TBLGEN_RULES_MK)
#include $(LLVM_GEN_INTRINSICS_MK)
#include $(BUILD_HOST_STATIC_LIBRARY)

# For the device only
# =====================================================
ifeq ($(TARGET_ARCH),arm)
include $(CLEAR_VARS)
include $(CLEAR_TBLGEN_VARS)

LOCAL_MODULE:= AMDlibLLVMAMDILMDParser
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(amd_il_parser_SRC_FILES)
#LOCAL_C_INCLUDES := $(LOCAL_PATH)/MCTargetDesc
TBLGEN_TABLES := $(amd_il_parser_TBLGEN_TABLES)

include $(LLVM_DEVICE_BUILD_MK)
include $(LLVM_TBLGEN_RULES_MK)
include $(LLVM_GEN_INTRINSICS_MK)
include $(BUILD_STATIC_LIBRARY)
endif
