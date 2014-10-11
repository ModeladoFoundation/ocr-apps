#!/usr/bin/python

# this script runs benchmarks and generate plots of total execution time and speedup
# using matplotlib

import sys
import os
import shutil
import time
import numpy
import matplotlib as mtlib
mtlib.use('pdf')
import matplotlib.pyplot as plt
from matplotlib.font_manager import FontProperties
import csv

# =============================================================================
# Configurations
# =============================================================================
num_trials = 1
procs = [ 1 << x for x in range(6) ]
classes = ['A']
benches = ['cg','lu','ep','is','ft','mg']
procsLU = {1: (1,1,1), 2: (2,8,1), 4: (2,2,6), 8: (2,4,4), 16: (3,3,4), 32: (3,4,4), 64: (3,4,4)}
backend = [["pure_omp", "", "OpenMP", "b-x"], ["omp", "OMP_NUM_THREADS", "HTA/OpenMP", "g-x"],["ocr", "OCR_NUM_THREADS", "HTA/OCR", "m-x"]]
bench_full_names = {'ep' : 'Embarassingly Parallel', \
    'cg' : 'Conjugate Gradient', \
    'is' : 'Integer Sort', \
    'ft' : 'Fast Fourier Transform', \
    'mg' : 'Multigrid Method',
    'lu' : 'LU Factorization' }

markers = ['x', '^', 's', '.']
linestyle = ['-','-','--']
colors = ['r','g','b','m','y','c']

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
            if backend[0] == "pure_omp":
                cmd = "OMP_NUM_THREADS=" + str(i) + " NPB3.3-OMP-C/bin/" + bench_name + "." + c + ".x"
            else:
	      if bench_name == "lu":
		cmd = backend[1] + "=" + str(i) + " NPB3.3-HTA/bin/" + bench_name + "." + c + ".x." + backend[0] + " " + str(tiles[0]) + " " + str(tiles[1]) + " " + str(tiles[2]) + " " + str(i)
	      else:
		cmd = backend[1] + "=" + str(i) + " NPB3.3-HTA/bin/" + bench_name + "." + c + ".x." + backend[0] + " " + str(i)
            print cmd
            ret = os.system(cmd)
            assert ret == 0

def gen_seq_name(bench_name, c):
    "Generate path string serial version result"
    name = "rec_serial/" + bench_name + "." + c + ".rec"
    return name

def gen_rec_names(dirpath, bench_name, c, procs):
    "Generate path string list of benchmarks"
    names = []
    for i in procs:
        if dirpath == '../NPB3.3-MPI/rec/':
            name = dirpath + bench_name + "." + c + "." + str(i).zfill(2) + ".rec"
        else :
            name = dirpath + bench_name + "." + c + "." + str(i) + ".rec"
        names.append(name)
    return names

def process_seqrec(recname, col):
    with open(seqrec) as f:
        values = []
        for l in list(f): # treat f as a list of lines
            values.append(float(l.split()[col]))
        max_result = max(values)
        min_result = min(values)
        median_result = numpy.median(values)
        err_low = median_result - min_result
        err_high = max_result - median_result
        return [median_result, err_low, err_high]

def process_rec(col, recs):
    "Get the minimum record from rec files"
    total = []
    for rec in recs:
        print rec
        with open(rec) as f:
            values = []
            for l in list(f): # treat f as a list of lines
                values.append(float(l.split()[col]))
            max_result = max(values)
            min_result = min(values)
            median_result = numpy.median(values)
            err_low = median_result - min_result
            err_high = max_result - median_result
            total.append([median_result, err_low, err_high])
    return total
# =============================================================================
# Start main procedure
# =============================================================================

if len(sys.argv) < 2:
    mode = BUILD | RUNBENCH | GENPLOT
    print "Mode not specified. Run the benchmarks and process the result"
else:
    mode = int(sys.argv[1])
    print "Mode = " + str(mode)

timestring = str(int(time.time()))

if mode & BUILD:

    # Build executable
    os.chdir('NPB3.3-HTA')
    for bench_name in benches:
        for c in classes:
            os.system("make " + bench_name + " CLASS=" + c)
    os.chdir('..')
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
        recdir = 'rec_' + be[0]
        if not os.path.isdir('rec'):
            os.mkdir('rec')
        for c in classes:
            for bench_name in benches:
                runtest(bench_name, be, c,  procs, num_trials)
        if os.path.isdir(recdir):
            backup_rec(recdir, timestring)
        os.system('mv rec ' + recdir)

