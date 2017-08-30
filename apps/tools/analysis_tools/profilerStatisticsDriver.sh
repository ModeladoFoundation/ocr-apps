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

# input: Folder to recursively look for profiles
XP_FOLDER=$1

ALL_PROFILES=`${ANALYSIS_TOOLS}/extractProfiles.sh "${XP_FOLDER}"`

echo "done extracting ${ALL_PROFILES}"

# Sort profiles per number of threads
# TODO: uses file size for now
ALL_PROFILES_SORTED=`ls -rS ${ALL_PROFILES}`

python ${ANALYSIS_TOOLS}/psl/profilerStatistics.py ${ALL_PROFILES_SORTED}
