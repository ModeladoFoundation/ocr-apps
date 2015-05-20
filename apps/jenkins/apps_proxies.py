#!/usr/bin/env python
# This relies on OCR being built in the shared home
# but does not need any seperate checkout as it runs
# completely out of the source directory

# CoMD: llnl ocr
job_ocr_build_kernel_comdllnl_x86_regression = {
    'name': 'ocr-build-kernel-comdllnl-x86-regression',
    'depends': ('ocr-build-x86-pthread-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'comdllnl x86-pthread-x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/CoMD/refactored/ocr/llnl',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/xstack/apps/CoMD/refactored/ocr/llnl/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/CoMD/refactored/ocr/llnl/install'}
}

job_ocr_run_kernel_comdllnl_x86_remote_regression = {
    'name': 'ocr-run-kernel-comdllnl-x86-remote-regression',
    'depends': ('ocr-build-kernel-comdllnl-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'run-args': 'comdllnl x86-pthread-x86 ocr-run-kernel-comdllnl-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/CoMD/refactored/ocr/llnl',
                  'WORKLOAD_ARGS': '12 ${JJOB_SHARED_HOME}/xstack/apps/CoMD/datasets/lammps-inputfiles/comd',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/CoMD/refactored/ocr/llnl/install'}
}

# CoMD: sdsc ocr
job_ocr_build_kernel_comdsdsc_x86_regression = {
    'name': 'ocr-build-kernel-comdsdsc-x86-regression',
    'depends': ('ocr-build-x86-pthread-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'comdsdsc x86-pthread-x86',
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
    'run-args': 'comdsdsc x86-pthread-x86 ocr-run-kernel-comdsdsc-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/CoMD/refactored/ocr/sdsc',
                  'WORKLOAD_ARGS': '-x 5 -y 5 -z 5 -d ${JJOB_SHARED_HOME}/xstack/apps/CoMD/datasets/pots',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/CoMD/refactored/ocr/sdsc/install'}
}

# hpgmg-lite
job_ocr_build_kernel_hpgmglite_x86_regression = {
    'name': 'ocr-build-kernel-hpgmglite-x86-regression',
    'depends': ('ocr-build-x86-pthread-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'hpgmg x86-pthread-x86',
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
    'run-args': 'hpgmg x86-pthread-x86 ocr-run-kernel-hpgmglite-x86-remote-regression 5',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/mpilite/intel',
                  'WORKLOAD_ARGS': '',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/mpilite/intel/install'}
}

## hpgmg: sdsc ocr
#job_ocr_build_kernel_hpgmgsdsc_x86_regression = {
#    'name': 'ocr-build-kernel-hpgmgsdsc-x86-regression',
#    'depends': ('ocr-build-x86-pthread-x86',),
#    'jobtype': 'ocr-build-kernel-regression',
#    'run-args': 'hpgmg x86-pthread-x86',
#    'sandbox': ('shared','inherit0'),
#    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
#                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
#                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/ocr/sdsc',
#                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/xstack/apps/hpgmg/refactored/ocr/sdsc/build',
#                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/ocr/sdsc/install'}
#}
#
#job_ocr_run_kernel_hpgmgsdsc_x86_remote_regression = {
#    'name': 'ocr-run-kernel-hpgmgsdsc-x86-remote-regression',
#    'depends': ('ocr-build-kernel-hpgmgsdsc-x86-regression',),
#    'jobtype': 'ocr-run-kernel-remote-regression',
#    'run-args': 'hpgmg x86-pthread-x86 ocr-run-kernel-hpgmgsdsc-x86-remote-regression 1',
#    'sandbox': ('shared','inherit0'),
#    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
#                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
#                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/ocr/sdsc',
#                  'WORKLOAD_ARGS': '4 8',
#                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/ocr/sdsc/install'}
#}

# tempest-lite
job_ocr_build_kernel_tempestsw2lite_x86_regression = {
    'name': 'ocr-build-kernel-tempestsw2lite-x86-regression',
    'depends': ('ocr-build-x86-pthread-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'SWTest2 x86-pthread-x86',
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
    'run-args': 'SWTest2 x86-pthread-x86 ocr-run-kernel-tempestsw2lite-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/tempest/refactored/mpilite/intel/test/shallowwater_sphere',
                  'WORKLOAD_ARGS': '-r 4 -t 1600',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/tempest/refactored/mpilite/intel/test/shallowwater_sphere/install'}
}

# XSBench: Intel ocr
job_ocr_build_kernel_xsbench_x86_regression = {
    'name': 'ocr-build-kernel-xsbench-x86-regression',
    'depends': ('ocr-build-x86-pthread-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'XSBench x86-pthread-x86',
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
    'run-args': 'XSBench x86-pthread-x86 ocr-run-kernel-xsbench-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/XSBench/refactored/ocr/intel',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/XSBench/refactored/ocr/intel/install',
                  'WORKLOAD_ARGS': '-s small -g 1000 -l 100000',
                  'OCR_CONFIG': '${JJOB_SHARED_HOME}/xstack/apps/XSBench/refactored/ocr/intel/default.cfg'}
}
