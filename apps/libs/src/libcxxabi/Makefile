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
#	This build does not use the installed C++ headers, but references
#	the files in ../libcxx directly. However, it does require that the
#	newlib include files have been installed under APPS_INSTALL.
#	Similarly, the location of the apps library install root
#	(defaulting to apps/apps/libs/install/<arch>) can be overridden by
#	use of the APPS_INSTALL variable.
#
TG_ROOT ?= $(realpath $(CURDIR)/../../../../../tg/tg)
TG_INSTALL ?= $(TG_ROOT)/install
APPS_ROOT ?= $(realpath $(CURDIR)/../../..)
APPS_INSTALL_ROOT ?= $(APPS_ROOT)/libs/install
APPS_INSTALL ?= $(APPS_INSTALL_ROOT)/$(ARCH)
APPS_INCLUDE ?= $(APPS_INSTALL)/include

#
# Supported ARCH values
#
X86_ARCHES = x86 x86-test
ARCHES = tg-xe $(X86_ARCHES)

ifndef ARCH
$(error Need to specify ARCH! use one of: $(ARCHES))
endif

ifeq (,$(findstring $(ARCH),$(ARCHES)))
$(error Unknown/unsupported  ARCH ($(ARCH))! use one of: $(ARCHES))
endif
#
# flags and paths
#
LIB = libc++abi.a

LIBCXXABI_SRC = $(realpath libcxxabi)
LIBCXXABI_INC = $(LIBCXXABI_SRC)/include

LIBUNWIND_SRC = $(realpath ../libunwind/libunwind)
LIBUNWIND_INC = $(LIBUNWIND_SRC)/include

LIBCXX_SRC = $(realpath ../libcxx/libcxx)
LIBCXX_INC = $(LIBCXX_SRC)/include
LIBCXX_LIB = $(APPS_INSTALL)/lib

LLVM_CONFIG = $(TG_ROOT)/build/build.xe-llvm/build_llvm/bin/llvm-config

BUILD_PREF = build-
BUILD_DIR = $(BUILD_PREF)$(ARCH)$(BTYPE)

COMPILE_DEFINES = -D_POSIX_THREADS -U__STRICT_ANSI__

COMPILE_FLAGS = -std=c++11 --target=$(TARGET) -nostdinc++ $(COMPILE_DEFINES)

ifdef VERBOSE
  COMPILE_FLAGS += -v
endif
#
# communicate the apps install root to clang
#

LINK_FLAGS = $(APPS_INSTALL)/lib/libc++.a

CMAKE_ARGS =  -DLIBCXXABI_TARGET=$(CXXABI_TARGET) \
			  -DLIBCXXABI_LIBCXX_INCLUDES=$(LIBCXX_INC) \
			  -DLIBCXXABI_LIBCXX_PATH=$(LIBCXX_SRC) \
			  -DLIBCXXABI_LIBUNWIND_SOURCES=$(LIBUNWIND_SRC) \
			  -DLIBCXXABI_LIBUNWIND_INCLUDES_INTERNAL=$(LIBUNWIND_INC) \
			  -DCMAKE_INSTALL_PREFIX=$(APPS_INSTALL) \
			  -DCMAKE_C_COMPILER=$(CC) \
			  -DCMAKE_C_COMPILER_FORCED=TRUE \
			  -DCMAKE_CXX_COMPILER=$(CXX) \
			  -DCMAKE_CXX_COMPILER_FORCED=TRUE \
			  -DLIBCXXABI_HAS_PTHREAD_API=TRUE \
			  -DLIBCXXABI_ENABLE_THREADS=TRUE \
			  -DLIBCXXABI_ENABLE_SHARED=FALSE \
			  -DLIBCXXABI_LINK_FLAGS="$(LINK_FLAGS)" \
			  -DLIBCXXABI_COMPILE_FLAGS="$(COMPILE_FLAGS)" \
			  -DLIBCXXABI_USE_LLVM_UNWINDER=TRUE \
			  -DCMAKE_CXX_FLAGS:STRING="$(COMPILE_FLAGS)" \
			  -DLLVM_CONFIG=$(LLVM_CONFIG)

#
# XE Clang/llvm build
#
ifeq ($(ARCH), tg-xe)
  CC = $(TG_INSTALL)/bin/clang
  CXX = $(TG_INSTALL)/bin/clang++
  TARGET = xstg-linux-elf
  CXXABI_TARGET = XSTG
  BTYPE = -static
  COMPILE_FLAGS += -I$(APPS_INCLUDE)
  # export APPS_LIBS_INSTALL
endif

ifdef PIE
  ifneq ($(ARCH), tg-xe)
    $(error PIE is only valid for ARCH tg-xe)
  endif
  BTYPE = -pie
  COMPILE_FLAGS += -fPIE
  LIB = libc++abi.p.a
endif
#
# This defines an x86_64 build using the XE compiler
#
ifeq ($(ARCH), x86-test)
  CC = $(TG_INSTALL)/bin/clang
  CXX = $(TG_INSTALL)/bin/clang++
  TARGET = x86_64-linux-elf
  CXXABI_TARGET = x86_64
  #export APPS_LIBS_INSTALL
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
  CXXABI_TARGET = x86_64
  COMPILE_FLAGS = -std=c++11 -U__STRICT_ANSI__ -D__LITTLE_ENDIAN__
  COMPILE_FLAGS += -fno-stack-protector
  COMPILE_FLAGS += -isystem $(APPS_INSTALL)/include
endif

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

# This really doesn't have a dependency on building pie first. The
# dependency was added just in case you run it with -j so that only
# one is being built at a time.
tg-xe-static: tg-xe-pie
	$(MAKE) install

tg-xe: show_args tg-xe-pie tg-xe-static

$(X86_ARCHES): show_args
	$(MAKE) install

install: build
	@echo "Installing $(LIB)"
	@cp -u $(BUILD_DIR)/lib/libc++abi.a $(APPS_INSTALL)/lib/$(LIB)

build: config
	@echo "Building $(LIB)"
	@$(MAKE) -C $(BUILD_DIR)

config :
	@mkdir -p $(BUILD_DIR)
	@(cd $(BUILD_DIR) ; rm -rf CMakeFiles; cmake $(CMAKE_ARGS) $(LIBCXXABI_SRC) )

show_args :
	@echo TG_ROOT= $(TG_ROOT)
	@echo TG_INSTALL= $(TG_INSTALL)
	@echo APPS_ROOT= $(APPS_ROOT)
	@echo APPS_INSTALL $(APPS_INSTALL)

#
# Create a new build dir
#
clean : $(ARCH)-clean

x86-clean x86-test-clean:
	@rm -rf $(BUILD_DIR)

tg-xe-clean:
	@rm -rf $(BUILD_PREF)$(ARCH)*



.PHONY: config clean build
