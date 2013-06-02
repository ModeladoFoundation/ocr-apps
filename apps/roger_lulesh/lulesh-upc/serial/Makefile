ifndef OPT
	OPT=-O
endif

UPC=upcc -Wc,"-std=c99"
UPCFLAGS= -DupcConst=  $(OPT)
UPCLDFLAGS= -lm


all: lulesh.o lulesh

lulesh.o:  lulesh.upc domain.h Makefile
	$(UPC) $(UPCFLAGS) -c lulesh.upc

lulesh:	lulesh.o
	$(UPC) $(UPCFLAGS) -o lulesh lulesh.o $(UPCLDFLAGS)

run:	lulesh
	upcrun -n 1 ./lulesh

check:	lulesh ../../utils/gold.out
	rm -f nohup.out
	date
	time nohup upcrun -n 1 ./lulesh
	date
	diff nohup.out ../../utils/gold.out
	wc nohup.out ../../utils/gold.out

clean:
	rm -f lulesh.o lulesh nohup.out
