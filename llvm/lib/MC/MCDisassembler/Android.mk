LOCAL_PATH:= $(call my-dir)

mc_SRC_FILES := \
	Disassembler.cpp  \
	EDDisassembler.cpp  \
	EDInst.cpp  \
	EDMain.cpp  \
	EDOperand.cpp  \
	EDToken.cpp

# For the host
# =====================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(mc_SRC_FILES)

LOCAL_MODULE:= AMDlibLLVMMCDisassembler

LOCAL_MODULE_TAGS := optional


include $(LLVM_HOST_BUILD_MK)
include $(BUILD_HOST_STATIC_LIBRARY)

# For the device
# =====================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(mc_SRC_FILES)

LOCAL_MODULE:= AMDlibLLVMMCDisassembler

LOCAL_MODULE_TAGS := optional

include $(LLVM_DEVICE_BUILD_MK)
include $(BUILD_STATIC_LIBRARY)
