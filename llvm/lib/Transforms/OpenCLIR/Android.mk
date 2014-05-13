LOCAL_PATH:= $(call my-dir)

SRC_FILES := \
	AMDOpenCLIRTransform.cpp

# For the host
# =====================================================
include $(CLEAR_VARS)

LOCAL_MODULE:= AMDlibLLVMOpenCLIR
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(SRC_FILES)

#include $(LLVM_HOST_BUILD_MK)
#include $(LLVM_GEN_INTRINSICS_MK)
#include $(BUILD_HOST_STATIC_LIBRARY)

# For the device
# =====================================================
include $(CLEAR_VARS)

LOCAL_MODULE:= AMDlibLLVMOpenCLIR
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(SRC_FILES)

include $(LLVM_DEVICE_BUILD_MK)
include $(LLVM_GEN_INTRINSICS_MK)
include $(BUILD_STATIC_LIBRARY)
