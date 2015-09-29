#
# Makefile
# vim:syntax=make noexpandtab:
#
ARCH=x86_native

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
OCR_LIB = $(APPS_ROOT)/../ocr/build/x86_lib/objs/myocr.o

APP = hello
APP_SRCS = hello.c cmain.c
APP_LIBS =

APP_OBJS = $(APP_SRCS:.c=.o)

all: $(APP)

default: all

$(APP) : $(APP_OBJS)
	@$(LD) -r -o tmp.o $< --start-group $(NL_LIBSUFFIX) --end-group
	@nm -u tmp.o | sed 's/^.*U //' > syms
	@objcopy -S --keep-symbols keep --keep-symbols syms tmp.o
	@$(CC) -o $@ tmp.o $(OCR_LIB) -lthread_db
#	$(LD) -r -s -o app.o --start-group $< $(CRT0) $(LIBC) --end-group # Link against newlib
#	$(LD) -nostartfiles -nodefaultlibs -nostdlib -static -o $@ $(OCR_LIB) myapp.o -zmuldefs
#	@$(LD) -g -o $@ $(NL_LIBPREFIX) $< $(APP_LIBS) $(NL_LIBSUFFIX) $(OCR_LIB)
	@echo Successfully built $@

%.o : %.c
	@$(CC) -g -O0 -c $(CFLAGS) $< -o $@

dump:
	@echo APPS_ROOT = $(APPS_ROOT)
	@echo LIBS = $(LIBS)

clean:
	@rm -f $(APP) $(APP_OBJS)
