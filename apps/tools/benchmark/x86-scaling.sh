

if [[ -z "${OCR_ROOT}" ]]; then
    echo "error: OCR_ROOT must be set"
    exit 1
fi

if [[ -z "${OCR_TYPE}" ]]; then
    echo "error: OCR_TYPE must be set"
    exit 1
fi

if [[ -z "${APP_NAME}" ]]; then
    echo "error: APP_NAME must be set"
    exit 1
fi

if [[ -z "${APP_FOLDER}" ]]; then
    echo "error: APP_FOLDER must be set"
    exit 1
fi


#
# Invoke performance driver
#

export SCRIPT_ROOT=${OCR_ROOT}/tests/performance-tests/scripts
export OCR_MAKEFILE="${APP_FOLDER}/Makefile.${OCR_TYPE}"
export RUNNER_TYPE="Application"

# - Caller can override the following parameters
#   'CFLAGS_USER' for compile time
#   'WORKLOAD_ARGS' for runtime
# - Outputs report will be located in the current folder

${SCRIPT_ROOT}/perfDriver.sh -target ${OCR_TYPE} ${APP_NAME}
