LOCAL_PATH:= $(call my-dir)

vmcore_SRC_FILES := \
  AsmWriter.cpp \
  Attributes.cpp \
  AutoUpgrade.cpp \
  BasicBlock.cpp \
  ConstantFold.cpp \
  Constants.cpp \
  Core.cpp \
  DebugLoc.cpp \
  Dominators.cpp \
  Function.cpp \
  GCOV.cpp \
  Globals.cpp \
  GVMaterializer.cpp \
  InlineAsm.cpp \
  Instruction.cpp \
  Instructions.cpp \
  IntrinsicInst.cpp \
  IRBuilder.cpp \
  LeakDetector.cpp \
  LLVMContext.cpp \
  LLVMContextImpl.cpp \
  Metadata.cpp \
  Module.cpp \
  Pass.cpp \
  PassManager.cpp \
  PassRegistry.cpp \
  PrintModulePass.cpp \
  Type.cpp \
  Use.cpp \
  User.cpp \
  Value.cpp \
  ValueSymbolTable.cpp \
  ValueTypes.cpp \
  Verifier.cpp \
  DataLayout.cpp \
  DebugInfo.cpp \
  TypeFinder.cpp \
  DIBuilder.cpp \
  TargetTransformInfo.cpp

# For the host
# =====================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(vmcore_SRC_FILES)

LOCAL_MODULE:= AMDlibLLVMCore

LOCAL_MODULE_TAGS := optional

include $(LOCAL_PATH)/../../llvm-host-build.mk
include $(LLVM_GEN_INTRINSICS_MK)
include $(BUILD_HOST_STATIC_LIBRARY)

# For the device
# =====================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(vmcore_SRC_FILES)

LOCAL_MODULE:= AMDlibLLVMCore

LOCAL_MODULE_TAGS := optional

include $(LOCAL_PATH)/../../llvm-device-build.mk
include $(LLVM_GEN_INTRINSICS_MK)
include $(BUILD_STATIC_LIBRARY)
