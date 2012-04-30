FFTW_INC=-I/usr/local/include
FFTW_LIB=-L/use/local/lib -lfftw3f

all: datagen ./utils/viewData

SOURCES = \
	src/fir_filter.c \
	src/gen_rand.c \
	src/main.c \
	src/inputs.c 

CC=gcc
CFLAGS= -g -std=gnu99  -DRAG_ADD_ERROR=1
LDFLAGS=-lm


./utils/viewData: ./utils/viewData.c
	$(MAKE) -C ./utils

datagen: $(SOURCES)
	$(CC) $(CFLAGS) -o $@ -O $(FFTW_INC) $(SOURCES) $(FFTW_LIB) $(LDFLAGS)

run:
	$(MAKE) -C ./small run
	$(MAKE) -C ./large run

clean:
	$(RM) datagen
	$(MAKE) -C ./utils clean
	$(MAKE) -C ./small clean
	$(MAKE) -C ./large clean
