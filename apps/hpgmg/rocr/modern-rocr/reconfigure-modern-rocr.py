#!/usr/bin/python
import os,sys
from argparse import Namespace
sys.path.insert(0, os.path.abspath('.'))
import hpgmgconf
hpgmgconf.configure(Namespace(CC='gcc', CCLD='g++', CFLAGS='-O0 -g -fopenmp', CPPFLAGS='-DGSRB_R_STREAM -I/home/thenretty/xstack/ocr/install/x86-pthread-x86/include -I/home/thenretty/xstack/ocr/install/x86-pthread-x86/include/extensions -I/opt/reservoir/latest-thenretty/rstream-3.3.3/runtime/codelet/ocr/inc ', LDFLAGS='', LDLIBS='-lrstream_ocr -L/opt/reservoir/latest-thenretty/rstream-3.3.3/runtime/codelet/ocr -locr -L/home/thenretty/xstack/ocr/install/x86-pthread-x86/lib', arch='modern-rocr', fe=False, fv=True, fv_coarse_solver='bicgstab', fv_cycle='V', fv_mpi=False, fv_smoother='gsrb', fv_subcomm=True, fv_timer='omp', petsc_arch='', petsc_dir='', with_hpm=None))