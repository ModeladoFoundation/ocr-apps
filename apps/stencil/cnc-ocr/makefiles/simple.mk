# Make sure we can find OCR
ifndef OCR_INSTALL
ifndef XSTACK_SRC
$(error Please define XSTACK_SRC or OCR_INSTALL)
endif
OCR_TYPE ?= x86-pthread-x86
OCR_INSTALL=$(XSTACK_SRC)/ocr/install/$(OCR_TYPE)
endif

# Try to be more compatible with the full makefiles
ifndef ARGS
ARGS:=$(WORKLOAD_ARGS)
endif

TARGET := Stencil1D
CC := gcc
CC_OPTS := -pthread
CFLAGS := -DOCR_ASSERT -DOCR_ENABLE_VISUALIZER -DOCR_ENABLE_EDT_NAMING -DOCR_DEBUG_LVL=DEBUG_LVL_INFO -g -O2 -I. -Icncocr_support -I$(OCR_INSTALL)/include -Wall $(CC_OPTS)

include cncocr_support/Stencil1D_defs.mk
CNC_RUNTIME_SRCS := cncocr_support/cncocr.c
CNC_OP_SRCS := $(patsubst %,cncocr_support/Stencil1D_%_ops.c,step item graph)
SRCS := Main.c Stencil1D.c $(CNC_OP_SRCS) $(CNC_STEP_SRCS) $(CNC_RUNTIME_SRCS)
OBJS := $(patsubst %.c,%.o,$(SRCS))

compile: $(TARGET)

# building source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# linking - creating the executable
$(TARGET): $(OBJS)
	$(CC) -o $@ $(CC_OPTS) $(OBJS) $(OCR_INSTALL)/lib/libocr.a

# delete binaries
clean:
	rm -f $(OBJS) $(TARGET)

run: compile
	OCR_CONFIG=$(OCR_INSTALL)/config/default.cfg ./$(TARGET) $(ARGS)

gdb: compile
	OCR_CONFIG=$(OCR_INSTALL)/config/default.cfg gdb ./$(TARGET) -ex "r $(ARGS)"
