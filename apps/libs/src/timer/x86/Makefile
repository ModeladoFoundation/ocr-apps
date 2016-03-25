# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

ARCH                 	?= x86
OCR_TYPE                ?= x86
APPS_ROOT               ?= ../../../..

_INTERNAL_NO_CHECK_WORKLOAD := yes
include $(APPS_ROOT)/makefiles/make-varcheck.inc

SRCS := timer.c
OBJS := timer.o
INCLUDES := ../timer.h
LIB := timer.a

# installed files
INSTALL_TIMER_H := ${APPS_LIBS_INSTALL}/include/timer.h
INSTALL_TIMER_A := ${APPS_LIBS_INSTALL}/lib/libtimer.a

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
	$(AT)$(CC) $(CFLAGS) -c $< -o $@


$(LIB): $(OBJS)
	@echo "Linking static library ${$LIB}"
	$(AT)$(AR) $(ARFLAGS) $(LIB) $^

$(INSTALL_TIMER_H): timer.h | $(APPS_LIBS_INSTALL)/include
	$(AT)cp $< $@

$(INSTALL_TIMER_A): $(LIB) | $(APPS_LIBS_INSTALL)/lib
	$(AT)cp $< $@

.PHONY: install
install: $(INSTALL_TIMER_H) $(INSTALL_TIMER_A)

.PHONY: run
run:

.PHONY: clean
clean:
	$(AT)rm -f $(OBJS) $(LIB)

.PHONY: uninstall
uninstall:
	$(AT)for i in timer.h; do rm -f ${APPS_LIBS_INSTALL}/include/$$i ; done
	$(AT)rm -f $(APPS_LIBS_INSTALL)/lib/$(LIB)

.PHONY: squeaky
squeaky: clean uninstall

.PHONY: $(APPS_LIBS_INSTALL)/include $(APPS_LIBS_INSTALL)/lib
$(APPS_LIBS_INSTALL)/include $(APPS_LIBS_INSTALL)/lib:
	$(AT)mkdir -p $@
