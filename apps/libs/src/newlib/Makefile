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
#		TG_INSTALL - top of the tg repo install root. Used to
#					 find clang/llvm/binutils apps and includes.
#		APPS_ROOT  - path to the apps directory in the apps repo (apps/apps).
#					 Used to generate the path to the apps repo
#					 arch specific install root to install libs and
#					 includes to.
#		ARCH       - Architecture to build for. Required, no default.
#					 Used to determine compiler/toolchain to use and
#					 CPU arch to compile for. Also used to generate
#				     build product directory name.
#		USE_TGR    - Whether or not to build newlib to use the tg runtime as its
#					 backend. Defaults to no (use OCR instead). Set to
#					 yes to make it build for tg runtime
#
# Note that newlib is configured and built in BUILD_DIR (build_$(ARCH))
# and it installs to an intermediate install root NEWLIB_INSTALL (tmproot).
# This makefile then installs the build products from tmproot to the
# libs install root APPS_LIBS_INSTALL (apps/apps/libs/install/$(ARCH)).
#
TG_ROOT ?= $(realpath $(CURDIR)/../../../../../tg/tg)
TG_INSTALL ?= $(TG_ROOT)/install
APPS_ROOT ?= $(realpath $(CURDIR)/../../..)
APPS_LIBS_INSTALL_ROOT ?= $(APPS_ROOT)/libs/install
APPS_LIBS_INSTALL ?= $(APPS_LIBS_INSTALL_ROOT)/$(ARCH)

#
# Supported ARCH values
#
X86_ARCHES = x86 x86-test
ARCHES = tg-xe $(X86_ARCHES)

ifndef ARCH
$(error Need to specify ARCH! use one of: $(ARCHES))
endif

ifeq ($(USE_TGR), yes)
    SYS = tgr
else
    SYS = ocr
endif

ifeq (,$(findstring $(ARCH),$(ARCHES)))
$(error Unknown/unsupported  ARCH ($(ARCH))! use one of: $(ARCHES))
endif
#
# per-target build and local install dirs
#
BUILD_PREF = build-
BUILD_DIR = $(BUILD_PREF)$(SYS)-$(ARCH)$(BTYPE)

TMP_PREF = tmproot-
TMP_ROOT   = $(TMP_PREF)$(SYS)-$(ARCH)$(BTYPE)

NEWLIB_SRCDIR  = $(CURDIR)/newlib-2.1.0
NEWLIB_BUILD   = $(CURDIR)
NEWLIB_TMPROOT = $(NEWLIB_BUILD)/$(TMP_ROOT)
NEWLIB_INSTALL = $(NEWLIB_TMPROOT)/$(TARGET)

NEWLIB   = $(NEWLIB_INSTALL)/lib/libc.a

#
# common cflags
#
CFLAGS = -Os -mstackrealign -fno-stack-protector -I$(NEWLIB_SRCDIR)/newlib/libc/include $(DEFINES)
CONFIG_ARGS = --target=$(TARGET) --prefix=$(NEWLIB_TMPROOT) --with-newlib

# XE Clang/llvm build
#
ifeq ($(ARCH), tg-xe)
  CC = clang
  CXX = clang++
  CFLAGS_FOR_TARGET = -mllvm -set-global-default=1
  CONFIG_ARGS += --enable-libgloss=no
  TARGET = xstg-linux-elf
  TOOLBIN = $(TG_INSTALL)/bin
  TOOL_PREFIX = $(TOOLBIN)/$(TARGET)-
  BTYPE = -static
  export APPS_LIBS_INSTALL
endif

ifdef PIE
  ifneq ($(ARCH), tg-xe)
    $(error PIE is only valid for ARCH tg-xe)
  endif
  BTYPE = -pie
  CFLAGS += -fPIE
  # XXX donn-devel TOOLBIN = /build/eqware/pie/install/bin
