# Makefile for src-parallel

SHELL = /bin/bash

### CoMD can be built in either double or single precision and with or
### without MPI.  Select desired precision and MPI here.

# double precision (ON/OFF)
DOUBLE_PRECISION = ON
# MPI for parallel (ON/OFF)
DO_MPI = OFF

### Set your desired C compiler and any necessary flags.  Note that CoMD
### uses some c99 features.  You can also set flags for optimization and
### specify paths to include files that the compiler can't find on its
### own.  If you need any -L or -l switches to get C standard libraries
### (such as -lm for the math library) put them in C_LIB.
CC = icc
CFLAGS = -std=c99
OPTFLAGS = -g -O3 -xHost
INCLUDES = -I.
C_LIB = -lm


### If you need to specify include paths, library paths, or link flags
### for MPI, put them here.  Put both -L and -l switches into MPI_LIB.
MPI_LIB =
MPI_INCLUDE =

### A place to specify any other include or library switches your
### platform requires.
OTHER_LIB =
OTHER_INCLUDE =

#########################################
### Below here, it is pitch black.
### You are likely to be eaten by a grue.
##########################################

# clear all suffixes
.SUFFIXES:
# list only those that we use
.SUFFIXES: .c .o

.PHONY: DEFAULT clean distclean depend

#SRC_DIR:=./../../src-mpi
SRC_DIR:=$(shell tmydir=`pwd`; tpath="." ; while [[ $$tmydir != "/" ]]; do if [[ -d "src-mpi" ]]; then break; fi; cd ..; tmydir=`pwd`; tpath=$$tpath/..; done; echo $$tpath)/src-mpi

# Check for double precision
ifeq ($(DOUBLE_PRECISION), ON)
CFLAGS += -DDOUBLE
else
CFLAGS += -DSINGLE
endif

# Set executable name and add includes & libraries for MPI if needed.
CoMD_VARIANT = CoMD
ifeq ($(DO_MPI), ON)
CoMD_VARIANT = CoMD
INCLUDES += ${MPI_INCLUDE}
CFLAGS += -DDO_MPI
LDFLAGS += ${MPI_LIB}
endif
CoMD_EXE = ${CoMD_VARIANT}

LDFLAGS += ${C_LIB} ${OTHER_LIB}
CFLAGS  += ${OPTFLAGS} ${INCLUDES} ${OTHER_INCLUDE}


SOURCES=$(wildcard ${SRC_DIR}/*.c)
OBJECTS=$(patsubst ${SRC_DIR}/%, %, $(SOURCES:.c=.o))

DEFAULT: ${CoMD_EXE}

%.o: ${SRC_DIR}/%.c
	${CC} ${CFLAGS} -c $< -o $@

${CoMD_EXE}: CoMD_info.h ${OBJECTS}
	${CC} ${CFLAGS} -o ${CoMD_EXE} ${OBJECTS} ${LDFLAGS}

CoMD_info.h: Makefile
	${SRC_DIR}/generate_info_header ${CoMD_VARIANT} "$(CC)" "$(CFLAGS)" "$(LDFLAGS)"

clean:
	rm -f *.o CoMD_info.h .depend ${CoMD_EXE}

distclean: clean
	rm -f ${CoMD_EXE} .depend.bak
	rm -rf html latex

.depend: $(SOURCES)
	@touch .depend
	@$(MAKE) -f Makefile --no-print-directory depend

depend:
	@echo "Rebuilding dependencies..."
	@$(MAKE) -f Makefile CoMD_info.h
	@makedepend -f .depend -Y. --$(CFLAGS)-- $(SOURCES) 2> /dev/null


-include .depend
