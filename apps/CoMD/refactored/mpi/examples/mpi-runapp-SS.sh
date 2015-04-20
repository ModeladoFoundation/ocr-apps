#!/bin/bash


HERE=$PWD
cd ../build_and_run/mpi
make OCR_TYPE=x86-pthread-x86 CoMD

cd $HERE

CoMD=../build_and_run/mpi/CoMD
source mpi-strongScaling.inc
