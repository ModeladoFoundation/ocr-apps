#!/bin/bash
####---------------------------------------------------------------------------
# Script to build modern and exascale hpgmg proxy apps.
#
#------------------------------------------------------------------------------

# check for basic settings
: ${OCR_HOME:?"Please set the environment variable OCR_HOME to the location of OCR"}

# Setup environment
. ./env.sh

# Remove old builds
./clobber.sh

# Setup compilers
export CC=gcc
export CXX=g++
RSTREAM_DIST=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/rstream
LDLIBS="-lrstream_ocr -Wl,-rpath=$RSTREAM_DIST -L$RSTREAM_DIST -locr -L$OCR_INSTALL/lib"

#=== "MODERN" =================================================================

# Configure with OpenMP
./configure --no-fe --no-fv-mpi --fv-timer=omp \
--fv-cycle=V --fv-smoother=gsrb --fv-coarse-solver=bicgstab \
--arch=modern-rocr \
--CC=$CC --CCLD=$CXX --CFLAGS="-O3 -g -fopenmp -I$RSTREAM_DIST" \
--CPPFLAGS="-DGSRB_R_STREAM -I$OCR_INSTALL/include -I$OCR_INSTALL/include/extensions -I$RSTREAM_DIST " \
--LDLIBS="$LDLIBS"

# Build
make -j 16 -C modern-rocr V=1

#=== EXASCALE =================================================================
CFLAGS="-DRSTREAM_CHEBY -O3 -g -fopenmp -std=c99 -I$OCR_INSTALL/include -I$OCR_INSTALL/include/extensions -I$RSTREAM_DIST"

# Configure with OpenMP
./configure --no-fe --no-fv-mpi --fv-timer=omp \
--fv-cycle=F --fv-smoother=cheby --fv-coarse-solver=bicgstab \
--arch=exascale-rocr \
--CC=$CC --CCLD=$CXX \
--CFLAGS="$CFLAGS" --LDLIBS="$LDLIBS"

# Build
make -j 16 -C exascale-rocr V=1

