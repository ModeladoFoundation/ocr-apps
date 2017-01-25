#!/bin/bash
#SBATCH --partition=XAS

### look at runme.sh file for what the six arguments below mean and do.
scale=$1
runhm=$2
MYITACIN=$3
psize=$4
EXE=$5
ph=$6

mpihme="/home/rsalmon/dhome/fastfw/src/apps/apps/CoMD/baseline/mpi-serial/doe-original/build_and_run/mpi"
EXE="${mpihme}/CoMD"

datahm="/home/rsalmon/dhome/fastfw/src/apps/apps/CoMD/datasets"
cd ${runhm}

nitr=100 #number of iterations
nprt=10

# fill in nodes
nnodes=$SLURM_NNODES
np="`echo "${nnodes}*$ph"|bc`"
pow2tmp=`echo "l(${nnodes})/l(2)" |bc -l`
printf -v pow2 %.0f "$pow2tmp"


case $psize in
    small)
	ii=(6 6 6 12 12 12 24)
	ij=(3 6 6 6 12 12 12)
	ik=(2 2 4 4 4 8 8)
	psx=(72 72 72 144 144 144 288)
	psy=(36 72 72 72 144 144 144)
	psz=(24 24 48 48 48 96 96)
	;;
    medium)
	ii=(6 6 6 12 12 12 24)
	ij=(3 6 6 6 12 12 12)
	ik=(2 2 4 4 4 8 8)
	psx=(144 144 144 288 288 288 576)
	psy=(72 144 144 144 288 288 288)
	psz=(48 48 96 96 96 192 192)
	;;
    large)
	ii=(6 6 6 12 12 12 24)
	ij=(3 6 6 6 12 12 12)
	ik=(2 2 4 4 4 8 8)
	psx=(288 288 288 576 576 576 1152)
	psy=(144 288 288 288 576 576 576)
	psz=(96 96 192 192 192 384 384)
esac

export I_MPI_DEBUG=3
export NODEFILE=$runhm/hfile.txt.$$
pdsh -N -w "${SLURM_NODELIST}" hostname |sort > ${NODEFILE}

#################### begin itac ######################################
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
	#export VT_PCTRACE=6
	;;
    none)
	export MYITAC=" "
esac
#################### end itac ######################################

case $scale in
    strong)
	case $psize in
	    small)
		ps=192
		;;
	    medium)
		ps=240
		;;
	    large)
		ps=288
	esac
	i=`echo "${pow2}-1"|bc`
	lname=$scale.np.$np.log

	echo "##s########################## np=$np #############################"
	echo "    problem size          Rank config"
	echo " ${ps} x ${ps} x ${ps}            ${ii[$i]} x ${ij[$i]} x ${ik[$i]}"
	echo "mpirun -f ${NODEFILE} -ppn $ph ${MYITAC} -np $np ${EXE} -d ${datahm}/pots -N $nitr -n $nprt -x ${ps} -y ${ps} -z ${ps} -e -i ${ii[$i]} -j ${ij[$i]} -k ${ik[$i]}"

	#srun --mpi=pmi2  -n
	mpirun -f ${NODEFILE} -ppn $ph ${MYITAC} -np $np ${EXE} -d ${datahm}/pots -N $nitr -n $nprt -x ${ps} -y ${ps} -z ${ps} -e -i ${ii[$i]} -j ${ij[$i]} -k ${ik[$i]}


	;;
    weak)
	#ps=(180 227 286 360 454 572 720)
	#weak scaling test. Problem size increases with np. ratio of work stays the same
	i=`echo "${pow2}-1"|bc`
	lname=$scale.np.$np.log

	echo "##w########################## np=$np #############################"
	echo "    problem size          Rank config"
	echo " ${psx[$i]} x ${psy[$i]} x ${psz[$i]}            ${ii[$i]} x ${ij[$i]} x ${ik[$i]}"
	echo "mpirun -f ${NODEFILE} -ppn $ph ${MYITAC} -np $np ${EXE} -d ${datahm}/pots -N $nitr -n $nprt -x ${psx[$i]} -y ${psy[$i]} -z ${psz[$i]} -e -i ${ii[$i]} -j ${ij[$i]} -k ${ik[$i]}"

	mpirun -f ${NODEFILE} -ppn $ph ${MYITAC} -np $np ${EXE} -d ${datahm}/pots -N $nitr -n $nprt -x ${psx[$i]} -y ${psy[$i]} -z ${psz[$i]} -e -i ${ii[$i]} -j ${ij[$i]} -k ${ik[$i]}

esac


