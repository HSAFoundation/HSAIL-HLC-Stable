


usage:
	@echo "Usage: make -f Makefile <target>                "
	@echo "       where target is one of                   "
	@echo "       w7, w764a, lnx, lnx64a                   "
	@echo "       rel.w7, rel.w764a, rel.lnx, rel.lnx64a   "

w7:
	$(MAKE) -f Makefile.win TARGET=$@ FLAVOUR=DEBUG     ARCH=i386

w764a:
	$(MAKE) -f Makefile.win TARGET=w7 FLAVOUR=DEBUG     ARCH=i386

rel.w7:
	$(MAKE) -f Makefile.win TARGET=$@ FLAVOUR=RELEASE   ARCH=i386

rel.w764a:
	$(MAKE) -f Makefile.win TARGET=rel.w7 FLAVOUR=RELEASE   ARCH=i386

lnx:
	$(MAKE) -f Makefile.lnx TARGET=$@ FLAVOUR=DEBUG     ARCH=i386

lnx64a:
	$(MAKE) -f Makefile.lnx TARGET=lnx FLAVOUR=DEBUG     ARCH=i366

rel.lnx:
	$(MAKE) -f Makefile.lnx TARGET=$@ FLAVOUR=RELEASE   ARCH=i386

rel.lnx64a:
	$(MAKE) -f Makefile.lnx TARGET=rel.lnx FLAVOUR=RELEASE   ARCH=i386





all.w:
	$(MAKE) -f Makefile w7
	$(MAKE) -f Makefile w764a
	$(MAKE) -f Makefile rel.w7
	$(MAKE) -f Makefile rel.w764a

all.l:
	$(MAKE) -f Makefile lnx
	$(MAKE) -f Makefile lnx64a
	$(MAKE) -f Makefile rel.lnx
	$(MAKE) -f Makefile rel.lnx64a
