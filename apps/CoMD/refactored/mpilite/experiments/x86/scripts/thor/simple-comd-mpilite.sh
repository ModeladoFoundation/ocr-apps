#!/bin/bash
#SBATCH --partition=XAS

### look at runme.sh file for what the six arguments below mean and do.
scale=$1
runhm=$2
MYITACIN=$3
psize=$4
EXE=$5
ph=$6

#### change the two lines below to point to your OCR and APPS root dirs. #####
export OCR_INSTALL=/home/rsalmon/dhome/fastfw/src/ocr/ocr/install
export APPS_ROOT="/home/rsalmon/dhome/fastfw/src/apps/apps"
datahm="/home/rsalmon/dhome/fastfw/src/apps/apps/CoMD/datasets"
##############################################################################

export LD_LIBRARY_PATH=${OCR_INSTALL}/lib:${APPS_ROOT}/libs/install/x86/lib:${LD_LIBRARY_PATH}:/opt/bamboo_6_0/bin/rose/lib:/opt/bamboo_6_0/bin/boost/lib:/etc/alternatives/jre_1.7.0_openjdk/lib/amd64/server

export OCR_TYPE=x86-mpi
MYOCR=$OCR_INSTALL/bin/ocrrun_x86-mpi

cd ${runhm}

nitr=100 #number of iterations
nprt=10

# fill in nodes
nnodes=$SLURM_NNODES
np="`echo "${nnodes}*$ph"|bc`"
pow2tmp=`echo "l(${nnodes})/l(2)" |bc -l`
printf -v pow2 %.0f "$pow2tmp"

cfgfile=$runhm/generated.mpi-lite.$np.$scale.cfg
export OCR_CONFIG=$cfgfile

## ppn=18
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
export OCR_NODEFILE=$runhm/hfile.txt.$$
pdsh -N -w "${SLURM_NODELIST}" hostname |sort > ${OCR_NODEFILE}
#export OCRRUN_OPT_ENVKIND="SLURM"
#export OCRRUN_OPT_NUM_NODES="-n"

#################### begin itac ######################################
case $MYITACIN in
    p2p)
	source /opt/intel/itac_latest/bin/itacvars.sh -s
	export OCRRUN_ITAC=yes
	export OCRRUN_OPT_ITAC="-trace -trace-pt2pt"
	;;
    coll)
	source /opt/intel/itac_latest/bin/itacvars.sh -s
	export OCRRUN_ITAC=yes
	export OCRRUN_OPT_ITAC="-trace -trace-collectives"
	;;
    all)
	source /opt/intel/itac_latest/bin/itacvars.sh -s
	export OCRRUN_ITAC=yes
	export VT_CONFIG="/home/rsalmon/dhome/fastfw/src/apps/apps/CoMD/refactored/mpilite/vtconfig.conf"
	#export VT_PCTRACE=6
	export OCRRUN_OPT_ITAC="-trace"
	;;
    none)
	#no itac.
esac
#################### end itac ######################################

case $scale in
    strong)
	#strong scaling test. Problem size stays constant.
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

	export CONFIG_NUM_THREADS=`echo "$ph+2" |bc`
	export OCR_NUM_NODES=${nnodes}
	${OCR_INSTALL}/share/ocr/scripts/Configs/config-generator.py --remove-destination --threads $CONFIG_NUM_THREADS --output $cfgfile --target mpi --guid COUNTED_MAP --guid LABELED --scheduler LEGACY --dequetype LOCKED_DEQUE

	echo "##s########################## np=$np #############################"
	echo "    problem size          Rank config"
	echo " ${ps} x ${ps} x ${ps}            ${ii[$i]} x ${ij[$i]} x ${ik[$i]}"
	echo "$MYOCR ${EXE} -r $np --delta 0 -N $nitr -n $nprt -x ${ps} -y ${ps} -z ${ps} -e -i ${ii[$i]} -j ${ij[$i]} -k ${ik[$i]}  -d ${datahm}/pots "

	$MYOCR ${EXE} -r $np --delta 0 -N $nitr -n $nprt -x ${ps} -y ${ps} -z ${ps} -e -i ${ii[$i]} -j ${ij[$i]} -k ${ik[$i]}  -d ${datahm}/pots

	;;
    weak)
	#weak scaling test. Problem size increases with np. ratio of work stays the same
	#ps=(180 227 286 360 454 572 720)
	i=`echo "${pow2}-1"|bc`
	lname=$scale.np.$np.log

	export CONFIG_NUM_THREADS=`echo "$ph+2" |bc`
	export OCR_NUM_NODES=${nnodes}
	${OCR_INSTALL}/share/ocr/scripts/Configs/config-generator.py --remove-destination --threads $CONFIG_NUM_THREADS --output $cfgfile --target mpi --guid COUNTED_MAP --guid LABELED --scheduler LEGACY --dequetype LOCKED_DEQUE

	echo "##w########################## np=$np #############################"
	echo "    problem size          Rank config"
	echo " ${psx[$i]} x ${psy[$i]} x ${psz[$i]}            ${ii[$i]} x ${ij[$i]} x ${ik[$i]}"
	echo "$MYOCR ${EXE}  -r $np --delta 0 -N $nitr -n $nprt -x ${psx[$i]} -y ${psy[$i]} -z ${psz[$i]} -e -i ${ii[$i]} -j ${ij[$i]} -k ${ik[$i]} -d ${datahm}/pots "

	$MYOCR ${EXE}  -r $np --delta 0 -N $nitr -n $nprt -x ${psx[$i]} -y ${psy[$i]} -z ${psz[$i]} -e -i ${ii[$i]} -j ${ij[$i]} -k ${ik[$i]} -d ${datahm}/pots


esac


