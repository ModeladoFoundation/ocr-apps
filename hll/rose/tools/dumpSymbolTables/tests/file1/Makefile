# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

TARGET = main
SRC = main.c
ROSE_SRC = rose_main.c
TOOL = ../../bin/dumpSymbolTables
ROSE_TARGET = rose_$(TARGET)

include $(CURDIR)/../../../Makefile.def

all: run


$(ROSE_TARGET): $(SRC) $(TOOL)
	$(TOOL) $(CFLAGS) $(CC_INC) $(SRC) -o $(TARGET) > build.log


run: $(ROSE_TARGET)


dot:
	dotGenerator $(TARGET).c
	$(ZGRVIEW) -Pdot $(TARGET).c.dot &

clean:
	rm -f *~ ffwd.h $(TARGET) $(TARGET).log $(ROSE_TARGET) $(ROSE_SRC) *.o $(TARGET).c.dot  $(ROSE_TARGET).log build.log



