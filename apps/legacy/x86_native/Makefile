#
# Makefile
# vim:syntax=make noexpandtab:
#
ARCH=x86

APPS_ROOT  ?= $(realpath $(CURDIR)/../..)

APPS_LIBS_ROOT = $(APPS_ROOT)/libs/$(ARCH)
LIBS = $(APPS_LIBS_ROOT)/lib
OCR_INC = $(APPS_LIBS_ROOT)/include

CFLAGS += -std=c99 -U__STRICT_ANSI__ -isystem $(OCR_INC)

# Libraries to include from newlib
NL_LIBPREFIX = $(LIBS)/crt0.o
NL_LIBSUFFIX = $(LIBS)/libc.a $(LIBS)/libm.a

# Other library - this uses the libocrscaffold.a, but it
# should actually be the real ocr library/runtime.
OCR_LIB = $(LIBS)/libocrscaffold.a

APP = hello
APP_SRCS = hello.c
APP_LIBS =

APP_OBJS = $(APP_SRCS:.c=.o)

all: $(APP)

default: all

$(APP) : $(APP_OBJS)
	@$(LD) -g -o $@ $(NL_LIBPREFIX) $< $(APP_LIBS) $(NL_LIBSUFFIX) $(OCR_LIB)
	@echo Successfully built $@

%.o : %.c
	@$(CC) -c $(CFLAGS) $< -o $@

dump:
	@echo APPS_ROOT = $(APPS_ROOT)
	@echo LIBS = $(LIBS)

clean:
	@rm -f $(APP) $(APP_OBJS)
