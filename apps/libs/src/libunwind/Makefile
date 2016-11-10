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
#	use of the APPS_INSTALL variable.
#
TG_ROOT ?= $(realpath $(CURDIR)/../../../../../tg/tg)
TG_INSTALL ?= $(TG_ROOT)/install
APPS_ROOT ?= $(realpath $(CURDIR)/../../..)
APPS_INSTALL_ROOT ?= $(APPS_ROOT)/libs/install
APPS_INSTALL ?= $(APPS_INSTALL_ROOT)/$(ARCH)

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
LIBUNWIND_SRC = $(realpath libunwind)
LIBUNWIND_INC = $(LIBUNWIND_SRC)/include
LIB = libunwind.a

LIBCXX_SRC = $(realpath ../libcxx/libcxx)
LIBCXX_INC = $(LIBCXX_SRC)/include

BUILD_PREF = build-
BUILD_DIR = $(BUILD_PREF)$(ARCH)$(BTYPE)

#
# Default build type - overridden by debug target below
#
BUILD_TYPE ?= RELEASE
DBG_FLAGS ?= -DNDEBUG

COMPILE_DEFINES = -D__ELF__ -U__STRICT_ANSI__

COMPILE_FLAGS = --target=$(TARGET) -nostdinc++ $(COMPILE_DEFINES)

COMPILE_FLAGS += $(DBG_FLAGS)

ifdef VERBOSE
  COMPILE_FLAGS += -v
endif

C_FLAGS = $(CFLAGS) -std=gnu11 $(COMPILE_FLAGS)
CXX_FLAGS = -std=gnu++11 -nostdinc++ $(COMPILE_FLAGS)
#
# communicate the apps install root to clang
#

LINK_FLAGS = $(APPS_INSTALL)/lib/libc++.a

CMAKE_ARGS =  -DLIBUNWIND_TARGET=$(UNWIND_TARGET) \
			  -DLLVM_PATH=$(LLVM_SRC) \
			  -DLLVM_MAIN_SRC_DIR=$(LLVM_SRC) \
			  -DCMAKE_INSTALL_PREFIX=$(APPS_INSTALL) \
			  -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
			  -DCMAKE_C_COMPILER=$(CC) \
			  -DCMAKE_C_COMPILER_FORCED=TRUE \
			  -DCMAKE_CXX_COMPILER=$(CXX) \
			  -DCMAKE_CXX_COMPILER_FORCED=TRUE \
			  -DLIBUNWIND_ENABLE_SHARED=FALSE \
			  -DLIBUNWIND_LINK_FLAGS="$(LINK_FLAGS)" \
			  -DCMAKE_C_FLAGS:STRING="$(C_FLAGS)" \
			  -DCMAKE_CXX_FLAGS:STRING="$(CXX_FLAGS)"


#
# XE Clang/llvm build
#
ifeq ($(ARCH), tg-xe)
  CC = $(TG_INSTALL)/bin/clang
  CXX = $(TG_INSTALL)/bin/clang++
  COMPILE_FLAGS += -I$(TG_ROOT)/common/include
  TARGET = xstg-linux-elf
  UNWIND_TARGET = XSTG
  LLVM_SRC = $(TG_ROOT)/xe-llvm
  BTYPE = -static
  export APPS_LIBS_INSTALL=$(APPS_INSTALL)
endif

ifdef PIE
  ifneq ($(ARCH), tg-xe)
    $(error PIE is only valid for ARCH tg-xe)
  endif
  BTYPE = -pie
  COMPILE_FLAGS += -fPIE
  LIB = libunwind.p.a
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
  export APPS_LIBS_INSTALL=$(APPS_INSTALL)
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
  COMPILE_FLAGS += -isystem $(APPS_INSTALL)/include/c++/v1
  COMPILE_FLAGS += -isystem $(APPS_ROOT)/libs/src/libunwind/libunwind/include
  COMPILE_FLAGS += -isystem $(APPS_INSTALL)/include
endif

##########################
# Make Targets
#
default: all

all: $(ARCH)
	@echo Done making all

debug: export BUILD_TYPE = DEBUG
debug: export DBG_FLAGS = -UNDEBUG -DLOG_DEFAULT=true -g
debug: $(ARCH)
#
# ARCH specific targets
#
tg-xe-pie:
	$(MAKE) PIE=1 install

tg-xe-static: tg-xe-pie
	# This really doesn't have a dependency on building pie first. The
	# dependency was added just in case you run it with -j so that only
	# one is being built at a time.
	$(MAKE) install

tg-xe: show_args tg-xe-pie tg-xe-static

$(X86_ARCHES): show_args
	$(MAKE) install

install: build
	@echo "Installing $(LIB)"
	@cp -u $(BUILD_DIR)/lib/libunwind.a $(APPS_INSTALL)/lib/$(LIB)

build: config
	@echo "Building $(LIB)"
	@$(MAKE) -C $(BUILD_DIR)

config : show_args
	@mkdir -p $(BUILD_DIR)
	@(cd $(BUILD_DIR) ; rm -rf CMakeFiles ; cmake $(CMAKE_ARGS) $(LIBUNWIND_SRC) )

show_args :
	@echo TG_ROOT= $(TG_ROOT)
	@echo TG_INSTALL= $(TG_INSTALL)
	@echo APPS_ROOT= $(APPS_ROOT)
	@echo APPS_INSTALL $(APPS_INSTALL)
	@echo BUILD_DIR $(BUILD_DIR)
	@echo COMPILE_FLAGS $(COMPILE_FLAGS)
#
# Create a new build dir
#
clean : $(ARCH)-clean

x86-clean x86-test-clean:
	@rm -rf $(BUILD_DIR)

tg-xe-clean:
	@rm -rf $(BUILD_PREF)$(ARCH)*

.PHONY: config clean build show_args
