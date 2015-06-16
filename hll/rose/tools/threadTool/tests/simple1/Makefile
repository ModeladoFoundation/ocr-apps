TARGET = main
SRC = main.c
ROSE_SRC = rose_main.c
TOOL = ../../bin/threadTool
ROSE_TARGET = rose_$(TARGET)

include $(CURDIR)/../../../Makefile.def

all: run


$(ROSE_TARGET): $(SRC) $(TOOL)
	$(TOOL) $(CFLAGS) $(CC_INC) $(SRC) -o $(TARGET) > $(TOOL).log


run: $(ROSE_TARGET)
	$(CC) -g -O2  -I. $(ROSE_SRC) -o $(ROSE_TARGET)
	./$(ROSE_TARGET) 8 > $(ROSE_TARGET).log
	@if test `grep "48" $(ROSE_TARGET).log | wc -l` -eq 1; \
		then echo "$(TOOL) PASSED!" $(OUTPUT) ; \
		else echo "$(TOOL) FAILED." $(OUTPUT) ; \
	fi

dot:
	dotGenerator $(TARGET).c
	$(ZGRVIEW) -Pdot $(TARGET).c.dot &

clean:
	rm -f *~ ffwd.h $(TARGET) $(TARGET).log $(ROSE_TARGET) $(ROSE_SRC) *.o $(TARGET).c.dot  $(ROSE_TARGET).log



