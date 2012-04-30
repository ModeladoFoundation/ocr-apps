#!/bin/sh
set -x
cmp ../../SSCP_afl/small/Parameters.txt Parameters.txt
cmp ../../SSCP_afl/small/inputs.c inputs.c
cmp ../../SSCP_afl/small/common.h common.h
cmp ../../SSCP_afl/small/rag_rmd.h rag_rmd.h
cmp ../../SSCP_afl/small/rag_rmd.c rag_rmd.c
cmp ../../SSCP_afl/small/back_proj.c back_proj.c
cmp ../../SSCP_afl/small/ccd.c ccd.c
cmp ../../SSCP_afl/small/cfar.c cfar.c
cmp ../../SSCP_afl/small/digspot.c digspot.c
cmp ../../SSCP_afl/small/image_formation.c image_formation.c
cmp ../../SSCP_afl/small/main.c main.c
cmp ../../SSCP_afl/small/registration.c registration.c
cmp ../../SSCP_afl/small/sinc.c sinc.c
cmp ../../SSCP_afl/small/sinc_interp.c sinc_interp.c
cmp ../../SSCP_afl/small/fftw3.c fftw3.c
cmp ../../SSCP_afl/small/fftw3.h fftw3.h
