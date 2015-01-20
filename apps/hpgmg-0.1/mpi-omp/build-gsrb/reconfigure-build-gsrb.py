#!/usr/bin/python
import os,sys
from argparse import Namespace
sys.path.insert(0, os.path.abspath('.'))
import hpgmgconf
hpgmgconf.configure(Namespace(CC='mpiicc', CFLAGS='-g -UUSE -O3 -openmp -fp-model precise', CPPFLAGS='', LDFLAGS='', LDLIBS='', arch='build-gsrb', fe=False, fv=True, fv_coarse_solver='bicgstab', fv_cycle='V', fv_mpi=True, fv_smoother='gsrb', fv_subcomm=True, petsc_arch='', petsc_dir='', with_hpm=None))