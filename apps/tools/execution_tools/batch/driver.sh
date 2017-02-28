# Repo top
export REPO_TOP=${REPO_TOP-${PWD}/../../../../../..}
export APPS_ROOT=$PWD/../../../..

if [[ -z "${MHOME}" ]]; then
	# Set env variables to find OCR and such
	if [[ -d ${SCRATCH} ]]; then
		export MHOME=${SCRATCH} # For NERSC
	else
		export MHOME=${HOME}
	fi
	echo "Setting MHOME to ${MHOME}"
fi

# OCR variant, default search path under MHOME
if [[ -z ${OCR_INSTALL} ]]; then
	# Lookup default OCR install path
	OCR_VARIANT_PATH=${OCR_VARIANT_PATH-"ocr_install_mpiicc_d10_base"}
	if [[ ! -d ${OCR_VARIANT_PATH} ]]; then
		OCR_VARIANT_PATH="${MHOME}/${OCR_VARIANT_PATH}"
		if [[ ! -d ${OCR_VARIANT_PATH} ]]; then
			echo "error: invalid OCR_VARIANT_PATH ${OCR_VARIANT_PATH}"
			exit 1
		fi
	fi
else
	if [[ ! -d ${OCR_INSTALL} ]]; then
		echo "error: invalid OCR_INSTALL ${OCR_INSTALL}"
		exit 1
	else
		OCR_VARIANT_PATH=${OCR_INSTALL}
	fi
fi

# App variant, default search path is PWD
APP_VARIANT_PATH="${PWD}/install"
if [[ ! -d ${APP_VARIANT_PATH} ]]; then
	echo "error: invalid APP_VARIANT_PATH ${APP_VARIANT_PATH}"
	exit 1
fi


if [[ ! -d ${REPO_TOP} ]]; then
	echo "error: invalid REPO_TOP ${REPO_TOP}"
	exit 1
fi

if [[ ! -d ${CPGN} ]]; then
	mkdir ${CPGN}
	echo "Creating CPGN folder to ${CPGN}"
fi

# OCR metrics specific flags
# Stats requires single value for 'n' and 'c'
#export COMM_METRICS="yes"
#export EXTRA_FLAGS="-DSTAT_MSG -DSTAT_EDT_EXEC -DMETRIC_MAX_ENTRY_COUNT=1100"

function invoke {
	local args_names="$1"
	local workloadString="$2"
	local head="${args_names%% *}"
	local remaining="${args_names#[a-z]* }"
	echo "args_names=${args_names}"
	echo "workloadString=${workloadString}"
	echo "head=${head}"
	echo "remaining=${remaining}"
	if [[ "${head}" == "" ]]; then
		workloadStringPrintable=`echo $workloadString | sed -e "s/=/_/g" -e "s/ /./g"`
		for ocrPath in `echo ${OCR_VARIANT_PATH}`; do
			ocrName=${ocrPath##*/}
			#TODO here we're missing name mangling with both the app and ocr variant
			export outdir="${CPGN}/jobdir.${workloadStringPrintable}"
			if [[ ! -d ${outdir} ]]; then
				mkdir ${outdir}
			fi
			if [[ "$1" = "run" ]]; then
				CURDATE=`date +%F_%Hh%M`
				export EXP_OUTDIR=`mktemp -d ${outdir}/job_output_${ocrName}_${CURDATE}.XXX`
				ranks="${n}" ${APPS_ROOT}/tools/execution_tools/batch/submit.sh ws "$PWD/$outdir"
			elif [[ "$1" = "res" ]]; then
				for d in `ls -d ${outdir}/job_output_*`; do
					echo "${d}"
					ranks="${n}" ./scripts/batch/showMultiResults.sh "${d}" "${c}" pes_comp
				done
			else
				# APP_BATCH_ARGS="iter=$i tile=$t"
				# Resolve stuff
				echo "OCR_VARIANT_PATH=${OCR_VARIANT_PATH}" > ${outdir}/log_install_used
				echo "APP_VARIANT_PATH=${APP_VARIANT_PATH}" >> ${outdir}/log_install_used
				eval "WORKLOAD_ARGS_NAMES=\"${WORKLOAD_ARGS_NAMES}\" ${workloadString} ${APPS_ROOT}/tools/execution_tools/batch/genBatch.sh ws ${BATCH_TPL} ${OCR_VARIANT_PATH} ${APP_VARIANT_PATH}"
			fi
		done
	else
		for value in `echo ${!head}`; do
			echo "invoke ${remaining} "
			invoke "${remaining}" "${workloadString} ${head}=${value}"
		done
	fi
}

invoke "${WORKLOAD_ARGS_NAMES}" ""