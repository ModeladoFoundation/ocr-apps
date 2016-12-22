#!/bin/bash -l
#SBATCH -p regular
#SBATCH -N 1024
#SBATCH -t 2:00:00
#SBATCH -J JN_MPI_Stencil2D_1024.out
#SBATCH -o JN_MPI_Stencil2D_1024.out
#SBATCH -L SCRATCH
#SBATCH --qos=premium

export RUN_MODE=runApp
export OCRRUN_OPT_ENVKIND=SLURM

#For slurm run with Intel MPI the following is needed
module swap PrgEnv-intel PrgEnv-cray
#module load impi
#
#if [[ `hostname` == edison* ]]; then
#    export I_MPI_PMI_LIBRARY=/opt/slurm/default/lib/pmi/libpmi.so
#elif [[ `hostname` == cori* ]]; then
#    export I_MPI_PMI_LIBRARY=/usr/lib64/slurmpmi/libmpi.so
#fi

cd $SLURM_SUBMIT_DIR


echo weakscaling small 1024 1
srun  -n 1024 weakscaling_small_1024x1/stencil_cray 100 24576
srun  -n 1024 weakscaling_small_1024x1/stencil_cray 100 24576

echo weakscaling small 1024 4
srun  -n 4096 weakscaling_small_1024x4/stencil_cray 100 49152
srun  -n 4096 weakscaling_small_1024x4/stencil_cray 100 49152

echo weakscaling small 1024 16
srun  -n 16384 weakscaling_small_1024x16/stencil_cray 100 98304
srun  -n 16384 weakscaling_small_1024x16/stencil_cray 100 98304

echo weakscaling small 1024 24
srun  -n 24576 weakscaling_small_1024x24/stencil_cray 100 147456
srun  -n 24576 weakscaling_small_1024x24/stencil_cray 100 147456

echo weakscaling medium 1024 1
srun  -n 1024 weakscaling_medium_1024x1/stencil_cray 100 49152
srun  -n 1024 weakscaling_medium_1024x1/stencil_cray 100 49152

echo weakscaling medium 1024 4
srun  -n 4096 weakscaling_medium_1024x4/stencil_cray 100 98304
srun  -n 4096 weakscaling_medium_1024x4/stencil_cray 100 98304

echo weakscaling medium 1024 16
srun  -n 16384 weakscaling_medium_1024x16/stencil_cray 100 196608
srun  -n 16384 weakscaling_medium_1024x16/stencil_cray 100 196608

echo weakscaling medium 1024 24
srun  -n 24576 weakscaling_medium_1024x24/stencil_cray 100 294912
srun  -n 24576 weakscaling_medium_1024x24/stencil_cray 100 294912

