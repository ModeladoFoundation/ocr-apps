#
#
# vim:noexpandtab:
#
# Compile and link an x86_64 app with newlib and the OCR scaffold ocr.c
# Uses the xe-llvm-3.5.0 compiler with x86_64 target
# The linux system call interface is implemented via syscall.o which was
# extracted from the platform libc.a.
#
ARCH           ?= x86

APPS_ROOT      ?= ../../../../../
APPS_LIBS_ROOT ?= $(APPS_ROOT)/libs/$(ARCH)

WORKLOAD_BUILD_SRC    ?= .
WORKLOAD_BUILD_ROOT   ?= ../build/$(ARCH)
WORKLOAD_INSTALL_ROOT ?= ../install/$(ARCH)

# Make into absolute path
APPS_ROOT      := $(shell cd "${APPS_ROOT}" && pwd)
APPS_LIBS_ROOT := $(shell cd "${APPS_LIBS_ROOT}" && pwd)

WORKLOAD_SRC          := $(shell cd "${WORKLOAD_SRC}" && pwd)
WORKLOAD_BUILD_ROOT   := $(shell mkdir -p "${WORKLOAD_BUILD_ROOT}" && cd "${WORKLOAD_BUILD_ROOT}" && pwd)
WORKLOAD_INSTALL_ROOT := $(shell mkdir -p "${WORKLOAD_INSTALL_ROOT}" && cd "${WORKLOAD_INSTALL_ROOT}" && pwd)

# Select appropriate executables based on ARCH
ifeq ($(ARCH), tg)
TG_ROOT        ?= ../../../../../../intel/ss
TG_INSTALL     ?= $(TG_ROOT)/install

PREFIX         := $(TG_INSTALL)/bin/rmd-linux-elf-
CC             := clang
RANLIB         := ranlib
else ifeq ($(ARCH), x86)
PREFIX         := /usr/bin/
RANLIB         := ranlib
else
$(error "Unknown ARCH")
endif

CC     := $(PREFIX)$(CC)
AS     := $(PREFIX)$(AS)
AR     := $(PREFIX)$(AR)
RANLIB := $(PREFIX)$(RANLIB)
LD     := $(PREFIX)$(LD)

# Path to newlib
NL_INC = $(APPS_LIBS_ROOT)/include
NL_LIB = $(APPS_LIBS_ROOT)/lib

# Libraries to include from newlib
NL_LIBPREFIX = $(NL_LIB)/crt0.o
NL_LIBSUFFIX = $(NL_LIB)/libc.a

# Parameters for the ocrshim library
OCRSHIM_DEFINES  :=
OCRSHIM_INCLUDES := -isystem /usr/include -isystem /usr/include/x86_64-linux-gnu -isystem $(NL_INC)
OCRSHIM_LIBS     := $(WORKLOAD_INSTALL_ROOT)/libocrshim.a
OCRSHIM_OBJS     := $(WORKLOAD_BUILD_ROOT)/ocrshim.o $(WORKLOAD_BUILD_ROOT)/syscall.o

# Paramters for the test application (ocrtest.c)
TEST_DEFINES     :=
TEST_INCLUDES    := -isystem $(NL_INC)
TEST_OBJS        := ocrtest.o
TEST_EXE         := ocrtest

# Common CFLAGS
CFLAGS += -std=c11 -g -fno-builtin

# Targets
.PHONY: all
all: CFLAGS += -O2
all: $(OCRSHIM_OBJS)

.PHONY: debug
debug: CFLAGS += -O0
debug: $(OCRSHIM_OBJS)

.PHONY: install
install: $(OCRSHIM_LIBS)

.PHONY: uninstall
uninstall:
	rm -f $(OCRSHIM_LIBS)

.PHONY: clean
clean:
	rm -f $(TEST_OBJS) $(OCRSHIM_OBJS) ocrtest

.PHONY: test
test: $(TEST_EXE)

# Rules
$(TEST_EXE): $(TEST_OBJS) $(OCRSHIM_LIBS)
	$(LD) -g -o $@ $(NL_LIBPREFIX) $(TEST_OBJS) $(NL_LIBSUFFIX) $(OCRSHIM_LIBS)

$(OCRSHIM_LIBS): $(OCRSHIM_OBJS) | $(WORKLOAD_INSTALL_ROOT)
	$(RM) -f $@
	$(AR) cru $@ $(OCRSHIM_OBJS)
	$(RANLIB) $@

$(WORKLOAD_BUILD_ROOT)/%.o : $(WORKLOAD_SRC)/%.c | $(WORKLOAD_BUILD_ROOT)
	$(CC) $(CFLAGS) $(OCRSHIM_INCLUDES) -o $@ -c $<

$(WORKLOAD_BUILD_ROOT)/%.o : $(WORKLOAD_SRC)/%.S | $(WORKLOAD_BUILD_ROOT)
	$(AS) -v -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) $(TEST_INCLUDES) -o $@ -c $<

$(WORKLOAD_INSTALL_ROOT):
	mkdir -p $(WORKLOAD_INSTALL_ROOT)

$(WORKLOAD_BUILD_ROOT):
	mkdir -p $(WORKLOAD_BUILD_ROOT)
