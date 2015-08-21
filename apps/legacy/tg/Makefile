#
# Makefile
# vim:syntax=make noexpandtab:
#
ARCH=tg

TG_INSTALL ?= $(realpath $(CURDIR)/../../../../intel/ss/install)
APPS_ROOT  ?= $(realpath $(CURDIR)/../..)

APPS_LIBS_ROOT = $(APPS_ROOT)/libs/$(ARCH)
LIBS = $(APPS_LIBS_ROOT)/lib
OCR_INC = $(APPS_LIBS_ROOT)/include

TARGET = xstg-linux-elf
TOOLBIN = $(TG_INSTALL)/bin
TOOL_PREFIX = $(TOOLBIN)/$(TARGET)-
CC = $(TOOL_PREFIX)clang
LD = $(TOOL_PREFIX)ld
CFLAGS = -Os -isystem $(OCR_INC)
LINKSCRPT = -T $(LIBS)/elf64_xstg.t

# Libraries to include from newlib
NL_CRT = $(LIBS)/crt0.o
NL_LIBS = $(LIBS)/libc.a $(LIBS)/libm.a
NL_CRTEND = $(LIBS)/crtend.o

# Other library - this uses the libocrscaffold.a, but it
# should actually be the real ocr library/runtime.
OCR_LIB = $(LIBS)/libocrscaffold.a $(LIBS)/libswtest.a

APP = hello
APP_SRCS = hello.c
APP_LIBS =

APP_OBJS = $(APP_SRCS:.c=.o)

all: $(APP)

default: all

$(APP) : $(APP_OBJS)
	@$(LD) -o $@ $(LINKSCRPT) $(NL_CRT) $< $(NL_LIBS) $(OCR_LIB) $(APP_LIBS) $(NL_CRTEND)
	@echo Successfully built $@

%.o : %.c
	@$(CC) -c $(CFLAGS) $< -o $@

dump:
	@echo TG_INSTALL = $(TG_INSTALL)
	@echo APPS_ROOT = $(APPS_ROOT)
	@echo LIBS = $(LIBS)

clean:
	@rm -f $(APP) $(APP_OBJS)
