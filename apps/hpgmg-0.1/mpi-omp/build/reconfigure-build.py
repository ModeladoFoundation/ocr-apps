#!/usr/bin/python
import os,sys
from argparse import Namespace
sys.path.insert(0, os.path.abspath('.'))
import hpgmgconf
hpgmgconf.configure(Namespace(CC='', CFLAGS='', CPPFLAGS='', LDFLAGS='', LDLIBS='', arch='build', fe=True, fv=True, fv_coarse_solver='bicgstab', fv_cycle='F', fv_mpi=True, fv_smoother='cheby', fv_subcomm=True, petsc_arch='', petsc_dir='', with_hpm=None))