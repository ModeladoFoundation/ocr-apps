#!/bin/bash

source experiments/x86/parameters.job

root="${JOBHEADER}_ocr_RSBench"

rm ${root}.post
rm ${root}1.post

for nodes in ${NODE_LIST0[@]}; do

    pfile=${root}_$nodes.out

    grep "Prof " $pfile >> ${root}1.post

    numRuns=`grep 'Prof ' $pfile | wc -l`

    for ((k=1;k<=numRuns;k++)); do
        tfile="temp.file"
        awk -v n="$k" '/Prof / {f=0}; f && c==n; /Prof / {f=1; c++}' $pfile > temp.file

        matchFound=`grep 'Lookups/s' $tfile | wc -l`
        if [[ $matchFound != 0 ]]; then
            a=`grep 'Lookups/s' $tfile | awk '{print $2}' | tr -d ',' | awk -v nexp="${matchFound}" '{sum1+=$1;} (NR%nexp)==0{print sum1/nexp; sum1=0;}'`
            b=`grep 'Runtime' $tfile | awk '{print $2}' | awk -v nexp="${matchFound}" '{sum1+=$1;} (NR%nexp)==0{print sum1/nexp; sum1=0;}'`
        else
            a="-"
            b="-"
        fi

        echo $a $b
        echo $a $b >> ${root}.post
    done

done


awk 'NR==FNR{a[NR]=$0;next}{print a[FNR],$0}' ${root}1.post ${root}.post > ${root}_results.post