#!/bin/bash -l
#SBATCH -p regular
#SBATCH -N 16
#SBATCH -t 2:00:00
#SBATCH -J JN_MPI_Stencil2D_16.out
#SBATCH -o JN_MPI_Stencil2D_16.out
#SBATCH -L SCRATCH
#SBATCH --qos=premium

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


echo weakscaling small 16 1
srun --mpi=pmi2 -n 16 weakscaling_small_16x1/stencil 100 3072
srun --mpi=pmi2 -n 16 weakscaling_small_16x1/stencil 100 3072

echo weakscaling small 16 4
srun --mpi=pmi2 -n 64 weakscaling_small_16x4/stencil 100 6144
srun --mpi=pmi2 -n 64 weakscaling_small_16x4/stencil 100 6144

echo weakscaling small 16 16
srun --mpi=pmi2 -n 256 weakscaling_small_16x16/stencil 100 12288
srun --mpi=pmi2 -n 256 weakscaling_small_16x16/stencil 100 12288

echo weakscaling small 16 24
srun --mpi=pmi2 -n 384 weakscaling_small_16x24/stencil 100 18432
srun --mpi=pmi2 -n 384 weakscaling_small_16x24/stencil 100 18432

echo weakscaling medium 16 1
srun --mpi=pmi2 -n 16 weakscaling_medium_16x1/stencil 100 6144
srun --mpi=pmi2 -n 16 weakscaling_medium_16x1/stencil 100 6144

echo weakscaling medium 16 4
srun --mpi=pmi2 -n 64 weakscaling_medium_16x4/stencil 100 12288
srun --mpi=pmi2 -n 64 weakscaling_medium_16x4/stencil 100 12288

echo weakscaling medium 16 16
srun --mpi=pmi2 -n 256 weakscaling_medium_16x16/stencil 100 24576
srun --mpi=pmi2 -n 256 weakscaling_medium_16x16/stencil 100 24576

echo weakscaling medium 16 24
srun --mpi=pmi2 -n 384 weakscaling_medium_16x24/stencil 100 36864
srun --mpi=pmi2 -n 384 weakscaling_medium_16x24/stencil 100 36864

