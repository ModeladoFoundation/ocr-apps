#!/bin/bash

#
# This script can be used as a fallback to generate a scaling report out of run logs
# for batch submissions that could not be completed because of time allocations
# running out.
#
# Format:
# REPO_TOP ./scripts/batch/extractMissingReports.sh 'folder_with_runlogs' 'name_of_newly_generated_report' 'number_of_cluster_nodes' 'core_count_list'
#
# Example:
# REPO_TOP=/path/repo ./scripts/batch/extractMissingReports.sh job_output/rundir.ws_Stencil2D.2N/runlog-Stencil2D- newReport "1024" "2 5 17 24"

if [[ -z "${REPO_TOP}" ]]; then
    echo "error: REPO_TOP must be set"
    exit 1
fi

export SCRIPT_ROOT=${REPO_TOP}/ocr/ocr/tests/performance-tests/scripts
export OCR_INSTALL=${REPO_TOP}/ocr/ocr/install
REPORT_ONLY_RUNLOG_ARG="$1"
REPORT_ONLY_REPORT_ARG="$2"
NODE_SCALING="$3"
CORE_SCALING="$4"
. ${SCRIPT_ROOT}/runner.sh -reportOnly ${REPORT_ONLY_RUNLOG_ARG} ${REPORT_ONLY_REPORT_ARG}
