# Repo top

if [[ -z "${REPO_TOP}" ]]; then
    echo "error: REPO_TOP not defined"
fi

if [[ ! -d ${REPO_TOP}/ocr ]]; then
    echo "error: invalid REPO_TOP, no ocr/ subfolder found under ${REPO_TOP}"
    exit 1
fi

if [[ ! -d ${REPO_TOP}/apps ]]; then
    echo "error: invalid REPO_TOP, no apps/ subfolder found under ${REPO_TOP}"
    exit 1
fi

export APPS_ROOT="${APPS_ROOT-${REPO_TOP}/apps/apps}"

if [[ -z "${MHOME}" ]]; then
    # Set env variables to find OCR and such
    if [[ -d ${SCRATCH} ]]; then
        export MHOME=${SCRATCH} # For NERSC
    else
        export MHOME=${HOME}
    fi
fi

# OCR variant, default search path under MHOME
if [[ -z ${OCR_VARIANT_PATH} ]]; then
    # If no ocr variant path try to dig a default value
    if [[ -z ${OCR_INSTALL} ]]; then
        OCR_INSTALL=${REPO_TOP}/ocr/ocr/install
    fi

    if [[ ! -d ${OCR_INSTALL} ]]; then
        echo "error: invalid OCR_INSTALL ${OCR_INSTALL}"
        exit 1
    else
        OCR_VARIANT_PATH=${OCR_INSTALL}
    fi
fi

#TODO this should be tokenized
if [[ ! -d ${OCR_VARIANT_PATH} ]]; then
    echo "error: invalid OCR_VARIANT_PATH ${OCR_VARIANT_PATH}"
    exit 1
fi

# App variant, default search path is PWD
APP_VARIANT_PATH=${APP_VARIANT_PATH-"${PWD}/install"}

if [[ ! -d ${APP_VARIANT_PATH} ]]; then
    echo "error: invalid APP_VARIANT_PATH ${APP_VARIANT_PATH}"
    exit 1
fi

if [[ ! -d ${CPGN} ]]; then
    mkdir ${CPGN}
    echo "Creating CPGN folder to ${CPGN}"
fi

CMD=$1

function invoke {
    local args_names="$1"
    local workloadString="$2"
    local head="${args_names%% *}"
    local remaining="${args_names#[a-z]* }"
    if [[ "${head}" == "" ]]; then
        workloadStringPrintable=`echo $workloadString | sed -e "s/=/_/g" -e "s/ /./g"`
        for ocrPath in `echo ${OCR_VARIANT_PATH}`; do
            ocrName=${ocrPath##*/}
            #TODO here we're missing name mangling with the app variant
            export outdir="${CPGN}/jobdir"
            if [[ -n ${workloadStringPrintable} ]]; then
                outdir+=".${workloadStringPrintable}"
            fi
            if [[ ! -d ${outdir} ]]; then
                mkdir ${outdir}
            fi
            if [[ "${CMD}" = "run" ]]; then
                CURDATE=`date +%F_%Hh%M`
                #TODO: should mangle the script name with the output job name first
                EXP_OUTDIR="${outdir}/job_output_${ocrName}"
                if [[ -n "${BATCH_SCRIPT}" ]]; then
                    EXP_OUTDIR+="_"
                    EXP_OUTDIR+=`echo ${BATCH_SCRIPT} | sed -e "s/\.sh//g"`
                fi
                EXP_OUTDIR+="_${CURDATE}.XXX"
                export EXP_OUTDIR=`mktemp -d ${EXP_OUTDIR}`
                echo "BATCH_SCRIPT=${BATCH_SCRIPT} EXP_OUTDIR=${EXP_OUTDIR} outdir=$PWD/${outdir}"
                ranks="${n}" ${APPS_ROOT}/tools/execution_tools/batch/submit.sh ws "$PWD/$outdir" ${BATCH_SCRIPT}
            elif [[ "${CMD}" = "res" ]]; then
                for d in `ls -d ${outdir}/job_output_*`; do
                    echo "Processing ${d}"
                    ranks="${n}" ${APPS_ROOT}/tools/execution_tools/batch/showMultiResults.sh "${d}" "${c}" pes_comp
                done
            else
                echo "tools: BATCH_TPL=${BATCH_TPL} c=${c} n=${n}"
                echo "OCR_VARIANT_PATH=${OCR_VARIANT_PATH}" > ${outdir}/log_install_used
                echo "APP_VARIANT_PATH=${APP_VARIANT_PATH}" >> ${outdir}/log_install_used
                echo "WORKLOAD_ARGS_NAMES=\"${WORKLOAD_ARGS_NAMES}\" ${workloadString} ${APPS_ROOT}/tools/execution_tools/batch/genBatch.sh ws ${BATCH_TPL} ${OCR_VARIANT_PATH} ${APP_VARIANT_PATH}"
                eval "WORKLOAD_ARGS_NAMES=\"${WORKLOAD_ARGS_NAMES}\" ${workloadString} ${APPS_ROOT}/tools/execution_tools/batch/genBatch.sh ws ${BATCH_TPL} ${OCR_VARIANT_PATH} ${APP_VARIANT_PATH}"
            fi
        done
    else
        for value in `echo ${!head}`; do
            invoke "${remaining}" "${workloadString} ${head}=${value}"
        done
    fi
}

invoke "${WORKLOAD_ARGS_NAMES}" ""
