ifndef OPT
	OPT=-O
endif

GCC=gcc -std=c99
CFLAGS= -fpic -g $(OPT)
LDFLAGS = -lm

UPC=upcc -Wc,"-std=c99"
UPCFLAGS= -DUPC $(OPT)
UPCLDFLAGS= -lm


all: AMO.o lulesh.o lulesh

AMO.o:  AMO.upc RAG.h AMO.h Makefile
	$(UPC) $(UPCFLAGS) -c AMO.upc

lulesh.o:  lulesh.upc RAG.h AMO.h Makefile
	$(UPC) $(UPCFLAGS) -c lulesh.upc

lulesh:	lulesh.o AMO.o
	$(UPC) $(UPCFLAGS) -o lulesh lulesh.o AMO.o $(UPCLDFLAGS)

run:	lulesh
	upcrun -n 1 ./lulesh

check:	lulesh ../utils/gold.out
	rm -f nohup.out
	date
	time nohup upcrun -n 1 ./lulesh
	date
	diff nohup.out ../utils/gold.out
	wc nohup.out ../utils/gold.out

clean:
	rm -f AMO.o lulesh.o lulesh nohup.out
