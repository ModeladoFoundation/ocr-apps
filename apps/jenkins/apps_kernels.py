#!/usr/bin/env python

# Specific jobs

## FFT
job_ocr_build_kernel_fft_x86 = {
    'name': 'ocr-build-kernel-fft-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'fft x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'fft/ocr'
              }
}

job_ocr_run_kernel_fft_x86 = {
    'name': 'ocr-run-kernel-fft-x86',
    'depends': ('ocr-build-kernel-fft-x86',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'fft x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'fft/ocr'
              }
}

job_ocr_verify_kernel_fft_x86 = {
    'name': 'ocr-verify-kernel-fft-x86',
    'depends': ('ocr-run-kernel-fft-x86',),
    'jobtype': 'ocr-verify-app-local',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/fft/ocr/install/x86'}
}

job_ocr_run_kernel_fft_x86_remote_regression = {
    'name': 'ocr-run-kernel-fft-x86-remote-regression',
    'depends': ('ocr-build-kernel-fft-x86',),
    'jobtype': 'ocr-run-app-regression',
    'run-args': 'fft x86 ocr-run-kernel-fft-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'fft/ocr',
                  'WORKLOAD_ARGS': '20',
              }
}

job_ocr_run_kernel_fft_x86_remote_scaling = {
    'name': 'ocr-run-kernel-fft-x86-remote-scaling',
    'depends': ('ocr-build-kernel-fft-x86',),
    'jobtype': 'ocr-run-app-scaling',
    'run-args': 'fft x86 ocr-run-kernel-fft-x86-remote-scaling 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'fft/ocr',
                  'WORKLOAD_ARGS': '20',
              }
}

# MPI version
job_ocr_build_kernel_fft_mpi = {
    'name': 'ocr-build-kernel-fft-mpi',
    'depends': ('ocr-build-x86-mpi',),
    'jobtype': 'ocr-build-app',
    'run-args': 'fft x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'fft/ocr'
              }
}

job_ocr_run_kernel_fft_mpi = {
    'name': 'ocr-run-kernel-fft-mpi',
    'depends': ('ocr-build-kernel-fft-mpi',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'fft x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'PATH': '/opt/intel/tools/impi/5.1.1.109/intel64/bin:${PATH}',
                  'T_ARCH': 'x86',
                  'T_PATH': 'fft/ocr'
              }
}

job_ocr_verify_kernel_fft_mpi = {
    'name': 'ocr-verify-kernel-fft-mpi',
    'depends': ('ocr-run-kernel-fft-mpi',),
    'jobtype': 'ocr-verify-app-local',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/fft/ocr/install/x86-mpi'}
}

# Gasnet version
job_ocr_build_kernel_fft_gasnet = {
    'name': 'ocr-build-kernel-fft-gasnet',
    'depends': ('ocr-build-x86-gasnet',),
    'jobtype': 'ocr-build-app',
    'run-args': 'fft x86-gasnet',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'fft/ocr',
                  'MPI_ROOT': '/opt/intel/tools/impi/5.1.1.109/intel64',
                  'GASNET_ROOT': '/opt/rice/GASNet/1.24.0-impi',
                  'PATH': '${GASNET_ROOT}/bin:${MPI_ROOT}/bin:${PATH}',
                  'GASNET_CONDUIT': 'ibv',
                  'GASNET_TYPE': 'par',
                  'GASNET_EXTRA_LIBS': '-L/usr/lib64 -lrt -libverbs',
                  'CC': 'mpicc', # gasnet built with mpi
              }
}

job_ocr_run_kernel_fft_gasnet = {
    'name': 'ocr-run-kernel-fft-gasnet',
    'depends': ('ocr-build-kernel-fft-gasnet',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'fft x86-gasnet',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'fft/ocr',
                  'MPI_ROOT': '/opt/intel/tools/impi/5.1.1.109/intel64',
                  'GASNET_ROOT': '/opt/rice/GASNet/1.24.0-impi',
                  'PATH': '${GASNET_ROOT}/bin:${MPI_ROOT}/bin:${PATH}',
                  'GASNET_CONDUIT': 'ibv'}
}

job_ocr_verify_kernel_fft_gasnet = {
    'name': 'ocr-verify-kernel-fft-gasnet',
    'depends': ('ocr-run-kernel-fft-gasnet',),
    'jobtype': 'ocr-verify-app-local',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/fft/install/x86-gasnet'}
}

# TG-x86
job_ocr_build_kernel_fft_tgemul = {
    'name': 'ocr-build-kernel-fft-tgemul',
    'depends': ('ocr-build-tg-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'fft tg-x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'TG_INSTALL': '${JJOB_ENVDIR}',
                  'T_ARCH': 'x86',
                  'T_PATH': 'fft/ocr'
              }
}

job_ocr_run_kernel_fft_tgemul = {
    'name': 'ocr-run-kernel-fft-tgemul',
    'depends': ('ocr-build-kernel-fft-tgemul',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'fft tg-x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'fft/ocr',
                  'OCR_CONFIG': '${OCR_INSTALL}/share/ocr/config/tg-x86/mach-1block.cfg'
              }
}

job_ocr_verify_kernel_fft_tgemul = {
    'name': 'ocr-verify-kernel-fft-tgemul',
    'depends': ('ocr-run-kernel-fft-tgemul',),
    'jobtype': 'ocr-verify-app-local',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/fft/ocr/install/tg-x86'}
}

# TG
job_ocr_build_kernel_fft_tg = {
    'name': 'ocr-build-kernel-fft-tg',
    'depends': ('ocr-build-builder-ce', 'ocr-build-builder-xe',
                'ocr-build-tg-ce', 'ocr-build-tg-xe'),
    'jobtype': 'ocr-build-app-tg',
    'run-args': 'fft tg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'fft/ocr',
                  'WORKLOAD_ARGS': '9'}
}

job_ocr_run_kernel_fft_tg = {
    'name': 'ocr-run-kernel-fft-tg',
    'depends': ('ocr-build-kernel-fft-tg',),
    'jobtype': 'ocr-run-app-tg',
    'run-args': 'fft tg',
    'param-args': '-c ${WORKLOAD_INSTALL_ROOT}/tg/config.cfg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'fft/ocr',
              }
}

job_ocr_verify_kernel_fft_tg = {
    'name': 'ocr-verify-kernel-fft-tg',
    'depends': ('ocr-run-kernel-fft-tg',),
    'jobtype': 'ocr-verify-app-remote',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/fft/ocr/install/tg'}
}

## Cholesky
job_ocr_build_kernel_cholesky_x86 = {
    'name': 'ocr-build-kernel-cholesky-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'cholesky x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'cholesky/ocr'
              }
}

job_ocr_run_kernel_cholesky_x86 = {
    'name': 'ocr-run-kernel-cholesky-x86',
    'depends': ('ocr-build-kernel-cholesky-x86',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'cholesky x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'cholesky/ocr',
                  'WORKLOAD_ARGS': '--ds 1000 --ts 50 --fi ${APPS_ROOT}/cholesky/datasets/m_1000.in',
              }
}

