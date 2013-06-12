ifndef OPT
	OPT=-O
endif

GCC=gcc -std=c99
CFLAGS= -fpic -g $(OPT)
LDFLAGS = -lm

HCC=hcc -rose:C99 -DHAB_C
HCFLAGS= -I$(HC_HOME)/include/hc
HLDFLAGS = -lpthread -L$(HC_HOME)/lib -lxml2 -lhc

all: RAG_edt.o AMO.o lulesh.o lulesh

RAG_edt.o:  RAG_edt.c RAG_edt.h kernels.h RAG.h Makefile
	$(GCC) $(CFLAGS) -c RAG_edt.c

AMO.o:  AMO.c AMO.h RAG.h Makefile
	$(GCC) $(CFLAGS) -c AMO.c

lulesh.o:  lulesh.hc RAG.h AMO.h MEM.h FP.h RAG_edt.h kernels.h initialize.h Makefile
	$(HCC) $(HCFLAGS) $(CFLAGS) -c lulesh.hc

rose_lulesh.o: rose_lulesh.c AMO.h MEM.h FP.h RAG_edt.h kernels.h initialize.h Makefile
	$(GCC) $(HCFLAGS) $(CFLAGS) -c rose_lulesh.c

lulesh:	lulesh.o AMO.o RAG_edt.o Makefile
	$(GCC) $(CFLAGS) -o lulesh lulesh.o AMO.o RAG_edt.o $(HLDFLAGS) $(LDFLAGS)

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
	rm -f RAG_edt.o AMO.o rose_lulesh.o rose_lulesh.c lulesh.o lulesh
