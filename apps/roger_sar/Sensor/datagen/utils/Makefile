FFTW_INC=/usr/local/include

CC=gcc
CFLAGS=-g -std=c99 -I../src -I$(FFTW_INC)
LDFLAGS=-lm

all:	viewData

inputs.o: ../src/common.h ../src/inputs.c $(FFTW_INC)/fftw3.h
	$(CC) $(CFLAGS) -c ../src/inputs.c

viewData:	../src/common.h viewData.c inputs.o $(FFTW_INC)/fftw3.h
	$(CC) $(CFLAGS) -o viewData viewData.c inputs.o $(LDFLAGS)

clean:
	$(RM) viewData inputs.o
