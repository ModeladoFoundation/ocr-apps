#!/bin/bash

echo "Running with 2 threads should deadlock."
CONFIG_NUM_THREADS=2 make -f Makefile.x86-mpi run WORKLOAD_ARGS=""
