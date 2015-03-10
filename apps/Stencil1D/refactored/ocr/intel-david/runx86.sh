#!/bin/sh
#application requires no input arguments.
export OCR_ROOT=$HOME/xstack
export OCR_INSTALL=$HOME/xstack/ocr/install/x86-pthread-x86
export OCR_TYPE=x86-pthread-x86
export OCR_CONFIG=$OCR_INSTALL/config/default.cfg
export OCR_TYPE=x86-pthread-x86
#make -f Makefile.x86-pthread-x86 clean
#make -f Makefile.x86-pthread-x86 run
make clean
make run
