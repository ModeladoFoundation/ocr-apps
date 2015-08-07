#
# Makefile
# vim:syntax=make noexpandtab:
#
# (c) 2010, Intel
#
# Authors:
#		Justin Teller, Intel
#		EQware various
#
TG_INSTALL ?= $(realpath $(CURDIR)/../../../../../intel/ss/install)

APPS_ROOT  ?= $(realpath $(CURDIR)/../../..)

#
# Supported ARCH values
#
ARCHES = tg x86 x86_native x86_glibc

ifndef ARCH
$(error Need to specify ARCH! )
endif

ifeq (,$(findstring $(ARCH), $(ARCHES)))
$(error Unknown/unsupported  ARCH ($(ARCH)) ! )
endif

BUILD_DIR = build-$(ARCH)

APPS_LIBS_ROOT = $(APPS_ROOT)/libs/$(ARCH)
OCR_INC = $(APPS_LIBS_ROOT)/include

SRCDIR  = $(CURDIR)/$(ARCH)
LIB = $(BUILD_DIR)/libocrscaffold.a
#
# XE Clang/llvm build
#
ifeq ($(ARCH), tg)
  TARGET = xstg-linux-elf
  TOOLBIN = $(TG_INSTALL)/bin
  TOOL_PREFIX = $(TOOLBIN)/$(TARGET)-
  CC = $(TOOL_PREFIX)clang
  AS = $(TOOL_PREFIX)as
  AR = $(TOOL_PREFIX)ar
  LD = $(TOOL_PREFIX)ld
  RANLIB = $(TOOL_PREFIX)ranlib
  SRCS = ocrscaffold.c
  CFLAGS = -Os
  #CFLAGS = -Os -isystem $(OCR_INC)
  #
  # tell clang how to find includes/libs (clang 3.6)
  #
  export APPS_LIBS_ROOT
endif
#
# X86_64 Clang/llvm build
#
ifeq ($(ARCH), x86)
TARGET = x86_64-linux-elf
TOOLBIN = $(TG_INSTALL)/bin
TOOL_PREFIX = $(TOOLBIN)/$(TARGET)-
CC = $(TOOL_PREFIX)clang
AS = $(TOOL_PREFIX)as
AR = $(TOOL_PREFIX)ar
LD = $(TOOL_PREFIX)ld
RANLIB = $(TOOL_PREFIX)ranlib
SRCS = ocrscaffold.c syscall.S
INCS := -isystem /usr/include -isystem /usr/include/x86_64-linux-gnu -isystem $(OCR_INC)
CFLAGS += -std=c99
endif
#
# This defines a native/platform GCC build
# C99 is required to get the 'restrict' keyword
# Make sure we get just the default platform build
#
ifeq ($(ARCH), x86_native)
  SRCS = ocrscaffold.c syscall.S
  INCS := -isystem /usr/include \
          -isystem /usr/include/x86_64-linux-gnu \
          -isystem $(OCR_INC)
  CFLAGS += -std=c99
  RANLIB = /usr/bin/ranlib
endif
#
# This defines a native/platform GCC build
# C99 is required to get the 'restrict' keyword
# Make sure we get just the default platform build
#
ifeq ($(ARCH), x86_glibc)
  # override - use the x86 architecture support
  APPS_LIBS_ROOT = $(APPS_ROOT)/libs/x86_native
  # override - make a special library for glibc
  LIB = $(BUILD_DIR)/libocrscaffold-glibc.a
  SRCS = ocrscaffold.c
  INCS := -isystem /usr/include \
          -isystem /usr/include/x86_64-linux-gnu \
          -isystem $(OCR_INC)
  CFLAGS += -std=c99
  RANLIB = /usr/bin/ranlib
endif

OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(patsubst %.S, $(BUILD_DIR)/%.o, $(SRCS)))

##########################
# Make Targets
#
all: setup $(LIB)
	@echo Done making all
# Modify this target when ocrscaffold supports debug and non-debug builds

debug: default

default: all

# Always try to build these when we get here
.PHONY: $(LIB) setup clean

setup:
	@echo SRCS = $(SRCS)
	@echo OBJS = $(OBJS)
	@echo BUILD_DIR = $(BUILD_DIR)
	@echo SRCDIR = $(SRCDIR)
	@rm -rf $(BUILD_DIR); mkdir $(BUILD_DIR);

$(LIB): $(OBJS)
	@$(RM) -f $@
	@$(AR) cru $@ $(OBJS)
	@$(RANLIB) $@
	@echo Successfully built $@

$(BUILD_DIR)/%.o : $(SRCDIR)/%.c
	@$(CC) -c $(CFLAGS) $(INCS) $< -o $@

$(BUILD_DIR)/%.o : $(SRCDIR)/%.S
	@$(AS) $< -o $@

.PHONY: install
install: $(LIB)
	@echo Installing $(LIB) to $(APPS_LIBS_ROOT) ...
	@mkdir -p $(APPS_LIBS_ROOT)
	@cp -u -R $(LIB) $(APPS_LIBS_ROOT)/lib
	@echo Installation complete

.PHONY: uninstall
uninstall: # Doesn't actually do anything.
	@echo Uninstall is not implemented for this library

clean:
	@-rm -rf $(BUILD_DIR)
