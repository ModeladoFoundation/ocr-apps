#
# Makefile
# vim:noexpandtab:
#
# (c) 2015, Intel
#
# Authors:
#		EQware various
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
#	(defaulting to apps/apps/libs/isntall/<arch>) can be overridden by
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
LIBUNWIND_SRC = $(realpath libunwind)
LIBUNWIND_INC = $(LIBUNWIND_SRC)/include

BUILD = build_$(ARCH)
#
# Default build type - overridden by debug target below
#
BUILD_TYPE = RELEASE

COMPILE_FLAGS = --target=$(TARGET) -nostdinc++ -D__ELF__ -U__STRICT_ANSI__

ifdef VERBOSE
  COMPILE_FLAGS += -v
endif

C_FLAGS = -std=gnu11 -I$(APPS_LIBS_INSTALL)/include/c++/v1
C_FLAGS += $(CFLAGS) $(COMPILE_FLAGS)
CXX_FLAGS = -std=gnu++11 -nostdinc++ $(COMPILE_FLAGS)
#
# communicate the apps install root to clang
#

LINK_FLAGS = $(APPS_LIBS_INSTALL)/lib/libc++.a

CMAKE_ARGS =  -DLIBUNWIND_TARGET=$(UNWIND_TARGET) \
			  -DLLVM_PATH=$(LLVM_SRC) \
			  -DLLVM_MAIN_SRC_DIR=$(LLVM_SRC) \
			  -DCMAKE_INSTALL_PREFIX=$(APPS_LIBS_INSTALL) \
			  -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
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
ifeq ($(ARCH), tg-xe)
  CC = $(TG_INSTALL)/bin/clang
  CXX = $(TG_INSTALL)/bin/clang++
  TARGET = xstg-linux-elf
  UNWIND_TARGET = XSTG
  LLVM_SRC = $(TG_ROOT)/xe-llvm
  export APPS_LIBS_INSTALL
endif
#
# This defines an x86_64 build using the XE compiler
#
ifeq ($(ARCH), x86-test)
  CC = $(TG_INSTALL)/bin/clang
  CXX = $(TG_INSTALL)/bin/clang++
  TARGET = x86_64-linux-elf
  UNWIND_TARGET = x86_64
  LLVM_SRC = $(TG_ROOT)/xe-llvm
  export APPS_LIBS_INSTALL
endif
#
# This defines a native/platform GCC build
# C99 is required to get the 'restrict' keyword
# But C99 turns on strict ansi, which we disable
# Make sure we get just the default platform build
#
ifeq ($(ARCH), x86)
  LLVM_SRC = $(TG_ROOT)/xe-llvm
  CFLAGS += -std=c++11 -U__STRICT_ANSI__
  CONFIG_ARGS += --enable-multilib=no
  TARGET = x86_64-linux-gnu
  UNWIND_TARGET = x86_64
  COMPILE_FLAGS = -std=c++11 -U__STRICT_ANSI__ -D__XSTACK__ -D__LITTLE_ENDIAN__
  COMPILE_FLAGS += -isystem $(APPS_LIBS_INSTALL)/include/c++/v1
  COMPILE_FLAGS += -isystem $(APPS_ROOT)/libs/src/libunwind/libunwind/include
  COMPILE_FLAGS += -isystem $(APPS_LIBS_INSTALL)/include
endif

debug: BUILD_TYPE = DEBUG
debug: C_FLAGS += -UNDEBUG -DLOG_DEFAULT=true
debug: CXX_FLAGS += -UNDEBUG -DLOG_DEFAULT=true
debug: $(ARCH)

$(ARCHES) install: build
	@echo "Installing libunwind"
	@make -C $(BUILD) install

build: config
	@echo TG_ROOT= $(TG_ROOT)
	@echo TG_INSTALL= $(TG_INSTALL)
	@echo APPS_ROOT= $(APPS_ROOT)
	@echo APPS_LIBS_INSTALL $(APPS_LIBS_INSTALL)
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
