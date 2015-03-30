MPILITE=../../../../libs/src/mpilite
MPILITE_INC=../../../../libs/x86/include
MPILITE_LIB=../../../../libs/x86/lib
LIBMPILITE=$(MPILITE_LIB)/libmpilite.a

PROG=stencil
ARGS= 18 10
NUMRANKS=4
MAXTAG=0
CFILES=$(PROG).c  # $(MPILITE)/mpi_ocr.c $(MPILITE)/mpilite.c
OFILES=$(PROG).o   # mpi_ocr.o mpilite.o

# turn off -Werror because ocr-legacy.h issues warnings=>errors
#CFLAGS=-O0 -g -Werror -I. -I../../inc -c -std=c99 
#CFLAGS=-O0 -g -I. -I$(OCR_INSTALL)/include -c -std=c99 -I$(MPILITE)
CFLAGS=-O0 -g -I. -I$(OCR_INSTALL)/include -c -std=c99 -I$(MPILITE_INC)
OCR_FLAGS=-L${OCR_INSTALL}/lib -L$(MPILITE_LIB) -locr -lpthread -lmpilite
#OCR_FLAGS=-L${OCR_INSTALL}/lib -I${OCR_INSTALL}/include -locr -lpthread

ifndef OCR_INSTALL
$(error OCR_INSTALL not set)
endif

ifndef OCR_CONFIG
OCR_CONFIG=${OCR_INSTALL}/config/default.cfg
$(warning OCR_CONFIG not set, defaulting to ${OCR_CONFIG})
endif



OCR_RUN_FLAGS=-ocr:cfg ${OCR_CONFIG}

all-test: compile run

$(LIBMPILITE):
	cd $(MPILITE); make ARCH=x86 install

compile: $(LIBMPILITE)
	gcc $(CFLAGS) $(CFILES)
	gcc -g -o $(PROG).exe $(OFILES) $(OCR_FLAGS)

run:
	./$(PROG).exe $(OCR_RUN_FLAGS) -r $(NUMRANKS) -t $(MAXTAG)  $(ARGS)

clean:
	-rm -Rf *.o $(PROG).exe core.* log*
