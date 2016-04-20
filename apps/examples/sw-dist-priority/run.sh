#!/bin/bash

export CONFIG_NUM_THREADS=2
export OCR_NODEFILE="$PWD/hosts2.txt"
export CONFIG_FLAGS="--scheduler=PRIORITY"

rm -f ./install/x86-mpi/generated.cfg

for x in 0 1; do
    export BAD_AFFINITIES=$x
    echo BAD_AFFINITIES=$BAD_AFFINITIES
    RUN_TOOL=time make -f Makefile.x86-mpi run WORKLOAD_ARGS="5 5 ${PWD}/../../../apps/smithwaterman/datasets/string1-medium.txt ${PWD}/../../../apps/smithwaterman/datasets/string2-medium.txt ${PWD}/../../../apps/smithwaterman/datasets/score-medium.txt" 2>&1 | tee out${BAD_AFFINITIES}.txt
done
