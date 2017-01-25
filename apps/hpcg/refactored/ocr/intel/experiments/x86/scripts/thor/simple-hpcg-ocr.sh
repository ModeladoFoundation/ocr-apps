#!/bin/bash
#SBATCH --partition=XAS

scale=$1
runhm=$2
MYITACIN=$3
psize=$4
EXE=$5
ph=$6


#### change the two lines below to point to your OCR and APPS root dirs. #####
export OCR_INSTALL=/home/rsalmon/dhome/fastfw/src/ocr/ocr/install
export APPS_ROOT="/home/rsalmon/dhome/fastfw/src/apps/apps"
##############################################################################

export LD_LIBRARY_PATH=${OCR_INSTALL}/lib:${APPS_ROOT}/libs/install/x86/lib:${LD_LIBRARY_PATH}

export OCR_TYPE=x86-mpi
MYOCR=$OCR_INSTALL/bin/ocrrun_x86-mpi

cd ${runhm}

# fill in nodes
nnodes=$SLURM_NNODES
np="`echo "${nnodes}*$ph"|bc`"
pow2tmp=`echo "l(${nnodes})/l(2)" |bc -l`
printf -v pow2 %.0f "$pow2tmp"


cfgfile=$runhm/generated.mpi-lite.$np.$scale.cfg
export OCR_CONFIG=$cfgfile

# ## ppn=18
# ## ppn=18
case $psize in
    small)
	ii=(2 4 4 4 8 8 8)
	ij=(2 2 4 4 4 8 8)
	ik=(2 2 2 4 4 4 8)
	;;
    medium)
	ii=(2 4 4 4 8 8 8)
	ij=(2 2 4 4 4 8 8)
	ik=(2 2 2 4 4 4 8)
esac

export I_MPI_DEBUG=3
export OCR_NODEFILE=$runhm/hfile.txt.$$
pdsh -N -w "${SLURM_NODELIST}" hostname |sort > ${OCR_NODEFILE}

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
	export VT_PCTRACE=6
	export OCRRUN_OPT_ITAC="-trace"
	;;
    hpctoolkit)
	export PATH=/home/rsalmon/dhome/fastfw/src/hpctoolkit/install/bin:$PATH
	#hpcstruct ${OCR_INSTALL}/lib/libocr_x86-mpi.so
	#hpcstruct ${EXE}
	export OCRRUN_HPCTOOLKIT_OPTS="-e WALLCLOCK@5000"
	export OCRRUN_HPCTOOLKIT=yes
	#export MYHPCTK="hpcrun -t -o ./hpcg_meas -e WALLCLOCK@5000"
	;;
    none)
	#no itac.
esac
#################### end itac ######################################

case $scale in
    strong)
	#strong scaling test. Problem size stays constant.
	i=`echo "${pow2}-1"|bc`
	lname=$scale.np.$np.log
	case $psize in
	    small)
		ps=(128 102 81 64 51 40 32)
		#ps=(64 51 40 32 25 20 16)
		;;
	    medium)
		#ps=(256 204 161 128 102 81 64)
		ps=(160 127 101 80 64 50 40)
	esac

	export CONFIG_NUM_THREADS=`echo "$ph+2" |bc`
	#export CONFIG_NUM_THREADS=$ph
	export OCR_NUM_NODES=${nnodes}

	${OCR_INSTALL}/share/ocr/scripts/Configs/config-generator.py --remove-destination --threads $CONFIG_NUM_THREADS --output $cfgfile --target mpi --guid COUNTED_MAP --guid LABELED

	echo "##s########################## np=$np #############################"
	echo "    problem size          Rank config"
	echo " ${ii[$i]} x ${ij[$i]} x ${ik[$i]}     "
	echo "$MYOCR ${EXE} ${ii[$i]} ${ij[$i]} ${ik[$i]} ${ps[$i]} 50 0"

	$MYOCR ${EXE} ${ii[$i]} ${ij[$i]} ${ik[$i]} ${ps[$i]} 50 0
	;;
    weak)
	i=0
	lname=$scale.np.$np.log
	case $psize in
	    small)
		ps=128
		;;
	    medium)
		ps=160
	esac

	export CONFIG_NUM_THREADS=`echo "$ph+2" |bc`
	#export CONFIG_NUM_THREADS=$ph
	export OCR_NUM_NODES=${nnodes}

	${OCR_INSTALL}/share/ocr/scripts/Configs/config-generator.py --remove-destination --threads $CONFIG_NUM_THREADS --output $cfgfile --target mpi --guid COUNTED_MAP --guid LABELED

	echo "##w########################## np=$np #############################"
	echo "    problem size          Rank config"
	echo " ${ii[$i]} x ${ij[$i]} x ${ik[$i]}     "
	echo "$MYOCR ${EXE} ${ii[$i]} ${ij[$i]} ${ik[$i]} $ps 50 0"

	$MYOCR ${EXE} ${ii[$i]} ${ij[$i]} ${ik[$i]} $ps 50 0

esac

# case $MYITACIN in
#     hpctoolkit)
# 	hpcprof-mpi -I . -S ./libocr_x86-mpi.so.hpcstruct -S ./hpcg.hpcstruct -o ./hpcg.db ./hpcg_meas
# esac

