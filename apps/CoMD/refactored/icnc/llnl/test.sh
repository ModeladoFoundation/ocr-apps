#!/bin/bash
#export DIST_CNC=MPI

rm output/*.itr
#./runmd.sh -f $1 -i ../../../datasets/lammps-/inputfiles/13atoms_fff_v1_dump10.in -o output/ -x 1 -y 1 -z 1 -I 1000 -P 1 -T 0.001
#./runmd.sh -f $1 -i ../../../datasets/lammps-/inputfiles/13atoms_fff_v1_dump10.in -o output/ -x 2 -y 2 -z 2 -I 1000 -P 1 -T 0.001
#./runmd.sh -f $1 -i ../../../datasets/lammps-/inputfiles/13atoms_fff_v1_dump10.in -o output/ -x 3 -y 3 -z 3 -I 1000 -P 1 -T 0.001
#./runmd.sh -f $1 -i ../../../datasets/lammps-/inputfiles/55atoms_fff_v1_dump10.in -o output/ -x 3 -y 3 -z 3 -I 1000 -P 1 -T 0.001
#./runmd.sh -f $1 -i ../../../datasets/lammps-/inputfiles/64atoms_fff_v1_dump1.in -o output/ -x 3 -y 3 -z 3 -I 1000 -P 1 -T 0.001
#./runmd.sh -f $1 -i ../../../datasets/lammps-/inputfiles/7atoms_fff_v3_dump1.in -o output/ -x 3 -y 3 -z 3 -I 100 -P 1 -T 0.001
#./runmd.sh -f $1 -i ../../../datasets/lammps-/inputfiles/351atoms_fff_v1_dump100.in -o output/ -x 3 -y 3 -z 3 -I 10000 -P 100 -T 0.001
#./runmd.sh -f $1 -i ../../../datasets/lammps-/inputfiles/440atoms_fff_v1_dump100.in -o output/ -x 3 -y 3 -z 3 -I 11200 -P 100 -T 0.001
#./runmd.sh -f $1 -i ../../../datasets/lammps-/inputfiles/864atoms_fff_v1_dump10.in -o output/ -x 3 -y 3 -z 3 -I 1000 -P 10 -T 0.001
./runmd.sh -f $1 -i ../../../datasets/lammps-inputfiles/13atoms_fff_v1_dump10.in -o output/ -x 1 -y 1 -z 1 -I 1000 -P 1 -T 0.001
#./runmd.sh -f $1 -i inputfiles/864atoms_fff_v300_dump10_realunits.in -o output/ -e 0.167 -s 2.315 -c 5.7875 -x 3 -y 3 -z 3 -I 1000 -P 10 -T 0.001


#./runmd.sh -f $1 -i ../../../datasets/lammps-/inputfiles/64atoms_ppp_T1_dump1.in -o output/ -x 1 -y 1 -z 1 -I 100 -P 1 -T 0.01
#./runmd.sh -f $1 -i ../../../datasets/lammps-/inputfiles/64atoms_ppp_T1_dump1.in -o output/ -x 2 -y 2 -z 2 -I 100 -P 1 -T 0.01
#./runmd.sh -f $1 -i ../../../datasets/lammps-/inputfiles/64atoms_ppp_T1_dump1.in -o output/ -x 3 -y 3 -z 3 -I 100 -P 1 -T 0.01
#./runmd.sh -f $1 -i ../../../datasets/lammps-/inputfiles/7atoms_ppp_v5_dump1.in -o output/ -x 1 -y 1 -z 1 -I 1000 -P 1 -T 0.005
#./runmd.sh -f $1 -i ../../../datasets/lammps-/inputfiles/7atoms_ppp_v5_dump1.in -o output/ -x 2 -y 2 -z 2 -I 1000 -P 1 -T 0.005
#./runmd.sh -f $1 -i ../../../datasets/lammps-/inputfiles/7atoms_ppp_v5_dump1.in -o output/ -x 3 -y 3 -z 3 -I 1000 -P 1 -T 0.005
#./runmd.sh -f $1 -i ../../../datasets/lammps-/inputfiles/2000atoms_ppp_rho1.0_T3_dump100.in -o output/ -x 2 -y 2 -z 2 -I 1000 -P 100 -T 0.001
#./runmd.sh -f $1 -i ../../../datasets/lammps-/inputfiles/2000atoms_ppp_rho1.0_T3_dump100.in -o output/ -x 3 -y 3 -z 3 -I 1000 -P 100 -T 0.001
#./runmd.sh -f $1 -i ../../../datasets/lammps-/inputfiles/2000atoms_ppp_rho1.0_T3_dump100.in -o output/ -x 1 -y 2 -z 2 -I 1000 -P 100 -T 0.001
#./runmd.sh -f $1 -i ../../../datasets/lammps-/inputfiles/2048atoms_ppp_v300_dump100_realunits.in -o output/ -e 0.167 -s 2.315 -c 5.7875 -x 3 -y 3 -z 3 -I 10000 -P 100 -T 0.001
