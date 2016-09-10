#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "usage: submit.sh scaling_type"
    exit 1
fi

# ws or ss
SCALING_TYPE=$1

. ./scripts/batch/jobUtils.sh

# These need to be absolute names to work with slurm
P="${PWD}/scripts/batch"

#TODO these should be read from a conf file or something
#They are currently duplicated across scripts
ranks="1 4 16 64 256"

BATCH_LIST=`ls ${P}/${SCALING_TYPE}_job*.sh`

# Go over the list of batch and submit
submitBatchList "${BATCH_LIST}"
