#!/bin/bash

####---------------------------------------------------------------------------
# HPGMG / ROCR run function
#
# $1 - Version ('modern' or 'exascale')
# $2 - OCR config file
# $3 - log2 of box size
# $4 - target boxes / thread
run_ocr() {
    . ./env.sh
    export OCR_CONFIG=$2
    export OMP_NUM_THREADS=1
    ./${1}-rocr/bin/hpgmg-fv $3 $4
    unset OMP_NUM_THREADS
    unset OCR_CONFIG
}

usage() {
    echo "Usage: $0 [modern|exascale] [ocr cfg file] [log2 box size] [boxes per thread]"
    exit 1
}

if [ $# -ne 4 ] ; then
    usage
fi

if [ "$1" != "modern" -a "$1" != "exascale" ] ; then
    usage
fi

run_ocr $1 $2 $3 $4
