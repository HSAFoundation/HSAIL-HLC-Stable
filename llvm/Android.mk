LOCAL_PATH := $(call my-dir)
LLVM_ROOT_PATH := $(LOCAL_PATH)
LLVM_ENABLE_ASSERTION := false

include $(CLEAR_VARS)

# LLVM Libraries
subdirs := \
  lib/Analysis \
  lib/Analysis/IPA \
  lib/AsmParser \
  lib/Bitcode/Reader \
  lib/Bitcode/Writer \
  lib/CodeGen \
  lib/CodeGen/AsmPrinter \
  lib/CodeGen/SelectionDAG \
  lib/IR \
  lib/IRReader \
  lib/Linker \
  lib/MC \
  lib/MC/MCDisassembler \
  lib/MC/MCParser \
  lib/Object \
  lib/Option \
  lib/Support \
  lib/VMCore \
  lib/TableGen \
  lib/Target \
  lib/Transforms/IPO \
  lib/Transforms/InstCombine \
  lib/Transforms/Instrumentation \
  lib/Transforms/ObjCARC \
  lib/Transforms/Scalar \
  lib/Transforms/Utils \
  lib/Transforms/Vectorize \
  lib/Transforms/SPIR \
  utils/FileCheck \
  utils/TableGen \
  lib/Archive \
  lib/Transforms/OpenCLIR \
  lib/Target/HSAIL \
  lib/ExecutionEngine/MCJIT \
  lib/ExecutionEngine/Interpreter \
  lib/ExecutionEngine/JIT \
  lib/ExecutionEngine/RuntimeDyld \
  lib/ExecutionEngine



# ARM Code Generation Libraries
subdirs += \
  lib/Target/ARM \
  lib/Target/ARM/AsmParser \
  lib/Target/ARM/InstPrinter \
  lib/Target/ARM/Disassembler \
  lib/Target/ARM/MCTargetDesc \
  lib/Target/ARM/TargetInfo

# MIPS Code Generation Libraries
subdirs += \
  lib/Target/Mips \
  lib/Target/Mips/AsmParser \
  lib/Target/Mips/InstPrinter \
  lib/Target/Mips/Disassembler \
  lib/Target/Mips/MCTargetDesc \
  lib/Target/Mips/TargetInfo

# X86 Code Generation Libraries
subdirs += \
  lib/Target/X86 \
  lib/Target/X86/AsmParser \
  lib/Target/X86/InstPrinter \
  lib/Target/X86/Disassembler \
  lib/Target/X86/MCTargetDesc \
  lib/Target/X86/TargetInfo \
  lib/Target/X86/Utils

# AMDIL
subdirs += \
  lib/Target/AMDIL/MDParser
#  lib/Target/AMDIL \
#  lib/Target/AMDIL/Apple \
#  lib/Target/AMDIL/Internal \
#  lib/Target/AMDIL/macrodb \
#  lib/Target/AMDIL/MCTargetDesc \
#  lib/Target/AMDIL/MDParser \
#  lib/Target/AMDIL/TargetInfo \

# LLVM Command Line Tools
#subdirs += \
	tools/llvm-link \

#	tools/opt

#	../hsail-tools/libHSAIL \
#	tools/llc \
	tools/llvm-as \
	tools/llvm-dis \
	tools/opt

#Todd_add-begin-----------------------------------------------------
exclude := \
  lib/IR \
  lib/IRReader \
  lib/Option \
  lib/Transforms/ObjCARC \
  lib/Target/Mips \
  lib/Target/Mips/AsmParser \
  lib/Target/Mips/InstPrinter \
  lib/Target/Mips/Disassembler \
  lib/Target/Mips/MCTargetDesc \
  lib/Target/Mips/TargetInfo

subdirs := $(filter-out $(exclude), $(subdirs))
#subdirs += lib/VMCore
#Todd_add-end-------------------------------------------------------
include $(LOCAL_PATH)/llvm.mk
include $(LOCAL_PATH)/shared_llvm.mk
include $(addprefix $(LOCAL_PATH)/,$(addsuffix /Android.mk, $(subdirs)))
