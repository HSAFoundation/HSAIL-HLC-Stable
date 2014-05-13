LLVM_DEPTH = ../..

include $(LLVM_DEPTH)/llvmdefs

ifneq ($(BUILD_GPU_DEVICE),no)
SUBDIRS = AMDIL
endif

ifneq ($(BUILD_HSA_DEVICE),no)
SUBDIRS += HSAIL
endif

ifneq ($(BUILD_CPU_DEVICE),no)
SUBDIRS += ARM X86
endif

SUBDIRS += build

include $(LLVM_DEPTH)/llvmrules
