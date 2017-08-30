# Copyright (c) 2016        Bryan Pawlowski
# Copyright (c) 2000-2016   Paul Ullrich
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file License_1_0.txt or copy at http://boost.org/LICENSE_1_0.txt)



CXX=g++
F90=mpif90
MPICXX=g++
MPIF90=g++

CXXFLAGS+= -fPIC -O3
F90FLAGS+= -O3
LDFLAGS+= -O3 -lm

F90_RUNTIME= -lgfortran


# NETCDF
NETCDF_ROOT=        /opt/local
NETCDF_CXXFLAGS=    -I$(NETCDF_ROOT)/include
NETCDF_LIBRARIES=   -lnetcdf -lnetcdf_c++
NETCDF_LDFLAGS=     -L$(NETCDF_ROOT)/lib64


# PETSC

PETSC_ROOT=         /opt/local/lib/petsc
X11_ROOT=           /opt/X11
PETSC_CXXFLAGS=     -I$(PETSC_ROOT)/include
PETSC_LIBRARIES=    -lpetsc -lX11
PETSC_LDFLAGS=      -L$(PETSC_ROOT)/lib -L$(X11_ROOT)/lib

#LAPACK

LAPACK_INTERFACE=   FORTRAN
LAPACK_CXXFLAGS=
LAPACK_LIBRARIES= -llapack -lblas -lm
LAPACK_LDFLAGS=
