#!/bin/bash -l
#SBATCH -p regular
#SBATCH -N 64
#SBATCH -t 4:00:00
#SBATCH -J JOB_legacy_CoMD_64.out
#SBATCH -o JOB_legacy_CoMD_64.out
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


echo weakscaling smallLJ 64 1
srun --mpi=pmi2 -n 64 weakscaling_smallLJ_64x1/CoMD-mpi -x 48 -y 48 -z 48 -i 4 -j 4 -k 4 -N 100 -n 10
srun --mpi=pmi2 -n 64 weakscaling_smallLJ_64x1/CoMD-mpi -x 48 -y 48 -z 48 -i 4 -j 4 -k 4 -N 100 -n 10

echo weakscaling smallLJ 64 4
srun --mpi=pmi2 -n 256 weakscaling_smallLJ_64x4/CoMD-mpi -x 96 -y 96 -z 48 -i 8 -j 8 -k 4 -N 100 -n 10
srun --mpi=pmi2 -n 256 weakscaling_smallLJ_64x4/CoMD-mpi -x 96 -y 96 -z 48 -i 8 -j 8 -k 4 -N 100 -n 10

echo weakscaling smallLJ 64 8
srun --mpi=pmi2 -n 512 weakscaling_smallLJ_64x8/CoMD-mpi -x 96 -y 96 -z 96 -i 8 -j 8 -k 8 -N 100 -n 10
srun --mpi=pmi2 -n 512 weakscaling_smallLJ_64x8/CoMD-mpi -x 96 -y 96 -z 96 -i 8 -j 8 -k 8 -N 100 -n 10

echo weakscaling smallLJ 64 16
srun --mpi=pmi2 -n 1024 weakscaling_smallLJ_64x16/CoMD-mpi -x 192 -y 96 -z 96 -i 16 -j 8 -k 8 -N 100 -n 10
srun --mpi=pmi2 -n 1024 weakscaling_smallLJ_64x16/CoMD-mpi -x 192 -y 96 -z 96 -i 16 -j 8 -k 8 -N 100 -n 10

echo weakscaling smallLJ 64 24
srun --mpi=pmi2 -n 1536 weakscaling_smallLJ_64x24/CoMD-mpi -x 192 -y 144 -z 96 -i 16 -j 12 -k 8 -N 100 -n 10
srun --mpi=pmi2 -n 1536 weakscaling_smallLJ_64x24/CoMD-mpi -x 192 -y 144 -z 96 -i 16 -j 12 -k 8 -N 100 -n 10

echo weakscaling smallEAM 64 1
srun --mpi=pmi2 -n 64 weakscaling_smallEAM_64x1/CoMD-mpi -x 48 -y 48 -z 48 -i 4 -j 4 -k 4 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 64 weakscaling_smallEAM_64x1/CoMD-mpi -x 48 -y 48 -z 48 -i 4 -j 4 -k 4 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 64 4
srun --mpi=pmi2 -n 256 weakscaling_smallEAM_64x4/CoMD-mpi -x 96 -y 96 -z 48 -i 8 -j 8 -k 4 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 256 weakscaling_smallEAM_64x4/CoMD-mpi -x 96 -y 96 -z 48 -i 8 -j 8 -k 4 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 64 8
srun --mpi=pmi2 -n 512 weakscaling_smallEAM_64x8/CoMD-mpi -x 96 -y 96 -z 96 -i 8 -j 8 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 512 weakscaling_smallEAM_64x8/CoMD-mpi -x 96 -y 96 -z 96 -i 8 -j 8 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 64 16
srun --mpi=pmi2 -n 1024 weakscaling_smallEAM_64x16/CoMD-mpi -x 192 -y 96 -z 96 -i 16 -j 8 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 1024 weakscaling_smallEAM_64x16/CoMD-mpi -x 192 -y 96 -z 96 -i 16 -j 8 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 64 24
srun --mpi=pmi2 -n 1536 weakscaling_smallEAM_64x24/CoMD-mpi -x 192 -y 144 -z 96 -i 16 -j 12 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 1536 weakscaling_smallEAM_64x24/CoMD-mpi -x 192 -y 144 -z 96 -i 16 -j 12 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumLJ 64 1
srun --mpi=pmi2 -n 64 weakscaling_mediumLJ_64x1/CoMD-mpi -x 96 -y 96 -z 96 -i 4 -j 4 -k 4 -N 100 -n 10
srun --mpi=pmi2 -n 64 weakscaling_mediumLJ_64x1/CoMD-mpi -x 96 -y 96 -z 96 -i 4 -j 4 -k 4 -N 100 -n 10

