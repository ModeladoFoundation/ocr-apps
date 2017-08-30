
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
export APP_NAME=hpcgEagerRedevt

#
# Default Configuration
#

export COMMON_NAME=report-${APP_NAME}

# Should use Labeled-GUIDs by default
export CFGARG_GUID=${CFGARG_GUID-"LABELED"}

if [[ -z "${SWEEP_FILE}" ]]; then
    export WORKLOAD_ARGS=${WORKLOAD_ARGS-"2 2 2 64 10 0"}
fi

. ${SCRIPT_ROOT}/drivers/utils.sh

export REPORT_FILENAME_EXT=${REPORT_FILENAME_EXT-"-all-on"}
. ${APPS_ROOT}/tools/benchmark/${OCR_TYPE}-scaling.sh
