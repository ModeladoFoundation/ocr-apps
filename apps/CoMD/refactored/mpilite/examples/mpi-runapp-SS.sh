#!/bin/bash


HERE=$PWD
cd ../build_and_run/mpilite
make OCR_TYPE=x86 CC=icc install

cd $HERE

CoMD=../build_and_run/mpilite/build/x86/comd-mpi

if [[ "$OCR_INSTALL"1 == 1 ]]; then
    OCR_INSTALL=$APPS_ROOT/../ocr/install/x86
fi

source mpi-strongScaling.inc
