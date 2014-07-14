LLVM_DEPTH = ..

include $(LLVM_DEPTH)/llvmdefs

SUBDIRS = \
    opt \
    llvm-link \
    llc \
    llvm-as \
    llvm-dis \
    llvm-extract \
    llvm-ar \
    llvm-ranlib \
    llvm-nm 
#jgolds
#	llvm-split

ifeq ($(BUILD_HSA_TARGET),yes)
     SUBDIRS+= llvm-dwarfdump inflate
else
  ifeq ($(call gte,$(OPENCL_VERSION),200),$(true))
     SUBDIRS+= llvm-dwarfdump inflate
  endif
endif
include $(LLVM_DEPTH)/llvmrules
