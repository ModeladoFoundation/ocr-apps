include Makefile.common

SUBDIRS = test

.PHONY: default all clean tidy
.PHONY: $(SUBDIRS) src

default: all

doc:
	doxygen

src:
	$(MAKE) -C src $(MAKECMDGOALS)

$(SUBDIRS): src
	$(MAKE) -C $@ $(MAKECMDGOALS)

all tidy clean: $(SUBDIRS)
