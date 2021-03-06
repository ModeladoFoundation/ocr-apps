#!/bin/bash

if [[ -z "${APPS_ROOT}" ]]; then
    echo "\$APPS_ROOT is not set."
    exit 1
fi
OCR_TOP="${APPS_ROOT}/../../ocr"

nExp=1

source $APPS_ROOT/tools/execution_tools/aux_bash_functions
source ./experiments/x86/parameters.job

HOST_ID=`hostname -s`

JOBHEADER=${JOBHEADER-JOBPrb}

PHYSICAL_CORES_PER_NODE=16  #Available cores per node

case $HOST_ID in
edison*)
    PHYSICAL_CORES_PER_NODE=24
    HT_CORES_PER_NODE=48
    ;;

cori*)
    PHYSICAL_CORES_PER_NODE=32
    HT_CORES_PER_NODE=64
    ;;

thor*) # Not valid for all thor nodes
    PHYSICAL_CORES_PER_NODE=36
    HT_CORES_PER_NODE=72
    ;;

bar*) # Not valid for all thor nodes
    PHYSICAL_CORES_PER_NODE=16
    HT_CORES_PER_NODE=32
    ;;

eln6) # Valid for only XeonPhi
    PHYSICAL_CORES_PER_NODE=48
    HT_CORES_PER_NODE=192
    export CC=icc
    ;;
esac

CONFIG_GENERATE=${OCR_TOP}/ocr/install/share/ocr/scripts/Configs/config-generator.py

spath=(`pwd | tr "/" " "`) # ( . . . .  app baseOrRefactored runtime implVersion)
len=${#spath[@]}
app_name=${spath[$len-4]}

function getSizeList()
{
    local app_name=$1

    sizeList=(${SIZE_LIST[@]})

    echo ${sizeList[@]}
}

function getConfigFlags()
{
    #Changed for static scheduler
    local app_name=$1

    CONFIG_FLAGS="--guid LABELED --binding seq"  #Default work-stealing scheduler

    echo $CONFIG_FLAGS
}

function splitDimensions2D()
{
    local ranks=$1

    nz=1

    ranks=$(($ranks/$nz))

    ny=`myroot $ranks 2`
    nx=1

    for(( cy=$ny; cy>0; cy--)); do
    if [[ $(($ranks%$cy)) == 0 ]]; then
        nx=$(($ranks/$cy))
        break;
    fi
    done
    ny=$cy

    nx=$(($ranks/$ny))

    echo $nx $ny $nz
}

function splitDimensions3D()
{
    local ranks=$1

    nz=`myroot $ranks 3`
    ny=1
    nx=1

    for(( cz=$nz; cz>0; cz--)); do
    if [[ $(($ranks%$cz)) == 0 ]]; then
        ny=$(($ranks/$cz))
        break;
    fi
    done
    nz=$cz

    ranks=$(($ranks/$nz))

    ny=`myroot $ranks 2`
    nx=1

    for(( cy=$ny; cy>0; cy--)); do
    if [[ $(($ranks%$cy)) == 0 ]]; then
        nx=$(($ranks/$cy))
        break;
    fi
    done
    ny=$cy

    nx=$(($ranks/$ny))

    echo $nx $ny $nz
}

function splitDimensions()
{
    local ranks=$1
    local app_name=$2

    rxyz=(`splitDimensions3D $ranks`)

    case ${app_name} in

    "Stencil2D")
        rxyz=(`splitDimensions2D $ranks`)
        ;;
    esac

    echo ${rxyz[@]}
}

