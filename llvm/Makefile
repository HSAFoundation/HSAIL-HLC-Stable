LLVM_DEPTH = .

include $(LLVM_DEPTH)/llvmdefs

SUBDIRS = lib/Support lib/TableGen utils/TableGen lib/VMCore lib tools

ifeq ($(BUILD_TESTS), 1)
    #SUBDIRS += utils/count utils/FileCheck utils/not utils/llvm-lit unittests
endif

include llvmrules
