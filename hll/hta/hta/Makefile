include Makefile.common

SUBDIRS = src test

.PHONY: default all clean tidy
.PHONY: $(SUBDIRS) src

default: all

doc:
	doxygen

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

all tidy clean: $(SUBDIRS)
