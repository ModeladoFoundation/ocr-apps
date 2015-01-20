#!/bin/bash
#set -x
[ $# -ne 2 ] && { echo "Usage: $0 NX NITER "; exit 1; }
echo "Testing NX = $1, NITER = $2"
. $HOME/cnc/1.0.100/bin/cncvars.sh
#./Stencil1D $1 $2 2>&1 | tee log.$1
./Stencil1D $1 $2
