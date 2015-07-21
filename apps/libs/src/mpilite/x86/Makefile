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

SRCS := ../mpilite.c ../mpi_ocr.c ../mpi_ocr_messaging.c
OBJS := mpilite.o mpi_ocr.o mpi_ocr_messaging.o
INCLUDES := ../mpi.h ../mpi_ocr_messaging.h ../mpi_ocr.h
LIB := libmpilite.a

# installed files
INSTALL_MPI_H := ${APPS_LIBS_ROOT}/include/mpi.h
INSTALL_MPI_MESS_H := ${APPS_LIBS_ROOT}/include/mpi_ocr_messaging.h
INSTALL_MPI_A := ${APPS_LIBS_ROOT}/lib/$(LIB)

CC := gcc
AR := ar
# add members AND generate index. Ranlib not needed.
ARFLAGS := rsv
RANLIB := ranlib

# 4/15/15 Mark: doesn't hurt to use -O0 all the time during development
OPT_FLAGS := -O0
INC_FLAGS := -I.. -I$(OCR_ROOT)/inc
CFLAGS := -std=c99 -g
DEFINES := -DOCR_ENABLE_EDT_NAMING

CFLAGS := $(CFLAGS) $(INC_FLAGS)  $(OPT_FLAGS)

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

$(INSTALL_MPI_H): ../mpi.h
	cp $< $@

# Not used any more by users
#$(INSTALL_MPI_MESS_H): ../mpi_ocr_messaging.h
#	cp $< $@

$(INSTALL_MPI_A): $(LIB)
	cp $< $@

.PHONY: install
install: $(INSTALL_MPI_H) $(INSTALL_MPI_A)

installold: $(INCLUDES) $(LIB)
	@echo "Installing X86 standard files"
	@cp  $(INCLUDES) ${APPS_LIBS_ROOT}/include/
	@cp  $(LIB) ${APPS_LIBS_ROOT}/lib/

.PHONY: run
run:

.PHONY: clean
clean:
	rm -f $(OBJS) $(LIB)

.PHONY: uninstall
uninstall:
	for i in mpi.h; do rm -f ${APPS_LIBS_ROOT}/include/$$i ; done
	rm -f $(APPS_LIBS_ROOT)/lib/$(LIB)

.PHONY: squeeky
squeeky: clean uninstall