job_ocr_verify_kernel_cholesky_x86 = {
    'name': 'ocr-verify-kernel-cholesky-x86',
    'depends': ('ocr-run-kernel-cholesky-x86',),
    'jobtype': 'ocr-verify-diff',
    'run-args': '${WORKLOAD_EXEC}/cholesky.out.txt ${APPS_ROOT}/cholesky/datasets/cholesky_out_1000.txt',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/cholesky/ocr/install/x86'}
}

job_ocr_run_kernel_cholesky_x86_remote_regression = {
    'name': 'ocr-run-kernel-cholesky-x86-remote-regression',
    'depends': ('ocr-build-kernel-cholesky-x86',),
    'jobtype': 'ocr-run-app-regression',
    'run-args': 'cholesky x86 ocr-run-kernel-cholesky-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'cholesky/ocr',
                  'WORKLOAD_ARGS': '--ds 1000 --ts 20 --fi ${APPS_ROOT}/cholesky/datasets/m_1000.in',
              }
}

job_ocr_run_kernel_cholesky_x86_remote_scaling = {
    'name': 'ocr-run-kernel-cholesky-x86-remote-scaling',
    'depends': ('ocr-build-kernel-cholesky-x86',),
    'jobtype': 'ocr-run-app-scaling',
    'run-args': 'cholesky x86 ocr-run-kernel-cholesky-x86-remote-scaling 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'cholesky/ocr',
                  'WORKLOAD_ARGS': '--ds 1000 --ts 20 --fi ${APPS_ROOT}/cholesky/datasets/m_1000.in',
              }
}

# MPI version
job_ocr_build_kernel_cholesky_mpi = {
    'name': 'ocr-build-kernel-cholesky-mpi',
    'depends': ('ocr-build-x86-mpi',),
    'jobtype': 'ocr-build-app',
    'run-args': 'cholesky x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'cholesky/ocr'
              }
}

job_ocr_run_kernel_cholesky_mpi = {
    'name': 'ocr-run-kernel-cholesky-mpi',
    'depends': ('ocr-build-kernel-cholesky-mpi',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'cholesky x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'PATH': '/opt/intel/tools/impi/5.1.1.109/intel64/bin:${PATH}',
                  'T_ARCH': 'x86',
                  'T_PATH': 'cholesky/ocr',
                  'WORKLOAD_ARGS': '--ds 1000 --ts 50 --fi ${APPS_ROOT}/cholesky/datasets/m_1000.in',
              }
}

job_ocr_verify_kernel_cholesky_mpi = {
    'name': 'ocr-verify-kernel-cholesky-mpi',
    'depends': ('ocr-run-kernel-cholesky-mpi',),
    'jobtype': 'ocr-verify-diff',
    'run-args': '${WORKLOAD_EXEC}/cholesky.out.txt ${APPS_ROOT}/cholesky/datasets/cholesky_out_1000.txt',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/cholesky/ocr/install/x86-mpi'}
}

# Gasnet version
job_ocr_build_kernel_cholesky_gasnet = {
    'name': 'ocr-build-kernel-cholesky-gasnet',
    'depends': ('ocr-build-x86-gasnet',),
    'jobtype': 'ocr-build-app',
    'run-args': 'cholesky x86-gasnet',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'cholesky/ocr',
                  'MPI_ROOT': '/opt/intel/tools/impi/5.1.1.109/intel64',
                  'GASNET_ROOT': '/opt/rice/GASNet/1.24.0-impi',
                  'PATH': '${GASNET_ROOT}/bin:${MPI_ROOT}/bin:${PATH}',
                  'GASNET_CONDUIT': 'ibv',
                  'GASNET_TYPE': 'par',
                  'GASNET_EXTRA_LIBS': '-L/usr/lib64 -lrt -libverbs',
                  'CC': 'mpicc', # gasnet built with mpi
              }
}

job_ocr_run_kernel_cholesky_gasnet = {
    'name': 'ocr-run-kernel-cholesky-gasnet',
    'depends': ('ocr-build-kernel-cholesky-gasnet',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'cholesky x86-gasnet',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'cholesky/ocr',
                  'MPI_ROOT': '/opt/intel/tools/impi/5.1.1.109/intel64',
                  'GASNET_ROOT': '/opt/rice/GASNet/1.24.0-impi',
                  'PATH': '${GASNET_ROOT}/bin:${MPI_ROOT}/bin:${PATH}',
                  'GASNET_CONDUIT': 'ibv',
                  'WORKLOAD_ARGS': '--ds 1000 --ts 50 --fi ${APPS_ROOT}/cholesky/datasets/m_1000.in',
              }
}

job_ocr_verify_kernel_cholesky_gasnet = {
    'name': 'ocr-verify-kernel-cholesky-gasnet',
    'depends': ('ocr-run-kernel-cholesky-gasnet',),
    'jobtype': 'ocr-verify-diff',
    'run-args': '${WORKLOAD_EXEC}/cholesky.out.txt ${APPS_ROOT}/cholesky/datasets/cholesky_out_1000.txt',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/cholesky/install/x86-gasnet'}
}

# TG-x86
job_ocr_build_kernel_cholesky_tgemul = {
    'name': 'ocr-build-kernel-cholesky-tgemul',
    'depends': ('ocr-build-tg-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'cholesky tg-x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'TG_INSTALL': '${JJOB_ENVDIR}',
                  'T_ARCH': 'x86',
                  'T_PATH': 'cholesky/ocr'
              }
}

job_ocr_run_kernel_cholesky_tgemul = {
    'name': 'ocr-run-kernel-cholesky-tgemul',
    'depends': ('ocr-build-kernel-cholesky-tgemul',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'cholesky tg-x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'cholesky/ocr',
                  'WORKLOAD_ARGS': '--ds 1000 --ts 50 --fi ${APPS_ROOT}/cholesky/datasets/m_1000.in',
                  'OCR_CONFIG': '${OCR_INSTALL}/share/ocr/config/tg-x86/mach-1block.cfg'
              }
}

job_ocr_verify_kernel_cholesky_tgemul = {
    'name': 'ocr-verify-kernel-cholesky-tgemul',
    'depends': ('ocr-run-kernel-cholesky-tgemul',),
    'jobtype': 'ocr-verify-diff',
    'run-args': '${WORKLOAD_EXEC}/cholesky.out.txt ${APPS_ROOT}/cholesky/datasets/cholesky_out_1000.txt',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/cholesky/ocr/install/tg-x86'}
}

