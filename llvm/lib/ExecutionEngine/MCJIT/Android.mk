LOCAL_PATH:= $(call my-dir)

mcjit_SRC_FILES :=	\
	MCJIT.cpp

# For the host
# =====================================================
#include $(CLEAR_VARS)
#LOCAL_MODULE:= AMDlibLLVMJIT

#LOCAL_MODULE_TAGS := optional

#include $(LLVM_HOST_BUILD_MK)
#include $(BUILD_HOST_STATIC_LIBRARY)

# For the device
# =====================================================
include $(CLEAR_VARS)

LOCAL_MODULE:= AMDlibLLVMMCJIT
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(mcjit_SRC_FILES)

include $(LLVM_DEVICE_BUILD_MK)
include $(BUILD_STATIC_LIBRARY)
