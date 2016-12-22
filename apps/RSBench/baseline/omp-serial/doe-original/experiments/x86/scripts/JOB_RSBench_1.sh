#!/bin/bash -l
#SBATCH -p regular
#SBATCH -N 1
#SBATCH -t 4:00:00
#SBATCH -J JOB_RSBench_1.out
#SBATCH -o JOB_RSBench_1.out
#SBATCH -L SCRATCH

export RUN_MODE=runApp
export OCRRUN_OPT_ENVKIND=SLURM

#For slurm run with Intel MPI the following is needed
module load PrgEnv-intel
module load impi

if [[ `hostname` == edison* ]]; then
    export I_MPI_PMI_LIBRARY=/opt/slurm/default/lib/pmi/libpmi.so
elif [[ `hostname` == cori* ]]; then
    export I_MPI_PMI_LIBRARY=/usr/lib64/slurmpmi/libmpi.so
fi

cd $SLURM_SUBMIT_DIR


echo strongscaling small 1 1
export OMP_NUM_THREADS=1; strongscaling_small_1x1/rsbench -s small -t 1 -l 1000000 -d
export OMP_NUM_THREADS=1; strongscaling_small_1x1/rsbench -s small -t 1 -l 1000000 -d

echo strongscaling small 1 2
export OMP_NUM_THREADS=2; strongscaling_small_1x2/rsbench -s small -t 2 -l 1000000 -d
export OMP_NUM_THREADS=2; strongscaling_small_1x2/rsbench -s small -t 2 -l 1000000 -d

echo strongscaling small 1 4
export OMP_NUM_THREADS=4; strongscaling_small_1x4/rsbench -s small -t 4 -l 1000000 -d
export OMP_NUM_THREADS=4; strongscaling_small_1x4/rsbench -s small -t 4 -l 1000000 -d

echo strongscaling small 1 8
export OMP_NUM_THREADS=8; strongscaling_small_1x8/rsbench -s small -t 8 -l 1000000 -d
export OMP_NUM_THREADS=8; strongscaling_small_1x8/rsbench -s small -t 8 -l 1000000 -d

echo strongscaling small 1 16
export OMP_NUM_THREADS=16; strongscaling_small_1x16/rsbench -s small -t 16 -l 1000000 -d
export OMP_NUM_THREADS=16; strongscaling_small_1x16/rsbench -s small -t 16 -l 1000000 -d

echo strongscaling small 1 24
export OMP_NUM_THREADS=24; strongscaling_small_1x24/rsbench -s small -t 24 -l 1000000 -d
export OMP_NUM_THREADS=24; strongscaling_small_1x24/rsbench -s small -t 24 -l 1000000 -d

echo strongscaling large 1 1
export OMP_NUM_THREADS=1; strongscaling_large_1x1/rsbench -s large -t 1 -l 1000000 -d
export OMP_NUM_THREADS=1; strongscaling_large_1x1/rsbench -s large -t 1 -l 1000000 -d

echo strongscaling large 1 2
export OMP_NUM_THREADS=2; strongscaling_large_1x2/rsbench -s large -t 2 -l 1000000 -d
export OMP_NUM_THREADS=2; strongscaling_large_1x2/rsbench -s large -t 2 -l 1000000 -d

echo strongscaling large 1 4
export OMP_NUM_THREADS=4; strongscaling_large_1x4/rsbench -s large -t 4 -l 1000000 -d
export OMP_NUM_THREADS=4; strongscaling_large_1x4/rsbench -s large -t 4 -l 1000000 -d

echo strongscaling large 1 8
export OMP_NUM_THREADS=8; strongscaling_large_1x8/rsbench -s large -t 8 -l 1000000 -d
export OMP_NUM_THREADS=8; strongscaling_large_1x8/rsbench -s large -t 8 -l 1000000 -d

echo strongscaling large 1 16
export OMP_NUM_THREADS=16; strongscaling_large_1x16/rsbench -s large -t 16 -l 1000000 -d
export OMP_NUM_THREADS=16; strongscaling_large_1x16/rsbench -s large -t 16 -l 1000000 -d

echo strongscaling large 1 24
export OMP_NUM_THREADS=24; strongscaling_large_1x24/rsbench -s large -t 24 -l 1000000 -d
export OMP_NUM_THREADS=24; strongscaling_large_1x24/rsbench -s large -t 24 -l 1000000 -d

