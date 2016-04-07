if [ "$#" -ne 2 ]; then
    echo "usage: showResults.sh dir scaling_type"
    exit 1
fi

printf "%8s %9s %8s %10s\n" "Nodes" "MFlops/s" "Scaling" "Efficiency"

DIRE=${1}
SCALING_TYPE=${2}

#TODO these should be read from a conf file or something
#They are currently duplicated across scripts

ranks="1 4 16 64 256"

#TODO This was just to get around sorting and figuring out the size
for count in `echo $ranks`; do
    input=${DIRE}/${SCALING_TYPE}_Stencil2D.${count}x24
    nexp=`grep "MFlop" $input | wc -l`
    RES=`grep "MFlop" $input | awk '{print $3 " " $7}' | awk -v nexp="${nexp}" '{sum1+=$1; sum2+=$2} (NR%nexp)==0{print
sum1/nexp " " sum2/nexp; sum1=0; sum2=0}'`
    flops=`echo $RES | awk '{print $1}'`
    if [[ $count == 1 ]]; then
        flops0=$flops
        count0=$count
    fi
    scaling=`echo $flops/$flops0 | bc -l`
    efficiency=`echo $scaling/$count*$count0 | bc -l`
    printf "%8d %9.3f %8.2f %10.3f\n" $count $flops $scaling $efficiency
done
