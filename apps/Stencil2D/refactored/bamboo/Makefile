# Copyright (c) 2015, The Regents of the University of California.
# Produced at the University of California, San Diego
# by Tan Nguyen, Sergio Martin, Scott Baden, and Pietro Cicotti
# All rights reserved.
#
BAMBOO_PATH ?= /opt/bamboo_6_0/bin
MPI_PATH ?= /opt/intel/tools/compilers_and_libraries_2016.0.109/linux/mpi/intel64

CFLAGS = -O2
CC = $(MPI_PATH)/bin/mpicxx


.PHONY: all
all: run


bamboo_stencil: bamboo_stencil.cpp MPI_bail_out.c wtime.c
	$(BAMBOO_PATH)/bamboo-cxx  -I. -I$(BAMBOO_PATH) -I/usr/include -o $@ $^

bamboo_stencil.cpp: stencil.c
	$(BAMBOO_PATH)/bamboo-translate -I. -I$(BAMBOO_PATH) stencil.c

run: bamboo_stencil
	$(BAMBOO_PATH)/bamboo-run -w 4 -x 4 ./bamboo_stencil 4 6


.PHONY: clean
clean:
	$(RM) $(BINARIES) *.o bamboo*


