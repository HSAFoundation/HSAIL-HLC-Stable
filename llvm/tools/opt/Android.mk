LOCAL_PATH := $(call my-dir)

LLVM_ROOT_PATH := $(LOCAL_PATH)/../..


#===---------------------------------------------------------------===
# opt command line tool (common)
#===---------------------------------------------------------------===

llvm_opt_SRC_FILES := \
  AnalysisWrappers.cpp \
  GraphPrinters.cpp \
  opt.cpp \
  PrintSCC.cpp

llvm_opt_STATIC_LIBRARIES := \
  AMDlibLLVMSupport \
  AMDlibLLVMBitReader \
  AMDlibLLVMAsmParser \
  AMDlibLLVMBitWriter \
  AMDlibLLVMInstrumentation \
  AMDlibLLVMipo \
  AMDlibLLVMHSAILCodeGen \
  LIBHSAIL \
  AMDlibLLVMObject \
  AMDlibLLVMAsmPrinter \
  AMDlibLLVMMCParser \
  AMDlibLLVMSelectionDAG \
  AMDlibLLVMCodeGen \
  AMDlibLLVMScalarOpts \
  AMDlibLLVMVectorize \
  AMDlibLLVMInstCombine \
  AMDlibLLVMTransformUtils \
  AMDlibLLVMipa \
  AMDlibLLVMAnalysis \
  AMDlibLLVMTarget \
  AMDlibLLVMMC \
  AMDlibLLVMCore \
  AMDlibLLVMSupport \
  AMDlibLLVMAsmParser \

#===---------------------------------------------------------------===
# opt command line tool (host)
#===---------------------------------------------------------------===

include $(CLEAR_VARS)

LOCAL_MODULE := AMDopt
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_IS_HOST_MODULE := true

LOCAL_SRC_FILES := $(llvm_opt_SRC_FILES)
LOCAL_STATIC_LIBRARIES := $(llvm_opt_STATIC_LIBRARIES)
LOCAL_LDLIBS += -lpthread -lm -ldl \
	-L "/home/todli/git/kitkat-aosp-amd/vendor/amd/drivers/opencl/contrib/STLport/lib/lnx" -lstlport -Wl,-rpath-link=/home/todli/p4sw/dk/lnx/gcc/gcc-4.4.6-glibc-2.12.2-tls/i686-unknown-linux-gnu/i686-unknown-linux-gnu/lib
LOCAL_C_INCLUDES := \
	$(LLVM_ROOT_PATH)/include \
	$(LLVM_ROOT_PATH)/linux/include\
	$(intermediates)
LOCAL_CFLAGS :=	\


include $(LLVM_ROOT_PATH)/llvm.mk
include $(LLVM_HOST_BUILD_MK)
include $(LLVM_GEN_INTRINSICS_MK)
include $(BUILD_HOST_EXECUTABLE)


#===---------------------------------------------------------------===
# opt command line tool (target)
#===---------------------------------------------------------------===

include $(CLEAR_VARS)

LOCAL_MODULE := opt
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES

LOCAL_SRC_FILES := $(llvm_opt_SRC_FILES)
LOCAL_C_INCLUDES += external/llvm/include
LOCAL_STATIC_LIBRARIES := $(llvm_opt_STATIC_LIBRARIES)
LOCAL_SHARED_LIBRARIES :=  \
  libcutils  \
  libdl  \
  libstlport


#include $(LLVM_ROOT_PATH)/llvm.mk
#include $(LLVM_DEVICE_BUILD_MK)
#include $(LLVM_GEN_INTRINSICS_MK)
#include $(BUILD_EXECUTABLE)
