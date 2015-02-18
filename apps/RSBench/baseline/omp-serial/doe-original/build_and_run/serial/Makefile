#===============================================================================
# User Options
#===============================================================================

COMPILER  = intel
OPTIMIZE  = yes
DEBUG     = no
PROFILE   = no
STATUS    = yes
PAPI      = no

#===============================================================================
# Program name & source code list
#===============================================================================

program = rsbench

#SRC_DIR:=./../../src
SRC_DIR:=$(shell tmydir=`pwd`; tpath="." ; while [[ $$tmydir != "/" ]]; do if [[ -d "build_and_run" ]]; then break; fi; cd ..; tmydir=`pwd`; tpath=$$tpath/..; done; echo $$tpath)/src

source = \
main.c \
xs_kernel.c \
io.c \
init.c \
material.c \
utils.c

obj := $(source:.c=.o)

#===============================================================================
# Sets Flags
#===============================================================================

# Standard Flags
CFLAGS := -std=gnu99

# gcc Compiler
ifeq ($(COMPILER),gnu)
  CC = gcc
  CFLAGS += -fopenmp -ffast-math
endif

# intel compiler
ifeq ($(COMPILER),intel)
  CC = icc
  CFLAGS += -openmp -xhost -ansi-alias -no-prec-div
endif

# PGI compiler (thanks Chip!)
ifeq ($(COMPILER),pgi)
  CC = pgcc
  CFLAGS := -mp -fastsse
endif

# Linker Flags
LDFLAGS = -lm

# Debug Flags
ifeq ($(DEBUG),yes)
  CFLAGS += -g
  LDFLAGS += -g
endif

# Profiling Flags
ifeq ($(PROFILE),yes)
  CFLAGS += -pg
  LDFLAGS  += -pg
endif

# Optimization Flags
ifeq ($(OPTIMIZE),yes)
  CFLAGS += -Ofast
endif

# Calculation Status % Complete Indicator
ifeq ($(STATUS),yes)
  CFLAGS += -DSTATUS
endif

# PAPI Performance Counters
ifeq ($(PAPI),yes)
  CFLAGS += -DPAPI
  source += papi.c
  LDFLAGS += -lpapi
endif

source:=$(patsubst %, ${SRC_DIR}/%, $(source))

#===============================================================================
# Targets to Build
#===============================================================================

$(program): $(obj) ${SRC_DIR}/rsbench.h
	$(CC) $(CFLAGS) $(obj) -o $@ $(LDFLAGS)

%.o: ${SRC_DIR}/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf rsbench $(obj)

edit:
	vim -p $(source) ${SRC_DIR}/papi.c ${SRC_DIR}/rsbench.h

run:
	./rsbench