# TG
job_ocr_build_kernel_cholesky_tg = {
    'name': 'ocr-build-kernel-cholesky-tg',
    'depends': ('ocr-build-builder-ce', 'ocr-build-builder-xe',
                'ocr-build-tg-ce', 'ocr-build-tg-xe'),
    'jobtype': 'ocr-build-app-tg',
    'run-args': 'cholesky tg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'cholesky/ocr',
                  'WORKLOAD_ARGS': '--ds 50 --ts 10 --fi ${APPS_ROOT}/cholesky/datasets/m_50.in',}
}

job_ocr_run_kernel_cholesky_tg = {
    'name': 'ocr-run-kernel-cholesky-tg',
    'depends': ('ocr-build-kernel-cholesky-tg',),
    'jobtype': 'ocr-run-app-tg',
    'run-args': 'cholesky tg',
    'param-args': '-c ${WORKLOAD_INSTALL_ROOT}/tg/config.cfg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'cholesky/ocr',
                  'WORKLOAD_ARGS': '--ds 50 --ts 10 --fi ${APPS_ROOT}/cholesky/datasets/m_50.in',
              }
}

job_ocr_verify_kernel_cholesky_tg = {
    'name': 'ocr-verify-kernel-cholesky-tg',
    'depends': ('ocr-run-kernel-cholesky-tg',),
    'jobtype': 'ocr-verify-diff',
    'run-args': '${WORKLOAD_EXEC}/cholesky.out.txt ${APPS_ROOT}/cholesky/datasets/cholesky_out_50.txt',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/cholesky/ocr/install/tg'}
}

## Fibonacci
# x86
job_ocr_build_kernel_fibonacci_x86 = {
    'name': 'ocr-build-kernel-fibonacci-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'fibonacci x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'fibonacci/ocr'
              }
}

job_ocr_run_kernel_fibonacci_x86 = {
    'name': 'ocr-run-kernel-fibonacci-x86',
    'depends': ('ocr-build-kernel-fibonacci-x86',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'fib x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'fibonacci/ocr'
              }
}

job_ocr_verify_kernel_fibonacci_x86 = {
    'name': 'ocr-verify-kernel-fibonacci-x86',
    'depends': ('ocr-run-kernel-fibonacci-x86',),
    'jobtype': 'ocr-verify-app-local',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/fibonacci/ocr/install/x86'}
}

job_ocr_run_kernel_fibonacci_x86_remote_regression = {
    'name': 'ocr-run-kernel-fibonacci-x86-remote-regression',
    'depends': ('ocr-build-kernel-fibonacci-x86',),
    'jobtype': 'ocr-run-app-regression',
    'run-args': 'fibonacci x86 ocr-run-kernel-fibonacci-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'fibonacci/ocr',
                  'WORKLOAD_ARGS': '20',
              }
}

job_ocr_run_kernel_fibonacci_x86_remote_scaling = {
    'name': 'ocr-run-kernel-fibonacci-x86-remote-scaling',
    'depends': ('ocr-build-kernel-fibonacci-x86',),
    'jobtype': 'ocr-run-app-scaling',
    'run-args': 'fibonacci x86 ocr-run-kernel-fibonacci-x86-remote-scaling 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'fibonacci/ocr',
                  'WORKLOAD_ARGS': '20',
              }
}

# MPI version
job_ocr_build_kernel_fibonacci_mpi = {
    'name': 'ocr-build-kernel-fibonacci-mpi',
    'depends': ('ocr-build-x86-mpi',),
    'jobtype': 'ocr-build-app',
    'run-args': 'fib x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'fibonacci/ocr'
              }
}

job_ocr_run_kernel_fibonacci_mpi = {
    'name': 'ocr-run-kernel-fibonacci-mpi',
    'depends': ('ocr-build-kernel-fibonacci-mpi',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'fib x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'PATH': '/opt/intel/tools/impi/5.1.1.109/intel64/bin:${PATH}',
                  'T_ARCH': 'x86',
                  'T_PATH': 'fibonacci/ocr'
              }
}

job_ocr_verify_kernel_fibonacci_mpi = {
    'name': 'ocr-verify-kernel-fibonacci-mpi',
    'depends': ('ocr-run-kernel-fibonacci-mpi',),
    'jobtype': 'ocr-verify-app-local',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/fibonacci/ocr/install/x86-mpi'}
}

# Gasnet version
job_ocr_build_kernel_fibonacci_gasnet = {
    'name': 'ocr-build-kernel-fibonacci-gasnet',
    'depends': ('ocr-build-x86-gasnet',),
    'jobtype': 'ocr-build-app',
    'run-args': 'fib x86-gasnet',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'fibonacci/ocr',
                  'MPI_ROOT': '/opt/intel/tools/impi/5.1.1.109/intel64',
                  'GASNET_ROOT': '/opt/rice/GASNet/1.24.0-impi',
                  'PATH': '${GASNET_ROOT}/bin:${MPI_ROOT}/bin:${PATH}',
                  'GASNET_CONDUIT': 'ibv',
                  'GASNET_TYPE': 'par',
                  'GASNET_EXTRA_LIBS': '-L/usr/lib64 -lrt -libverbs',
                  'CC': 'mpicc', # gasnet built with mpi
              }
}

job_ocr_run_kernel_fibonacci_gasnet = {
    'name': 'ocr-run-kernel-fibonacci-gasnet',
    'depends': ('ocr-build-kernel-fibonacci-gasnet',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'fib x86-gasnet',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'fibonacci/ocr',
                  'MPI_ROOT': '/opt/intel/tools/impi/5.1.1.109/intel64',
                  'GASNET_ROOT': '/opt/rice/GASNet/1.24.0-impi',
                  'PATH': '${GASNET_ROOT}/bin:${MPI_ROOT}/bin:${PATH}',
                  'GASNET_CONDUIT': 'ibv'}
}

job_ocr_verify_kernel_fibonacci_gasnet = {
    'name': 'ocr-verify-kernel-fibonacci-gasnet',
    'depends': ('ocr-run-kernel-fibonacci-gasnet',),
    'jobtype': 'ocr-verify-app-local',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/fibonacci/install/x86-gasnet'}
}

# TG-x86
job_ocr_build_kernel_fibonacci_tgemul = {
    'name': 'ocr-build-kernel-fibonacci-tgemul',
    'depends': ('ocr-build-tg-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'fib tg-x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'TG_INSTALL': '${JJOB_ENVDIR}',
                  'T_ARCH': 'x86',
                  'T_PATH': 'fibonacci/ocr'
              }
}

job_ocr_run_kernel_fibonacci_tgemul = {
    'name': 'ocr-run-kernel-fibonacci-tgemul',
    'depends': ('ocr-build-kernel-fibonacci-tgemul',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'fib tg-x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'fibonacci/ocr',
                  'OCR_CONFIG': '${OCR_INSTALL}/share/ocr/config/tg-x86/mach-1block.cfg'
              }
}

job_ocr_verify_kernel_fibonacci_tgemul = {
    'name': 'ocr-verify-kernel-fibonacci-tgemul',
    'depends': ('ocr-run-kernel-fibonacci-tgemul',),
    'jobtype': 'ocr-verify-app-local',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/fibonacci/ocr/install/tg-x86'}
}

