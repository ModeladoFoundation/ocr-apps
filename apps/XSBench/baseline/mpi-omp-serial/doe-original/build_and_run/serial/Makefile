#===============================================================================
# User Options
#===============================================================================

COMPILER    = intel
OPTIMIZE    = yes
DEBUG       = no
PROFILE     = no
MPI         ?= no
PAPI        = no
VEC_INFO    = no
VERIFY      = no
BENCHMARK   = no
BINARY_DUMP = no
BINARY_READ = no

#===============================================================================
# Program name & source code list
#===============================================================================

program = XSBench

#SRC_DIR:=./../../src
SRC_DIR:=$(shell tmydir=`pwd`; tpath="." ; while [[ $$tmydir != "/" ]]; do if [[ -d "build_and_run" ]]; then break; fi; cd ..; tmydir=`pwd`; tpath=$$tpath/..; done; echo $$tpath)/src

source = \
Main.c \
io.c \
CalculateXS.c \
GridInit.c \
XSutils.c \
Materials.c


obj := $(source:.c=.o)

#===============================================================================
# Sets Flags
#===============================================================================

# Standard Flags
CFLAGS := -std=gnu99

# Linker Flags
LDFLAGS = -lm

# Regular gcc Compiler
ifeq ($(COMPILER),gnu)
  CC = gcc
  CFLAGS += -fopenmp
endif

# Intel Compiler
ifeq ($(COMPILER),intel)
  CC = icc
  CFLAGS += -openmp
endif

# BG/Q gcc Cross-Compiler
ifeq ($(MACHINE),bluegene)
  CC = mpiicc
endif

# Debug Flags
ifeq ($(DEBUG),yes)
  CFLAGS += -g
  LDFLAGS  += -g
endif

# Profiling Flags
ifeq ($(PROFILE),yes)
  CFLAGS += -pg
  LDFLAGS  += -pg
endif

# Optimization Flags
ifeq ($(OPTIMIZE),yes)
  CFLAGS += -O3
endif

# Compiler Vectorization (needs -O3 flag) information
ifeq ($(VEC_INFO),yes)
  CFLAGS += -ftree-vectorizer-verbose=6
endif

# PAPI source (you may need to provide -I and -L pointing
# to PAPI depending on your installation
ifeq ($(PAPI),yes)
  source += papi.c
  CFLAGS += -DPAPI
  #CFLAGS += -I/soft/apps/packages/papi/papi-5.1.1/include
  #LDFLAGS += -L/soft/apps/packages/papi/papi-5.1.1/lib -lpapi
  LDFLAGS += -lpapi
endif

# MPI
ifeq ($(MPI),yes)
  CC = mpiicc
  CFLAGS += -DMPI
endif

# Verification of results mode
ifeq ($(VERIFY),yes)
  CFLAGS += -DVERIFICATION
endif

# Adds outer 'benchmarking' loop to do multiple trials for
# 1 < threads <= max_threads
ifeq ($(BENCHMARK),yes)
  CFLAGS += -DBENCHMARK
endif

# Binary dump for file I/O based initialization
ifeq ($(BINARY_DUMP),yes)
  CFLAGS += -DBINARY_DUMP
endif

# Binary read for file I/O based initialization
ifeq ($(BINARY_READ),yes)
  CFLAGS += -DBINARY_READ
endif

source:=$(patsubst %, ${SRC_DIR}/%, $(source))

#===============================================================================
# Targets to Build
#===============================================================================

$(program): $(obj) ${SRC_DIR}/XSbench_header.h
	$(CC) $(CFLAGS) $(obj) -o $@ $(LDFLAGS)

%.o: ${SRC_DIR}/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) ${SRC_DIR}/papi.c ${SRC_DIR}/XSbench_header.h

run:
	./$(program)

bgqrun:
	qsub -t 10 -n 1 -O test XSBench
