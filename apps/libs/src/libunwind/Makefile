#
# Makefile
# vim:noexpandtab:
#
# (c) 2015, Intel
#
# Authors:
#		EQware various
# Clang:
#	This assumes that llvm/clang have been installed in the intel
#	repo install root tree (intel/ss/install) and that the intel
#	repo is actually named 'intel' and is at the same level in
# 	the directory tree as this xstack repo.
#
#	However this install root location can be overridden by adding
#	TG_INSTALL=<your_install_path> to the make cmd line or adding
#	that same variable to your environment (properly exported).
#
# Xstack install:
#	Similarly, the location of the xstack library install root
#	(defaulting to xstack/apps/libs/<arch>) can be overridden by
#	use of the APPS_LIBS_ROOT variable.
#
TG_INSTALL ?= $(realpath $(CURDIR)/../../../../../tg/tg/install)

APPS_ROOT  ?= $(realpath $(CURDIR)/../../..)
APPS_LIBS_ROOT ?= $(realpath $(CURDIR)/../../$(ARCH))

#
# Supported ARCH values
#
ARCHES = tg x86 x86_native

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

LLVM_SRC = $(TG_INSTALL)/../xe-llvm

BUILD = build_$(ARCH)

COMPILE_FLAGS = --target=$(TARGET) -nostdinc++ -D__ELF__ -U__STRICT_ANSI__

ifdef VERBOSE
  COMPILE_FLAGS += -v
  C_FLAGS += -v
endif

C_FLAGS = -I$(APPS_LIBS_ROOT)/include/c++/v1
C_FLAGS += $(CFLAGS) $(COMPILE_FLAGS)
CXX_FLAGS = -std=c++11 -nostdinc++ $(COMPILE_FLAGS)
#
# communicate the apps install root to clang
#
export APPS_LIBS_ROOT



LINK_FLAGS = $(APPS_LIBS_ROOT)/lib/libc++.a

CMAKE_ARGS =  -DLIBUNWIND_TARGET=$(UNWIND_TARGET) \
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
  UNWIND_TARGET = XSTG
endif
#
# This defines an x86_64 build using the XE compiler
#
ifeq ($(ARCH), x86)
  CC = $(TG_INSTALL)/bin/clang
  CXX = $(TG_INSTALL)/bin/clang++
  TARGET = x86_64-linux-elf
  UNWIND_TARGET = x86_64
endif
#
# This defines a native/platform GCC build
# C99 is required to get the 'restrict' keyword
# But C99 turns on strict ansi, which we disable
# Make sure we get just the default platform build
#
ifeq ($(ARCH), x86_native)
  CFLAGS += -std=c++11 -U__STRICT_ANSI__
  CONFIG_ARGS += --enable-multilib=no
  TARGET = x86_64-linux-gnu
  UNWIND_TARGET = x86_64
  COMPILE_FLAGS = -std=c++11 -nostdinc -nostdinc++ -U__STRICT_ANSI__ -D__XSTACK__ -D__LITTLE_ENDIAN__
  COMPILE_FLAGS += -isystem $(APPS_LIBS_ROOT)/include/c++/v1
  COMPILE_FLAGS += -isystem $(APPS_ROOT)/libs/src/libunwind/libunwind/include
  COMPILE_FLAGS += -isystem $(TG_INSTALL)/lib/clang/3.6.0/include
  COMPILE_FLAGS += -isystem $(TG_INSTALL)/include
  COMPILE_FLAGS += -isystem $(TG_INSTALL)/include/x86_64-linux-elf
  COMPILE_FLAGS += -isystem $(APPS_LIBS_ROOT)/include
endif

debug: BUILD_TYPE = DEBUG
debug: C_FLAGS += -UNDEBUG -DLOG_DEFAULT=true
debug: CXX_FLAGS += -UNDEBUG -DLOG_DEFAULT=true
debug: $(ARCH)

$(ARCHES) install: build
	@echo "Installing libunwind"
	@make -C $(BUILD) install

build: config
	@echo APPS_LIBS_ROOT $(APPS_LIBS_ROOT)
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
