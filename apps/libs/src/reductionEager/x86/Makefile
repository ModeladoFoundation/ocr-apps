# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

ARCH                 	?= x86
OCR_TYPE                ?= x86
APPS_ROOT               ?= ../../../..

_INTERNAL_NO_CHECK_WORKLOAD := yes
include $(APPS_ROOT)/makefiles/make-varcheck.inc

SRC_DIR      := ../
SRCS         := reductionEager.c
SRC_INCLUDES := reductionEager.h

SRCS         := $(addprefix $(SRC_DIR), $(SRCS))
SRC_INCLUDES := $(addprefix $(SRC_DIR), $(SRC_INCLUDES))
OBJS         := $(patsubst $(SRC_DIR)%.c, %.o, $(SRCS))

LIB := libreductionEager.a

# Other libraries required
REQ_LIBS := macros

# Installed files
INSTALL_HEADERS := $(patsubst $(SRC_DIR)%.h,$(APPS_LIBS_INSTALL)/include/%.h, $(SRC_INCLUDES))
INSTALL_LIBS    := $(APPS_LIBS_INSTALL)/lib/$(LIB)

### Compilation rules ####
PREFIX := $(TG_INSTALL)/bin/xstg-linux-elf

CC     ?= gcc
AR     := ar

# add members AND generate index. Ranlib not needed.
ARFLAGS := rsv

# 4/15/15 Mark: doesn't hurt to use -O0 all the time during development
OPT_FLAGS := -O3

INC_FLAGS := -I$(SRC_DIR) -I$(OCR_INSTALL)/include -I$(APPS_LIBS_INSTALL)/include

CFLAGS    += -g
CFLAGS    += -DENABLE_EXTENSION_PARAMS_EVT
CFLAGS    += -DENABLE_EXTENSION_COUNTED_EVT
CFLAGS    += -DENABLE_EXTENSION_CHANNEL_EVT

DEFINES   := -DOCR_TYPE_H=$(OCR_TYPE).h -DENABLE_EXTENSION_LABELING -DENABLE_EXTENSION_AFFINITY
#DEFINES  := -DOCR_ENABLE_EDT_NAMING

CFLAGS    := $(CFLAGS) $(INC_FLAGS) $(DEFINES) $(OPT_FLAGS)

.PHONY: all
all: install

.PHONY: debug
debug: CFLAGS += -g -O0
debug: install

.PHONY: compile
compile: $(OBJS)

.PHONY: link
link: $(LIB)


%.o: $(SRC_DIR)%.c Makefile $(SRC_INCLUDES) | reqlibs
	@echo "Compiling $<"
	$(AT)$(CC) $(CFLAGS) -c $< -o $@

$(LIB): $(OBJS)
	@echo "Linking static library ${$LIB}"
	$(AT)$(AR) $(ARFLAGS) $(LIB) $^

$(APPS_LIBS_INSTALL)/include/%.h: $(SRC_DIR)%.h | $(APPS_LIBS_INSTALL)/include
	$(AT)cp $^ $@

$(APPS_LIBS_INSTALL)/lib/$(LIB): $(LIB) | $(APPS_LIBS_INSTALL)/lib
	$(AT)cp $^ $@

.PHONY: install
install: $(INSTALL_HEADERS) $(INSTALL_LIBS)

.PHONY: run
run:

.PHONY: clean
clean:
	$(AT)rm -f $(OBJS) $(LIB)

.PHONY: uninstall
uninstall:
	$(AT)for i in $(INSTALL_HEADERS); do rm -f $$i ; done
	$(AT)rm -f $(INSTALL_LIBS)

.PHONY: squeeky
squeeky: clean uninstall

.PHONY: $(APPS_LIBS_INSTALL)/include
$(APPS_LIBS_INSTALL)/include:
	$(AT)mkdir -p $@

.PHONY: $(APPS_LIBS_INSTALL)/lib
$(APPS_LIBS_INSTALL)/lib:
	$(AT)mkdir -p $@

include $(APPS_ROOT)/makefiles/make-libs.inc