function getworkloadargs()
{
    local scalingtype=$1
    local nodes=$2
    local threads=$3
    local taskfactor=$4
    local size=$5
    local app_name=$6
    local runtime=$7

    local ranks=$(($nodes))

    S=(`getSize $app_name $size $scalingtype`)
    problem_size_x=${S[0]}
    problem_size_y=${S[1]}
    problem_size_z=${S[2]}

    iter=`getIterations $app_name $size $scalingtype`

    tx=1; ty=1; tz=1 #Baseline thread count
    txyz=(`splitDimensions $threads $app_name`)
    tx=${txyz[0]}
    ty=${txyz[1]}
    tz=${txyz[2]}

    rankxyz=(1 1 1)
    rankxyz=(`splitDimensions $ranks $app_name`)
    rx=${rankxyz[0]}
    ry=${rankxyz[1]}
    rz=${rankxyz[2]}

    if [[ "$scalingtype" == "weakscaling" ]]; then
        Nx=$(($problem_size_x*${rankxyz[0]}*${tx}))
        Ny=$(($problem_size_y*${rankxyz[1]}*${ty}))
        Nz=$(($problem_size_z*${rankxyz[2]}*${tz}))
    elif [[ "$scalingtype" == "strongscaling" ]]; then
        Nx=$problem_size_x
        Ny=$problem_size_y
        Nz=$problem_size_z
    fi

    px=$(($tx*${rankxyz[0]}))
    py=$(($ty*${rankxyz[1]}))
    pz=$(($tz*${rankxyz[2]}))

    case ${app_name} in

    "Stencil2D")
        if [[ $runtime == ocr ]]; then
        WORKLOAD_ARGS="$Nx $((${threads}*${nodes}*${taskfactor}*${taskfactor})) ${iter}"
        elif [[ $runtime == mpi ]]; then
        WORKLOAD_ARGS="${iter} $Nx ${threads}"
        fi
        ;;

    "CoMD")
        WORKLOAD_ARGS="-x $Nx -y $Ny -z $Nz -i $(($px*$taskfactor)) -j $(($py*$taskfactor)) -k $(($pz*$taskfactor)) -N ${iter} -n 10"
        if [[ $size == *EAM ]]; then
            WORKLOAD_ARGS="$WORKLOAD_ARGS -e"
            if [[ $runtime == mpi ]]; then
                WORKLOAD_ARGS="$WORKLOAD_ARGS -d ../../../datasets/pots"
            fi
        fi
        ;;

    "XSBench")
        WORKLOAD_ARGS="-s $size -t $threads -l ${iter}"
        ;;

    "RSBench")
        WORKLOAD_ARGS="-s $size -t $threads -l ${iter} -d"
        ;;

    "miniAMR")
        init_x=$((${problem_size_x}/${rx}/${tx}))
        init_y=$((${problem_size_y}/${ry}/${ty}))
        init_z=$((${problem_size_z}/${rz}/${tz}))
        num_refine=(`getRefinements $app_name $size $scalingtype`)
        lb=(`getlbOpt $app_name $size $scalingtype`)
        gS=(`getGrainSizeMiniAMR $size $scalingtype`)

        BASE_ARGS="--num_refine ${num_refine} --max_blocks $(((8**(${num_refine}+1))*$init_x*$init_y*$init_z)) --nx ${gS[0]} --ny ${gS[1]} --nz ${gS[2]} --init_x $init_x --init_y $init_y --init_z $init_z --npx $((${rx}*${tx})) --npy $((${ry}*${ty})) --npz $((${rz}*${tz})) --report_diffusion 1 --lb_opt ${lb}"
        #WORKLOAD_ARGS="--num_objects 1 --object 2 0 -1.71 -1.71 -1.71 0.04 0.04 0.04 1.7 1.7 1.7 0.0 0.0 0.0${BASE_ARGS}" #One sphere moving diagonally (MPI code fails for multi-rank ranks)
        #WORKLOAD_ARGS="--num_objects 1 --object 2 0 -0.01 -0.01 -0.01 0.0 0.0 0.0 0.0 0.0 0.0 0.0009 0.0009 0.0009 ${BASE_ARGS}"  # An expanding sphere (This is stable for MPI code)
        WORKLOAD_ARGS="--num_objects 2 --object 2 0 -1.10 -1.10 -1.10 0.030 0.030 0.030 1.5 1.5 1.5 0.0 0.0 0.0 --object 2 0 0.5 0.5 1.76 0.0 0.0 -0.025 0.75 0.75 0.75 0.0 0.0 0.0 ${BASE_ARGS}" #Two moving spheres (MPI runs are stable)
        ;;

    esac

    echo $WORKLOAD_ARGS
}