if mode & GENPLOT:

    if os.path.isdir("fig"):
        os.system('tar zcvf fig' + timestring +'.tgz fig')
        os.system('rm -rf fig')

    fig = 1
    col = 0   # the column to take in the time records
    ratios_of_classes = []
    labels_of_classes = []
    fontP = FontProperties()
    fontP.set_size('small')

    for c in classes:
        backend_speedup_ratios = []
        backend_speedup_ratio_labels = []
        for bench_name in benches:
            # get data and save in lists for different bacends
            backend_totals = []
            backend_speedups = []
            for be in backend:
                backend_totals.append([])

            # Collect time record and plot
            seqrec = gen_seq_name(bench_name, c)
            seq_total = process_seqrec(seqrec, col)[0]

            # PIL records
            for x in range(len(backend)):
                recs = gen_rec_names('rec_' + backend[x][0] + '/', bench_name, c, procs)
                backend_totals[x] = process_rec(col, recs)
            for total in backend_totals:
                backend_speedups.append([seq_total / i[0] for i in total])

            # plot
            plt.figure(fig)
            ax = plt.subplot(211)
            plt.title(bench_name.upper() + ": " + bench_full_names[bench_name] + "(CLASS = " + c + ")")
            plt.xlabel('Number of PEs')
            plt.ylabel('Total execution time (sec)')
            for x in range(len(backend)):
                values = [z[0] for z in backend_totals[x]]
                err_low = [z[1] for z in backend_totals[x]]
                err_high = [z[2] for z in backend_totals[x]]
                plt.errorbar(procs, values, yerr=[err_low, err_high], fmt = backend[x][3], label = backend[x][2])
            #plot sequential execution time
            plt.plot(procs, [seq_total]*len(procs), "k--", label = "SEQ", alpha=0.3);
            plt.legend(loc=1, prop = fontP)

            ax = plt.subplot(212)
            #plt.title('Speedup')
            plt.xlabel('Number of PEs')
            plt.ylabel('Speedup')
            plt.plot(procs, procs, 'm-', label='Ideal') # ideal speedup
            for i in range(len(backend)):
                plt.plot(procs, backend_speedups[i], backend[i][3], label = backend[i][2])
            plt.plot(procs, [1.0]*len(procs), "k--", label = "SEQ", alpha=0.3);
            #plt.plot(procs, mpi_speedup, "y-x", label = "MPI");
            plt.legend(loc=2, prop = fontP)

            plt.tight_layout() # adjust the spacing between subplots
            if not os.path.isdir('fig'):
               os.mkdir('fig')
            plt.savefig('fig/' + bench_name + '.' + c + '.pdf')
            fig += 1

            # plot speedup ratio use the first backend as base
            for j in range(len(backend_speedups)):
                if j == 0:
                    continue # skip j == 0
                ratios = []
                for i in range(len(procs)):
                    x = backend_speedups[j][i]
                    y = backend_speedups[0][i]
                    ratios.append(x/y)
                backend_speedup_ratios.append(ratios)
                backend_speedup_ratio_labels.append(bench_name + " " + backend[j][2] + "(" + c + ")")
        ratios_of_classes.append(backend_speedup_ratios)
        labels_of_classes.append(backend_speedup_ratio_labels)

    for c in range(len(ratios_of_classes)):
        backend_speedup_ratios = ratios_of_classes[c]
        print backend_speedup_ratios
        backend_speedup_ratio_labels = labels_of_classes[c]
        print backend_speedup_ratio_labels
        plt.figure(fig)
        plt.title("Speedup Ratio for class " + classes[c])
        plt.xlabel('Number of PEs')
        plt.ylabel('Speedup Ratio wrt pure OpenMP')
        ax = plt.subplot(111)
        n_backends = len(backend) - 1
        for i in range(len(backend_speedup_ratios)):
            ax.plot(procs, backend_speedup_ratios[i], colors[(i/n_backends) % len(benches)] + '-' + markers[i % n_backends], label = backend_speedup_ratio_labels[i])
        ax.plot(procs, [1.0]*len(procs), "k--", alpha=0.3);
        plt.ylim(0.0, 2.0)
        box = ax.get_position()
        ax.set_position([box.x0, box.y0, box.width * 0.8, box.height])
        #plt.legend(loc=1)
        ax.legend(loc='center left', bbox_to_anchor=(1, 0.5), prop=fontP)

        #plt.tight_layout() # adjust the spacing between subplots
        if not os.path.isdir('fig'):
           os.mkdir('fig')
        plt.savefig('fig/ratio.' + classes[c] + '.pdf')
        fig += 1

