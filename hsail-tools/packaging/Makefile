usage:
	@echo "Usage: $(MAKE) -f Makefile <target>                "
	@echo "       where target is one of                   "
	@echo "       w7, w764a, lnx, lnx64a                   "
	@echo "       rel.w7, rel.w764a, rel.lnx, rel.lnx64a   "


ifeq ("$(_PLATFORM)","WIN")
MAKEFILE = Makefile.win
else
MAKEFILE = Makefile.lnx
endif
SUBMAKE_OPTS = -f $(MAKEFILE) TARGET=$(_TARGET) FLAVOUR=$(_FLAVOUR) ARCH=$(_ARCH) PLATFORM=$(_PLATFORM) all

ifeq ("$(_TABLEGEN)","SKIP")
tablegen:
	@echo Skipping tablegen in a 64-bit build

else
tablegen: llvm
	$(MAKE) -C llvm/TableGen $(SUBMAKE_OPTS)

endif

llvm:
	$(MAKE) -C llvm $(SUBMAKE_OPTS)


HSAILAsm: libHSAIL llvm
	$(MAKE) -C HSAILAsm $(SUBMAKE_OPTS)


finalizer: libHSAIL llvm
	$(MAKE) -C finalizer $(SUBMAKE_OPTS)


libHSAIL: tablegen
	$(MAKE) -C libHSAIL $(SUBMAKE_OPTS)


all_impl: finalizer HSAILAsm


w7:
	$(MAKE) -f Makefile all_impl _FLAVOUR=DEBUG _PLATFORM=WIN _ARCH=i386 _TARGET=$@

w764a:
	$(MAKE) -f Makefile all_impl _FLAVOUR=DEBUG _PLATFORM=WIN _ARCH=x64  _TARGET=$@ _TABLEGEN=SKIP

rel.w7:
	$(MAKE) -f Makefile all_impl _FLAVOUR=DEBUG _PLATFORM=WIN _ARCH=i386 _TARGET=$@

rel.w764a:
	$(MAKE) -f Makefile all_impl _FLAVOUR=DEBUG _PLATFORM=WIN _ARCH=x64 _TARGET=$@ _TABLEGEN=SKIP

lnx:
	$(MAKE) -f Makefile all_impl _FLAVOUR=DEBUG _PLATFORM=LNX _ARCH=i386 _TARGET=$@

lnx64a:
	$(MAKE) -f Makefile all_impl _FLAVOUR=DEBUG _PLATFORM=LNX _ARCH=x64  _TARGET=$@ _TABLEGEN=SKIP

rel.lnx:
	$(MAKE) -f Makefile all_impl _FLAVOUR=DEBUG _PLATFORM=LNX _ARCH=i386 _TARGET=$@

rel.lnx64a:
	$(MAKE) -f Makefile all_impl _FLAVOUR=DEBUG _PLATFORM=LNX _ARCH=x64 _TARGET=$@ _TABLEGEN=SKIP

clean:
	rm -rf build/


.PHONY: w7 w764a rel.w7 rel.w64a lnx lnx64a rel.lnx all_impl usage finalizer HSAILAsm libHSAIL llvm tablegen
