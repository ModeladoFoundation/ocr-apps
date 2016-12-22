#!/bin/bash -l
#SBATCH -p regular
#SBATCH -N 4
#SBATCH -t 2:00:00
#SBATCH -J JN_MPI_Stencil2D_4.out
#SBATCH -o JN_MPI_Stencil2D_4.out
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


echo weakscaling small 4 1
srun --mpi=pmi2 -n 4 weakscaling_small_4x1/stencil 100 1536
srun --mpi=pmi2 -n 4 weakscaling_small_4x1/stencil 100 1536

echo weakscaling small 4 4
srun --mpi=pmi2 -n 16 weakscaling_small_4x4/stencil 100 3072
srun --mpi=pmi2 -n 16 weakscaling_small_4x4/stencil 100 3072

echo weakscaling small 4 16
srun --mpi=pmi2 -n 64 weakscaling_small_4x16/stencil 100 6144
srun --mpi=pmi2 -n 64 weakscaling_small_4x16/stencil 100 6144

echo weakscaling small 4 24
srun --mpi=pmi2 -n 96 weakscaling_small_4x24/stencil 100 9216
srun --mpi=pmi2 -n 96 weakscaling_small_4x24/stencil 100 9216

echo weakscaling medium 4 1
srun --mpi=pmi2 -n 4 weakscaling_medium_4x1/stencil 100 3072
srun --mpi=pmi2 -n 4 weakscaling_medium_4x1/stencil 100 3072

echo weakscaling medium 4 4
srun --mpi=pmi2 -n 16 weakscaling_medium_4x4/stencil 100 6144
srun --mpi=pmi2 -n 16 weakscaling_medium_4x4/stencil 100 6144

echo weakscaling medium 4 16
srun --mpi=pmi2 -n 64 weakscaling_medium_4x16/stencil 100 12288
srun --mpi=pmi2 -n 64 weakscaling_medium_4x16/stencil 100 12288

echo weakscaling medium 4 24
srun --mpi=pmi2 -n 96 weakscaling_medium_4x24/stencil 100 18432
srun --mpi=pmi2 -n 96 weakscaling_medium_4x24/stencil 100 18432