function getProfilerArgs()
{
    local profiler=$1
    case $profiler in

    noProf|baseline)
        MAKE_PROFILER_ARGS=""
        ;;

    minimalProf)
        MAKE_PROFILER_ARGS='ENABLE_PROFILER=yes PROFILER_FOCUS=userCode PROFILER_IGNORE_RT=yes PROFILER_COUNT_OTHER=yes'
        ;;

    detailedProf)
        MAKE_PROFILER_ARGS='ENABLE_PROFILER=yes PROFILER_FOCUS=wo_hc_workShift PROFILER_IGNORE_RT=yes PROFILER_COUNT_OTHER=yes PROFILER_CONSIDER_USER="wo_hc_getWork wo_hc_executeWork wo_hc_wrapupWork ta_hc_execute ta_hc_executeSetup ta_hc_executeCleanup userCode" PROFILER_PEEK=userCode'
        ;;

    esac

    echo $MAKE_PROFILER_ARGS
}

function getOcrRootDir()
{
    local profiler=$1
    local arch0=$2
    case $profiler in

    noProf|baseline)
        OCR_ROOT_DIR="${APPS_ROOT}/../../ocr"
        #OCR_ROOT_DIR="${APPS_ROOT}/../../ocr_gcc" #TODO - CoMD
        if [[ $arch0 == "x86-mpiprobe" ]]; then
            OCR_ROOT_DIR="${APPS_ROOT}/../../ocr_mpiprobe"
        fi
        ;;

    minimalProf)
        OCR_ROOT_DIR="${APPS_ROOT}/../../ocr_minimalProf"
        ;;

    detailedProf)
        OCR_ROOT_DIR="${APPS_ROOT}/../../ocr_detailedProf"
        ;;

    lazyDB)
        OCR_ROOT_DIR="${APPS_ROOT}/../../ocr_lazyDB"
        ;;

    esac

    echo $OCR_ROOT_DIR/ocr/install
}

function getSizeStencil()
{
    local size=$1
    local scalingtype=$2

    case ${scalingtype} in
    "weakscaling")
        case ${size} in
        "small")
            S1=768
            ;;
        "medium")
            S1=1536
            ;;
        "large")
            S1=3072
            ;;
        esac
        ;;

    "strongscaling")
        case ${size} in
        "small")
            S1=23170
            ;;
        "medium")
            S1=32768
            ;;
        "large")
            S1=46340
            ;;
        esac
        ;;

    esac

    echo $S1 $S1 $1
}

function getSizeComd()
{
    local size=$1
    local scalingtype=$2

    case ${scalingtype} in
    "weakscaling")
        case ${size} in
        small*)
            S1=12
            ;;
        medium*)
            S1=24
            ;;
        large*)
            S1=48
            ;;
        esac
        ;;

    "strongscaling")
        case ${size} in
        small*)
            S1=192
            ;;
        medium*)
            S1=240
            ;;
        large*)
            S1=288
            ;;
        esac
        ;;

    esac

    echo $S1 $S1 $S1
}

function getSizeMiniAMR()
{
    local size=$1
    local scalingtype=$2

    case ${scalingtype} in
    "weakscaling") #Weak-scaling is not suported
        case ${size} in
        "small")
            S1=8
            ;;
        "medium")
            S1=16
            ;;
        "large")
            S1=32
            ;;
        esac
        ;;

    "strongscaling")
        case ${size} in
        small*)
            S1=8
            ;;
        medium*)
            S1=16
            ;;
        large*)
            S1=32
            ;;
        esac
        ;;

    esac

    echo $S1 $S1 $S1
}

