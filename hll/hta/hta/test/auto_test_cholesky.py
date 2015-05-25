#!/usr/bin/env python
import sys
import os
import time
import subprocess
import argparse
import re

# Configurations
NUM_TRIALS      = 3
PROCS           = [ x for x in range(1,33)]
BACKENDS        = ['omp', 'swarm', 'ocr']
MODES           = ['spmd', 'fj']
OUTPUT_PREFIX   = 'cholesky_result'
EXEC_PREFIX     = './HTA_cholesky'

def run_and_get_min_exec_time(cmd, num_trials):
    min_exec_time = None
    for i in range(num_trials):
        print "Executing command: " + cmd
        proc = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

        while True:
            line = proc.stdout.readline()
            if line == '':
                break
            else:
                if line.startswith('Tiled    HTA Cholesky decomposition, Matrix size'):
                    numbers = [int(s) for s in line.split() if s.isdigit()]
                    if min_exec_time == None:
                        min_exec_time = numbers[1]
                    elif min_exec_time > numbers[1]:
                        min_exec_time = numbers[1]
    assert min_exec_time != None
    return min_exec_time

# main
timestring = str(int(time.time()))
#ocr_config_path = os.environ['OCR_INSTALL'] + "/config/"
ocr_config_path = os.environ['UHPC'] + "/ocr/configs/mallocproxy/"

# Parse arguments
parser = argparse.ArgumentParser()
parser.add_argument("num_tiles", nargs=1, help="Specify the number of tiles in each dimension", type=int, metavar='NUM_TILES')
parser.add_argument("num_elems", nargs=1, help="Specify the number of elements in each dimension of a tile", type=int, metavar='NUM_ELEMS')
parser.add_argument("-b", "--backend", nargs='*', help="Specify the backend used in your experiment", default=BACKENDS, choices=['omp','swarm','ocr'], metavar='BACKEND')
parser.add_argument("-t", "--trials", nargs=1, help="Specify the number of trials for each configuration", type=int, default=[NUM_TRIALS], metavar='NUM_TRIALS')
parser.add_argument("-m", "--mode", nargs='*', default=MODES, choices=['spmd', 'fj'], metavar='MODE')
parser.add_argument("-p", "--procs", nargs='*',  help="Specify the number of processes", default=PROCS, choices=range(1,81), metavar='PROC_NUM')
#parser.add_argument("--base", nargs='?', help='Specify the base to normalize (only support pure_omp for now)', default='pure_omp', choices=['pure_omp'])
parser.add_argument("-o", "--output", nargs='?', help="Specify the file name prefix of the output", default=OUTPUT_PREFIX)
parser.add_argument("-d", "--distribution", nargs='?', help="Specify the distribution of tiles", default=5, choices=range(7), type=int, metavar='DIST')
ns = parser.parse_args()

# Check executable file existence
for mode in ns.mode:
    for b in ns.backend:
        # Generate the name of the executable
        if mode == 'spmd':
            exec_name = EXEC_PREFIX + '_' + mode + '_' + b
        else:
            exec_name = EXEC_PREFIX + '_' + b
        assert os.path.isfile(exec_name), "Executable %r does not exist" % exec_name

# Run tests
for mode in ns.mode:
    if mode == 'spmd':
        output = open('./' + ns.output + '_spmd.csv', 'w')
    else:
        output = open('./' + ns.output + '.csv', 'w')

    # Write the configurations
    output.write(str(ns))
    output.write('\n')

    output.write("NP")
    for i in ns.procs:
        output.write("," + str(i))
    output.write("\n")

    for b in ns.backend:
        # Generate the name of the executable
        if mode == 'spmd':
            exec_name = EXEC_PREFIX + '_' + mode + '_' + b
        else:
            exec_name = EXEC_PREFIX + '_' + b
        assert os.path.isfile(exec_name), "Executable %r does not exist" % exec_name

        output.write(exec_name)

        for p in ns.procs:
            if mode == 'spmd':
                os.environ["PIL_NUM_THREADS"] = str(p)
                os.environ["OMP_NUM_THREADS"] = str(p)
                os.environ["SWARM_NR_THREADS"] = str(p*2)
            else:
                os.environ["OMP_SCHEDULE"] = "dynamic,1"
                os.environ["OMP_NUM_THREADS"] = str(p)
                os.environ["SWARM_NR_THREADS"] = str(p+1)

            if b == "ocr":
                if mode == 'spmd':
                    args = ' D ' + str(ns.num_tiles[0]) + ' ' + str(ns.num_elems[0]) + ' ' + str(p) + ' ' + str(ns.distribution) + " -ocr:cfg " + ocr_config_path + str(p*2) + "w-Regular-mallocproxy.cfg "
                else:
                    args = ' D ' + str(ns.num_tiles[0]) + ' ' + str(ns.num_elems[0]) + ' ' + str(p) + ' ' + str(ns.distribution) + " -ocr:cfg " + ocr_config_path + str(p+1) + "w-Regular-mallocproxy.cfg "
            else:
                args = ' D ' + str(ns.num_tiles[0]) + ' ' + str(ns.num_elems[0]) + ' ' + str(p) + ' ' + str(ns.distribution)
            cmd = exec_name + args
            t = run_and_get_min_exec_time(cmd, ns.trials[0])
            output.write(","+str(t))
        output.write("\n")
        output.flush()

    output.close()

