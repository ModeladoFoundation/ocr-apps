ifndef OPT
	OPT=-O
endif

GCC=gcc -std=c99
CFLAGS= -fpic -g $(OPT)
LDFLAGS = -lm

all: AMO.o lulesh.o lulesh

AMO.o:  AMO.c AMO.h RAG.h Makefile
	$(GCC) $(CFLAGS) -c AMO.c

lulesh.o:  lulesh.c RAG.h AMO.h MEM.h FP.h Makefile
	$(GCC) $(CFLAGS) $(CFLAGS) -c lulesh.c

lulesh:	lulesh.o AMO.o Makefile
	$(GCC) $(CFLAGS) -o lulesh lulesh.o AMO.o $(LDFLAGS)

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
	rm -f AMO.o lulesh.o lulesh