# TG
job_ocr_build_kernel_fibonacci_tg = {
    'name': 'ocr-build-kernel-fibonacci-tg',
    'depends': ('ocr-build-builder-ce', 'ocr-build-builder-xe',
                'ocr-build-tg-ce', 'ocr-build-tg-xe'),
    'jobtype': 'ocr-build-app-tg',
    'run-args': 'fib tg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'fibonacci/ocr',
                  'WORKLOAD_ARGS': '9',
              }
}

job_ocr_run_kernel_fibonacci_tg = {
    'name': 'ocr-run-kernel-fibonacci-tg',
    'depends': ('ocr-build-kernel-fibonacci-tg',),
    'jobtype': 'ocr-run-app-tg',
    'run-args': 'fib tg',
    'param-args': '-c ${WORKLOAD_INSTALL_ROOT}/tg/config.cfg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'fibonacci/ocr',
              }
}

job_ocr_verify_kernel_fibonacci_tg = {
    'name': 'ocr-verify-kernel-fibonacci-tg',
    'depends': ('ocr-run-kernel-fibonacci-tg',),
    'jobtype': 'ocr-verify-app-remote',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/fibonacci/ocr/install/tg'}
}

## printf

job_ocr_build_kernel_printf_x86 = {
    'name': 'ocr-build-kernel-printf-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'printf x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'printf/ocr'
              }
}

job_ocr_run_kernel_printf_x86 = {
    'name': 'ocr-run-kernel-printf-x86',
    'depends': ('ocr-build-kernel-printf-x86',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'printf x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'printf/ocr'
              }
}

job_ocr_verify_kernel_printf_x86 = {
    'name': 'ocr-verify-kernel-printf-x86',
    'depends': ('ocr-run-kernel-printf-x86',),
    'jobtype': 'ocr-verify-app-local',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/printf/ocr/install/x86'}
}

job_ocr_run_kernel_printf_x86_remote_regression = {
    'name': 'ocr-run-kernel-printf-x86-remote-regression',
    'depends': ('ocr-build-kernel-printf-x86',),
    'jobtype': 'ocr-run-app-regression',
    'run-args': 'printf x86 ocr-run-kernel-printf-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'printf/ocr',
              }
}

job_ocr_run_kernel_printf_x86_remote_scaling = {
    'name': 'ocr-run-kernel-printf-x86-remote-scaling',
    'depends': ('ocr-build-kernel-printf-x86',),
    'jobtype': 'ocr-run-app-scaling',
    'run-args': 'printf x86 ocr-run-kernel-printf-x86-remote-scaling 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'printf/ocr',
              }
}

# MPI version
job_ocr_build_kernel_printf_mpi = {
    'name': 'ocr-build-kernel-printf-mpi',
    'depends': ('ocr-build-x86-mpi',),
    'jobtype': 'ocr-build-app',
    'run-args': 'printf x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'printf/ocr'
              }
}

job_ocr_run_kernel_printf_mpi = {
    'name': 'ocr-run-kernel-printf-mpi',
    'depends': ('ocr-build-kernel-printf-mpi',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'printf x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'PATH': '/opt/intel/tools/impi/5.1.1.109/intel64/bin:${PATH}',
                  'T_ARCH': 'x86',
                  'T_PATH': 'printf/ocr'
              }
}

job_ocr_verify_kernel_printf_mpi = {
    'name': 'ocr-verify-kernel-printf-mpi',
    'depends': ('ocr-run-kernel-printf-mpi',),
    'jobtype': 'ocr-verify-app-local',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/printf/ocr/install/x86-mpi'}
}

# Gasnet version
job_ocr_build_kernel_printf_gasnet = {
    'name': 'ocr-build-kernel-printf-gasnet',
    'depends': ('ocr-build-x86-gasnet',),
    'jobtype': 'ocr-build-app',
    'run-args': 'printf x86-gasnet',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'printf/ocr',
                  'MPI_ROOT': '/opt/intel/tools/impi/5.1.1.109/intel64',
                  'GASNET_ROOT': '/opt/rice/GASNet/1.24.0-impi',
                  'PATH': '${GASNET_ROOT}/bin:${MPI_ROOT}/bin:${PATH}',
                  'GASNET_CONDUIT': 'ibv',
                  'GASNET_TYPE': 'par',
                  'GASNET_EXTRA_LIBS': '-L/usr/lib64 -lrt -libverbs',
                  'CC': 'mpicc', # gasnet built with mpi
              }
}

job_ocr_run_kernel_printf_gasnet = {
    'name': 'ocr-run-kernel-printf-gasnet',
    'depends': ('ocr-build-kernel-printf-gasnet',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'printf x86-gasnet',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'printf/ocr',
                  'MPI_ROOT': '/opt/intel/tools/impi/5.1.1.109/intel64',
                  'GASNET_ROOT': '/opt/rice/GASNet/1.24.0-impi',
                  'PATH': '${GASNET_ROOT}/bin:${MPI_ROOT}/bin:${PATH}',
                  'GASNET_CONDUIT': 'ibv'}
}

job_ocr_verify_kernel_printf_gasnet = {
    'name': 'ocr-verify-kernel-printf-gasnet',
    'depends': ('ocr-run-kernel-printf-gasnet',),
    'jobtype': 'ocr-verify-app-local',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/printf/install/x86-gasnet'}
}

# TG-x86
job_ocr_build_kernel_printf_tgemul = {
    'name': 'ocr-build-kernel-printf-tgemul',
    'depends': ('ocr-build-tg-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'printf tg-x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'TG_INSTALL': '${JJOB_ENVDIR}',
                  'T_ARCH': 'x86',
                  'T_PATH': 'printf/ocr'
              }
}

job_ocr_run_kernel_printf_tgemul = {
    'name': 'ocr-run-kernel-printf-tgemul',
    'depends': ('ocr-build-kernel-printf-tgemul',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'printf tg-x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'printf/ocr',
                  'OCR_CONFIG': '${OCR_INSTALL}/share/ocr/config/tg-x86/mach-1block.cfg'
              }
}

job_ocr_verify_kernel_printf_tgemul = {
    'name': 'ocr-verify-kernel-printf-tgemul',
    'depends': ('ocr-run-kernel-printf-tgemul',),
    'jobtype': 'ocr-verify-app-local',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/printf/ocr/install/tg-x86'}
}

# TG
job_ocr_build_kernel_printf_tg = {
    'name': 'ocr-build-kernel-printf-tg',
    'depends': ('ocr-build-builder-ce', 'ocr-build-builder-xe',
                'ocr-build-tg-ce', 'ocr-build-tg-xe'),
    'jobtype': 'ocr-build-app-tg',
    'run-args': 'printf tg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'printf/ocr',
              }
}

