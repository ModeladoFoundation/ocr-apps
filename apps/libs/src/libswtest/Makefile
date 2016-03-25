#
# Create a library for test env specific utility functions
#
# Find 'apps' repo dir (actually, whatever's above 'apps')
# And we assume the 'tg' repo is at the same level as 'apps'
#
TG_ROOT ?= $(realpath $(CURDIR)/../../../../../tg/tg)
TG_INSTALL ?= $(TG_ROOT)/install
APPS_ROOT ?= $(realpath $(CURDIR)/../../..)
APPS_LIBS_INSTALL_ROOT ?= $(APPS_ROOT)/libs/install
APPS_LIBS_INSTALL ?= $(APPS_LIBS_INSTALL_ROOT)/$(ARCH)

ARCH_PREFIX = xstg-linux-elf
ARCH = tg-xe

INCS = libswtest.h
SRCS = ce_intf.c nonewlib.c
OBJS = $(patsubst %.c,$(BUILDDIR)/%.o,$(SRCS))
LIB = $(BUILDDIR)/libswtest.a
LDSCRIPT = elf64_xstg.t

BUILDDIR = build

# INCLUDES = -isystem $(APPS_REPO)/apps/libs/tg/include
LLVM_BIN = $(TG_INSTALL)/bin

CFLAGS += -Os -std=c99 $(INCLUDES)

CXX = $(LLVM_BIN)/$(ARCH_PREFIX)-clang++
CC = $(LLVM_BIN)/$(ARCH_PREFIX)-clang
LLC = $(LLVM_BIN)/$(ARCH_PREFIX)-llc
GAS = $(LLVM_BIN)/$(ARCH_PREFIX)-as
LD = $(LLVM_BIN)/$(ARCH_PREFIX)-ld
AR = $(LLVM_BIN)/$(ARCH_PREFIX)-ar
#
# for clang to find includes
#
export APPS_LIBS_INSTALL

################### Targets #####################################
default: all

builddir:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)

all: builddir $(LIB)
	@echo Done

$(LIB) : $(OBJS)
	@echo "  = $@"
	@$(AR) -r $@ $^

$(OBJS) : libswtest.h

$(BUILDDIR)/%.o : %.c
	@echo "  - $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

%.s : %.c
	@echo Generating Assembler file $@ from $<
	@$(CC) $(CFLAGS) $(INCLUDES) -S $<

install: $(LIB)
	@echo Installing $(LIB) to $(APPS_LIBS_INSTALL) ...
	@mkdir -p $(APPS_LIBS_INSTALL)/lib
	@cp -u $(LIB) $(APPS_LIBS_INSTALL)/lib
	@cp -u $(LDSCRIPT) $(APPS_LIBS_INSTALL)/lib
	@echo Installing $(INCS) to $(APPS_LIBS_INSTALL) ...
	@mkdir -p $(APPS_LIBS_INSTALL)/include
	@cp -u $(INCS) $(APPS_LIBS_INSTALL)/include
	@echo Installation complete

clean:
	rm -rf $(LIB) $(OBJS)

dump:
	@echo TG_ROOT= $(TG_ROOT)
	@echo TG_INSTALL= $(TG_INSTALL)
	@echo APPS_ROOT= $(APPS_ROOT)
	@echo APPS_LIBS_INSTALL $(APPS_LIBS_INSTALL)

.PHONY: builddir clean dump
