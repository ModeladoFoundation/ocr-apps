#
# Makefile
# vim:syntax=make noexpandtab:
#
# (c) 2010, Intel
#
# Authors:
#		EQware various
#
# May use variables
# 	TG_ROOT:	Where the root (tg/tg) director of the tg tree is.
# 			(Default: `pwd`/../../../../../tg/tg)
#
# 	TG_INSTALL:	Where the install director of the tg tree is.
# 			(Default: $TG_ROOT/install)
#
# 	ARCH:		Which architecture to build for.
# 			May be one of: tg-xe, x86, x86-test
# 			(Required, No Default)
#
#	USE_TGR:	Whether or not to use the tg runtime.
#			If not using the tg runtime, then it defaults to
#			using the OCR runtime instead.
#			May be one of: yes, no
#			(Default: no)
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
$(error Need to specify ARCH! )
endif

ifeq (,$(findstring $(ARCH), $(ARCHES)))
$(error Unknown/unsupported  ARCH ($(ARCH)) ! )
endif

NEWLIB_INC = $(APPS_LIBS_INSTALL)/include

ifeq ($(USE_TGR), yes)
  SYS = tgr
else
  SYS = ocr
endif

BUILD_ROOT = build-
BUILD_DIR = $(BUILD_ROOT)$(SYS)-$(ARCH)$(BTYPE)

SRCDIR  = $(CURDIR)/$(SYS)/$(ARCH)
OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(patsubst %.S, $(BUILD_DIR)/%.o, $(SRCS)))
LIB = $(BUILD_DIR)/libscaffold.a
#
# XE Clang/llvm build
#
ifeq ($(ARCH), tg-xe)
  TARGET = xstg-linux-elf
  TOOLBIN = $(TG_INSTALL)/bin
  TOOL_PREFIX = $(TOOLBIN)/$(TARGET)-
  CC = $(TOOL_PREFIX)clang
  AS = $(TOOL_PREFIX)as
  AR = $(TOOL_PREFIX)ar
  LD = $(TOOL_PREFIX)ld
  RANLIB = $(TOOL_PREFIX)ranlib
  SRCS = scaffold.c
  INCS := -isystem $(NEWLIB_INC)
  CFLAGS = -Os
  BTYPE = -static
  export APPS_LIBS_INSTALL
endif

ifdef PIE
  ifneq ($(ARCH), tg-xe)
    $(error PIE is only valid for ARCH tg-xe)
  endif
  BTYPE = -pie
  CFLAGS += -fPIE
endif
#
# X86_64 Clang/llvm build
#
ifeq ($(ARCH), x86-test)
  TARGET = x86_64-linux-elf
  TOOLBIN = $(TG_INSTALL)/bin
  TOOL_PREFIX = $(TOOLBIN)/$(TARGET)-
  CC = $(TOOL_PREFIX)clang
  AS = $(TOOL_PREFIX)as
  AR = $(TOOL_PREFIX)ar
  LD = $(TOOL_PREFIX)ld
  RANLIB = $(TOOL_PREFIX)ranlib
  SRCS = scaffold.c syscall.S
  INCS := -isystem /usr/include -isystem /usr/include/x86_64-linux-gnu -isystem $(NEWLIB_INC)
  CFLAGS += -std=c99
  export APPS_LIBS_INSTALL
endif
#
# This defines a native/platform GCC build
# C99 is required to get the 'restrict' keyword
# Make sure we get just the default platform build
#
ifeq ($(ARCH), x86)
  SRCS = scaffold.c syscall.S
  INCS := -isystem /usr/include -isystem /usr/include/x86_64-linux-gnu -isystem $(NEWLIB_INC)
  CFLAGS += -std=c99
  RANLIB = /usr/bin/ranlib
endif


##########################
# Make Targets
#
debug: CFLAGS += -g
debug: default

default: all

.PHONY: check-newlib-build
check-newlib-build:


all: check-newlib-build $(SYS)/$(ARCH)
	@echo Done making all

$(SYS)/tg-xe-pie :
	@echo Building tg-xe for PIE
	$(MAKE) PIE=1 install

$(SYS)/tg-xe-static :
	@echo Building tg-xe static
	$(MAKE) install

$(SYS)/tg-xe : $(SYS)/tg-xe-static $(SYS)/tg-xe-pie

$(SYS)/x86 $(SYS)/x86-test: install

# Always try to build these when we get here
.PHONY: $(LIB) showvars clean

showvars:
	@echo SRCS = $(SRCS)
	@echo OBJS = $(OBJS)
	@echo BUILD_DIR = $(BUILD_DIR)
	@echo SRCDIR = $(SRCDIR)
	@echo LIB = $(LIB)

$(LIB): $(BUILD_DIR) $(OBJS)
	@$(RM) -f $@
	@$(AR) cru $@ $(OBJS)
	@$(RANLIB) $@
	@echo Successfully built $@

$(BUILD_DIR):
	@[ -d $(BUILD_DIR) ] || mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o : $(SRCDIR)/%.c
	@$(CC) -c $(CFLAGS) $(INCS) $< -o $@

$(BUILD_DIR)/%.o : $(SRCDIR)/%.S
	@$(AS) $< -o $@

.PHONY: install
install: $(LIB)
	@echo Installing $(LIB) to $(APPS_LIBS_INSTALL) ...
	@if [ -z $(PIE) ] ; then \
	   cp -u -R $(LIB) $(APPS_LIBS_INSTALL)/lib ; \
	 else \
	   cp -u $(BUILD_DIR)/libscaffold.a $(APPS_LIBS_INSTALL)/lib/libscaffold.p.a ; \
	 fi
	@echo Installation complete

.PHONY: uninstall
uninstall: # Doesn't actually do anything.
	@echo Uninstall is not implemented for this library

clean:
	@-rm -rf $(BUILD_ROOT)*$(ARCH)*
