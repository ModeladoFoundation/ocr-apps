#!/usr/bin/env python
# This relies on OCR being built in the shared home
# but does not need any seperate checkout as it runs
# completely out of the source directory

jobtype_gatherStats_regression = {
    'name': 'gatherStats-regression',
    'isLocal': True,
    'run-cmd': '${JJOB_SHARED_HOME}/xstack/apps/jenkins/scripts/statCollector.sh',
    'param-cmd': '${JJOB_SHARED_HOME}/xstack/jenkins/scripts/empty-cmd.sh',
    'keywords': ('regression',),
    'timeout': 20,
    'sandbox': ('shared', 'shareOK'),
    'req-repos': ('xstack',)
}

jobtype_ocr_build_kernel_regression = {
    'name': 'ocr-build-kernel-regression',
    'isLocal': True,
    'run-cmd': '${JJOB_SHARED_HOME}/xstack/ocr/jenkins/scripts/kernel-build.sh',
    'param-cmd': '${JJOB_SHARED_HOME}/xstack/jenkins/scripts/empty-cmd.sh',
    'keywords': ('regression',),
    'timeout': 300,
    'sandbox': ('local', 'shared', 'shareOK'),
    'req-repos': ('xstack',),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'OCR_ROOT': '${JJOB_PRIVATE_HOME}/xstack/ocr',
                  'OCR_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/ocr/install',
                  'OCR_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/xstack/ocr/build'}
}

jobtype_ocr_run_kernel_remote_regression = {
    'name': 'ocr-run-kernel-remote-regression',
    'isLocal': False,
    'run-cmd': '${JJOB_SHARED_HOME}/xstack/apps/jenkins/scripts/kernel-run-remote.sh',
    'param-cmd': '${JJOB_SHARED_HOME}/xstack/apps/jenkins/scripts/remote-param-cmd.sh',
    'keywords': ('regression',),
    'timeout': 900,
    'sandbox': ('shared', 'shareOK'),
    'req-repos': ('xstack',),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'OCR_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/ocr/install'}
}

jobtype_ocr_run_kernel_remote_scaling = {
    'name': 'ocr-run-kernel-remote-scaling',
    'isLocal': False,
    'run-cmd': '${JJOB_SHARED_HOME}/xstack/apps/jenkins/scripts/kernel-run-remote-scaling.sh',
    'param-cmd': '${JJOB_SHARED_HOME}/xstack/apps/jenkins/scripts/remote-param-cmd.sh',
    'keywords': ('regression',),
    'timeout': 300,
    'sandbox': ('shared', 'shareOK'),
    'req-repos': ('xstack',),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps',
                  'OCR_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/ocr/install'}
}

# Specific jobs


# FFT
job_ocr_build_kernel_fft_x86_regression = {
    'name': 'ocr-build-kernel-fft-x86-regression',
    'depends': ('ocr-build-x86-pthread-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'fft x86-pthread-x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${APPS_ROOT}/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/fft/ocr',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/xstack/apps/fft/ocr/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/fft/ocr/install'}
}

