#!/bin/bash -l
#SBATCH -p regular
#SBATCH -N 216
#SBATCH -t 4:00:00
#SBATCH -J JOB_MC_CoMD_216.out
#SBATCH -o JOB_MC_CoMD_216.out
#SBATCH -L SCRATCH

export RUN_MODE=runApp
export OCRRUN_OPT_ENVKIND=SLURM

#For slurm run with Intel MPI the following is needed
#module load PrgEnv-intel
module swap PrgEnv-intel PrgEnv-cray
#module load impi
#
#if [[ `hostname` == edison* ]]; then
#    export I_MPI_PMI_LIBRARY=/opt/slurm/default/lib/pmi/libpmi.so
#elif [[ `hostname` == cori* ]]; then
#    export I_MPI_PMI_LIBRARY=/usr/lib64/slurmpmi/libmpi.so
#fi

cd $SLURM_SUBMIT_DIR


echo weakscaling smallLJ 216 1
srun  -n 216 weakscaling_smallLJ_216x1/CoMD-mpi_cray -x 72 -y 72 -z 72 -i 6 -j 6 -k 6 -N 100 -n 10
srun  -n 216 weakscaling_smallLJ_216x1/CoMD-mpi_cray -x 72 -y 72 -z 72 -i 6 -j 6 -k 6 -N 100 -n 10

echo weakscaling smallLJ 216 4
srun  -n 864 weakscaling_smallLJ_216x4/CoMD-mpi_cray -x 144 -y 144 -z 72 -i 12 -j 12 -k 6 -N 100 -n 10
srun  -n 864 weakscaling_smallLJ_216x4/CoMD-mpi_cray -x 144 -y 144 -z 72 -i 12 -j 12 -k 6 -N 100 -n 10

echo weakscaling smallLJ 216 8
srun  -n 1728 weakscaling_smallLJ_216x8/CoMD-mpi_cray -x 144 -y 144 -z 144 -i 12 -j 12 -k 12 -N 100 -n 10
srun  -n 1728 weakscaling_smallLJ_216x8/CoMD-mpi_cray -x 144 -y 144 -z 144 -i 12 -j 12 -k 12 -N 100 -n 10

echo weakscaling smallLJ 216 16
srun  -n 3456 weakscaling_smallLJ_216x16/CoMD-mpi_cray -x 288 -y 144 -z 144 -i 24 -j 12 -k 12 -N 100 -n 10
srun  -n 3456 weakscaling_smallLJ_216x16/CoMD-mpi_cray -x 288 -y 144 -z 144 -i 24 -j 12 -k 12 -N 100 -n 10

echo weakscaling smallLJ 216 24
srun  -n 5184 weakscaling_smallLJ_216x24/CoMD-mpi_cray -x 288 -y 216 -z 144 -i 24 -j 18 -k 12 -N 100 -n 10
srun  -n 5184 weakscaling_smallLJ_216x24/CoMD-mpi_cray -x 288 -y 216 -z 144 -i 24 -j 18 -k 12 -N 100 -n 10

echo weakscaling smallEAM 216 1
srun  -n 216 weakscaling_smallEAM_216x1/CoMD-mpi_cray -x 72 -y 72 -z 72 -i 6 -j 6 -k 6 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 216 weakscaling_smallEAM_216x1/CoMD-mpi_cray -x 72 -y 72 -z 72 -i 6 -j 6 -k 6 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 216 4
srun  -n 864 weakscaling_smallEAM_216x4/CoMD-mpi_cray -x 144 -y 144 -z 72 -i 12 -j 12 -k 6 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 864 weakscaling_smallEAM_216x4/CoMD-mpi_cray -x 144 -y 144 -z 72 -i 12 -j 12 -k 6 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 216 8
srun  -n 1728 weakscaling_smallEAM_216x8/CoMD-mpi_cray -x 144 -y 144 -z 144 -i 12 -j 12 -k 12 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 1728 weakscaling_smallEAM_216x8/CoMD-mpi_cray -x 144 -y 144 -z 144 -i 12 -j 12 -k 12 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 216 16
srun  -n 3456 weakscaling_smallEAM_216x16/CoMD-mpi_cray -x 288 -y 144 -z 144 -i 24 -j 12 -k 12 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 3456 weakscaling_smallEAM_216x16/CoMD-mpi_cray -x 288 -y 144 -z 144 -i 24 -j 12 -k 12 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling smallEAM 216 24
srun  -n 5184 weakscaling_smallEAM_216x24/CoMD-mpi_cray -x 288 -y 216 -z 144 -i 24 -j 18 -k 12 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 5184 weakscaling_smallEAM_216x24/CoMD-mpi_cray -x 288 -y 216 -z 144 -i 24 -j 18 -k 12 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumLJ 216 1
srun  -n 216 weakscaling_mediumLJ_216x1/CoMD-mpi_cray -x 144 -y 144 -z 144 -i 6 -j 6 -k 6 -N 100 -n 10
srun  -n 216 weakscaling_mediumLJ_216x1/CoMD-mpi_cray -x 144 -y 144 -z 144 -i 6 -j 6 -k 6 -N 100 -n 10

