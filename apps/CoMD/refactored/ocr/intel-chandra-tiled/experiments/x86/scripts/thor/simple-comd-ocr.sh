#!/bin/bash
#SBATCH --partition=XAS

scale=$1
runhm=$2
MYITACIN=$3
psize=$4
EXE=$5
ph=$6


#### change the two lines below to point to your OCR and APPS root dirs. #####
export OCR_INSTALL=/home/rsalmon/dhome/fastfw/cleanone/ocr/ocr/install
export APPS_ROOT="/home/rsalmon/dhome/fastfw/cleanone/apps/apps"
datahm="/home/rsalmon/dhome/fastfw/src/apps/apps/CoMD/datasets"
##############################################################################

export LD_LIBRARY_PATH=${OCR_INSTALL}/lib:${APPS_ROOT}/libs/install/x86/lib:${LD_LIBRARY_PATH}

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
	i=`echo "${pow2}-1" | bc`
	lname=$scale.np.$np.log

	export CONFIG_NUM_THREADS=`echo "$ph+2" |bc`
	export OCR_NUM_NODES=${nnodes}
	${OCR_INSTALL}/share/ocr/scripts/Configs/config-generator.py --remove-destination --threads ${CONFIG_NUM_THREADS} --output $cfgfile --target mpi --guid COUNTED_MAP --guid LABELED

	echo "##s########################## np=$np  #############################"
	echo "$MYOCR ${EXE} -r $OCR_NUM_NODES --delta 0 -N $nitr -n $nprt -x ${ps} -y ${ps} -z ${ps} -e -i ${ii[$i]} -j ${ij[$i]} -k ${ik[$i]}  -d ${datahm}/pots"
	$MYOCR ${EXE} -r $OCR_NUM_NODES --delta 0 -N $nitr -n $nprt -x ${ps} -y ${ps} -z ${ps} -e -i ${ii[$i]} -j ${ij[$i]} -k ${ik[$i]}  -d ${datahm}/pots

	;;
    weak)
	#weak scaling test. Problem size increases with np. ratio of work stays the same
	i=`echo "${pow2}-1" | bc`
	lname=$scale.np.$np.log

	export CONFIG_NUM_THREADS=`echo "$ph+2" |bc`
	export OCR_NUM_NODES=${nnodes}
	${OCR_INSTALL}/share/ocr/scripts/Configs/config-generator.py --remove-destination --threads ${CONFIG_NUM_THREADS} --output $cfgfile --target mpi --guid COUNTED_MAP --guid LABELED

	echo "##w########################## np=$np  #############################"
	echo "$MYOCR ${EXE} -r $OCR_NUM_NODES --delta 0 -N $nitr -n $nprt -x ${psx[$i]} -y ${psy[$i]} -z ${psz[$i]} -e -i ${ii[$i]} -j ${ij[$i]} -k ${ik[$i]}  -d ${datahm}/pots"

	$MYOCR ${EXE} -r $OCR_NUM_NODES --delta 0 -N $nitr -n $nprt -x ${psx[$i]} -y ${psy[$i]} -z ${psz[$i]} -e -i ${ii[$i]} -j ${ij[$i]} -k ${ik[$i]}  -d ${datahm}/pots


esac

