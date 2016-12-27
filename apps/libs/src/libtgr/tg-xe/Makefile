TG_ROOT ?= $(realpath $(CURDIR)/../../../../../../tg/tg)
TG_INSTALL ?= $(TG_ROOT)/install

APPS_ROOT ?= $(realpath $(CURDIR)/../../../..)
APPS_LIBS_INSTALL_ROOT ?= $(APPS_ROOT)/libs/install
APPS_LIBS_INSTALL ?= $(APPS_LIBS_INSTALL_ROOT)/tg-xe

BUILD_DIR = ../build-$(TGR_TYPE)-static
BUILD_DIR_P = ../build-$(TGR_TYPE)-pie

TOOLBIN = $(TG_INSTALL)/bin
TOOL_PREFIX = $(TOOLBIN)/xstg-linux-elf-

CC = $(TOOL_PREFIX)clang
AS = $(TOOL_PREFIX)as
AR = $(TOOL_PREFIX)ar
LD = $(TOOL_PREFIX)ld
RANLIB = $(TOOL_PREFIX)ranlib

NEWLIB_INC = -isystem $(APPS_LIBS_INSTALL)/include
TG_CE_INC  = -I../tg-ce/include

INCLUDES = $(NEWLIB_INC) $(TG_CE_INC)

CFLAGS = $(INCLUDES) -std=c99

ifdef DEBUG
CFLAGS += -O0 -g
else
CFLAGS += -Os
endif

SRCS = tgr-xe.c \
       memory.c \
       io.c \
       process.c \
       time.c \
       nonewlib.c

OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))
OBJS_P = $(patsubst %.c, $(BUILD_DIR_P)/%.o, $(SRCS))

TARGET_LIB = libtgr-xe.a
TARGET_LIB_P = libtgr-xe.p.a
TARGET = $(BUILD_DIR)/$(TARGET_LIB)
TARGET_P = $(BUILD_DIR_P)/$(TARGET_LIB_P)

all: static pie
static: $(TARGET)
pie: $(TARGET_P)

$(TARGET): $(BUILD_DIR) $(OBJS)
	@rm -rf $@
	@$(AR) cru $@ $(OBJS)
	@$(RANLIB) $@
	@echo " * $(notdir $@)"

$(TARGET_P): $(OBJS_P)
	@rm -rf $@
	@$(AR) cru $@ $(OBJS_P)
	@$(RANLIB) $@
	@echo " * $(notdir $@)"

$(BUILD_DIR) $(BUILD_DIR_P):
	@mkdir -p $@

$(BUILD_DIR)/%.o : %.c | $(BUILD_DIR)
	@echo " - $(notdir $@)"
	@$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR_P)/%.o : %.c | $(BUILD_DIR_P)
	@echo " - $(notdir $@) pie"
	@$(CC) -c $(CFLAGS) -fPIE $< -o $@

install_static: $(TARGET)
	@cp $(TARGET) $(APPS_LIBS_INSTALL)/lib/$(TARGET_LIB)
	@echo " ** $(TARGET_LIB) -> $(APPS_LIBS_INSTALL)/lib"

install_pie: $(TARGET_P)
	@cp $(TARGET_P) $(APPS_LIBS_INSTALL)/lib/$(TARGET_LIB_P)
	@echo " ** $(TARGET_LIB_P) -> $(APPS_LIBS_INSTALL)/lib"

install: install_static install_pie

clean:
	@rm -rf $(BUILD_DIR)
	@rm -rf $(BUILD_DIR_P)

.PHONY: all install install_static install_pie clean
