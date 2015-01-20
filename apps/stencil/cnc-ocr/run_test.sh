#!/bin/bash
set -x

export OCR_INSTALL=$HOME/xstack_intel/ocr/install/x86-pthread-x86
export OCR_TYPE=x86-pthread-x86
. $HOME/xstack_intel/hll/cnc/setup_env.sh
export ENABLE_VISUALIZER=yes
[ $# -ne 2 ] && { echo "Usage: $0 NX NITER "; exit 1; }
echo "Testing NX = $1, NITER = $2"
export OCR_CONFIG=$OCR_INSTALL/config/default.cfg
ls -ltr $OCR_CONFIG
./Stencil1D $1 $2 2>&1 | tee log.$1
#./Stencil1D $1 $2 > $HOME/xstack/ocr/scripts/Visualizer/stencil_log.txt 2>&1
