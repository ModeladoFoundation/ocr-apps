#!/bin/sh
./configure.sh linux-cluster
source  ./sample_trees.sh
set -x
make	uts-seq uts-omp uts-pthread	\
	uts-upc uts-upc-dcq		\
	uts-upc-enhanced		\
	uts-mpi-wm uts-mpi-wm-nb	\
	uts-mpi-ws
#test
./uts-seq		$T4 # -x 2
exit
echo strace ./uts-seq			$T1
echo strace ./uts-pthread -T 12		$T1
echo strace OMP_NUM_THREADS=12 ./uts-omp	$T1
echo strace upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc		$T1
echo strace upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-enhanced	$T1
echo strace upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-dcq	$T1
echo strace mpirun -n 12 ./uts-mpi-wm	$T1
echo strace mpirun -n 12 ./uts-mpi-wm-nb	$T1
echo strace mpirun -n 12 ./uts-mpi-ws	$T1
exit
#./uts-seq			$T2
#./uts-pthread -T 12		$T2
#OMP_NUM_THREADS=12 ./uts-omp	$T2
#upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc		$T2
#upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-enhanced	$T2
#upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-dcq	$T2
#mpirun -n 12 ./uts-mpi-wm	$T2
#mpirun -n 12 ./uts-mpi-wm-nb	$T2
#mpirun -n 12 ./uts-mpi-ws	$T2

#./uts-seq			$T3
#./uts-pthread -T 12		$T3
#OMP_NUM_THREADS=12 ./uts-omp	$T3
#upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc		$T3
#upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-enhanced	$T3
#upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-dcq	$T3
#mpirun -n 12 ./uts-mpi-wm	$T3
#mpirun -n 12 ./uts-mpi-wm-nb	$T3
#mpirun -n 12 ./uts-mpi-ws	$T3

#./uts-seq			$T4
#./uts-pthread -T 12		$T4
#OMP_NUM_THREADS=12 ./uts-omp	$T4
#upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc		$T4
#upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-enhanced	$T4
#upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-dcq	$T4
#mpirun -n 12 ./uts-mpi-wm	$T4
#mpirun -n 12 ./uts-mpi-wm-nb	$T4
#mpirun -n 12 ./uts-mpi-ws	$T4

#./uts-seq			$T5
#./uts-pthread -T 12		$T5
#OMP_NUM_THREADS=12 ./uts-omp	$T5
#upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc		$T5
#upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-enhanced	$T5
#upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-dcq	$T5
#mpirun -n 12 ./uts-mpi-wm	$T5
#mpirun -n 12 ./uts-mpi-wm-nb	$T5
#mpirun -n 12 ./uts-mpi-ws	$T5

#./uts-seq			$T1L
#./uts-pthread -T 12		$T1L
#OMP_NUM_THREADS=12 ./uts-omp	$T1L
#upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc		$T1L
#upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-enhanced	$T1L
#upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-dcq	$T1L
#mpirun -n 12 ./uts-mpi-wm	$T1L
#mpirun -n 12 ./uts-mpi-wm-nb	$T1L
#mpirun -n 12 ./uts-mpi-ws	$T1L

#./uts-seq			$T2L
#./uts-pthread -T 12		$T2L
#OMP_NUM_THREADS=12 ./uts-omp	$T2L
#upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc		$T2L
#upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-enhanced	$T2L
#upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-dcq	$T2L
#mpirun -n 12 ./uts-mpi-wm	$T2L
#mpirun -n 12 ./uts-mpi-wm-nb	$T2L
#mpirun -n 12 ./uts-mpi-ws	$T2L

#./uts-seq			$T3L
#./uts-pthread -T 12		$T3L
#OMP_NUM_THREADS=12 ./uts-omp	$T3L
#upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc		$T3L
#upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-enhanced	$T3L
#upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-dcq	$T3L
#mpirun -n 12 ./uts-mpi-wm	$T3L
#mpirun -n 12 ./uts-mpi-wm-nb	$T3L
#mpirun -n 12 ./uts-mpi-ws	$T3L

#./uts-seq			$T1XL
./uts-pthread -T 12		$T1XL
OMP_NUM_THREADS=12 ./uts-omp	$T1XL
upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc		$T1XL
upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-enhanced	$T1XL
upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-dcq	$T1XL
mpirun -n 12 ./uts-mpi-wm	$T1XL
mpirun -n 12 ./uts-mpi-wm-nb	$T1XL
mpirun -n 12 ./uts-mpi-ws	$T1XL

#./uts-seq			$T1XXL
./uts-pthread -T 12		$T1XXL
OMP_NUM_THREADS=12 ./uts-omp	$T1XXL
upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc		$T1XXL
upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-enhanced	$T1XXL
upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-dcq	$T1XXL
mpirun -n 12 ./uts-mpi-wm	$T1XXL
mpirun -n 12 ./uts-mpi-wm-nb	$T1XXL
mpirun -n 12 ./uts-mpi-ws	$T1XXL

#./uts-seq			$T3XXL
./uts-pthread -T 12		$T3XXL
OMP_NUM_THREADS=12 ./uts-omp	$T3XXL
upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc		$T3XXL
upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-enhanced	$T3XXL
upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-dcq	$T3XXL
mpirun -n 12 ./uts-mpi-wm	$T3XXL
mpirun -n 12 ./uts-mpi-wm-nb	$T3XXL
mpirun -n 12 ./uts-mpi-ws	$T3XXL

# RAG -- ss_push(stack overflow) on one node
##./uts-seq			$T2XXL
##./uts-pthread -T 12		$T2XXL
##OMP_NUM_THREADS=12 ./uts-omp	$T2XXL
##upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc		$T2XXL
##upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-enhanced	$T2XXL
##upcrun -n 12 -nodes=1 -cpus-per-node=12 -q ./uts-upc-dcq	$T2XXL
##mpirun -n 12 ./uts-mpi-wm	$T2XXL
##mpirun -n 12 ./uts-mpi-wm-nb	$T2XXL
##mpirun -n 12 ./uts-mpi-ws	$T2XXL

