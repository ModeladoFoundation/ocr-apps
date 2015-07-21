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

ARCH           ?= x86
APPS_ROOT      ?= ../../../../
APPS_LIBS_ROOT ?= $(APPS_ROOT)/libs/$(ARCH)

WORKLOAD_SRC          ?= ./src
WORKLOAD_BUILD_ROOT   ?= ./build/$(ARCH)
WORKLOAD_INSTALL_ROOT ?= ./install/$(ARCH)

APPS_ROOT      := $(shell cd "${APPS_ROOT}" && pwd)
APPS_LIBS_ROOT := $(shell cd "${APPS_LIBS_ROOT}" && pwd)

WORKLOAD_SRC          := $(shell cd "${WORKLOAD_SRC}" && pwd)
WORKLOAD_BUILD_ROOT   := $(shell mkdir -p "${WORKLOAD_BUILD_ROOT}" && cd "${WORKLOAD_BUILD_ROOT}" && pwd)
WORKLOAD_INSTALL_ROOT := $(shell mkdir -p "${WORKLOAD_INSTALL_ROOT}" && cd "${WORKLOAD_INSTALL_ROOT}" && pwd)

NL_INC := $(APPS_LIBS_ROOT)/include
NL_LIB := $(APPS_LIBS_ROOT)/lib

CC = gcc
CFLAGS = -std=c11 -fno-builtin
OPTFLAGS = -g -O3
INCLUDES = -isystem $(NL_INC) -I$(WORKLOAD_BUILD_ROOT)
C_LIB = $(NL_LIB)/libc.a $(NL_LIB)/libm.a


### If you need to specify include paths, library paths, or link flags
### for MPI, put them here.  Put both -L and -l switches into MPI_LIB.
MPI_LIB =
MPI_INCLUDE =

### A place to specify any other include or library switches your
### platform requires.
OTHER_LIB = $(APPS_LIBS_ROOT)/lib/libocrscaffold.a
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


# Check for double precision
ifeq ($(DOUBLE_PRECISION), ON)
CFLAGS += -DDOUBLE
else
CFLAGS += -DSINGLE
endif

# Set executable name and add includes & libraries for MPI if needed.
CoMD_VARIANT = CoMD-serial
ifeq ($(DO_MPI), ON)
CoMD_VARIANT = CoMD-mpi
INCLUDES += ${MPI_INCLUDE}
CFLAGS += -DDO_MPI
LDFLAGS += ${MPI_LIB}
endif
CoMD_EXE = $(WORKLOAD_INSTALL_ROOT)/${CoMD_VARIANT}

LDFLAGS += ${C_LIB} ${OTHER_LIB}
CFLAGS  += ${OPTFLAGS} ${INCLUDES} ${OTHER_INCLUDE}


SOURCES=$(wildcard ${WORKLOAD_SRC}/*.c)
OBJECTS=$(patsubst ${WORKLOAD_SRC}/%, $(WORKLOAD_BUILD_ROOT)/%, $(SOURCES:.c=.o))

DEFAULT: $(WORKLOAD_INSTALL_ROOT)/${CoMD_EXE}

${WORKLOAD_BUILD_ROOT}/%.o: ${WORKLOAD_SRC}/%.c | $(WORKLOAD_BUILD_ROOT)
	${CC} ${CFLAGS} -c $< -o $@

$(WORKLOAD_INSTALL_ROOT)/${CoMD_EXE}: $(WORKLOAD_BUILD_ROOT)/CoMD_info.h ${OBJECTS} | $(WORKLOAD_INSTALL_ROOT)
	${CC} ${CFLAGS} -o ${CoMD_EXE} ${OBJECTS} ${LDFLAGS}

$(WORKLOAD_BUILD_ROOT)/CoMD_info.h: Makefile | $(WORKLOAD_BUILD_ROOT)
	${WORKLOAD_SRC}/generate_info_header ${CoMD_VARIANT} "$(CC)" "$(CFLAGS)" "$(LDFLAGS)"
	mv CoMD_info.h $(WORKLOAD_BUILD_ROOT)

clean:
	rm -f $(WORKLOAD_BUILD_ROOT)/*.o $(WORKLOAD_BUILD_ROOT)/CoMD_info.h .depend $(WORKLOAD_INSTALL_ROOT)/${CoMD_EXE}

distclean: clean
	rm -f .depend.bak
	rm -rf html latex

$(WORKLOAD_BUILD_ROOT)/.depend: $(SOURCES)
	@touch $(WORKLOAD_BUILD_ROOT)/.depend
	@$(MAKE) -f Makefile.x86 --no-print-directory depend

depend:
	@echo "Rebuilding dependencies..."
	@$(MAKE) -f Makefile.x86 $(WORKLOAD_BUILD_ROOT)/CoMD_info.h
	@makedepend -f .depend -Y. --$(CFLAGS)-- $(SOURCES) 2> /dev/null

$(WORKLOAD_BUILD_ROOT):
	mkdir -p $(WORKLOAD_BUILD_ROOT)

$(WORKLOAD_INSTALL_ROOT):
	mkdir -p $(WORKLOAD_INSTALL_ROOT)

-include .depend

