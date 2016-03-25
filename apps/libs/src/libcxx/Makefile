#
# Makefile
# vim:noexpandtab:
#
# (c) 2015, Intel
#
# Authors:
#		EQware various
#
# Clang:
#	This assumes that llvm/clang have been installed in the tg
#	repo install root tree (tg/tg/install) and that the tg
#	repo is actually named 'tg' and is at the same level in
# 	the directory tree as this apps repo.
#
#	However this install root location can be overridden by adding
#	TG_INSTALL=<your_install_path> to the make cmd line or adding
#	that same variable to your environment (properly exported).
#
# Xstack install:
#	Similarly, the location of the apps library install root
#	(defaulting to apps/apps/libs/install/<arch>) can be overridden by
#	use of the APPS_LIBS_INSTALL variable.
#
TG_ROOT ?= $(realpath $(CURDIR)/../../../../../tg/tg)
TG_INSTALL ?= $(TG_ROOT)/install
APPS_ROOT ?= $(realpath $(CURDIR)/../../..)
APPS_LIBS_INSTALL_ROOT ?= $(APPS_ROOT)/libs/install
APPS_LIBS_INSTALL ?= $(APPS_LIBS_INSTALL_ROOT)/$(ARCH)

#
# Supported ARCH values
#
ARCHES = tg-xe x86 x86-test

ifndef ARCH
$(error Need to specify ARCH! use one of: $(ARCHES))
endif

ifeq (,$(findstring $(ARCH),$(ARCHES)))
$(error Unknown/unsupported  ARCH ($(ARCH))! use one of: $(ARCHES))
endif
#
# paths and flags
#
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

CMAKE_ARGS =  -DLIBCXX_TARGET=$(CXX_TARGET) \
			  -DLIBCXX_CXX_ABI=libcxxabi \
			  -DLIBCXX_CXX_ABI_INCLUDE_PATHS=$(LIBCXXABI_INC) \
			  -DLIT_EXECUTABLE=$(TG_INSTALL)/llvm-lit \
			  -DCMAKE_BINARY_DIR=$(TG_INSTALL) \
			  -DCMAKE_INSTALL_PREFIX=$(APPS_LIBS_INSTALL) \
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
ifeq ($(ARCH), tg-xe)
  CC = $(TG_INSTALL)/bin/clang
  CXX = $(TG_INSTALL)/bin/clang++
  TARGET = xstg-linux-elf
  CXX_TARGET = XSTG
  CFLAGS += -D_LIBCPP_HAS_LONG_DOUBLE
  export APPS_LIBS_INSTALL
endif
#
# This defines an x86_64 build using the XE compiler
#
ifeq ($(ARCH), x86-test)
  CC = $(TG_INSTALL)/bin/clang
  CXX = $(TG_INSTALL)/bin/clang++
  TARGET = x86_64-linux-elf
  CXX_TARGET = x86_64
  COMPILE_FLAGS += -fno-use-cxa-atexit
  export APPS_LIBS_INSTALL
endif

#
# This defines a native/platform GCC build
# C99 is required to get the 'restrict' keyword
# But C99 turns on strict ansi, which we disable
# Make sure we get just the default platform build
#
ifeq ($(ARCH), x86)
  CFLAGS += -std=c++11 -U__STRICT_ANSI__
  CONFIG_ARGS += --enable-multilib=no
  TARGET = x86_64-linux-gnu
  CXX_TARGET = x86_64
  COMPILE_FLAGS =  -std=c++11 -U__STRICT_ANSI__ -D__XSTACK__ -D__LITTLE_ENDIAN__
  COMPILE_FLAGS += -fno-stack-protector -fno-use-cxa-atexit
  COMPILE_FLAGS += -isystem $(APPS_LIBS_INSTALL)/include
endif

$(ARCHES) install: build
	@echo "Installing libcxx"
	@make -C $(BUILD) install

build: config
	@echo "Building libcxx"
	@make -C $(BUILD)

config : clean
	@echo TG_ROOT= $(TG_ROOT)
	@echo TG_INSTALL= $(TG_INSTALL)
	@echo APPS_ROOT= $(APPS_ROOT)
	@echo APPS_LIBS_INSTALL $(APPS_LIBS_INSTALL)
	@(cd $(BUILD) ; cmake $(CMAKE_ARGS) $(LIBCXX_SRC) )

#
# Create a new build dir
#
clean :
	@-[ -d $(BUILD) ] && rm -rf $(BUILD)
	@mkdir -p $(BUILD)

.PHONY: config clean build
