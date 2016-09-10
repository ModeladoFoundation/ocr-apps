#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "usage: genBatch.sh scaling_type"
    exit 1
fi

#
# Sweeps one rank per node
#
LARG_REPO_TOP=${REPO_TOP}
LARG_NAME="Stencil2D"
LARG_QUEUE=regular
#ws or ss
LARG_SCALING_TYPE=$1

ranks="1 4 16 32 64 256"
c=${c-"16"}

TPL_ROOT=./scripts/batch
TPL_NAME=${TPL_NAME-"$TPL_ROOT/${LARG_NAME}.btpl"}

LARG_MIN=05

SCRIPT_NAME=${0##*/}

if [[ -z "${REPO_TOP}" ]]; then
    echo "error: ${SCRIPT_NAME} environment REPO_TOP is not defined"
    exit 1
fi

function generate() {
    for NPROC in `echo "${LARG_NODE_SCALING}"`; do
        OUTNAME=${LARG_SCALING_TYPE}_job${NPROC}.sh
        rm -f ${OUTNAME} 2>/dev/null
        more ${TPL_NAME} \
            | sed -e "s|ARG_REPO_TOP|${LARG_REPO_TOP}|g" \
            | sed -e "s|ARG_NAME|${LARG_NAME}|g" \
            | sed -e "s|ARG_QUEUE|${LARG_QUEUE}|g" \
            | sed -e "s|ARG_TIME|`date`|g" \
            | sed -e "s|ARG_NODE_SCALING|${NPROC}|g" \
            | sed -e "s|ARG_HOUR|${LARG_HOUR}|g" \
            | sed -e "s|ARG_MIN|${LARG_MIN}|g" \
            | sed -e "s|ARG_CORE_SCALING|${LARG_CORE_SCALING}|g" \
            | sed -e "s|ARG_SCALING_TYPE|${LARG_SCALING_TYPE}|g" \
            > ${OUTNAME}
            chmod u+x ${OUTNAME}
    done
}

#TODO these should be read from a conf file or something
#They are currently duplicated across scripts

for r in `echo "$ranks"`; do
    LARG_NODE_SCALING=$r
    LARG_CORE_SCALING=${c} # Use all cores available
    LARG_HOUR="00"
    LARG_MIN="10"
    generate
done

mv ${LARG_SCALING_TYPE}_job* ${TPL_ROOT}
