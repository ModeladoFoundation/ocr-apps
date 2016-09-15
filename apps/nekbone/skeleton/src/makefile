#CC:=mpiCC
CC:=mpiicc

cFLAG  = -g -O0 -I.
lFLAG  = -g -O0 -I.

DOTO = nekcgsim.o amain.o

default:	nekcg
all:	nekcg

nekcgsim.o:	./nekcgsim.cpp
		$(CC) -c $(cFLAG) $< -o $@

amain.o	:	./amain.cpp
		$(CC) -c $(cFLAG) $< -o $@

nekcg: 	clean $(DOTO)
	$(CC) -o nekskeleton.x $(lFLAG)  $(DOTO)

clean:
	rm -f *.o nekskeleton.x*



