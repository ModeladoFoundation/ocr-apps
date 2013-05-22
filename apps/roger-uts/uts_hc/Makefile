CC=gcc -Dinline=
CCFLAGS= -fpic -O0 -g  -DBRG_C99_TYPES -std=c99 -DBRG_RNG -DTRACE=0 -DDEBUG=0
##-DRAG_FIN_LAST

LDFLAGS= -lm

HCC=hcc -Dinline=
HCCFLAGS= -rose:C99 -I$(HC_HOME)/include/hc
HLDFLAGS= -lpthread -L$(HC_HOME)/lib -lxml2 -lhc

.SUFFIXES:
.SUFFIXES: .o .h .c .hc .s

all:	uts-dfs-serial uts-dfs-parallel


## SERIAL

uts-dfs-serial:  brg_endian.h brg_types.h brg_sha1.c uts-dfs.c uts.c  Makefile
	$(CC) $(CCFLAGS) -DRAG_SERIAL -o uts-dfs-serial uts-dfs.c brg_sha1.c uts.c $(LDFLAGS)

## PARALLEL

brg_sha1.o: brg_sha1.c brg_sha1.h brg_endian.h brg_types.h Makefile
	$(CC) $(CCFLAGS) -c brg_sha1.c

uts.o: uts.c uts.h rng.h Makefile
	$(CC) $(CCFLAGS) -c uts.c 

uts-dfs.o: uts-dfs.hc uts.h rng.h Makefile
	$(HCC) $(CCFLAGS) $(HCCFLAGS) -DRAG_HC -c uts-dfs.hc 

uts-dfs-parallel:  uts-dfs.o brg_sha1.o uts.o Makefile
	$(CC) $(CCFLAGS) -o uts-dfs-parallel uts-dfs.o brg_sha1.o uts.o $(HLDFLAGS) $(LDFLAGS)

run:	uts-dfs-serial uts-dfs-parallel
	time ./uts-dfs-parallel
	time ./uts-dfs-serial

run-big: uts-dfs-serial uts-dfs-parallel
	time ./uts-dfs-parallel   -t 1 -a 3 -d 10 -b 4 -r 19
	time ./uts-dfs-serial     -t 1 -a 3 -d 10 -b 4 -r 19

run-bigger: uts-dfs uts-dfs-hc ## uts-dfs-afl ## uts-dfs-cilk
	time ./uts-dfs-parallel  -t 1 -a 3 -d 15 -b 4 -r 29
	time ./uts-dfs-serial    -t 1 -a 3 -d 15 -b 4 -r 29

clean:
	rm -f	uts-dfs-serial uts-dfs-parallel \
		brg_sha1.o uts.o uts-dfs.o rose_uts-dfs.c
