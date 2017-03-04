#
# Makefile for running TGR-CE tests under fsim
# See the README for more details
#
TG_REPO ?= $(realpath $(CURDIR)/../../../../../../../tg)
APPS_REPO ?= $(realpath $(CURDIR)/../../../../../..)
#
# TOOLS_REPO allows you to specify a different repo for the XE clang
# compiler
#
TOOLS_REPO ?= $(TG_REPO)

TG_ROOT   ?= $(TG_REPO)/tg
TG_INSTALL ?= $(TG_ROOT)/install

APPS_INSTALL = $(APPS_REPO)/apps/libs/install/tg-xe
APPS_INCLUDE = $(APPS_INSTALL)/include

TOOLS_ROOT ?= $(TOOLS_REPO)/tg
TOOLS_INSTALL = $(TOOLS_ROOT)/install
TOOLS_BIN = $(TOOLS_INSTALL)/bin

TGR_ROOT = $(TG_ROOT)/tg/baremetal
TGR_BUILD = ../../build-tg-ce

FSIM = $(TG_INSTALL)/bin/fsim
XE_GDB = $(TOOLS_BIN)/xstg-linux-elf-gdb

FSIM_ARGS = -s -c fsim.cfg -L rd
#
# Define GDB to run fsim under gdb
#
ifdef GDB
FSIM_GDB = gdb --args
FSIM_ARGS += -E 12345 -R localhost
endif

ifdef PIE
PIE_SUFFIX = .p
endif

TARGETS = $(patsubst %.c,%,$(wildcard c_test_*c))
all: $(TARGETS)
default: all

#
# Build an XE test app
#
XE_CFLAGS = -g -nodefaultlibs -fxstg-tgr
XE_STATIC_CFLAGS = -static $(XE_CFLAGS)
XE_PIE_CFLAGS = -fPIE $(XE_CFLAGS)

# Adding FORCE as a dependency of c_test_% to make it behave phony. This is needed
# because .PHONY doesn't work on implicit targets.
.PHONY: FORCE
FORCE:

#
# Build both static and PIE versions of single file tests
#
c_test_% : export APPS_LIBS_INSTALL = $(APPS_INSTALL)
c_test_% : c_test_%.c FORCE
	@echo " - $@"
	@$(TOOLS_BIN)/clang -o $@.swtest $(XE_STATIC_CFLAGS) -fxstg-swtest $< $(XE_LIBS)
	@$(TOOLS_BIN)/clang -o $@.fsim $(XE_STATIC_CFLAGS) $< $(XE_LIBS)
	@$(TOOLS_BIN)/clang -o $@.p $(XE_PIE_CFLAGS) $< $(XE_PIE_LIBS)

#
# Run a test on FSIM using its tgkrnl
#
%-run %-rungdb : export TGKRNL = ../../build-tg-ce/build.tgkrnl/tgkrnl.dbg
%-run %-rungdb : export TEST_ARGS = $(ARGS)
%-run : export TEST = $(subst -run,,$@)$(PIE_SUFFIX)
%-rungdb : export TEST = $(subst -rungdb,,$@)

%-run : %
	@echo "Running $^ with args '$$TEST_ARGS'"
	@echo "TGKRNL = $$TGKRNL"
	@echo "TEST = $$TEST"
	@mkdir -p rd
	$(FSIM_GDB) $(FSIM) $(FSIM_ARGS)
#
# Run a test under xe-gdb (running fsim)
#
%-rungdb : %
	@mkdir -p rd
	$(XE_GDB) $(TEST) -ex "target xstg -f fsim -c fsim.cfg -l rd"

TESTS = c_test_*.swtest c_test_*.fsim c_test_*.p

.PHONY: clean

clean :
	@rm -rf *.o *.s $(TESTS)