job_ocr_run_kernel_printf_tg = {
    'name': 'ocr-run-kernel-printf-tg',
    'depends': ('ocr-build-kernel-printf-tg',),
    'jobtype': 'ocr-run-app-tg',
    'run-args': 'printf tg',
    'param-args': '-c ${WORKLOAD_INSTALL_ROOT}/tg/config.cfg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'printf/ocr',
              }
}

job_ocr_verify_kernel_printf_tg = {
    'name': 'ocr-verify-kernel-printf-tg',
    'depends': ('ocr-run-kernel-printf-tg',),
    'jobtype': 'ocr-verify-app-remote',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/printf/ocr/install/tg'}
}

## quicksort

job_ocr_build_kernel_quicksort_x86 = {
    'name': 'ocr-build-kernel-quicksort-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'quicksort x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'quicksort/ocr'
              }
}

job_ocr_run_kernel_quicksort_x86 = {
    'name': 'ocr-run-kernel-quicksort-x86',
    'depends': ('ocr-build-kernel-quicksort-x86',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'quicksort x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'quicksort/ocr'
              }
}

job_ocr_verify_kernel_quicksort_x86 = {
    'name': 'ocr-verify-kernel-quicksort-x86',
    'depends': ('ocr-run-kernel-quicksort-x86',),
    'jobtype': 'ocr-verify-app-local',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/quicksort/ocr/install/x86'}
}

job_ocr_run_kernel_quicksort_x86_remote_regression = {
    'name': 'ocr-run-kernel-quicksort-x86-remote-regression',
    'depends': ('ocr-build-kernel-quicksort-x86',),
    'jobtype': 'ocr-run-app-regression',
    'run-args': 'quicksort x86 ocr-run-kernel-quicksort-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'quicksort/ocr',
              }
}

job_ocr_run_kernel_quicksort_x86_remote_scaling = {
    'name': 'ocr-run-kernel-quicksort-x86-remote-scaling',
    'depends': ('ocr-build-kernel-quicksort-x86',),
    'jobtype': 'ocr-run-app-scaling',
    'run-args': 'quicksort x86 ocr-run-kernel-quicksort-x86-remote-scaling 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'quicksort/ocr',
              }
}

# MPI version
job_ocr_build_kernel_quicksort_mpi = {
    'name': 'ocr-build-kernel-quicksort-mpi',
    'depends': ('ocr-build-x86-mpi',),
    'jobtype': 'ocr-build-app',
    'run-args': 'quicksort x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'quicksort/ocr'
              }
}

job_ocr_run_kernel_quicksort_mpi = {
    'name': 'ocr-run-kernel-quicksort-mpi',
    'depends': ('ocr-build-kernel-quicksort-mpi',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'quicksort x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'PATH': '/opt/intel/tools/impi/5.1.1.109/intel64/bin:${PATH}',
                  'T_ARCH': 'x86',
                  'T_PATH': 'quicksort/ocr'
              }
}

job_ocr_verify_kernel_quicksort_mpi = {
    'name': 'ocr-verify-kernel-quicksort-mpi',
    'depends': ('ocr-run-kernel-quicksort-mpi',),
    'jobtype': 'ocr-verify-app-local',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/quicksort/ocr/install/x86-mpi'}
}

# Gasnet version
job_ocr_build_kernel_quicksort_gasnet = {
    'name': 'ocr-build-kernel-quicksort-gasnet',
    'depends': ('ocr-build-x86-gasnet',),
    'jobtype': 'ocr-build-app',
    'run-args': 'quicksort x86-gasnet',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'quicksort/ocr',
                  'MPI_ROOT': '/opt/intel/tools/impi/5.1.1.109/intel64',
                  'GASNET_ROOT': '/opt/rice/GASNet/1.24.0-impi',
                  'PATH': '${GASNET_ROOT}/bin:${MPI_ROOT}/bin:${PATH}',
                  'GASNET_CONDUIT': 'ibv',
                  'GASNET_TYPE': 'par',
                  'GASNET_EXTRA_LIBS': '-L/usr/lib64 -lrt -libverbs',
                  'CC': 'mpicc', # gasnet built with mpi
              }
}

job_ocr_run_kernel_quicksort_gasnet = {
    'name': 'ocr-run-kernel-quicksort-gasnet',
    'depends': ('ocr-build-kernel-quicksort-gasnet',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'quicksort x86-gasnet',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'quicksort/ocr',
                  'MPI_ROOT': '/opt/intel/tools/impi/5.1.1.109/intel64',
                  'GASNET_ROOT': '/opt/rice/GASNet/1.24.0-impi',
                  'PATH': '${GASNET_ROOT}/bin:${MPI_ROOT}/bin:${PATH}',
                  'GASNET_CONDUIT': 'ibv'}
}

job_ocr_verify_kernel_quicksort_gasnet = {
    'name': 'ocr-verify-kernel-quicksort-gasnet',
    'depends': ('ocr-run-kernel-quicksort-gasnet',),
    'jobtype': 'ocr-verify-app-local',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/quicksort/install/x86-gasnet'}
}

# TG-x86
job_ocr_build_kernel_quicksort_tgemul = {
    'name': 'ocr-build-kernel-quicksort-tgemul',
    'depends': ('ocr-build-tg-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'quicksort tg-x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'TG_INSTALL': '${JJOB_ENVDIR}',
                  'T_ARCH': 'x86',
                  'T_PATH': 'quicksort/ocr'
              }
}

job_ocr_run_kernel_quicksort_tgemul = {
    'name': 'ocr-run-kernel-quicksort-tgemul',
    'depends': ('ocr-build-kernel-quicksort-tgemul',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'quicksort tg-x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'quicksort/ocr',
                  'OCR_CONFIG': '${OCR_INSTALL}/share/ocr/config/tg-x86/mach-1block.cfg'
              }
}

job_ocr_verify_kernel_quicksort_tgemul = {
    'name': 'ocr-verify-kernel-quicksort-tgemul',
    'depends': ('ocr-run-kernel-quicksort-tgemul',),
    'jobtype': 'ocr-verify-app-local',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/quicksort/ocr/install/tg-x86'}
}

# TG
job_ocr_build_kernel_quicksort_tg = {
    'name': 'ocr-build-kernel-quicksort-tg',
    'depends': ('ocr-build-builder-ce', 'ocr-build-builder-xe',
                'ocr-build-tg-ce', 'ocr-build-tg-xe'),
    'jobtype': 'ocr-build-app-tg',
    'run-args': 'quicksort tg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'quicksort/ocr',
              }
}

job_ocr_run_kernel_quicksort_tg = {
    'name': 'ocr-run-kernel-quicksort-tg',
    'depends': ('ocr-build-kernel-quicksort-tg',),
    'jobtype': 'ocr-run-app-tg',
    'run-args': 'quicksort tg',
    'param-args': '-c ${WORKLOAD_INSTALL_ROOT}/tg/config.cfg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'quicksort/ocr',
              }
}

