#!/usr/bin/env python
# This relies on OCR being built in the shared home
# but does not need any separate checkout as it runs
# completely out of the source directory

jobtype_apps_init = {
    'name': 'apps-init',
    'isLocal': True,
    'run-cmd': '${JJOB_INITDIR_APPS}/apps/jenkins/scripts/init.sh',
    'param-cmd': '${JJOB_INITDIR_APPS}/jenkins/scripts/empty-cmd.sh',
    'depends': ('__alternate ocr-init', ),
    'keywords': ('apps',),
    'timeout': 300,
    'sandbox': ('local', 'shared', 'shareOK'),
    'req-repos': ('apps', 'ocr')
}

jobtype_gatherStats_regression = {
    'name': 'gatherStats-regression',
    'isLocal': True,
    'run-cmd': '${JJOB_INITDIR_APPS}/apps/jenkins/scripts/statCollector.sh',
    'param-cmd': '${JJOB_INITDIR_APPS}/jenkins/scripts/empty-cmd.sh',
    'keywords': ('apps', 'regression', 'nightly'),
    'timeout': 20,
    'sandbox': ('shared', 'shareOK'),
    'req-repos': ('apps', )
}

jobtype_ocr_build_app = {
    'name': 'ocr-build-app',
    'isLocal': True,
    'run-cmd': '${JJOB_INITDIR_APPS}/apps/jenkins/scripts/kernel-build.sh',
    'param-cmd': '${JJOB_INITDIR_APPS}/jenkins/scripts/empty-cmd.sh',
    'depends': ('apps-init-job', ),
    'keywords': ('apps', ),
    'timeout': 300,
    'sandbox': ('local', 'shared', 'shareOK'),
    'req-repos': ('apps', 'ocr'),
    'env-vars': { 'ARCH': '${T_ARCH}',
                  # We always add the MPI component as some apps need MPI to build.
                  'PATH': '/opt/intel/tools/impi/5.1.1.109/intel64/bin:${PATH}',
                  'APPS_ROOT': '${JJOB_SHARED_HOME}/apps/apps',
                  'APPS_LIBS_ROOT': '${APPS_ROOT}/libs/src',
                  'APPS_ROOT_PRIV': '${JJOB_PRIVATE_HOME}/apps/apps',
                  'OCR_ROOT': '${JJOB_PRIVATE_HOME}/ocr/ocr',
                  'OCR_INSTALL': '${JJOB_SHARED_HOME}/ocr/ocr/install',
                  'OCR_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/ocr/ocr/build',
                  'APPS_MAKEFILE': '${APPS_ROOT}/${T_PATH}/Makefile',
                  'S_PATH': '${T_PATH}',
                  'B_PATH': '${T_PATH}/build',
                  'I_PATH': '${T_PATH}/install',
                  'WORKLOAD_SRC': '${APPS_ROOT}/${S_PATH}',
                  'WORKLOAD_BUILD_ROOT': '${APPS_ROOT_PRIV}/${B_PATH}',
                  'WORKLOAD_INSTALL_ROOT': '${APPS_ROOT}/${I_PATH}'
              }
}

