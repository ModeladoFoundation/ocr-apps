#!/bin/bash

ROOT="${CNCOCR_ROOT-"${XSTACK_ROOT?Missing CNCOCR_ROOT or XSTACK_ROOT environment variable}/hll/cnc"}"
WORKLOAD_SRC=`${ROOT}/jenkins/scripts/get-app-path.sh ${1}`

echo ">>> CnC-OCR: Building example ${1}"

# Need to run a different makefile for regressions
cd "${WORKLOAD_SRC}" && \
if [ "$2" = "regression" ]; then
    echo ">>> (regression build)"
    make -f Makefile.${OCR_TYPE-x86-pthread-x86} build install
else
    make
fi
