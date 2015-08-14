#
# newlib Makefile
#
# (c) 2010, Intel
#
# Authors:
# 		Justin Teller, Intel
#		EQware various
#
#	Variables - overrideable via cmd line or env. Defaults below.
#		TG_INSTALL - top of the intel repo install root. Used to
#					 find clang/llvm/binutils apps and includes.
#		APPS_ROOT  - path to the apps directory in the xstack repo
#					 Used to generate the path to the xstack repo
#					 arch specific install root to install libs and
#					 includes to.
#		ARCH       - Architecture to build for. Required, no default.
#					 Used to determine compiler/toolchain to use and
#					 CPU arch to compile for. Also used to generate
#				     build product directory name.
#
# Note that newlib is configured and built in BUILD_DIR (build_$(ARCH))
# and it installs to an intermediate install root NEWLIB_INSTALL (tmproot).
# This makefile then installs the build products from tmproot to the
# libs install root APPS_LIBS_ROOT (xstack/apps/libs/$(ARCH)).
#
TG_INSTALL ?= $(realpath $(CURDIR)/../../../../../intel/ss/install)

APPS_ROOT  ?= $(realpath $(CURDIR)/../../..)

#
# Supported ARCH values
#
ARCHES = tg x86 x86_native

ifndef ARCH
$(error Need to specify ARCH! use one of: $(ARCHES))
endif

ifeq (,$(findstring $(ARCH),$(ARCHES)))
$(error Unknown/unsupported  ARCH ($(ARCH))! use one of: $(ARCHES))
endif

APPS_LIBS_ROOT ?= $(APPS_ROOT)/libs/$(ARCH)

NEWLIB_BUILD   = $(CURDIR)
NEWLIB_INSTALL = $(NEWLIB_BUILD)/tmproot
NEWLIB_SRCDIR  = $(CURDIR)/newlib-2.1.0

TARGET = "xstg-linux-elf"

#
# per-target build dir
#
BUILD_DIR = build_$(ARCH)

NEWLIB   = $(NEWLIB_INSTALL)/$(TARGET)/lib/libc.a
CONFIG_ARGS ?= --target=$(TARGET) --prefix=$(NEWLIB_INSTALL) --with-newlib

#
# common cflags
#
CFLAGS ?= -Os -mstackrealign -fno-stack-protector -I$(NEWLIB_SRCDIR)/newlib/libc/include $(DEFINES)
#CFLAGS += -I$(TG_SRCDIR)/xe-sim/include

#
# Environment variables for newlib configure
#
CFLAGS_FOR_TARGET ?= $(CFLAGS)
CC_FOR_TARGET  ?= $(TOOL_PREFIX)$(CC)
LD_FOR_TARGET  ?= $(TOOL_PREFIX)ld
CXX_FOR_TARGET ?= $(TOOL_PREFIX)$(CXX)
AR_FOR_TARGET  ?= $(TOOL_PREFIX)ar
AS_FOR_TARGET  ?= $(TOOL_PREFIX)as
NM_FOR_TARGET  ?= $(TOOL_PREFIX)nm
CPP_FOR_TARGET ?= $(TOOL_PREFIX)$(CPP)
RANLIB_FOR_TARGET ?= $(TOOL_PREFIX)ranlib
READELF_FOR_TARGET ?= $(TOOL_PREFIX)readelf

export CFLAGS CONFIG_ARGS
export CFLAGS_FOR_TARGET CC_FOR_TARGET CXX_FOR_TARGET CPP_FOR_TARGET
export AS_FOR_TARGET LD_FOR_TARGET NM_FOR_TARGET AR_FOR_TARGET
export RANLIB_FOR_TARGET READELF_FOR_TARGET

CFLAGS_FOR_BUILD = -g
export CFLAGS_FOR_BUILD

##########################
# Make Targets
#

all: $(ARCH)
	@echo Done making all
#
# XE Clang/llvm build
#
ifeq ($(ARCH), tg)
CC = clang
CXX = clang++
TARGET = xstg-linux-elf
TOOLBIN = $(TG_INSTALL)/bin
TOOL_PREFIX = $(TOOLBIN)/$(TARGET)-
endif
#

