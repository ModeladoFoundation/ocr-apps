#!/bin/bash

source experiments/x86/parameters.job

root="${JOBHEADER}_MPI_XSBench"

rm ${root}.post
rm ${root}1.post

for nodes in ${NODE_LIST0[@]}; do

    pfile=${root}_$nodes.out

    #grep "scaling " $pfile >> ${root}1.post
    grep "scaling " ${root}_${nodes}.sh | sed 's/echo //g' >> ${root}1.post

    numRuns=`grep 'scaling ' $pfile | wc -l`
    numRuns_jobFile=`grep 'scaling ' ${root}_${nodes}.sh | wc -l`

    for ((k=1;k<=numRuns;k++)); do
        tfile="temp.file"
        awk -v n="$k" '/scaling / {f=0}; f && c==n; /scaling / {f=1; c++}' $pfile > temp.file

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

    for ((k=$numRuns+1;k<=$numRuns_jobFile;k++)); do
        a="-"
        b="-"
        echo $a $b >> ${root}.post
    done

done

awk 'NR==FNR{a[NR]=$0;next}{print a[FNR],$0}' ${root}1.post ${root}.post > ${root}_results.post
