#!/bin/bash

#
# Driver to build binaries, generate and run batch, show results
#

export JOB_PROPERTIES=${JOB_PROPERTIES-scripts/job.properties}

. ${JOB_PROPERTIES}

function build {
	eval "TARGET=${TARGET_NAME} RUN_MODE=buildApp V=1 make install"
}

function gen {
    # There's a nxyz entry per node entry we need to generate
    nxyzArray=(`echo "${nxyz}"`)
    nodeArray=(`echo "${NODE_SCALING}"`)
    nbEntries=${#nxyzArray[*]}
    if [[ $nbEntries != ${#nodeArray[*]} ]]; then
        echo "error: NODE_SCALING and nxyz of different size"
        exit 1
    fi
    echo "$nxyz"
    echo "In gen ${nbEntries}"
    let i=0
    while (( $i < $nbEntries )); do
        echo "${nxyzArray[$i]} => ${nodeArray[$i]}"
        TPLARG_APP_NAME=${TARGET_NAME} n=${nodeArray[$i]} c="${CORE_SCALING}" nxyz=${nxyzArray[$i]} ./scripts/invoke.sh gen
        let i=$i+1
    done
}

function run {
    TARGET=${TARGET_NAME} ./scripts/invoke.sh run
}

function clean {
    rm -Rf ${CPGN} build install/x86-mpi/${TARGET_NAME}
}

function res {
    filenamePrefix="ws_hpcgEager" c=${CORE_SCALING} ./scripts/invoke.sh res
}

while [[ $# -gt 0 ]]; do
    cmd="${1#-}"
    echo "Invoking $cmd"
    eval ${cmd}
    shift
done
