#!/bin/bash


HERE=$PWD
cd ..
make OCR_TYPE=x86-pthread-x86 CC=icc install

cd $HERE

CoMD=../install/x86-pthread-x86/comd-mpi
source mpi-strongScaling.inc

