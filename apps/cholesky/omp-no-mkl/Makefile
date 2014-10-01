OCR_INPUT_HOME=../../apps
PROG=omp_cholesky
SUFFIX=.exe
CFLAGS= -O2 -g -DOCR_ENABLE_EDT_NAMING
OCR_FLAGS=-L${OCR_INSTALL}/lib -I${OCR_INSTALL}/include -locr -lpthread -lm
OCR_INPUT=${OCR_INPUT_HOME}/cholesky
CC=icc
CPP=icc

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

all: compile.omp.task
all-test: compile.omp.task run.omp.task verify

MATRIX_SIZE=50
TILE_SIZE=5

INPUT_FILE=m_${MATRIX_SIZE}.in
OUTPUT_FILE=cholesky_out_${MATRIX_SIZE}.txt

compile:
	$(CC) -g $(CFLAGS) $(OCR_FLAGS) -I. $(PROG).c -o $(PROG)$(SUFFIX)

compile.omp.task:
	$(CC) -fopenmp $(CFLAGS) $(PROG).c -o $(PROG)$(SUFFIX) -lm

compile_pp:
	$(CPP) $(CFLAGS) $(OCR_FLAGS) -I. $(PROG).cpp -o $(PROG)$(SUFFIX)

run:
	./$(PROG)$(SUFFIX) $(OCR_RUN_FLAGS) ${MATRIX_SIZE} ${TILE_SIZE} ${OCR_INPUT}/${INPUT_FILE}

run.omp.task:
	./$(PROG)$(SUFFIX) ${MATRIX_SIZE} ${TILE_SIZE} ${OCR_INPUT}/${INPUT_FILE}

clean:
	-rm -Rf *.o $(PROG)$(SUFFIX) $(PROG).out core.* *.csv

verify:
	diff -b ${OCR_INPUT}/$(OUTPUT_FILE) $(PROG).out
