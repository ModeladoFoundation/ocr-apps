#!/bin/bash

sizes=("small")
iters=(1 12801)
BLOCKCOUNTS=(1 2)

mkdir -p analyzed

for BLOCKCOUNT in ${BLOCKCOUNTS[@]}; do
for size in ${sizes[@]}; do
for iter in ${iters[@]}; do
    jobHeader="${size}_${iter}_${BLOCKCOUNT}"
    mkdir -p analyzed/${jobHeader}
    python $OCR_TOP/ocr/scripts/tgStats_new/tgStats.py $PWD/install_${jobHeader}/tg/logs
    mv results/* analyzed/${jobHeader}/
    echo ${jobHeader}
    echo "instruction_breakdown.csv"
    cat analyzed/${jobHeader}/instruction_breakdown.csv
    echo "energy_breakdown.csv"
    cat analyzed/${jobHeader}/energy_breakdown.csv
    echo "net_traffic.csv"
    cat analyzed/${jobHeader}/net_traffic.csv
    echo "blockwise_net_traffic.csv"
    cat analyzed/${jobHeader}/blockwise_net_traffic.csv
done
done
done