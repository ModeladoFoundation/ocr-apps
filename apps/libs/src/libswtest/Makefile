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

#.SECONDEXPANSION:

INCS = libswtest.h
SRCS = ce_intf.c nonewlib.c

OBJS = $(patsubst %.c,$(BUILDDIR)/%.o,$(SRCS))
LIB = $(BUILDDIR)/libswtest.a

OBJS_P = $(patsubst %.c,$(BUILDPDIR)/%.p.o,$(SRCS))
LIB_P = $(BUILDPDIR)/libswtest.p.a

LIBS = $(LIB) $(LIB_P)

LDSCRIPT = elf64_xstg.t

BUILDDIR = build-tg-xe-static
BUILDPDIR = build-tg-xe-pie
BUILDDIRS = $(BUILDDIR) $(BUILDPDIR)

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

$(BUILDDIRS) :
	@[ -d $@ ] || mkdir -p $@

all: $(LIB) $(LIB_P)
	@echo Done

$(LIB_P) : CFLAGS += -fPIE

$(LIB) : $(OBJS)
	@echo "  = $@"
	@$(AR) -r $@ $^ > /dev/null 2>&1

$(LIB_P) : $(OBJS_P)
	@echo "  = $@"
	@$(AR) -r $@ $^ > /dev/null 2>&1

$(OBJS) $(OBJS_P) : libswtest.h

$(BUILDDIR)/%.o : %.c | $(BUILDDIR)
	@echo "  - $@"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILDPDIR)/%.p.o : %.c | $(BUILDPDIR)
	@echo "  - $@"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

%.s : %.c
	@echo Generating Assembler file $@ from $<
	@$(CC) $(CFLAGS) $(INCLUDES) -S $<

install-$(LIB) install-static : $(LIB)
	@echo Installing $(LIB) to $(APPS_LIBS_INSTALL) ...
	@mkdir -p $(APPS_LIBS_INSTALL)/lib
	@cp -u $(LIB) $(APPS_LIBS_INSTALL)/lib
	@cp -u $(LDSCRIPT) $(APPS_LIBS_INSTALL)/lib

install-$(LIB_P) install-pie : $(LIB_P)
	@echo Installing $(LIB_P) to $(APPS_LIBS_INSTALL) ...
	@mkdir -p $(APPS_LIBS_INSTALL)/lib
	@cp -u $(LIB_P) $(APPS_LIBS_INSTALL)/lib

install-incs :
	@echo Installing $(INCS) to $(APPS_LIBS_INSTALL) ...
	@mkdir -p $(APPS_LIBS_INSTALL)/include
	@cp -u $(INCS) $(APPS_LIBS_INSTALL)/include

install: install-$(LIB) install-$(LIB_P) install-incs
	@echo Installation complete

clean:
	rm -rf $(BUILDDIRS)

dump:
	@echo TG_ROOT= $(TG_ROOT)
	@echo TG_INSTALL= $(TG_INSTALL)
	@echo APPS_ROOT= $(APPS_ROOT)
	@echo APPS_LIBS_INSTALL $(APPS_LIBS_INSTALL)

.PHONY: builddir clean dump all
