ifndef OPT
	OPT=-O
endif

ifndef RMD_INSTALL_BASE
	RMD_INSTALL_BASE = ../../../ss
endif
ifndef OCR_INSTALL
	OCR_INSTALL = ../../../ocr/runtime/ocr-x86/ocr-install
endif

GCC	= gcc -std=c99 -DOCR
## -DOCR_DEBUG
CFLAGS	= -fpic -g $(OPT) -I$(OCR_INSTALL)/include -I../x86_64/include
LDFLAGS	= -fpic -g $(OPT) -L$(OCR_INSTALL)/lib -L../x86_64/lib
LDLIBS	= -locr -lcrlibm

all: test

test.o:  test.c Makefile
	$(GCC) $(CFLAGS) -c test.c

test:	test.o Makefile
	$(GCC) $(LDFLAGS) -o test test.o $(LDLIBS)

run:	test
	./test -ocr:cfg ./default.cfg

clean:
	rm -f test.o test
