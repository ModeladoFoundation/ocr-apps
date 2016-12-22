#!/bin/bash

root="JOB_MPI_CoMD"

rm ${root}.post
rm ${root}1.post

for nodes in 1 4 8 64 216 512 1024; do

    pfile=${root}_$nodes.out

    grep "scaling " $pfile >> ${root}1.post

    numRuns=`grep 'scaling ' $pfile | wc -l`

    for ((k=1;k<=numRuns;k++)); do
        tfile="temp.file"
        awk -v n="$k" '/scaling / {f=0}; f && c==n; /scaling / {f=1; c++}' $pfile > temp.file

        matchFound=`grep 'timestep\s*10 \s*' $tfile | wc -l`
        if [[ $matchFound != 0 ]]; then
            a=`grep 'timestep\s*10 \s*' $tfile | awk '{print $4}' | awk -v nexp="${matchFound}" '{sum1+=$1;} (NR%nexp)==0{print sum1/nexp; sum1=0;}'`
        else
            a="-"
        fi

        matchFound=`grep 'force\s*101 \s*' $tfile | wc -l`
        if [[ $matchFound != 0 ]]; then
            b=`grep 'force\s*101 \s*' $tfile | awk '{print $4}' | awk -v nexp="${matchFound}" '{sum1+=$1;} (NR%nexp)==0{print sum1/nexp; sum1=0;}'`
        else
            b="-"
        fi

        matchFound=`grep 'atomHalo\s*101 \s*' $tfile | wc -l`
        if [[ $matchFound != 0 ]]; then
            c=`grep 'atomHalo\s*101 \s*' $tfile | awk '{print $4}' | awk -v nexp="${matchFound}" '{sum1+=$1;} (NR%nexp)==0{print sum1/nexp; sum1=0;}'`
        else
            c="-"
        fi


        echo $a $b $c
        echo $a $b $c >> ${root}.post

    done

done
