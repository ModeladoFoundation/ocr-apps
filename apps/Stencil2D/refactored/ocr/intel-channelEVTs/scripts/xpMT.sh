
#
# Sweep over various build of OCR with micro-tasks for communications turned on
#

if [[ -z "${REPO_TOP}" ]]; then
    echo "error: REPO_TOP not defined"
    exit 1
fi

cd ${REPO_TOP}/apps/apps/Stencil2D/refactored/ocr/intel-channelEVTs

# Build OCR for this experiment
function buildOcr {
    export OCR_SRC_ARG=${REPO_TOP};
    export OCR_ARG=~/ocr_install_mpicc_mt-asan;  export CC=mpicc; unset NO_DEBUG; export OCR_ASAN="yes"; export CFLAGS_USER="-DINIT_DEQUE_CAPACITY=4096 -DNANNYMODE_SYSALLOC -DREG_ASYNC_SGL -DUTASK_COMM"; mkdir -p ${OCR_ARG}; . ${OCR_SRC_ARG}/ocr/ocr/scripts/xs-tools/jump_mpi_ws ${OCR_SRC_ARG} build; cp -R ../install/* ${OCR_ARG}
    export OCR_ARG=~/ocr_install_mpiicc_mt-nodebug; export CC=mpiicc; export NO_DEBUG=yes; unset OCR_ASAN; export CFLAGS_USER="-DINIT_DEQUE_CAPACITY=4096 -DNANNYMODE_SYSALLOC -DREG_ASYNC_SGL -DUTASK_COMM"; mkdir -p ${OCR_ARG}; . ${OCR_SRC_ARG}/ocr/ocr/scripts/xs-tools/jump_mpi_ws ${OCR_SRC_ARG} build; cp -R ../install/* ${OCR_ARG}
    export OCR_ARG=~/ocr_install_mpiicc_mt-nodebug-baseline; export CC=mpiicc; export NO_DEBUG=yes; unset OCR_ASAN; export CFLAGS_USER="-DINIT_DEQUE_CAPACITY=4096 -DNANNYMODE_SYSALLOC -DREG_ASYNC_SGL"; mkdir -p ${OCR_ARG}; . ${OCR_SRC_ARG}/ocr/ocr/scripts/xs-tools/jump_mpi_ws ${OCR_SRC_ARG} build; cp -R ../install/* ${OCR_ARG}
}

# Execute commands provided on the command line
function execute {
    cmd=$1

    if [[ $cmd == "res" ]]; then
        METRIC="flops" SHOW_SCALING=yes c="${cs}" n="${ns}" iter="${iter}" tile="${tile}" TPLARG_MIN=${TPLARG_MIN} TPLARG_QUEUE=${TPLARG_QUEUE} BATCH_TPL=${BATCH_TPL} OCR_INSTALL=~/${OCR_BASE}${ocr_variant} CPGN=${CPGNBASE}-${ocr_variant} ./scripts/driver.sh -${cmd}
    else
        c="${cs}" n="${ns}" iter="${iter}" tile="${tile}" TPLARG_MIN=${TPLARG_MIN} TPLARG_QUEUE=${TPLARG_QUEUE} BATCH_TPL=${BATCH_TPL} OCR_INSTALL=~/${OCR_BASE}${ocr_variant} CPGN=${CPGNBASE}-${ocr_variant} ./scripts/driver.sh -${cmd}
    fi
}

# Default values for SLURM job generation
# Campaign base name which will be augmented with the OCR variant name
CPGNBASE=${CPGNBASE-"xp-mt-stencil2d"}
# OCR variants base install folder name
OCR_BASE=${OCR_BASE-"ocr_install_"}
# OCR variants to sweep over
OCR_VARIANT_LIST=${OCR_VARIANT_LIST-"mpiicc_mt-nodebug mpiicc_mt-nodebug-baseline"}

# Node scaling
ns=${ns-"1 4 16 64 128"}
# Core scaling
cs=${cs-"2 5 17"} #33 hangs
# Iter sweep
iter=${iter-"1000"}
# Tile sweep
tile=${tile-"4 360 768 1536"}

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