#
# Master Makefile for "other" regression tests
#
# This will build the tg tree with all of the fsim variants, the
# apps tg-xe libs, and all the examples in the tg and apps trees.
#
# This should be run before attempting to execute any of the
# regression tests. It should be run with no target (or you may use
# "make all") to build everything.
#

TG_ROOT ?= $(realpath $(CURDIR)/../../../../tg/tg)
APPS_ROOT ?= $(realpath $(CURDIR)/../..)

all: tg-examples apps-legacy apps-examples

tg-tree:
	$(MAKE) -C $(TG_ROOT)
	$(MAKE) -C $(TG_ROOT)/build/build.fsim -f make-me.make install-fsim-rtlsim
	$(MAKE) -C $(TG_ROOT)/build/build.fsim -f make-me.make install-fsim-swtest

apps-libs: tg-tree
	$(MAKE) -C $(APPS_ROOT)/libs/src -f Makefile.core ARCH=tg-xe

tg-examples: apps-libs
	$(MAKE) -C $(TG_ROOT)/fsim/swtest DEBUG=1 cdemo_nonewlib
	$(MAKE) -C $(TG_ROOT)/fsim/swtest DEBUG=1 cdemo
	$(MAKE) -C $(TG_ROOT)/fsim/swtest DEBUG=1 cxxdemo
	$(MAKE) -C $(TG_ROOT)/fsim/swtest DEBUG=1 throw
	$(MAKE) -C $(TG_ROOT)/fsim/tests/rtl-asm
	$(MAKE) -C $(TG_ROOT)/fsim/tests/rtl-asm2
	$(MAKE) -C $(TG_ROOT)/fsim/tests/rtl-asm8
	$(MAKE) -C $(TG_ROOT)/xe-llvm/test/PIE fptr_simple
	$(MAKE) -C $(TG_ROOT)/xe-llvm/test/PIE multi_seg

apps-legacy: apps-libs
	$(MAKE) -C $(APPS_ROOT)/legacy/tg-xe hello
	$(MAKE) -C $(APPS_ROOT)/legacy/tg-xe cxxhello
	$(MAKE) -C $(APPS_ROOT)/legacy/tg-xe iotest

apps-examples: apps-libs
	$(MAKE) -C $(APPS_ROOT)/examples/printf -f Makefile.tg install
	$(MAKE) -C $(APPS_ROOT)/examples/fib -f Makefile.tg install
