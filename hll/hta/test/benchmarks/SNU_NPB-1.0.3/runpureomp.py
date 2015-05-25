#!/usr/bin/python

# this script runs benchmarks and generate plots of total execution time and speedup
# using matplotlib

import sys
import os
import shutil
import time

# =============================================================================
# Configurations
# =============================================================================
num_trials = 5
NUM_RETRY = 3
procs = [ 1 << x for x in range(6) ]
classes = ['C']
benches = ['ep','is','ft','mg','cg','lu']
backend = [["pure_omp", "OMP_NUM_THREADS", "OpenMP", "b-x"]]

# constants for different modes
BUILD = 0x1
RUNBENCH = 0x2         # it can run independently
GENPLOT = 0x4         # depends on the records generated from both RUNSERIAL and RUNBENCH

# =============================================================================
# Functions
# =============================================================================
def runtest(bench_name, backend, c, procs, num_trials):
    "Run benchmark and generate time record"
    for i in procs:
        for x in range(num_trials):
          retry = 0
          while True:
            cmd = "OMP_NUM_THREADS=" + str(i) + " NPB3.3-OMP-C/bin/" + bench_name + "." + c + ".x"
            print cmd
            ret = os.system(cmd)

            if ret == 0:
                break
            else:
                print "!!!!!!!!!!!!!!!! SOMETHING IS WRONG WITH THE EXECUTION !!!!!!!!!!!!!!!!!!!!!!!!!"
                print "!!!!!!!!!!!!!!!! ATTEMPT TO RETRY (" + str(retry) + ") !!!!!!!!!!!!!!!!!!!!!!!!!"
                retry += 1
            if retry == NUM_RETRY:
                print "!!!!!!!!!!!!!!!! EXECUTION FAILED !!!!!!!!!!!!!!!!!!!!!!!!!"
                sys.exit(1)

# =============================================================================
# Start main procedure
# =============================================================================

if len(sys.argv) < 2:
    mode = BUILD | RUNBENCH
    print "Mode not specified. Run the benchmarks and process the result"
else:
    mode = int(sys.argv[1])
    print "Mode = " + str(mode)

timestring = str(int(time.time()))

if mode & BUILD:
    # Build executable
    os.chdir('NPB3.3-OMP-C')
    for bench_name in benches:
        for c in classes:
            os.system("make " + bench_name + " CLASS=" + c)
    os.chdir('..')

def backup_rec(recdir, timestring):
    if os.path.isdir(recdir):
        os.system('tar zcvf ' + recdir + '.' + timestring +'.tgz ' + recdir)
        os.system('rm -rf ' + recdir)

if mode & RUNBENCH:
    if not os.path.exists('timer.flag'):
        os.system('touch timer.flag');
        print 'Creates timer.flag at current directory'
    for be in backend:
        recdir = 'rec_pure_omp'
        if os.path.isdir(recdir):
            backup_rec(recdir, timestring)
        if not os.path.isdir(recdir):
            os.mkdir(recdir)
        for c in classes:
            for bench_name in benches:
                runtest(bench_name, be, c,  procs, num_trials)

if mode & GENPLOT:
    print "Please use genplot.py to generate plots"
