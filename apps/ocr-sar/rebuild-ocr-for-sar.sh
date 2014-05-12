#!/bin/sh
set -x
cd ~/xstack/ocr/runtime/ocr-x86
./clobber.sh
OCR_CONF_OPTS="--enable-deque=8192" ./install.sh
