# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

TARGET = main
SRC = main.c
ROSE_SRC = rose_main.c
TOOL = ../../../bin/globals2struct
ROSE_TARGET = rose_$(TARGET)

include $(CURDIR)/../../../../Makefile.def

all: run


$(ROSE_TARGET): $(SRC) $(TOOL)
	$(TOOL) $(CFLAGS) $(CC_INC) $(SRC) -o $(TARGET) > build.log


run: $(ROSE_TARGET)
	$(CC) -g -O2  -I. $(ROSE_SRC) -o $(ROSE_TARGET)
	./$(ROSE_TARGET) > $(ROSE_TARGET).log
	@if test `grep Warning build.log  | wc -l` -eq 0; \
		then echo "$(TOOL) PASSED!" $(OUTPUT) ; \
		else echo "$(TOOL) FAILED." $(OUTPUT) ; \
	fi


dot:
	dotGenerator $(TARGET).c
	$(ZGRVIEW) -Pdot $(TARGET).c.dot &

clean:
	rm -f *~ rose_ffwd.h $(ROSE_TARGET) $(ROSE_SRC) *.o $(TARGET) $(TARGET).c.dot build.log $(ROSE_TARGET).log



