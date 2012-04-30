#!/bin/sh
set -x
cmp ../../SSCP_sim/small/Parameters.txt Parameters.txt
cmp ../../SSCP_sim/small/inputs.c inputs.c
cmp ../../SSCP_sim/small/common.h common.h
cmp ../../SSCP_sim/small/rag_rmd.h rag_rmd.h
cmp ../../SSCP_sim/small/rag_rmd.c rag_rmd.c
cmp ../../SSCP_sim/small/back_proj.c back_proj.c
cmp ../../SSCP_sim/small/ccd.c ccd.c
cmp ../../SSCP_sim/small/cfar.c cfar.c
cmp ../../SSCP_sim/small/digspot.c digspot.c
cmp ../../SSCP_sim/small/image_formation.c image_formation.c
cmp ../../SSCP_sim/small/main.c main.c
cmp ../../SSCP_sim/small/registration.c registration.c
cmp ../../SSCP_sim/small/sinc.c sinc.c
cmp ../../SSCP_sim/small/sinc_interp.c sinc_interp.c
cmp ../../SSCP_sim/small/fftw3.c fftw3.c
cmp ../../SSCP_sim/small/fftw3.h fftw3.h
