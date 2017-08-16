
#
# Sweeps over Stencil-2D configurations to extract runtime vs user ratios on selected application-level EDTs
#

if [[ -z "${REPO_TOP}" ]]; then
    echo "error: REPO_TOP not defined"
    exit 1
fi

cd ${REPO_TOP}/apps/apps/Stencil2D/refactored/ocr/intel-channelEVTs

# Build OCR for this experiment
function buildOcr {
    echo "No OCR build setup"
    exit 1
}

cd /home/vcave/xstack-reduction-event/apps/apps/Stencil2D/refactored/ocr/intel-channelEVTs

function execute {
	cmd=$1
	if [[ "$cmd" == "res" ]]; then
		CPGN=${CPGNBASE}-${ocr_variant}
		# Transforms addresses into function names and place results in *_named file
		../../../../tools/analysis_tools/metrics/metricsAddrToFuncName.sh ${CPGN}
		# Sweep over that to extract ratio rt/user information for a subset of application level EDTs relevant for Stencil-2D
		for func in `echo "FNC_Rrecv FNC_Lrecv FNC_Brecv FNC_Trecv FNC_Rsend FNC_Lsend FNC_Bsend FNC_Tsend FNC_update"`; do
			for file in `ls -t ${CPGN}/**/**/ws_*_named | sort -t '_' -n -k3`; do
				echo "$file"; more $file | grep "$func" |  grep "ratioRtUser";
			done | sed -e "s/=/ /g";
		done > result-rtVsUser-${CPGN} # Places the output in a result-rtVsUser-* file for each campaign variant
	else
    	c="${cs}" n="${ns}" iter="${iter}" tile="${tile}" TPLARG_MIN=${TPLARG_MIN} TPLARG_QUEUE=${TPLARG_QUEUE} BATCH_TPL=${BATCH_TPL} OCR_INSTALL=~/${OCR_BASE}${ocr_variant} CPGN=${CPGNBASE}-${ocr_variant} ./scripts/driver.sh -${cmd}
    fi
}

# Default values for SLURM job generation
# Campaign base name which will be augmented with the OCR variant name
CPGNBASE=${CPGNBASE-"rtvsuser-metrics-stencil2d"}
# OCR variants base install folder name
OCR_BASE=${OCR_BASE-"ocr_install_mpicc_"}
# OCR variants to sweep over
OCR_VARIANT_LIST=${OCR_VARIANT_LIST-"metrics-worker-edt"}

# Node scaling
ns=${ns-"1"}
# Core scaling
cs=${cs-"17"}
# Iter sweep
iter=${iter-"3000"}
# Tile sweep
tile=${tile-"45 90 180 360 768 1536 3072"}

JOB_PROPERTIES=${JOB_PROPERTIES-./scripts/job.properties}
. ${JOB_PROPERTIES}

if [[ "${1}" == "-buildOcr" ]]; then
    buildOcr
else
    for ocr_variant in `echo ${OCR_VARIANT_LIST}`; do
        while [[ $# -gt 0 ]]; do
            cmd="${1#-}"
            execute ${cmd}
            shift
        done
    done
fi