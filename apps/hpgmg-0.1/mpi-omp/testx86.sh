#!/bin/bash
###############################################################################
#
# Script to test reference hpgmg installation on known good problem sizes
# the example run will use 1 MPI processes with 8 OMP_THREADS
#
# Author: Tom Henretty <henretty@reservoir.com
# Adapted to x86 installation by: Gabriele Jost  <Gabriele.Jost@intel.com>
#
###############################################################################

# $1 - 'modern' or 'exascale'
# $2 - log2 box dim
# $3 - target boxes
# $4 - number of MPI procs
# $5 - number of MPI processes per node
# $6 - number of OMP threads
test() {
    echo "Testing $1 with log2 box dim = $2 and target boxes = $3, please wait..."
    export OMP_NUM_THREADS=$6
    mpiexec.hydra -n $4  -ppn $5 build-$1/bin/hpgmg-fv $2 $3 | grep error | grep h
    #mpiexec.hydra -n $4  -ppn $5 build-$1/bin/hpgmg-fv $2 $3

    if [ "$1" == "modern" ] ; then
    	echo "Expected:"
	MODERN_RESULT=' h =  7.812500000000000e-03  ||error|| =  5.797955958337405e-08'
	echo "$MODERN_RESULT"
    else
    	echo "Expected ('x' values: TBD):"
	EXASCALE_RESULT=' h =  7.812500000000000e-03  ||error|| =  5.77xxxxxxxxxxxxxe-08'
	echo "$EXASCALE_RESULT"
    fi
    echo ""
}


cd $HOME/xstack/apps/hpgmg-0.1/mpi-omp
# Test modern
test modern 4 512 1 1 8
test modern 5 64 1 1 8
test modern 6 8 1 1 8

# Test exascale
test exascale 4 512 1 1 8
test exascale 5 64 1 1 8
test exascale 6 8 1 1 8
