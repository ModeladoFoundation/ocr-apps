#!/bin/bash


HERE=$PWD
cd ../build_and_run/mpilite
make OCR_TYPE=x86-pthread-x86 CC=icc install

cd $HERE

CoMD=../build_and_run/mpilite/build/x86-pthread-x86/comd-mpi
source mpi-strongScaling.inc
