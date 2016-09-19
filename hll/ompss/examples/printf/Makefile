
export OCR_INSTALL=$(HOME)/src/ocr/ocr/install
export OCR_TYPE=x86

CC=mcc
CPPFLAGS=-I$(OCR_INSTALL)/include -include ocr.h
CFLAGS=--ompss -K -O0 -ggdb3
LDFLAGS=-lnanos6 -locr_x86 -L$(OCR_INSTALL)/lib

all: printf

printf: printf.o

clean:
	rm -f core.* printf printf.o mcc_printf.c core.*

run: printf
	OCR_CONFIG=$(OCR_INSTALL)/share/ocr/config/x86/default.cfg ./printf

