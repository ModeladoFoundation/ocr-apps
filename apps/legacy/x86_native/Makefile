#
# Makefile
# vim:syntax=make noexpandtab:
#
# To run C example: ./chello -ocr:cfg ./generated.cfg
# To run C++ example: ./cxxhello -ocr:cfg ./generated.cfg
#
ARCH=x86_native

APPS_ROOT  ?= $(realpath $(CURDIR)/../..)

APPS_LIBS_ROOT = $(APPS_ROOT)/libs/$(ARCH)
LIBS = $(APPS_LIBS_ROOT)/lib
OCR_INC = $(APPS_LIBS_ROOT)/include

CFLAGS += -std=c99 -U__STRICT_ANSI__ -isystem $(OCR_INC)

CXXFLAGS += -U__STRICT_ANSI__ -D__XSTACK__ -D__LITTLE_ENDIAN__ -isystem $(OCR_INC)
CXXFLAGS += -fno-stack-protector -fno-use-cxa-atexit
CXXFLAGS += -I$(APPS_LIBS_ROOT)/include/c++/v1
CXXFLAGS += -I$(APPS_LIBS_ROOT)/include

LIBC = /usr/lib/x86_64-linux-gnu/libc.a
PTHREAD_LIB = /usr/lib/x86_64-linux-gnu/libpthread.a
LIB_GCC = /usr/lib/gcc/x86_64-linux-gnu/4.9/libgcc_eh.a
GLIBC_A = $(PTHREAD_LIB) $(LIB_GCC) $(LIBC)

# Libraries to include from newlib
NL_LIBS = $(LIBS)/libc.a $(LIBS)/libm.a
LIBCXX_LIBS = $(LIBS)/libc++.a $(LIBS)/libc++abi.a $(LIBS)/libunwind.a

OCR_LIB_PATH = $(APPS_ROOT)/../ocr/build/x86_lib/objs

# OCR library - this can be the libocrscaffold.a, or
# the real ocr library/runtime. Uncomment the one you want.
#OCR_LIB = $(LIBS)/libocrscaffold.a
OCR_LIB = $(OCR_LIB_PATH)/libocr.o $(GLIBC_A)

CAPP = chello
CAPP_SRCS = chello.c
CAPP_LIBS = $(NL_LIBS)
CAPP_OBJS = $(CAPP_SRCS:.c=.o)

CXXAPP = cxxhello
CXXAPP_SRCS = cxxhello.cc
CXXAPP_LIBS = $(LIBCXX_LIBS) $(NL_LIBS)
CXXAPP_OBJS = $(CXXAPP_SRCS:.cc=.o)

PREMAIN_SRC = premain.cc
PREMAIN_OBJ = $(PREMAIN_SRC:.cc=.o)

all: $(CAPP) $(CXXAPP)

default: all

$(CAPP) : $(PREMAIN_OBJ) $(CAPP_OBJS) $(CAPP_LIBS)
	@$(LD) -r -static -o applib.o $(PREMAIN_OBJ) $(CAPP_OBJS) $(CAPP_LIBS)
	@nm applib.o | grep -vE " U |main|__errno"  | sed 's/^.* . //' > s
	@objcopy --localize-symbols=s applib.o
	@$(CC) -static -o $@ applib.o $(OCR_LIB) -T elf64_x86_64.t
	@echo Successfully built $@

$(CXXAPP) : $(PREMAIN_OBJ) $(CXXAPP_OBJS) $(CXXAPP_LIBS)
	@$(LD) -r -static -o applib.o $(PREMAIN_OBJ) $(CXXAPP_OBJS) $(CXXAPP_LIBS)
	@nm applib.o | grep -vE " U |main|__errno"  | sed 's/^.* . //' > s
	@objcopy --localize-symbols=s applib.o
	@$(CXX) -static -o $@ applib.o $(OCR_LIB) -T elf64_x86_64.t
	@echo Successfully built $@

%.o : %.c
	@$(CC) -g -O0 -c $(CFLAGS) $< -o $@

%.o : %.cc
	@$(CXX) -g -O0 -c $(CXXFLAGS) $< -o $@

dump:
	@echo APPS_ROOT = $(APPS_ROOT)
	@echo LIBS = $(LIBS)

clean:
	@rm -f $(CAPP) $(CAPP_OBJS) $(CXXAPP) $(CXXAPP_OBJS) $(PREMAIN_OBJ) *.o s
