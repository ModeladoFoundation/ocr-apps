#!/usr/bin/env python

# CoMD: sdsc ocr
job_ocr_build_kernel_comdsdsc_x86_regression = {
    'name': 'ocr-build-kernel-comdsdsc-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'comdsdsc x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'CoMD/refactored/ocr/sdsc',
                }
}

job_ocr_run_kernel_comdsdsc_x86_remote_regression = {
    'name': 'ocr-run-kernel-comdsdsc-x86-remote-regression',
    'depends': ('ocr-build-kernel-comdsdsc-x86',),
    'jobtype': 'ocr-run-app-regression',
    'run-args': 'comdsdsc x86 ocr-run-kernel-comdsdsc-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'CoMD/refactored/ocr/sdsc',
                  'WORKLOAD_ARGS': '-x 5 -y 5 -z 5 -d ${APPS_ROOT}/CoMD/datasets/pots',
                }
}

job_ocr_run_kernel_comdsdsc_x86_remote_scaling = {
    'name': 'ocr-run-kernel-comdsdsc-x86-remote-scaling',
    'depends': ('ocr-build-kernel-comdsdsc-x86',),
    'jobtype': 'ocr-run-app-scaling',
    'run-args': 'comdsdsc x86 ocr-run-kernel-comdsdsc-x86-remote-scaling 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'CoMD/refactored/ocr/sdsc',
                  'WORKLOAD_ARGS': '-x 5 -y 5 -z 5 -d ${APPS_ROOT}/CoMD/datasets/pots',
                }
}

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
    'run-args': 'comdlite x86 ocr-run-kernel-comdlite-x86-remote-regression 10',
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
                 'T_PATH': 'hpgmg/refactored/ocr/sdsc',
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
#                  'T_PATH': 'hpgmg/refactored/ocr/sdsc',
#                  'WORKLOAD_ARGS': '4 8',
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
