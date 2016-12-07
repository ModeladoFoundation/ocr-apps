# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

ARCH                 	?= tg-xe
OCR_TYPE                ?= tg
APPS_ROOT               ?= ../../../..

_INTERNAL_NO_CHECK_WORKLOAD := yes
include $(APPS_ROOT)/makefiles/make-varcheck.inc

SRC_DIR      := ../
SRCS         := reduction.c reduction_v1.c
SRC_INCLUDES := reduction.h reduction_v1.h

SRCS         := $(addprefix $(SRC_DIR), $(SRCS))
SRC_INCLUDES := $(addprefix $(SRC_DIR), $(SRC_INCLUDES))
OBJS         := $(patsubst $(SRC_DIR)%.c, %.o, $(SRCS))

LIB := libreduction.a

# Other libraries required
REQ_LIBS := macros libc

# Installed files
INSTALL_HEADERS := $(patsubst $(SRC_DIR)%.h,$(APPS_LIBS_INSTALL)/include/%.h, $(SRC_INCLUDES))
INSTALL_LIBS    := $(APPS_LIBS_INSTALL)/lib/$(LIB)

### Compilation rules ####
PREFIX := $(TG_INSTALL)/bin/xstg-linux-elf

CC     := $(PREFIX)-clang
AR     := $(PREFIX)-ar

# add members AND generate index. Ranlib not needed.
ARFLAGS := rsv

# 4/15/15 Mark: doesn't hurt to use -O0 all the time during development
OPT_FLAGS := -O3

INC_FLAGS := -I$(SRC_DIR) -I$(OCR_INSTALL)/include

# Special case to include old libc files
USE_OLD_LIBC := $(strip $(foreach count, $(shell seq -s ' ' $(words $(REQ_LIBS))), $(if $(findstring libc,$(word $(count), $(REQ_LIBS))),yes)))

ifeq ($(USE_OLD_LIBC), yes)
  INC_FLAGS := $(INC_FLAGS) -I $(APPS_LIBS_INSTALL)/include.old
else
  INC_FLAGS := $(INC_FLAGS) -I $(APPS_LIBS_INSTALL)/include
endif
INC_FLAGS := $(INC_FLAGS) -I$(TG_INSTALL)/include

CFLAGS    := -fno-builtin -g
CFLAGS    += -DENABLE_EXTENSION_AFFINITY
CFLAGS    += -DENABLE_EXTENSION_PARAMS_EVT
CFLAGS    += -DENABLE_EXTENSION_COUNTED_EVT
CFLAGS    += -DENABLE_EXTENSION_CHANNEL_EVT

DEFINES   := -DOCR_TYPE_H=$(ARCH).h -DENABLE_EXTENSION_LABELING
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
