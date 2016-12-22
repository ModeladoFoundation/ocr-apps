#!/bin/bash -l
#SBATCH -p regular
#SBATCH -N 8
#SBATCH -t 4:00:00
#SBATCH -J JOB_legacy_CoMD_8.out
#SBATCH -o JOB_legacy_CoMD_8.out
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


echo weakscaling smallLJ 8 1
srun --mpi=pmi2 -n 8 weakscaling_smallLJ_8x1/CoMD-mpi -x 24 -y 24 -z 24 -i 2 -j 2 -k 2 -N 100 -n 10
srun --mpi=pmi2 -n 8 weakscaling_smallLJ_8x1/CoMD-mpi -x 24 -y 24 -z 24 -i 2 -j 2 -k 2 -N 100 -n 10

echo weakscaling smallLJ 8 4
srun --mpi=pmi2 -n 32 weakscaling_smallLJ_8x4/CoMD-mpi -x 48 -y 48 -z 24 -i 4 -j 4 -k 2 -N 100 -n 10
srun --mpi=pmi2 -n 32 weakscaling_smallLJ_8x4/CoMD-mpi -x 48 -y 48 -z 24 -i 4 -j 4 -k 2 -N 100 -n 10

echo weakscaling smallLJ 8 8
srun --mpi=pmi2 -n 64 weakscaling_smallLJ_8x8/CoMD-mpi -x 48 -y 48 -z 48 -i 4 -j 4 -k 4 -N 100 -n 10
srun --mpi=pmi2 -n 64 weakscaling_smallLJ_8x8/CoMD-mpi -x 48 -y 48 -z 48 -i 4 -j 4 -k 4 -N 100 -n 10

echo weakscaling smallLJ 8 16
srun --mpi=pmi2 -n 128 weakscaling_smallLJ_8x16/CoMD-mpi -x 96 -y 48 -z 48 -i 8 -j 4 -k 4 -N 100 -n 10
srun --mpi=pmi2 -n 128 weakscaling_smallLJ_8x16/CoMD-mpi -x 96 -y 48 -z 48 -i 8 -j 4 -k 4 -N 100 -n 10

echo weakscaling smallLJ 8 24
srun --mpi=pmi2 -n 192 weakscaling_smallLJ_8x24/CoMD-mpi -x 96 -y 72 -z 48 -i 8 -j 6 -k 4 -N 100 -n 10
srun --mpi=pmi2 -n 192 weakscaling_smallLJ_8x24/CoMD-mpi -x 96 -y 72 -z 48 -i 8 -j 6 -k 4 -N 100 -n 10

echo weakscaling smallEAM 8 1
srun --mpi=pmi2 -n 8 weakscaling_smallEAM_8x1/CoMD-mpi -x 24 -y 24 -z 24 -i 2 -j 2 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 8 weakscaling_smallEAM_8x1/CoMD-mpi -x 24 -y 24 -z 24 -i 2 -j 2 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 8 4
srun --mpi=pmi2 -n 32 weakscaling_smallEAM_8x4/CoMD-mpi -x 48 -y 48 -z 24 -i 4 -j 4 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 32 weakscaling_smallEAM_8x4/CoMD-mpi -x 48 -y 48 -z 24 -i 4 -j 4 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 8 8
srun --mpi=pmi2 -n 64 weakscaling_smallEAM_8x8/CoMD-mpi -x 48 -y 48 -z 48 -i 4 -j 4 -k 4 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 64 weakscaling_smallEAM_8x8/CoMD-mpi -x 48 -y 48 -z 48 -i 4 -j 4 -k 4 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 8 16
srun --mpi=pmi2 -n 128 weakscaling_smallEAM_8x16/CoMD-mpi -x 96 -y 48 -z 48 -i 8 -j 4 -k 4 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 128 weakscaling_smallEAM_8x16/CoMD-mpi -x 96 -y 48 -z 48 -i 8 -j 4 -k 4 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 8 24
srun --mpi=pmi2 -n 192 weakscaling_smallEAM_8x24/CoMD-mpi -x 96 -y 72 -z 48 -i 8 -j 6 -k 4 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 192 weakscaling_smallEAM_8x24/CoMD-mpi -x 96 -y 72 -z 48 -i 8 -j 6 -k 4 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumLJ 8 1
srun --mpi=pmi2 -n 8 weakscaling_mediumLJ_8x1/CoMD-mpi -x 48 -y 48 -z 48 -i 2 -j 2 -k 2 -N 100 -n 10
srun --mpi=pmi2 -n 8 weakscaling_mediumLJ_8x1/CoMD-mpi -x 48 -y 48 -z 48 -i 2 -j 2 -k 2 -N 100 -n 10

