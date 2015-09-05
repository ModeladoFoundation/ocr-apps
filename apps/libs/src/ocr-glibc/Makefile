#
# Makefile for compiling the OCR glibc support using the platform compiler
# We make 2 libs:
#	libocr-glibc.a		for use with C programs
#	libocr-glibcxx.a	for use with C++ programs
#
ARCH=x86_native

APPS_ROOT  ?= $(realpath $(CURDIR)/../../..)
APPS_LIBS_ROOT ?= $(APPS_ROOT)/libs/$(ARCH)

OCR_LIBC = $(realpath ../newlib/newlib-2.1.0/newlib/libc)
OCR_GLIBC_BUILD = $(CURDIR)
BUILD_DIR = build-$(ARCH)
SRCDIR = src
OCR_SRCDIR  = $(OCR_LIBC)/sys/ocr

OCR_SRCS = gettod.c inode.c io.c isatty.c malloc.c ocr_shim.c
LIB_SRCS = $(SRCDIR)/cxxmain.cc $(SRCDIR)/cmain.c

OCR_OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(OCR_SRCS))
COBJS = $(BUILD_DIR)/cmain.o
CXXOBJS = $(BUILD_DIR)/cxxmain.o

.SECONDEXPANSION:
#
# The rules for the specific libs will add their own files to OBJS
#
OBJS = $(OCR_OBJS)
CLIB = $(BUILD_DIR)/libocr-glibc.a
CXXLIB = $(BUILD_DIR)/libocr-glibcxx.a

OPT = -g
DEFINES = -D_OCR_GLIBC
INCLUDES = -I./include

CFLAGS += $(INCLUDES) -std=c99 $(OPT) $(DEFINES)
CXXFLAGS += $(INCLUDES) -std=c++11 $(OPT) $(DEFINES)

AR_FLAGS = qs

##########################
# Make Targets
#

all: setup $(CLIB) $(CXXLIB)
	@echo Done making all

default: all

# Always try to build these when we get here
.PHONY: $(CLIB) $(CXXLIB) setup clean
#
# Create a build dir and link to the sys/ocr/sys/ocr.h since we can't use
# it in place.
#
setup:
	@rm -rf $(BUILD_DIR); mkdir $(BUILD_DIR);
	@[ -d include/sys ] || mkdir -p include/sys
	@[ -e include/sys/ocr.h ] || ln -s  $(OCR_SRCDIR)/sys/ocr.h include/sys/ocr.h

#
# Build libocr-glibc*
#
$(CXXLIB): OBJS += $(CXXOBJS)
$(CLIB): OBJS += $(COBJS)

$(CXXLIB) $(CLIB): $$(OBJS)
	@echo Building $@ ...
	@$(AR) $(AR_FLAGS) $@ $(OBJS)
	@echo Successfully built $@!
#
# Rules to build our various objects
#
$(BUILD_DIR)/%.o : $(OCR_SRCDIR)/%.c
	@echo "  - $@"
	@$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(BUILD_DIR)/%.o : $(SRCDIR)/%.c
	@echo "  - $@"
	@$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(BUILD_DIR)/%.o : $(SRCDIR)/%.cc
	@echo "  - $@"
	@$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@

#
# Copy lib to the actual root install tree
#
.PHONY: install
install: root_install
root_install:
	@echo Installing $(CLIB) and $(CXXLIB) to $(APPS_LIBS_ROOT) ...
	@mkdir -p $(APPS_LIBS_ROOT)
	@cp -u -R $(CLIB) $(APPS_LIBS_ROOT)/lib
	@cp -u -R $(CXXLIB) $(APPS_LIBS_ROOT)/lib
	@echo Installation complete

.PHONY: uninstall
uninstall: # Doesn't actually do anything.
	@echo Uninstall is not implemented for this library

clean:
	@-if test -d $(OCR_GLIBC_BUILD) ; then          \
		cd $(OCR_GLIBC_BUILD);                     \
		rm -rf $(BUILD_DIR); \
	 fi
	@rm -rf include/sys
