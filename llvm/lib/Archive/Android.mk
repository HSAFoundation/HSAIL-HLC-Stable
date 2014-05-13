LOCAL_PATH:= $(call my-dir)

SRC_FILES := \
	Archive.cpp \
	ArchiveReader.cpp \
	ArchiveWriter.cpp

# For the host
# =====================================================
include $(CLEAR_VARS)

LOCAL_MODULE:= AMDlibLLVMArchive
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(SRC_FILES)

include $(LLVM_HOST_BUILD_MK)
LOCAL_C_INCLUDES :=	\
	bionic \
	external/stlport/stlport \
	$(LOCAL_C_INCLUDES)
	
include $(LLVM_GEN_INTRINSICS_MK)
include $(BUILD_HOST_STATIC_LIBRARY)

# For the device
# =====================================================
include $(CLEAR_VARS)

LOCAL_MODULE:= AMDlibLLVMArchive
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(SRC_FILES)

include $(LLVM_DEVICE_BUILD_MK)
include $(LLVM_GEN_INTRINSICS_MK)
include $(BUILD_STATIC_LIBRARY)
