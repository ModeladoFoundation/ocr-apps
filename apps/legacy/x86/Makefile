#
# Makefile
# vim:syntax=make noexpandtab:
#
# If run as `make LIBSCAFFOLD=yes' then the examples will be compiled
# against libscaffold instead of libocr.
#
# If compiled without LIBSCAFFOLD=yes then run like this:
#
# To run C example: ./chello -ocr:cfg ./test_config.cfg
# To run C++ example: ./cxxhello -ocr:cfg ./test_config.cfg
#
# If compiled with LIBSCAFFOLD=yes then run directly without arguments.
#
ARCH=x86
OCR_TYPE=x86-newlib

APPS_ROOT  ?= $(realpath $(CURDIR)/../..)

APPS_LIBS_INSTALL = $(APPS_ROOT)/libs/install/$(ARCH)
LIBS = $(APPS_LIBS_INSTALL)/lib
OCR_INC = $(APPS_LIBS_INSTALL)/include

CFLAGS += -std=c99 -U__STRICT_ANSI__ -isystem $(OCR_INC)

CXXFLAGS += -U__STRICT_ANSI__ -D__XSTACK__ -D__LITTLE_ENDIAN__ -isystem $(OCR_INC)
CXXFLAGS += -fno-stack-protector -fno-use-cxa-atexit
CXXFLAGS += -I$(APPS_LIBS_INSTALL)/include/c++/v1
CXXFLAGS += -I$(APPS_LIBS_INSTALL)/include

LIBC = `gcc -print-file-name=libc.a`
LIBPTHREAD = `gcc -print-file-name=libpthread.a`
LIBGCC_EH = `gcc -print-file-name=libgcc_eh.a`
GLIBC_A = $(LIBPTHREAD) $(LIBGCC_EH) $(LIBC)

# Libraries to include from newlib
NL_LIBS = $(LIBS)/libc.a $(LIBS)/libm.a
LIBCXX_LIBS = $(LIBS)/libc++.a $(LIBS)/libc++abi.a $(LIBS)/libunwind.a

ifeq ($(LIBSCAFFOLD),yes)
OCR_LIB = $(LIBS)/libscaffold.a
else
OCR_LIB_PATH = $(APPS_ROOT)/../../ocr/ocr/install/lib/
OCR_LIB = $(OCR_LIB_PATH)/libocr_$(OCR_TYPE).a $(GLIBC_A)
endif

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
	@echo "  - applib.o"
	@$(LD) -r -static -o applib.o $(CAPP_OBJS) $(CAPP_LIBS)
	@nm applib.o | grep -vE " U |main|__errno|_impure_ptr"  | sed 's/^.* . //' > s
	@objcopy --localize-symbols=s applib.o
	@echo "  - $@"
	@$(CC) -static -o $@ applib.o $(PREMAIN_OBJ) $(OCR_LIB) -T elf64_x86_64.t
	@echo Successfully built $@

$(CXXAPP) : $(PREMAIN_OBJ) $(CXXAPP_OBJS) $(CXXAPP_LIBS)
	@echo "  - applib.o"
	@$(LD) -r -static -zmuldefs -o applib.o $(CXXAPP_OBJS) $(CXXAPP_LIBS)
	@nm applib.o | grep -vE " U |main|__errno|_impure_ptr"  | sed 's/^.* . //' > s
	@objcopy --localize-symbols=s applib.o
	@echo "  - $@"
	@$(CXX) -static -o $@ applib.o $(PREMAIN_OBJ) $(OCR_LIB) -Wl,--eh-frame-hdr -T elf64_x86_64.t
	@echo Successfully built $@

%.o : %.c
	@echo "  - $@"
	@$(CC) -g -O0 -c $(CFLAGS) $< -o $@

%.o : %.cc
	@echo "  - $@"
	@$(CXX) -g -O0 -c $(CXXFLAGS) $< -o $@

dump:
	@echo APPS_ROOT = $(APPS_ROOT)
	@echo LIBS = $(LIBS)

clean:
	@rm -f $(CAPP) $(CAPP_OBJS) $(CXXAPP) $(CXXAPP_OBJS) $(PREMAIN_OBJ) *.o s