job_ocr_run_kernel_fft_x86_remote_regression = {
    'name': 'ocr-run-kernel-fft-x86-remote-regression',
    'depends': ('ocr-build-kernel-fft-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'run-args': 'fft x86-pthread-x86 ocr-run-kernel-fft-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${APPS_ROOT}/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/fft/ocr',
                  'WORKLOAD_ARGS': '20',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/fft/ocr/install' }
}
# Cholesky
job_ocr_build_kernel_cholesky_x86_regression = {
    'name': 'ocr-build-kernel-cholesky-x86-regression',
    'depends': ('ocr-build-x86-pthread-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'cholesky x86-pthread-x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${APPS_ROOT}/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/cholesky/ocr',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/xstack/apps/cholesky/ocr/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/cholesky/ocr/install'}
}

job_ocr_run_kernel_cholesky_x86_remote_regression = {
    'name': 'ocr-run-kernel-cholesky-x86-remote-regression',
    'depends': ('ocr-build-kernel-cholesky-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'run-args': 'cholesky x86-pthread-x86 ocr-run-kernel-cholesky-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${APPS_ROOT}/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/cholesky/ocr',
                  'WORKLOAD_ARGS': '--ds 1000 --ts 20 --fi ${APPS_ROOT}/cholesky/datasets/m_1000.in',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/cholesky/ocr/install' }
}

# fibonacci
job_ocr_build_kernel_fibonacci_x86_regression = {
    'name': 'ocr-build-kernel-fibonacci-x86-regression',
    'depends': ('ocr-build-x86-pthread-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'fibonacci x86-pthread-x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/fibonacci/ocr',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/xstack/apps/fibonacci/ocr/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/fibonacci/ocr/install'}
}
job_ocr_run_kernel_fibonacci_x86_remote_regression = {
    'name': 'ocr-run-kernel-fibonacci-x86-remote-regression',
    'depends': ('ocr-build-kernel-fibonacci-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'run-args': 'fibonacci x86-pthread-x86 ocr-run-kernel-fibonacci-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/fibonacci/ocr',
                  'WORKLOAD_ARGS': '20',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/fibonacci/ocr/install' }
}

#job_ocr_run_kernel_fibonacci_x86_remote_scaling = {
#    'name': 'ocr-run-kernel-fibonacci-x86-remote-scaling',
#    'depends': ('ocr-run-kernel-fibonacci-x86-remote-regression',),
#    'jobtype': 'ocr-run-kernel-remote-scaling',
#    'run-args': 'fibonacci x86-pthread-x86 ocr-run-kernel-fibonacci-x86-remote-scaling 10',
#    'sandbox': ('shared','inherit0'),
#    'env-vars': { 'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
#                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/fibonacci/ocr',
#                  'WORKLOAD_ARGS': '23',
#                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/fibonacci/ocr/install' }
#}

# Smith-Waterman
job_ocr_build_kernel_smithwaterman_x86_regression = {
    'name': 'ocr-build-kernel-smithwaterman-x86-regression',
    'depends': ('ocr-build-x86-pthread-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'smithwaterman x86-pthread-x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/smithwaterman/ocr',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/xstack/apps/smithwaterman/ocr/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/smithwaterman/ocr/install'}
}

job_ocr_run_kernel_smithwaterman_x86_remote_regression = {
    'name': 'ocr-run-kernel-smithwaterman-x86-remote-regression',
    'depends': ('ocr-build-kernel-smithwaterman-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'run-args': 'smithwaterman x86-pthread-x86 ocr-run-kernel-smithwaterman-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/smithwaterman/ocr',
                  'WORKLOAD_ARGS': '10 10 ${JJOB_SHARED_HOME}/xstack/apps/smithwaterman/datasets/string1-medium-large.txt ${JJOB_SHARED_HOME}/xstack/apps/smithwaterman/datasets/string2-medium-large.txt',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/smithwaterman/ocr/install'}
}

# 1d Stencil David Scott
job_ocr_build_kernel_Stencil1DDavid_x86_regression = {
    'name': 'ocr-build-kernel-Stencil1DDavid-x86-regression',
    'depends': ('ocr-build-x86-pthread-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'stencil x86-pthread-x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/Stencil1D/refactored/ocr/intel-david',
                  'CODE_TYPE': '2',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/xstack/apps/Stencil1D/refactored/ocr/intel-david/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/Stencil1D/refactored/ocr/intel-david/install'}
}

job_ocr_run_kernel_Stencil1DDavid_x86_remote_regression = {
    'name': 'ocr-run-kernel-Stencil1DDavid-x86-remote-regression',
    'depends': ('ocr-build-kernel-Stencil1DDavid-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'run-args': 'stencil x86-pthread-x86 ocr-run-kernel-Stencil1DDavid-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/Stencil1D/refactored/ocr/intel-david',
                  'CODE_TYPE': '2',
                  'WORKLOAD_ARGS': '',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/Stencil1D/refactored/ocr/intel-david/install'}
}

#1d Stencil - Chandra Martha
job_ocr_build_kernel_Stencil1DChandra_x86_regression = {
    'name': 'ocr-build-kernel-Stencil1DChandra-x86-regression',
    'depends': ('ocr-build-x86-pthread-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'stencil_1d x86-pthread-x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/Stencil1D/refactored/ocr/intel-chandra',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/xstack/apps/Stencil1D/refactored/ocr/intel-chandra/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/Stencil1D/refactored/ocr/intel-chandra/install'}
}

job_ocr_run_kernel_Stencil1DChandra_x86_remote_regression = {
    'name': 'ocr-run-kernel-Stencil1DChandra-x86-remote-regression',
    'depends': ('ocr-build-kernel-Stencil1DChandra-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'run-args': 'stencil_1d x86-pthread-x86 ocr-run-kernel-Stencil1DChandra-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/xstack/apps/Stencil1D/refactored/ocr/intel-chandra',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/xstack/apps/Stencil1D/refactored/ocr/intel-chandra/install',
                  'WORKLOAD_ARGS': '1000 100 500 5 -1 -1'}
}

#Aggregates execution times in csv file
job_gatherStats = {
    'name': 'gatherStats',
    'depends': ('__type ocr-run-kernel-remote-regression', '__type ocr-run-kernel-remote-scaling',),
    'jobtype': 'gatherStats-regression',
    'run-args': '${JJOB_SHARED_HOME}/xstack/runtime 10',
    'sandbox': ('shared','inherit0')
}
