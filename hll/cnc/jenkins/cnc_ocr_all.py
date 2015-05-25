#!/usr/bin/env python

import os

###################################################
# job types

jobtype_cnc_ocr_bootstrap = {
    'name': 'cnc-ocr-bootstrap-base',
    'isLocal': True,
    'run-cmd': '${JJOB_PRIVATE_HOME}/xstack/hll/cnc/py/bootstrap.sh',
    'param-cmd': '${JJOB_PRIVATE_HOME}/xstack/jenkins/scripts/empty-cmd.sh',
    'keywords': ('cnc-ocr',),
    'timeout': 60,
    'sandbox': ('local', 'emptyShared', 'shareOK'),
    'req-repos': ('xstack',),
    'env-vars': {'XSTACK_ROOT': '${JJOB_PRIVATE_HOME}/xstack'}
}

jobtype_cnc_ocr_example_gen = {
    'name': 'cnc-ocr-example-gen',
    'isLocal': True,
    'run-cmd': '${JJOB_PRIVATE_HOME}/xstack/hll/cnc/jenkins/scripts/example-gen.sh',
    'param-cmd': '${JJOB_PRIVATE_HOME}/xstack/jenkins/scripts/empty-cmd.sh',
    'keywords': ('cnc-ocr',),
    'timeout': 60,
    'sandbox': ('local', 'emptyShared', 'shareOK'),
    'req-repos': ('xstack',),
    'env-vars': {'XSTACK_ROOT': '${JJOB_PRIVATE_HOME}/xstack'}
}

jobtype_cnc_ocr_example_make = {
    'name': 'cnc-ocr-example-make',
    'isLocal': True,
    'run-cmd': '${JJOB_PRIVATE_HOME}/xstack/hll/cnc/jenkins/scripts/example-make.sh',
    'param-cmd': '${JJOB_PRIVATE_HOME}/xstack/jenkins/scripts/empty-cmd.sh',
    'keywords': ('cnc-ocr',),
    'timeout': 60,
    'sandbox': ('local', 'emptyShared', 'shareOK'),
    'req-repos': ('xstack',),
    'env-vars': {'XSTACK_ROOT': '${JJOB_PRIVATE_HOME}/xstack'}
}

jobtype_cnc_ocr_example_run = {
    'name': 'cnc-ocr-example-run',
    'isLocal': True,
    'run-cmd': '${JJOB_PRIVATE_HOME}/xstack/hll/cnc/jenkins/scripts/example-run.sh',
    'param-cmd': '${JJOB_PRIVATE_HOME}/xstack/jenkins/scripts/empty-cmd.sh',
    'keywords': ('cnc-ocr',),
    'timeout': 60,
    'sandbox': ('local', 'emptyShared', 'shareOK'),
    'req-repos': ('xstack',),
    'env-vars': {'XSTACK_ROOT': '${JJOB_PRIVATE_HOME}/xstack'}
}

jobtype_cnc_ocr_example_grep_verify = {
    'name': 'cnc-ocr-example-grep-verify',
    'isLocal': True,
    'run-cmd': '${JJOB_PRIVATE_HOME}/xstack/hll/cnc/jenkins/scripts/example-grep-verify.sh',
    'param-cmd': '${JJOB_PRIVATE_HOME}/xstack/jenkins/scripts/empty-cmd.sh',
    'keywords': ('cnc-ocr',),
    'timeout': 60,
    'sandbox': ('local', 'emptyShared', 'shareOK'),
    'req-repos': ('xstack',),
    'env-vars': {'XSTACK_ROOT': '${JJOB_PRIVATE_HOME}/xstack'}
}

###################################################
# specific jobs

job_cnc_ocr_bootstrap = {
    'name': 'cnc-ocr-bootstrap',
    'depends': (),
    'jobtype': 'cnc-ocr-bootstrap-base',
    'run-args': '',
    'sandbox': ()
}

# Smith-Waterman

job_cnc_ocr_example_gen_sw = {
    'name': 'cnc-ocr-example-gen-sw',
    'depends': ('cnc-ocr-bootstrap',),
    'jobtype': 'cnc-ocr-example-gen',
    'run-args': 'SmithWaterman',
    'sandbox': ('inherit0',),
}

