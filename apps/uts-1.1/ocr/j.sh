#!/bin/sh
set -x

if [ 1 == 1 ]; then

OCR_TYPE=x86 make uninstall
OCR_TYPE=x86 make clean
OCR_TYPE=x86 make all
OCR_TYPE=x86 make install

fi

if [ 1 == 1 ]; then

rm -f j1.txt
echo run with 1 XE in the block
export XSTG_ROOT=$(dirname $(dirname $(dirname $(dirname $PWD))))
OCR_INSTALL=$XSTG_ROOT/ocr/ocr/install \
LD_LIBRARY_PATH=$XSTG_ROOT/ocr/ocr/install/lib:$XSTG_ROOT/apps/apps/libs/install/x86/lib \
OCR_CONFIG=$XSTG_ROOT/apps/apps/uts-1.1/ocr/default1.cfg \
time $XSTG_ROOT/apps/apps/uts-1.1/ocr/install/x86/uts >& j1.txt

#$HOME/sde*/sde  -top_blocks 1000 -omix worker1.txt -- \
#
fi

if [ 1 == 1 ]; then

rm -f j8.txt
echo run with 8 XEs in the block

OCR_INSTALL=$XSTG_ROOT/ocr/ocr/install \
LD_LIBRARY_PATH=$XSTG_ROOT/ocr/ocr/install/lib:$XSTG_ROOT/apps/apps/libs/install/x86/lib \
OCR_CONFIG=$XSTG_ROOT/apps/apps/uts-1.1/ocr/default8.cfg \
time $XSTG_ROOT/apps/apps/uts-1.1/ocr/install/x86/uts >& j8.txt

#$HOME/sde*/sde  -top_blocks 1000 -omix worker8.txt -- \
#
fi
diff -y -W 141 j1.txt j8.txt
