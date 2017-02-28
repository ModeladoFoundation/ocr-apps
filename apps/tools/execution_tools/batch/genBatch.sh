#!/bin/bash

#
# Generates a batch file from a template
#
# Arguments are:
#   - scaling_type :  'ws' | 'ss'
#   - btpl_name    :  name of template without .btpl extension
#   - ocr_install  :  OCR install to use
#   - app_install  :  APP install to use
#
# A number of values are sed into the template. The convention is to name them starting with 'TPLARG_'
# Default values are provided in this script but can be overriden from the caller's environment
#
#
# Workload-specific arguments:
#
# The script will check 'WORKLOAD_ARGS_NAMES' for a list of variables names
# and sed appropriate TPLARG_NAME in the template file with the value returned by ${name}
# This is just a generic way for setting workload specific values in the batch template.
#

if [ "$#" -ne 4 ]; then
    echo "usage: genBatch.sh scaling_type btpl_name ocr_install app_install"
    exit 1
fi

TPLARG_REPO_TOP=${REPO_TOP}
TPLARG_SCALING_TYPE=$1
TPLARG_NAME=$2
TPLARG_OCR_INSTALL=$3
TPLARG_APP_INSTALL=$4
TPLARG_NB_RUN=${TPLARG_NB_RUN-3}

n=${n-"1"}
c=${c-"2"}

# Job Setup
TPLARG_QUEUE=${TPLARG_QUEUE-regular}
TPLARG_HOUR=${TPLARG_HOUR-00}
TPLARG_MIN=${TPLARG_MIN-15}

#Local to where the script is invoked under the specific application's folder
TPL_ROOT=./scripts/batch
TPL_NAME=${TPL_NAME-"$TPL_ROOT/${TPLARG_NAME}.btpl"}

SCRIPT_NAME=${0##*/}

if [[ -z "${REPO_TOP}" ]]; then
    echo "error: ${SCRIPT_NAME} environment REPO_TOP is not defined"
    exit 1
fi

function generate() {
    for NPROC in `echo "${TPLARG_NODE_SCALING}"`; do
        OUTNAME=${outdir}/${TPLARG_SCALING_TYPE}_job${NPROC}.sh
        more ${TPL_NAME} \
            | sed -e "s|TPLARG_REPO_TOP|${TPLARG_REPO_TOP}|g" \
            | sed -e "s|TPLARG_NAME|${TPLARG_NAME}|g" \
            | sed -e "s|TPLARG_QUEUE|${TPLARG_QUEUE}|g" \
            | sed -e "s|TPLARG_TIME|`date`|g" \
            | sed -e "s|TPLARG_NODE_SCALING|${NPROC}|g" \
            | sed -e "s|TPLARG_HOUR|${TPLARG_HOUR}|g" \
            | sed -e "s|TPLARG_MIN|${TPLARG_MIN}|g" \
            | sed -e "s|TPLARG_NB_RUN|${TPLARG_NB_RUN}|g" \
            | sed -e "s|TPLARG_CORE_SCALING|${TPLARG_CORE_SCALING}|g" \
            | sed -e "s|TPLARG_SCALING_TYPE|${TPLARG_SCALING_TYPE}|g" \
            | sed -e "s|TPLARG_OCR_INSTALL|${TPLARG_OCR_INSTALL}|g" \
            | sed -e "s|TPLARG_APP_INSTALL|${TPLARG_APP_INSTALL}|g" \
            > ${OUTNAME}
            # Inserting application specific workload arguments
            for name in `echo ${WORKLOAD_ARGS_NAMES}`; do
                nameU=`echo ${name} | tr '[:lower:]' '[:upper:]'`
                tplarg_name="TPLARG_${nameU}"
                sed --in-place='' -e "s|${tplarg_name}|${!name}|g" ${OUTNAME}
            done
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

for r in `echo "${n}"`; do
    TPLARG_NODE_SCALING=$r
    TPLARG_CORE_SCALING=${c} # Use all cores available
    TPLARG_HOUR="${TPLARG_HOUR}"
    TPLARG_MIN="${TPLARG_MIN}"
    generate
done

echo "Generated batch files under ${outdir}"