function getGrainSizeMiniAMR()
{
    local size=$1
    local scalingtype=$2

    case ${scalingtype} in
    "weakscaling") #Weak-scaling is not suported
        case ${size} in
        "small")
            S1=8
            ;;
        "medium")
            S1=16
            ;;
        "large")
            S1=32
            ;;
        esac
        ;;

    "strongscaling")
        case ${size} in
        *-gs1-*)
            S1=8
            ;;
        *-gs2*)
            S1=16
            ;;
        *-gs4*)
            S1=32
            ;;
        *-gs5*)
            S1=40
            ;;
        *-gs6*)
            S1=48
        esac
        ;;

    esac

    echo $S1 $S1 $S1
}

function getIterationsStencil()
{
    local size=$1
    local scalingtype=$2

    case ${scalingtype} in
    "weakscaling")
        case ${size} in
        "small")
            S1=1000
            ;;
        "medium")
            S1=100
            ;;
        "large")
            S1=100
            ;;
        esac
        ;;

    "strongscaling")
        case ${size} in
        "small")
            S1=100
            ;;
        "medium")
            S1=100
            ;;
        "large")
            S1=100
            ;;
        esac
        ;;

    esac

    echo $S1
}

function getIterationsComd()
{
    local size=$1
    local scalingtype=$2

    case ${scalingtype} in
    "weakscaling")
        case ${size} in
        small*)
            S1=200
            ;;
        medium*)
            S1=100
            ;;
        large*)
            S1=100
            ;;
        esac
        ;;

    "strongscaling")
        case ${size} in
        small*)
            S1=100
            ;;
        medium*)
            S1=100
            ;;
        large*)
            S1=100
            ;;
        esac
        ;;

    esac

    echo $S1
}

function getIterationsBenches()
{
    local size=$1
    local scalingtype=$2

    case ${scalingtype} in
    "weakscaling")
        case ${size} in
        "small")
            S1=1000000
            ;;
        "medium")
            S1=1000000
            ;;
        "large")
            S1=1000000
            ;;
        esac
        ;;

    "strongscaling")
        case ${size} in
        "small")
            S1=1000000
            ;;
        "medium")
            S1=1000000
            ;;
        "large")
            S1=1000000
            ;;
        esac
        ;;

    esac

    echo $S1
}

function getIterations()
{
    local app_name=$1
    local size=$2
    local scalingtype=$3

    case ${app_name} in

    "Stencil2D")
        S=(`getIterationsStencil $size $scalingtype`)
        ;;

    "CoMD")
        S=(`getIterationsComd $size $scalingtype`)
        ;;

    *SBench)   #there's no medium size; S1 is dummy for this app
        S=(`getIterationsBenches $size $scalingtype`)
        ;;

    esac

    echo ${S[@]}
}

function getRefinements()
{
    local app_name=$1
    local size=$2
    local scalingtype=$3

    case ${size} in

    *-r0-*)
        R=0
        ;;

    *-r1-*)
        R=1
        ;;

    *-r2-*)
        R=2
        ;;

    *-r3-*)
        R=3
        ;;

    esac

    echo $R
}

function getlbOpt()
{
    local app_name=$1
    local size=$2
    local scalingtype=$3

    case ${size} in

    *-lb0)
        R=0
        ;;

    *-lb1)
        R=1
        ;;

    *-lb31)
        R=31
        ;;

    *-lb33)
        R=33
        ;;

    esac

    echo $R
}

function getSize()
{
    local app_name=$1
    local size=$2
    local scalingtype=$3

    case ${app_name} in

    "Stencil2D")
        S=(`getSizeStencil $size $scalingtype`)
        ;;

    "CoMD")
        S=(`getSizeComd $size $scalingtype`)
        ;;

    "miniAMR")
        S=(`getSizeMiniAMR $size $scalingtype`)
        ;;

    *SBench)   #there's no medium size; S1 is dummy for this app
        case ${size} in
        "small")
            S=(1 1 1)
            ;;
        "large")
            S=(1 1 1)
            ;;
        esac
        ;;

    esac

    echo ${S[@]}
}

