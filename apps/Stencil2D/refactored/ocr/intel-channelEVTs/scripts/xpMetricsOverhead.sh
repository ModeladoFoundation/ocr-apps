
#
# Metrics overhead experiment
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

function execute {
    cmd=$1
    # SHOW_SCALING only relevant for -res
    for ocr_variant in `echo ${OCR_VARIANT_LIST}`; do
        SHOW_SCALING=yes c="${cs}" n="${ns}" iter="${iter}" tile="${tile}" TPLARG_MIN=${TPLARG_MIN} TPLARG_QUEUE=${TPLARG_QUEUE} BATCH_TPL=${BATCH_TPL} OCR_INSTALL=~/${OCR_BASE}${ocr_variant} CPGN=${CPGNBASE}-${ocr_variant} ./scripts/driver.sh -${cmd}
    done
}

# Default values for SLURM job generation
# Campaign base name which will be augmented with the OCR variant name
CPGNBASE=${CPGNBASE-"overhead-metrics-stencil2d"}
# OCR variants base install folder name
OCR_BASE=${OCR_BASE-"ocr_install_mpicc_"}
# OCR variants to sweep over
OCR_VARIANT_LIST=${OCR_VARIANT_LIST-"metrics-none metrics-worker metrics-edt metrics-worker-edt"}

# Node scaling
ns=${ns-"1"}
# Core scaling
cs=${cs-"2 5 17 33"}
# Iter sweep
iter=${iter-"1000"}
# Tile sweep
tile=${tile-"768"}

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