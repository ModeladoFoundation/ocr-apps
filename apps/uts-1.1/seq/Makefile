###  UTS - Unbalanced Tree Search  ###

SRCS = uts.c
INCS = uts.h

# ------------------------------------- #
# Set Random Number Generator sources:
# ------------------------------------- #

# Set the default RNG
ifndef RNG
RNG=BRG
endif

ifeq ($(RNG), Devine)
RNG_SRCS = rng/devine_sha1.c
RNG_INCS = rng/devine_sha1.h
RNG_DEFS = -DDEVINE_RNG
endif
ifeq ($(RNG), BRG)
RNG_SRCS = rng/brg_sha1.c
RNG_INCS = rng/brg_sha1.h
RNG_DEFS = -DBRG_RNG
endif
ifeq ($(RNG), ALFG)
RNG_SRCS = rng/alfg.c
RNG_INCS = rng/alfg.h
RNG_DEFS = -DUTS_ALFG
endif


CC=gcc
CFLAGS=-O3 -std=c99 -DSTATS -Wall
LDFLAGS=-lm

all:	uts

uts:	$(INCS) $(SRCS) $(RNG_INCS) $(RNG_SRCS)
	$(CC) $(CFLAGS) $(RNG_DEFS) -o $@ $+ $(LDFLAGS)

run:	uts
	echo "(T4) Hybrid ------------------ Tree size = 4132453, tree depth = 134, num leaves = 3108986 (75.23%)"
	time ./uts -g 1 -t 2 -a 0 -d 16 -b 6 -r 1 -q 0.234375 -m 4

clean:
	rm -f *.o uts

run_long: uts
	echo "(T1) Geometric [fixed] ------- Tree size = 4130071, tree depth = 10, num leaves = 3305118 (80.03%)"
	time ./uts -t 1 -a 3 -d 10 -b 4 -r 19
	echo "(T5) Geometric [linear dec.] - Tree size = 4147582, tree depth = 20, num leaves = 2181318 (52.59%)"
	time ./uts -t 1 -a 0 -d 20 -b 4 -r 34
	echo "(T2) Geometric [cyclic] ------ Tree size = 4117769, tree depth = 81, num leaves = 2342762 (56.89%)"
	time ./uts -t 1 -a 2 -d 16 -b 6 -r 502
	echo "(T3) Binomial ---------------- Tree size = 4112897, tree depth = 1572, num leaves = 3599034 (87.51%)"
	time ./uts -t 0 -b 2000 -q 0.124875 -m 8 -r 42
