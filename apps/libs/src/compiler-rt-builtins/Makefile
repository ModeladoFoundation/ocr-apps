MAKE := make

ifndef ARCH
	$(error Need to specify ARCH!)
endif

all:
	@$(MAKE) -f Makefile.$(ARCH) $@

debug:
	@$(MAKE) -f Makefile.$(ARCH) $@

install:
	@$(MAKE) -f Makefile.$(ARCH) $@

run:
	@$(MAKE) -f Makefile.$(ARCH) $@

clean:
	@$(MAKE) -f Makefile.$(ARCH) $@

uninstall:
	@$(MAKE) -f Makefile.$(ARCH) $@
