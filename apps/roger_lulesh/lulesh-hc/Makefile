ifndef OPT
	OPT=-O
endif

GCC=gcc -std=c99
CFLAGS= -fpic -g $(OPT)
LDFLAGS = -lm

HCC=hcc -rose:C99 -DHAB_C
HCFLAGS= -I$(HC_HOME)/include/hc
HLDFLAGS = -lpthread -L$(HC_HOME)/lib -lxml2 -lhc

all: AMO.o lulesh.o lulesh

AMO.o:  AMO.c AMO.h RAG.h Makefile
	$(GCC) $(CFLAGS) -c AMO.c

lulesh.o:  lulesh.hc RAG.h AMO.h MEM.h FP.h Makefile
	$(HCC) $(HCFLAGS) $(CFLAGS) -c lulesh.hc

rose_lulesh.o: rose_lulesh.c AMO.h MEM.h FP.h Makefile
	$(GCC) $(HCFLAGS) $(CFLAGS) -c rose_lulesh.c

lulesh:	lulesh.o AMO.o Makefile
	$(GCC) $(CFLAGS) -o lulesh lulesh.o AMO.o $(HLDFLAGS) $(LDFLAGS)

run:	lulesh
	./lulesh -nproc 8
#	./lulesh -nproc 1

check:	lulesh ../utils/gold.out
	rm -f nohup.out
	date
	time nohup ./lulesh -nproc 8
	date
	diff nohup.out ../utils/gold.out
	wc nohup.out ../utils/gold.out

clean:
	rm -f AMO.o rose_lulesh.o rose_lulesh.c lulesh.o lulesh nohup.out