jobtype_ocr_build_app_tg = {
    'name': 'ocr-build-app-tg',
    'isLocal': True,
    'run-cmd': '${JJOB_INITDIR_APPS}/apps/jenkins/scripts/kernel-build.sh',
    'param-cmd': '${JJOB_INITDIR_APPS}/jenkins/scripts/empty-cmd.sh',
    'depends': ('apps-init-job', ),
    'keywords': ('apps', ),
    'timeout': 300,
    'sandbox': ('local', 'shared', 'shareOK'),
    'req-repos': ('apps', 'ocr', 'tg'),
    'env-vars': { 'ARCH': 'tg-xe',
                  'TG_INSTALL': '${JJOB_ENVDIR}',
                  'TG_ROOT': '${JJOB_INITDIR_TG}/tg',
                  'APPS_ROOT': '${JJOB_SHARED_HOME}/apps/apps',
                  'APPS_LIBS_ROOT': '${APPS_ROOT}/libs/src',
                  'APPS_ROOT_PRIV': '${JJOB_PRIVATE_HOME}/apps/apps',
                  'OCR_ROOT': '${JJOB_PRIVATE_HOME}/ocr/ocr',
                  'OCR_INSTALL': '${JJOB_SHARED_HOME}/ocr/ocr/install',
                  'OCR_BUILD_ROOT': '${JJOB_PRIVATE_HOME}/xstack/ocr/build',
                  'APPS_MAKEFILE': '${APPS_ROOT}/${T_PATH}/Makefile',
                  'S_PATH':'${T_PATH}',
                  'B_PATH': '${T_PATH}/build',
                  'I_PATH': '${T_PATH}/install',
                  'WORKLOAD_SRC': '${APPS_ROOT}/${S_PATH}',
                  'WORKLOAD_BUILD_ROOT': '${APPS_ROOT_PRIV}/${B_PATH}',
                  'WORKLOAD_INSTALL_ROOT': '${APPS_ROOT}/${I_PATH}'
              }
}

jobtype_ocr_run_app_nonregression = {
    'name': 'ocr-run-app-nonregression',
    'isLocal': True,
    'run-cmd': '${JJOB_INITDIR_APPS}/apps/jenkins/scripts/run-non-regression.sh',
    'param-cmd': '${JJOB_INITDIR_APPS}/jenkins/scripts/empty-cmd.sh',
    'keywords': ('apps', 'nonregression', 'nightly'),
    'timeout': 300,
    'sandbox': ('shared', 'shareOK'),
    'req-repos': ('apps', 'ocr'),
    'env-vars': { 'TG_INSTALL': '${JJOB_ENVDIR}',
                  'APPS_ROOT': '${JJOB_SHARED_HOME}/apps/apps', # Use this for the makefiles
                  'APPS_LIBS_ROOT': '${APPS_ROOT}/libs/src',
                  'OCR_ROOT': '${JJOB_SHARED_HOME}/ocr/ocr',
                  'OCR_INSTALL': '${JJOB_SHARED_HOME}/ocr/ocr/install',
                  'I_PATH': '${T_PATH}/install',
                  'WORKLOAD_INSTALL_ROOT': '${APPS_ROOT}/${I_PATH}'
              }
}

jobtype_ocr_run_app_tg = {
    'name': 'ocr-run-app-tg',
    'isLocal': False,
    'run-cmd': '${JJOB_INITDIR_APPS}/apps/jenkins/scripts/run-non-regression.sh',
    'param-cmd': '${JJOB_INITDIR_TG}/tg/jenkins/scripts/fsim-param-cmd.sh',
    'epilogue-cmd': '${JJOB_ENVDIR}/bin/scripts/fsim-epilogue.sh',
    'keywords': ('apps', 'nonregression', 'nightly'),
    'timeout': 300,
    'sandbox': ('shared', 'shareOK'),
    'req-repos': ('apps', 'ocr', 'tg'),
    'env-vars': { 'TG_INSTALL': '${JJOB_ENVDIR}',
                  'APPS_ROOT': '${JJOB_SHARED_HOME}/apps/apps', # Use this for the makefiles
                  'APPS_LIBS_ROOT': '${APPS_ROOT}/libs/src',
                  'OCR_INSTALL': '${JJOB_SHARED_HOME}/ocr/ocr/install',
                  'I_PATH': '${T_PATH}/install',
                  'WORKLOAD_INSTALL_ROOT': '${APPS_ROOT}/${I_PATH}'
              }
}