job_ocr_verify_kernel_quicksort_tg = {
    'name': 'ocr-verify-kernel-quicksort-tg',
    'depends': ('ocr-run-kernel-quicksort-tg',),
    'jobtype': 'ocr-verify-app-remote',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/quicksort/ocr/install/tg'}
}

## Smith-Waterman

job_ocr_build_kernel_smithwaterman_x86 = {
    'name': 'ocr-build-kernel-smithwaterman-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'smithwaterman x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'smithwaterman/ocr'
              }
}

job_ocr_run_kernel_smithwaterman_x86 = {
    'name': 'ocr-run-kernel-smithwaterman-x86',
    'depends': ('ocr-build-kernel-smithwaterman-x86',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'smithwaterman x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'smithwaterman/ocr',
                  'WORKLOAD_ARGS': '10 10 ${APPS_ROOT}/smithwaterman/datasets/string1-medium.txt  ${APPS_ROOT}/smithwaterman/datasets/string2-medium.txt ${APPS_ROOT}/smithwaterman/datasets/score-medium.txt',

              }
}

job_ocr_verify_kernel_smithwaterman_x86 = {
    'name': 'ocr-verify-kernel-smithwaterman-x86',
    'depends': ('ocr-run-kernel-smithwaterman-x86',),
    'jobtype': 'ocr-verify-app-local',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/smithwaterman/ocr/install/x86'}
}

job_ocr_run_kernel_smithwaterman_x86_remote_regression = {
    'name': 'ocr-run-kernel-smithwaterman-x86-remote-regression',
    'depends': ('ocr-build-kernel-smithwaterman-x86',),
    'jobtype': 'ocr-run-app-regression',
    'run-args': 'smithwaterman x86 ocr-run-kernel-smithwaterman-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'smithwaterman/ocr',
                  'WORKLOAD_ARGS': '10 10 ${APPS_ROOT}/smithwaterman/datasets/string1-medium-large.txt ${APPS_ROOT}/smithwaterman/datasets/string2-medium-large.txt ${APPS_ROOT}/smithwaterman/datasets/score-medium-large.txt',
              }
}

job_ocr_run_kernel_smithwaterman_x86_remote_scaling = {
    'name': 'ocr-run-kernel-smithwaterman-x86-remote-scaling',
    'depends': ('ocr-build-kernel-smithwaterman-x86',),
    'jobtype': 'ocr-run-app-scaling',
    'run-args': 'smithwaterman x86 ocr-run-kernel-smithwaterman-x86-remote-scaling 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'smithwaterman/ocr',
                  'WORKLOAD_ARGS': '10 10 ${APPS_ROOT}/smithwaterman/datasets/string1-medium-large.txt ${APPS_ROOT}/smithwaterman/datasets/string2-medium-large.txt ${APPS_ROOT}/smithwaterman/datasets/score-medium-large.txt',
              }
}

# MPI version
job_ocr_build_kernel_smithwaterman_mpi = {
    'name': 'ocr-build-kernel-smithwaterman-mpi',
    'depends': ('ocr-build-x86-mpi',),
    'jobtype': 'ocr-build-app',
    'run-args': 'smithwaterman x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'smithwaterman/ocr'
              }
}

job_ocr_run_kernel_smithwaterman_mpi = {
    'name': 'ocr-run-kernel-smithwaterman-mpi',
    'depends': ('ocr-build-kernel-smithwaterman-mpi',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'smithwaterman x86-mpi',
    'sandbox': ('inherit0',),
    'env-vars': { 'PATH': '/opt/intel/tools/impi/5.1.1.109/intel64/bin:${PATH}',
                  'T_ARCH': 'x86',
                  'T_PATH': 'smithwaterman/ocr',
                  'WORKLOAD_ARGS': '100 100 ${APPS_ROOT}/smithwaterman/datasets/string1-medium-large.txt  ${APPS_ROOT}/smithwaterman/datasets/string2-medium-large.txt ${APPS_ROOT}/smithwaterman/datasets/score-medium-large.txt',
              }
}

job_ocr_verify_kernel_smithwaterman_mpi = {
    'name': 'ocr-verify-kernel-smithwaterman-mpi',
    'depends': ('ocr-run-kernel-smithwaterman-mpi',),
    'jobtype': 'ocr-verify-app-local',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/smithwaterman/ocr/install/x86-mpi'}
}

# Gasnet version
job_ocr_build_kernel_smithwaterman_gasnet = {
    'name': 'ocr-build-kernel-smithwaterman-gasnet',
    'depends': ('ocr-build-x86-gasnet',),
    'jobtype': 'ocr-build-app',
    'run-args': 'smithwaterman x86-gasnet',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'smithwaterman/ocr',
                  'MPI_ROOT': '/opt/intel/tools/impi/5.1.1.109/intel64',
                  'GASNET_ROOT': '/opt/rice/GASNet/1.24.0-impi',
                  'PATH': '${GASNET_ROOT}/bin:${MPI_ROOT}/bin:${PATH}',
                  'GASNET_CONDUIT': 'ibv',
                  'GASNET_TYPE': 'par',
                  'GASNET_EXTRA_LIBS': '-L/usr/lib64 -lrt -libverbs',
                  'CC': 'mpicc', # gasnet built with mpi
              }
}

job_ocr_run_kernel_smithwaterman_gasnet = {
    'name': 'ocr-run-kernel-smithwaterman-gasnet',
    'depends': ('ocr-build-kernel-smithwaterman-gasnet',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'smithwaterman x86-gasnet',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'smithwaterman/ocr',
                  'MPI_ROOT': '/opt/intel/tools/impi/5.1.1.109/intel64',
                  'GASNET_ROOT': '/opt/rice/GASNet/1.24.0-impi',
                  'PATH': '${GASNET_ROOT}/bin:${MPI_ROOT}/bin:${PATH}',
                  'GASNET_CONDUIT': 'ibv',
                  'WORKLOAD_ARGS': '100 100 ${APPS_ROOT}/smithwaterman/datasets/string1-medium-large.txt  ${APPS_ROOT}/smithwaterman/datasets/string2-medium-large.txt ${APPS_ROOT}/smithwaterman/datasets/score-medium-large.txt',
              }
}

job_ocr_verify_kernel_smithwaterman_gasnet = {
    'name': 'ocr-verify-kernel-smithwaterman-gasnet',
    'depends': ('ocr-run-kernel-smithwaterman-gasnet',),
    'jobtype': 'ocr-verify-app-local',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/smithwaterman/install/x86-gasnet'}
}

# TG-x86
job_ocr_build_kernel_smithwaterman_tgemul = {
    'name': 'ocr-build-kernel-smithwaterman-tgemul',
    'depends': ('ocr-build-tg-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'smithwaterman tg-x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'TG_INSTALL': '${JJOB_ENVDIR}',
                  'T_ARCH': 'x86',
                  'T_PATH': 'smithwaterman/ocr'
              }
}

