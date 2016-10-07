CC = gcc
CFLAGS=-O3 -fopenmp

.PHONY: all clean

all: nqueens

nqueens: nqueens.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f nqueens
