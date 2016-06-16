# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

#ARCH                 	?= x86
#OCR_TYPE                ?= x86
APPS_ROOT               ?= ../../../

_INTERNAL_NO_CHECK_WORKLOAD := yes
include $(APPS_ROOT)/makefiles/make-varcheck.inc

SRCS := ocr-reservations.c
OBJS := ocr-reservations.o
INCLUDES := ocr-reservations.h
LIB := libocr-reservations.a

# installed files
INSTALL_RESERVATION_H := ${APPS_LIBS_INSTALL}/include/ocr-reservations.h
INSTALL_RESERVATION_A := ${APPS_LIBS_INSTALL}/lib/libocr-reservations.a

CC := gcc
AR := ar
# add members AND generate index. Ranlib not needed.
ARFLAGS := rsv
RANLIB := ranlib

OPT_FLAGS := -O3
INC_FLAGS := -I.. -I$(OCR_ROOT)/inc
CFLAGS := -g
CFLAGS += -DENABLE_EXTENSION_PARAMS_EVT
CFLAGS += -DENABLE_EXTENSION_CHANNEL_EVT

#DEFINES := -DOCR_ENABLE_EDT_NAMING

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
	$(AT)$(CC) $(CFLAGS) -c $< -o $@

$(LIB): $(OBJS)
	@echo "Linking static library ${$LIB}"
	$(AT)$(AR) $(ARFLAGS) $(LIB) $^

$(INSTALL_RESERVATION_H): ocr-reservations.h | $(APPS_LIBS_INSTALL)/include
	$(AT)cp $^ $@

$(INSTALL_RESERVATION_A): $(LIB) | $(APPS_LIBS_INSTALL)/lib
	$(AT)cp $< $@

.PHONY: install
install: $(INSTALL_RESERVATION_H) $(INSTALL_RESERVATION_A)

.PHONY: run
run:

.PHONY: clean
clean:
	$(AT)rm -f $(OBJS) $(LIB)

.PHONY: uninstall
uninstall:
	$(AT)for i in ocr-reservations.h; do rm -f ${APPS_LIBS_INSTALL}/include/$$i ; done
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
