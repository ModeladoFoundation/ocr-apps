#!/bin/bash

ROOT="${UCNC_ROOT-"${XSTACK_ROOT?Missing UCNC_ROOT or XSTACK_ROOT environment variable}/hll/cnc"}"
WORKLOAD_SRC=`${ROOT}/jenkins/scripts/get-app-path.sh ${1}`

EXAMPLE=${1}
shift

echo ">>> CnC-OCR: Running example ${EXAMPLE} with args = $*"
cd "${WORKLOAD_SRC}" && make run WORKLOAD_ARGS="$*" &>"${WORKLOAD_SRC}/test.out"
