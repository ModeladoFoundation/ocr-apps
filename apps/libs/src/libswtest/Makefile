#
# Create a library for test env specific utility functions
#
# Find 'xstack' repo dir (actually, whatever's above 'apps')
# And we assume the 'intel' repo is at the same level as 'xstack'
#
XSTACK_REPO ?= $(shell echo $(CURDIR) | sed -e 's:/apps/.*$$::')

APPS_ROOT  ?= $(XSTACK_REPO)/apps
TG_INSTALL ?= $(realpath $(XSTACK_REPO)/../intel/ss/install)

APPS_LIBS_ROOT = $(APPS_ROOT)/libs/$(ARCH)

ARCH_PREFIX = xstg-linux-elf
ARCH = tg

INCS = libswtest.h
SRCS = ce_intf.c nonewlib.c
OBJS = $(patsubst %.c,$(BUILDDIR)/%.o,$(SRCS))
LIB = $(BUILDDIR)/libswtest.a
LDSCRIPT = elf64_xstg.t

BUILDDIR = build

# INCLUDES = -isystem $(XSTACK_REPO)/apps/libs/tg/include
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
export APPS_LIBS_ROOT

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
	@echo Installing $(LIB) to $(APPS_LIBS_ROOT) ...
	@mkdir -p $(APPS_LIBS_ROOT)/lib
	@cp -u $(LIB) $(APPS_LIBS_ROOT)/lib
	@cp -u $(LDSCRIPT) $(APPS_LIBS_ROOT)/lib
	@echo Installing $(INCS) to $(APPS_LIBS_ROOT) ...
	@mkdir -p $(APPS_LIBS_ROOT)/include
	@cp -u $(INCS) $(APPS_LIBS_ROOT)/include
	@echo Installation complete

clean:
	rm -rf $(LIB) $(OBJS)

.PHONY: builddir clean
