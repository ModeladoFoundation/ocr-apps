#!/bin/bash
#SBATCH --partition=XAS

### look at runme.sh file for what the six arguments below mean and do.
scale=$1
runhm=$2
MYITACIN=$3
psize=$4
EXE=$5
ph=$6

cd ${runhm}

# fill in nodes
nnodes=$SLURM_NNODES
np="`echo "${nnodes}*$ph"|bc`"
pow2tmp=`echo "l(${nnodes})/l(2)" |bc -l`
printf -v pow2 %.0f "$pow2tmp"


case $psize in
    small)
	ii=(128 64 64 64 32 32 32)
	ij=(128 128 64 64 64 32 32)
	ik=(128 128 128 64 64 64 32)
	;;
    medium)
	ii=(160 80 80 80 40 40 40)
	ij=(160 160 80 80 80 40 40)
	ik=(160 160 160 80 80 80 40)

	# ii=(256 128 128 128 64 64 64)
	# ij=(256 256 128 128 128 64 64)
	# ik=(256 256 256 128 128 128 64)
esac

export I_MPI_DEBUG=3
export NODEFILE=$runhm/hfile.txt.$$
pdsh -N -w "${SLURM_NODELIST}" hostname |sort > ${NODEFILE}

#################### begin itac ######################################
export MYITAC=" "
export MYHPCTK=" "
case $MYITACIN in
    p2p)
	### generate itac trace files.
	source /opt/intel/itac_latest/bin/itacvars.sh -s
	export MYITAC="-trace -trace-pt2pt"
	;;
    coll)
	source /opt/intel/itac_latest/bin/itacvars.sh -s
	export MYITAC="-trace -trace-collectives"
	;;
    all)
	source /opt/intel/itac_latest/bin/itacvars.sh -s
	export VT_CONFIG="/home/rsalmon/dhome/fastfw/src/apps/apps/CoMD/refactored/mpilite/vtconfig.conf"
	export MYITAC="-trace "
	export VT_PCTRACE=6
	;;
    hpctoolkit)
	export PATH=/home/rsalmon/dhome/fastfw/src/hpctoolkit/install/bin:$PATH
	hpcstruct ${EXE}
	export MYHPCTK="hpcrun -t -o ./hpcg_meas -e WALLCLOCK@5000"
	;;
    none)
	export MYITAC=" "
	export MYHPCTK=" "
esac
#################### end itac ######################################

case $scale in
    strong)
	#strong scaling test. Problem size stays constant.
	i=`echo "${pow2}-1"|bc`
	lname=$scale.np.$np.log

	cat <<EOF > hpcg.dat
HPCG benchmark input file
Sandia National Laboratories; University of Tennessee, Knoxville
${ii[$i]} ${ij[$i]} ${ik[$i]}
50
EOF

	echo "##s########################## np=$np #############################"
	echo "    problem size          Rank config"
	echo " ${ii[$i]} x ${ij[$i]} x ${ik[$i]}     "
	echo "mpirun -f $NODEFILE -perhost 1 -genv OMP_NUM_THREADS=${ph} -genv KMP_AFFINITY=granularity=fine,compact,1,0 ${MYITAC} -np ${nnodes} ${MYHPCTK} ${EXE}"

	#The Intel AVX and Intel AVX2 optimized versions perform best with one process per socket and one OpenMP* thread per core skipping hyper-threads: set the affinity as KMP_AFFINITY=granularity=fine,compact,1,0. Specifically, for a 128-node cluster with two Intel Xeon Processor E5-2697 v3 per node, run the executable as follows:
	#I_MPI_ADJUST_ALLREDUCE=5 mpiexec.hydra –machinefile .machinefile -n 256 -perhost 2 env OMP_NUM_THREADS=14 KMP_AFFINITY=granularity=fine,compact,1,0 $EXE --n=168

	mpirun -f ${NODEFILE} -perhost 1 -genv OMP_NUM_THREADS=${ph} -genv KMP_AFFINITY=granularity=fine,compact,1,0 ${MYITAC} -np ${nnodes} ${MYHPCTK} ${EXE}
	;;

    weak)
	i=0
	lname=$scale.np.$np.log

	cat <<EOF > hpcg.dat
HPCG benchmark input file
Sandia National Laboratories; University of Tennessee, Knoxville
${ii[$i]} ${ij[$i]} ${ik[$i]}
50
EOF

	echo "##w########################## np=$np #############################"
	echo "    problem size          Rank config"
	echo " ${ii[$i]} x ${ij[$i]} x ${ik[$i]}     "
	echo "mpirun -f $NODEFILE -perhost 1 -genv OMP_NUM_THREADS=${ph} -genv KMP_AFFINITY=granularity=fine,compact,1,0 ${MYITAC} -np ${nnodes} ${MYHPCTK} ${EXE}"

	mpirun -f ${NODEFILE} -perhost 1 -genv OMP_NUM_THREADS=${ph} -genv KMP_AFFINITY=granularity=fine,compact,1,0 ${MYITAC} -np ${nnodes} ${MYHPCTK} ${EXE}

esac

case $MYITACIN in
    hpctoolkit)
	hpcprof-mpi -S xhpcg.hpcstruct -I . -o hpcg.db ./hpcg_meas
esac


hfile=`ls -v HPCG-Ben*`
sfile=`ls -v ${scale}.np.*.log`

name=`echo "${sfile}" |sed 's/.log//'`
newname="$name.${hfile}"

rm -rf ${sfile}
mv ${hfile} $newname

