#!/bin/bash
set -x
[ $# -ne 2 ] && { echo "Usage: $0 NX NITER "; exit 1; }
rm -f ./stencil
gcc -o stencil stencil.c
echo "Testing NX = $1, NITER = $2"
./stencil $1 $2  2>&1 | tee log.$1
