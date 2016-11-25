#
# Makefile
# vim:syntax=make noexpandtab:
#
# If newlib was compiled for tgr instead of ocr set USE_TGR=yes.
# This is required for pthread and openmp examples. Not using this
# will default to using the scaffold with libswtest.
#
ARCH=tg-xe

TG_INSTALL ?= $(realpath $(CURDIR)/../../../../tg/tg/install)
APPS_ROOT  ?= $(realpath $(CURDIR)/../..)

export APPS_LIBS_INSTALL=$(APPS_ROOT)/libs/install/$(ARCH)

TARGET = xstg-linux-elf
TOOLBIN = $(TG_INSTALL)/bin
TOOL_PREFIX = $(TOOLBIN)/$(TARGET)-
CC = $(TOOL_PREFIX)clang
CXX = $(TOOL_PREFIX)clang++

CFLAGS += -fxstg-swtest

ifeq ($(USE_TGR), yes)
# Use this if newlib is compiled for tgr instead of ocr.
CFLAGS += -fopenmp
endif

CFLAGS += -g -O0
CXXFLAGS = $(CFLAGS) -fexceptions -Xlinker --eh-frame-hdr

SRCS = $(wildcard *.c) $(wildcard *.cpp)
APPS = $(patsubst %.c,%,$(patsubst %.cpp,%,$(SRCS)))

all: $(APPS)

default: all

%: %.c
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) $< -o $@

%: %.cpp
	@echo "Compiling $<"
	@$(CXX) $(CXXFLAGS) $< -o $@

dump:
	@echo TG_INSTALL = $(TG_INSTALL)
	@echo APPS_ROOT = $(APPS_ROOT)
	@echo APPS_LIBS_INSTALL = $(APPS_LIBS_INSTALL)
	@echo CC = $(CC)
	@echo CXX = $(CXX)

clean:
	@rm -f $(APPS)

.PHONY: all default dump clean
