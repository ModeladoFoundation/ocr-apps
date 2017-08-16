#!/bin/bash

if [[ -z "${REPO_TOP}" ]]; then
    echo "error: $0: REPO_TOP must be defined and point to the folder containing apps/ and ocr/"
    exit 1
fi

ANALYSIS_TOOLS="${REPO_TOP}/apps/apps/tools/analysis_tools"

if [[ $# != 1 ]]; then
    echo "error: $0: missing argument - usage: $0 ABSOLUTE_PATH_TO_FOLDER"
    exit 2
fi

python ${ANALYSIS_TOOLS}/metrics/metricsStatistics.py ${ALL_PROFILES_SORTED}
