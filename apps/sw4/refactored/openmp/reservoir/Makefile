CC=icc
CFLAGS=-O3 -march=native -DRSTREAM_IVDEP
LDFLAGS=-lm

SEQ=serial_reference
SEQ_OBJS=seq/micro_kernels.o seq/core2duo_flop.o seq/rhs4sg.o
PAR=rstream_optimized
PAR_OBJS=par/micro_kernels.o par/core2duo_flop.o par/rhs4sg.gen.o

all: setup $(PAR) $(SEQ)

setup:
	mkdir -p par
	mkdir -p seq

clean:
	rm -rf par seq $(PAR) $(SEQ)

.PHONY: all clean setup

par/%.o: %.c
	$(CC) $(CFLAGS) -fopenmp -c $^ -o $@

seq/%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(PAR): $(PAR_OBJS)
	$(CC) $^ $(LDFLAGS) -fopenmp -o $@

$(SEQ): $(SEQ_OBJS)
	$(CC) $^ $(LDFLAGS) -o $@

