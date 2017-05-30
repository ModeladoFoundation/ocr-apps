#!/usr/bin/env python


# RSBench: Intel ocr
job_ocr_build_kernel_rsbench_x86_regression = {
    'name': 'ocr-build-kernel-rsbench-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'RSBench x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'RSBench/refactored/ocr/intel',
                  'S_PATH': '${T_PATH}/src',
                }
}

job_ocr_run_kernel_rsbench_x86_remote_regression = {
    'name': 'ocr-run-kernel-rsbench-x86-remote-regression',
    'depends': ('ocr-build-kernel-rsbench-x86',),
    'jobtype': 'ocr-run-app-regression',
    'run-args': 'RSBench x86 ocr-run-kernel-rsbench-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'RSBench/refactored/ocr/intel',
                  'S_PATH': '${T_PATH}/src',
                  'WORKLOAD_ARGS': '-d -s small -l 100000',
                }
}

job_ocr_run_kernel_rsbench_x86_remote_scaling = {
   'name': 'ocr-run-kernel-rsbench-x86-remote-scaling',
   'depends': ('ocr-build-kernel-rsbench-x86',),
   'jobtype': 'ocr-run-app-scaling',
   'run-args': 'RSBench x86 ocr-run-kernel-rsbench-x86-remote-scaling 10',
   'sandbox': ('shared','inherit0'),
   'env-vars': { 'T_ARCH': 'x86',
                 'T_PATH': 'RSBench/refactored/ocr/intel',
                 'S_PATH': '${T_PATH}/src',
                 'WORKLOAD_ARGS': '-d -s small -l 100000',
               }
}

# XSBench: Intel ocr
job_ocr_build_kernel_xsbench_x86_regression = {
    'name': 'ocr-build-kernel-xsbench-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'XSBench x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'XSBench/refactored/ocr/intel',
                }
}

job_ocr_run_kernel_xsbench_x86_remote_regression = {
    'name': 'ocr-run-kernel-xsbench-x86-remote-regression',
    'depends': ('ocr-build-kernel-xsbench-x86',),
    'jobtype': 'ocr-run-app-regression',
    'run-args': 'XSBench x86 ocr-run-kernel-xsbench-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'XSBench/refactored/ocr/intel',
                  'WORKLOAD_ARGS': '-s small -g 1000 -l 100000',
                }
}

job_ocr_run_kernel_xsbench_x86_remote_scaling = {
   'name': 'ocr-run-kernel-xsbench-x86-remote-scaling',
   'depends': ('ocr-build-kernel-xsbench-x86',),
   'jobtype': 'ocr-run-app-scaling',
   'run-args': 'XSBench x86 ocr-run-kernel-xsbench-x86-remote-scaling 10',
   'sandbox': ('shared','inherit0'),
   'env-vars': { 'T_ARCH': 'x86',
                 'T_PATH': 'XSBench/refactored/ocr/intel',
                 'WORKLOAD_ARGS': '-s small -g 1000 -l 100000',
               }
}



## XSBench-intel-sharedDB

job_ocr_build_kernel_XSBench_intel_sharedDB_x86 = {
    'name': 'ocr-build-kernel-XSBench-intel-sharedDB-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'XSBench-intel-sharedDB x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'XSBench/refactored/ocr/intel-sharedDB',
                  'S_PATH': '${T_PATH}/src',
              }
}

job_ocr_run_kernel_XSBench_intel_sharedDB_x86 = {
    'name': 'ocr-run-kernel-XSBench-intel-sharedDB-x86',
    'depends': ('ocr-build-kernel-XSBench-intel-sharedDB-x86',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'XSBench-intel-sharedDB x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'XSBench/refactored/ocr/intel-sharedDB',
                  'S_PATH': '${T_PATH}/src',
                  'WORKLOAD_ARGS': '-s small -l 100000 -t 32',
              }
}

# job_ocr_verify_kernel_XSBench_intel_sharedDB_x86 = {
#     'name': 'ocr-verify-kernel-XSBench-intel-sharedDB-x86',
#     'depends': ('ocr-run-kernel-XSBench-intel-sharedDB-x86',),
#     'jobtype': 'ocr-verify-app-local',
#     'run-args': '-w -c 1',
#     'sandbox': ('inherit0',),
#     'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/XSBench-intel-sharedDB/ocr/install/x86'}
# }

# TODO: calibrate and add to baselineVal.py
# job_ocr_run_kernel_XSBench_intel_sharedDB_x86_remote_regression = {
#     'name': 'ocr-run-kernel-XSBench-intel-sharedDB-x86-remote-regression',
#     'depends': ('ocr-build-kernel-XSBench-intel-sharedDB-x86',),
#     'jobtype': 'ocr-run-app-regression',
#     'run-args': 'XSBench-intel-sharedDB x86 ocr-run-kernel-XSBench-intel-sharedDB-x86-remote-regression 10',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { 'T_ARCH': 'x86',
#                   'T_PATH': 'XSBench/refactored/ocr/intel-sharedDB',
#                   'S_PATH': '${T_PATH}/src',
#                   'WORKLOAD_ARGS': '-s small -l 100000 -t 32',
#               }
# }

# job_ocr_run_kernel_XSBench_intel_sharedDB_x86_remote_scaling = {
#     'name': 'ocr-run-kernel-XSBench-intel-sharedDB-x86-remote-scaling',
#     'depends': ('ocr-build-kernel-XSBench-intel-sharedDB-x86',),
#     'jobtype': 'ocr-run-app-scaling',
#     'run-args': 'XSBench-intel-sharedDB x86 ocr-run-kernel-XSBench-intel-sharedDB-x86-remote-scaling 10',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { 'T_ARCH': 'x86',
#                   'T_PATH': 'XSBench/refactored/ocr/intel-sharedDB',
#                   'S_PATH': '${T_PATH}/src',
#                   'WORKLOAD_ARGS': '-s small -l 100000 -t 32',
#               }
# }

# MPI version
job_ocr_build_kernel_XSBench_intel_sharedDB_mpi = {
    'name': 'ocr-build-kernel-XSBench-intel-sharedDB-mpi',
    'depends': ('ocr-build-x86-mpi',),
    'jobtype': 'ocr-build-app',
    'run-args': 'XSBench-intel-sharedDB x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'XSBench/refactored/ocr/intel-sharedDB',
                  'S_PATH': '${T_PATH}/src',
                  'WORKLOAD_ARGS': '-s small -l 100000 -t 32',
              }
}

job_ocr_run_kernel_XSBench_intel_sharedDB_mpi = {
    'name': 'ocr-run-kernel-XSBench-intel-sharedDB-mpi',
    'depends': ('ocr-build-kernel-XSBench-intel-sharedDB-mpi',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'XSBench-intel-sharedDB x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'PATH': '/opt/intel/tools/impi/5.1.1.109/intel64/bin:${PATH}',
                  'T_ARCH': 'x86',
                  'T_PATH': 'XSBench/refactored/ocr/intel-sharedDB',
                  'S_PATH': '${T_PATH}/src',
                  'WORKLOAD_ARGS': '-s small -l 100000 -t 32',
              }
}

# TG
job_ocr_build_kernel_RSBench_intel_sharedDB_tg = {
    'name': 'ocr-build-kernel-RSBench-intel-sharedDB-tg',
    'depends': ('ocr-build-builder-ce', 'ocr-build-builder-xe',
                'ocr-build-tg-ce', 'ocr-build-tg-xe', ),
    'jobtype': 'ocr-build-app-tg',
    'run-args': 'RSBench-intel-sharedDB tg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'RSBench/refactored/ocr/intel-sharedDB',
                  'S_PATH': '${T_PATH}/src',
                  #'WORKLOAD_ARGS': '',
              }
}

