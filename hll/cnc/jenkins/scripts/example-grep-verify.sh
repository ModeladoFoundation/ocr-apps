#!/bin/bash

ROOT="${CNCOCR_ROOT-"${XSTACK_ROOT?Missing CNCOCR_ROOT or XSTACK_ROOT environment variable}/hll/cnc"}"

EXAMPLE="${1}"
PATTERN="${2}"

echo ">>> CnC-OCR: Verifying example ${EXAMPLE}"
cd "${ROOT}/examples/${EXAMPLE}" && \
if ! grep -q '\b'"${PATTERN}"'\b' < test.out; then
    echo "FAILED: expected ${PATTERN}"
    cat test.out
    exit 1
fi
