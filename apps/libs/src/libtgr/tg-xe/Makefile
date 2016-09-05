TG_ROOT ?= $(realpath $(CURDIR)/../../../../../../tg/tg)
TG_INSTALL ?= $(TG_ROOT)/install
APPS_ROOT ?= $(realpath $(CURDIR)/../../../..)
APPS_LIBS_INSTALL_ROOT ?= $(APPS_ROOT)/libs/install
APPS_LIBS_INSTALL ?= $(APPS_LIBS_INSTALL_ROOT)/tg-xe

BUILD_DIR = ../build-$(TGR_TYPE)

TOOLBIN = $(TG_INSTALL)/bin
TOOL_PREFIX = $(TOOLBIN)/xstg-linux-elf-

CC = $(TOOL_PREFIX)clang
AS = $(TOOL_PREFIX)as
AR = $(TOOL_PREFIX)ar
LD = $(TOOL_PREFIX)ld
RANLIB = $(TOOL_PREFIX)ranlib

NEWLIB_INC = $(APPS_LIBS_INSTALL)/include
#CFLAGS = -isystem $(NEWLIB_INC) -std=c99 -Os

#TODO debug only
CFLAGS = -isystem $(NEWLIB_INC) -std=c99 -O0 -ggdb3

SRCS = tgr-xe.c \
       memory.c \
       io.c \
       process.c \
       time.c

OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(patsubst %.S, $(BUILD_DIR)/%.o, $(SRCS)))

TARGET_LIB = libtgr-xe.a
TARGET = $(BUILD_DIR)/$(TARGET_LIB)

all: $(TARGET)

$(TARGET): $(BUILD_DIR) $(OBJS)
	@rm -rf $@
	@$(AR) cru $@ $(OBJS)
	@$(RANLIB) $@
	@echo Successfully built $@

$(BUILD_DIR):
	@[ -d $(BUILD_DIR) ] || mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o : %.S
	$(AR) -c $(CFLAGS) $< -o $@

install: $(TARGET)
	@cp $(TARGET) $(APPS_LIBS_INSTALL)/lib/$(TARGET_LIB)
	@echo Successfully installed $(APPS_LIBS_INSTALL)/lib/$(TARGET_LIB)

clean:
	@rm -rf $(BUILD_DIR)

.PHONY: all install clean