function generateJobScript()
{
    local app_name=$1
    local nodes=$2
    local type=$3

    local queue="regular"
    local hours="24"
    local minutes="00"

    local tplName=""
    local queue=""

    case $HOST_ID in

    edison*)
        tplName="EdisonJob.template"
        queue="regular"
        ;;

    thor*)
        tplName="ThorJob.template"
        queue="XAS64"
        ;;

    bar*)
        tplName="BarJob.template"
        queue="XAS"
        ;;

    eln6)
        tplName="Eln6Job.template"
        queue="XAS"
        ;;

    esac

    local TPL_NAME="${APPS_ROOT}/tools/execution_tools/${tplName}"
    JOBNAME=${JOBHEADER}_${type}_${app_name}_${nodes}
    OUTNAME=${JOBNAME}.sh

    if [ ! -e ${OUTNAME} ]; then
        rm -f ${OUTNAME} 2>/dev/null
        more ${TPL_NAME} \
            | sed -e "s|ARG_QUEUE|${queue}|g" \
            | sed -e "s|ARG_NODES|${nodes}|g" \
            | sed -e "s|ARG_HOUR|${hours}|g" \
            | sed -e "s|ARG_MIN|${minutes}|g" \
            | sed -e "s|ARG_NAME|${JOBNAME}.out|g" \
            > ${OUTNAME}
        chmod u+x ${OUTNAME}
    fi

    echo $OUTNAME
}

if [[ ${spath[$len-2]} == "ocr" ]]; then

