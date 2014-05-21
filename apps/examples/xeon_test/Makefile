PROG=xeon_test
CFLAGS=-O0 -g -Werror -I. -I../../inc -c
OCR_FLAGS=-L${OCR_INSTALL}/lib -L/opt/intel/composer_xe_2013_sp1.1.106/compiler/lib/mic	-limf -lsvml -lirng -lintlc -I${OCR_INSTALL}/include -locr

ifndef OCR_INSTALL
$(error OCR_INSTALL not set)
endif

ifndef OCR_CONFIG
OCR_CONFIG=${OCR_INSTALL}/config/default.cfg
$(warning OCR_CONFIG not set, defaulting to ${OCR_CONFIG})
endif

OCR_RUN_FLAGS=-ocr:cfg ${OCR_CONFIG}

all-test: compile run

compile:
	icc -mmic $(CFLAGS) $(PROG).c
	icc -mmic -g -o $(PROG).exe $(PROG).o $(OCR_FLAGS)

run:
	./$(PROG).exe $(OCR_RUN_FLAGS)

clean:
	-rm -Rf *.o $(PROG).exe
