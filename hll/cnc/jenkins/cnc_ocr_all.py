#!/usr/bin/env python

import os

###################################################
# job types

jobtype_cnc_ocr_bootstrap = {
    'name': 'cnc-ocr-bootstrap-base',
    'isLocal': True,
    'run-cmd': '${JJOB_SHARED_HOME}/xstack/hll/cnc/py/bootstrap.sh',
    'param-cmd': '${JJOB_SHARED_HOME}/xstack/jenkins/scripts/empty-cmd.sh',
    'keywords': ('cnc-ocr',),
    'timeout': 60,
    'sandbox': ('shared', 'shareOK'),
    'req-repos': ('xstack',),
    'env-vars': {'XSTACK_ROOT': '${JJOB_SHARED_HOME}/xstack'}
}

jobtype_cnc_ocr_app_gen = {
    'name': 'cnc-ocr-app-gen',
    'isLocal': True,
    'run-cmd': '${JJOB_SHARED_HOME}/xstack/hll/cnc/jenkins/scripts/app-gen.sh',
    'param-cmd': '${JJOB_SHARED_HOME}/xstack/jenkins/scripts/empty-cmd.sh',
    'keywords': ('cnc-ocr',),
    'timeout': 120,
    'sandbox': ('shared', 'shareOK'),
    'req-repos': ('xstack',),
    'env-vars': {'XSTACK_ROOT': '${JJOB_SHARED_HOME}/xstack'}
}

jobtype_cnc_ocr_app_build = {
    'name': 'cnc-ocr-app-build',
    'isLocal': True,
    'run-cmd': '${JJOB_SHARED_HOME}/xstack/hll/cnc/jenkins/scripts/app-build.sh',
    'param-cmd': '${JJOB_SHARED_HOME}/xstack/jenkins/scripts/empty-cmd.sh',
    'keywords': ('cnc-ocr',),
    'timeout': 60,
    'sandbox': ('shared', 'shareOK'),
    'req-repos': ('xstack',),
    'env-vars': {'XSTACK_ROOT': '${JJOB_SHARED_HOME}/xstack'}
}

jobtype_cnc_ocr_app_run = {
    'name': 'cnc-ocr-app-run',
    'isLocal': True,
    'run-cmd': '${JJOB_SHARED_HOME}/xstack/hll/cnc/jenkins/scripts/app-run.sh',
    'param-cmd': '${JJOB_SHARED_HOME}/xstack/jenkins/scripts/empty-cmd.sh',
    'keywords': ('cnc-ocr',),
    'timeout': 60,
    'sandbox': ('shared', 'shareOK'),
    'req-repos': ('xstack',),
    'env-vars': {'XSTACK_ROOT': '${JJOB_SHARED_HOME}/xstack'}
}

jobtype_cnc_ocr_app_grep_verify = {
    'name': 'cnc-ocr-app-grep-verify',
    'isLocal': True,
    'run-cmd': '${JJOB_SHARED_HOME}/xstack/hll/cnc/jenkins/scripts/app-grep-verify.sh',
    'param-cmd': '${JJOB_SHARED_HOME}/xstack/jenkins/scripts/empty-cmd.sh',
    'keywords': ('cnc-ocr',),
    'timeout': 60,
    'sandbox': ('shared', 'shareOK'),
    'req-repos': ('xstack',),
    'env-vars': {'XSTACK_ROOT': '${JJOB_SHARED_HOME}/xstack'}
}

###################################################
# specific jobs

job_cnc_ocr_bootstrap_x86 = {
    'name': 'cnc-ocr-bootstrap-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'cnc-ocr-bootstrap-base',
    'run-args': '',
    'sandbox': ('shared', 'inherit0'),
}

# Smith-Waterman

job_cnc_ocr_app_gen_sw_x86 = {
    'name': 'cnc-ocr-app-gen-sw-x86',
    'depends': ('cnc-ocr-bootstrap-x86',),
    'jobtype': 'cnc-ocr-app-gen',
    'run-args': 'SmithWaterman',
    'sandbox': ('shared', 'inherit0'),
}

