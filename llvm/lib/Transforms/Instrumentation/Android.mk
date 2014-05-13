LOCAL_PATH:= $(call my-dir)

instrumentation_SRC_FILES := \
  AddressSanitizer.cpp \
  EdgeProfiling.cpp \
  GCOVProfiling.cpp \
  Instrumentation.cpp \
  OptimalEdgeProfiling.cpp \
  PathProfiling.cpp \
  ProfilingUtils.cpp \
  ThreadSanitizer.cpp \
  BlackList.cpp \
  BoundsChecking.cpp

# For the host
# =====================================================
include $(CLEAR_VARS)

LOCAL_MODULE:= AMDlibLLVMInstrumentation
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(instrumentation_SRC_FILES)

include $(LLVM_HOST_BUILD_MK)
include $(LLVM_GEN_INTRINSICS_MK)
include $(BUILD_HOST_STATIC_LIBRARY)

# For the target
# =====================================================
include $(CLEAR_VARS)

LOCAL_MODULE:= AMDlibLLVMInstrumentation
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(instrumentation_SRC_FILES)

include $(LLVM_DEVICE_BUILD_MK)
include $(LLVM_GEN_INTRINSICS_MK)
include $(BUILD_STATIC_LIBRARY)
