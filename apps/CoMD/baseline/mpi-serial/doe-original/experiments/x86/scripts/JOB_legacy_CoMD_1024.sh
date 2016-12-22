#!/bin/bash -l
#SBATCH -p regular
#SBATCH -N 1024
#SBATCH -t 4:00:00
#SBATCH -J JOB_MC_CoMD_1024.out
#SBATCH -o JOB_MC_CoMD_1024.out
#SBATCH -L SCRATCH

export RUN_MODE=runApp
export OCRRUN_OPT_ENVKIND=SLURM

module swap PrgEnv-intel PrgEnv-cray

#For slurm run with Intel MPI the following is needed
#module load PrgEnv-intel
#module load impi
#
#if [[ `hostname` == edison* ]]; then
#    export I_MPI_PMI_LIBRARY=/opt/slurm/default/lib/pmi/libpmi.so
#elif [[ `hostname` == cori* ]]; then
#    export I_MPI_PMI_LIBRARY=/usr/lib64/slurmpmi/libmpi.so
#fi

cd $SLURM_SUBMIT_DIR


echo weakscaling smallLJ 1024 1
srun  -n 1024 weakscaling_smallLJ_1024x1/CoMD-mpi_cray -x 192 -y 96 -z 96 -i 16 -j 8 -k 8 -N 100 -n 10
srun  -n 1024 weakscaling_smallLJ_1024x1/CoMD-mpi_cray -x 192 -y 96 -z 96 -i 16 -j 8 -k 8 -N 100 -n 10

echo weakscaling smallLJ 1024 4
srun  -n 4096 weakscaling_smallLJ_1024x4/CoMD-mpi_cray -x 384 -y 192 -z 96 -i 32 -j 16 -k 8 -N 100 -n 10
srun  -n 4096 weakscaling_smallLJ_1024x4/CoMD-mpi_cray -x 384 -y 192 -z 96 -i 32 -j 16 -k 8 -N 100 -n 10

echo weakscaling smallLJ 1024 8
srun  -n 8192 weakscaling_smallLJ_1024x8/CoMD-mpi_cray -x 384 -y 192 -z 192 -i 32 -j 16 -k 16 -N 100 -n 10
srun  -n 8192 weakscaling_smallLJ_1024x8/CoMD-mpi_cray -x 384 -y 192 -z 192 -i 32 -j 16 -k 16 -N 100 -n 10

echo weakscaling smallLJ 1024 16
srun  -n 16384 weakscaling_smallLJ_1024x16/CoMD-mpi_cray -x 768 -y 192 -z 192 -i 64 -j 16 -k 16 -N 100 -n 10
srun  -n 16384 weakscaling_smallLJ_1024x16/CoMD-mpi_cray -x 768 -y 192 -z 192 -i 64 -j 16 -k 16 -N 100 -n 10

echo weakscaling smallLJ 1024 24
srun  -n 24576 weakscaling_smallLJ_1024x24/CoMD-mpi_cray -x 768 -y 288 -z 192 -i 64 -j 24 -k 16 -N 100 -n 10
srun  -n 24576 weakscaling_smallLJ_1024x24/CoMD-mpi_cray -x 768 -y 288 -z 192 -i 64 -j 24 -k 16 -N 100 -n 10

echo weakscaling smallEAM 1024 1
srun  -n 1024 weakscaling_smallEAM_1024x1/CoMD-mpi_cray -x 192 -y 96 -z 96 -i 16 -j 8 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 1024 weakscaling_smallEAM_1024x1/CoMD-mpi_cray -x 192 -y 96 -z 96 -i 16 -j 8 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 1024 4
srun  -n 4096 weakscaling_smallEAM_1024x4/CoMD-mpi_cray -x 384 -y 192 -z 96 -i 32 -j 16 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 4096 weakscaling_smallEAM_1024x4/CoMD-mpi_cray -x 384 -y 192 -z 96 -i 32 -j 16 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 1024 8
srun  -n 8192 weakscaling_smallEAM_1024x8/CoMD-mpi_cray -x 384 -y 192 -z 192 -i 32 -j 16 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 8192 weakscaling_smallEAM_1024x8/CoMD-mpi_cray -x 384 -y 192 -z 192 -i 32 -j 16 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 1024 16
srun  -n 16384 weakscaling_smallEAM_1024x16/CoMD-mpi_cray -x 768 -y 192 -z 192 -i 64 -j 16 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 16384 weakscaling_smallEAM_1024x16/CoMD-mpi_cray -x 768 -y 192 -z 192 -i 64 -j 16 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 1024 24
srun  -n 24576 weakscaling_smallEAM_1024x24/CoMD-mpi_cray -x 768 -y 288 -z 192 -i 64 -j 24 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 24576 weakscaling_smallEAM_1024x24/CoMD-mpi_cray -x 768 -y 288 -z 192 -i 64 -j 24 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumLJ 1024 1
srun  -n 1024 weakscaling_mediumLJ_1024x1/CoMD-mpi_cray -x 384 -y 192 -z 192 -i 16 -j 8 -k 8 -N 100 -n 10
srun  -n 1024 weakscaling_mediumLJ_1024x1/CoMD-mpi_cray -x 384 -y 192 -z 192 -i 16 -j 8 -k 8 -N 100 -n 10

