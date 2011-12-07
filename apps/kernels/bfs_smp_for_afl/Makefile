CFLAGS=#-O3 -g -I../../ -I../../common
LDFLAGS=-lpthread #-lc -lrt

CC=g++

OBJS=bfs_smp_step_by_step.o int_vec.o
LIBS=#../../rmd_afl.a

TARGET=bfs_smp
all: $(TARGET)

$(TARGET): $(OBJS) $(LIBS)
	$(CC) $(LDFLAGS) -o $@ $^

.PHONY: clean
clean:
	-rm -f $(TARGET) $(OBJS)