job_ocr_run_kernel_smithwaterman_tgemul = {
    'name': 'ocr-run-kernel-smithwaterman-tgemul',
    'depends': ('ocr-build-kernel-smithwaterman-tgemul',),
    'jobtype': 'ocr-run-app-nonregression',
    'run-args': 'smithwaterman tg-x86',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'smithwaterman/ocr',
                  'WORKLOAD_ARGS': '100 100 ${APPS_ROOT}/smithwaterman/datasets/string1-medium-large.txt  ${APPS_ROOT}/smithwaterman/datasets/string2-medium-large.txt ${APPS_ROOT}/smithwaterman/datasets/score-medium-large.txt',
                  'OCR_CONFIG': '${OCR_INSTALL}/share/ocr/config/tg-x86/mach-1block.cfg'
              }
}

job_ocr_verify_kernel_smithwaterman_tgemul = {
    'name': 'ocr-verify-kernel-smithwaterman-tgemul',
    'depends': ('ocr-run-kernel-smithwaterman-tgemul',),
    'jobtype': 'ocr-verify-app-local',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/smithwaterman/ocr/install/tg-x86'}
}

# TG
job_ocr_build_kernel_smithwaterman_tg = {
    'name': 'ocr-build-kernel-smithwaterman-tg',
    'depends': ('ocr-build-builder-ce', 'ocr-build-builder-xe',
                'ocr-build-tg-ce', 'ocr-build-tg-xe'),
    'jobtype': 'ocr-build-app-tg',
    'run-args': 'smithwaterman tg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'smithwaterman/ocr',
                  'WORKLOAD_ARGS': '10 10 ${APPS_ROOT}/smithwaterman/datasets/string1-medium.txt ${APPS_ROOT}/smithwaterman/datasets/string2-medium.txt ${APPS_ROOT}/smithwaterman/datasets/score-medium.txt',
                }
}

job_ocr_run_kernel_smithwaterman_tg = {
    'name': 'ocr-run-kernel-smithwaterman-tg',
    'depends': ('ocr-build-kernel-smithwaterman-tg',),
    'jobtype': 'ocr-run-app-tg',
    'run-args': 'smithwaterman tg',
    'param-args': '-c ${WORKLOAD_INSTALL_ROOT}/tg/config.cfg',
    'sandbox': ('inherit0',),
    'env-vars': { 'T_PATH': 'smithwaterman/ocr',
              }
}

job_ocr_verify_kernel_smithwaterman_tg = {
    'name': 'ocr-verify-kernel-smithwaterman-tg',
    'depends': ('ocr-run-kernel-smithwaterman-tg',),
    'jobtype': 'ocr-verify-app-remote',
    'run-args': '-w -c 1',
    'sandbox': ('inherit0',),
    'env-vars': { 'WORKLOAD_EXEC': '${APPS_ROOT}/smithwaterman/ocr/install/tg'}
}

# 2D Stencil (Chandra Martha)
job_ocr_build_kernel_Stencil2DChandra_x86_regression = {
    'name': 'ocr-build-kernel-Stencil2DChandra-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'stencil_1d x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'Stencil2D/refactored/ocr/intel-chandra',
              }
}

job_ocr_run_kernel_Stencil2DChandra_x86_remote_regression = {
    'name': 'ocr-run-kernel-Stencil2DChandra-x86-remote-regression',
    'depends': ('ocr-build-kernel-Stencil2DChandra-x86',),
    'jobtype': 'ocr-run-app-regression',
    'run-args': 'stencil_1d x86 ocr-run-kernel-Stencil2DChandra-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'Stencil2D/refactored/ocr/intel-chandra',
                  'WORKLOAD_ARGS': '2048 32 1000'
              }
}

job_ocr_run_kernel_Stencil2DChandra_x86_remote_scaling = {
    'name': 'ocr-run-kernel-Stencil2DChandra-x86-remote-scaling',
    'depends': ('ocr-build-kernel-Stencil2DChandra-x86',),
    'jobtype': 'ocr-run-app-scaling',
    'run-args': 'Stencil2DChandra x86 ocr-run-kernel-Stencil2DChandra-x86-remote-scaling 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'Stencil2D/refactored/ocr/intel-chandra',
                  'WORKLOAD_ARGS': '2048 32 1000'
              }
}


#
# TO DEPRECATE
#

# Triangle (David Scott)
job_ocr_build_kernel_triangle_x86 = {
    'name': 'ocr-build-kernel-triangle-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'triangle x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'triangle/refactored/ocr/intel',
              }
}

job_ocr_run_kernel_triangle_x86_remote_regression = {
    'name': 'ocr-run-kernel-triangle-x86-remote-regression',
    'depends': ('ocr-build-kernel-triangle-x86',),
    'jobtype': 'ocr-run-app-regression',
    'run-args': 'triangle x86 ocr-run-kernel-triangle-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'triangle/refactored/ocr/intel',
                  'WORKLOAD_ARGS': '',
              }
}

#job_ocr_run_kernel_triangle_x86_remote_scaling = {
#    'name': 'ocr-run-kernel-triangle-x86-remote-scaling',
#    'depends': ('ocr-build-kernel-triangle-x86',),
#    'jobtype': 'ocr-run-app-scaling',
#    'run-args': 'triangle x86 ocr-run-kernel-triangle-x86-remote-scaling 10',
#    'sandbox': ('shared','inherit0'),
#    'env-vars': { 'T_ARCH': 'x86',
#                  'T_PATH': 'triangle/refactored/ocr/intel',
#                  'WORKLOAD_ARGS': '',
#              }
#}

# p2p synch (David Scott)
job_ocr_build_kernel_p2p_x86 = {
    'name': 'ocr-build-kernel-p2p-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'p2p x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'p2p/refactored/ocr/intel',
              }
}

job_ocr_run_kernel_p2p_x86_remote_regression = {
    'name': 'ocr-run-kernel-p2p-x86-remote-regression',
    'depends': ('ocr-build-kernel-p2p-x86',),
    'jobtype': 'ocr-run-app-regression',
    'run-args': 'p2p x86 ocr-run-kernel-p2p-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'p2p/refactored/ocr/intel',
                  'WORKLOAD_ARGS': '',
              }
}

job_ocr_run_kernel_p2p_x86_remote_scaling = {
    'name': 'ocr-run-kernel-p2p-x86-remote-scaling',
    'depends': ('ocr-build-kernel-p2p-x86',),
    'jobtype': 'ocr-run-app-scaling',
    'run-args': 'p2p x86 ocr-run-kernel-p2p-x86-remote-scaling 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'p2p/refactored/ocr/intel',
                  'WORKLOAD_ARGS': '',
              }
}

# Global Sum (David Scott)
job_ocr_build_kernel_globalsum_x86_regression = {
    'name': 'ocr-build-kernel-globalsum-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'globalsum x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'globalsum/refactored/ocr/intel',
                  'CODE_TYPE': '2',
              }
}