echo weakscaling mediumLJ 8 4
srun --mpi=pmi2 -n 32 weakscaling_mediumLJ_8x4/CoMD-mpi -x 96 -y 96 -z 48 -i 4 -j 4 -k 2 -N 100 -n 10
srun --mpi=pmi2 -n 32 weakscaling_mediumLJ_8x4/CoMD-mpi -x 96 -y 96 -z 48 -i 4 -j 4 -k 2 -N 100 -n 10

echo weakscaling mediumLJ 8 8
srun --mpi=pmi2 -n 64 weakscaling_mediumLJ_8x8/CoMD-mpi -x 96 -y 96 -z 96 -i 4 -j 4 -k 4 -N 100 -n 10
srun --mpi=pmi2 -n 64 weakscaling_mediumLJ_8x8/CoMD-mpi -x 96 -y 96 -z 96 -i 4 -j 4 -k 4 -N 100 -n 10

echo weakscaling mediumLJ 8 16
srun --mpi=pmi2 -n 128 weakscaling_mediumLJ_8x16/CoMD-mpi -x 192 -y 96 -z 96 -i 8 -j 4 -k 4 -N 100 -n 10
srun --mpi=pmi2 -n 128 weakscaling_mediumLJ_8x16/CoMD-mpi -x 192 -y 96 -z 96 -i 8 -j 4 -k 4 -N 100 -n 10

echo weakscaling mediumLJ 8 24
srun --mpi=pmi2 -n 192 weakscaling_mediumLJ_8x24/CoMD-mpi -x 192 -y 144 -z 96 -i 8 -j 6 -k 4 -N 100 -n 10
srun --mpi=pmi2 -n 192 weakscaling_mediumLJ_8x24/CoMD-mpi -x 192 -y 144 -z 96 -i 8 -j 6 -k 4 -N 100 -n 10

echo weakscaling mediumEAM 8 1
srun --mpi=pmi2 -n 8 weakscaling_mediumEAM_8x1/CoMD-mpi -x 48 -y 48 -z 48 -i 2 -j 2 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 8 weakscaling_mediumEAM_8x1/CoMD-mpi -x 48 -y 48 -z 48 -i 2 -j 2 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 8 4
srun --mpi=pmi2 -n 32 weakscaling_mediumEAM_8x4/CoMD-mpi -x 96 -y 96 -z 48 -i 4 -j 4 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 32 weakscaling_mediumEAM_8x4/CoMD-mpi -x 96 -y 96 -z 48 -i 4 -j 4 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 8 8
srun --mpi=pmi2 -n 64 weakscaling_mediumEAM_8x8/CoMD-mpi -x 96 -y 96 -z 96 -i 4 -j 4 -k 4 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 64 weakscaling_mediumEAM_8x8/CoMD-mpi -x 96 -y 96 -z 96 -i 4 -j 4 -k 4 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 8 16
srun --mpi=pmi2 -n 128 weakscaling_mediumEAM_8x16/CoMD-mpi -x 192 -y 96 -z 96 -i 8 -j 4 -k 4 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 128 weakscaling_mediumEAM_8x16/CoMD-mpi -x 192 -y 96 -z 96 -i 8 -j 4 -k 4 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 8 24
srun --mpi=pmi2 -n 192 weakscaling_mediumEAM_8x24/CoMD-mpi -x 192 -y 144 -z 96 -i 8 -j 6 -k 4 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 192 weakscaling_mediumEAM_8x24/CoMD-mpi -x 192 -y 144 -z 96 -i 8 -j 6 -k 4 -N 100 -n 10 -e -d ../../../datasets/pots

