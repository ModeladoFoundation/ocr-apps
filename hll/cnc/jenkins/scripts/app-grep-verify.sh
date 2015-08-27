#!/bin/bash

ROOT="${UCNC_ROOT-"${XSTACK_ROOT?Missing UCNC_ROOT or XSTACK_ROOT environment variable}/hll/cnc"}"
WORKLOAD_SRC=`${ROOT}/jenkins/scripts/get-app-path.sh ${1}`
PATTERN="${2}"

echo ">>> CnC-OCR: Verifying example ${1}"
cd "${WORKLOAD_SRC}" && \
if ! grep -q '\b'"${PATTERN}"'\b' < "${WORKLOAD_SRC}/test.out"; then
    echo "FAILED: expected ${PATTERN}"
    cat "${WORKLOAD_SRC}/test.out"
    exit 1
fi
