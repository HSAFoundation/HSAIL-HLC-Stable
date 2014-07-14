LOCAL_PATH:= $(call my-dir)

transforms_scalar_SRC_FILES := \
  ADCE.cpp \
  AMDEDGToIA64Translator.cpp \
  AMDGenerateDevEnqMetadata.cpp \
  AMDLowerAtomics.cpp \
  AMDLowerEnqueueKernel.cpp \
  AMDLowerPipeBuiltins.cpp \
  AMDLowerToPreciseBuiltins.cpp \
  AMDMemCombine.cpp \
  AMDPeephole.cpp \
  AMDPrintfRuntimeBinding.cpp \
  AMDScalarReplArrayElem.cpp \
  AMDSimplifyLibCalls.cpp \
  AMDVectorCoarsening.cpp \
  AMDX86Adapter.cpp \
  BasicBlockPlacement.cpp \
  CodeGenPrepare.cpp \
  ConstantProp.cpp \
  CorrelatedValuePropagation.cpp \
  DCE.cpp \
  DeadStoreElimination.cpp \
  EarlyCSE.cpp \
  GlobalMerge.cpp \
  GVN.cpp \
  IndVarSimplify.cpp \
  JumpThreading.cpp \
  LICM.cpp \
  LoopDeletion.cpp \
  LoopIdiomRecognize.cpp \
  LoopInstSimplify.cpp \
  LoopRotation.cpp \
  LoopStrengthReduce.cpp \
  LoopUnrollPass.cpp \
  LoopUnswitch.cpp \
  LowerAtomic.cpp \
  MemCpyOptimizer.cpp \
  Reassociate.cpp \
  Reg2Mem.cpp \
  SCCP.cpp \
  Scalar.cpp \
  ScalarReplAggregates.cpp \
  SimplifyCFGPass.cpp \
  Sink.cpp \
  SimplifyLibCalls.cpp \
  ObjCARC.cpp \
  SROA.cpp \
  TailRecursionElimination.cpp \

# For the host
# =====================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES :=	\
	$(transforms_scalar_SRC_FILES)

LOCAL_MODULE:= AMDlibLLVMScalarOpts

LOCAL_MODULE_TAGS := optional

#include $(LLVM_HOST_BUILD_MK)
#include $(LLVM_GEN_INTRINSICS_MK)
#include $(BUILD_HOST_STATIC_LIBRARY)

# For the device
# =====================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(transforms_scalar_SRC_FILES)
LOCAL_MODULE:= AMDlibLLVMScalarOpts

# Override the default optimization level to work around a SIGSEGV
# on x86 target builds for SROA.cpp.
# Bug: 8047767
ifeq ($(TARGET_ARCH),x86)
LOCAL_CFLAGS += -O1
endif

LOCAL_CFLAGS += \
	-D OPENCL_MAJOR=2 \
	-D OPENCL_MINOR=0 \
	-std=c++11 \
	-nostdinc++

LOCAL_MODULE_TAGS := optional

include $(LLVM_DEVICE_BUILD_MK)
include $(LLVM_GEN_INTRINSICS_MK)
include $(BUILD_STATIC_LIBRARY)
