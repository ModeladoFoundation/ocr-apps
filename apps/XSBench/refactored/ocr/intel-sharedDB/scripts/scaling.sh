
#
# Application Scaling Driver
#

if [[ -z "${OCR_TYPE}" ]]; then
    echo "error: OCR_TYPE must be set"
    exit 1
fi

# ROOTs - This is application-specific depending on folder hierarchy
REPO_TOP=${REPO_TOP-../../../../../..}
APPS_ROOT=${APPS_ROOT-${REPO_TOP}/apps/apps}
OCR_ROOT=${OCR_ROOT-${REPO_TOP}/ocr/ocr}

#
# Configure runs
#

export APP_FOLDER=${PWD}
export APP_NAME=XSbench

. ${APPS_ROOT}/tools/benchmark/${OCR_TYPE}-scaling.sh