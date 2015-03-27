#!/bin/bash
set -x
[ $# -ne 2 ] && { echo "Usage: $0 NX NITER "; exit 1; }
make PROG=stencil_send clean
make PROG=stencil_send compile
echo "Testing NX = $1, NITER = $2"
make PROG=stencil_send ARGS="$1 $2" run 2>1 |  tee log_send.$1
#./stencil_send.exe -r 4 $1 $2  2>&1 | tee log_send.$1
 