job_ocr_run_kernel_globalsum_x86_remote_regression = {
    'name': 'ocr-run-kernel-globalsum-x86-remote-regression',
    'depends': ('ocr-build-kernel-globalsum-x86',),
    'jobtype': 'ocr-run-app-regression',
    'run-args': 'globalsum x86 ocr-run-kernel-globalsum-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'globalsum/refactored/ocr/intel',
                  'CODE_TYPE': '2',
                  'WORKLOAD_ARGS': '',
              }
}

job_ocr_run_kernel_globalsum_x86_remote_scaling = {
    'name': 'ocr-run-kernel-globalsum-x86-remote-scaling',
    'depends': ('ocr-build-kernel-globalsum-x86',),
    'jobtype': 'ocr-run-app-scaling',
    'run-args': 'globalsum x86 ocr-run-kernel-globalsum-x86-remote-scaling 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'globalsum/refactored/ocr/intel',
                  'CODE_TYPE': '2',
                  'WORKLOAD_ARGS': '',
              }
}

# # 1D Stencil (David Scott)
# job_ocr_build_kernel_Stencil1DDavid_x86_regression = {
#     'name': 'ocr-build-kernel-Stencil1DDavid-x86',
#     'depends': ('ocr-build-x86',),
#     'jobtype': 'ocr-build-app',
#     'run-args': 'stencil x86',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { 'T_ARCH': 'x86',
#                   'T_PATH': 'Stencil1D/refactored/ocr/intel-david',
#                   'CODE_TYPE': '2',
#               }
# }

# job_ocr_run_kernel_Stencil1DDavid_x86_remote_regression = {
#     'name': 'ocr-run-kernel-Stencil1DDavid-x86-remote-regression',
#     'depends': ('ocr-build-kernel-Stencil1DDavid-x86',),
#     'jobtype': 'ocr-run-app-regression',
#     'run-args': 'stencil x86 ocr-run-kernel-Stencil1DDavid-x86-remote-regression 10',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { 'T_ARCH': 'x86',
#                   'T_PATH': 'Stencil1D/refactored/ocr/intel-david',
#                   'CODE_TYPE': '2',
#                   'WORKLOAD_ARGS': ''
#               }
# }

# job_ocr_run_kernel_Stencil1DDavid_x86_remote_scaling = {
#     'name': 'ocr-run-kernel-Stencil1DDavid-x86-remote-scaling',
#     'depends': ('ocr-build-kernel-Stencil1DDavid-x86',),
#     'jobtype': 'ocr-run-app-scaling',
#     'run-args': 'Stencil1DDavid x86 ocr-run-kernel-Stencil1DDavid-x86-remote-scaling 10',
#     'sandbox': ('shared','inherit0'),
#     'env-vars': { 'T_ARCH': 'x86',
#                   'T_PATH': 'Stencil1D/refactored/ocr/intel-david',
#                   'CODE_TYPE': '2',
#                   'WORKLOAD_ARGS': ''
#               }
# }

# 1D Stencil (Chandra Martha)
job_ocr_build_kernel_Stencil1DChandra_x86_regression = {
    'name': 'ocr-build-kernel-Stencil1DChandra-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'stencil_1d x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'Stencil1D/refactored/ocr/intel-chandra',
              }
}

job_ocr_run_kernel_Stencil1DChandra_x86_remote_regression = {
    'name': 'ocr-run-kernel-Stencil1DChandra-x86-remote-regression',
    'depends': ('ocr-build-kernel-Stencil1DChandra-x86',),
    'jobtype': 'ocr-run-app-regression',
    'run-args': 'stencil_1d x86 ocr-run-kernel-Stencil1DChandra-x86-remote-regression 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'Stencil1D/refactored/ocr/intel-chandra',
                  'WORKLOAD_ARGS': '1000 100 500'
              }
}

job_ocr_run_kernel_Stencil1DChandra_x86_remote_scaling = {
    'name': 'ocr-run-kernel-Stencil1DChandra-x86-remote-scaling',
    'depends': ('ocr-build-kernel-Stencil1DChandra-x86',),
    'jobtype': 'ocr-run-app-scaling',
    'run-args': 'Stencil1DChandra x86 ocr-run-kernel-Stencil1DChandra-x86-remote-scaling 10',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'Stencil1D/refactored/ocr/intel-chandra',
                  'WORKLOAD_ARGS': '1000 100 500'
              }
}

# 1D Stencil - mpilite
job_ocr_build_kernel_Stencil1Dlite_x86_regression = {
    'name': 'ocr-build-kernel-Stencil1Dlite-x86',
    'depends': ('ocr-build-x86',),
    'jobtype': 'ocr-build-app',
    'run-args': 'stencil_1d x86',
    'sandbox': ('shared','inherit0'),
    'env-vars': { 'T_ARCH': 'x86',
                  'T_PATH': 'Stencil1D/refactored/mpilite/intel',
              }
}

# Disabled, see bug #805
#job_ocr_run_kernel_Stencil1Dlite_x86_remote_regression = {
#    'name': 'ocr-run-kernel-Stenc  il1Dlite-x86-remote-regression',
#    'depends': ('ocr-build-kernel-Stencil1Dlite-x86',),
#    'jobtype': 'ocr-run-app-regression',
#    'run-args': 'stencil x86 ocr-run-kernel-Stencil1Dlite-x86-remote-regression 10',
#    'sandbox': ('shared','inherit0'),
#    'env-vars': { 'T_ARCH': 'x86',
#                  'T_PATH': 'Stencil1D/refactored/mpilite/intel',
#                  'WORKLOAD_ARGS': '65538 20000'
#              }
#}


# Disabled, see bug #804
# use different workload_args from plain "run" to get less negative scaling
#job_ocr_run_kernel_Stencil1Dlite_x86_remote_scaling = {
#    'name': 'ocr-run-kernel-Stencil1Dlite-x86-remote-scaling',
#    'depends': ('ocr-build-kernel-Stencil1Dlite-x86',),
#    'jobtype': 'ocr-run-app-scaling',
#    'run-args': 'stencil x86 ocr-run-kernel-Stencil1Dlite-x86-remote-scaling 10',
#    'sandbox': ('shared','inherit0'),
#    'env-vars': { 'T_ARCH': 'x86',
#                  'T_PATH': 'Stencil1D/refactored/mpilite/intel',
#                  'WORKLOAD_ARGS': '65538 20000'
#              }
#}

#Aggregates execution times in csv file
job_gatherStats = {
    'name': 'gatherStats',
    'depends': ('__type ocr-run-app-regression', '__type ocr-run-app-scaling',),
    'jobtype': 'gatherStats-regression',
    'run-args': '${JJOB_SHARED_HOME}/apps/runtime 10',
    'sandbox': ('shared','inherit0')
}
