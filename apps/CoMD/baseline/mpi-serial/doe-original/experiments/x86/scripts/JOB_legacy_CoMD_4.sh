#!/bin/bash -l
#SBATCH -p regular
#SBATCH -N 4
#SBATCH -t 4:00:00
#SBATCH -J JOB_legacy_CoMD_4.out
#SBATCH -o JOB_legacy_CoMD_4.out
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


echo weakscaling smallLJ 4 1
srun --mpi=pmi2 -n 4 weakscaling_smallLJ_4x1/CoMD-mpi -x 24 -y 24 -z 12 -i 2 -j 2 -k 1 -N 100 -n 10
srun --mpi=pmi2 -n 4 weakscaling_smallLJ_4x1/CoMD-mpi -x 24 -y 24 -z 12 -i 2 -j 2 -k 1 -N 100 -n 10

echo weakscaling smallLJ 4 4
srun --mpi=pmi2 -n 16 weakscaling_smallLJ_4x4/CoMD-mpi -x 48 -y 48 -z 12 -i 4 -j 4 -k 1 -N 100 -n 10
srun --mpi=pmi2 -n 16 weakscaling_smallLJ_4x4/CoMD-mpi -x 48 -y 48 -z 12 -i 4 -j 4 -k 1 -N 100 -n 10

echo weakscaling smallLJ 4 8
srun --mpi=pmi2 -n 32 weakscaling_smallLJ_4x8/CoMD-mpi -x 48 -y 48 -z 24 -i 4 -j 4 -k 2 -N 100 -n 10
srun --mpi=pmi2 -n 32 weakscaling_smallLJ_4x8/CoMD-mpi -x 48 -y 48 -z 24 -i 4 -j 4 -k 2 -N 100 -n 10

echo weakscaling smallLJ 4 16
srun --mpi=pmi2 -n 64 weakscaling_smallLJ_4x16/CoMD-mpi -x 96 -y 48 -z 24 -i 8 -j 4 -k 2 -N 100 -n 10
srun --mpi=pmi2 -n 64 weakscaling_smallLJ_4x16/CoMD-mpi -x 96 -y 48 -z 24 -i 8 -j 4 -k 2 -N 100 -n 10

echo weakscaling smallLJ 4 24
srun --mpi=pmi2 -n 96 weakscaling_smallLJ_4x24/CoMD-mpi -x 96 -y 72 -z 24 -i 8 -j 6 -k 2 -N 100 -n 10
srun --mpi=pmi2 -n 96 weakscaling_smallLJ_4x24/CoMD-mpi -x 96 -y 72 -z 24 -i 8 -j 6 -k 2 -N 100 -n 10

echo weakscaling smallEAM 4 1
srun --mpi=pmi2 -n 4 weakscaling_smallEAM_4x1/CoMD-mpi -x 24 -y 24 -z 12 -i 2 -j 2 -k 1 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 4 weakscaling_smallEAM_4x1/CoMD-mpi -x 24 -y 24 -z 12 -i 2 -j 2 -k 1 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 4 4
srun --mpi=pmi2 -n 16 weakscaling_smallEAM_4x4/CoMD-mpi -x 48 -y 48 -z 12 -i 4 -j 4 -k 1 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 16 weakscaling_smallEAM_4x4/CoMD-mpi -x 48 -y 48 -z 12 -i 4 -j 4 -k 1 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 4 8
srun --mpi=pmi2 -n 32 weakscaling_smallEAM_4x8/CoMD-mpi -x 48 -y 48 -z 24 -i 4 -j 4 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 32 weakscaling_smallEAM_4x8/CoMD-mpi -x 48 -y 48 -z 24 -i 4 -j 4 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 4 16
srun --mpi=pmi2 -n 64 weakscaling_smallEAM_4x16/CoMD-mpi -x 96 -y 48 -z 24 -i 8 -j 4 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 64 weakscaling_smallEAM_4x16/CoMD-mpi -x 96 -y 48 -z 24 -i 8 -j 4 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 4 24
srun --mpi=pmi2 -n 96 weakscaling_smallEAM_4x24/CoMD-mpi -x 96 -y 72 -z 24 -i 8 -j 6 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 96 weakscaling_smallEAM_4x24/CoMD-mpi -x 96 -y 72 -z 24 -i 8 -j 6 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumLJ 4 1
srun --mpi=pmi2 -n 4 weakscaling_mediumLJ_4x1/CoMD-mpi -x 48 -y 48 -z 24 -i 2 -j 2 -k 1 -N 100 -n 10
srun --mpi=pmi2 -n 4 weakscaling_mediumLJ_4x1/CoMD-mpi -x 48 -y 48 -z 24 -i 2 -j 2 -k 1 -N 100 -n 10