job_ocr_run_kernel_RSBench_intel_sharedDB_tg = {
    'name': 'ocr-run-kernel-RSBench-intel-sharedDB-tg',
    'depends': ('ocr-build-kernel-RSBench-intel-sharedDB-tg',),
    'jobtype': 'ocr-run-app-tg-newlib',
    'run-args': 'RSBench-intel-sharedDB tg',
    'param-args': '-c ${WORKLOAD_INSTALL_ROOT}/tg/config.cfg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'RSBench/refactored/ocr/intel-sharedDB',
                  'S_PATH': '${T_PATH}/src',
                  #'WORKLOAD_ARGS': '',
              }
}

# job_ocr_verify_kernel_RSBench_intel_sharedDB_tg = {
#     'name': 'ocr-verify-kernel-RSBench-intel-sharedDB-tg',
#     'depends': ('ocr-run-kernel-RSBench-intel-sharedDB-tg',),
#     'jobtype': 'ocr-verify-app-remote',
#     'run-args': '-w -c 1',
#     'sandbox': ('inherit0',),
#     'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/RSBench-intel-sharedDB/ocr/install/tg'}
# }

## miniAMR-intel-bryan

job_ocr_build_kernel_miniAMR_intel_bryan_x86 = {
    'name': 'ocr-build-kernel-miniAMR-intel-bryan-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'miniAMR-intel-bryan x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'miniAMR/refactored/ocr/intel-bryan',
                  'S_PATH': '${T_PATH}',
              }
}

