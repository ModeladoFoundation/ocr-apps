#!/usr/bin/env python
import sys
import math
import os
from optparse import OptionParser

def gen_job_config(max_ranks, psize, core_count, lranks, lnodes, lpsize, lthreads):
    ranks = 1
    grid_size = psize
    while ranks <= max_ranks:
        lranks.append(ranks)
        nodes = int(ranks/(core_count * 1.0) + 0.5)
        if nodes < 1:
            lnodes.append(1)
        else:
            lnodes.append(nodes)
        if ranks < core_count:
            lthreads.append(ranks)
        else:
            lthreads.append(core_count)
        if(ranks == 1):
            grid_size = psize
        else:
            grid_size = int(psize * math.sqrt(ranks))
        lpsize.append(grid_size)
        ranks = ranks * 2

def gen_job_scripts(target_name, exp_name, max_ranks, partition_name, problem_size, niterations, core_count):
    lranks = []
    lnodes = []
    lpsize = []
    lthreads = []
    time_limit = '00:30:00'

    # Generate job configurations
    gen_job_config(max_ranks, problem_size, core_count, lranks, lnodes, lpsize, lthreads)

    try:
        os.mkdir('jobs', 0755)
    except:
        pass

    slurm_batch_script = ("#!/bin/bash\n"
    "#SBATCH -p {partition}\n"
    "#SBATCH -N {0}\n"
    "#SBATCH -t {time}\n"
    "#SBATCH --output={name}-{nrank}-{niter}-{problem_size}.out\n"
    "#SBATCH --error={name}-{nrank}-{niter}-{problem_size}.err\n"
    "export OCRRUN_OPT_ENVKIND=SLURM\n"
    "\n"
    "export PATH=/opt/intel/impi/5.1.2.150/intel64/bin:$PATH\n"
    "export LD_LIBRARY_PATH=/opt/intel/impi/5.1.2.150/intel64/lib:$LD_LIBRARY_PATH\n"
    "\n"
    "export GMON_OUT_PREFIX=gmon.out-\n"
    "export OCR_NUM_NODES={0}\n"
    "export CONFIG_NUM_THREADS={thread_count}\n"
    "export SLURM_SUBMIT_DIR=/home/saananth/xstack-report-expts/apps/hll/rose/tools/ocrTranslate/tests/stencil2D-mpi\n"
    "cd $SLURM_SUBMIT_DIR\n"
    "OCR_TYPE=x86-mpi make run {nrank} {niter} {problem_size}\n"
    )

    for r,n,p,t in zip(lranks, lnodes, lpsize, lthreads):
        nranks = r
        nnodes = n
        npsize = p
        nthreads = t
        print nranks, nnodes, npsize
        with open("jobs/{job_name}.{0}-{1:03}.slurm"
                      .format(nranks, niterations, job_name=exp_name), "w") as f:
            f.write(slurm_batch_script.format(nnodes, nrank=nranks, partition=partition_name, name=exp_name,
                                                  time=time_limit, niter=niterations, target=target_name,
                                                  problem_size=npsize, thread_count=nthreads))

def gen_slurm_scripts():
    parser = OptionParser(usage="usage: %prog [options]")
    parser.add_option("-t", "--target", action="store_true", dest="target_name", default=False, help="target binary name")
    parser.add_option("-r", "--ranks", action="store_true", dest="max_ranks", default=False, help="maximum number of ranks")
    parser.add_option("-p", "--partition", action="store_true", dest="partition_name", default=False, help="partition name")
    parser.add_option("-s", "--size", action="store_true", dest="problem_size", default=False, help="beginning size of the problem")
    parser.add_option("-i", "--iterations", action="store_true", dest="niterations", default=False, help="number of iterations")
    parser.add_option("-c", "--core_count", action="store_true", dest="core_count", default=False, help="number of cores per node")
    (options, args) = parser.parse_args()

    if len(args) != 6:
        parser.error("Expecting 6 arguments\n")

    target_name = args[0]
    exp_name = args[0]
    max_ranks = int(args[1])
    partition_name = args[2]
    problem_size = int(args[3])
    niterations = int(args[4])
    core_count = int(args[5])

    #Generate the job scripts
    gen_job_scripts(target_name, exp_name, max_ranks, partition_name, problem_size, niterations, core_count)

#Entry Point
gen_slurm_scripts()
