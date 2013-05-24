ifndef OPT
	OPT=-O
endif

GCC=gcc -Dinline= -DhcConst= -std=c99
CFLAGS= -fpic -g $(OPT)
LDFLAGS = -lm

HCC=hcc -Dinline= -DhcConst=  -rose:C99 -DSHOW_BUG_OFF
HCFLAGS= -I$(HC_HOME)/include/hc -DHAB_C
HLDFLAGS = -lpthread -L$(HC_HOME)/lib -lxml2 -lhc

all: AMO.o lulesh.o lulesh

AMO.o:  AMO.c AMO.h Makefile
	$(GCC) $(CFLAGS) -c AMO.c

lulesh.o:  lulesh.hc AMO.h Makefile
	$(HCC) $(HCFLAGS) $(CFLAGS) -c lulesh.hc

rose_lulesh.o: rose_lulesh.c AML.h 
	$(GCC) $(HCFLAGS) $(CFLAGS) -c rose_lulesh.c

lulesh:	lulesh.o AMO.o
	$(GCC) $(CFLAGS) -o lulesh lulesh.o AMO.o $(HLDFLAGS) $(LDFLAGS)

run:	lulesh
	./lulesh -nproc 8
#	./lulesh -nproc 1

check:	lulesh gold.out
	rm -f nohup.out
	date
	time nohup ./lulesh -nproc 8
	date
	diff nohup.out gold.out
	wc nohup.out gold.out

clean:
	rm -f rose_lulesh.o rose_lulesh.c lulesh.o lulesh
