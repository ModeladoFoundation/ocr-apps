CC = gcc
MCC = mcc
CFLAGS=-O3 -std=c11
MCCFLAGS=--ompss --no-copy-deps $(CFLAGS)
LDFLAGS=-L$(OCR_INSTALL)/lib -lnanos6 -locr_x86

.PHONY: all clean

all: nqueens

nqueens: nqueens.c atomic.o
	$(MCC) $(MCCFLAGS) $(LDFLAGS) $^ -o $@

atomic.o: atomic.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f nqueens atomic.o