endif
#
# This defines an x86_64 build using the XE compiler
#
ifeq ($(ARCH), x86-test)
  CC = clang
  CXX = clang++
  TARGET = x86_64-linux-elf
  TOOLBIN = $(TG_INSTALL)/bin
  TOOL_PREFIX = $(TOOLBIN)/$(TARGET)-
  export APPS_LIBS_INSTALL
endif
#
# This defines a native/platform GCC build
# C99 is required to get the 'restrict' keyword
# But C99 turns on strict ansi, which we disable
# Make sure we get just the default platform build
#
ifeq ($(ARCH), x86)
  CFLAGS += -std=c99 -U__STRICT_ANSI__
  CONFIG_ARGS += --enable-multilib=no
  TARGET = x86_64-linux-elf
endif

#
# Environment variables for newlib configure
#
CFLAGS_FOR_TARGET := $(CFLAGS) $(CFLAGS_FOR_TARGET)
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
default: all

all: $(ARCH)
	@echo Done making all
#
# ARCH specific targets
#
tg-xe-pie:
	$(MAKE) PIE=1 install

tg-xe-static:
	$(MAKE) install

tg-xe: show_args tg-xe-pie tg-xe-static

$(X86_ARCHES): show_args
	$(MAKE) install
#
# Debug assist
#
show_args:
	@echo APPS_ROOT = $(APPS_ROOT)
	@echo ARCH = $(ARCH)
	@echo TARGET = $(TARGET)
	@echo CFLAGS = $(CFLAGS)
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
	@echo BUILDDIR =  $(BUILD_DIR)
	@echo NEWLIB_INSTALL =  $(NEWLIB_INSTALL)

# Modify this target when newlib supports debug and non-debug builds
debug: default

# Always try to build these when we get here
.PHONY: $(NEWLIB) clean newlib show_args

$(NEWLIB_BUILD):
	@[ -d $(NEWLIB_BUILD) ] || mkdir -p $(NEWLIB_BUILD)
#
# Configure and build newlib, installing to tmproot
#
newlib: show_args newlib_install

newlib-config $(BUILD_DIR)/config.log:
	@echo Configuring Newlib ...
	@rm -rf $(BUILD_DIR); mkdir $(BUILD_DIR);
	cd $(BUILD_DIR); $(NEWLIB_SRCDIR)/configure $(CONFIG_ARGS)

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
	@rm -rf $(NEWLIB_INSTALL)
	@mkdir -p $(NEWLIB_INSTALL)
	@$(MAKE) -C $(NEWLIB_BUILD)/$(BUILD_DIR) install
#
# Copy from the tmp install tree to the actual install tree
# For PIE builds we need to rename the files in lib as we copy them
# inserting a '.p' before the file suffix.
#
root_install: newlib_install
	@echo Installing Newlib to $(APPS_LIBS_INSTALL) ...
	@mkdir -p $(APPS_LIBS_INSTALL)/lib
	@cp -n -u -R $(NEWLIB_INSTALL)/include $(APPS_LIBS_INSTALL)/
	@if [ -z $(PIE) ] ; then \
	   cp -n -u -R $(NEWLIB_INSTALL)/lib $(APPS_LIBS_INSTALL)/ ; \
	 else \
	   for f in $(NEWLIB_INSTALL)/lib/*.[oa] ; do \
	     fp=`basename $$f | sed -e 's/\(..\)$$/.p\1/'` ;        \
	     cp -u $$f $(APPS_LIBS_INSTALL)/lib/$$fp ; \
	   done ; \
	 fi
	@echo Installation complete

.PHONY: install
install: show_args root_install

.PHONY: uninstall
uninstall: # Doesn't actually do anything.
	@echo Uninstall is not implemented for this library

clean:
	@-if test -d $(NEWLIB_BUILD) ; then          \
	    rm -rf $(BUILD_PREF)$(ARCH)*; \
		rm -rf $(NEWLIB_BUILD)/$(TMP_PREF)$(ARCH)* ; \
	 fi
