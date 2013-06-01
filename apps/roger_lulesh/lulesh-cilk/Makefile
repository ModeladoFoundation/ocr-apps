ifndef OPT
	OPT=-O
endif

GCC=gcc -DhcConst=const -std=c99
ICC=icc -DhcConst=const -std=c99 -DCILK
CFLAGS= -fpic -g $(OPT)
LDFLAGS = -lm

all: AMO.o lulesh.o lulesh

AMO.o:  AMO.c AMO.h Makefile
	$(GCC) $(CFLAGS) -c AMO.c

lulesh.o:  lulesh.cpp RAG.h AMO.h Makefile
	$(ICC) $(CFLAGS) $(CFLAGS) -c lulesh.cpp

lulesh:	lulesh.o AMO.o
	$(ICC) $(CFLAGS) -o lulesh lulesh.o AMO.o $(LDFLAGS)

run:	lulesh
	./lulesh

check:	lulesh ../utils/gold.out
	rm -f nohup.out
	date
	CILK_NWORKERS=8 time nohup ./lulesh
	date
	diff nohup.out ../utils/gold.out
	wc nohup.out ../utils/gold.out

clean:
	rm -f AMO.o lulesh.o lulesh nohup.out
