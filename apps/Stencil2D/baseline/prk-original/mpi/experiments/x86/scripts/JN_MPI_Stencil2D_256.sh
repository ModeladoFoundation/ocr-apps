#!/bin/bash -l
#SBATCH -p regular
#SBATCH -N 256
#SBATCH -t 2:00:00
#SBATCH -J JN_MPI_Stencil2D_256.out
#SBATCH -o JN_MPI_Stencil2D_256.out
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


echo weakscaling small 256 1
srun  -n 256 weakscaling_small_256x1/stencil_cray 100 12288
srun  -n 256 weakscaling_small_256x1/stencil_cray 100 12288

echo weakscaling small 256 4
srun  -n 1024 weakscaling_small_256x4/stencil_cray 100 24576
srun  -n 1024 weakscaling_small_256x4/stencil_cray 100 24576

echo weakscaling small 256 16
srun  -n 4096 weakscaling_small_256x16/stencil_cray 100 49152
srun  -n 4096 weakscaling_small_256x16/stencil_cray 100 49152

echo weakscaling small 256 24
srun  -n 6144 weakscaling_small_256x24/stencil_cray 100 73728
srun  -n 6144 weakscaling_small_256x24/stencil_cray 100 73728

echo weakscaling medium 256 1
srun  -n 256 weakscaling_medium_256x1/stencil_cray 100 24576
srun  -n 256 weakscaling_medium_256x1/stencil_cray 100 24576

echo weakscaling medium 256 4
srun  -n 1024 weakscaling_medium_256x4/stencil_cray 100 49152
srun  -n 1024 weakscaling_medium_256x4/stencil_cray 100 49152

echo weakscaling medium 256 16
srun  -n 4096 weakscaling_medium_256x16/stencil_cray 100 98304
srun  -n 4096 weakscaling_medium_256x16/stencil_cray 100 98304

echo weakscaling medium 256 24
srun  -n 6144 weakscaling_medium_256x24/stencil_cray 100 147456
srun  -n 6144 weakscaling_medium_256x24/stencil_cray 100 147456

