LOCAL_PATH:= $(call my-dir)

spir_SRC_FILES := \
	AMDMetadataUtils.cpp \
	AMDSPIRLoader.cpp \
	AMDSPIRMutator.cpp \
	AMDSPIRUtils.cpp \
	cxa_demangle.cpp \
	khrext.cpp \
	SPIRVerifier.cpp \

# For the device
# =====================================================
include $(CLEAR_VARS)

LOCAL_MODULE:= AMDlibLLVMSPIR
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(spir_SRC_FILES)

include $(LLVM_DEVICE_BUILD_MK)
include $(LLVM_GEN_INTRINSICS_MK)

LOCAL_C_INCLUDES :=	\
	bionic \
	external/stlport/stlport \
	$(LLVM_ROOT_PATH)	\
	$(LLVM_ROOT_PATH)/include	\
	$(LLVM_ROOT_PATH)/device/include	\
	$(LLVM_ROOT_PATH)/linux/include	\
	$(LLVM_ROOT_PATH)/../../api/opencl \
	$(LLVM_ROOT_PATH)/../../contrib/STLport/include/stlport \
	$(LLVM_ROOT_PATH)/include \

LOCAL_CPPFLAGS :=	\
	$(LOCAL_CPPFLAGS) \
	-Wno-error=non-virtual-dtor \
	-frtti

include $(BUILD_STATIC_LIBRARY)
