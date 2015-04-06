# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.


# Library environment
APPS_LIBS_ROOT ?= ../../../x86
APPS_ROOT ?= ../../../..


APPS_LIBS_ROOT := $(shell cd "${APPS_LIBS_ROOT}" && pwd)
APPS_ROOT      := $(shell cd "${APPS_ROOT}" && pwd)

# OCR top level directory
#
OCR_ROOT ?= ../../../../../ocr
OCR_ROOT := $(shell cd "${OCR_ROOT}" && pwd)

SRCS := ../mpilite.c ../mpi_ocr.c
OBJS := mpilite.o mpi_ocr.o
INCLUDES := ../mpi.h ../mpi_ocr_messaging.h
LIB := libmpilite.a

CC := gcc
AR := ar
# add members AND generate index. Ranlib not needed.
ARFLAGS := rsv
RANLIB := ranlib

INC_FLAGS := -I.. -I$(OCR_ROOT)/inc
CFLAGS := -std=c99 -g -O0

CFLAGS := $(CFLAGS) $(INC_FLAGS)

.PHONY: all
all: install

.PHONY: debug
debug: CFLAGS += -g -O0
debug: install

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

