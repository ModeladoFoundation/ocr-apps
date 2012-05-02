#!/bin/sh
set -x
cmp ../../SSCP_afl/large/common.h common.h
cmp ../../SSCP_afl/large/rag_rmd.h rag_rmd.h
cmp ../../SSCP_afl/large/rag_rmd.c rag_rmd.c
cmp ../../SSCP_afl/large/main.c main.c
cmp ../../SSCP_afl/large/inputs.c inputs.c
cmp ../../SSCP_afl/large/image_formation.c image_formation.c
cmp ../../SSCP_afl/large/digspot.c digspot.c
cmp ../../SSCP_afl/large/back_proj.c back_proj.c
cmp ../../SSCP_afl/large/registration.c registration.c
cmp ../../SSCP_afl/large/ccd.c ccd.c
cmp ../../SSCP_afl/large/cfar.c cfar.c
cmp ../../SSCP_afl/large/sinc.c sinc.c
cmp ../../SSCP_afl/large/sinc_interp.c sinc_interp.c
cmp ../../SSCP_afl/large/fftw3.c fftw3.c
cmp ../../SSCP_afl/large/fftw3.h fftw3.h
cmp ../../SSCP_afl/large/Parameters.txt Parameters.txt
cmp ../../SSCP_afl/large/Parameters.h Parameters.h
cmp ../../SSCP_afl/large/block_sizes.h block_sizes.h
