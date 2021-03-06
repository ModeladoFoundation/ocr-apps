#!/bin/bash

if [ "$#" -lt 2 ]; then
    echo "usage: submit.sh scaling_type jobdir [batch_script]"
    exit 1
fi

# ws or ss
SCALING_TYPE=$1
JOBDIR=$2

if [ "$#" -eq 3 ]; then
    BATCH_SCRIPT=$3
fi

if [ -z "${APPS_ROOT}" ]; then
    echo "error: APPS_ROOT must be set"
    exit 1
fi

. ${APPS_ROOT}/tools/execution_tools/batch/jobUtils.sh

# These need to be absolute names to work with slurm

if [[ -z "${BATCH_SCRIPT}" ]]; then
    #TODO these should be read from a conf file or something
    #They are currently duplicated across scripts
    ranks=${ranks-""}

    if [[ -z "${ranks}" ]]; then
        BATCH_LIST=`ls ${JOBDIR}/${SCALING_TYPE}_*.sh`
    else
        BATCH_LIST=""
        for r in `echo $ranks`; do
            BATCH_LIST+="${JOBDIR}/${SCALING_TYPE}_job${r}.sh "
        done
    fi
else
    BATCH_LIST="${JOBDIR}/${BATCH_SCRIPT}"
fi

# Go over the list of batch and submit
echo "submitBatchList ${BATCH_LIST}"
submitBatchList "${BATCH_LIST}"
