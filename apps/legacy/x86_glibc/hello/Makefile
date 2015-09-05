#
# Makefile
# vim:syntax=make noexpandtab:
#
ARCH=x86_native

APPS_ROOT  ?= $(realpath $(CURDIR)/../../..)

APPS_LIBS_ROOT = $(APPS_ROOT)/libs/$(ARCH)
LIBS = $(APPS_LIBS_ROOT)/lib

CFLAGS += -I../include -std=c99 -g

# These are the ocr shim libraries.  See
# libs/src/ocr-glibc/README for more info.
OCR_SHIM_CLIB = $(LIBS)/libocr-glibc.a
OCR_SHIM_CXXLIB = $(LIBS)/libocr-glibcxx.a

# Other library - this uses the libocrscaffoldi-glibc.a, but it
# should actually be the real ocr library/runtime.
OCR_LIB = $(LIBS)/libocrscaffold-glibc.a

CAPP = chello
CAPP_SRCS = chello.c
CAPP_LIBS =
CAPP_OBJS = $(CAPP_SRCS:.c=.o)

CXXAPP = cxxhello
CXXAPP_SRCS = cxxhello.cc
CXXAPP_LIBS =
CXXAPP_OBJS = $(CXXAPP_SRCS:.cc=.o)

all: $(CAPP) $(CXXAPP)

default: all

$(CAPP) : $(OCR_SHIM_CLIB) $(CAPP_LIBS) $(OCR_LIB)
	$(CC) -static -o $@ $(CAPP_SRCS) $^
	@echo Successfully built $@

$(CXXAPP) : $(OCR_SHIM_CXXLIB) $(CXXAPP_LIBS) $(OCR_LIB)
	$(CXX) -static -o $@ $(CXXAPP_SRCS) $^
	@echo Successfully built $@

dump:
	@echo APPS_ROOT = $(APPS_ROOT)
	@echo LIBS = $(LIBS)

clean:
	@rm -f $(CAPP) $(CXXAPP) $(CAPP_OBJS) $(CXXAPP_OBJS)
