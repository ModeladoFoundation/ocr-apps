# CnC-OCR Makefile for {{g.name}}
TARGET := {{g.name}}

#################################################
# DEBUG / OPTIMIZATION OPTIONS

# Optimization flags
CFLAGS += -g -O2

# Enable CnC assertions and runtime sanity checks
CFLAGS += -DCNC_DEBUG

# Enable debug logging for x86 (also serializes step execution)
#CFLAGS += -DCNC_DEBUG_LOG=\"./cnc_events.log\"


#################################################
# ENVIRONMENT CHECK

OCR_TYPE ?= x86-pthread-x86

# Make sure we can find OCR and CnC-OCR
ifndef OCR_INSTALL_ROOT
ifndef XSTACK_ROOT
$(error Please define XSTACK_ROOT or OCR_INSTALL_ROOT)
endif #XSTACK_ROOT
OCR_INSTALL_ROOT=$(XSTACK_ROOT)/ocr/install/$(OCR_TYPE)
CNCOCR_ROOT ?= $(XSTACK_ROOT)/hll/cnc
else
ifndef CNCOCR_ROOT
$(error Please define XSTACK_ROOT or CNCOCR_ROOT)
endif #CNCOCR_ROOT
endif #CNC_INSTALL_ROOT

OCR_HOME=$(shell dirname `dirname $(OCR_INSTALL_ROOT)`)


#################################################
# INCLUDES

# Auto-parse arguments for "make run" (WORKLOAD_ARGS)
WORKLOAD_INPUT_FILE_IDX := 0
-include $(OCR_HOME)/ocr-apps/makefiles/make-pre.inc

include cncocr_support/{{g.name}}_defs.mk


#################################################
# OCR VARIANTS

# Distributed (MPI)
ifeq "$(OCR_TYPE)" "x86-pthread-mpi"
CC := mpicc
ifdef OCR_NODEFILE
MPI_FLAGS := --machinefile $(OCR_NODEFILE)
else ifdef OCR_NUM_NODES
MPI_FLAGS := -np $(OCR_NUM_NODES)
else
$(error Must specify either OCR_NODEFILE or OCR_NUM_NODES)
endif #OCR_NODEFILE
CFG_NAME ?= mach-hc-dist-mpi-clone-2w-lockable.cfg
RUN := mpirun $(MPI_FLAGS) ./$(TARGET)

# Emulated TG
else ifeq "$(OCR_TYPE)" "x86-pthread-mpi"
CFG_NAME ?= jenkins-1block-lockableDB.cfg

# Default (x86 shared memory)
else
OCR_CONFIG := $(CNCOCR_ROOT)/resources/cncocr/config/m8w.cfg
endif #x86-pthread-mpi

CC ?= gcc
RUN ?= ./$(TARGET)
OCR_CONFIG ?= $(OCR_INSTALL_ROOT)/config/$(CFG_NAME)
OCR_AR := $(OCR_INSTALL_ROOT)/lib/libocr.a


#################################################
# COMPILER FLAGS SETUP

CC_OPTS := -pthread
LDFLAGS := -lm $(CC_OPTS)
IFLAGS := -I. -Icncocr_support -I$(OCR_INSTALL_ROOT)/include
CFLAGS += $(IFLAGS) $(OPT_FLAGS) -Wall $(CC_OPTS)

CNC_RUNTIME_SRCS := cncocr_support/cncocr.c
CNC_OP_SRCS := $(patsubst %,cncocr_support/{{g.name}}_%_ops.c,step item graph)
HEADERS := {{g.name}}_defs.h $(wildcard cncocr_support/*.h)
SRCS := Main.c {{g.name}}.c $(CNC_OP_SRCS) $(CNC_STEP_SRCS) $(CNC_RUNTIME_SRCS)
OBJS := $(patsubst %.c,%.o,$(SRCS))


#################################################
# MAKE TARGETS

compile: $(TARGET)

# building source files
%.o: %.c $(HEADERS) | $(OCR_AR)
	$(CC) $(CFLAGS) -c $< -o $@

# build the ocr runtime if needed
$(OCR_AR):
	@echo OCR_HOME=$(OCR_HOME)
	@[ -f "$(OCR_HOME)/ocr.dox" ] \
		|| (echo "ERROR: Can't build OCR runtime (can't find OCR build directory)" && exit 1)
	OCR_TYPE=$(OCR_TYPE) make -C $(OCR_HOME) install

# linking - creating the executable
$(TARGET): $(OCR_AR) $(OBJS)
	$(CC) -o $@ $(OBJS) $(OCR_AR) $(LDFLAGS)

# delete binaries
clean:
	rm -f $(OBJS) $(TARGET)

run: compile
	OCR_CONFIG=$(OCR_CONFIG) $(RUN) $(WORKLOAD_ARGS)

gdb: compile
	OCR_CONFIG=$(OCR_CONFIG) gdb -ex r --args ./$(TARGET) $(WORKLOAD_ARGS)

.PHONY: compile clean run gdb

