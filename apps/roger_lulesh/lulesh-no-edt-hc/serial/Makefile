ifndef OPT
	OPT=-O3
endif

GCC=gcc -Dinline= -DhcConst= -std=c99
CFLAGS= -fpic -g $(OPT)
LDFLAGS = -lm

HCC=hcc -Dinline= -DhcConst=  -rose:C99
HCFLAGS= -I$(HC_HOME)/include/hc -DHAB_C
HLDFLAGS = -lpthread -L$(HC_HOME)/lib -lxml2 -lhc

all: lulesh.o lulesh

lulesh.o:  lulesh.hc domain.h Makefile
	$(HCC) $(HCFLAGS) $(CFLAGS) -c lulesh.hc

rose_lulesh.o: rose_lulesh.c 
	$(GCC) $(HCFLAGS) $(CFLAGS) -c rose_lulesh.c

lulesh:	lulesh.o
	$(GCC) $(CFLAGS) -o lulesh lulesh.o $(HLDFLAGS) $(LDFLAGS)

run:	lulesh
	./lulesh -nproc 1

clean:
	rm -f rose_lulesh.o rose_lulesh.c lulesh.o lulesh