for profiler in ${PROFILER_LIST[@]}; do

    MAKE_PROFILER_ARGS=`getProfilerArgs $profiler`
    for scalingtype in ${SCALINGTYPE_LIST[@]}; do

        SIZE_LIST=(`getSizeList ${app_name}`)

        for size in ${SIZE_LIST[@]}; do

            for taskfactor in ${TASKFACTOR_LIST[@]}; do

                for arch0 in ${ARCH_LIST[@]}; do

                    OCR_INSTALL=`getOcrRootDir $profiler $arch0`
                    arch="${arch0}"
                    INSTPATHROOT="install"

                    if [[ $arch0 == "x86" ]]; then
                        CONFIG_COMM_LAYER=x86
                        CONFIG_FLAGS_BASE=""
                    elif [[ $arch0 == "x86-phi" ]]; then
                        CONFIG_COMM_LAYER=x86
                        CONFIG_FLAGS_BASE="--guid COUNTED_MAP"
                    elif [[ $arch0 == "x86-mpi" ]]; then
                        CONFIG_COMM_LAYER="mpi"
                        CONFIG_FLAGS_BASE="--guid COUNTED_MAP"
                    elif [[ $arch0 == "x86-mpiprobe" ]]; then
                        arch="x86-mpi"
                        CONFIG_COMM_LAYER="mpi_probe"
                        CONFIG_FLAGS_BASE="--guid COUNTED_MAP"
                        INSTPATHROOT="installmpiprobe"
                    fi

                    for appopts in ${APPOPTS_LIST[@]}; do

                        CFLAGS0=""
                        if [[ $appopts == "wEagerDB" ]]; then
                            CFLAGS0="-DUSE_EAGER_DB_HINT"
                        elif [[ $appopts == "wLazyDB" ]]; then
                            CFLAGS0="-DUSE_LAZY_DB_HINT"
                        fi

                        for schedulername in ${SCHEDULER_LIST[@]}; do

                            CFLAGS="$CFLAGS0"
                            CONFIG_FLAGS_EXTRA=""
                            STATIC_SCHEDULER_MAKE_OPTS=""

                            if [[ ${app_name} != *SBench ]]; then
                                if [[ $schedulername == "workstealing" ]]; then
                                    STATIC_SCHEDULER=0
                                elif [[ $schedulername == "static" ]]; then
                                    STATIC_SCHEDULER=1
                                    CONFIG_FLAGS_EXTRA="--scheduler STATIC"
                                fi
                                STATIC_SCHEDULER_MAKE_OPTS="STATIC_SCHEDULER=${STATIC_SCHEDULER}"
                            else
                                if [[ $schedulername == "static" ]]; then
                                    CFLAGS="$CFLAGS0 -DSCHEDULER_TYPE=0"
                                elif [[ $schedulername == "workstealing" ]]; then
                                    CFLAGS="$CFLAGS0 -DSCHEDULER_TYPE=1 -DCHUNK_SIZE=$(($taskfactor*${benchBaseTaskSize}))"
                                fi
                            fi

                            #rm -rf build install
                            #MAKE_COMMAND=`echo ${STATIC_SCHEDULER_MAKE_OPTS} CFLAGS=\"${CFLAGS}\" OCR_INSTALL=${OCR_INSTALL} V=1 make -f Makefile.$arch clean install`
                            #MAKE_COMMAND=`echo ${STATIC_SCHEDULER_MAKE_OPTS} CFLAGS=\"${CFLAGS}\" OCR_ROOT=${OCR_INSTALL}/.. ${MAKE_PROFILER_ARGS} V=1 make -f Makefile.$arch clean install`
                            MAKE_COMMAND=`echo ${STATIC_SCHEDULER_MAKE_OPTS} CFLAGS=\"${CFLAGS}\" OCR_ROOT=${OCR_INSTALL}/.. V=1 make -f Makefile.$arch clean install`
                            eval $MAKE_COMMAND | tee make.log
                            CONFIG_FLAGS=`getConfigFlags $app_name`
                            CONFIG_FLAGS="${CONFIG_FLAGS} ${CONFIG_FLAGS_EXTRA}"

                            mv make.log install/$arch

                            if [[ $arch == "x86" ]]; then
                                NODE_LIST=(1)
                            elif [[ $arch == "x86-phi" ]]; then
                                NODE_LIST=(1)
                            elif [[ $arch == "x86-mpi" ]]; then
                                NODE_LIST=(${NODE_LIST0[@]})
                            fi

                            for nodes in ${NODE_LIST[@]}; do

                                OUTNAME=`generateJobScript ${app_name} ${nodes} ocr`

                                for computeThreads in ${THREAD_LIST[@]}; do

                                    runtimeThreads=$computeThreads
                                    ranks=$(($nodes*$computeThreads))

                                    if [[ $arch == "x86-mpi" && $computeThreads != $PHYSICAL_CORES_PER_NODE ]]; then
                                        runtimeThreads=$(($computeThreads+1))
                                    fi

                                    install_root=${INSTPATHROOT}_${profiler}_${scalingtype}_${size}_sub${taskfactor}_${appopts}_${schedulername}_${nodes}x${computeThreads}
                                    ndir=${install_root}/${arch}
                                    #rm -rf ${ndir}
                                    mkdir -p ${ndir}
                                    cp -r install/${arch} ${install_root}
                                    echo ${CONFIG_GENERATE} --remove-destination --threads ${runtimeThreads} --output ${ndir}/generated.cfg --target ${CONFIG_COMM_LAYER} ${CONFIG_FLAGS_BASE} ${CONFIG_FLAGS}
                                    eval ${CONFIG_GENERATE} --remove-destination --threads ${runtimeThreads} --output ${ndir}/generated.cfg --target ${CONFIG_COMM_LAYER} ${CONFIG_FLAGS_BASE} ${CONFIG_FLAGS}

                                    WORKLOAD_ARGS=`getworkloadargs $scalingtype $nodes $computeThreads $taskfactor $size $app_name ocr`

                                    RUN_COMMAND=`echo RUN_MODE=runApp OCR_TYPE=$arch OCR_INSTALL=${OCR_INSTALL} WORKLOAD_INSTALL_ROOT=./${install_root} OCR_CONFIG=$PWD/${ndir}/generated.cfg OCR_NUM_NODES=${nodes} CONFIG_NUM_THREADS=${runtimeThreads} WORKLOAD_ARGS=\"${WORKLOAD_ARGS}\" make run`

                                    echo $RUN_COMMAND | tee run_command
                                    mv run_command $ndir/

                                    echo "echo ${profiler} ${scalingtype} ${size} ${taskfactor} ${appopts} ${schedulername} ${nodes} ${computeThreads} ${arch0}"  >> ${OUTNAME}
                                    #echo "echo $RUN_COMMAND" >> ${OUTNAME}
                                    echo $RUN_COMMAND >> ${OUTNAME}
                                    if [[ ${profiler} == "noProf" ]]; then
                                        for(( iexp=0; iexp < $nExp; iexp++ )); do
                                            echo $RUN_COMMAND >> ${OUTNAME}
                                        done
                                    fi
                                    echo "" >> ${OUTNAME}

                                done #computeThreads
                            done #nodes
                        done #schedulername
                    done #appopts
                done #arch
            done #taskfactor
        done #size
    done #scalingtype
