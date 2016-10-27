
export OCR_INSTALL=$(HOME)/src/ocr/ocr/install
export OCR_TYPE=x86

CC=mcc
CPPFLAGS=-I$(OCR_INSTALL)/include -include ocr.h -I/home/jbellon/src/apps/hll/ompss/ompss/src -include common.h -DOCR_TYPE_H=$(OCR_TYPE).h
CFLAGS=--ompss --no-copy-deps -O3
LDFLAGS=-lnanos6 -locr_x86 -L$(OCR_INSTALL)/lib

all: printf

printf: printf.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -f core.* printf printf.o mcc_printf.c core.*

run: printf
	OCR_CONFIG=$(OCR_INSTALL)/share/ocr/config/x86/default.cfg ./printf

