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
#	only its own files libcxx/include directly. However, it does require
#	that the newlib include files have been installed under APPS_INSTALL.
#	The location of the apps library install root (defaulting to
#	apps/apps/libs/install/<arch>) can be overridden by
#	use of the APPS_INSTALL variable.
#
TG_ROOT ?= $(realpath $(CURDIR)/../../../../../tg/tg)
TG_INSTALL ?= $(TG_ROOT)/install
APPS_ROOT ?= $(realpath $(CURDIR)/../../..)
APPS_INSTALL_ROOT ?= $(APPS_ROOT)/libs/install
APPS_INSTALL ?= $(APPS_INSTALL_ROOT)/$(ARCH)
APPS_INCLUDE ?= $(APPS_INSTALL)/include

LLVM_CONFIG = $(TG_ROOT)/build/build.xe-llvm/build_llvm/bin/llvm-config

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
# paths and flags
#
LIBCXX_SRC = $(realpath libcxx)
LIBCXXABI_SRC = $(realpath ../libcxxabi/libcxxabi)
LIBCXXABI_INC = $(LIBCXXABI_SRC)/include

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

CMAKE_ARGS =  -DLIBCXX_TARGET=$(CXX_TARGET) \
			  -DLIBCXX_CXX_ABI=libcxxabi \
			  -DLIBCXX_CXX_ABI_INCLUDE_PATHS=$(LIBCXXABI_INC) \
			  -DLIT_EXECUTABLE=$(TG_INSTALL)/llvm-lit \
			  -DCMAKE_BINARY_DIR=$(TG_INSTALL) \
			  -DCMAKE_INSTALL_PREFIX=$(APPS_INSTALL) \
			  -DCMAKE_C_COMPILER=$(CC) \
			  -DCMAKE_C_COMPILER_FORCED=TRUE \
			  -DCMAKE_CXX_COMPILER=$(CXX) \
			  -DCMAKE_CXX_COMPILER_FORCED=TRUE \
			  -DLIBCXX_HAS_PTHREAD_API=TRUE \
			  -DLIBCXX_ENABLE_THREADS=TRUE \
			  -DLIBCXX_ENABLE_SHARED=FALSE \
			  -DLIBCXX_ENABLE_FILESYSTEM=FALSE \
			  -DCMAKE_CXX_FLAGS:STRING="$(COMPILE_FLAGS)" \
			  -DLLVM_CONFIG="$(LLVM_CONFIG)"

#
# XE Clang/llvm build
#
ifeq ($(ARCH), tg-xe)
  CC = $(TG_INSTALL)/bin/clang
  CXX = $(TG_INSTALL)/bin/clang++
  TARGET = xstg-linux-elf
  CXX_TARGET = XSTG
  CFLAGS += -D_LIBCPP_HAS_LONG_DOUBLE
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
  LIB = libc++.p.a
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
  COMPILE_FLAGS += -I$(APPS_INCLUDE)
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
  CXX_TARGET = x86_64
  COMPILE_FLAGS =  -std=c++11 -U__STRICT_ANSI__ -D__XSTACK__ -D__LITTLE_ENDIAN__
  COMPILE_FLAGS += -fno-stack-protector -fno-use-cxa-atexit
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
	@echo "Installing $(LIB) and C++ includes"
	@cp -u $(BUILD_DIR)/lib/libc++.a $(APPS_INSTALL)/lib/$(LIB)
	@INCS="$(APPS_INSTALL)/include" ; \
	 if [ ! -d $$INCS/c++ ] ; then \
	  cp -R -u $(BUILD_DIR)/include/c++ $$INCS ; \
	  cp -u $(BUILD_DIR)/include/*.h $$INCS/c++/v1 ; \
	 fi

build: config
	@echo "Building $(LIB)"
	@$(MAKE) -C $(BUILD_DIR)

config: show_args
	@mkdir -p $(BUILD_DIR)
	@(cd $(BUILD_DIR) ; cmake $(CMAKE_ARGS) $(LIBCXX_SRC) )

show_args:
	@echo TG_ROOT= $(TG_ROOT)
	@echo TG_INSTALL= $(TG_INSTALL)
	@echo APPS_ROOT= $(APPS_ROOT)
	@echo APPS_INSTALL $(APPS_INSTALL)

#
# Create a new build dir
#
clean: $(ARCH)-clean

x86-test-clean x86-clean:
	@rm -rf $(BUILD_DIR)


tg-xe-clean:
	@rm -rf $(BUILD_PREF)$(ARCH)*


.PHONY: config clean build
