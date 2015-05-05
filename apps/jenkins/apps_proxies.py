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

# hpgmg-lite
job_ocr_build_kernel_hpgmglite_x86_regression = {
    'name': 'ocr-build-kernel-hpgmglite_x86-regression',
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
    'name': 'ocr-run-kernel-hpgmglLite-x86-remote-regression',
    'depends': ('ocr-build-kernel-hpgmglite-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'run-args': 'hpgmg x86-pthread-x86 ocr-run-kernel-hpgmgLite-x86-remote-regression',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/mpilite/intel',
                  'WORKLOAD_ARGS': '',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/hpgmg/refactored/mpilite/intel/install'}
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
job_ocr_build_kernel_comdllnl_x86_regression = {
    'name': 'ocr-build-kernel-tempestsw2lite-x86-regression',
    'depends': ('ocr-build-x86-pthread-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'tempestsw2lite x86-pthread-x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/tempest/refactored/mpilite/intel/test/shallowwater_sphere',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/xstack/apps/CoMD/refactored/ocr/intel/test/shallowwater_sphere/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/CoMD/refactored/ocr/intel/test/shallowwater_sphere/install'}
}

job_ocr_run_kernel_tempestsw2lite = {
    'name': 'ocr-run-kernel-tempestsw2lite-x86-remote-regression',
    'depends': ('ocr-build-kernel-tempestite-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'run-args': 'SWTest2 x86-pthread-x86 ocr-run-kernel-comdllnl-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/CoMD/refactored/mpilite/test/shallowwater_sphere',
                  'WORKLOAD_ARGS': '-r 4 -t 1600',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/CoMD/mpilite/test/shallowwater_sphere/install'}
}
