# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

MAKE := make

ifndef OCR_TYPE
  $(error Need to specify OCR_TYPE!)
endif
OCR_TYPE ?= x86

TARGET = stencil
SRC = stencil.c
ROSE_SRC = rose_stencil.c
ROSE_TOOL = ../../../bin/globals2db
ROSE_TARGET = rose_$(TARGET)
ROSE_OPT = -rose:skipfinalCompileStep


include $(CURDIR)/../../../../Makefile.def

all: run


$(ROSE_TARGET): $(SRC) $(ROSE_TOOL)
	$(ROSE_TOOL) $(ROSE_OPT) $(CFLAGS) $(CC_INC) $(SRC) $(MPI_FLAGS) > build.log


run: $(ROSE_TARGET)
	make -f Makefile.$(OCR_TYPE)
	make -f Makefile.$(OCR_TYPE) TARGET=$(ROSE_TARGET) clean debug install
	make -f Makefile.$(OCR_TYPE) TARGET=$(ROSE_TARGET) WORKLOAD_ARGS="-r 4 -t 0 " run


dot:
	dotGenerator $(TARGET).c
	$(ZGRVIEW) -Pdot $(TARGET).c.dot &

clean:
	rm -f *~ rose_ffwd.h $(TARGET) $(ROSE_TARGET) $(ROSE_SRC) *.o $(TARGET).c.dot build.log
	rm -rf install build





