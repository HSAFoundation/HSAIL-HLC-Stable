LOCAL_CFLAGS :=	\
	-D_GNU_SOURCE	\
	-D__STDC_LIMIT_MACROS	\
	-D__STDC_CONSTANT_MACROS	\
	-D__STDC_FORMAT_MACROS	\
	-O2	\
	-fomit-frame-pointer	\
	-Wall	\
	-W	\
	-Wno-unused-parameter	\
	-Wwrite-strings	\
	$(LOCAL_CFLAGS)

# The three inline options together reduce libbcc.so almost 1MB.
# We move them from global build/core/combo/TARGET_linux-arm.mk
# to here.
LOCAL_CFLAGS := -DANDROID_TARGET_BUILD \
		-finline-limit=64 \
		-finline-functions \
		-fno-inline-functions-called-once \
		$(LOCAL_CFLAGS)

ifeq ($(TARGET_BUILD_VARIANT),eng)
LOCAL_CFLAGS := -DANDROID_ENGINEERING_BUILD \
                $(LOCAL_CFLAGS)
endif

ifeq ($(LLVM_ENABLE_ASSERTION),true)
LOCAL_CFLAGS :=	\
	$(LOCAL_CFLAGS) \
	-D_DEBUG	\
	-UNDEBUG
endif

ifneq ($(REQUIRES_EH),1)
LOCAL_CFLAGS +=	-fno-exceptions
else
# No action. The device target should not have exception enabled since bionic
# doesn't support it
REQUIRES_EH := 0
endif

ifneq ($(REQUIRES_RTTI),1)
#LOCAL_CPPFLAGS +=	-fno-rtti
else
REQUIRES_RTTI := 0
endif

LOCAL_CPPFLAGS :=	\
	$(LOCAL_CPPFLAGS)	\
	-Woverloaded-virtual	\
	-Wno-sign-promo	\
	-DANDROID
#from AMD tblgen
LOCAL_CPPFLAGS :=	\
	$(LOCAL_CPPFLAGS)	\
	-fms-extensions \
	-DLINUX \
	-march=i686 \
	-D__i386__ \
	-gdwarf-2 \
	-msse2 \
	-pthread \
	-DPIC \
	-pipe \
	-ffast-math \
	-fno-finite-math-only \
	-fno-math-errno \
	-fmerge-all-constants \
	-fno-tree-vectorize \
	-frtti \
	-DUNIX_OS \
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
	-D AMD_LLVM_INTERNAL \
	-D AMD_HSAIL
LOCAL_CPPFLAGS :=	\
	$(LOCAL_CPPFLAGS)	\
	-Wno-non-virtual-dtor \
	-Wno-overloaded-virtual \
	-Wno-return-type \
	-frtti \
	-fexceptions \


# Make sure bionic is first so we can include system headers.
LOCAL_C_INCLUDES :=	\
	bionic \
	external/stlport/stlport \
	$(LLVM_ROOT_PATH)	\
	$(LLVM_ROOT_PATH)/include	\
	$(LLVM_ROOT_PATH)/device/include	\
	$(LLVM_ROOT_PATH)/linux/include	\
	$(LOCAL_C_INCLUDES)

###########################################################
## Commands for running tblgen to compile a td file
###########################################################
define transform-device-td-to-out
@mkdir -p $(dir $@)
@echo "Device TableGen (gen-$(1)): $(TBLGEN_LOCAL_MODULE) <= $<"
$(hide) $(TBLGEN) \
	-I $(dir $<)	\
	-I $(LLVM_ROOT_PATH)/include	\
	-I $(LLVM_ROOT_PATH)/device/include	\
	-I $(LLVM_ROOT_PATH)/lib/Target	\
    -gen-$(strip $(1)) \
    -o $@ $<
endef
