#!/bin/sh

# Simple strong scaling study with eam potential and 256,000 atoms
# $CoMD -ocr:cfg  $OCR_INSTALL/config/default.cfg -r  -t 10 -e -i 2 -j 2 -k 1 -x 40 -y 40 -z 40

LD_LIBRARY_PATH=$OCR_INSTALL/lib $CoMD -ocr:cfg  $OCR_INSTALL/config/mach-hc-1w.cfg    -e -i 1 -j 1 -k 1 -x 40 -y 40 -z 40
LD_LIBRARY_PATH=$OCR_INSTALL/lib $CoMD -ocr:cfg  $OCR_INSTALL/config/mach-hc-2w.cfg     -e -i 2 -j 1 -k 1 -x 40 -y 40 -z 40
LD_LIBRARY_PATH=$OCR_INSTALL/lib $CoMD -ocr:cfg  $OCR_INSTALL/config/mach-hc-4w.cfg    -e -i 2 -j 2 -k 1 -x 40 -y 40 -z 40
#mpirun -np 2  ../bin/CoMD-mpi -e -i 2 -j 1 -k 1 -x 40 -y 40 -z 40
#mpirun -np 4  ../bin/CoMD-mpi -e -i 2 -j 2 -k 1 -x 40 -y 40 -z 40
LD_LIBRARY_PATH=$OCR_INSTALL/lib $CoMD -ocr:cfg  $OCR_INSTALL/config/mach-hc-8w.cfg    -e -i 2 -j 2 -k 2 -x 40 -y 40 -z 40
LD_LIBRARY_PATH=$OCR_INSTALL/lib $CoMD -ocr:cfg  $OCR_INSTALL/config/mach-hc-16w.cfg   -e -i 4 -j 2 -k 2 -x 40 -y 40 -z 40