jobtype_ocr_run_app_regression = {
    'name': 'ocr-run-app-regression',
    'isLocal': False,
    'run-cmd': '${JJOB_SHARED_HOME}/apps/apps/jenkins/scripts/run-regression.sh',
    'param-cmd': '${JJOB_SHARED_HOME}/apps/apps/jenkins/scripts/remote-param-cmd.sh',
    'keywords': ('apps', 'regression', 'nightly'),
    'timeout': 900,
    'sandbox': ('shared', 'shareOK'),
    'req-repos': ('apps', 'ocr'),
    'env-vars': { 'TG_INSTALL': '${JJOB_ENVDIR}',
                  'APPS_ROOT': '${JJOB_SHARED_HOME}/apps/apps',
                  'APPS_ROOT_PRIV': '${JJOB_PRIVATE_HOME}/apps/apps',
                  'APPS_LIBS_ROOT': '${APPS_ROOT}/libs/src',
                  'OCR_ROOT': '${JJOB_SHARED_HOME}/ocr/ocr',
                  'OCR_INSTALL': '${JJOB_SHARED_HOME}/ocr/ocr/install',
                  'S_PATH':'${T_PATH}',
                  'B_PATH': '${T_PATH}/build',
                  'I_PATH': '${T_PATH}/install',
                  'WORKLOAD_SRC': '${APPS_ROOT}/${S_PATH}',
                  'WORKLOAD_INSTALL_ROOT': '${APPS_ROOT}/${I_PATH}'
              }
}

jobtype_ocr_run_app_scaling = {
    'name': 'ocr-run-app-scaling',
    'isLocal': False,
    'run-cmd': '${JJOB_SHARED_HOME}/apps/apps/jenkins/scripts/run-scaling.sh',
    'param-cmd': '${JJOB_SHARED_HOME}/apps/apps/jenkins/scripts/remote-param-cmd.sh',
    'keywords': ('apps', 'scaling', 'regression', 'nightly'),
    'timeout': 900,
    'sandbox': ('shared', 'shareOK'),
    'req-repos': ('apps', 'ocr'),
    'env-vars': { 'APPS_ROOT': '${JJOB_SHARED_HOME}/apps/apps',
                  'APPS_ROOT_PRIV': '${JJOB_PRIVATE_HOME}/apps/apps',
                  'APPS_LIBS_ROOT': '${APPS_ROOT}/libs/src',
                  'OCR_ROOT': '${JJOB_SHARED_HOME}/ocr/ocr',
                  'OCR_INSTALL': '${JJOB_SHARED_HOME}/ocr/ocr/install',
                  'S_PATH':'${T_PATH}',
                  'B_PATH': '${T_PATH}/build',
                  'I_PATH': '${T_PATH}/install',
                  'WORKLOAD_SRC': '${APPS_ROOT}/${S_PATH}',
                  'WORKLOAD_INSTALL_ROOT': '${APPS_ROOT}/${I_PATH}'
              }
}

jobtype_ocr_verify_app_local = {
    'name': 'ocr-verify-app-local',
    'isLocal': True,
    'run-cmd': '${JJOB_INITDIR_APPS}/apps/jenkins/scripts/ocr-verify-local.sh',
    'param-cmd': '${JJOB_INITDIR_APPS}/jenkins/scripts/empty-cmd.sh',
    'keywords': ('apps', ),
    'timeout': 60,
    'sandbox': ('shared', 'shareOK'),
    'req-repos': ('apps', )
}

jobtype_ocr_verify_kernel_remote = {
    'name': 'ocr-verify-app-remote',
    'isLocal': True,
    'run-cmd': '${JJOB_INITDIR_APPS}/apps/jenkins/scripts/fsim-verify.sh',
    'param-cmd': '${JJOB_INITDIR_APPS}/jenkins/scripts/empty-cmd.sh',
    'keywords': ('apps',),
    'timeout': 60,
    'sandbox': ('shared', 'shareOK'),
    'req-repos': ('apps',)
}

jobtype_ocr_verify_by_diff = {
    'name': 'ocr-verify-diff',
    'isLocal': True,
    'run-cmd': '${JJOB_INITDIR_APPS}/apps/jenkins/scripts/verify-diff.sh',
    'param-cmd': '${JJOB_INITDIR_APPS}/jenkins/scripts/empty-cmd.sh',
    'keywords': ('apps',),
    'timeout': 60,
    'sandbox': ('shared', 'shareOK'),
    'req-repos': ('apps',),
    'env-vars': { 'APPS_ROOT': '${JJOB_INITDIR_APPS}/apps' }
}
