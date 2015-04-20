#!/bin/sh

# Simple strong scaling study with eam potential and 500 atoms to match ocr sdsc comd run
# ../build_and_run/mpi/CoMD -ocr:cfg  $OCR_INSTALL/config/default.cfg -r  -t 10 -e -i 2 -j 2 -k 1 -x 5 -y 5 -z 5
 ../build_and_run/mpi/CoMD -ocr:cfg  $OCR_INSTALL/config/mach-hc-1w.cfg -r 1  -t 10  -e -i 1 -j 1 -k 1 -x 5 -y 5 -z 5
 ../build_and_run/mpi/CoMD -ocr:cfg  $OCR_INSTALL/config/mach-hc-2w.cfg -r 2  -t 10   -e -i 2 -j 1 -k 1 -x 5 -y 5 -z 5
 ../build_and_run/mpi/CoMD -ocr:cfg  $OCR_INSTALL/config/mach-hc-4w.cfg -r 4  -t 10  -e -i 2 -j 2 -k 1 -x 5 -y 5 -z 5
#mpirun -np 2  ../bin/CoMD-mpi -e -i 2 -j 1 -k 1 -x 5 -y 5 -z 5
#mpirun -np 4  ../bin/CoMD-mpi -e -i 2 -j 2 -k 1 -x 5 -y 5 -z 5
 ../build_and_run/mpi/CoMD -ocr:cfg  $OCR_INSTALL/config/mach-hc-8w.cfg -r 8  -t 10  -e -i 2 -j 2 -k 2 -x 5 -y 5 -z 5
 ../build_and_run/mpi/CoMD -ocr:cfg  $OCR_INSTALL/config/mach-hc-16w.cfg -r 16  -t 10  -e -i 4 -j 2 -k 2 -x 5 -y 5 -z 5
