#
# Makefile
# vim:syntax=make noexpandtab:
#
ARCH=tg-xe

TG_INSTALL ?= $(realpath $(CURDIR)/../../../../tg/tg/install)
APPS_ROOT  ?= $(realpath $(CURDIR)/../..)

export APPS_LIBS_INSTALL=$(APPS_ROOT)/libs/install/$(ARCH)
LIBS = $(APPS_LIBS_INSTALL)/lib
OCR_INC = $(APPS_LIBS_INSTALL)/include

TARGET = xstg-linux-elf
TOOLBIN = $(TG_INSTALL)/bin
TOOL_PREFIX = $(TOOLBIN)/$(TARGET)-
CC = $(TOOL_PREFIX)clang
CXX = $(TOOL_PREFIX)clang++
LD = $(TOOL_PREFIX)ld

# Libraries to include from newlib
NL_CRT = $(LIBS)/crt0t.o
NL_LIBS = $(LIBS)/libc.a $(LIBS)/libm.a
CPP_LIBS = $(LIBS)/libc++.a $(LIBS)/libc++abi.a $(LIBS)/libunwind.a
NL_CRTEND = $(LIBS)/crtend.o

# Other library - this uses the libocrscaffold.a, but it
# should actually be the real ocr library/runtime.
OCR_LIB = $(LIBS)/libocrscaffold.a $(LIBS)/libswtest.a

C_LIBS =
CFLAGS = -g -O0 -nostdinc -isystem $(OCR_INC)
CFLAGS += -isystem $(TG_INSTALL)/lib/clang/3.6.0/include
CFLAGS += -isystem $(TG_INSTALL)/include
CFLAGS += -isystem $(TG_INSTALL)/include/x86_64-linux-elf
CFLAGS += -isystem $(APPS_LIBS_INSTALL)/include

CXX_LIBS =
CXXFLAGS = -g -O0 -nostdinc -isystem $(OCR_INC)
CXXFLAGS += -isystem $(APPS_LIBS_INSTALL)/include/c++/v1
CXXFLAGS += -isystem $(APPS_ROOT)/libs/src/libunwind/libunwind/include
CXXFLAGS += -isystem $(TG_INSTALL)/lib/clang/3.6.0/include
CXXFLAGS += -isystem $(TG_INSTALL)/include
CXXFLAGS += -isystem $(TG_INSTALL)/include/x86_64-linux-elf
CXXFLAGS += -isystem $(APPS_LIBS_INSTALL)/include

SRCS = $(wildcard *.c) $(wildcard *.cpp)
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SRCS)))
APPS = $(OBJS:.o=)

all: $(APPS)

default: all

%: %.o %.c
	@$(LD) -o $@ -T $(LIBS)/elf64_xstg.t $(NL_CRT) $@.o $(NL_LIBS) $(OCR_LIB) $(C_LIBS) $(NL_CRTEND)
	@echo Successfully built $@

%: %.o %.cpp
	@$(LD) -o $@ -T elf64_xstg.t $(NL_CRT) $@.o $(CPP_LIBS) $(NL_LIBS) $(OCR_LIB) $(CXX_LIBS) $(NL_CRTEND)
	@echo Successfully built $@

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

%.o: %.cpp
	@$(CXX) -c $(CXXFLAGS) $< -o $@

dump:
	@echo TG_INSTALL = $(TG_INSTALL)
	@echo APPS_ROOT = $(APPS_ROOT)
	@echo APPS_LIBS_INSTALL = $(APPS_LIBS_INSTALL)
	@echo LIBS = $(LIBS)
	@echo CC = $(CC)
	@echo CXX = $(CXX)
	@echo LD = $(LD)

clean:
	@rm -f $(APPS) $(OBJS)

# Disable implicit rules
.SUFFIXES:

# Don't remove the .o files when finished building
.PRECIOUS: %.o

.PHONY: all default dump clean