echo weakscaling mediumLJ 1024 4
srun  -n 4096 weakscaling_mediumLJ_1024x4/CoMD-mpi_cray -x 768 -y 384 -z 192 -i 32 -j 16 -k 8 -N 100 -n 10
srun  -n 4096 weakscaling_mediumLJ_1024x4/CoMD-mpi_cray -x 768 -y 384 -z 192 -i 32 -j 16 -k 8 -N 100 -n 10

echo weakscaling mediumLJ 1024 8
srun  -n 8192 weakscaling_mediumLJ_1024x8/CoMD-mpi_cray -x 768 -y 384 -z 384 -i 32 -j 16 -k 16 -N 100 -n 10
srun  -n 8192 weakscaling_mediumLJ_1024x8/CoMD-mpi_cray -x 768 -y 384 -z 384 -i 32 -j 16 -k 16 -N 100 -n 10

echo weakscaling mediumLJ 1024 16
srun  -n 16384 weakscaling_mediumLJ_1024x16/CoMD-mpi_cray -x 1536 -y 384 -z 384 -i 64 -j 16 -k 16 -N 100 -n 10
srun  -n 16384 weakscaling_mediumLJ_1024x16/CoMD-mpi_cray -x 1536 -y 384 -z 384 -i 64 -j 16 -k 16 -N 100 -n 10

echo weakscaling mediumLJ 1024 24
srun  -n 24576 weakscaling_mediumLJ_1024x24/CoMD-mpi_cray -x 1536 -y 576 -z 384 -i 64 -j 24 -k 16 -N 100 -n 10
srun  -n 24576 weakscaling_mediumLJ_1024x24/CoMD-mpi_cray -x 1536 -y 576 -z 384 -i 64 -j 24 -k 16 -N 100 -n 10

echo weakscaling mediumEAM 1024 1
srun  -n 1024 weakscaling_mediumEAM_1024x1/CoMD-mpi_cray -x 384 -y 192 -z 192 -i 16 -j 8 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 1024 weakscaling_mediumEAM_1024x1/CoMD-mpi_cray -x 384 -y 192 -z 192 -i 16 -j 8 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 1024 4
srun  -n 4096 weakscaling_mediumEAM_1024x4/CoMD-mpi_cray -x 768 -y 384 -z 192 -i 32 -j 16 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 4096 weakscaling_mediumEAM_1024x4/CoMD-mpi_cray -x 768 -y 384 -z 192 -i 32 -j 16 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 1024 8
srun  -n 8192 weakscaling_mediumEAM_1024x8/CoMD-mpi_cray -x 768 -y 384 -z 384 -i 32 -j 16 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 8192 weakscaling_mediumEAM_1024x8/CoMD-mpi_cray -x 768 -y 384 -z 384 -i 32 -j 16 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 1024 16
srun  -n 16384 weakscaling_mediumEAM_1024x16/CoMD-mpi_cray -x 1536 -y 384 -z 384 -i 64 -j 16 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 16384 weakscaling_mediumEAM_1024x16/CoMD-mpi_cray -x 1536 -y 384 -z 384 -i 64 -j 16 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 1024 24
srun  -n 24576 weakscaling_mediumEAM_1024x24/CoMD-mpi_cray -x 1536 -y 576 -z 384 -i 64 -j 24 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 24576 weakscaling_mediumEAM_1024x24/CoMD-mpi_cray -x 1536 -y 576 -z 384 -i 64 -j 24 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots

