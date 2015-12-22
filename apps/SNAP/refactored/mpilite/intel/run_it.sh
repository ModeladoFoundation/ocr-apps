#!/bin/bash

cd install/x86
COUNTER=200
until [  $COUNTER -eq 0 ]; do
    LD_LIBRARY_PATH=/home/glyons/xstg-am6/xstack/ocr/install/x86/lib:/home/glyons/xstg-am6/xstack/apps/libs/x86 OCR_CONFIG=/home/glyons/xstg-am6/xstack/apps/SNAP/refactored/mpilite/intel/install/x86/generated.cfg bash -c 'cd /home/glyons/xstg-am6/xstack/apps/SNAP/refactored/mpilite/intel/install/x86 &&  ./snap_lite -r 4 -t 1600 --fi /home/glyons/xstg-am6/xstack/apps/SNAP/refactored/mpilite/intel/fin04 --fo /home/glyons/xstg-am6/xstack/apps/SNAP/refactored/mpilite/intel/fout04 '
    let COUNTER=COUNTER-1
done
echo "Test is complete."

