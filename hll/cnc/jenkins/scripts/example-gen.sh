#!/bin/bash

ROOT="${CNCOCR_ROOT-"${XSTACK_ROOT?Missing CNCOCR_ROOT or XSTACK_ROOT environment variable}/hll/cnc"}"

echo ">>> CnC-OCR: Running translator for example ${1}"
cd "${ROOT}/examples/${1}" && ${ROOT}/bin/cncocr_t
