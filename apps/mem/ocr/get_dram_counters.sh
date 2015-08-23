#!/bin/bash
counters=(
    'LOCAL_READ_COUNT'
    'REMOTE_READ_COUNT'
    'LOCAL_WRITE_COUNT'
    'REMOTE_WRITE_COUNT'
    'BYTES_READ'
    'BYTES_WRITTEN'
    'INSTRUCTIONS_EXECUTED'
    )
for i in "${counters[@]}" 
do
    #echo $i
    grep -v DMA_REMOTE_READ_COUNT install/tg/logs/ocr-cholesky.log.brd00.dram00.BSM.0*|grep $i|awk '{ sum+=$4} END {print sum}' 
done
#grep -v DMA_REMOTE_READ_COUNT install/tg/logs/ocr-cholesky.log.brd00.chp00.unt00.blk00.XE.0*|grep "LOCAL_READ_COUNT\|REMOTE_READ_COUNT\|LOCAL_WRITE_COUNT\|REMOTE_READ_COUNT\|BYTES_READ\|BYTES_WRITTEN"|tail -20|awk NR%5==0

