# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

ARCH                 	?= x86
OCR_TYPE                ?= x86
APPS_ROOT               ?= ../../../..

_INTERNAL_NO_CHECK_WORKLOAD := yes
include $(APPS_ROOT)/makefiles/make-varcheck.inc

SRCS := ../mpilite.c ../mpi_ocr.c ../mpi_ocr_messaging.c
OBJS := mpilite.o mpi_ocr.o mpi_ocr_messaging.o
INCLUDES := ../mpi.h ../mpi_ocr_messaging.h ../mpi_ocr.h
LIB := libmpilite.a

# installed files
INSTALL_MPI_H := ${APPS_LIBS_INSTALL}/include/mpi.h
INSTALL_MPI_MESS_H := ${APPS_LIBS_INSTALL}/include/mpi_ocr_messaging.h
INSTALL_MPI_A := ${APPS_LIBS_INSTALL}/lib/$(LIB)

CC := gcc
AR := ar
# add members AND generate index. Ranlib not needed.
ARFLAGS := rsv
RANLIB := ranlib

# USER_FLAGS, for any special -D defines, like -DDEBUG_MPI=1
USER_FLAGS ?=
OPT_FLAGS := -O2
INC_FLAGS := -I.. -I$(OCR_ROOT)/inc
CFLAGS := -std=c99 -g
DEFINES := -DOCR_ENABLE_EDT_NAMING

CFLAGS := $(CFLAGS) $(INC_FLAGS)  $(OPT_FLAGS) $(USER_FLAGS) -DOCR_TYPE_H=$(OCR_TYPE).h

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
	$(AT)$(CC) $(CFLAGS) -c $< -o $@


$(LIB): $(OBJS)
	@echo "Linking static library ${$LIB}"
	$(AT)$(AR) $(ARFLAGS) $(LIB) $^

$(INSTALL_MPI_H): ../mpi.h | $(APPS_LIBS_INSTALL)/include
	$(AT)cp $< $@

# Not used any more by users
#$(INSTALL_MPI_MESS_H): ../mpi_ocr_messaging.h
#	cp $< $@

$(INSTALL_MPI_A): $(LIB) | $(APPS_LIBS_INSTALL)/lib
	$(AT)cp $< $@

.PHONY: install
install: $(INSTALL_MPI_H) $(INSTALL_MPI_A)

installold: $(INCLUDES) $(LIB)
	@echo "Installing X86 standard files"
	$(AT)cp  $(INCLUDES) ${APPS_LIBS_ROOT}/include/
	$(AT)cp  $(LIB) ${APPS_LIBS_ROOT}/lib/

.PHONY: $(APPS_LIBS_INSTALL)/include $(APPS_LIBS_INSTALL)/lib
$(APPS_LIBS_INSTALL)/include $(APPS_LIBS_INSTALL)/lib:
	$(AT)mkdir -p $@
.PHONY: run
run:

.PHONY: clean
clean:
	rm -f $(OBJS) $(LIB)

.PHONY: uninstall
uninstall:
	$(AT)for i in mpi.h; do rm -f ${APPS_LIBS_INSTALL}/include/$$i ; done
	$(AT)rm -f $(APPS_LIBS_INSTALL)/lib/$(LIB)

.PHONY: squeeky
squeeky: clean uninstall
