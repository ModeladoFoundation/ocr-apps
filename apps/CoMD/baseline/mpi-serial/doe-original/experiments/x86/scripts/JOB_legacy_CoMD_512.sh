#!/bin/bash -l
#SBATCH -p regular
#SBATCH -N 512
#SBATCH -t 4:00:00
#SBATCH -J JOB_MC_CoMD_512.out
#SBATCH -o JOB_MC_CoMD_512.out
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


echo weakscaling smallLJ 512 1
srun  -n 512 weakscaling_smallLJ_512x1/CoMD-mpi_cray -x 96 -y 96 -z 96 -i 8 -j 8 -k 8 -N 100 -n 10
srun  -n 512 weakscaling_smallLJ_512x1/CoMD-mpi_cray -x 96 -y 96 -z 96 -i 8 -j 8 -k 8 -N 100 -n 10

echo weakscaling smallLJ 512 4
srun  -n 2048 weakscaling_smallLJ_512x4/CoMD-mpi_cray -x 192 -y 192 -z 96 -i 16 -j 16 -k 8 -N 100 -n 10
srun  -n 2048 weakscaling_smallLJ_512x4/CoMD-mpi_cray -x 192 -y 192 -z 96 -i 16 -j 16 -k 8 -N 100 -n 10

echo weakscaling smallLJ 512 8
srun  -n 4096 weakscaling_smallLJ_512x8/CoMD-mpi_cray -x 192 -y 192 -z 192 -i 16 -j 16 -k 16 -N 100 -n 10
srun  -n 4096 weakscaling_smallLJ_512x8/CoMD-mpi_cray -x 192 -y 192 -z 192 -i 16 -j 16 -k 16 -N 100 -n 10

echo weakscaling smallLJ 512 16
srun  -n 8192 weakscaling_smallLJ_512x16/CoMD-mpi_cray -x 384 -y 192 -z 192 -i 32 -j 16 -k 16 -N 100 -n 10
srun  -n 8192 weakscaling_smallLJ_512x16/CoMD-mpi_cray -x 384 -y 192 -z 192 -i 32 -j 16 -k 16 -N 100 -n 10

echo weakscaling smallLJ 512 24
srun  -n 12288 weakscaling_smallLJ_512x24/CoMD-mpi_cray -x 384 -y 288 -z 192 -i 32 -j 24 -k 16 -N 100 -n 10
srun  -n 12288 weakscaling_smallLJ_512x24/CoMD-mpi_cray -x 384 -y 288 -z 192 -i 32 -j 24 -k 16 -N 100 -n 10

echo weakscaling smallEAM 512 1
srun  -n 512 weakscaling_smallEAM_512x1/CoMD-mpi_cray -x 96 -y 96 -z 96 -i 8 -j 8 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 512 weakscaling_smallEAM_512x1/CoMD-mpi_cray -x 96 -y 96 -z 96 -i 8 -j 8 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 512 4
srun  -n 2048 weakscaling_smallEAM_512x4/CoMD-mpi_cray -x 192 -y 192 -z 96 -i 16 -j 16 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 2048 weakscaling_smallEAM_512x4/CoMD-mpi_cray -x 192 -y 192 -z 96 -i 16 -j 16 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 512 8
srun  -n 4096 weakscaling_smallEAM_512x8/CoMD-mpi_cray -x 192 -y 192 -z 192 -i 16 -j 16 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 4096 weakscaling_smallEAM_512x8/CoMD-mpi_cray -x 192 -y 192 -z 192 -i 16 -j 16 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 512 16
srun  -n 8192 weakscaling_smallEAM_512x16/CoMD-mpi_cray -x 384 -y 192 -z 192 -i 32 -j 16 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 8192 weakscaling_smallEAM_512x16/CoMD-mpi_cray -x 384 -y 192 -z 192 -i 32 -j 16 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 512 24
srun  -n 12288 weakscaling_smallEAM_512x24/CoMD-mpi_cray -x 384 -y 288 -z 192 -i 32 -j 24 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 12288 weakscaling_smallEAM_512x24/CoMD-mpi_cray -x 384 -y 288 -z 192 -i 32 -j 24 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumLJ 512 1
srun  -n 512 weakscaling_mediumLJ_512x1/CoMD-mpi_cray -x 192 -y 192 -z 192 -i 8 -j 8 -k 8 -N 100 -n 10
srun  -n 512 weakscaling_mediumLJ_512x1/CoMD-mpi_cray -x 192 -y 192 -z 192 -i 8 -j 8 -k 8 -N 100 -n 10

