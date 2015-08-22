#!/bin/bash

ROOT="${UCNC_ROOT-"${XSTACK_ROOT?Missing UCNC_ROOT or XSTACK_ROOT environment variable}/hll/cnc"}"
WORKLOAD_SRC=`${ROOT}/jenkins/scripts/get-app-path.sh ${1}`

echo ">>> CnC-OCR: Running translator for example ${1}"
cd "${WORKLOAD_SRC}" && ${ROOT}/bin/ucnc_t -p ${UCNC_PLATFORM:-ocr/x86}
