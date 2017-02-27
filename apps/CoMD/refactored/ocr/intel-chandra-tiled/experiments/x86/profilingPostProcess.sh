#!/bin/bash

source experiments/x86/parameters.job

root="${JOBHEADER}_ocr_CoMD"

AROOT=$PWD
spath=(`pwd | tr "/" " "`) # ( . . . .  app baseOrRefactored runtime implVersion)
len=${#spath[@]}
app_name=${spath[$len-4]}

function getSizeList()
{
    local app_name=$1
    case ${app_name} in

    "Stencil2D")
        sizeList=(${SIZE_LIST_STENCIL2D[@]})
        ;;

    "CoMD")
        sizeList=(${SIZE_LIST_COMD[@]})
        ;;

    *SBench)
        sizeList=(${SIZE_LIST_BENCH[@]})
        ;;

    esac

    echo ${sizeList[@]}
}

rm ${root}_results.prof

if [[ ${spath[$len-2]} == "ocr" ]]; then
for nodes in ${NODE_LIST0[@]}; do

for profiler in ${PROFILER_LIST[@]}; do

    for scalingtype in ${SCALINGTYPE_LIST[@]}; do

        SIZE_LIST=(`getSizeList ${app_name}`)

        for size in ${SIZE_LIST[@]}; do

            for taskfactor in ${TASKFACTOR_LIST[@]}; do

                for arch0 in ${ARCH_LIST[@]}; do

                    arch="${arch0}"
                    INSTPATHROOT="install"

                    for appopts in ${APPOPTS_LIST[@]}; do

                        for schedulername in ${SCHEDULER_LIST[@]}; do

                            if [[ $arch == "x86" && $nodes > 1 ]]; then
                                break;
                            fi

                                for computeThreads in ${THREAD_LIST[@]}; do

                                    install_root=${INSTPATHROOT}_${profiler}_${scalingtype}_${size}_sub${taskfactor}_${appopts}_${schedulername}_${nodes}x${computeThreads}
                                    ndir=${install_root}/${arch}

                                    cd ${AROOT}/${ndir}

                                    echo ${ndir}
                                    echo "$OCR_TOP/ocr/scripts/Profiler/analyzeProfile.py -e EVENT_OTHER -t '*' > all.prof"
                                    $OCR_TOP/ocr/scripts/Profiler/analyzeProfile.py -e EVENT_OTHER -t '*' > all.prof

                                    cd ${AROOT}

                                    python $OCR_TOP/ocr/scripts/ProfilerUtils/profExtract.py ${ndir}

                                    if [ ! -e ${OUTNAME} ]; then
                                    string=`head -n 1 ${ndir}/*_breakdown.csv`
                                    echo ${string} >> ${root}_results.prof
                                    fi

                                    string=`tail -n 1 ${ndir}/*_breakdown.csv`
                                    echo ${string} >> ${root}_results.prof
                                    echo ${string}


                                done #computeThreads
                        done #schedulername
                    done #appopts
                done #arch
            done #taskfactor
        done #size
    done #scalingtype
done #profiler
done #nodes

fi

awk 'NR==FNR{a[NR]=$0;next}{print a[FNR],$0}' ${root}_results.post ${root}_results.prof > ${root}_resultsAll.post
