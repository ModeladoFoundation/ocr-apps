FFTW_INC=/usr/local/include

CC=gcc
CFLAGS=-g -std=c99 -I../src -I/$(FFTW_INC)
LDFLAGS=-lm

all:	viewImage    viewCorr    dataImage \
	viewImageTxt viewCorrTxt detects_ascii_to_txt

inputs.o: ../src/common.h  ../src/inputs.c
	$(CC) $(CFLAGS) -c ../src/inputs.c

viewImage: ../src/common.h           viewImage.c inputs.o
	$(CC) $(CFLAGS) -o viewImage viewImage.c  inputs.o $(LDFLAGS)

viewCorr: ../src/common.h           viewCorr.c inputs.o
	$(CC) $(CFLAGS) -o viewCorr viewCorr.c inputs.o $(LDFLAGS)

dataImage: ../src/common.h           dataImage.c inputs.o
	$(CC) $(CFLAGS) -o dataImage dataImage.c inputs.o $(LDFLAGS)

viewImageTxt: ../src/common.h           viewImageTxt.c inputs.o
	$(CC) $(CFLAGS) -o viewImageTxt viewImageTxt.c inputs.o $(LDFLAGS)

viewCorrTxt: ../src/common.h            viewCorrTxt.c inputs.o
	$(CC) $(CFLAGS) -o viewCorrTxt  viewCorrTxt.c inputs.o $(LDFLAGS)

detects_ascii_to_txt:	                        detects_ascii_to_txt.c
	$(CC) $(CFLAGS) -o detects_ascii_to_txt detects_ascii_to_txt.c $(LDFLAGS)

clean:
	$(RM) viewImage    viewCorr    dataImage inputs.o \
	      viewImageTxt viewCorrTxt detects_ascii_to_txt
