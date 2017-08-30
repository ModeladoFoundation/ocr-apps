#!/bin/bash

FOLDERS=$1

function preprocess {
    local file=$1
    local N=$2
    local C=(`more $file | grep "= NB" | cut -d' ' -f2-2 | sed -e s/NB_WORKERS=//g | sort -g | uniq`)
    local coreCount=0
    let i=0
    let j=0
    # 5 header | 3 info line per rank | 2 footer
    let ub=$(( 7 + ${N} * 3 ))
    more $file | grep "INFO" | {
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
        for i in `ls ${f}/jobdir/*/ws_reduction* | sort -V -k19 -t"_"`; do
            N=`echo $i | cut -d'_' -f19-19 | sed -e s/R//g`
            # echo "Processing $N nodes output log"
            rm tmp_agg_${N}N_*
            preprocess $i $N
            for j in `ls tmp_agg_${N}N_* | sort -t'_' -k4 -V`; do
                echo $j
                ./scripts/y_makeAllStats.bash $j
            done
        done
    done
}

function showPerFileResults {
    for f in `echo ${FOLDERS}`; do
        #TODO this sucks as it depends to much on the folder/filename formatting
        for i in `ls ${f}/jobdir/*/ws_reduction* | sort -V -k18 -t"_"`; do
            echo $i
            ./scripts/y_makeAllStats.bash $i
        done
    done
}

showDemangledResults
