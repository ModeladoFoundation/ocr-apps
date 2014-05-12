FFTW_INC=-I/usr/include
FFTW_LIB=-L/usr/lib64 -lfftw3f

SOURCES = \
	src/fir_filter.c \
	src/gen_rand.c \
	src/main.c \
	src/inputs.c

CC=gcc
CFLAGS= -g -std=gnu99  -DRAG_ADD_ERROR=1
LDFLAGS=-lm

all: datagen ./utils/viewData

./utils/viewData: ./utils/viewData.c
	$(MAKE) -C ./utils all

datagen: $(SOURCES)
	$(CC) $(CFLAGS) -o $@ -O $(FFTW_INC) $(SOURCES) $(FFTW_LIB) $(LDFLAGS)

tiny:	all
	$(MAKE) -C ./tiny run

small:	all
	$(MAKE) -C ./small run

medium:	all
	$(MAKE) -C ./medium run

large:	all
	$(MAKE) -C ./large run

clean:
	$(RM) datagen
	$(MAKE) -C ./utils  clean
	$(MAKE) -C ./tiny   clean
	$(MAKE) -C ./small  clean
	$(MAKE) -C ./medium clean
	$(MAKE) -C ./large  clean
