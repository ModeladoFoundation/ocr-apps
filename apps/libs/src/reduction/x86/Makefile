# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.


SRCS := ../reduction.c ../reduction_v1.c
OBJS := reduction.o reduction_v1.o
INCLUDES := ../reduction.h ../reduction_v1.h
LIB := libreduction.a

# installed files
INSTALL_REDUCTION_H := ${APPS_LIBS_INSTALL}/include/reduction.h
INSTALL_REDUCTION_H1 := ${APPS_LIBS_INSTALL}/include/reduction_v1.h
INSTALL_REDUCTION_A := ${APPS_LIBS_INSTALL}/lib/libreduction.a

CC := gcc
AR := ar
# add members AND generate index. Ranlib not needed.
ARFLAGS := rsv
RANLIB := ranlib

# 4/15/15 Mark: doesn't hurt to use -O0 all the time during development
OPT_FLAGS := -O0
INC_FLAGS := -I.. -I$(OCR_ROOT)/inc -I$(APPS_LIBS_INSTALL)/include
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

REQ_LIBS := macros

%.o: ../%.c Makefile $(INCLUDES) | reqlibs
	@echo "Compiling $<"
	$(AT)$(CC) $(CFLAGS) -c $< -o $@

$(LIB): $(OBJS)
	@echo "Linking static library ${$LIB}"
	$(AT)$(AR) $(ARFLAGS) $(LIB) $^

$(INSTALL_REDUCTION_H): ../reduction.h | $(APPS_LIBS_INSTALL)/include
	$(AT)cp $^ $@

$(INSTALL_REDUCTION_H1): ../reduction_v1.h | $(APPS_LIBS_INSTALL)/include
	$(AT)cp $^ $@

# Not used any more by users
#$(INSTALL_REDUCTION_MESS_H): ../mpi_ocr_messaging.h
#	cp $< $@

$(INSTALL_REDUCTION_A): $(LIB) | $(APPS_LIBS_INSTALL)/lib
	$(AT)cp $< $@

.PHONY: install
install:  $(INSTALL_REDUCTION_H) $(INSTALL_REDUCTION_H1) $(INSTALL_REDUCTION_A)

.PHONY: run
run:

.PHONY: clean
clean:
	$(AT)rm -f $(OBJS) $(LIB)

.PHONY: uninstall
uninstall:
	$(AT)for i in reduction.h reduction_v1.h; do rm -f ${APPS_LIBS_INSTALL}/include/$$i ; done
	$(AT)rm -f $(APPS_LIBS_INSTALL)/lib/$(LIB)

.PHONY: squeeky
squeeky: clean uninstall

.PHONY: $(APPS_LIBS_INSTALL)/include
$(APPS_LIBS_INSTALL)/include:
	$(AT)mkdir -p $@

.PHONY: $(APPS_LIBS_INSTALL)/lib
$(APPS_LIBS_INSTALL)/lib:
	$(AT)mkdir -p $@

include $(APPS_ROOT)/makefiles/make-libs.inc
