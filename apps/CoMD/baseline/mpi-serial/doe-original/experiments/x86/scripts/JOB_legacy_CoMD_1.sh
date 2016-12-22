#!/bin/bash -l
#SBATCH -p regular
#SBATCH -N 1
#SBATCH -t 4:00:00
#SBATCH -J JOB_legacy_CoMD_1.out
#SBATCH -o JOB_legacy_CoMD_1.out
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


echo weakscaling smallLJ 1 1
srun --mpi=pmi2 -n 1 weakscaling_smallLJ_1x1/CoMD-mpi -x 12 -y 12 -z 12 -i 1 -j 1 -k 1 -N 100 -n 10
srun --mpi=pmi2 -n 1 weakscaling_smallLJ_1x1/CoMD-mpi -x 12 -y 12 -z 12 -i 1 -j 1 -k 1 -N 100 -n 10

echo weakscaling smallLJ 1 4
srun --mpi=pmi2 -n 4 weakscaling_smallLJ_1x4/CoMD-mpi -x 24 -y 24 -z 12 -i 2 -j 2 -k 1 -N 100 -n 10
srun --mpi=pmi2 -n 4 weakscaling_smallLJ_1x4/CoMD-mpi -x 24 -y 24 -z 12 -i 2 -j 2 -k 1 -N 100 -n 10

echo weakscaling smallLJ 1 8
srun --mpi=pmi2 -n 8 weakscaling_smallLJ_1x8/CoMD-mpi -x 24 -y 24 -z 24 -i 2 -j 2 -k 2 -N 100 -n 10
srun --mpi=pmi2 -n 8 weakscaling_smallLJ_1x8/CoMD-mpi -x 24 -y 24 -z 24 -i 2 -j 2 -k 2 -N 100 -n 10

echo weakscaling smallLJ 1 16
srun --mpi=pmi2 -n 16 weakscaling_smallLJ_1x16/CoMD-mpi -x 48 -y 24 -z 24 -i 4 -j 2 -k 2 -N 100 -n 10
srun --mpi=pmi2 -n 16 weakscaling_smallLJ_1x16/CoMD-mpi -x 48 -y 24 -z 24 -i 4 -j 2 -k 2 -N 100 -n 10

echo weakscaling smallLJ 1 24
srun --mpi=pmi2 -n 24 weakscaling_smallLJ_1x24/CoMD-mpi -x 48 -y 36 -z 24 -i 4 -j 3 -k 2 -N 100 -n 10
srun --mpi=pmi2 -n 24 weakscaling_smallLJ_1x24/CoMD-mpi -x 48 -y 36 -z 24 -i 4 -j 3 -k 2 -N 100 -n 10

echo weakscaling smallEAM 1 1
srun --mpi=pmi2 -n 1 weakscaling_smallEAM_1x1/CoMD-mpi -x 12 -y 12 -z 12 -i 1 -j 1 -k 1 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 1 weakscaling_smallEAM_1x1/CoMD-mpi -x 12 -y 12 -z 12 -i 1 -j 1 -k 1 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 1 4
srun --mpi=pmi2 -n 4 weakscaling_smallEAM_1x4/CoMD-mpi -x 24 -y 24 -z 12 -i 2 -j 2 -k 1 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 4 weakscaling_smallEAM_1x4/CoMD-mpi -x 24 -y 24 -z 12 -i 2 -j 2 -k 1 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 1 8
srun --mpi=pmi2 -n 8 weakscaling_smallEAM_1x8/CoMD-mpi -x 24 -y 24 -z 24 -i 2 -j 2 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 8 weakscaling_smallEAM_1x8/CoMD-mpi -x 24 -y 24 -z 24 -i 2 -j 2 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 1 16
srun --mpi=pmi2 -n 16 weakscaling_smallEAM_1x16/CoMD-mpi -x 48 -y 24 -z 24 -i 4 -j 2 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 16 weakscaling_smallEAM_1x16/CoMD-mpi -x 48 -y 24 -z 24 -i 4 -j 2 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 1 24
srun --mpi=pmi2 -n 24 weakscaling_smallEAM_1x24/CoMD-mpi -x 48 -y 36 -z 24 -i 4 -j 3 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 24 weakscaling_smallEAM_1x24/CoMD-mpi -x 48 -y 36 -z 24 -i 4 -j 3 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumLJ 1 1
srun --mpi=pmi2 -n 1 weakscaling_mediumLJ_1x1/CoMD-mpi -x 24 -y 24 -z 24 -i 1 -j 1 -k 1 -N 100 -n 10
srun --mpi=pmi2 -n 1 weakscaling_mediumLJ_1x1/CoMD-mpi -x 24 -y 24 -z 24 -i 1 -j 1 -k 1 -N 100 -n 10

