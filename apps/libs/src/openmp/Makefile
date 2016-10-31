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
ARCHES = tg-xe

ifndef ARCH
$(error Need to specify ARCH! use one of: $(ARCHES))
endif

ifeq (,$(findstring $(ARCH),$(ARCHES)))
$(error Unknown/unsupported  ARCH ($(ARCH))! use one of: $(ARCHES))
endif
#
# paths and flags
#
OPENMP_SRC = $(realpath openmp)
LIB = libomp.a

BUILD_PREF = build-
BUILD_DIR = $(BUILD_PREF)$(ARCH)$(BTYPE)

#INC_FLAGS = -I$(APPS_LIBS_INSTALL)/include -I$(APPS_LIBS_INSTALL)/include/sys
INC_FLAGS = -I$(APPS_LIBS_INSTALL)/include
#COMPILE_FLAGS = "-D__XSTG__ -std=c++11 --target=$(TARGET) -nostdinc++ -U__STRICT_ANSI__ $(INC_FLAGS)"
#LibompHandleFlags already has defaults including -std=c++11
COMPILE_FLAGS = "-D__XSTG__ --target=$(TARGET) -nostdinc++ -U__STRICT_ANSI__ $(INC_FLAGS)"

ifdef VERBOSE
  COMPILE_FLAGS += -v
endif
#
# communicate the apps install root to clang
#

#-DLIBCXX_ENABLE_THREADS=TRUE
CMAKE_ARGS = \
	-DCMAKE_C_COMPILER=$(CC) \
	-DCMAKE_C_COMPILER_FORCED=TRUE \
	-DCMAKE_CXX_COMPILER=$(CXX) \
	-DCMAKE_CXX_COMPILER_FORCED=TRUE \
	-DLIT_EXECUTABLE=$(TG_INSTALL)/llvm-lit \
	-DCMAKE_BINARY_DIR=$(TG_INSTALL) \
	-DCMAKE_INSTALL_PREFIX=$(APPS_LIBS_INSTALL) \
	-DLIBOMP_ENABLE_SHARED=FALSE \
	-DLIBOMP_COPY_EXPORTS=FALSE \
	-DLIBOMP_VERSION=30 \
	-DLIBOMP_USE_ADAPTIVE_LOCKS=off \
	-DLIBOMP_USE_HWLOC=off \
	-DLIBOMP_USE_ITT_NOTIFY=off \
	-DLIBOMP_OMPT_SUPPORT=off \
	-DLIBOMP_STATS=off \
	-D_POSIX_THREADS=TRUE

#
# XE Clang/llvm build
#
ifeq ($(ARCH), tg-xe)
  CC = $(TG_INSTALL)/bin/clang
  CXX = $(TG_INSTALL)/bin/clang++
  TARGET = xstg-linux-elf
  CXX_TARGET = XSTG
  BTYPE = -static
  #COMPILE_FLAGS += -D_LIBCPP_HAS_NO_LONG_LONG
  CFLAGS = $(COMPILE_FLAGS)
  CMAKE_ARGS += -DLIBOMP_ARCH=xstg \
	-DLIBCXX_HAS_PTHREAD_API=TRUE \
	-DLIBCXX_ENABLE_THREADS=TRUE \
	-DLIBOMP_USE_ITT_NOTIFY=0 \
	-DCOMPILE_FLAGS=$(COMPILE_FLAGS)
  export APPS_LIBS_INSTALL
endif

ifdef PIE
  ifneq ($(ARCH), tg-xe)
    $(error PIE is only valid for ARCH tg-xe)
  endif
  BTYPE = -pie
  COMPILE_FLAGS += -fPIE
  LIB = libomp.p.a
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
tg-xe-static:
	$(MAKE) install

tg-xe: show_args tg-xe-static tg-xe-pie

install: build
	@echo "Installing libomp.a and includes"
	@mkdir -p $(APPS_LIBS_INSTALL)/lib
	@mkdir -p $(APPS_LIBS_INSTALL)/include
	@cp -u $(BUILD_DIR)/runtime/src/libomp.a $(APPS_LIBS_INSTALL)/lib/
	@ln -sf libomp.a $(APPS_LIBS_INSTALL)/lib/libgomp.a
	@ln -sf libomp.a $(APPS_LIBS_INSTALL)/lib/libiomp5.a
	@cp -u $(BUILD_DIR)/runtime/src/*.h $(APPS_LIBS_INSTALL)/include

build: config
	@echo "Building $(LIB)"
	@$(MAKE) -C $(BUILD_DIR)

config:
	@mkdir -p $(BUILD_DIR)
	@echo "config: CMAKE_ARGS = " $(CMAKE_ARGS)
	@(cd $(BUILD_DIR) ; cmake $(CMAKE_ARGS) $(OPENMP_SRC) )

show_args:
	@echo TG_ROOT= $(TG_ROOT)
	@echo TG_INSTALL= $(TG_INSTALL)
	@echo APPS_ROOT= $(APPS_ROOT)
	@echo APPS_LIBS_INSTALL $(APPS_LIBS_INSTALL)
	@echo APPS_LIBS_ROOT $(APPS_LIBS_ROOT)
	@echo APPS_LIBS_INSTALL_ROOT $(APPS_LIBS_INSTALL_ROOT)
	@echo TARGET $(TARGET)
	@echo COMPILE_FLAGS $(COMPILE_FLAGS)
	@echo CMAKE_ARGS $(CMAKE_ARGS)

#
# Create a new build dir
#
clean: $(ARCH)-clean

tg-xe-clean:
	@rm -rf $(BUILD_PREF)$(ARCH)*

.PHONY: config clean build
