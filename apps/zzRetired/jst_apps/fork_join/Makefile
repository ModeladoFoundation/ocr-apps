HCC=hcc
CC=gcc

CFLAGS=-O3 -I$(HC_HOME)/inc
LDFLAGS=-L$(HC_HOME)/lib -lpthread -lz -lhc -lrt -lxml2

TARGET=fork_join
OBJS=fork_join.o

default: all

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.hc
	$(HCC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	-rm -f rose_*.c $(OBJS) $(TARGET)