echo weakscaling mediumLJ 64 4
srun --mpi=pmi2 -n 256 weakscaling_mediumLJ_64x4/CoMD-mpi -x 192 -y 192 -z 96 -i 8 -j 8 -k 4 -N 100 -n 10
srun --mpi=pmi2 -n 256 weakscaling_mediumLJ_64x4/CoMD-mpi -x 192 -y 192 -z 96 -i 8 -j 8 -k 4 -N 100 -n 10

echo weakscaling mediumLJ 64 8
srun --mpi=pmi2 -n 512 weakscaling_mediumLJ_64x8/CoMD-mpi -x 192 -y 192 -z 192 -i 8 -j 8 -k 8 -N 100 -n 10
srun --mpi=pmi2 -n 512 weakscaling_mediumLJ_64x8/CoMD-mpi -x 192 -y 192 -z 192 -i 8 -j 8 -k 8 -N 100 -n 10

echo weakscaling mediumLJ 64 16
srun --mpi=pmi2 -n 1024 weakscaling_mediumLJ_64x16/CoMD-mpi -x 384 -y 192 -z 192 -i 16 -j 8 -k 8 -N 100 -n 10
srun --mpi=pmi2 -n 1024 weakscaling_mediumLJ_64x16/CoMD-mpi -x 384 -y 192 -z 192 -i 16 -j 8 -k 8 -N 100 -n 10

echo weakscaling mediumLJ 64 24
srun --mpi=pmi2 -n 1536 weakscaling_mediumLJ_64x24/CoMD-mpi -x 384 -y 288 -z 192 -i 16 -j 12 -k 8 -N 100 -n 10
srun --mpi=pmi2 -n 1536 weakscaling_mediumLJ_64x24/CoMD-mpi -x 384 -y 288 -z 192 -i 16 -j 12 -k 8 -N 100 -n 10

echo weakscaling mediumEAM 64 1
srun --mpi=pmi2 -n 64 weakscaling_mediumEAM_64x1/CoMD-mpi -x 96 -y 96 -z 96 -i 4 -j 4 -k 4 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 64 weakscaling_mediumEAM_64x1/CoMD-mpi -x 96 -y 96 -z 96 -i 4 -j 4 -k 4 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 64 4
srun --mpi=pmi2 -n 256 weakscaling_mediumEAM_64x4/CoMD-mpi -x 192 -y 192 -z 96 -i 8 -j 8 -k 4 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 256 weakscaling_mediumEAM_64x4/CoMD-mpi -x 192 -y 192 -z 96 -i 8 -j 8 -k 4 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 64 8
srun --mpi=pmi2 -n 512 weakscaling_mediumEAM_64x8/CoMD-mpi -x 192 -y 192 -z 192 -i 8 -j 8 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 512 weakscaling_mediumEAM_64x8/CoMD-mpi -x 192 -y 192 -z 192 -i 8 -j 8 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 64 16
srun --mpi=pmi2 -n 1024 weakscaling_mediumEAM_64x16/CoMD-mpi -x 384 -y 192 -z 192 -i 16 -j 8 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 1024 weakscaling_mediumEAM_64x16/CoMD-mpi -x 384 -y 192 -z 192 -i 16 -j 8 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 64 24
srun --mpi=pmi2 -n 1536 weakscaling_mediumEAM_64x24/CoMD-mpi -x 384 -y 288 -z 192 -i 16 -j 12 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots
srun --mpi=pmi2 -n 1536 weakscaling_mediumEAM_64x24/CoMD-mpi -x 384 -y 288 -z 192 -i 16 -j 12 -k 8 -N 100 -n 10 -e -d ../../../datasets/pots

