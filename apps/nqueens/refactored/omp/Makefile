CC = gcc

LIBS_DIR=../../../libs/install/x86

CFLAGS=-O3 -fopenmp
CPPFLAGS=-I$(LIBS_DIR)/include
LDFLAGS=-L$(LIBS_DIR)/lib -ltimer

.PHONY: all clean

all: nqueens

nqueens: nqueens.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -f nqueens
