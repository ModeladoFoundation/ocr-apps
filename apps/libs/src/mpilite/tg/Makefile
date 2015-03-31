# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.


# Library environment
APPS_LIBS_ROOT ?= ../../../tg
APPS_ROOT ?= ../../../..


APPS_LIBS_ROOT := $(shell cd "${APPS_LIBS_ROOT}" && pwd)
APPS_ROOT      := $(shell cd "${APPS_ROOT}" && pwd)

# OCR top level directory
#
OCR_ROOT ?= ../../../../../ocr
OCR_ROOT := $(shell cd "${OCR_ROOT}" && pwd)

TG_ROOT        ?= ../../../../../../intel/ss
TG_INSTALL     ?= ../../../../../../intel/ss/install
TG_ROOT        := $(shell cd "${TG_ROOT}" && pwd)
TG_INSTALL     := $(shell cd "${TG_INSTALL}" && pwd)

TG_PREFIX     := $(TG_INSTALL)/bin/rmd-linux-elf
OCR_INSTALL   := $(OCR_ROOT_BASE)/tg-null-tg-xe

CRLIBM_ROOT    ?= ../../crlibm-1.0beta4
CRLIBM_ROOT    := $(shell cd "${CRLIBM_ROOT}" && pwd)

AS	= $(TG_PREFIX)-as
ASFLAGS	=

AR	= $(TG_PREFIX)-ar
ARFLAGS	= rcs

CC	= $(TG_PREFIX)-clang -DOCR -DTG_ARCH -frmd-extensions

CP = cp
RM = rm

CFLAGS	= -g -ggdb -g3 -fno-builtin -DHAVE_CONFIG_H -I.

## -DPROCESSOR_HAS_FMA=1

OBJCOPY	= $(TG_PREFIX)-objcopy

SRCS := ../mpilite.c ../mpi_ocr.c
OBJS := mpilite.o mpi_ocr.o
INCLUDES := ../mpi.h ../mpi_ocr_messaging.h
LIB := libmpilite.a

# crlibm_root/tg has stdio.h stdlib.h for TG
INC_FLAGS := -I.. -I$(OCR_ROOT)/inc -I$(CRLIBM_ROOT)/tg

# following would have any tg-specific ocr.h differences, but it doesn't happen 
# to be built at the moment
#INC_FLAGS := -I..  -I$(OCR_INSTALL)/include
CFLAGS += -std=c99

CFLAGS := -g $(CFLAGS) $(INC_FLAGS)

.PHONY: all
all: install

.PHONY: debug
debug:

.PHONY: compile
compile: $(OBJS)

.PHONY: link
link: $(LIB)

%.o: ../%.c Makefile $(INCLUDES)
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@


$(LIB): $(OBJS)
	@echo "Linking static library ${$LIB}"
	@$(AR) $(ARFLAGS) $(LIB) $^

.PHONY: install
install: $(INCLUDES) $(LIB)
	@echo "Installing X86 standard files"
	@cp --remove-destination $(INCLUDES) ${APPS_LIBS_ROOT}/include/
	@cp --remove-destination $(LIB) ${APPS_LIBS_ROOT}/lib/

.PHONY: run
run:

.PHONY: clean
clean:
	rm -f $(OBJS) $(LIB)

.PHONY: uninstall
uninstall:
	for i in mpi.h mpi_ocr_messaging.h; do rm -f ${APPS_LIBS_ROOT}/include/$$i ; done
	rm -f $(APPS_LIBS_ROOT)/lib/$(LIB)

.PHONY: squeeky
squeeky: clean uninstall
