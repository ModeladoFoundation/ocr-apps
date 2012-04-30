#!/bin/sh
set -x
cmp ../../SSCP_afl/src/common.h common.h
cmp ../../SSCP_afl/src/rag_rmd.h rag_rmd.h
cmp ../../SSCP_afl/src/rag_rmd.c rag_rmd.c
cmp ../../SSCP_afl/src/back_proj.c back_proj.c
cmp ../../SSCP_afl/src/digspot.c digspot.c
cmp ../../SSCP_afl/src/image_formation.c image_formation.c
cmp ../../SSCP_afl/src/main.c main.c
cmp ../../SSCP_afl/src/registration.c registration.c
cmp ../../SSCP_afl/src/sinc.c sinc.c
cmp ../../SSCP_afl/src/sinc_interp.c sinc_interp.c
cmp ../../SSCP_afl/src/fftw3.c fftw3.c
cmp ../../SSCP_afl/src/fftw3.h fftw3.h
