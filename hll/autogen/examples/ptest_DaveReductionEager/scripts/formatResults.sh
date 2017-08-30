#!/bin/bash

#
# Processes the output of extractResults and generate
# excel formulas to be copy-pasted into a spreadsheet
#
# Currently requires tweaking and setting default values
#

# "E" is the median column
COL="E"

# OFFSET is the line number the data dump in excel starts
OFFSET=20

function oldFormat {
    NODES="1 3 7 15 31 63"
    CORES="2 3 5 9 19 37"
    ARITIES="2 4 8 18 36"

    read -r -a nodes <<< "${NODES}"
    read -r -a cores <<< "${CORES}"
    read -r -a arities <<< "${ARITIES}"

    let i=${OFFSET}
    let a=0
    while (( $i < 1240 )); do
        let c=0
        let t=0
        ARAGG=""
        arity=${arities[$a]}
        tn_per_rn=$((${arity} / 2))
        for nd in `echo $NODES`; do
            if [[ $nd == 1 ]]; then
                ARAGG+="1 "
            else
                ARAGG+="$(($nd*${tn_per_rn}-(${tn_per_rn}-1))) "
            fi
        done
        echo "ARITY=${arity} ${ARAGG}"
        # echo "Cores/Nodes ${NODES}"
        echo "  ${NODES}"
        while (( $c < 6 )); do
        let n=0
        AGG="${cores[$c]}"
        while (( $n < 6 )); do
            AGG+=" =${COL}${i}"
            let n=$n+1;
            let i=$i+8
        done
        echo "$AGG"
        let i=$i+1 # skip invoke line
        let c=$c+1;
        done
        echo ""
        let a=$a+1
        let i=$i+1
    done
}

function newFormat {
    NODES="1 3 7 15 31 63"
    CORES="2 3 5 9 18 36"
    ARITIES="2 4 8 18 36"

    read -r -a nodes <<< "${NODES}"
    read -r -a cores <<< "${CORES}"
    read -r -a arities <<< "${ARITIES}"

    let i=${OFFSET}
    echo "  ${NODES}"
    for c in `echo $CORES`; do
        ARAGG="${c}"
        let j=${i}
        for n in `echo $NODES`; do
            if [[ $nd == 1 ]]; then
                ARAGG+="$c"
            else
                ARAGG+=" =${COL}${j}"
            fi
            let j=${j}+60
        done
        let i=${i}+10
        echo ${ARAGG}
    done
}

newFormat
