
SRCS =	src/fir_filter.c \
	src/gen_rand.c \
	src/main.c \
	src/inputs.c

CFLAGS = -fopenmp -Wall -O3 -g -DRAG -DRAG_PARAM_ONLY_off

RM=rm -f

all: datagen

datagen: $(SRCS)
	$(CC) $(CFLAGS) -o $@ -lm -lfftw3f $(SRCS)

run:
	$(MAKE) -C ./huge run
	$(MAKE) -C ./test run

clean:
	$(RM) datagen mpidatagen
	$(MAKE) -C ./huge clean
	$(MAKE) -C ./test clean

install:
	$(MAKE) -C ./huge install
	$(MAKE) -C ./test install

uninstall:
	$(MAKE) -C ./huge uninstall
	$(MAKE) -C ./test uninstall
#
# not tested
#
mpidatagen: $(SRCS)
	mpicc $(CFLAGS) -o $@ -lm -lfftw3f $(SRCS) -DUSE_MPI