echo weakscaling mediumLJ 4 4
srun --mpi=pmi2 -n 16 weakscaling_mediumLJ_4x4/CoMD-mpi -x 96 -y 96 -z 24 -i 4 -j 4 -k 1 -N 100 -n 10
srun --mpi=pmi2 -n 16 weakscaling_mediumLJ_4x4/CoMD-mpi -x 96 -y 96 -z 24 -i 4 -j 4 -k 1 -N 100 -n 10

echo weakscaling mediumLJ 4 8
srun --mpi=pmi2 -n 32 weakscaling_mediumLJ_4x8/CoMD-mpi -x 96 -y 96 -z 48 -i 4 -j 4 -k 2 -N 100 -n 10
srun --mpi=pmi2 -n 32 weakscaling_mediumLJ_4x8/CoMD-mpi -x 96 -y 96 -z 48 -i 4 -j 4 -k 2 -N 100 -n 10

echo weakscaling mediumLJ 4 16
srun --mpi=pmi2 -n 64 weakscaling_mediumLJ_4x16/CoMD-mpi -x 192 -y 96 -z 48 -i 8 -j 4 -k 2 -N 100 -n 10
srun --mpi=pmi2 -n 64 weakscaling_mediumLJ_4x16/CoMD-mpi -x 192 -y 96 -z 48 -i 8 -j 4 -k 2 -N 100 -n 10

echo weakscaling mediumLJ 4 24
srun --mpi=pmi2 -n 96 weakscaling_mediumLJ_4x24/CoMD-mpi -x 192 -y 144 -z 48 -i 8 -j 6 -k 2 -N 100 -n 10
srun --mpi=pmi2 -n 96 weakscaling_mediumLJ_4x24/CoMD-mpi -x 192 -y 144 -z 48 -i 8 -j 6 -k 2 -N 100 -n 10

echo weakscaling mediumEAM 4 1
srun --mpi=pmi2 -n 4 weakscaling_mediumEAM_4x1/CoMD-mpi -x 48 -y 48 -z 24 -i 2 -j 2 -k 1 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 4 weakscaling_mediumEAM_4x1/CoMD-mpi -x 48 -y 48 -z 24 -i 2 -j 2 -k 1 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 4 4
srun --mpi=pmi2 -n 16 weakscaling_mediumEAM_4x4/CoMD-mpi -x 96 -y 96 -z 24 -i 4 -j 4 -k 1 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 16 weakscaling_mediumEAM_4x4/CoMD-mpi -x 96 -y 96 -z 24 -i 4 -j 4 -k 1 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 4 8
srun --mpi=pmi2 -n 32 weakscaling_mediumEAM_4x8/CoMD-mpi -x 96 -y 96 -z 48 -i 4 -j 4 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 32 weakscaling_mediumEAM_4x8/CoMD-mpi -x 96 -y 96 -z 48 -i 4 -j 4 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 4 16
srun --mpi=pmi2 -n 64 weakscaling_mediumEAM_4x16/CoMD-mpi -x 192 -y 96 -z 48 -i 8 -j 4 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 64 weakscaling_mediumEAM_4x16/CoMD-mpi -x 192 -y 96 -z 48 -i 8 -j 4 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 4 24
srun --mpi=pmi2 -n 96 weakscaling_mediumEAM_4x24/CoMD-mpi -x 192 -y 144 -z 48 -i 8 -j 6 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 96 weakscaling_mediumEAM_4x24/CoMD-mpi -x 192 -y 144 -z 48 -i 8 -j 6 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots

