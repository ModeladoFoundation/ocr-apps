CPGN=${CPGN-xp-benchmark-hpcgEager}
METRIC=${METRIC-mean}

if [[ $METRIC == "mean" ]]; then
    MFIELD="3"
fi

if [[ $METRIC == "avg" ]]; then
    MFIELD="4"
fi

#TODO doesn't sort properly
echo "Warning: sorting is not working properly, check output"
for file in `find  ${CPGN} -name "ws_hpcgEager*.*N" | sort -g`; do
    # Extract per rank timer
    more $file | grep rank |cut -d' ' -f3-3 > tmpRankTimer
    # Extract total time elapsed
    grep elapsed $file | cut -d' ' -f3-3 > tmpElapsedTimer
    EL=`python ./scripts/extractRankTimer.py tmpElapsedTimer`
    RA=`python ./scripts/extractRankTimer.py tmpRankTimer`
    A_EL=`echo $EL | cut -d' ' -f${MFIELD}`
    A_RA=`echo $RA | cut -d' ' -f${MFIELD}`
    R=`echo "scale=3; $A_EL/$A_RA" | bc`
    echo "${A_EL} ${A_RA} $R"
    rm tmpElapsedTimer
    rm tmpRankTimer
done

