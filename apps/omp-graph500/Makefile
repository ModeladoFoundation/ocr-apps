include make.inc

AFL_PATH_FROM_HERE = ../../tools/rmd_afl
AFLINCLUDES = -I$(AFL_PATH_FROM_HERE)/common  -I$(AFL_PATH_FROM_HERE)/rmd_codelet -I$(AFL_PATH_FROM_HERE) -I../../ss/rmdkrnl/inc/  -I../../ss/runtime/common

CDCINCLUDES = -I./generator  
CDCFLAGS=-O3 -g -Wall -DAFL_WRAPPED $(CDCINCLUDES)
CDLDFLAGS=-lpthread -lxml2 -lc -ldl -lz -lm -lrt
CDDEFINES= -DRMD_DB_MEM -DCODELETS 

CFLAGS_OPENMP = -DOPEN_MP -fopenmp

RCKDEFINES= -DAFL_WRAPPED

GENERATOR_SRCS=generator/splittable_mrg.c	\
	generator/graph_generator.c \
        generator/utils.c generator/make_graph.c

GRAPH500_SOURCES= options.c rmat.c kronecker.c verify.c  prng.c \
	xalloc.c timer.c $(GENERATOR_SRCS)

BIN = graph500 ompgraph500

.PHONY: all
all: $(BIN) $(GRAPH500_SOURCES)

%.o: %.c options.h 
	$(CC) -c $(CDDEFINES)  -DPRINT_DEBUG_INFO $(CDCINCLUDES) $(AFLINCLUDES) $(RCKDEFINES) -o $@ $<

ompgraph500: omp-csr.c $(GRAPH500_SOURCES) graph500.c ../../tools/rmd_afl/rmd_codelet/rmd_codelet.a ../../tools/rmd_afl/rmd_afl.a 
	$(CC) -std=c99  $(CDDEFINES) -o $@  $+  $(CFLAGS_OPENMP) $(CDCINCLUDES) $(CDLDFLAGS)


graph500: graph500.o seq-csr.o ../../tools/rmd_afl/rmd_codelet/rmd_codelet.a ../../tools/rmd_afl/rmd_afl.a 
	$(CC) -std=c99  $(CDDEFINES) -o $@  $+ $(GRAPH500_SOURCES) $(CDCINCLUDES) $(CDLDFLAGS)


.PHONY:	clean
clean:
	rm -f *.o *~ map.out graph500 ompgraph500