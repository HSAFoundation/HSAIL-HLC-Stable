LOCAL_PATH:= $(call my-dir)

libhsail_SRC_FILES := \
	HSAILBrigantine.cpp \
	HSAILBrigContainer.cpp \
	HSAILBrigObjectFile.cpp \
	HSAILDisassembler.cpp \
	HSAILDump.cpp \
	HSAILFloats.cpp \
	HSAILItems.cpp \
	HSAILParser.cpp \
	HSAILScanner.cpp \
	HSAILScannerRules.cpp \
	HSAILUtilities.cpp \
	HSAILValidatorBase.cpp \
	HSAILValidator.cpp

LOCAL_CFLAGS :=	\
	$(LOCAL_CFLAGS) \
	-Wall \
	-Wno-unused \
	-fms-extensions \
	-DLINUX \
	-march=i686 \
	-D__i386__   \
	-gdwarf-2 \
	-Werror \
	-msse2 \
	-pthread \
	-fPIC \
	-DPIC \
	-pipe \
	-ffast-math \
	-fno-finite-math-only \
	-fno-math-errno \
	-fmerge-all-constants \
	-DUNIX_OS \
	-DDEBUG \
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
	-fexceptions

# For the host
# =====================================================
include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE:= LIBHSAIL
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(libhsail_SRC_FILES)
intermediates := $(call local-intermediates-dir)

include $(LLVM_HOST_BUILD_MK)
include $(LLVM_GEN_INTRINSICS_MK)
include $(BUILD_HOST_STATIC_LIBRARY)
DK := $(DK_ROOT)
COMPILER := $(LLVM_ROOT_PATH)/..
HOSTSTATIC := $(TOP)/out/host/linux-x86/obj/STATIC_LIBRARIES
PERLLIB := $(DK_ROOT)/perl/5.16.1/lib

$(addprefix $(intermediates)/, HSAILBrigantine.o):$(addprefix $(intermediates)/,HSAILTemplateUtilities_gen.hpp)
$(addprefix $(intermediates)/,HSAILTemplateUtilities_gen.hpp):$(COMPILER)/hsail-tools/libHSAIL/generate.pl $(COMPILER)/hsail-tools/libHSAIL/HDLProcessor.pl
	$(DK)/perl/5.16.1/bin/perl -I "$(PERLLIB)" $(COMPILER)/hsail-tools/libHSAIL/generate.pl --dk=$(DK) $(COMPILER)/hsail-tools/libHSAIL $(HOSTSTATIC)/LIBHSAIL_intermediates
	#$(DK)/perl/5.16.1/bin/perl-static -I "$(PERLLIB)" $(COMPILER)/hsail-tools/libHSAIL/HDLProcessor.pl -target=validator <$(COMPILER)/hsail-tools/libHSAIL/HSAILBrigInstr.hdl >$(HOSTSTATIC)/libloader_intermediates/HSAILInstValidation_gen.hpp.tmp