job_cnc_ocr_app_build_sw_x86 = {
    'name': 'cnc-ocr-app-build-sw-x86',
    'depends': ('cnc-ocr-app-gen-sw-x86',),
    'jobtype': 'cnc-ocr-app-build',
    'run-args': 'SmithWaterman',
    'sandbox': ('shared', 'inherit0'),
}

job_cnc_ocr_app_run_sw_x86 = {
    'name': 'cnc-ocr-app-run-sw-x86',
    'depends': ('cnc-ocr-app-build-sw-x86',),
    'jobtype': 'cnc-ocr-app-run',
    'run-args': 'SmithWaterman 10 10 ${DATA_DIR}/string1-medium.txt ${DATA_DIR}/string2-medium.txt',
    'sandbox': ('shared', 'inherit0'),
    'env-vars': {'DATA_DIR': '${XSTACK_ROOT}/apps/smithwaterman/datasets'}
}

job_cnc_ocr_app_verify_sw_x86 = {
    'name': 'cnc-ocr-app-verify-sw-x86',
    'depends': ('cnc-ocr-app-run-sw-x86',),
    'jobtype': 'cnc-ocr-app-grep-verify',
    'run-args': 'SmithWaterman "score: 80"',
    'sandbox': ('shared', 'inherit0'),
}


# combinations

job_cnc_ocr_app_gen_combinations_x86 = {
    'name': 'cnc-ocr-app-gen-combinations-x86',
    'depends': ('cnc-ocr-bootstrap-x86',),
    'jobtype': 'cnc-ocr-app-gen',
    'run-args': 'Combinations',
    'sandbox': ('shared', 'inherit0'),
}

job_cnc_ocr_app_build_combinations_x86 = {
    'name': 'cnc-ocr-app-build-combinations-x86',
    'depends': ('cnc-ocr-app-gen-combinations-x86',),
    'jobtype': 'cnc-ocr-app-build',
    'run-args': 'Combinations',
    'sandbox': ('shared', 'inherit0'),
}

job_cnc_ocr_app_run_combinations_x86 = {
    'name': 'cnc-ocr-app-run-combinations-x86',
    'depends': ('cnc-ocr-app-build-combinations-x86',),
    'jobtype': 'cnc-ocr-app-run',
    'run-args': 'Combinations 100 7',
    'sandbox': ('shared', 'inherit0'),
}

job_cnc_ocr_app_verify_combinations_x86 = {
    'name': 'cnc-ocr-app-verify-combinations-x86',
    'depends': ('cnc-ocr-app-run-combinations-x86',),
    'jobtype': 'cnc-ocr-app-grep-verify',
    'run-args': 'Combinations "100 choose 7 = 16007560800"',
    'sandbox': ('shared', 'inherit0'),
}

# Cholesky

job_cnc_ocr_app_gen_cholesky_x86 = {
    'name': 'cnc-ocr-app-gen-cholesky-x86',
    'depends': ('cnc-ocr-bootstrap-x86',),
    'jobtype': 'cnc-ocr-app-gen',
    'run-args': 'Cholesky/generated_input',
    'sandbox': ('shared', 'inherit0'),
}

job_cnc_ocr_app_build_cholesky_x86 = {
    'name': 'cnc-ocr-app-build-cholesky-x86',
    'depends': ('cnc-ocr-app-gen-cholesky-x86',),
    'jobtype': 'cnc-ocr-app-build',
    'run-args': 'Cholesky/generated_input',
    'sandbox': ('shared', 'inherit0'),
}

job_cnc_ocr_app_run_cholesky_x86 = {
    'name': 'cnc-ocr-app-run-cholesky-x86',
    'depends': ('cnc-ocr-app-build-cholesky-x86',),
    'jobtype': 'cnc-ocr-app-run',
    'run-args': 'Cholesky/generated_input 2500 125',
    'sandbox': ('shared', 'inherit0'),
}

job_cnc_ocr_app_verify_cholesky_x86 = {
    'name': 'cnc-ocr-app-verify-cholesky-x86',
    'depends': ('cnc-ocr-app-run-cholesky-x86',),
    'jobtype': 'cnc-ocr-app-grep-verify',
    'run-args': 'Cholesky/generated_input "checksum: d5ff728615a593f"',
    'sandbox': ('shared', 'inherit0'),
}
