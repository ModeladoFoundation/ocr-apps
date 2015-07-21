# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

TARGET = stencil
SRC = stencil.c
ROSE_SRC = rose_stencil.c
TOOL = ../../../bin/globals2db
ROSE_TARGET = rose_$(TARGET)

include $(CURDIR)/../../../../Makefile.def

all: run


$(ROSE_TARGET): $(SRC) $(TOOL)
	$(TOOL) $(CFLAGS) $(CC_INC) $(SRC) $(MPI_FLAGS) -o $(TARGET) > build.log

run: $(ROSE_TARGET)
#	$(CC) -g -O2  -I. $(ROSE_SRC) $(MPI_FLAGS) -o $(ROSE_TARGET)
#	./$(ROSE_TARGET) 8 > $(ROSE_TARGET).log
	@if test `grep "MPI_Init" build.log | wc -l` -eq 1; \
		then echo "$(TOOL) PASSED!" $(OUTPUT) ; \
		else echo "$(TOOL) FAILED." $(OUTPUT) ; \
	fi

dot:
	dotGenerator $(TARGET).c
	$(ZGRVIEW) -Pdot $(TARGET).c.dot &

clean:
	rm -f *~ ffwd.h $(TARGET) $(TARGET).log $(ROSE_TARGET) $(ROSE_SRC) *.o $(TARGET).c.dot build.log $(ROSE_TARGET).log



