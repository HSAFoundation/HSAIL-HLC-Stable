LOCAL_PATH:= $(call my-dir)

llvm_link_SRC_FILES := \
  bclinker.cpp \
  llvm-link.cpp

llvm_link_STATIC_LIBRARIES := \
  AMDlibLLVMLinker \
  AMDlibLLVMArchive \
  AMDlibLLVMBitReader \
  AMDlibLLVMScalarOpts \
  AMDlibLLVMVectorize \
  AMDlibLLVMInstCombine \
  AMDlibLLVMTransformUtils \
  AMDlibLLVMipa \
  AMDlibLLVMAnalysis \
  AMDlibLLVMTarget \
  AMDlibLLVMMC \
  AMDlibLLVMBitWriter \
  AMDlibLLVMAsmParser \
  AMDlibLLVMCore \
  AMDlibLLVMSupport \

#===---------------------------------------------------------------===
# llvm-link command line tool (host)
#===---------------------------------------------------------------===

include $(CLEAR_VARS)

LOCAL_MODULE := AMDllvm-link
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(llvm_link_SRC_FILES)
LOCAL_STATIC_LIBRARIES := $(llvm_link_STATIC_LIBRARIES)

include $(LLVM_HOST_BUILD_MK)
LOCAL_CPPFLAGS :=	\
	$(LOCAL_CPPFLAGS)	\
	-Wall \
	-Wno-unused \
	-fms-extensions \
	-DLINUX \
	-march=i686 \
	-D__i386__ \
	-gdwarf-2 \
	-msse2 \
	-pthread \
	-fPIC \
	-DPIC \
	-pipe \
	-ffast-math \
	-fno-finite-math-only \
	-fno-math-errno \
	-fmerge-all-constants \
	-fno-tree-vectorize \
	-DUNIX_OS \
	-D_DEBUG \
	-DqLittleEndian \
	-D OPENCL_MAJOR=1 \
	-D OPENCL_MINOR=2 \
	-D WITH_TARGET_HSAIL \
	-D WITH_AQL \
	-D WITH_ONLINE_COMPILER \
	-D ATI_OS_LINUX \
	-D ATI_ARCH_X86 \
	-D LITTLEENDIAN_CPU \
	-D ATI_BITS_32 \
	-D ATI_COMP_GCC \
	-D AMD_LLVM_INTERNAL \
	-D _GNU_SOURCE \
	-D __STDC_LIMIT_MACROS \
	-D __STDC_CONSTANT_MACROS \
	-D __STDC_FORMAT_MACROS \
	-D AMD_HSAIL

#LOCAL_C_INCLUDES +=	\
	bionic \
	external/stlport/stlport \

LOCAL_LDLIBS += -lpthread -lm -ldl \
	-L "/home/todli/git/kitkat-aosp-amd/vendor/amd/drivers/opencl/contrib/STLport/lib/lnx" -lstlport -lpthread -lpthread -ldl -lm  -Wl,-rpath-link=/home/todli/p4sw/dk/lnx/gcc/gcc-4.4.6-glibc-2.12.2-tls/i686-unknown-linux-gnu/i686-unknown-linux-gnu/lib

include $(BUILD_HOST_EXECUTABLE)

#===---------------------------------------------------------------===
# llvm-link command line tool (target)
#===---------------------------------------------------------------===

include $(CLEAR_VARS)

LOCAL_MODULE := AMDllvm-link
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(llvm_link_SRC_FILES)
LOCAL_STATIC_LIBRARIES := $(llvm_link_STATIC_LIBRARIES)
LOCAL_SHARED_LIBRARIES := \
  libcutils  \
  libstlport



#include $(LLVM_DEVICE_BUILD_MK)
#include $(BUILD_EXECUTABLE)
