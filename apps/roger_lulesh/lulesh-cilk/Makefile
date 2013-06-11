ifndef OPT
	OPT=-O
endif

GCC=gcc -std=c99
ICC=icc -std=c99 -DCILK
CFLAGS= -fpic -g $(OPT)
LDFLAGS = -lm

all: RAG_edt.o AMO.o lulesh.o lulesh

RAG_edt.o:  RAG_edt.c kernels.h RAG_edt.h RAG.h AMO.h FP.h MEM.h Makefile
	$(GCC) $(CFLAGS) -c RAG_edt.c

AMO.o:  AMO.c AMO.h RAG.h Makefile
	$(GCC) $(CFLAGS) -c AMO.c

lulesh.o:  lulesh.cpp RAG.h AMO.h MEM.h FP.h RAG_edt.h kernels.h Makefile
	$(ICC) $(CFLAGS) -c lulesh.cpp

lulesh:	lulesh.o AMO.o RAG_edt.o Makefile
	$(ICC) $(CFLAGS) -o lulesh lulesh.o AMO.o RAG_edt.o $(LDFLAGS)

run:	lulesh
	CILK_NWORKERS=8 ./lulesh

check:	lulesh ../utils/gold.out
	rm -f nohup.out
	date
	CILK_NWORKERS=8 time nohup ./lulesh
	date
	diff nohup.out ../utils/gold.out
	wc nohup.out ../utils/gold.out

clean:
	rm -f RAG_edt.o AMO.o lulesh.o lulesh
