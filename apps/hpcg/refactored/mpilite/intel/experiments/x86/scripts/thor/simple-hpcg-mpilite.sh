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
##############################################################################

export LD_LIBRARY_PATH=${OCR_INSTALL}/lib:${APPS_ROOT}/libs/install/x86/lib:${LD_LIBRARY_PATH}:/opt/bamboo_6_0/bin/rose/lib:/opt/bamboo_6_0/bin/boost/lib:/etc/alternatives/jre_1.7.0_openjdk/lib/amd64/server

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

	#export CONFIG_NUM_THREADS=`echo "$ph+2" |bc`
	export CONFIG_NUM_THREADS=$ph
	export OCR_NUM_NODES=${nnodes}

	${OCR_INSTALL}/share/ocr/scripts/Configs/config-generator.py --remove-destination --threads $CONFIG_NUM_THREADS --output $cfgfile --target mpi --guid COUNTED_MAP --guid LABELED --scheduler LEGACY --dequetype LOCKED_DEQUE

	cat <<EOF > hpcg.dat
HPCG benchmark input file
Sandia National Laboratories; University of Tennessee, Knoxville
${ii[$i]} ${ij[$i]} ${ik[$i]}
50
EOF

	echo "##s########################## np=$np #############################"
	echo "    problem size          Rank config"
	echo " ${ii[$i]} x ${ij[$i]} x ${ik[$i]}     "
	echo "$MYOCR ${EXE} -r $OCR_NUM_NODES -t $CONFIG_NUM_THREADS"

	$MYOCR ${EXE} -r $OCR_NUM_NODES -t $CONFIG_NUM_THREADS
	;;
    weak)
	i=0
	lname=$scale.np.$np.log

	#export CONFIG_NUM_THREADS=`echo "$ph+2" |bc`
	export CONFIG_NUM_THREADS=$ph
	export OCR_NUM_NODES=${nnodes}

	${OCR_INSTALL}/share/ocr/scripts/Configs/config-generator.py --remove-destination --threads $CONFIG_NUM_THREADS --output $cfgfile --target mpi --guid COUNTED_MAP --guid LABELED --scheduler LEGACY --dequetype LOCKED_DEQUE

	cat <<EOF > hpcg.dat
HPCG benchmark input file
Sandia National Laboratories; University of Tennessee, Knoxville
${ii[$i]} ${ij[$i]} ${ik[$i]}
50
EOF

	echo "##w########################## np=$np #############################"
	echo "    problem size          Rank config"
	echo " ${ii[$i]} x ${ij[$i]} x ${ik[$i]}     "
	echo "$MYOCR ${EXE} -r $OCR_NUM_NODES -t $CONFIG_NUM_THREADS"

	$MYOCR ${EXE} -r $OCR_NUM_NODES -t $CONFIG_NUM_THREADS

esac

# case $MYITACIN in
#     hpctoolkit)
# 	hpcprof-mpi -I . -S ./libocr_x86-mpi.so.hpcstruct -S ./hpcg.hpcstruct -o ./hpcg.db ./hpcg_meas
# esac


hfile=`ls -v HPCG-Ben*`
sfile=`ls -v ${scale}.np.*.log`

name=`echo "${sfile}" |sed 's/.log//'`
newname="$name.${hfile}"

rm -rf ${sfile}
mv ${hfile} $newname

