ifndef OPT
	OPT=-O
endif

GCC=gcc -std=c99
CFLAGS= -fpic -g $(OPT)
LDFLAGS = -lm

all: RAG_edt.o AMO.o lulesh.o lulesh

RAG_edt.o:  RAG_edt.c RAG_edt.h AMO.h MEM.h FP.h RAG.h Makefile
	$(GCC) $(CFLAGS) -c RAG_edt.c

AMO.o:  AMO.c AMO.h RAG.h Makefile
	$(GCC) $(CFLAGS) -c AMO.c

lulesh.o:  lulesh.c RAG_edt.h RAG.h AMO.h MEM.h FP.h Makefile
	$(GCC) $(CFLAGS) -c lulesh.c

lulesh:	lulesh.o AMO.o RAG_edt.o Makefile
	$(GCC) $(CFLAGS) -o lulesh lulesh.o AMO.o RAG_edt.o $(LDFLAGS)

run:	lulesh
	./lulesh

check:	lulesh ../utils/gold.out
	rm -f nohup.out
	date
	time nohup ./lulesh
	date
	diff nohup.out ../utils/gold.out
	wc nohup.out ../utils/gold.out

clean:
	rm -f RAG_edt.o AMO.o lulesh.o lulesh
