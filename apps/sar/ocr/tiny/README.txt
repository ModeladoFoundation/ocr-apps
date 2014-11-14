#!/bin/sh
# README.txt -- Here are examples of how to build and run SAR.
set -x
cat j.sh
env 
gcc --version
mpicc -v
date
OCR_TYPE=x86-pthread-x86 make clean all run
date
OCR_NUM_NODES=4 OCR_TYPE=x86-pthread-mpi make clean all run
date
exit
OCR_TYPE=x86-pthread-tg  make clean all run
date
OCR_TYPE=tg              make clean all
date
OCR_TYPE=tg              make           run
date
#grep CONSOLE:\ XE\[0\-7\] install/tg/logs/*.brd*.chp*.unt*.blk*.CE.00
#find build   -print
#find install -print
