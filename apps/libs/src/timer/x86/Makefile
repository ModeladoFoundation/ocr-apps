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

SRCS := timer.c
OBJS := timer.o
INCLUDES := ../timer.h
LIB := timer.a

# installed files
INSTALL_TIMER_H := ${APPS_LIBS_ROOT}/include/timer.h
INSTALL_TIMER_A := ${APPS_LIBS_ROOT}/lib/libtimer.a

CC := gcc
AR := ar
# add members AND generate index. Ranlib not needed.
ARFLAGS := rsv
RANLIB := ranlib

INC_FLAGS := -I.. -I$(OCR_ROOT)/inc
CFLAGS := -std=c99 -g
DEFINES := -DOCR_ENABLE_EDT_NAMING

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

$(INSTALL_TIMER_H): timer.h
	cp $< $@

$(INSTALL_TIMER_A): $(LIB)
	cp $< $@

.PHONY: install
install: $(INSTALL_TIMER_H) $(INSTALL_TIMER_A)

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
	for i in timer.h; do rm -f ${APPS_LIBS_ROOT}/include/$$i ; done
	rm -f $(APPS_LIBS_ROOT)/lib/$(LIB)

.PHONY: squeaky
squeaky: clean uninstall
