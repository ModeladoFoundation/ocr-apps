#!/bin/sh
set -x
cmp ../../SSCP_sim/src/common.h common.h
cmp ../../SSCP_sim/src/rag_rmd.h rag_rmd.h
cmp ../../SSCP_sim/src/rag_rmd.c rag_rmd.c
cmp ../../SSCP_sim/src/main.c main.c
cmp ../../SSCP_sim/src/inputs.c inputs.c
cmp ../../SSCP_sim/src/image_formation.c image_formation.c
cmp ../../SSCP_sim/src/digspot.c digspot.c
cmp ../../SSCP_sim/src/back_proj.c back_proj.c
cmp ../../SSCP_sim/src/registration.c registration.c
cmp ../../SSCP_sim/src/ccd.c ccd.c
cmp ../../SSCP_sim/src/cfar.c cfar.c
cmp ../../SSCP_sim/src/sinc.c sinc.c
cmp ../../SSCP_sim/src/sinc_interp.c sinc_interp.c
cmp ../../SSCP_sim/src/fftw3.c fftw3.c
cmp ../../SSCP_sim/src/fftw3.h fftw3.h
