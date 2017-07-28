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

# Need to export TPLARG as we'll be reading them from the environment
export TPLARG_REPO_TOP=${REPO_TOP}
export TPLARG_SCALING_TYPE=$1
export TPLARG_NAME=$2
export TPLARG_OCR_INSTALL=$3
export TPLARG_APP_INSTALL=$4
export TPLARG_NB_RUN=${TPLARG_NB_RUN-3}

n=${n-"1"}
c=${c-"2"}

# Job Setup
export TPLARG_QUEUE=${TPLARG_QUEUE-regular}
export TPLARG_HOUR=${TPLARG_HOUR-00}
export TPLARG_MIN=${TPLARG_MIN-15}
# CURDATE=`date`
# export TPLARG_TIME=${TPLARG_TIME-${CURDATE}}

#Local to where the script is invoked under the specific application's folder
TPL_ROOT=./scripts/batch
TPL_NAME=${TPL_NAME-"$TPL_ROOT/${TPLARG_NAME}.btpl"}

SCRIPT_NAME=${0##*/}

if [[ -z "${REPO_TOP}" ]]; then
    echo "error: ${SCRIPT_NAME} environment REPO_TOP is not defined" #"
    exit 1
fi

function generate() {
    # If no job name has been provided, build a default one
    # This is for backward compatibility with older templates. Ideally we should provide
    # some for of string building facility like printf to specify what's in a name
    TMP_JOB_NAME=${TPLARG_JOB_NAME-${TPLARG_SCALING_TYPE}_${TPLARG_NAME}.${TPLARG_NODE_SCALING}N}
    TMP_OUTFILE=${TMP_JOB_NAME-${TPLARG_SCALING_TYPE}_job${TPLARG_NODE_SCALING}}
    TMP_OUTFILE+=".sh"
    OUTNAME=${outdir}/${TMP_OUTFILE}
    echo "generating to TPLARG_NODE_SCALING=${TPLARG_NODE_SCALING} OUTNAME=${OUTNAME}"
    more ${TPL_NAME} \
        | sed -e "s|TPLARG_TIME|`date`|g" \
        > ${OUTNAME}

        # Replace all TPLARG_ defined in the env
        env | grep TPLARG_ | {
        while read tplarg
        do
            parsed=(${tplarg//=/ })
            tplarg_name=${parsed[0]}
            sed --in-place='' -e "s|${tplarg_name}|${!tplarg_name}|g" ${OUTNAME}
        done
        }
        # for tplarg in `env | grep TPLARG_`; do
        # done

        # Insert application specific workload arguments
        for name in `echo ${WORKLOAD_ARGS_NAMES}`; do
            nameU=`echo ${name} | tr '[:lower:]' '[:upper:]'`
            tplarg_name="TPLARG_${nameU}"
            sed --in-place='' -e "s|${tplarg_name}|${!name}|g" ${OUTNAME}
        done
        chmod u+x ${OUTNAME}
        unset TMP_JOB_NAME
        unset TMP_OUTFILE
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

#TODO error if TPL_OUTFILE is set and more than one entry in ${n}

for r in `echo "${n}"`; do
    export TPLARG_NODE_SCALING=$r
    export TPLARG_CORE_SCALING="${TPLARG_CORE_SCALING-${c}}"
    export TPLARG_HOUR="${TPLARG_HOUR}"
    export TPLARG_MIN="${TPLARG_MIN}"
    generate
done

echo "Generated batch files under ${outdir}"
