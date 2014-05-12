#!/bin/sh
set -x
echo run in linux/tiny linux/small linux/medium linux/large
echo run in fsim/tiny fsim/small fsim/medium and fsim/large
echo you are in `pwd`
rm -i back_proj.c ccd.c cfar.c common.h digspot.c fftw3.c fftw3.h image_formation.c inputs.c main.c rag_ocr.c rag_ocr.h registration.c sinc.c sinc_interp.c
ln -s ../../src/*.c
ln -s ../../src/*.h
