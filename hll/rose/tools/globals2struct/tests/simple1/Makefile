# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

TARGET = main
SRC = main.c foo.c
ROSE_SRC = rose_main.c rose_foo.c
TOOL = ../../bin/globals2struct
ROSE_TARGET = rose_$(TARGET)

include $(CURDIR)/../../../Makefile.def

all: run


$(ROSE_TARGET): $(SRC) $(TOOL)
	$(TOOL) $(CFLAGS) $(CC_INC) $(SRC) -o $(TARGET) > build.log


run: $(ROSE_TARGET)
	$(CC) -g -O2  -I. $(ROSE_SRC) -o $(ROSE_TARGET)
	./$(ROSE_TARGET) 8 > $(ROSE_TARGET).log
	$(CC) -O2 $(SRC) -o $(TARGET)
	./$(TARGET) 8 > $(TARGET).log
	@if test `diff $(TARGET).log $(ROSE_TARGET).log  | wc -l` -eq 0; \
		then echo "$(TOOL) PASSED!" $(OUTPUT) ; \
		else echo "$(TOOL) FAILED." $(OUTPUT) ; \
	fi


dot:
	dotGenerator $(TARGET).c
	$(ZGRVIEW) -Pdot $(TARGET).c.dot &

clean:
	rm -f *~ ffwd.h $(TARGET) $(TARGET).log $(ROSE_TARGET) $(ROSE_SRC) *.o $(TARGET).c.dot build.log $(ROSE_TARGET).log



