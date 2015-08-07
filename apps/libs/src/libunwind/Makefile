#
# Makefile
# vim:noexpandtab:
#
# (c) 2015, Intel
#
# Authors:
#		EQware various
#
TG_INSTALL ?= $(realpath $(CURDIR)/../../../../../intel/ss/install)

APPS_ROOT  ?= $(realpath $(CURDIR)/../../..)
APPS_LIBS_ROOT ?= $(realpath $(CURDIR)/../../$(ARCH))

#
# Supported ARCH values
#
ARCHES = tg x86

ifndef ARCH
$(error Need to specify ARCH! )
endif

ifeq (,$(findstring $(ARCH), $(ARCHES)) )
$(error Unknown/unsupported  ARCH ($(ARCH)) ! )
endif

LIBCXXABI_SRC = $(realpath libcxxabi)
LIBCXXABI_INC = $(LIBCXXABI_SRC)/include

LIBUNWIND_SRC = $(realpath libunwind)
LIBUNWIND_INC = $(LIBUNWIND_SRC)/include

LIBCXX_SRC = $(realpath ../libcxxX/libcxx)
LIBCXX_INC = $(LIBCXX_SRC)/include
LIBCXX_LIB = $(APPS_LIBS_ROOT)/lib

LLVM_SRC = $(TG_INSTALL)/../xe-llvm

BUILD = build_$(ARCH)

COMPILE_FLAGS = --target=$(TARGET) -D__ELF__
COMPILE_FLAGS += -I$(APPS_LIBS_ROOT)/include/c++/v1

ifdef VERBOSE
  COMPILE_FLAGS += -v
endif
C_FLAGS = $(CFLAGS) $(COMPILE_FLAGS)
CXX_FLAGS = -std=c++11 -nostdinc++ $(COMPILE_FLAGS)
#
# communicate the apps install root to clang
#
export APPS_ROOT = $(APPS_LIBS_ROOT)



LINK_FLAGS = $(APPS_LIBS_ROOT)/lib/libc++.a

CMAKE_ARGS =  -DLIBUNWIND_TARGET=XSTG \
			  -DLLVM_PATH=$(LLVM_SRC) \
			  -DLLVM_MAIN_SRC_DIR=$(LLVM_SRC) \
			  -DCMAKE_INSTALL_PREFIX=$(APPS_LIBS_ROOT) \
			  -DCMAKE_C_COMPILER=$(CC) \
			  -DCMAKE_C_COMPILER_FORCED=TRUE \
			  -DCMAKE_CXX_COMPILER=$(CXX) \
			  -DCMAKE_CXX_COMPILER_FORCED=TRUE \
			  -DLIBUNWIND_ENABLE_SHARED=FALSE \
			  -DLIBUNWIND_LINK_FLAGS="$(LINK_FLAGS)" \
			  -DCMAKE_C_FLAGS:STRING="$(C_FLAGS)" \
			  -DCMAKE_CXX_FLAGS:STRING="$(CXX_FLAGS)"

##########################
# Make Targets
#
all: $(ARCH)
	@echo Done making all
#
# XE Clang/llvm build
#
ifeq ($(ARCH), tg)
CC = $(TG_INSTALL)/bin/clang
CXX = $(TG_INSTALL)/bin/clang++
TARGET = xstg-linux-elf
endif
#
# This defines an x86_64 build using the XE compiler
#
ifeq ($(ARCH), x86)
CC = $(TG_INSTALL)/bin/clang
CXX = $(TG_INSTALL)/bin/clang++
TARGET = x86_64-linux-elf
endif

$(ARCHES) install: build
	@echo "Installing libunwind"
	@make -C $(BUILD) install

build: config
	@echo "Building libunwind"
	@make -C $(BUILD)

config : clean
	@(cd $(BUILD) ; cmake $(CMAKE_ARGS) $(LIBUNWIND_SRC) )

#
# Create a new build dir
#
clean :
	@-[ -d $(BUILD) ] && rm -rf $(BUILD)
	@mkdir -p $(BUILD)

.PHONY: config clean build
