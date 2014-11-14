#!/bin/sh
# README.txt -- Here are examples of how to build and run SAR.
set -x
date
OCR_TYPE=x86-pthread-x86 make clean all install run
date
OCR_NUM_NODES=4 OCR_TYPE=x86-pthread-mpi make clean all install run
date
exit
OCR_TYPE=x86-pthread-tg  make clean all install run
date
##RUN_TOOL=valgrind OCR_TYPE=x86-pthread-x86 make clean all install run
##date
##RUN_TOOL=valgrind OCR_TYPE=x86-pthread-tg  make clean all install run
##date
OCR_TYPE=tg             make clean all install run
date
