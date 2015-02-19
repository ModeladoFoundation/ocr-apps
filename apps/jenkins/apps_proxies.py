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
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/apps/CoMD/refactored/ocr/llnl',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/apps/CoMD/refactored/ocr/llnl/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/apps/CoMD/refactored/ocr/llnl/install'}
}

job_ocr_run_kernel_comdllnl_x86_remote_regression = {
    'name': 'ocr-run-kernel-comdllnl-x86-remote-regression',
    'depends': ('ocr-build-kernel-comdllnl-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'run-args': 'comdllnl x86-pthread-x86 ocr-run-kernel-comdllnl-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/apps/CoMD/refactored/ocr/llnl',
                  'WORKLOAD_ARGS': '12 ${JJOB_SHARED_HOME}/apps/CoMD/datasets/lammps-inputfiles/comd',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/apps/CoMD/refactored/ocr/llnl/install'}
}

# CoMD: sdsc ocr
job_ocr_build_kernel_comdsdsc_x86_regression = {
    'name': 'ocr-build-kernel-comdsdsc-x86-regression',
    'depends': ('ocr-build-x86-pthread-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'comdsdsc x86-pthread-x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/apps/CoMD/refactored/ocr/sdsc',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/apps/CoMD/refactored/ocr/sdsc/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/apps/CoMD/refactored/ocr/sdsc/install'}
}

job_ocr_run_kernel_comdsdsc_x86_remote_regression = {
    'name': 'ocr-run-kernel-comdsdsc-x86-remote-regression',
    'depends': ('ocr-build-kernel-comdsdsc-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'run-args': 'comdsdsc x86-pthread-x86 ocr-run-kernel-comdsdsc-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/apps',
                  'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/apps/CoMD/refactored/ocr/sdsc',
                  'WORKLOAD_ARGS': '-x 5 -y 5 -z 5 -d ${JJOB_SHARED_HOME}/apps/CoMD/datasets/pots',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/apps/CoMD/refactored/ocr/sdsc/install'}
}
