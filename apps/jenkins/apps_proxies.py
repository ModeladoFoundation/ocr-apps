#!/usr/bin/env python
# This relies on OCR being built in the shared home
# but does not need any seperate checkout as it runs
# completely out of the source directory

# CoMD: sdsc ocr
job_ocr_build_kernel_comdsdsc_x86_regression = {
    'name': 'ocr-build-kernel-comdsdsc-x86-regression',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'comdsdsc x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/CoMD/refactored/ocr/sdsc',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/xstack/apps/CoMD/refactored/ocr/sdsc/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/CoMD/refactored/ocr/sdsc/install'}
}

job_ocr_run_kernel_comdsdsc_x86_remote_regression = {
    'name': 'ocr-run-kernel-comdsdsc-x86-remote-regression',
    'depends': ('ocr-build-kernel-comdsdsc-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'run-args': 'comdsdsc x86 ocr-run-kernel-comdsdsc-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/CoMD/refactored/ocr/sdsc',
                  'WORKLOAD_ARGS': '-x 5 -y 5 -z 5 -d ${JJOB_SHARED_HOME}/xstack/apps/CoMD/datasets/pots',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/CoMD/refactored/ocr/sdsc/install'}
}

job_ocr_run_kernel_comdsdsc_x86_remote_scaling = {
    'name': 'ocr-run-kernel-comdsdsc-x86-remote-scaling',
    'depends': ('ocr-build-kernel-comdsdsc-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-scaling',
    'run-args': 'comdsdsc x86 ocr-run-kernel-comdsdsc-x86-remote-scaling 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/CoMD/refactored/ocr/sdsc',
                  'WORKLOAD_ARGS': '-x 5 -y 5 -z 5 -d ${JJOB_SHARED_HOME}/xstack/apps/CoMD/datasets/pots',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/CoMD/refactored/ocr/sdsc/install'}
}

# Disabled until feature #806 is added
# CoMD: MPI-Lite Intel
#job_ocr_build_kernel_comdlite_x86_regression = {
#    'name': 'ocr-build-kernel-comdlite-x86-regression',
#    'depends': ('ocr-build-x86',),
#    'jobtype': 'ocr-build-kernel-regression',
#    'run-args': 'comdlite x86',
#    'sandbox': ('shared','inherit0'),
#    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
#                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
#                  'APPS_MAKEFILE': '${JJOB_SHARED_HOME}/xstack/apps/CoMD/refactored/mpilite/Makefile',
#                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/CoMD/refactored/mpilite/src-mpi',
#                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/xstack/apps/CoMD/refactored/mpilite/build',
#                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/CoMD/refactored/mpilite/install'}
#}
#
# the run takes 27 seconds
#job_ocr_run_kernel_comdlite_x86_remote_regression = {
#    'name': 'ocr-run-kernel-comdlite-x86-remote-regression',
#    'depends': ('ocr-build-kernel-comdlite-x86-regression',),
#    'jobtype': 'ocr-run-kernel-remote-regression',
#    'run-args': 'comdlite x86 ocr-run-kernel-comdlite-x86-remote-regression 10',
#    'sandbox': ('shared','inherit0'),
#    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
#                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
#                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/CoMD/refactored/mpilite/src-mpi',
#                  'WORKLOAD_ARGS': '-r 4  -t 10  -e -i 2 -j 2 -k 1 -x 40 -y 40 -z 40 -d ${JJOB_SHARED_HOME}/xstack/apps/CoMD/datasets/pots',
#                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/CoMD/refactored/mpilite/install'}
#}

# hpgmg-lite
job_ocr_build_kernel_hpgmglite_x86_regression = {
    'name': 'ocr-build-kernel-hpgmglite-x86-regression',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'hpgmg x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/mpilite/intel',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/xstack/apps/hpgmg/refactored/mpilite/intel/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/mpilite/intel/install'}
}


job_ocr_run_kernel_hpgmglite_x86_remote_regression = {
   'name': 'ocr-run-kernel-hpgmglite-x86-remote-regression',
   'depends': ('ocr-build-kernel-hpgmglite-x86-regression',),
   'jobtype': 'ocr-run-kernel-remote-regression',
   'run-args': 'hpgmg x86 ocr-run-kernel-hpgmglite-x86-remote-regression 5',
   'sandbox': ('shared','inherit0'),
   'env-vars': { 'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                 'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/mpilite/intel',
                 'WORKLOAD_ARGS': ' -r 4 -t 1600 4 8',
                 'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/mpilite/intel/install'}
}

# hpgmg: sdsc ocr
job_ocr_build_kernel_hpgmgsdsc_x86_regression = {
    'name': 'ocr-build-kernel-hpgmgsdsc-x86-regression',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'hpgmg x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/ocr/sdsc',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/xstack/apps/hpgmg/refactored/ocr/sdsc/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/ocr/sdsc/install'}
}

#job_ocr_run_kernel_hpgmglite_x86_remote_scaling = {
#    'name': 'ocr-run-kernel-hpgmglite-x86-remote-scaling',
#    'depends': ('ocr-build-kernel-hpgmglite-x86-regression',),
#    'jobtype': 'ocr-run-kernel-remote-scaling',
#    'run-args': 'hpgmg x86 ocr-run-kernel-hpgmglite-x86-remote-scaling 5',
#    'sandbox': ('shared','inherit0'),
#    'env-vars': { 'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
#                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/mpilite/intel',
#                  'WORKLOAD_ARGS': '',
#                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/mpilite/intel/install'}
#}

job_ocr_run_kernel_hpgmgsdsc_x86_remote_regression = {
    'name': 'ocr-run-kernel-hpgmgsdsc-x86-remote-regression',
    'depends': ('ocr-build-kernel-hpgmgsdsc-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'run-args': 'hpgmg x86 ocr-run-kernel-hpgmgsdsc-x86-remote-regression 3',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/ocr/sdsc',
                  'WORKLOAD_ARGS': '4 8',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/ocr/sdsc/install'}
}

# tempest-lite
job_ocr_build_kernel_tempestsw2lite_x86_regression = {
    'name': 'ocr-build-kernel-tempestsw2lite-x86-regression',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'SWTest2 x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/tempest/refactored/mpilite/intel/test/shallowwater_sphere',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/xstack/apps/tempest/refactored/mpilite/intel/test/shallowwater_sphere/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/tempest/refactored/mpilite/intel/test/shallowwater_sphere/install'}
}

job_ocr_run_kernel_tempestsw2lite = {
    'name': 'ocr-run-kernel-tempestsw2lite-x86-remote-regression',
    'depends': ('ocr-build-kernel-tempestsw2lite-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'run-args': 'SWTest2 x86 ocr-run-kernel-tempestsw2lite-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/tempest/refactored/mpilite/intel/test/shallowwater_sphere',
                  'WORKLOAD_ARGS': '-r 4 -t 1600',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/tempest/refactored/mpilite/intel/test/shallowwater_sphere/install'}
}

#job_ocr_run_kernel_tempestsw2lite_scaling = {
#    'name': 'ocr-run-kernel-tempestsw2lite-x86-remote-scaling',
#    'depends': ('ocr-build-kernel-tempestsw2lite-x86-regression',),
#    'jobtype': 'ocr-run-kernel-remote-scaling',
#    'run-args': 'SWTest2 x86 ocr-run-kernel-tempestsw2lite-x86-remote-scaling 10',
#    'sandbox': ('shared','inherit0'),
#    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
#                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
#                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/tempest/refactored/mpilite/intel/test/shallowwater_sphere',
#                  'WORKLOAD_ARGS': '-r 4 -t 1600',
#                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/tempest/refactored/mpilite/intel/test/shallowwater_sphere/install'}
#}

# XSBench: Intel ocr
job_ocr_build_kernel_xsbench_x86_regression = {
    'name': 'ocr-build-kernel-xsbench-x86-regression',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'XSBench x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/XSBench/refactored/ocr/intel',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/xstack/apps/XSBench/refactored/ocr/intel/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/XSBench/refactored/ocr/intel/install'}
}

job_ocr_run_kernel_xsbench_x86_remote_regression = {
    'name': 'ocr-run-kernel-xsbench-x86-remote-regression',
    'depends': ('ocr-build-kernel-xsbench-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'run-args': 'XSBench x86 ocr-run-kernel-xsbench-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/XSBench/refactored/ocr/intel',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/XSBench/refactored/ocr/intel/install',
                  'WORKLOAD_ARGS': '-s small -g 1000 -l 100000'}
}

# job_ocr_run_kernel_xsbench_x86_remote_scaling = {
#    'name': 'ocr-run-kernel-xsbench-x86-remote-scaling',
#    'depends': ('ocr-build-kernel-xsbench-x86-regression',),
#    'jobtype': 'ocr-run-kernel-remote-scaling',
#    'run-args': 'XSBench x86 ocr-run-kernel-xsbench-x86-remote-scaling 10',
#    'sandbox': ('shared','inherit0'),
#    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
#                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
#                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/XSBench/refactored/ocr/intel',
#                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/XSBench/refactored/ocr/intel/install',
#                  'WORKLOAD_ARGS': '-s small -g 1000 -l 100000'}
# }

# XSBench: Intel MPI-Lite
job_ocr_build_kernel_xsbenchlite_x86_regression = {
        'name': 'ocr-build-kernel-xsbenchlite-x86-regression',
        'depends': ('ocr-build-x86',),
        'jobtype': 'ocr-build-kernel-regression',
        'run-args': 'XSBenchlite x86',
        'sandbox': ('shared','inherit0'),
        'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
        'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
        'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/XSBench/refactored/mpilite/intel/src',
        'APPS_MAKEFILE': '${JJOB_SHARED_HOME}/xstack/apps/XSBench/refactored/mpilite/intel/Makefile',
        'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/xstack/apps/XSBench/refactored/mpilite/intel/build',
        'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/XSBench/refactored/mpilite/intel/install'}
}

job_ocr_run_kernel_xsbenchlite_x86_remote_regression = {
        'name': 'ocr-run-kernel-xsbenchlite-x86-remote-regression',
        'depends': ('ocr-build-kernel-xsbenchlite-x86-regression',),
        'jobtype': 'ocr-run-kernel-remote-regression',
        'run-args': 'XSBenchlite x86 ocr-run-kernel-xsbenchlite-x86-remote-regression 10',
        'sandbox': ('shared','inherit0'),
        'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
        'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
        'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/XSBench/refactored/mpilite/intel/src',
        'APPS_MAKEFILE': '${JJOB_SHARED_HOME}/xstack/apps/XSBench/refactored/mpilite/intel/Makefile',
        'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/XSBench/refactored/mpilite/intel/install',
        'WORKLOAD_ARGS': '-r 4 -s small -g 1000 -l 100000 -t 1'}
}

# LULESH 2.0: PNNL CnC-OCR
job_cnc_ocr_gen_lulesh2pnnl_x86_regression = {
    'name': 'cnc-ocr-gen-lulesh2pnnl-x86',
    'depends': ('cnc-ocr-bootstrap-x86',),
    'jobtype': 'cnc-ocr-app-gen',
    'run-args': '${WORKLOAD_SRC} regression',
    'sandbox': ('shared', 'inherit0'),
    'env-vars': { 'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/lulesh-2.0.3/refactored/cnc-ocr/pnnl/per-element', }
}

job_cnc_ocr_build_kernel_lulesh2pnnl_x86_regression = {
    'name': 'cnc-ocr-build-kernel-lulesh2pnnl-x86-regression',
    'depends': ('cnc-ocr-gen-lulesh2pnnl-x86',),
    'jobtype': 'cnc-ocr-app-build',
    'run-args': '${WORKLOAD_SRC} regression',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/lulesh-2.0.3/refactored/cnc-ocr/pnnl/per-element',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/lulesh-2.0.3/refactored/cnc-ocr/pnnl/per-element/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/lulesh-2.0.3/refactored/cnc-ocr/pnnl/per-element/install' }
}

job_ocr_run_kernel_lulesh2pnnl_x86_remote_regression = {
    'name': 'ocr-run-kernel-lulesh2pnnl-x86-remote-regression',
    'depends': ('cnc-ocr-build-kernel-lulesh2pnnl-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'keywords': ('cnc-ocr',),
    'run-args': 'lulesh-2.0.3 x86 ocr-run-kernel-lulesh2pnnl-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'XSTACK_ROOT': '${JJOB_SHARED_HOME}/xstack',
                  'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/lulesh-2.0.3/refactored/cnc-ocr/pnnl/per-element',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/lulesh-2.0.3/refactored/cnc-ocr/pnnl/per-element/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/lulesh-2.0.3/refactored/cnc-ocr/pnnl/per-element/install' }
}

# HPGMG: PNNL CnC-OCR
job_cnc_ocr_gen_hpgmg_pnnl_x86_regression = {
    'name': 'cnc-ocr-gen-hpgmg-pnnl-x86',
    'depends': ('cnc-ocr-bootstrap-x86',),
    'jobtype': 'cnc-ocr-app-gen',
    'run-args': '${WORKLOAD_SRC} regression',
    'sandbox': ('shared', 'inherit0'),
    'env-vars': { 'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/cnc/pnnl', }
}

job_cnc_ocr_build_kernel_hpgmg_pnnl_x86_regression = {
    'name': 'cnc-ocr-build-kernel-hpgmg-pnnl-x86-regression',
    'depends': ('cnc-ocr-gen-hpgmg-pnnl-x86',),
    'jobtype': 'cnc-ocr-app-build',
    'run-args': '${WORKLOAD_SRC} regression',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/cnc/pnnl',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/cnc/pnnl/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/cnc/pnnl/install' }
}

job_ocr_run_kernel_hpgmg_pnnl_x86_remote_regression = {
    'name': 'ocr-run-kernel-hpgmg-pnnl-x86-remote-regression',
    'depends': ('cnc-ocr-build-kernel-hpgmg-pnnl-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'keywords': ('cnc-ocr',),
    'run-args': 'hpgmg x86 ocr-run-kernel-hpgmg-pnnl-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'XSTACK_ROOT': '${JJOB_SHARED_HOME}/xstack',
                  'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/cnc/pnnl',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/cnc/pnnl/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/cnc/pnnl/install' }
}

job_ocr_build_kernel_SNAPlite_x86_regression = {
    'name': 'ocr-build-kernel-SNAPlite-x86-regression',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'snap_lite x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/SNAP/refactored/mpilite/intel/',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/xstack/apps/SNAP/refactored/mpilite/intel/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/SNAP/refactored/mpilite/intel/install'}
}

job_ocr_run_kernel_SNAPlite = {
    'name': 'ocr-run-kernel-SNAPlite-x86-remote-regression',
    'depends': ('ocr-build-kernel-SNAPlite-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'run-args': 'snap_lite x86 ocr-run-kernel-SNAPlite-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/SNAP/refactored/mpilite/intel/',
                  'WORKLOAD_ARGS': '-r 4 -t 1600  --fi ${APPS_ROOT}/SNAP/refactored/mpilite/intel/fin04 --fo ${APPS_ROOT}/SNAP/refactored/mpilite/intel/fout04',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/SNAP/refactored/mpilite/intel/install'}
}

#job_ocr_run_kernel_SNAPlite_scaling = {
#    'name': 'ocr-run-kernel-SNAPlite-x86-remote-scaling',
#    'depends': ('ocr-build-kernel-SNAPlite-x86-regression',),
#    'jobtype': 'ocr-run-kernel-remote-scaling',
#    'run-args': 'snap_lite x86 ocr-run-kernel-SNAPlite-x86-remote-scaling 10',
#    'sandbox': ('shared','inherit0'),
#    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
#                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
#                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/SNAP/refactored/mpilite/intel/',
#                  'WORKLOAD_ARGS': '-r 4 -t 1600',
#                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/SNAP/refactored/mpilite/intel/install'}
#}
