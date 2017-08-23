#!/bin/bash

# Processes all ws_reduction* files found in the given experimental folder

# Experiment folder
FOLDERS=$1
ordering="$2"

function preprocess {
    local file=$1
    local N=$2
    echo "file is $file"
    local C=(`more $file | grep "= NB" | cut -d' ' -f2-2 | sed -e s/NB_WORKERS=//g | sort -g | uniq`)
    local coreCount=0
    let i=0
    let j=0
    # 5 header | 3 info line per rank | 2 footer
    let ub=$(( 7 + ${N} * 3 ))
    more $file | grep "INFO" | {
        # C is an array of core count
        # For each core count, looks for 'ub' consecutive lines and dump
        # them into the current worker count tmp file.
        while read line ; do
            if [[ $i -eq 0 ]]; then
                if [[ $j -eq ${#C[@]} ]]; then
                    let j=0
                fi
                let coreCount=${C[$j]}
            fi
            echo $line >> tmp_agg_${N}N_${coreCount}C
            let i=$i+1
            if [[ $i -eq $ub ]]; then
                let j=$j+1
                let i=0
            fi
        done
    }
}

function showDemangledResults {
    for f in `echo ${FOLDERS}`; do
        # 21 is the position of the node argument in the ws_reduction's filename
        for i in `ls ${f}/jobdir/*/ws_reduction* | sort -V -k${NOFFSET} -t"_"`; do
            echo NOFFSET=${NOFFSET} $i
            N=`echo $i | cut -d'_' -f${NOFFSET}-${NOFFSET} | sed -e s/N//g`
            rm tmp_agg_${N}N_*
            preprocess $i $N
            for j in `ls tmp_agg_${N}N_* | sort -t'_' -k4 -V`; do
                echo $j
                ./scripts/y_makeAllStats.bash $j
            done
        done
    done
}

function processPerCoreCampaign {
    for f in `echo ${FOLDERS}`; do
        for i in `ls ${f}/jobdir/*/ws_reduction* | sort -V -k${NOFFSET} -t"_"`; do
            N=`echo $i | cut -d'_' -f${NOFFSET}-${NOFFSET} | sed -e s/N//g`
            preprocess $i $N
        done
    done
}

function showPerCoreCampaign {
    rm tmp_agg_*
    for c in `echo "${CORE_SCALING}"`; do
        FOLDERS=${FOLDERS}-${c}c processPerCoreCampaign
    done
    for j in `ls tmp_agg_* | sort -t'_' -k3 -k4 -V`; do
        echo $j
        ./scripts/y_makeAllStats.bash $j
    done
}

if [[ `hostname` =~ "edison" ]]; then
    NOFFSET=9
else
    NOFFSET=21
fi

if [[ "$ordering" == "PER_CORE" ]]; then
    CORE_SCALING=${CORE_SCALING-"2 3 5 9 18 36"} showPerCoreCampaign
else
    showDemangledResults "PER_NODE"
fi