# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.

MAKE := make

#ifndef OCR_TYPE
#  $(error Need to specify OCR_TYPE!)
#endif
OCR_TYPE=x86

TARGET = main
SRC = main.c
ROSE_SRC = rose_main.c
ROSE_TOOL = ../../../bin/globals2struct
ROSE_TARGET = rose_$(TARGET)
ROSE_OPT = -rose:binary

include $(CURDIR)/../../../../Makefile.def

all: run


$(ROSE_TARGET): $(SRC) $(ROSE_TOOL)

	$(ROSE_TOOL) $(CFLAGS) $(CC_INC) $(SRC) -o rose_main > build.log


run: $(ROSE_TARGET)
	./$(ROSE_TARGET)

dot:
#  	dotGenerator $(TARGET).c -D__leaf__=__externally_visible__
#	$(ZGRVIEW) -Pdot $(TARGET).c.dot &
	dotGeneratorWholeASTGraph $(TARGET).c -D__leaf__=__externally_visible__
	$(ZGRVIEW) -Pdot $(TARGET).c_WholeAST.dot &

clean:
	rm -f *~ ffwd.h $(TARGET) $(ROSE_TARGET) $(ROSE_SRC) *.o $(TARGET).c.dot build.log
	rm -rf install build





