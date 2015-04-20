#!/bin/bash


HERE=$PWD
cd ../build_and_run/mpilite
make OCR_TYPE=x86-pthread-x86 CoMD

cd $HERE

CoMD=../build_and_run/mpilite/CoMD
source mpi-strongScaling.inc
