OCR_INPUT_HOME=.
PROG=qsort
SUFFIX=.exe
CC = gcc
CFLAGS= -O0 -g
OCR_FLAGS=-L${OCR_INSTALL}/lib  -I${OCR_INSTALL}/include -locr -lrt
OCR_INPUT=${OCR_INPUT_HOME}/cholesky

ifdef mic
	OCR_FLAGS := ${OCR_FLAGS}  -mmic -L/opt/intel/composer_xe_2013_sp1.1.106/compiler/lib/mic -limf -lsvml -lirng -lintlc
    CC = icc
endif

ifndef OCR_INSTALL
$(error OCR_INSTALL not set)
endif

ifndef OCR_CONFIG
OCR_CONFIG=${OCR_INSTALL}/config/default.cfg
$(warning OCR_CONFIG not set, defaulting to ${OCR_CONFIG})
endif

ifndef OCR_INPUT_HOME
$(error OCR_INPUT_HOME not set)
endif

OCR_RUN_FLAGS=-ocr:cfg ${OCR_CONFIG}

all: compile run
all-test: compile run verify

MATRIX_SIZE=2000
TILE_SIZE=80

INPUT_FILE=m_${MATRIX_SIZE}.in
OUTPUT_FILE=cholesky_out_${MATRIX_SIZE}.txt

compile:
	$(CC)  -g $(CFLAGS) $(OCR_FLAGS) -I. $(PROG).c -o $(PROG)$(SUFFIX) -lm

compile.omp.task:
	gcc -fopenmp $(CFLAGS) $(PROG).omp.task.c -o $(PROG).omp.task$(SUFFIX) -lm

compile_pp:
	g++ $(CFLAGS) $(OCR_FLAGS) -I. $(PROG).cpp -o $(PROG)$(SUFFIX)

run:
	./$(PROG)$(SUFFIX) $(OCR_RUN_FLAGS) ${MATRIX_SIZE} ${TILE_SIZE} ${OCR_INPUT}/${INPUT_FILE}

run.omp.task:
	./$(PROG).omp.task$(SUFFIX) ${MATRIX_SIZE} ${TILE_SIZE} ${OCR_INPUT}/${INPUT_FILE}

clean:
	-rm -Rf *.o $(PROG)$(SUFFIX) cholesky.out

verify:
	diff -b ${OCR_INPUT}/$(OUTPUT_FILE) cholesky.out
