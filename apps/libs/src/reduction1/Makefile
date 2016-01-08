# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.


MAKE := make

ifndef ARCH
$(error Need to specify ARCH!)
endif

all:
	@$(MAKE) -C $(ARCH) -f Makefile $@

debug:
	@$(MAKE) -C $(ARCH) -f Makefile $@

install:
	@$(MAKE) -C $(ARCH) -f Makefile $@

run:
	@$(MAKE) -C $(ARCH) -f Makefile $@

clean:
	@$(MAKE) -C $(ARCH) -f Makefile $@

squeeky:
	@$(MAKE) -C $(ARCH) -f Makefile $@

uninstall:
	@$(MAKE) -C $(ARCH) -f Makefile $@
