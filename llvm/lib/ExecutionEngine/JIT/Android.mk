LOCAL_PATH:= $(call my-dir)

lib_ee_jit_SRC_FILES :=	\
	JIT.cpp	\
	JITDwarfEmitter.cpp \
	JITEmitter.cpp	\
	JITMemoryManager.cpp

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

LOCAL_MODULE:= AMDlibLLVMJIT
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(lib_ee_jit_SRC_FILES)

include $(LLVM_DEVICE_BUILD_MK)
include $(BUILD_STATIC_LIBRARY)
