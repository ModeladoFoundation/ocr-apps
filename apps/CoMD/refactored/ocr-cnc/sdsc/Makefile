# Make sure we can find OCR
XSTACK_SRC ?= ../../../../../
ifndef OCR_INSTALL
ifndef XSTACK_SRC
$(error Please define XSTACK_SRC or OCR_INSTALL)
endif
OCR_TYPE ?= x86-pthread-x86
OCR_INSTALL=$(XSTACK_SRC)/ocr/install/$(OCR_TYPE)
endif
OCR_HOME=$(shell dirname `dirname $(OCR_INSTALL)`)

# Auto-parse arguments for "make run"
WORKLOAD_INPUT_FILE_IDX := 0
-include $(XSTACK_SRC)/apps/makefiles/make-pre.inc
ifndef WORKLOAD_ARGS
WORKLOAD_ARGS := $(ARGS)
endif


TARGET := comd

# Distributed (MPI)
ifeq "$(OCR_TYPE)" "x86-pthread-mpi"
CC := mpicc
ifdef OCR_NODEFILE
MPI_FLAGS := --machinefile $(OCR_NODEFILE)
else ifdef OCR_NUM_NODES
MPI_FLAGS := -np $(OCR_NUM_NODES)
else
$(error Must specify either OCR_NODEFILE or OCR_NUM_NODES)
endif
CFG_NAME ?= mach-hc-dist-mpi-clone-2w-lockable.cfg
RUN := mpirun $(MPI_FLAGS) ./$(TARGET)
# Shared memory
else
CFG_NAME ?= mach-hc-8w-lockableDB.cfg
CC := gcc
RUN := ./$(TARGET)
endif

CC_OPTS := -pthread -std=c99 -Wno-unused-function
LDFLAGS := -lm $(CC_OPTS)
CFLAGS := -DCNC_DEBUG -g -O2 -I. -Icncocr_support -I$(OCR_INSTALL)/include -Wall $(CC_OPTS)

include cncocr_support/comd_defs.mk
CNC_RUNTIME_SRCS := cncocr_support/cncocr.c
#CNC_OP_SRCS := $(patsubst %,cncocr_support/comd_%_ops.c,step item graph)
CNC_OP_SRCS := $(patsubst %,cncocr_support/comd_%_ops.c,step item graph) cmdLineParser.c mycommand.c random.c parallel.c initAtoms.c linkCells.c ljForce.c decomposition.c eam.c
SRCS := Main.c comd.c $(CNC_OP_SRCS) $(CNC_STEP_SRCS) $(CNC_RUNTIME_SRCS)
OBJS := $(patsubst %.c,%.o,$(SRCS))

OCR_CONFIG ?= $(OCR_INSTALL)/config/$(CFG_NAME)
OCR_AR=$(OCR_INSTALL)/lib/libocr.a

compile: $(TARGET)

# building source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# build the ocr runtime if needed
$(OCR_AR):
	echo $(OCR_HOME)
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
	OCR_CONFIG=$(OCR_CONFIG) $(RUN) $(ARGS)

gdb: compile
	OCR_CONFIG=$(OCR_CONFIG) gdb ./$(TARGET) -ex "r $(ARGS)"
