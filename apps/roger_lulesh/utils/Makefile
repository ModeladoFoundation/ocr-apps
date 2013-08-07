ifndef OPT
	OPT=-O
endif

GCC=gcc -std=c99
CFLAGS= -fpic -g $(OPT)
LDFLAGS = -lm

all: hex2sci

hex2sci:	hex2sci.c
	$(GCC) $(CFLAGS) -o hex2sci hex2sci.c $(LDFLAGS)

clean:
	rm -f hex2sci
