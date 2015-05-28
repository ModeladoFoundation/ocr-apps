#!/bin/bash


HERE=$PWD
cd ..
make OCR_TYPE=x86 CC=icc install

cd $HERE

CoMD=../install/x86/comd-mpi

if [[ "$OCR_INSTALL"1 == 1 ]]; then
    OCR_INSTALL=$APPS_ROOT/../ocr/install/x86
fi

source mpi-strongScaling.inc


