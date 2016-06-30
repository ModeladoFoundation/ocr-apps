#!/bin/bash

echo "Running with 4 threads should work."
CONFIG_NUM_THREADS=4 make -f Makefile.x86-mpi run WORKLOAD_ARGS=""
