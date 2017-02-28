#!/bin/bash

if [ -z "${APPS_ROOT}" ]; then
    echo "error: APPS_ROOT must be set"
    exit 1
fi

. ${APPS_ROOT}/tools/execution_tools/batch/jobUtils.sh

# Go over the list of batch and submit
isDone $@
