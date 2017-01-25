#!/bin/bash

if [ "$#" -ne 4 ]; then
    echo "usage: genBatch.sh scaling_type btpl_name ocr_install app_install"
    exit 1
fi

#
# Sweeps one rank per node
#
LARG_REPO_TOP=${REPO_TOP}
LARG_QUEUE=${LARG_QUEUE-regular}
#ws or ss
LARG_SCALING_TYPE=$1
LARG_NAME=$2
LARG_OCR_INSTALL=$3
LARG_APP_INSTALL=$4

n=${n-"1 4 8 16 32 64 128 256 512"}
c=${c-"16"}
iter=${iter-"200"}
tile=${tile-"2160"}

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
        OUTNAME=${outdir}/${LARG_SCALING_TYPE}_job${NPROC}.sh
        more ${TPL_NAME} \
            | sed -e "s|ARG_REPO_TOP|${LARG_REPO_TOP}|g" \
            | sed -e "s|ARG_NAME|${LARG_NAME}|g" \
            | sed -e "s|ARG_QUEUE|${LARG_QUEUE}|g" \
            | sed -e "s|ARG_TIME|`date`|g" \
            | sed -e "s|ARG_NODE_SCALING|${NPROC}|g" \
            | sed -e "s|ARG_TILE|${LARG_TILE}|g" \
            | sed -e "s|ARG_ITER|${LARG_ITER}|g" \
            | sed -e "s|ARG_HOUR|${LARG_HOUR}|g" \
            | sed -e "s|ARG_MIN|${LARG_MIN}|g" \
            | sed -e "s|ARG_CORE_SCALING|${LARG_CORE_SCALING}|g" \
            | sed -e "s|ARG_SCALING_TYPE|${LARG_SCALING_TYPE}|g" \
            | sed -e "s|ARG_OCR_INSTALL|${LARG_OCR_INSTALL}|g" \
            | sed -e "s|ARG_APP_INSTALL|${LARG_APP_INSTALL}|g" \
            > ${OUTNAME}
            chmod u+x ${OUTNAME}
    done
}

#TODO these should be read from a conf file or something
#They are currently duplicated across scripts

if [[ -z "${outdir}" ]]; then
    export outdir=`mktemp -d jobdir.XXXXXX`
else
    if [[ ! -d "${outdir}" ]]; then
        mkdir ${outdir}
    fi
fi

for r in `echo "$n"`; do
    LARG_NODE_SCALING=$r
    LARG_CORE_SCALING=${c} # Use all cores available
    LARG_HOUR="00"
    LARG_MIN="59"
    LARG_ITER="${iter}"
    LARG_TILE="${tile}"
    generate
done

echo "Generated batch files under $outdir"
#mv ${LARG_SCALING_TYPE}_job* ${TPL_ROOT}
