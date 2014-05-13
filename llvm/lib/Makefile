LLVM_DEPTH = ..

include $(LLVM_DEPTH)/llvmdefs

SUBDIRS = \
    Analysis \
    Archive \
    AsmParser \
    Bitcode \
    CodeGen \
    DebugInfo \
    ExecutionEngine \
    Linker \
    MC \
    Object \
    Target \
    Transforms

# Note: lib subdirs Support, TableGen, and VMCore are built specifically
# by the ../Makefile. lib/VMCore requires utils/Tablegen, which requires
# lib/Support and lib/TableGen.

include $(LLVM_DEPTH)/llvmrules
