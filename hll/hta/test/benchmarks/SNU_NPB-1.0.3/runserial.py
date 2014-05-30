#!/usr/bin/python

# this script runs benchmarks and generate plots of total execution time and speedup
# using matplotlib

import os
import shutil
import time
import sys

def runtest(bench_name, c, num_trials):
    "Run benchmark and generate time record"
    for x in range(num_trials):
        cmd = "NPB3.3-SER-C/bin/" + bench_name + "." + c + ".x"
        print cmd
        ret = os.system(cmd)
        assert ret == 0

# =============================================================================
# Configurations
# =============================================================================
num_trials = 10
classes = ['A', 'B', 'C']
benches = ['ep','is','ft','cg','mg','lu']
# =============================================================================
# Start main procedure
# =============================================================================

MODE = 0x3
if len(sys.argv) > 1:
    MODE = int(sys.argv[1])

if MODE & 0x1:    
    # Build executable
    os.chdir('NPB3.3-SER-C')
    for bench_name in benches:
        for c in classes:
            os.system("make " + bench_name + " CLASS=" + c)
    os.chdir('..')

if MODE & 0x2:
    # Remove old data (FIXME: should backup old data instead)
    if not os.path.exists('timer.flag'):
        os.system('touch timer.flag');
        print 'Creates timer.flag at current directory'

    timestring = str(int(time.time()))
    if os.path.isdir("rec_serial"):
        os.system('tar zcvf rec_serial' + timestring +'.tgz rec_serial')
        os.system('rm -rf rec_serial')
    os.mkdir("rec_serial")

    for bench_name in benches:
        for c in classes:
            runtest(bench_name, c, num_trials)