echo weakscaling mediumLJ 512 4
srun  -n 2048 weakscaling_mediumLJ_512x4/CoMD-mpi_cray -x 384 -y 384 -z 192 -i 16 -j 16 -k 8 -N 100 -n 10
srun  -n 2048 weakscaling_mediumLJ_512x4/CoMD-mpi_cray -x 384 -y 384 -z 192 -i 16 -j 16 -k 8 -N 100 -n 10

echo weakscaling mediumLJ 512 8
srun  -n 4096 weakscaling_mediumLJ_512x8/CoMD-mpi_cray -x 384 -y 384 -z 384 -i 16 -j 16 -k 16 -N 100 -n 10
srun  -n 4096 weakscaling_mediumLJ_512x8/CoMD-mpi_cray -x 384 -y 384 -z 384 -i 16 -j 16 -k 16 -N 100 -n 10

echo weakscaling mediumLJ 512 16
srun  -n 8192 weakscaling_mediumLJ_512x16/CoMD-mpi_cray -x 768 -y 384 -z 384 -i 32 -j 16 -k 16 -N 100 -n 10
srun  -n 8192 weakscaling_mediumLJ_512x16/CoMD-mpi_cray -x 768 -y 384 -z 384 -i 32 -j 16 -k 16 -N 100 -n 10

echo weakscaling mediumLJ 512 24
srun  -n 12288 weakscaling_mediumLJ_512x24/CoMD-mpi_cray -x 768 -y 576 -z 384 -i 32 -j 24 -k 16 -N 100 -n 10
srun  -n 12288 weakscaling_mediumLJ_512x24/CoMD-mpi_cray -x 768 -y 576 -z 384 -i 32 -j 24 -k 16 -N 100 -n 10

echo weakscaling mediumEAM 512 1
srun  -n 512 weakscaling_mediumEAM_512x1/CoMD-mpi_cray -x 192 -y 192 -z 192 -i 8 -j 8 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 512 weakscaling_mediumEAM_512x1/CoMD-mpi_cray -x 192 -y 192 -z 192 -i 8 -j 8 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 512 4
srun  -n 2048 weakscaling_mediumEAM_512x4/CoMD-mpi_cray -x 384 -y 384 -z 192 -i 16 -j 16 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 2048 weakscaling_mediumEAM_512x4/CoMD-mpi_cray -x 384 -y 384 -z 192 -i 16 -j 16 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 512 8
srun  -n 4096 weakscaling_mediumEAM_512x8/CoMD-mpi_cray -x 384 -y 384 -z 384 -i 16 -j 16 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 4096 weakscaling_mediumEAM_512x8/CoMD-mpi_cray -x 384 -y 384 -z 384 -i 16 -j 16 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 512 16
srun  -n 8192 weakscaling_mediumEAM_512x16/CoMD-mpi_cray -x 768 -y 384 -z 384 -i 32 -j 16 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 8192 weakscaling_mediumEAM_512x16/CoMD-mpi_cray -x 768 -y 384 -z 384 -i 32 -j 16 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 512 24
srun  -n 12288 weakscaling_mediumEAM_512x24/CoMD-mpi_cray -x 768 -y 576 -z 384 -i 32 -j 24 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 12288 weakscaling_mediumEAM_512x24/CoMD-mpi_cray -x 768 -y 576 -z 384 -i 32 -j 24 -k 16 -N 100 -n 10 -e -d ../../../datasets/pots