job_ocr_run_kernel_miniAMR_intel_bryan_x86 = {
    'name': 'ocr-run-kernel-miniAMR-intel-bryan-x86',
    'depends': ('ocr-build-kernel-miniAMR-intel-bryan-x86',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'miniAMR-intel-bryan x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'miniAMR/refactored/ocr/intel-bryan',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

# job_ocr_verify_kernel_miniAMR_intel_bryan_x86 = {
#     'name': 'ocr-verify-kernel-miniAMR-intel-bryan-x86',
#     'depends': ('ocr-run-kernel-miniAMR-intel-bryan-x86',),
#     'jobtype': 'ocr-verify-app-local',
#     'run-args': '-w -c 1',
#     'sandbox': ('inherit0',),
#     'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/miniAMR-intel-bryan/ocr/install/x86'}
# }

# TODO: calibrate and add to baselineVal.py
# job_ocr_run_kernel_miniAMR_intel_bryan_x86_remote_regression = {
#     'name': 'ocr-run-kernel-miniAMR-intel-bryan-x86-remote-regression',
#     'depends': ('ocr-build-kernel-miniAMR-intel-bryan-x86',),
#     'jobtype': 'ocr-run-app-regression',
#     'run-args': 'miniAMR-intel-bryan x86 ocr-run-kernel-miniAMR-intel-bryan-x86-remote-regression 10',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { 'T_ARCH': 'x86',
#                   'T_PATH': 'miniAMR/refactored/ocr/intel-bryan',
#                   'S_PATH': '${T_PATH}',
#                   #'WORKLOAD_ARGS': '',
#               }
# }

# job_ocr_run_kernel_miniAMR_intel_bryan_x86_remote_scaling = {
#     'name': 'ocr-run-kernel-miniAMR-intel-bryan-x86-remote-scaling',
#     'depends': ('ocr-build-kernel-miniAMR-intel-bryan-x86',),
#     'jobtype': 'ocr-run-app-scaling',
#     'run-args': 'miniAMR-intel-bryan x86 ocr-run-kernel-miniAMR-intel-bryan-x86-remote-scaling 10',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { 'T_ARCH': 'x86',
#                   'T_PATH': 'miniAMR/refactored/ocr/intel-bryan',
#                   'S_PATH': '${T_PATH}',
#                   #'WORKLOAD_ARGS': '',
#               }
# }

# MPI version
job_ocr_build_kernel_miniAMR_intel_bryan_mpi = {
    'name': 'ocr-build-kernel-miniAMR-intel-bryan-mpi',
    'depends': ('ocr-build-x86-mpi',),
    'jobtype': 'ocr-build-app',
    'run-args': 'miniAMR-intel-bryan x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'miniAMR/refactored/ocr/intel-bryan',
                  'S_PATH': '${T_PATH}',
              }
}

job_ocr_run_kernel_miniAMR_intel_bryan_mpi = {
    'name': 'ocr-run-kernel-miniAMR-intel-bryan-mpi',
    'depends': ('ocr-build-kernel-miniAMR-intel-bryan-mpi',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'miniAMR-intel-bryan x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'PATH': '/opt/intel/tools/impi/5.1.1.109/intel64/bin:${PATH}',
                  'T_ARCH': 'x86',
                  'T_PATH': 'miniAMR/refactored/ocr/intel-bryan',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

# job_ocr_verify_kernel_miniAMR_intel_bryan_mpi = {
#     'name': 'ocr-verify-kernel-miniAMR-intel-bryan-mpi',
#     'depends': ('ocr-run-kernel-miniAMR-intel-bryan-mpi',),
#     'jobtype': 'ocr-verify-app-local',
#     'run-args': '-w -c 1',
#     'sandbox': ('inherit0',),
#     'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/miniAMR-intel-bryan/ocr/install/x86-mpi'}
# }

# TG
job_ocr_build_kernel_miniAMR_intel_bryan_tg = {
    'name': 'ocr-build-kernel-miniAMR-intel-bryan-tg',
    'depends': ('ocr-build-builder-ce', 'ocr-build-builder-xe',
                'ocr-build-tg-ce', 'ocr-build-tg-xe', ),
    'jobtype': 'ocr-build-app-tg',
    'run-args': 'miniAMR-intel-bryan tg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'miniAMR/refactored/ocr/intel-bryan',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

job_ocr_run_kernel_miniAMR_intel_bryan_tg = {
    'name': 'ocr-run-kernel-miniAMR-intel-bryan-tg',
    'depends': ('ocr-build-kernel-miniAMR-intel-bryan-tg',),
    'jobtype': 'ocr-run-app-tg-newlib',
    'run-args': 'miniAMR-intel-bryan tg',
    'param-args': '-c ${WORKLOAD_INSTALL_ROOT}/tg/config.cfg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'miniAMR/refactored/ocr/intel-bryan',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

# job_ocr_verify_kernel_miniAMR_intel_bryan_tg = {
#     'name': 'ocr-verify-kernel-miniAMR-intel-bryan-tg',
#     'depends': ('ocr-run-kernel-miniAMR-intel-bryan-tg',),
#     'jobtype': 'ocr-verify-app-remote',
#     'run-args': '-w -c 1',
#     'sandbox': ('inherit0',),
#     'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/miniAMR-intel-bryan/ocr/install/tg'}
# }

## hpcg-intel-Eager

job_ocr_build_kernel_hpcg_intel_Eager_x86 = {
    'name': 'ocr-build-kernel-hpcg-intel-Eager-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'hpcg-intel-Eager x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'hpcg/refactored/ocr/intel-Eager',
                  'S_PATH': '${T_PATH}',
              }
}

job_ocr_run_kernel_hpcg_intel_Eager_x86 = {
    'name': 'ocr-run-kernel-hpcg-intel-Eager-x86',
    'depends': ('ocr-build-kernel-hpcg-intel-Eager-x86',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'hpcg-intel-Eager x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'hpcg/refactored/ocr/intel-Eager',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

# job_ocr_verify_kernel_hpcg_intel_Eager_x86 = {
#     'name': 'ocr-verify-kernel-hpcg-intel-Eager-x86',
#     'depends': ('ocr-run-kernel-hpcg-intel-Eager-x86',),
#     'jobtype': 'ocr-verify-app-local',
#     'run-args': '-w -c 1',
#     'sandbox': ('inherit0',),
#     'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/hpcg-intel-Eager/ocr/install/x86'}
# }

# TODO: calibrate and add to baselineVal.py
# job_ocr_run_kernel_hpcg_intel_Eager_x86_remote_regression = {
#     'name': 'ocr-run-kernel-hpcg-intel-Eager-x86-remote-regression',
#     'depends': ('ocr-build-kernel-hpcg-intel-Eager-x86',),
#     'jobtype': 'ocr-run-app-regression',
#     'run-args': 'hpcg-intel-Eager x86 ocr-run-kernel-hpcg-intel-Eager-x86-remote-regression 10',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { 'T_ARCH': 'x86',
#                   'T_PATH': 'hpcg/refactored/ocr/intel-Eager',
#                   'S_PATH': '${T_PATH}',
#                   #'WORKLOAD_ARGS': '',
#               }
# }

# job_ocr_run_kernel_hpcg_intel_Eager_x86_remote_scaling = {
#     'name': 'ocr-run-kernel-hpcg-intel-Eager-x86-remote-scaling',
#     'depends': ('ocr-build-kernel-hpcg-intel-Eager-x86',),
#     'jobtype': 'ocr-run-app-scaling',
#     'run-args': 'hpcg-intel-Eager x86 ocr-run-kernel-hpcg-intel-Eager-x86-remote-scaling 10',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { 'T_ARCH': 'x86',
#                   'T_PATH': 'hpcg/refactored/ocr/intel-Eager',
#                   'S_PATH': '${T_PATH}',
#                   #'WORKLOAD_ARGS': '',
#               }
# }

# MPI version
job_ocr_build_kernel_hpcg_intel_Eager_mpi = {
    'name': 'ocr-build-kernel-hpcg-intel-Eager-mpi',
    'depends': ('ocr-build-x86-mpi',),
    'jobtype': 'ocr-build-app',
    'run-args': 'hpcg-intel-Eager x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'hpcg/refactored/ocr/intel-Eager',
                  'S_PATH': '${T_PATH}',
              }
}

job_ocr_run_kernel_hpcg_intel_Eager_mpi = {
    'name': 'ocr-run-kernel-hpcg-intel-Eager-mpi',
    'depends': ('ocr-build-kernel-hpcg-intel-Eager-mpi',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'hpcg-intel-Eager x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'PATH': '/opt/intel/tools/impi/5.1.1.109/intel64/bin:${PATH}',
                  'T_ARCH': 'x86',
                  'T_PATH': 'hpcg/refactored/ocr/intel-Eager',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

# job_ocr_verify_kernel_hpcg_intel_Eager_mpi = {
#     'name': 'ocr-verify-kernel-hpcg-intel-Eager-mpi',
#     'depends': ('ocr-run-kernel-hpcg-intel-Eager-mpi',),
#     'jobtype': 'ocr-verify-app-local',
#     'run-args': '-w -c 1',
#     'sandbox': ('inherit0',),
#     'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/hpcg-intel-Eager/ocr/install/x86-mpi'}
# }

# TG
job_ocr_build_kernel_hpcg_intel_Eager_tg = {
    'name': 'ocr-build-kernel-hpcg-intel-Eager-tg',
    'depends': ('ocr-build-builder-ce', 'ocr-build-builder-xe',
                'ocr-build-tg-ce', 'ocr-build-tg-xe', ),
    'jobtype': 'ocr-build-app-tg',
    'run-args': 'hpcg-intel-Eager tg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'hpcg/refactored/ocr/intel-Eager',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

job_ocr_run_kernel_hpcg_intel_Eager_tg = {
    'name': 'ocr-run-kernel-hpcg-intel-Eager-tg',
    'depends': ('ocr-build-kernel-hpcg-intel-Eager-tg',),
    'jobtype': 'ocr-run-app-tg-newlib',
    'run-args': 'hpcg-intel-Eager tg',
    'param-args': '-c ${WORKLOAD_INSTALL_ROOT}/tg/config.cfg',
    'timeout': 1800,
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'hpcg/refactored/ocr/intel-Eager',
                  'S_PATH': '${T_PATH}',
                  'WORKLOAD_ARGS': '1 1 1 16 2 0',
              }
}

# job_ocr_verify_kernel_hpcg_intel_Eager_tg = {
#     'name': 'ocr-verify-kernel-hpcg-intel-Eager-tg',
#     'depends': ('ocr-run-kernel-hpcg-intel-Eager-tg',),
#     'jobtype': 'ocr-verify-app-remote',
#     'run-args': '-w -c 1',
#     'sandbox': ('inherit0',),
#     'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/hpcg-intel-Eager/ocr/install/tg'}
# }

## hpcg-intel

job_ocr_build_kernel_hpcg_intel_x86 = {
    'name': 'ocr-build-kernel-hpcg-intel-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'hpcg-intel x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'hpcg/refactored/ocr/intel',
                  'S_PATH': '${T_PATH}',
              }
}

job_ocr_run_kernel_hpcg_intel_x86 = {
    'name': 'ocr-run-kernel-hpcg-intel-x86',
    'depends': ('ocr-build-kernel-hpcg-intel-x86',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'hpcg-intel x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'hpcg/refactored/ocr/intel',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

# job_ocr_verify_kernel_hpcg_intel_x86 = {
#     'name': 'ocr-verify-kernel-hpcg-intel-x86',
#     'depends': ('ocr-run-kernel-hpcg-intel-x86',),
#     'jobtype': 'ocr-verify-app-local',
#     'run-args': '-w -c 1',
#     'sandbox': ('inherit0',),
#     'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/hpcg-intel/ocr/install/x86'}
# }

# TODO: calibrate and add to baselineVal.py
# job_ocr_run_kernel_hpcg_intel_x86_remote_regression = {
#     'name': 'ocr-run-kernel-hpcg-intel-x86-remote-regression',
#     'depends': ('ocr-build-kernel-hpcg-intel-x86',),
#     'jobtype': 'ocr-run-app-regression',
#     'run-args': 'hpcg-intel x86 ocr-run-kernel-hpcg-intel-x86-remote-regression 10',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { 'T_ARCH': 'x86',
#                   'T_PATH': 'hpcg/refactored/ocr/intel',
#                   'S_PATH': '${T_PATH}',
#                   #'WORKLOAD_ARGS': '',
#               }
# }

# job_ocr_run_kernel_hpcg_intel_x86_remote_scaling = {
#     'name': 'ocr-run-kernel-hpcg-intel-x86-remote-scaling',
#     'depends': ('ocr-build-kernel-hpcg-intel-x86',),
#     'jobtype': 'ocr-run-app-scaling',
#     'run-args': 'hpcg-intel x86 ocr-run-kernel-hpcg-intel-x86-remote-scaling 10',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { 'T_ARCH': 'x86',
#                   'T_PATH': 'hpcg/refactored/ocr/intel',
#                   'S_PATH': '${T_PATH}',
#                   #'WORKLOAD_ARGS': '',
#               }
# }

# MPI version
job_ocr_build_kernel_hpcg_intel_mpi = {
    'name': 'ocr-build-kernel-hpcg-intel-mpi',
    'depends': ('ocr-build-x86-mpi',),
    'jobtype': 'ocr-build-app',
    'run-args': 'hpcg-intel x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'hpcg/refactored/ocr/intel',
                  'S_PATH': '${T_PATH}',
              }
}

job_ocr_run_kernel_hpcg_intel_mpi = {
    'name': 'ocr-run-kernel-hpcg-intel-mpi',
    'depends': ('ocr-build-kernel-hpcg-intel-mpi',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'hpcg-intel x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'PATH': '/opt/intel/tools/impi/5.1.1.109/intel64/bin:${PATH}',
                  'T_ARCH': 'x86',
                  'T_PATH': 'hpcg/refactored/ocr/intel',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

# job_ocr_verify_kernel_hpcg_intel_mpi = {
#     'name': 'ocr-verify-kernel-hpcg-intel-mpi',
#     'depends': ('ocr-run-kernel-hpcg-intel-mpi',),
#     'jobtype': 'ocr-verify-app-local',
#     'run-args': '-w -c 1',
#     'sandbox': ('inherit0',),
#     'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/hpcg-intel/ocr/install/x86-mpi'}
# }

# TG
job_ocr_build_kernel_hpcg_intel_tg = {
    'name': 'ocr-build-kernel-hpcg-intel-tg',
    'depends': ('ocr-build-builder-ce', 'ocr-build-builder-xe',
                'ocr-build-tg-ce', 'ocr-build-tg-xe', ),
    'jobtype': 'ocr-build-app-tg',
    'run-args': 'hpcg-intel tg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'hpcg/refactored/ocr/intel',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

job_ocr_run_kernel_hpcg_intel_tg = {
    'name': 'ocr-run-kernel-hpcg-intel-tg',
    'depends': ('ocr-build-kernel-hpcg-intel-tg',),
    'jobtype': 'ocr-run-app-tg-newlib',
    'run-args': 'hpcg-intel tg',
    'param-args': '-c ${WORKLOAD_INSTALL_ROOT}/tg/config.cfg',
    'timeout': 1800,
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'hpcg/refactored/ocr/intel',
                  'S_PATH': '${T_PATH}',
                  'WORKLOAD_ARGS': '1 1 1 16 2 0',
              }
}

# job_ocr_verify_kernel_hpcg_intel_tg = {
#     'name': 'ocr-verify-kernel-hpcg-intel-tg',
#     'depends': ('ocr-run-kernel-hpcg-intel-tg',),
#     'jobtype': 'ocr-verify-app-remote',
#     'run-args': '-w -c 1',
#     'sandbox': ('inherit0',),
#     'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/hpcg-intel/ocr/install/tg'}
# }

## CoMD-intel-chandra-tiled

job_ocr_build_kernel_CoMD_intel_chandra_tiled_x86 = {
    'name': 'ocr-build-kernel-CoMD-intel-chandra-tiled-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'CoMD-intel-chandra-tiled x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'CoMD/refactored/ocr/intel-chandra-tiled',
                  'S_PATH': '${T_PATH}',
              }
}

job_ocr_run_kernel_CoMD_intel_chandra_tiled_x86 = {
    'name': 'ocr-run-kernel-CoMD-intel-chandra-tiled-x86',
    'depends': ('ocr-build-kernel-CoMD-intel-chandra-tiled-x86',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'CoMD-intel-chandra-tiled x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'CoMD/refactored/ocr/intel-chandra-tiled',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

# job_ocr_verify_kernel_CoMD_intel_chandra_tiled_x86 = {
#     'name': 'ocr-verify-kernel-CoMD-intel-chandra-tiled-x86',
#     'depends': ('ocr-run-kernel-CoMD-intel-chandra-tiled-x86',),
#     'jobtype': 'ocr-verify-app-local',
#     'run-args': '-w -c 1',
#     'sandbox': ('inherit0',),
#     'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/CoMD-intel-chandra-tiled/ocr/install/x86'}
# }

# TODO: calibrate and add to baselineVal.py
# job_ocr_run_kernel_CoMD_intel_chandra_tiled_x86_remote_regression = {
#     'name': 'ocr-run-kernel-CoMD-intel-chandra-tiled-x86-remote-regression',
#     'depends': ('ocr-build-kernel-CoMD-intel-chandra-tiled-x86',),
#     'jobtype': 'ocr-run-app-regression',
#     'run-args': 'CoMD-intel-chandra-tiled x86 ocr-run-kernel-CoMD-intel-chandra-tiled-x86-remote-regression 10',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { 'T_ARCH': 'x86',
#                   'T_PATH': 'CoMD/refactored/ocr/intel-chandra-tiled',
#                   'S_PATH': '${T_PATH}',
#                   #'WORKLOAD_ARGS': '',
#               }
# }

# job_ocr_run_kernel_CoMD_intel_chandra_tiled_x86_remote_scaling = {
#     'name': 'ocr-run-kernel-CoMD-intel-chandra-tiled-x86-remote-scaling',
#     'depends': ('ocr-build-kernel-CoMD-intel-chandra-tiled-x86',),
#     'jobtype': 'ocr-run-app-scaling',
#     'run-args': 'CoMD-intel-chandra-tiled x86 ocr-run-kernel-CoMD-intel-chandra-tiled-x86-remote-scaling 10',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { 'T_ARCH': 'x86',
#                   'T_PATH': 'CoMD/refactored/ocr/intel-chandra-tiled',
#                   'S_PATH': '${T_PATH}',
#                   #'WORKLOAD_ARGS': '',
#               }
# }

# MPI version
job_ocr_build_kernel_CoMD_intel_chandra_tiled_mpi = {
    'name': 'ocr-build-kernel-CoMD-intel-chandra-tiled-mpi',
    'depends': ('ocr-build-x86-mpi',),
    'jobtype': 'ocr-build-app',
    'run-args': 'CoMD-intel-chandra-tiled x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'CoMD/refactored/ocr/intel-chandra-tiled',
                  'S_PATH': '${T_PATH}',
              }
}

job_ocr_run_kernel_CoMD_intel_chandra_tiled_mpi = {
    'name': 'ocr-run-kernel-CoMD-intel-chandra-tiled-mpi',
    'depends': ('ocr-build-kernel-CoMD-intel-chandra-tiled-mpi',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'CoMD-intel-chandra-tiled x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'PATH': '/opt/intel/tools/impi/5.1.1.109/intel64/bin:${PATH}',
                  'T_ARCH': 'x86',
                  'T_PATH': 'CoMD/refactored/ocr/intel-chandra-tiled',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

# job_ocr_verify_kernel_CoMD_intel_chandra_tiled_mpi = {
#     'name': 'ocr-verify-kernel-CoMD-intel-chandra-tiled-mpi',
#     'depends': ('ocr-run-kernel-CoMD-intel-chandra-tiled-mpi',),
#     'jobtype': 'ocr-verify-app-local',
#     'run-args': '-w -c 1',
#     'sandbox': ('inherit0',),
#     'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/CoMD-intel-chandra-tiled/ocr/install/x86-mpi'}
# }

# TG
job_ocr_build_kernel_CoMD_intel_chandra_tiled_tg = {
    'name': 'ocr-build-kernel-CoMD-intel-chandra-tiled-tg',
    'depends': ('ocr-build-builder-ce', 'ocr-build-builder-xe',
                'ocr-build-tg-ce', 'ocr-build-tg-xe', ),
    'jobtype': 'ocr-build-app-tg',
    'run-args': 'CoMD-intel-chandra-tiled tg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'CoMD/refactored/ocr/intel-chandra-tiled',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

job_ocr_run_kernel_CoMD_intel_chandra_tiled_tg = {
    'name': 'ocr-run-kernel-CoMD-intel-chandra-tiled-tg',
    'depends': ('ocr-build-kernel-CoMD-intel-chandra-tiled-tg',),
    'jobtype': 'ocr-run-app-tg-newlib',
    'run-args': 'CoMD-intel-chandra-tiled tg',
    'param-args': '-c ${WORKLOAD_INSTALL_ROOT}/tg/config.cfg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'CoMD/refactored/ocr/intel-chandra-tiled',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

# job_ocr_verify_kernel_CoMD_intel_chandra_tiled_tg = {
#     'name': 'ocr-verify-kernel-CoMD-intel-chandra-tiled-tg',
#     'depends': ('ocr-run-kernel-CoMD-intel-chandra-tiled-tg',),
#     'jobtype': 'ocr-verify-app-remote',
#     'run-args': '-w -c 1',
#     'sandbox': ('inherit0',),
#     'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/CoMD-intel-chandra-tiled/ocr/install/tg'}
# }

## nqueens-ocr

job_ocr_build_kernel_nqueens_ocr_x86 = {
    'name': 'ocr-build-kernel-nqueens-ocr-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'nqueens-ocr x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'nqueens/refactored/ocr',
                  'S_PATH': '${T_PATH}',
              }
}

job_ocr_run_kernel_nqueens_ocr_x86 = {
    'name': 'ocr-run-kernel-nqueens-ocr-x86',
    'depends': ('ocr-build-kernel-nqueens-ocr-x86',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'nqueens-ocr x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'nqueens/refactored/ocr',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

# job_ocr_verify_kernel_nqueens_ocr_x86 = {
#     'name': 'ocr-verify-kernel-nqueens-ocr-x86',
#     'depends': ('ocr-run-kernel-nqueens-ocr-x86',),
#     'jobtype': 'ocr-verify-app-local',
#     'run-args': '-w -c 1',
#     'sandbox': ('inherit0',),
#     'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/nqueens-ocr/ocr/install/x86'}
# }

# TODO: calibrate and add to baselineVal.py
# job_ocr_run_kernel_nqueens_ocr_x86_remote_regression = {
#     'name': 'ocr-run-kernel-nqueens-ocr-x86-remote-regression',
#     'depends': ('ocr-build-kernel-nqueens-ocr-x86',),
#     'jobtype': 'ocr-run-app-regression',
#     'run-args': 'nqueens-ocr x86 ocr-run-kernel-nqueens-ocr-x86-remote-regression 10',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { 'T_ARCH': 'x86',
#                   'T_PATH': 'nqueens/refactored/ocr',
#                   'S_PATH': '${T_PATH}',
#                   #'WORKLOAD_ARGS': '',
#               }
# }

# job_ocr_run_kernel_nqueens_ocr_x86_remote_scaling = {
#     'name': 'ocr-run-kernel-nqueens-ocr-x86-remote-scaling',
#     'depends': ('ocr-build-kernel-nqueens-ocr-x86',),
#     'jobtype': 'ocr-run-app-scaling',
#     'run-args': 'nqueens-ocr x86 ocr-run-kernel-nqueens-ocr-x86-remote-scaling 10',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { 'T_ARCH': 'x86',
#                   'T_PATH': 'nqueens/refactored/ocr',
#                   'S_PATH': '${T_PATH}',
#                   #'WORKLOAD_ARGS': '',
#               }
# }

## Stencil2D-intel-channelEVTs

job_ocr_build_kernel_Stencil2D_intel_channelEVTs_x86 = {
    'name': 'ocr-build-kernel-Stencil2D-intel-channelEVTs-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'Stencil2D-intel-channelEVTs x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'Stencil2D/refactored/ocr/intel-channelEVTs',
                  'S_PATH': '${T_PATH}',
              }
}

job_ocr_run_kernel_Stencil2D_intel_channelEVTs_x86 = {
    'name': 'ocr-run-kernel-Stencil2D-intel-channelEVTs-x86',
    'depends': ('ocr-build-kernel-Stencil2D-intel-channelEVTs-x86',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'Stencil2D-intel-channelEVTs x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'Stencil2D/refactored/ocr/intel-channelEVTs',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

# job_ocr_verify_kernel_Stencil2D_intel_channelEVTs_x86 = {
#     'name': 'ocr-verify-kernel-Stencil2D-intel-channelEVTs-x86',
#     'depends': ('ocr-run-kernel-Stencil2D-intel-channelEVTs-x86',),
#     'jobtype': 'ocr-verify-app-local',
#     'run-args': '-w -c 1',
#     'sandbox': ('inherit0',),
#     'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/Stencil2D-intel-channelEVTs/ocr/install/x86'}
# }

# TODO: calibrate and add to baselineVal.py
# job_ocr_run_kernel_Stencil2D_intel_channelEVTs_x86_remote_regression = {
#     'name': 'ocr-run-kernel-Stencil2D-intel-channelEVTs-x86-remote-regression',
#     'depends': ('ocr-build-kernel-Stencil2D-intel-channelEVTs-x86',),
#     'jobtype': 'ocr-run-app-regression',
#     'run-args': 'Stencil2D-intel-channelEVTs x86 ocr-run-kernel-Stencil2D-intel-channelEVTs-x86-remote-regression 10',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { 'T_ARCH': 'x86',
#                   'T_PATH': 'Stencil2D/refactored/ocr/intel-channelEVTs',
#                   'S_PATH': '${T_PATH}',
#                   'WORKLOAD_ARGS': '2048 32 1000',
#               }
# }

# job_ocr_run_kernel_Stencil2D_intel_channelEVTs_x86_remote_scaling = {
#     'name': 'ocr-run-kernel-Stencil2D-intel-channelEVTs-x86-remote-scaling',
#     'depends': ('ocr-build-kernel-Stencil2D-intel-channelEVTs-x86',),
#     'jobtype': 'ocr-run-app-scaling',
#     'run-args': 'Stencil2D-intel-channelEVTs x86 ocr-run-kernel-Stencil2D-intel-channelEVTs-x86-remote-scaling 10',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { 'T_ARCH': 'x86',
#                   'T_PATH': 'Stencil2D/refactored/ocr/intel-channelEVTs',
#                   'S_PATH': '${T_PATH}',
#                   'WORKLOAD_ARGS': '2048 32 1000',
#               }
# }

# MPI version
job_ocr_build_kernel_Stencil2D_intel_channelEVTs_mpi = {
    'name': 'ocr-build-kernel-Stencil2D-intel-channelEVTs-mpi',
    'depends': ('ocr-build-x86-mpi',),
    'jobtype': 'ocr-build-app',
    'run-args': 'Stencil2D-intel-channelEVTs x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'Stencil2D/refactored/ocr/intel-channelEVTs',
                  'S_PATH': '${T_PATH}',
              }
}

job_ocr_run_kernel_Stencil2D_intel_channelEVTs_mpi = {
    'name': 'ocr-run-kernel-Stencil2D-intel-channelEVTs-mpi',
    'depends': ('ocr-build-kernel-Stencil2D-intel-channelEVTs-mpi',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'Stencil2D-intel-channelEVTs x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'PATH': '/opt/intel/tools/impi/5.1.1.109/intel64/bin:${PATH}',
                  'T_ARCH': 'x86',
                  'T_PATH': 'Stencil2D/refactored/ocr/intel-channelEVTs',
                  'S_PATH': '${T_PATH}',
                  'WORKLOAD_ARGS': '2048 32 1000',
              }
}

# job_ocr_verify_kernel_Stencil2D_intel_channelEVTs_mpi = {
#     'name': 'ocr-verify-kernel-Stencil2D-intel-channelEVTs-mpi',
#     'depends': ('ocr-run-kernel-Stencil2D-intel-channelEVTs-mpi',),
#     'jobtype': 'ocr-verify-app-local',
#     'run-args': '-w -c 1',
#     'sandbox': ('inherit0',),
#     'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/Stencil2D-intel-channelEVTs/ocr/install/x86-mpi'}
# }

# TG
job_ocr_build_kernel_Stencil2D_intel_channelEVTs_tg = {
    'name': 'ocr-build-kernel-Stencil2D-intel-channelEVTs-tg',
    'depends': ('ocr-build-builder-ce', 'ocr-build-builder-xe',
                'ocr-build-tg-ce', 'ocr-build-tg-xe', ),
    'jobtype': 'ocr-build-app-tg',
    'run-args': 'Stencil2D-intel-channelEVTs tg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'Stencil2D/refactored/ocr/intel-channelEVTs',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

job_ocr_run_kernel_Stencil2D_intel_channelEVTs_tg = {
    'name': 'ocr-run-kernel-Stencil2D-intel-channelEVTs-tg',
    'depends': ('ocr-build-kernel-Stencil2D-intel-channelEVTs-tg',),
    'jobtype': 'ocr-run-app-tg-newlib',
    'run-args': 'Stencil2D-intel-channelEVTs tg',
    'param-args': '-c ${WORKLOAD_INSTALL_ROOT}/tg/config.cfg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'Stencil2D/refactored/ocr/intel-channelEVTs',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

# job_ocr_verify_kernel_Stencil2D_intel_channelEVTs_tg = {
#     'name': 'ocr-verify-kernel-Stencil2D-intel-channelEVTs-tg',
#     'depends': ('ocr-run-kernel-Stencil2D-intel-channelEVTs-tg',),
#     'jobtype': 'ocr-verify-app-remote',
#     'run-args': '-w -c 1',
#     'sandbox': ('inherit0',),
#     'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/Stencil2D-intel-channelEVTs/ocr/install/tg'}
# }

## reduction-intel

job_ocr_build_kernel_reduction_intel_x86 = {
    'name': 'ocr-build-kernel-reduction-intel-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'reduction-intel x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'reduction/refactored/ocr/intel',
                  'S_PATH': '${T_PATH}',
              }
}

job_ocr_run_kernel_reduction_intel_x86 = {
    'name': 'ocr-run-kernel-reduction-intel-x86',
    'depends': ('ocr-build-kernel-reduction-intel-x86',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'reduction-intel x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'reduction/refactored/ocr/intel',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

# job_ocr_verify_kernel_reduction_intel_x86 = {
#     'name': 'ocr-verify-kernel-reduction-intel-x86',
#     'depends': ('ocr-run-kernel-reduction-intel-x86',),
#     'jobtype': 'ocr-verify-app-local',
#     'run-args': '-w -c 1',
#     'sandbox': ('inherit0',),
#     'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/reduction-intel/ocr/install/x86'}
# }

# TODO: calibrate and add to baselineVal.py
# job_ocr_run_kernel_reduction_intel_x86_remote_regression = {
#     'name': 'ocr-run-kernel-reduction-intel-x86-remote-regression',
#     'depends': ('ocr-build-kernel-reduction-intel-x86',),
#     'jobtype': 'ocr-run-app-regression',
#     'run-args': 'reduction-intel x86 ocr-run-kernel-reduction-intel-x86-remote-regression 10',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { 'T_ARCH': 'x86',
#                   'T_PATH': 'reduction/refactored/ocr/intel',
#                   'S_PATH': '${T_PATH}',
#                   #'WORKLOAD_ARGS': '',
#               }
# }

# job_ocr_run_kernel_reduction_intel_x86_remote_scaling = {
#     'name': 'ocr-run-kernel-reduction-intel-x86-remote-scaling',
#     'depends': ('ocr-build-kernel-reduction-intel-x86',),
#     'jobtype': 'ocr-run-app-scaling',
#     'run-args': 'reduction-intel x86 ocr-run-kernel-reduction-intel-x86-remote-scaling 10',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { 'T_ARCH': 'x86',
#                   'T_PATH': 'reduction/refactored/ocr/intel',
#                   'S_PATH': '${T_PATH}',
#                   #'WORKLOAD_ARGS': '',
#               }
# }

# MPI version
job_ocr_build_kernel_reduction_intel_mpi = {
    'name': 'ocr-build-kernel-reduction-intel-mpi',
    'depends': ('ocr-build-x86-mpi',),
    'jobtype': 'ocr-build-app',
    'run-args': 'reduction-intel x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'reduction/refactored/ocr/intel',
                  'S_PATH': '${T_PATH}',
              }
}

job_ocr_run_kernel_reduction_intel_mpi = {
    'name': 'ocr-run-kernel-reduction-intel-mpi',
    'depends': ('ocr-build-kernel-reduction-intel-mpi',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'reduction-intel x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'PATH': '/opt/intel/tools/impi/5.1.1.109/intel64/bin:${PATH}',
                  'T_ARCH': 'x86',
                  'T_PATH': 'reduction/refactored/ocr/intel',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

# job_ocr_verify_kernel_reduction_intel_mpi = {
#     'name': 'ocr-verify-kernel-reduction-intel-mpi',
#     'depends': ('ocr-run-kernel-reduction-intel-mpi',),
#     'jobtype': 'ocr-verify-app-local',
#     'run-args': '-w -c 1',
#     'sandbox': ('inherit0',),
#     'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/reduction-intel/ocr/install/x86-mpi'}
# }

# TG
job_ocr_build_kernel_reduction_intel_tg = {
    'name': 'ocr-build-kernel-reduction-intel-tg',
    'depends': ('ocr-build-builder-ce', 'ocr-build-builder-xe',
                'ocr-build-tg-ce', 'ocr-build-tg-xe', ),
    'jobtype': 'ocr-build-app-tg',
    'run-args': 'reduction-intel tg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'reduction/refactored/ocr/intel',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

job_ocr_run_kernel_reduction_intel_tg = {
    'name': 'ocr-run-kernel-reduction-intel-tg',
    'depends': ('ocr-build-kernel-reduction-intel-tg',),
    'jobtype': 'ocr-run-app-tg-newlib',
    'run-args': 'reduction-intel tg',
    'param-args': '-c ${WORKLOAD_INSTALL_ROOT}/tg/config.cfg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'reduction/refactored/ocr/intel',
                  'S_PATH': '${T_PATH}',
                  #'WORKLOAD_ARGS': '',
              }
}

# job_ocr_verify_kernel_reduction_intel_tg = {
#     'name': 'ocr-verify-kernel-reduction-intel-tg',
#     'depends': ('ocr-run-kernel-reduction-intel-tg',),
#     'jobtype': 'ocr-verify-app-remote',
#     'run-args': '-w -c 1',
#     'sandbox': ('inherit0',),
#     'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/reduction-intel/ocr/install/tg'}
# }


#
# MPI LITE
#


# CoMD: MPI-Lite Intel
job_ocr_build_kernel_comdlite_x86_regression = {
    'name': 'ocr-build-kernel-comdlite-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'comdlite x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'CoMD/refactored/mpilite',
                  'S_PATH': '${T_PATH}/src-mpi',
                  'B_PATH': '${T_PATH}/build_and_run/mpilite/build',
                  'I_PATH': '${T_PATH}/build_and_run/mpilite/install',
                }
}

# the run takes 27 seconds
job_ocr_run_kernel_comdlite_x86_remote_regression = {
    'name': 'ocr-run-kernel-comdlite-x86-remote-regression',
    'depends': ('ocr-build-kernel-comdlite-x86',),
    'jobtype': 'ocr-run-app-regression',
    'run-args': 'comdlite x86 ocr-run-kernel-comdlite-x86-remote-regression 3',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'CoMD/refactored/mpilite',
                  'I_PATH': '${T_PATH}/build_and_run/mpilite/install',
                  'WORKLOAD_ARGS': '-r 4 -t 10  -e -i 2 -j 2 -k 1 -x 40 -y 40 -z 40 -d ${APPS_ROOT}/CoMD/datasets/pots',
                }
}

# hpgmg-lite
job_ocr_build_kernel_hpgmglite_x86_regression = {
    'name': 'ocr-build-kernel-hpgmglite-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'hpgmg x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'hpgmg/refactored/mpilite/intel'
                }
}

job_ocr_run_kernel_hpgmglite_x86_remote_regression = {
   'name': 'ocr-run-kernel-hpgmglite-x86-remote-regression',
   'depends': ('ocr-build-kernel-hpgmglite-x86',),
   'jobtype': 'ocr-run-app-regression',
   'run-args': 'hpgmg x86 ocr-run-kernel-hpgmglite-x86-remote-regression 5',
   'sandbox': ('shared','inherit0'),
   'env-vars': { 'T_ARCH': 'x86',
                 'T_PATH': 'hpgmg/refactored/mpilite/intel',
                 'WORKLOAD_ARGS': '-r 4 -t 1600 4 8',
               }
}

#job_ocr_run_kernel_hpgmglite_x86_remote_scaling = {
#    'name': 'ocr-run-kernel-hpgmglite-x86-remote-scaling',
#    'depends': ('ocr-build-kernel-hpgmgsdsc-x86',),
#    'jobtype': 'ocr-run-app-scaling',
#    'run-args': 'hpgmg x86 ocr-run-kernel-hpgmglite-x86-remote-scaling 5',
#    'sandbox': ('shared','inherit0'),
#    'env-vars': { 'T_ARCH': 'x86',
#                  'T_PATH': 'hpgmg/refactored/mpilite/intel',
#                  'WORKLOAD_ARGS': '4 8',
#                }
#}

# XSBench: Intel MPI-Lite
job_ocr_build_kernel_xsbenchlite_x86_regression = {
    'name': 'ocr-build-kernel-xsbenchlite-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'XSBenchlite x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'XSBench/refactored/mpilite/intel',
                  'S_PATH': '${T_PATH}/src',
                }
}

job_ocr_run_kernel_xsbenchlite_x86_remote_regression = {
    'name': 'ocr-run-kernel-xsbenchlite-x86-remote-regression',
    'depends': ('ocr-build-kernel-xsbenchlite-x86',),
    'jobtype': 'ocr-run-app-regression',
    'run-args': 'XSBenchlite x86 ocr-run-kernel-xsbenchlite-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'XSBench/refactored/mpilite/intel',
                  'WORKLOAD_ARGS': '-r 4 -s small -g 1000 -l 100000 -t 1',
                }
}


#
# TO DEPRECATE
#


# CoMD: sdsc ocr version2 - active implementation
job_ocr_build_kernel_comdsdsc2_x86_regression = {
    'name': 'ocr-build-kernel-comdsdsc2-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'comdsdsc2 x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'CoMD/refactored/ocr/sdsc2',
                }
}

job_ocr_run_kernel_comdsdsc2_x86_remote_regression = {
    'name': 'ocr-run-kernel-comdsdsc2-x86-remote-regression',
    'depends': ('ocr-build-kernel-comdsdsc2-x86',),
    'jobtype': 'ocr-run-app-regression',
    'run-args': 'comdsdsc2 x86 ocr-run-kernel-comdsdsc2-x86-remote-regression 3',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'CoMD/refactored/ocr/sdsc2',
                  'WORKLOAD_ARGS': '-x 15 -y 15 -z 15',
                }
}

job_ocr_run_kernel_comdsdsc2_x86_remote_scaling = {
    'name': 'ocr-run-kernel-comdsdsc2-x86-remote-scaling',
    'depends': ('ocr-build-kernel-comdsdsc2-x86',),
    'jobtype': 'ocr-run-app-scaling',
    'run-args': 'comdsdsc2 x86 ocr-run-kernel-comdsdsc2-x86-remote-scaling 3',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'CoMD/refactored/ocr/sdsc2',
                  'WORKLOAD_ARGS': '-x 15 -y 15 -z 15',
                }
}


# CoMD: sdsc ocr - inactive implementation
#job_ocr_build_kernel_comdsdsc_x86_regression = {
#    'name': 'ocr-build-kernel-comdsdsc-x86',
#    'depends': ('ocr-build-x86',),
#    'jobtype': 'ocr-build-app',
#    'run-args': 'comdsdsc x86',
#    'sandbox': ('shared','inherit0'),
#    'env-vars': { 'T_ARCH': 'x86',
#                  'T_PATH': 'CoMD/refactored/ocr/sdsc',
#                }
#}
#
#job_ocr_run_kernel_comdsdsc_x86_remote_regression = {
#    'name': 'ocr-run-kernel-comdsdsc-x86-remote-regression',
#    'depends': ('ocr-build-kernel-comdsdsc-x86',),
#    'jobtype': 'ocr-run-app-regression',
#    'run-args': 'comdsdsc x86 ocr-run-kernel-comdsdsc-x86-remote-regression 3',
#    'sandbox': ('shared','inherit0'),
#    'env-vars': { 'T_ARCH': 'x86',
#                  'T_PATH': 'CoMD/refactored/ocr/sdsc',
#                  'WORKLOAD_ARGS': '-x 5 -y 5 -z 5 -d ${APPS_ROOT}/CoMD/datasets/pots',
#                }
#}
#
#job_ocr_run_kernel_comdsdsc_x86_remote_scaling = {
#    'name': 'ocr-run-kernel-comdsdsc-x86-remote-scaling',
#    'depends': ('ocr-build-kernel-comdsdsc-x86',),
#    'jobtype': 'ocr-run-app-scaling',
#    'run-args': 'comdsdsc x86 ocr-run-kernel-comdsdsc-x86-remote-scaling 3',
#    'sandbox': ('shared','inherit0'),
#    'env-vars': { 'T_ARCH': 'x86',
#                  'T_PATH': 'CoMD/refactored/ocr/sdsc',
#                  'WORKLOAD_ARGS': '-x 5 -y 5 -z 5 -d ${APPS_ROOT}/CoMD/datasets/pots',
#                }
#}


# hpgmg: sdsc ocr
job_ocr_build_kernel_hpgmgsdsc_x86_regression = {
    'name': 'ocr-build-kernel-hpgmgsdsc-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'hpgmg x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'hpgmg/refactored/ocr/sdsc',
                }
}

job_ocr_run_kernel_hpgmgsdsc_x86_remote_regression = {
    'name': 'ocr-run-kernel-hpgmgsdsc-x86-remote-regression',
    'depends': ('ocr-build-kernel-hpgmgsdsc-x86',),
    'jobtype': 'ocr-run-app-regression',
    'run-args': 'hpgmg x86 ocr-run-kernel-hpgmgsdsc-x86-remote-regression 3',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'hpgmg/refactored/ocr/sdsc',
                  'WORKLOAD_ARGS': '4 8',
                }
}

job_ocr_build_kernel_SNAPlite_x86_regression = {
    'name': 'ocr-build-kernel-SNAPlite-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'snap_lite x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'SNAP/refactored/mpilite/intel',
                }
}

job_ocr_run_kernel_SNAPlite_x86_remote_regression = {
    'name': 'ocr-run-kernel-SNAPlite-x86-remote-regression',
    'depends': ('ocr-build-kernel-SNAPlite-x86',),
    'jobtype': 'ocr-run-app-regression',
    'run-args': 'snap_lite x86 ocr-run-kernel-SNAPlite-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'SNAP/refactored/mpilite/intel',
                  'WORKLOAD_ARGS': '-r 4 -t 1600  --fi ${APPS_ROOT}/${T_PATH}/fin04 --fo ${APPS_ROOT}/${T_PATH}/fout04',
                }
}

#job_ocr_run_kernel_SNAPlite_x86_remote_scaling = {
#    'name': 'ocr-run-kernel-SNAPlite-x86-remote-scaling',
#    'depends': ('ocr-build-kernel-SNAPlite-x86',),
#    'jobtype': 'ocr-run-app-scaling',
#    'run-args': 'snap_lite x86 ocr-run-kernel-SNAPlite-x86-remote-scaling 10',
#    'sandbox': ('shared','inherit0'),
#    'env-vars': { 'T_ARCH': 'x86',
#                  'T_PATH': 'SNAP/refactored/mpilite/intel',
#                  'WORKLOAD_ARGS': '-r 4 -t 1600',
#                }
#}

# tempest-lite
job_ocr_build_kernel_tempestsw2lite_x86_regression = {
    'name': 'ocr-build-kernel-tempestsw2lite-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'SWTest2 x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'tempest/refactored/mpilite/intel/test/shallowwater_sphere',
                }
}

job_ocr_run_kernel_tempestsw2lite = {
    'name': 'ocr-run-kernel-tempestsw2lite-x86-remote-regression',
    'depends': ('ocr-build-kernel-tempestsw2lite-x86',),
    'jobtype': 'ocr-run-app-regression',
    'run-args': 'SWTest2 x86 ocr-run-kernel-tempestsw2lite-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'tempest/refactored/mpilite/intel/test/shallowwater_sphere',
                  'WORKLOAD_ARGS': '-r 4 -t 1600',
                }
}

#job_ocr_run_kernel_tempestsw2lite_scaling = {
#    'name': 'ocr-run-kernel-tempestsw2lite-x86-remote-scaling',
#    'depends': ('ocr-build-kernel-tempestsw2lite-x86',),
#    'jobtype': 'ocr-run-app-scaling',
#    'run-args': 'SWTest2 x86 ocr-run-kernel-tempestsw2lite-x86-remote-scaling 10',
#    'sandbox': ('shared','inherit0'),
#    'env-vars': { 'APPS_LIBS_ROOT': '${APPS_ROOT}/libs/x86',
#                  'WORKLOAD_SRC': '${APPS_ROOT}/tempest/refactored/mpilite/intel/test/shallowwater_sphere',
#                  'WORKLOAD_ARGS': '-r 4 -t 1600',
#                  'WORKLOAD_INSTALL_ROOT': '${APPS_ROOT}/tempest/refactored/mpilite/intel/test/shallowwater_sphere/install'}
#}

# SAR: Intel ocr
job_ocr_build_kernel_sar_tiny_x86_regression = {
    'name': 'ocr-build-kernel-sar-tiny-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'sar x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'sar/ocr/tiny',
                }
}

job_ocr_run_kernel_sar_tiny_x86_remote_regression = {
    'name': 'ocr-run-kernel-sar-tiny-x86-remote-regression',
    'depends': ('ocr-build-kernel-sar-tiny-x86',),
    'jobtype': 'ocr-run-app-regression',
    'run-args': 'sar x86 ocr-run-kernel-sar-tiny-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'sar/ocr/tiny'
                }
}

job_ocr_run_kernel_sar_tiny_x86_remote_scaling = {
   'name': 'ocr-run-kernel-sar-tiny-x86-remote-scaling',
   'depends': ('ocr-build-kernel-sar-tiny-x86',),
   'jobtype': 'ocr-run-app-scaling',
   'run-args': 'sar x86 ocr-run-kernel-sar-tiny-x86-remote-scaling 10',
   'sandbox': ('shared','inherit0'),
   'env-vars': { 'T_ARCH': 'x86',
                 'T_PATH': 'sar/ocr/tiny'
               }
}

# # LULESH 2.0: PNNL CnC-OCR
# job_cnc_ocr_gen_lulesh2pnnl_x86_regression = {
#     'name': 'cnc-ocr-gen-lulesh2pnnl-x86',
#     'depends': ('cnc-ocr-bootstrap-x86',),
#     'jobtype': 'cnc-ocr-app-gen',
#     'run-args': '${WORKLOAD_SRC} regression',
#     'sandbox': ('shared', 'inherit0'),
#     'env-vars': { 'WORKLOAD_SRC': '${APPS_ROOT}/lulesh-2.0.3/refactored/cnc-ocr/pnnl/per-element', }
# }

# job_cnc_ocr_build_kernel_lulesh2pnnl_x86_regression = {
#     'name': 'cnc-ocr-build-kernel-lulesh2pnnl-x86',
#     'depends': ('cnc-ocr-gen-lulesh2pnnl-x86',),
#     'jobtype': 'cnc-ocr-app-build',
#     'run-args': '${WORKLOAD_SRC} regression',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { 'WORKLOAD_SRC': '${APPS_ROOT}/lulesh-2.0.3/refactored/cnc-ocr/pnnl/per-element',
#                   'WORKLOAD_BUILD_ROOT': '${APPS_ROOT}/lulesh-2.0.3/refactored/cnc-ocr/pnnl/per-element/build',
#                   'WORKLOAD_INSTALL_ROOT': '${APPS_ROOT}/lulesh-2.0.3/refactored/cnc-ocr/pnnl/per-element/install' }
# }

# job_ocr_run_kernel_lulesh2pnnl_x86_remote_regression = {
#     'name': 'ocr-run-kernel-lulesh2pnnl-x86-remote-regression',
#     'depends': ('cnc-ocr-build-kernel-lulesh2pnnl-x86',),
#     'jobtype': 'ocr-run-app-regression',
#     'keywords': ('cnc-ocr',),
#     'run-args': 'lulesh-2.0.3 x86 ocr-run-kernel-lulesh2pnnl-x86-remote-regression 10',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { # TODO: This is a problem 'XSTACK_ROOT': '${JJOB_SHARED_HOME}/xstack',
#                   'APPS_LIBS_ROOT': '${APPS_ROOT}/libs/x86',
#                   'WORKLOAD_SRC': '${APPS_ROOT}/lulesh-2.0.3/refactored/cnc-ocr/pnnl/per-element',
#                   'WORKLOAD_BUILD_ROOT': '${APPS_ROOT}/lulesh-2.0.3/refactored/cnc-ocr/pnnl/per-element/build',
#                   'WORKLOAD_INSTALL_ROOT': '${APPS_ROOT}/lulesh-2.0.3/refactored/cnc-ocr/pnnl/per-element/install' }
# }

# # HPGMG: PNNL CnC-OCR
# job_cnc_ocr_gen_hpgmg_pnnl_x86_regression = {
#     'name': 'cnc-ocr-gen-hpgmg-pnnl-x86',
#     'depends': ('cnc-ocr-bootstrap-x86',),
#     'jobtype': 'cnc-ocr-app-gen',
#     'run-args': '${WORKLOAD_SRC} regression',
#     'sandbox': ('shared', 'inherit0'),
#     'env-vars': { 'WORKLOAD_SRC': '${APPS_ROOT}/hpgmg/refactored/cnc/pnnl', }
# }

# job_cnc_ocr_build_kernel_hpgmg_pnnl_x86_regression = {
#     'name': 'cnc-ocr-build-kernel-hpgmg-pnnl-x86',
#     'depends': ('cnc-ocr-gen-hpgmg-pnnl-x86',),
#     'jobtype': 'cnc-ocr-app-build',
#     'run-args': '${WORKLOAD_SRC} regression',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { 'WORKLOAD_SRC': '${APPS_ROOT}/hpgmg/refactored/cnc/pnnl',
#                   'WORKLOAD_BUILD_ROOT': '${APPS_ROOT}/hpgmg/refactored/cnc/pnnl/build',
#                   'WORKLOAD_INSTALL_ROOT': '${APPS_ROOT}/hpgmg/refactored/cnc/pnnl/install' }
# }

# job_ocr_run_kernel_hpgmg_pnnl_x86_remote_regression = {
#     'name': 'ocr-run-kernel-hpgmg-pnnl-x86-remote-regression',
#     'depends': ('cnc-ocr-build-kernel-hpgmg-pnnl-x86',),
#     'jobtype': 'ocr-run-app-regression',
#     'keywords': ('cnc-ocr',),
#     'run-args': 'hpgmg x86 ocr-run-kernel-hpgmg-pnnl-x86-remote-regression 10',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { # TODO: THIS IS A PROBLEM 'XSTACK_ROOT': '${JJOB_SHARED_HOME}/xstack',
#                   'APPS_LIBS_ROOT': '${APPS_ROOT}/libs/x86',
#                   'WORKLOAD_SRC': '${APPS_ROOT}/hpgmg/refactored/cnc/pnnl',
#                   'WORKLOAD_BUILD_ROOT': '${APPS_ROOT}/hpgmg/refactored/cnc/pnnl/build',
#                   'WORKLOAD_INSTALL_ROOT': '${APPS_ROOT}/hpgmg/refactored/cnc/pnnl/install' }
# }