job_cnc_ocr_example_make_sw = {
    'name': 'cnc-ocr-example-make-sw',
    'depends': ('cnc-ocr-example-gen-sw','ocr-build-x86-pthread-x86'),
    'jobtype': 'cnc-ocr-example-make',
    'run-args': 'SmithWaterman',
    'sandbox': ('inherit0',),
}

job_cnc_ocr_example_run_sw = {
    'name': 'cnc-ocr-example-run-sw',
    'depends': ('cnc-ocr-example-make-sw',),
    'jobtype': 'cnc-ocr-example-run',
    'run-args': 'SmithWaterman 10 10 ${DATA_DIR}/string1-medium.txt ${DATA_DIR}/string2-medium.txt',
    'sandbox': ('inherit0',),
    'env-vars': {'DATA_DIR': '${XSTACK_ROOT}/apps/smithwaterman/datasets'}
}

job_cnc_ocr_example_verify_sw = {
    'name': 'cnc-ocr-example-verify-sw',
    'depends': ('cnc-ocr-example-run-sw',),
    'jobtype': 'cnc-ocr-example-grep-verify',
    'run-args': 'SmithWaterman "score: 80"',
    'sandbox': ('inherit0',),
}


# combinations

job_cnc_ocr_example_gen_combinations = {
    'name': 'cnc-ocr-example-gen-combinations',
    'depends': ('cnc-ocr-bootstrap',),
    'jobtype': 'cnc-ocr-example-gen',
    'run-args': 'Combinations',
    'sandbox': ('inherit0',),
}

job_cnc_ocr_example_make_combinations = {
    'name': 'cnc-ocr-example-make-combinations',
    'depends': ('cnc-ocr-example-gen-combinations','ocr-build-x86-pthread-x86'),
    'jobtype': 'cnc-ocr-example-make',
    'run-args': 'Combinations',
    'sandbox': ('inherit0',),
}

job_cnc_ocr_example_run_combinations = {
    'name': 'cnc-ocr-example-run-combinations',
    'depends': ('cnc-ocr-example-make-combinations',),
    'jobtype': 'cnc-ocr-example-run',
    'run-args': 'Combinations 100 7',
    'sandbox': ('inherit0',),
}

job_cnc_ocr_example_verify_combinations = {
    'name': 'cnc-ocr-example-verify-combinations',
    'depends': ('cnc-ocr-example-run-combinations',),
    'jobtype': 'cnc-ocr-example-grep-verify',
    'run-args': 'Combinations "100 choose 7 = 16007560800"',
    'sandbox': ('inherit0',),
}

# Cholesky

job_cnc_ocr_example_gen_cholesky = {
    'name': 'cnc-ocr-example-gen-cholesky',
    'depends': ('cnc-ocr-bootstrap',),
    'jobtype': 'cnc-ocr-example-gen',
    'run-args': 'Cholesky/generated_input',
    'sandbox': ('inherit0',),
}

job_cnc_ocr_example_make_cholesky = {
    'name': 'cnc-ocr-example-make-cholesky',
    'depends': ('cnc-ocr-example-gen-cholesky','ocr-build-x86-pthread-x86'),
    'jobtype': 'cnc-ocr-example-make',
    'run-args': 'Cholesky/generated_input',
    'sandbox': ('inherit0',),
}

job_cnc_ocr_example_run_cholesky = {
    'name': 'cnc-ocr-example-run-cholesky',
    'depends': ('cnc-ocr-example-make-cholesky',),
    'jobtype': 'cnc-ocr-example-run',
    'run-args': 'Cholesky/generated_input 2500 125',
    'sandbox': ('inherit0',),
}

job_cnc_ocr_example_verify_cholesky = {
    'name': 'cnc-ocr-example-verify-cholesky',
    'depends': ('cnc-ocr-example-run-cholesky',),
    'jobtype': 'cnc-ocr-example-grep-verify',
    'run-args': 'Cholesky/generated_input "checksum: d5ff728615a593f"',
    'sandbox': ('inherit0',),
}