echo weakscaling mediumLJ 1 4
srun --mpi=pmi2 -n 4 weakscaling_mediumLJ_1x4/CoMD-mpi -x 48 -y 48 -z 24 -i 2 -j 2 -k 1 -N 100 -n 10
srun --mpi=pmi2 -n 4 weakscaling_mediumLJ_1x4/CoMD-mpi -x 48 -y 48 -z 24 -i 2 -j 2 -k 1 -N 100 -n 10

echo weakscaling mediumLJ 1 8
srun --mpi=pmi2 -n 8 weakscaling_mediumLJ_1x8/CoMD-mpi -x 48 -y 48 -z 48 -i 2 -j 2 -k 2 -N 100 -n 10
srun --mpi=pmi2 -n 8 weakscaling_mediumLJ_1x8/CoMD-mpi -x 48 -y 48 -z 48 -i 2 -j 2 -k 2 -N 100 -n 10

echo weakscaling mediumLJ 1 16
srun --mpi=pmi2 -n 16 weakscaling_mediumLJ_1x16/CoMD-mpi -x 96 -y 48 -z 48 -i 4 -j 2 -k 2 -N 100 -n 10
srun --mpi=pmi2 -n 16 weakscaling_mediumLJ_1x16/CoMD-mpi -x 96 -y 48 -z 48 -i 4 -j 2 -k 2 -N 100 -n 10

echo weakscaling mediumLJ 1 24
srun --mpi=pmi2 -n 24 weakscaling_mediumLJ_1x24/CoMD-mpi -x 96 -y 72 -z 48 -i 4 -j 3 -k 2 -N 100 -n 10
srun --mpi=pmi2 -n 24 weakscaling_mediumLJ_1x24/CoMD-mpi -x 96 -y 72 -z 48 -i 4 -j 3 -k 2 -N 100 -n 10

echo weakscaling mediumEAM 1 1
srun --mpi=pmi2 -n 1 weakscaling_mediumEAM_1x1/CoMD-mpi -x 24 -y 24 -z 24 -i 1 -j 1 -k 1 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 1 weakscaling_mediumEAM_1x1/CoMD-mpi -x 24 -y 24 -z 24 -i 1 -j 1 -k 1 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 1 4
srun --mpi=pmi2 -n 4 weakscaling_mediumEAM_1x4/CoMD-mpi -x 48 -y 48 -z 24 -i 2 -j 2 -k 1 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 4 weakscaling_mediumEAM_1x4/CoMD-mpi -x 48 -y 48 -z 24 -i 2 -j 2 -k 1 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 1 8
srun --mpi=pmi2 -n 8 weakscaling_mediumEAM_1x8/CoMD-mpi -x 48 -y 48 -z 48 -i 2 -j 2 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 8 weakscaling_mediumEAM_1x8/CoMD-mpi -x 48 -y 48 -z 48 -i 2 -j 2 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 1 16
srun --mpi=pmi2 -n 16 weakscaling_mediumEAM_1x16/CoMD-mpi -x 96 -y 48 -z 48 -i 4 -j 2 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 16 weakscaling_mediumEAM_1x16/CoMD-mpi -x 96 -y 48 -z 48 -i 4 -j 2 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 1 24
srun --mpi=pmi2 -n 24 weakscaling_mediumEAM_1x24/CoMD-mpi -x 96 -y 72 -z 48 -i 4 -j 3 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 24 weakscaling_mediumEAM_1x24/CoMD-mpi -x 96 -y 72 -z 48 -i 4 -j 3 -k 2 -N 100 -n 10 -e -d ../../../datasets/pots