done #profiler

fi

function getExecutableName()
{
    local app_name=$1
    case ${app_name} in

    "Stencil2D")
        executable="./stencil"
        ;;

    "CoMD")
        executable="bin/CoMD-mpi"
        ;;

    "miniAMR")
        executable="./miniAMR.x"
        ;;

    "XSBench")
        executable="src/XSBench"
        ;;

    "RSBench")
        executable="src/rsbench"
        ;;

    esac

    echo $executable
}

function getMakeCommand()
{
    local app_name=$1
    case ${app_name} in

    "Stencil2D")
        make stencil
        ;;

    "CoMD")
        CC=mpiicc OPTFLAGS="-g -O3" make -C src-mpi
        ;;

    "XSBench")
        make -C src
        ;;

    "RSBench")
        make -C src
        ;;

    esac
}

echo ${spath[$len-3]}
if [[ ${spath[$len-3]} == baseline* ]]; then

for scalingtype in ${SCALINGTYPE_LIST[@]}; do
    SIZE_LIST=(`getSizeList ${app_name}`)
    for size in ${SIZE_LIST[@]}; do
        TASKFACTOR_LIST=(1)
        for taskfactor in ${TASKFACTOR_LIST[@]}; do
        for nodes in ${NODE_LIST0[@]}; do

            OUTNAME=`generateJobScript ${app_name} ${nodes} MPI`

            for computeThreads in ${THREAD_LIST[@]}; do

                ranks=$(($nodes*$computeThreads))

                getMakeCommand ${app_name}

                ndir=${scalingtype}_${size}_${nodes}x${computeThreads}
                executable=`getExecutableName ${app_name}`

                split=(`echo $executable | tr "/" " "`)
                len1=${#split[@]}
                executableName=${split[$len1-1]}

                mkdir -p ${ndir}
                cp $executable ${ndir}/

                WORKLOAD_ARGS=`getworkloadargs $scalingtype $nodes $computeThreads $taskfactor $size $app_name mpi`

                if [[ "${SRUN_AFFINITY}" = "yes" ]]; then
                    REM=`echo "${HT_CORES_PER_NODE}%${computeThreads}" | bc`
                    if [[ ${REM} != 0 ]]; then
                    echo "WARNING: srun affinity binding failed"
                    SRUN_OPTS=""
                    else
                    SRUN_OPTS="-c $((${HT_CORES_PER_NODE}/${computeThreads})) "
                    fi
                fi

                echo $scalingtype $size $nodes $computeThreads
                RUN_COMMAND=`echo srun --mpi=pmi2 -n $(($nodes*$computeThreads)) ${SRUN_OPTS} ./$ndir/$executableName ${WORKLOAD_ARGS}`

                if [[ $app_name == *SBench ]]; then
                RUN_COMMAND=`echo "export OMP_NUM_THREADS=$computeThreads; ./$ndir/$executableName ${WORKLOAD_ARGS}"`
                fi

                echo $RUN_COMMAND | tee run_command
                mv run_command $ndir/

                echo "echo $scalingtype $size $nodes $computeThreads" >> ${OUTNAME}
                for(( iexp=0; iexp < $nExp; iexp++ )); do
                    echo $RUN_COMMAND >> ${OUTNAME}
                done
                echo "" >> ${OUTNAME}

            done #computeThreads
        done #nodes
        done #taskfactor
    done
done

fi
