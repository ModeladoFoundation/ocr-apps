#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "usage: submit.sh scaling_type jobdir"
    exit 1
fi

# ws or ss
SCALING_TYPE=$1
JOBDIR=$2

. ./scripts/batch/jobUtils.sh

# These need to be absolute names to work with slurm

#TODO these should be read from a conf file or something
#They are currently duplicated across scripts
ranks=${ranks-""}

if [[ -z "${ranks}" ]]; then
    BATCH_LIST=`ls ${JOBDIR}/${SCALING_TYPE}_job*.sh`
else
    BATCH_LIST=""
    for r in `echo $ranks`; do
        BATCH_LIST+="${JOBDIR}/${SCALING_TYPE}_job${r}.sh "
    done
fi

# Go over the list of batch and submit
submitBatchList "${BATCH_LIST}"
