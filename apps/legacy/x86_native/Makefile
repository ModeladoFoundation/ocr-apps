#
# Makefile
# vim:syntax=make noexpandtab:
#
ARCH=x86_native

APPS_ROOT  ?= $(realpath $(CURDIR)/../..)
#TG_ROOT = $(APPS_ROOT)/../../tg
#LD = $(TG_ROOT)/tg/install/bin/x86_64-linux-elf-ld

APPS_LIBS_ROOT = $(APPS_ROOT)/libs/$(ARCH)
LIBS = $(APPS_LIBS_ROOT)/lib
OCR_INC = $(APPS_LIBS_ROOT)/include

CFLAGS += -std=c99 -U__STRICT_ANSI__ -isystem $(OCR_INC)
LINK_OPTS = -Xlinker -rpath=$(APPS_ROOT)/legacy/x86_native -Xlinker -rpath=$(APPS_ROOT)/../ocr/build/x86_lib/objs

GLIBC_LIB_PATH = -L$(CURDIR) -L$(OCR_LIB_PATH) -L/usr/lib/x86_64-linux-gnu -L/usr/lib/gcc/x86_64-linux-gnu/4.9 -L/lib/x86_64-linux-gnu -L/lib -L/usr/lib
GLIBC_LIBPREFIX = /usr/lib/x86_64-linux-gnu/crt1.o /usr/lib/x86_64-linux-gnu/crti.o /usr/lib/gcc/x86_64-linux-gnu/4.9/crtbegin.o
GLIBC_LIBSUFFIX = /usr/lib/gcc/x86_64-linux-gnu/4.9/crtend.o /usr/lib/x86_64-linux-gnu/crtn.o

# Libraries to include from newlib
NL_LIBPREFIX = $(LIBS)/crt0.o
NL_LIBSUFFIX = $(LIBS)/libc.a $(LIBS)/libm.a

# Other library - this uses the libocrscaffold.a, but it
# should actually be the real ocr library/runtime.
OCR_LIB_PATH = $(APPS_ROOT)/../ocr/build/x86_lib/objs
#OCR_LIB = $(OCR_LIB_PATH)/libocr.so
OCR_LIB = $(OCR_LIB_PATH)/libocr.a

APP = hello
APP_SRCS = cmain.c hello.c
APP_LIBS =

APP_OBJS = $(APP_SRCS:.c=.o)

all: $(APP)

default: all

$(APP) : $(APP_OBJS)
	$(LD) -shared -static -o libhello.so hello.o $(NL_LIBSUFFIX)
#	$(CC) -v -o $@ cmain.o -L$(APPS_ROOT)/legacy/x86_native -L$(APPS_ROOT)/../ocr/build/x86_lib/objs/ -L/usr/lib/x86_64-linux-gnu -locr -lc -lpthread -lnewlibc hello.o $(LINK_OPTS)
	$(CC) -v -o $@ cmain.o -L$(APPS_ROOT)/legacy/x86_native -L$(APPS_ROOT)/../ocr/build/x86_lib/objs/ -L/usr/lib/x86_64-linux-gnu -locr -lc -lpthread -lhello $(LINK_OPTS)
#	$(CC) -v -o $@ cmain.o $(OCR_LIB) -L$(APPS_ROOT)/legacy/x86_native -lhello $(LINK_OPTS)
#	$(LD) -static -o $@ $(GLIBC_LIBPREFIX) $(GLIBC_LIB_PATH) cmain.o -lhello -locr -L/usr/lib/x86_64-linux-gnu -lc -lpthread $(GLIBC_LIBSUFFIX)
#	$(LD) -static -o hello $(APP_OBJS) $(NL_LIBSUFFIX) -L$(APPS_ROOT)/../ocr/build/x86_lib/objs/ -locr -L/usr/lib/x86_64-linux-gnu -lc -lpthread
#	nm -u libhello.so | sed 's/^.*U //' > syms
#	objcopy -S --keep-symbols keep --keep-symbols syms libhello.so
#	$(CC) -v -o $@ cmain.o hello.o -L$(APPS_ROOT)/../ocr/build/x86_lib/objs/ -locr
#	$(CC) -o $@ -lhello $(OCR_LIB) -lthread_db
#	$(LD) -r -s -o app.o --start-group $< $(CRT0) $(LIBC) --end-group # Link against newlib
#	$(LD) -nostartfiles -nodefaultlibs -nostdlib -static -o $@ $(OCR_LIB) myapp.o -zmuldefs
#	$(LD) -g -o $@ $(NL_LIBPREFIX) $< $(APP_LIBS) $(NL_LIBSUFFIX) $(OCR_LIB)
	@echo Successfully built $@

%.o : %.c
	$(CC) -g -fPIC -O0 -c $(CFLAGS) $< -o $@

dump:
	@echo APPS_ROOT = $(APPS_ROOT)
	@echo LIBS = $(LIBS)

clean:
	@rm -f $(APP) $(APP_OBJS) *.so *.o
