#!/bin/bash

if [ $# -lt 2 ]; then
    echo "Usage $0 TARGET BENCHMARK ARGS"
    exit
fi

if [ ! -d "$2/ocr" ]; then
    echo "No OCR directory for $2"
    exit
fi

if [ -z "${APPS_ROOT}" ]; then
    APPS_ROOT=`cd . && pwd`
fi

if [ -z "${APPS_LIBS_ROOT}" ]; then
    APPS_LIBS_ROOT=`cd ./libs && pwd`
fi

if [ -z "${OCR_SRC}" ]; then
    OCR_SRC=`cd ./../ocr/src && pwd`
fi

if [ -z "${OCR_INSTALL_ROOT}" ]; then
    OCR_INSTALL_ROOT=`cd ./../ocr/install && pwd`
fi

if [ -z "${OCR_BUILD_ROOT}" ]; then
    OCR_BUILD_ROOT=`cd ./../ocr/build && pwd`
fi

cd $2/ocr
TARGET=$1

shift 2
if [ "$TARGET" == "whole-fsim" ]; then
    APPS_ROOT=${APPS_ROOT} APPS_LIBS_ROOT=${APPS_LIBS_ROOT}/tg \
    OCR_INSTALL_ROOT=${OCR_INSTALL_ROOT} OCR_SRC=${OCR_SRC} OCR_BUILD_ROOT=${OCR_BUILD_ROOT} make -f Makefile.whole-fsim $@
elif [ "$TARGET" == "x86-pthread-x86" ]; then
    APPS_ROOT=${APPS_ROOT} APPS_LIBS_ROOT=${APPS_LIBS_ROOT}/x86 \
    OCR_INSTALL_ROOT=${OCR_INSTALL_ROOT} OCR_SRC=${OCR_SRC} OCR_BUILD_ROOT=${OCR_BUILD_ROOT} make -f Makefile.x86-pthread-x86 $@
elif [ "$TARGET" == "x86-pthread-fsim" ]; then
    APPS_ROOT=${APPS_ROOT} APPS_LIBS_ROOT=${APPS_LIBS_ROOT}\
    OCR_INSTALL_ROOT=${OCR_INSTALL_ROOT} OCR_SRC=${OCR_SRC} OCR_BUILD_ROOT=${OCR_BUILD_ROOT} make -f Makefile.x86-pthread-fsim $@
else
    echo "Unknown target: $TARGET; allowed are 'whole-fsim', 'x86-pthread-x86' and 'x86-pthread-fsim'"
fi

