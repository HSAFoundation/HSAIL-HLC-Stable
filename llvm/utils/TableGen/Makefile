LLVM_DEPTH = ../..

include $(LLVM_DEPTH)/llvmdefs

SUBDIRS = build

ifdef ATI_OS_LINUX
    GCXXOPTS := $(filter-out -fno-exceptions,$(GCXXOPTS))
endif

include $(LLVM_DEPTH)/llvmrules
