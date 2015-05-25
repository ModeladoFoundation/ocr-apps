#!/bin/bash
####---------------------------------------------------------------------------
# Script to build modern and exascale hpgmg proxy apps.
#
#------------------------------------------------------------------------------

# Setup environment
. ./env.sh

# Remove old builds
./clobber.sh

# Setup compilers
export CC=gcc
export CXX=g++
LDLIBS="-lrstream_ocr -L$ROCR_HOME -locr -L$OCR_INSTALL/lib"

#=== "MODERN" =================================================================

# Configure with OpenMP
./configure --no-fe --no-fv-mpi --fv-timer=omp \
--fv-cycle=V --fv-smoother=gsrb --fv-coarse-solver=bicgstab \
--arch=modern-rocr \
--CC=$CC --CCLD=$CXX --CFLAGS="-O0 -g -fopenmp" \
--CPPFLAGS="-DGSRB_R_STREAM -I$OCR_INSTALL/include -I$OCR_INSTALL/include/extensions -I$ROCR_HOME/inc " \
--LDLIBS="$LDLIBS"

# Build
make -j 16 -C modern-rocr V=1

#=== EXASCALE =================================================================
CFLAGS="-DRSTREAM_CHEBY -O0 -g -fopenmp -std=c99 -I$OCR_INSTALL/include -I$OCR_INSTALL/include/extensions -I$ROCR_HOME/inc"

# Configure with OpenMP
./configure --no-fe --no-fv-mpi --fv-timer=omp \
--fv-cycle=F --fv-smoother=cheby --fv-coarse-solver=bicgstab \
--arch=exascale-rocr \
--CC=$CC --CCLD=$CXX \
--CFLAGS="$CFLAGS" --LDLIBS="$LDLIBS"

# Build
make -j 16 -C exascale-rocr V=1

