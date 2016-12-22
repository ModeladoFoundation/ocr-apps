#!/bin/bash -l
#SBATCH -p regular
#SBATCH -N 1
#SBATCH -t 2:00:00
#SBATCH -J JN_MPI_Stencil2D_1.out
#SBATCH -o JN_MPI_Stencil2D_1.out
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


echo weakscaling small 1 1
srun --mpi=pmi2 -n 1 weakscaling_small_1x1/stencil 100 768
srun --mpi=pmi2 -n 1 weakscaling_small_1x1/stencil 100 768

echo weakscaling small 1 4
srun --mpi=pmi2 -n 4 weakscaling_small_1x4/stencil 100 1536
srun --mpi=pmi2 -n 4 weakscaling_small_1x4/stencil 100 1536

echo weakscaling small 1 16
srun --mpi=pmi2 -n 16 weakscaling_small_1x16/stencil 100 3072
srun --mpi=pmi2 -n 16 weakscaling_small_1x16/stencil 100 3072

echo weakscaling small 1 24
srun --mpi=pmi2 -n 24 weakscaling_small_1x24/stencil 100 4608
srun --mpi=pmi2 -n 24 weakscaling_small_1x24/stencil 100 4608

echo weakscaling medium 1 1
srun --mpi=pmi2 -n 1 weakscaling_medium_1x1/stencil 100 1536
srun --mpi=pmi2 -n 1 weakscaling_medium_1x1/stencil 100 1536

echo weakscaling medium 1 4
srun --mpi=pmi2 -n 4 weakscaling_medium_1x4/stencil 100 3072
srun --mpi=pmi2 -n 4 weakscaling_medium_1x4/stencil 100 3072

echo weakscaling medium 1 16
srun --mpi=pmi2 -n 16 weakscaling_medium_1x16/stencil 100 6144
srun --mpi=pmi2 -n 16 weakscaling_medium_1x16/stencil 100 6144

echo weakscaling medium 1 24
srun --mpi=pmi2 -n 24 weakscaling_medium_1x24/stencil 100 9216
srun --mpi=pmi2 -n 24 weakscaling_medium_1x24/stencil 100 9216

