#!/bin/bash
set -x

####---------------------------------------------------------------------------
# HPGMG 0.1 MPI/OpenMP run function
#
# $1 - Version ('modern' or 'exascale')
# $2 - log2 of box size
# $3 - target boxes / thread
# $4 - number of MPI procs
# $5 - number of MPI processes per node
# $6 - number of OMP threads

run_mpi() {
    export OMP_NUM_THREADS=$6
    mpiexec.hydra -n $4  -ppn $5 ./build-$1/bin/hpgmg-fv $2 $3
    unset OMP_NUM_THREADS
}

usage() {
    echo "Usage: $0 [modern|exascale] [log2 box size] [boxes per thread] [ # MPI procs] [ # MPI procs per node] [ # OMP threads ]"
    exit 1
}

if [ $# -ne 6 ] ; then
    usage
fi

if [ "$1" != "modern" -a "$1" != "exascale" ] ; then
    usage
fi



run_mpi  $1 $2 $3 $4 $5 $6

