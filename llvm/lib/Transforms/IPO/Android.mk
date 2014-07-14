LOCAL_PATH:= $(call my-dir)

transforms_ipo_SRC_FILES := \
  AMDInlineAll.cpp \
  AMDOptOptions.cpp \
  AMDPassManagerBuilder.cpp \
  AMDSimplifyCall.cpp \
  AMDSymbolLinkage.cpp \
  ArgumentPromotion.cpp \
  BarrierNoopPass.cpp \
  ConstantMerge.cpp \
  DeadArgumentElimination.cpp \
  ExtractGV.cpp \
  FunctionAttrs.cpp \
  GlobalDCE.cpp \
  GlobalOpt.cpp \
  InlineAlways.cpp \
  Inliner.cpp \
  InlineSimple.cpp \
  Internalize.cpp \
  IPConstantPropagation.cpp \
  IPO.cpp \
  LoopExtractor.cpp \
  MergeFunctions.cpp \
  PartialInlining.cpp \
  PassManagerBuilder.cpp \
  PruneEH.cpp \
  StripDeadPrototypes.cpp \
  StripSymbols.cpp \
  AMDRemoveNoalias.cpp

# For the host
# =====================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(transforms_ipo_SRC_FILES)
LOCAL_MODULE:= AMDlibLLVMipo

LOCAL_MODULE_TAGS := optional

include $(LLVM_HOST_BUILD_MK)
include $(LLVM_GEN_INTRINSICS_MK)
include $(BUILD_HOST_STATIC_LIBRARY)

# For the device
# =====================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(transforms_ipo_SRC_FILES)
LOCAL_MODULE:= AMDlibLLVMipo

LOCAL_MODULE_TAGS := optional

include $(LLVM_DEVICE_BUILD_MK)
include $(LLVM_GEN_INTRINSICS_MK)
include $(BUILD_STATIC_LIBRARY)
