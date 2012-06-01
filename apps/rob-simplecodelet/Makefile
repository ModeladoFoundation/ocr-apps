
AFL_PATH_FROM_HERE = ../../tools/rmd_afl

CDCFLAGS=-O3 -g -Wall -I$(AFL_PATH_FROM_HERE)/common -I$(AFL_PATH_FROM_HERE)/rmd_codelet -I$(AFL_PATH_FROM_HERE)

CDLDFLAGS=-lpthread -lxml2 -lc -ldl -lz -lm -lrt

SOURCES= stest.c

BIN = stest

.PHONY: all
all: $(BIN)

RMD_ALLOVER_INCS =  -I../../ss/rmdkrnl/inc/ -I./generator -I../../tools/rmd_afl/common/ -I../../ss/runtime/common

%.o: %.c $(HEADERS)
	$(CC) -c -DCODELETS -DRMD_DB_MEM -DPRINT_DEBUG_INFO $(RMD_ALLOVER_INCS) $(CDCFLAGS) -o $@ $<

stest:  stest.o  ../../tools/rmd_afl/rmd_codelet/rmd_codelet.a ../../tools/rmd_afl/rmd_afl.a
	$(CC) -std=c99 -o $@  $+  -lpthread -lxml2 -lc -ldl -lz -lm -lrt

.PHONY:	clean
clean:
	rm -f *.o *~ stest