echo weakscaling mediumLJ 216 4
srun  -n 864 weakscaling_mediumLJ_216x4/CoMD-mpi_cray -x 288 -y 288 -z 144 -i 12 -j 12 -k 6 -N 100 -n 10
srun  -n 864 weakscaling_mediumLJ_216x4/CoMD-mpi_cray -x 288 -y 288 -z 144 -i 12 -j 12 -k 6 -N 100 -n 10

echo weakscaling mediumLJ 216 8
srun  -n 1728 weakscaling_mediumLJ_216x8/CoMD-mpi_cray -x 288 -y 288 -z 288 -i 12 -j 12 -k 12 -N 100 -n 10
srun  -n 1728 weakscaling_mediumLJ_216x8/CoMD-mpi_cray -x 288 -y 288 -z 288 -i 12 -j 12 -k 12 -N 100 -n 10

echo weakscaling mediumLJ 216 16
srun  -n 3456 weakscaling_mediumLJ_216x16/CoMD-mpi_cray -x 576 -y 288 -z 288 -i 24 -j 12 -k 12 -N 100 -n 10
srun  -n 3456 weakscaling_mediumLJ_216x16/CoMD-mpi_cray -x 576 -y 288 -z 288 -i 24 -j 12 -k 12 -N 100 -n 10

echo weakscaling mediumLJ 216 24
srun  -n 5184 weakscaling_mediumLJ_216x24/CoMD-mpi_cray -x 576 -y 432 -z 288 -i 24 -j 18 -k 12 -N 100 -n 10
srun  -n 5184 weakscaling_mediumLJ_216x24/CoMD-mpi_cray -x 576 -y 432 -z 288 -i 24 -j 18 -k 12 -N 100 -n 10

echo weakscaling mediumEAM 216 1
srun  -n 216 weakscaling_mediumEAM_216x1/CoMD-mpi_cray -x 144 -y 144 -z 144 -i 6 -j 6 -k 6 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 216 weakscaling_mediumEAM_216x1/CoMD-mpi_cray -x 144 -y 144 -z 144 -i 6 -j 6 -k 6 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 216 4
srun  -n 864 weakscaling_mediumEAM_216x4/CoMD-mpi_cray -x 288 -y 288 -z 144 -i 12 -j 12 -k 6 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 864 weakscaling_mediumEAM_216x4/CoMD-mpi_cray -x 288 -y 288 -z 144 -i 12 -j 12 -k 6 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 216 8
srun  -n 1728 weakscaling_mediumEAM_216x8/CoMD-mpi_cray -x 288 -y 288 -z 288 -i 12 -j 12 -k 12 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 1728 weakscaling_mediumEAM_216x8/CoMD-mpi_cray -x 288 -y 288 -z 288 -i 12 -j 12 -k 12 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 216 16
srun  -n 3456 weakscaling_mediumEAM_216x16/CoMD-mpi_cray -x 576 -y 288 -z 288 -i 24 -j 12 -k 12 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 3456 weakscaling_mediumEAM_216x16/CoMD-mpi_cray -x 576 -y 288 -z 288 -i 24 -j 12 -k 12 -N 100 -n 10 -e -d ../../../datasets/pots

echo weakscaling mediumEAM 216 24
srun  -n 5184 weakscaling_mediumEAM_216x24/CoMD-mpi_cray -x 576 -y 432 -z 288 -i 24 -j 18 -k 12 -N 100 -n 10 -e -d ../../../datasets/pots
srun  -n 5184 weakscaling_mediumEAM_216x24/CoMD-mpi_cray -x 576 -y 432 -z 288 -i 24 -j 18 -k 12 -N 100 -n 10 -e -d ../../../datasets/pots

