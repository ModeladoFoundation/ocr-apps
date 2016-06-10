# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

ARCH                 	?= x86
OCR_TYPE                ?= x86
APPS_ROOT               ?= ../../../..

_INTERNAL_NO_CHECK_WORKLOAD := yes
include $(APPS_ROOT)/makefiles/make-varcheck.inc

SRCS := ../spmd.cpp
OBJS := spmd.o
INCLUDES := ../spmd.h
LIB := libspmd.a

# installed files
INSTALL_REDUCTION_H := ${APPS_LIBS_INSTALL}/include/spmd.h
INSTALL_REDUCTION_A := ${APPS_LIBS_INSTALL}/lib/libspmd.a

CC := g++
AR := ar
# add members AND generate index. Ranlib not needed.
ARFLAGS := rsv
RANLIB := ranlib

# 4/15/15 Mark: doesn't hurt to use -O0 all the time during development
OPT_FLAGS := -O3
INC_FLAGS := -I.. -I$(OCR_ROOT)/inc -I$(APPS_LIBS_INSTALL)/include
CFLAGS := -g
CFLAGS += -DENABLE_EXTENSION_AFFINITY
CFLAGS += -DENABLE_EXTENSION_PARAMS_EVT
CFLAGS += -DENABLE_EXTENSION_CHANNEL_EVT
CFLAGS += -DENABLE_EXTENSION_RTITF

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

%.o: ../%.cpp Makefile $(INCLUDES) | reqlibs
	@echo "Compiling $<"
	$(AT)$(CC) $(CFLAGS) -c $< -o $@

$(LIB): $(OBJS)
	@echo "Linking static library ${$LIB}"
	$(AT)$(AR) $(ARFLAGS) $(LIB) $^

$(INSTALL_REDUCTION_H): ../spmd.h | $(APPS_LIBS_INSTALL)/include
	$(AT)cp $^ $@

$(INSTALL_REDUCTION_A): $(LIB) | $(APPS_LIBS_INSTALL)/lib
	$(AT)cp $< $@

.PHONY: install
install: $(INSTALL_REDUCTION_H) $(INSTALL_REDUCTION_A)

.PHONY: run
run:

.PHONY: clean
clean:
	$(AT)rm -f $(OBJS) $(LIB)

.PHONY: uninstall
uninstall:
	$(AT)for i in spmd.h ; do rm -f ${APPS_LIBS_INSTALL}/include/$$i ; done
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
