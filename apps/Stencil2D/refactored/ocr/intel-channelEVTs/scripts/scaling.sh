
#
# Application Scaling Driver
#

if [[ -z "${OCR_TYPE}" ]]; then
    echo "error: OCR_TYPE must be set"
    exit 1
fi

# ROOTs - This is application-specific depending on folder hierarchy

# Path to the folder containing both apps/ and ocr/ GIT repos
# This is computing from this script's parent folder where the
# makefiles to build and run are located.
REPO_TOP=${REPO_TOP-../../../../../..}
APPS_ROOT=${APPS_ROOT-${REPO_TOP}/apps/apps}
OCR_ROOT=${OCR_ROOT-${REPO_TOP}/ocr/ocr}
SCRIPT_ROOT=${OCR_ROOT}/tests/performance-tests/scripts

#
# Configure runs
#

export APP_FOLDER=${PWD}
export APP_NAME=Stencil2D

#
# Default Configuration
#

export COMMON_NAME=report-${APP_NAME}

# Should use Labeled-GUIDs by default
export CFGARG_GUID=${CFGARG_GUID-"LABELED"}

if [[ -z "${SWEEP_FILE}" ]]; then
    export WORKLOAD_ARGS=${WORKLOAD_ARGS-"6 6 4"}
fi


# export NODE_SCALING="4"
# export CORE_SCALING="8"
# export NB_RUN=1

export MAKE_THREADS=16
export RT_CFLAGS=" -DDEBUG_MPI_HOSTNAMES"

. ${SCRIPT_ROOT}/drivers/utils.sh

# export CFLAGS_USER="${RT_CFLAGS} -DREG_ASYNC_SGL -DSTENCIL_WITH_COUNTED_EVT -DALLOW_EAGER_DB -DSTENCIL_WITH_EAGER_DB"
export REPORT_FILENAME_EXT=${REPORT_FILENAME_EXT-"-all-on"}
# buildOcr
. ${APPS_ROOT}/tools/benchmark/${OCR_TYPE}-scaling.sh
