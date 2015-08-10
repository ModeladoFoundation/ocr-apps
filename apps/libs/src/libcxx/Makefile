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

LIBCXX_SRC = $(realpath libcxx)
LIBCXXABI_SRC = $(realpath ../libcxxabi/libcxxabi)
LIBCXXABI_INC = $(LIBCXXABI_SRC)/include

BUILD = build_$(ARCH)

COMPILE_FLAGS = -std=c++11 --target=$(TARGET) -nostdinc++ -U__STRICT_ANSI__

ifdef VERBOSE
  COMPILE_FLAGS += -v
endif
#
# communicate the apps install root to clang
#
export APPS_LIBS_ROOT

CMAKE_ARGS =  -DLIBCXX_TARGET=XSTG \
			  -DLIBCXX_CXX_ABI=libcxxabi \
			  -DLIBCXX_CXX_ABI_INCLUDE_PATHS=$(LIBCXXABI_INC) \
			  -DLIT_EXECUTABLE=$(TG_INSTALL)/llvm-lit \
			  -DCMAKE_BINARY_DIR=$(TG_INSTALL) \
			  -DCMAKE_INSTALL_PREFIX=$(APPS_LIBS_ROOT) \
			  -DCMAKE_C_COMPILER=$(CC) \
			  -DCMAKE_C_COMPILER_FORCED=TRUE \
			  -DCMAKE_CXX_COMPILER=$(CXX) \
			  -DCMAKE_CXX_COMPILER_FORCED=TRUE \
			  -DLIBCXX_ENABLE_THREADS=FALSE \
			  -DLIBCXX_ENABLE_SHARED=FALSE \
			  -DCMAKE_CXX_FLAGS:STRING="$(COMPILE_FLAGS)"

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
	@echo "Installing libcxx"
	@make -C $(BUILD) install

build: config
	@echo "Building libcxx"
	@make -C $(BUILD)

config : clean
	@(cd $(BUILD) ; cmake $(CMAKE_ARGS) $(LIBCXX_SRC) )

#
# Create a new build dir
#
clean :
	@-[ -d $(BUILD) ] && rm -rf $(BUILD)
	@mkdir -p $(BUILD)

.PHONY: config clean build
