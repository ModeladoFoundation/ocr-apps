#!/bin/bash
set -x
[ $# -ne 2 ] && { echo "Usage: $0 NX NITER "; exit 1; }
make -f Makefile.x86 TARGET=stencil clean install
echo "Testing NX = $1, NITER = $2"
make -f Makefile.x86 TARGET=stencil WORKLOAD_ARGS="-r 8 -t 0 $1 $2" CONFIG_NUM_THREADS=8 run 2>&1 |  tee log.$1


