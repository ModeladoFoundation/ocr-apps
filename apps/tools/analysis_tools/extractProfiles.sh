#!/bin/bash

if [[ -z "${REPO_TOP}" ]]; then
    echo "error: $0: REPO_TOP must be defined and point to the folder containing apps/ and ocr/"
    exit 1
fi

OCR_TOP=${OCR_TOP-${REPO_TOP}/ocr}

if [[ $# != 1 ]]; then
    echo "error: $0: missing argument - usage: $0 ABSOLUTE_PATH_TO_FOLDER"
    exit 2
fi

# input: Folder to recursively look for profiles
XP_FOLDER=$1

# output:
# - to filesystem: one profile file per folder where profiling output has been found
# - to stdout: a whitespace separated list of profiles generated or found

# Default output filename
PROFILER_DEFAULT_OUTPUT=${PROFILER_DEFAULT_OUTPUT-all.prof}

# Set to yes in caller to force regeneration of profile
PROFILER_FORCE_GENERATION=${PROFILER_FORCE_GENERATION-no}

# Since there's always at least one thread, just look for thread zero profile
ALL_PROFILE_ZERO=`find ${XP_FOLDER} -name "profiler_0*"`

# Extract folder names
ALL_PROFILE_FOLDERS=""
for file in `echo ${ALL_PROFILE_ZERO}`; do
    folder=`dirname $file`
    ALL_PROFILE_FOLDERS+="$folder "
done

# clean-up candidates
ALL_PROFILE_FOLDERS=`echo $ALL_PROFILE_FOLDERS | tr ' ' '\n' | sort | uniq`
ALL_PROFILES=""
# Check if profile extraction already happened
for folder in `echo ${ALL_PROFILE_FOLDERS}`; do
    OUTPUT="${folder}/${PROFILER_DEFAULT_OUTPUT}"
    if [[ ( ! -f "${OUTPUT}" ) || ( "${PROFILER_FORCE_GENERATION}" == "yes" ) ]]; then
        cd ${folder}
        if [[ "${PROFILER_EXCLUDE_COMMWORKER}" == "yes" ]]; then
            $OCR_TOP/ocr/scripts/Profiler/analyzeProfile.py -e "hc_worker_comm" -t '*' > ${OUTPUT}
        else
            $OCR_TOP/ocr/scripts/Profiler/analyzeProfile.py -t '*' > ${OUTPUT}
        fi
        cd - 1>/dev/null
    fi
    ALL_PROFILES+="${OUTPUT} "
done

echo ${ALL_PROFILES}
