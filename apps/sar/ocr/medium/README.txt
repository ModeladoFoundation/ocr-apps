#!/bin/sh
# README.txt -- Here are examples of how to build and run SAR.
set -x
date
OCR_CONFIG=`pwd`/medium-x86-pthread-x86.cfg OCR_TYPE=x86-pthread-x86 make clean all run
date
OCR_NUM_NODES=2 OCR_CONFIG=`pwd`/hc-dist-mpi-8w.cfg OCR_TYPE=x86-pthread-mpi make clean all run
date
OCR_CONFIG=`pwd`/medium-x86-pthread-tg.cfg  OCR_TYPE=x86-pthread-tg  make clean all run
date
OCR_TYPE=x86-pthread-tg  make clean all run
date

