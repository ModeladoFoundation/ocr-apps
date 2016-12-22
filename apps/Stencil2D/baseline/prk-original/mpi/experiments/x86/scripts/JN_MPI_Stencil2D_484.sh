#!/bin/bash -l
#SBATCH -p regular
#SBATCH -N 484
#SBATCH -t 2:00:00
#SBATCH -J JN_MPI_Stencil2D_484.out
#SBATCH -o JN_MPI_Stencil2D_484.out
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


echo weakscaling small 484 1
srun  -n 484 weakscaling_small_484x1/stencil_cray 100 16896
srun  -n 484 weakscaling_small_484x1/stencil_cray 100 16896

echo weakscaling small 484 4
srun  -n 1936 weakscaling_small_484x4/stencil_cray 100 33792
srun  -n 1936 weakscaling_small_484x4/stencil_cray 100 33792

echo weakscaling small 484 16
srun  -n 7744 weakscaling_small_484x16/stencil_cray 100 67584
srun  -n 7744 weakscaling_small_484x16/stencil_cray 100 67584

echo weakscaling small 484 24
srun  -n 11616 weakscaling_small_484x24/stencil_cray 100 101376
srun  -n 11616 weakscaling_small_484x24/stencil_cray 100 101376

echo weakscaling medium 484 1
srun  -n 484 weakscaling_medium_484x1/stencil_cray 100 33792
srun  -n 484 weakscaling_medium_484x1/stencil_cray 100 33792

echo weakscaling medium 484 4
srun  -n 1936 weakscaling_medium_484x4/stencil_cray 100 67584
srun  -n 1936 weakscaling_medium_484x4/stencil_cray 100 67584

echo weakscaling medium 484 16
srun  -n 7744 weakscaling_medium_484x16/stencil_cray 100 135168
srun  -n 7744 weakscaling_medium_484x16/stencil_cray 100 135168

echo weakscaling medium 484 24
srun  -n 11616 weakscaling_medium_484x24/stencil_cray 100 202752
srun  -n 11616 weakscaling_medium_484x24/stencil_cray 100 202752

