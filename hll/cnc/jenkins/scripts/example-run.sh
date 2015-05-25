#!/bin/bash

ROOT="${CNCOCR_ROOT-"${XSTACK_ROOT?Missing CNCOCR_ROOT or XSTACK_ROOT environment variable}/hll/cnc"}"

EXAMPLE=${1}
shift

echo ">>> CnC-OCR: Running example ${EXAMPLE} with args = $*"
cd "${ROOT}/examples/${EXAMPLE}" && make run WORKLOAD_ARGS="$*" &>test.out
