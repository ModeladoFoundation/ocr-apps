#!/bin/sh
set -x

if [ 1 == 1 ]; then

OCR_TYPE=x86 make uninstall
OCR_TYPE=x86 make clean
OCR_TYPE=x86 make all
OCR_TYPE=x86 make install

fi

if [ 1 == 1 ]; then

echo run with 1 XE in the block

OCR_INSTALL=$HOME/ocr/ocr/install/x86 \
LD_LIBRARY_PATH=$HOME/ocr/ocr/install/lib:$HOME/apps/apps/libs/x86/lib \
OCR_CONFIG=$HOME/apps/apps/uts-1.1/ocr/default1.cfg \
time $HOME/apps/apps/uts-1.1/ocr/install/x86/uts

fi

if [ 1 == 1 ]; then

echo run with 8 XEs in the block

OCR_INSTALL=$HOME/ocr/ocr/install/x86 \
LD_LIBRARY_PATH=$HOME/ocr/ocr/install/lib:$HOME/apps/apps/libs/x86/lib \
OCR_CONFIG=$HOME/apps/apps/uts-1.1/ocr/default8.cfg \
time $HOME/apps/apps/uts-1.1/ocr/install/x86/uts

fi
