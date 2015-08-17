#!/bin/bash

gather_stats() {
    local out=$1
    echo >$out
    for i in {1..50}; do
        echo $out $i
        make run &>>$out
    done
}

get_times() {
    sed -n 's/.*wall.*:\([.0-9]\+\)$/\1/p' < res_${1}.txt > times_${1}.txt
}

unset OCR_CONFIG
export RUN_TOOL='/bin/time -v'
export CONFIG_NUM_THREADS=8
export CONFIG_FLAGS="--binding seq"
export WORKLOAD_ARGS="13 5000"

# Baseline
ucnc_t
make clean
make install
gather_stats res_deq.txt
get_times deq

export OCR_CONFIG=$PWD/priority.cfg

# DFS priority
ucnc_t -t dfs.cnct
make
gather_stats res_dfs.txt
get_times dfs

# BFS priority
ucnc_t -t bfs.cnct
make
gather_stats res_bfs.txt
get_times bfs

