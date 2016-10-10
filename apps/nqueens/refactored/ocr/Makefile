CC = gcc
CFLAGS=-O3 -std=c11
CPPFLAGS=-I$(OCR_INSTALL)/include
LDFLAGS=-L$(OCR_INSTALL)/lib -locr_$(OCR_TYPE)

.PHONY: all clean

all: nqueens

nqueens: nqueens.c atomic.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -f nqueens
