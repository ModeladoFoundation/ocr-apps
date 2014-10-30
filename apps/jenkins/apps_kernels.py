#!/usr/bin/env python
# This relies on OCR being built in the shared home
# but does not need any seperate checkout as it runs
# completely out of the source directory

jobtype_gatherStats_regression = {
    'name': 'gatherStats-regression',
    'isLocal': True,
    'run-cmd': '${JJOB_SHARED_HOME}/apps/jenkins/scripts/statCollector.sh',
    'param-cmd': '${JJOB_SHARED_HOME}/jenkins/scripts/empty-cmd.sh',
    'keywords': ('regression',),
    'timeout': 20,
    'sandbox': ('shared', 'shareOK')
}
jobtype_ocr_build_kernel_regression = {
    'name': 'ocr-build-kernel-regression',
    'isLocal': True,
    'run-cmd': '${JJOB_SHARED_HOME}/ocr/jenkins/scripts/kernel-build.sh',
    'param-cmd': '${JJOB_SHARED_HOME}/jenkins/scripts/empty-cmd.sh',
    'keywords': ('regression',),
    'timeout': 300,
    'sandbox': ('local', 'shared', 'shareOK'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/apps',
                  'OCR_SRC': '${JJOB_PRIVATE_HOME}/ocr',
                  'OCR_INSTALL_ROOT': '${JJOB_SHARED_HOME}/ocr/install',
                  'OCR_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/ocr/build'}
}

jobtype_ocr_run_kernel_remote_regression = {
    'name': 'ocr-run-kernel-remote-regression',
    'isLocal': False,
    'run-cmd': '${JJOB_SHARED_HOME}/apps/jenkins/scripts/kernel-run-remote.sh',
    'param-cmd': '${JJOB_SHARED_HOME}/apps/jenkins/scripts/remote-param-cmd.sh',
    'keywords': ('regression',),
    'timeout': 300,
    'sandbox': ('shared', 'shareOK'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/apps',
                  'OCR_INSTALL_ROOT': '${JJOB_SHARED_HOME}/ocr/install'}
}

# Specific jobs

# Cholesky
job_ocr_build_kernel_cholesky_x86_regression = {
    'name': 'ocr-build-kernel-cholesky-x86-regression',
    'depends': ('ocr-build-x86-pthread-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'cholesky x86-pthread-x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${APPS_ROOT}/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/apps/cholesky/ocr',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/apps/cholesky/ocr/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/apps/cholesky/ocr/install'}
}

job_ocr_run_kernel_cholesky_x86_remote_regression = {
    'name': 'ocr-run-kernel-cholesky-x86-remote-regression',
    'depends': ('ocr-build-kernel-cholesky-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'run-args': 'cholesky x86-pthread-x86 ocr-run-kernel-cholesky-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${APPS_ROOT}/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/apps/cholesky/ocr',
                  'WORKLOAD_ARGS': '1000 20 ${APPS_ROOT}/cholesky/datasets/m_1000.in',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/apps/cholesky/ocr/install' }
}

# FFT
job_ocr_build_kernel_fft_x86_regression = {
    'name': 'ocr-build-kernel-fft-x86-regression',
    'depends': ('ocr-build-x86-pthread-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'fft x86-pthread-x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${APPS_ROOT}/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/apps/fft/ocr',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/apps/fft/ocr/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/apps/fft/ocr/install'}
}

job_ocr_run_kernel_fft_x86_remote_regression = {
    'name': 'ocr-run-kernel-fft-x86-remote-regression',
    'depends': ('ocr-build-kernel-fft-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'run-args': 'fft x86-pthread-x86 ocr-run-kernel-fft-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${APPS_ROOT}/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/apps/fft/ocr',
                  'WORKLOAD_ARGS': '20',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/apps/fft/ocr/install' }
}

# fibonacci
job_ocr_build_kernel_fibonacci_x86_regression = {
    'name': 'ocr-build-kernel-fibonacci-x86-regression',
    'depends': ('ocr-build-x86-pthread-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'fibonacci x86-pthread-x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/apps/fibonacci/ocr',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/apps/fibonacci/ocr/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/apps/fibonacci/ocr/install'}
}
job_ocr_run_kernel_fibonacci_x86_remote_regression = {
    'name': 'ocr-run-kernel-fibonacci-x86-remote-regression',
    'depends': ('ocr-build-kernel-fibonacci-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'run-args': 'fibonacci x86-pthread-x86 ocr-run-kernel-fibonacci-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/apps/fibonacci/ocr',
                  'WORKLOAD_ARGS': '20',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/apps/fibonacci/ocr/install' }
}

# Smith-Waterman
job_ocr_build_kernel_smithwaterman_x86_regression = {
    'name': 'ocr-build-kernel-smithwaterman-x86-regression',
    'depends': ('ocr-build-x86-pthread-x86',),
    'jobtype': 'ocr-build-kernel-regression',
    'run-args': 'smithwaterman x86-pthread-x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/apps/smithwaterman/ocr',
                  'WORKLOAD_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/apps/smithwaterman/ocr/build',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/apps/smithwaterman/ocr/install'}
}

job_ocr_run_kernel_smithwaterman_x86_remote_regression = {
    'name': 'ocr-run-kernel-smithwaterman-x86-remote-regression',
    'depends': ('ocr-build-kernel-smithwaterman-x86-regression',),
    'jobtype': 'ocr-run-kernel-remote-regression',
    'run-args': 'smithwaterman x86-pthread-x86 ocr-run-kernel-smithwaterman-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'APPS_LIBS_ROOT': '${JJOB_SHARED_HOME}/apps/libs/x86',
                  'WORKLOAD_SRC': '${JJOB_SHARED_HOME}/apps/smithwaterman/ocr',
                  'WORKLOAD_ARGS': '10 10 ${JJOB_SHARED_HOME}/apps/smithwaterman/datasets/string1-medium-large.txt ${JJOB_SHARED_HOME}/apps/smithwaterman/datasets/string2-medium-large.txt',
                  'WORKLOAD_INSTALL_ROOT': '${JJOB_SHARED_HOME}/apps/smithwaterman/ocr/install'}
}
#Aggregates execution times in csv file
job_gatherStats = {
    'name': 'gatherStats',
    'depends': ('ocr-run-kernel-cholesky-x86-remote-regression','ocr-run-kernel-fft-x86-remote-regression',\
                'ocr-run-kernel-smithwaterman-x86-remote-regression',\
                'ocr-run-kernel-fibonacci-x86-remote-regression'),
    'jobtype': 'gatherStats-regression',
    'run-args': '${JJOB_SHARED_HOME}/runtime 10',
    'sandbox': ('shared','inherit0')
}
