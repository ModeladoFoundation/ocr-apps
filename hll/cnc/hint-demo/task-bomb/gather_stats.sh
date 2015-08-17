#!/bin/bash

do_runs() {
    make clean
    make install
    sleep 0.5

    local OUTFILE=res_${1}.txt
    local TIMESFILE=times_${1}.txt
    local TMPFILE=.tmp.out

    export WORKLOAD_ARGS=""

    echo >$OUTFILE

    for i in {1..50}; do
        echo $1 $i
        make run &>$TMPFILE
        cat $TMPFILE>>$OUTFILE
    done

    rm $TMPFILE

    sed -n 's/.*wall clock.*0:\([.0-9]\+\)$/\1/p' < $OUTFILE > $TIMESFILE
}

unset OCR_CONFIG
export RUN_TOOL="/bin/time -v"
export CONFIG_NUM_THREADS=8
export CONFIG_FLAGS="--binding seq"

if [ "$1" = "s" ]; then
# stoker
ucnc_t -t stoke.cnct
do_runs stoker
elif [ "$1" = "p" ]; then
# priority
ucnc_t -t priority.cnct
export OCR_CONFIG=$PWD/priority.cfg
do_runs priority
else
# default
ucnc_t
do_runs default
fi