# This defines an x86_64 build using the XE compiler
#
ifeq ($(ARCH), x86)
CC = clang
CXX = clang++
TARGET = x86_64-linux-elf
TOOLBIN = $(TG_INSTALL)/bin
TOOL_PREFIX = $(TOOLBIN)/$(TARGET)-
endif
#
# This defines a native/platform GCC build
# C99 is required to get the 'restrict' keyword
# But C99 turns on strict ansi, which we disable
# Make sure we get just the default platform build
#
ifeq ($(ARCH), x86_native)
CFLAGS += -std=c99 -U__STRICT_ANSI__
CONFIG_ARGS += --enable-multilib=no
TARGET = x86_64-linux-elf
endif

$(ARCHES): show_args
	$(MAKE) TARGET=$(TARGET) newlib
#
# Debug assist
#
show_args:
	@echo APPS_ROOT = $(APPS_ROOT)
	@echo ARCH = $(ARCH)
	@echo TARGET = $(TARGET)
	@echo CFLAGS_FOR_TARGET = $(CFLAGS)
	@echo CC_FOR_TARGET =  $(CC_FOR_TARGET)
	@echo LD_FOR_TARGET =  $(LD_FOR_TARGET)
	@echo CXX_FOR_TARGET =  $(CXX_FOR_TARGET)
	@echo AR_FOR_TARGET =  $(AR_FOR_TARGET)
	@echo AS_FOR_TARGET =  $(AS_FOR_TARGET)
	@echo NM_FOR_TARGET =  $(NM_FOR_TARGET)
	@echo CPP_FOR_TARGET =  $(CPP_FOR_TARGET)
	@echo RANLIB_FOR_TARGET =  $(RANLIB_FOR_TARGET)
	@echo READELF_FOR_TARGET =  $(READELF_FOR_TARGET)
	@echo CONFIG_ARGS =  $(CONFIG_ARGS)

# Modify this target when newlib supports debug and non-debug builds
debug: default

default: all

# Always try to build these when we get here
.PHONY: $(NEWLIB) clean newlib tg_install show_args

$(NEWLIB_BUILD):
	@[ -d $(NEWLIB_BUILD) ] || mkdir -p $(NEWLIB_BUILD)
#
# Configure and build newlib
#
newlib: show_args $(NEWLIB) newlib_install

newlib-config $(BUILD_DIR)/config.log:
	@echo Configuring Newlib ...
	@rm -rf $(BUILD_DIR); mkdir $(BUILD_DIR);
	@cd $(BUILD_DIR); $(NEWLIB_SRCDIR)/configure $(CONFIG_ARGS)

$(NEWLIB): $(BUILD_DIR)/config.log
	@echo Building Newlib ...
	@$(MAKE) -C $(BUILD_DIR) all
	@echo Successfully built newlib!
#
# we let newlib install to a tmp root and then copy over the
# includes and libs to the official root
#
newlib_install: $(NEWLIB)
	@echo Installing Newlib to tmproot ...
	@rm -rf $(NEWLIB_INSTALL)/$(TARGET)
	@mkdir -p $(NEWLIB_INSTALL)
	@$(MAKE) -C $(NEWLIB_BUILD)/$(BUILD_DIR) install
#
# Copy from the tmp install tree to the actual install tree
#
.PHONY: install
install: root_install
root_install: newlib_install
	@echo Installing Newlib to $(APPS_LIBS_ROOT) ...
	@mkdir -p $(APPS_LIBS_ROOT)
	@cp -u -R $(NEWLIB_INSTALL)/$(TARGET)/include $(APPS_LIBS_ROOT)/
	@cp -u -R $(NEWLIB_INSTALL)/$(TARGET)/lib $(APPS_LIBS_ROOT)/
	@echo Installation complete

.PHONY: uninstall
uninstall: # Doesn't actually do anything.
	@echo Uninstall is not implemented for this library

clean:
	@-if test -d $(NEWLIB_BUILD) ; then          \
	    cd $(NEWLIB_BUILD);                     \
	    [ -d $(BUILD_DIR) ] && $(MAKE) -C $(BUILD_DIR) -ik clean;  \
	    rm -rf $(BUILD_DIR); \
	 fi
	@rm -rf $(NEWLIB_INSTALL)
