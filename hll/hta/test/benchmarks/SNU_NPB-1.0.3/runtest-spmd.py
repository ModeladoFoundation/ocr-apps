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
PROC_BIND = " OMP_PROC_BIND=FALSE "
num_trials = 1
NUM_RETRY = 1
procs = [ 1 << x for x in range(6) ]
classes = ['S']
benches = ['ep','is','ft','cg','mg']
procsLU = {1: (1,1,1), 2: (2,15,1), 4: (2,2,18), 8: (2,4,15), 16: (4,4,18), 32: (8,16,4)}
#backend = [["omp", PROC_BIND + "OMP_SCHEDULE=dynamic,1 OMP_NUM_THREADS", "HTA/OpenMP", "g-x"],["ocr", "OCR_NUM_THREADS", "HTA/OCR", "m-x"]]
backend = [["omp", PROC_BIND + "OMP_SCHEDULE=dynamic,1 OMP_NUM_THREADS", "HTA/OpenMP", "g-x"]]

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
	if bench_name == "lu" and backend[0] != "pure_omp":
	  tiles = procsLU[i]
        for x in range(num_trials):
          retry = 0
          while True:
            if backend[0] == "pure_omp":
                cmd = "OMP_NUM_THREADS=" + str(i) + " NPB3.3-OMP-C/bin/" + bench_name + "." + c + ".x"
            elif backend[0] == "swarm":
	      if bench_name == "lu":
		cmd = "PIL_NUM_THREADS=" + str(i) + " " + backend[1] + "=" + str(i*2) + " NPB3.3-HTA-SPMD/bin/" + bench_name + "." + c + ".x." + backend[0] + " " + str(tiles[0]) + " " + str(tiles[1]) + " " + str(tiles[2]) + " " + str(i)
	      else:
		cmd = "PIL_NUM_THREADS=" + str(i) + " " + backend[1] + "=" + str(i*2) + " NPB3.3-HTA-SPMD/bin/" + bench_name + "." + c + ".x." + backend[0] + " " + str(i)
            elif backend[0] == "ocr":
              ocr_config_path = os.environ['UHPC'] + "/ocr/configs/mallocproxy/"
	      if bench_name == "lu":
                cmd = "PIL_NUM_THREADS=" + str(i) + " " + " NPB3.3-HTA-SPMD/bin/" + bench_name + "." + c + ".x." + backend[0] + " -ocr:cfg " + ocr_config_path + str(i*2) + "w-Regular-mallocproxy.cfg "  + str(tiles[0]) + " " + str(tiles[1]) + " " + str(tiles[2]) + " " + str(i)
	      else:
                cmd = "PIL_NUM_THREADS=" + str(i) + " " + " NPB3.3-HTA-SPMD/bin/" + bench_name + "." + c + ".x." + backend[0] + " -ocr:cfg " + ocr_config_path + str(i*2) + "w-Regular-mallocproxy.cfg "  + str(i)
            else:
	      if bench_name == "lu":
		cmd = "PIL_NUM_THREADS=" + str(i) + " " + backend[1] + "=" + str(i) + " NPB3.3-HTA-SPMD/bin/" + bench_name + "." + c + ".x." + backend[0] + " " + str(tiles[0]) + " " + str(tiles[1]) + " " + str(tiles[2]) + " " + str(i)
	      else:
		cmd = "PIL_NUM_THREADS=" + str(i) + " " + backend[1] + "=" + str(i) + " NPB3.3-HTA-SPMD/bin/" + bench_name + "." + c + ".x." + backend[0] + " " + str(i)
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
    os.chdir('NPB3.3-HTA-SPMD')
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
        recdir = 'rec_spmd_' + be[0]
        if not os.path.isdir('rec_spmd'):
            os.mkdir('rec_spmd')
        for c in classes:
            for bench_name in benches:
                runtest(bench_name, be, c,  procs, num_trials)
        if os.path.isdir(recdir):
            backup_rec(recdir, timestring)
        os.system('mv rec_spmd ' + recdir)

if mode & GENPLOT:
    print "Please use genplot.py to generate plots"
