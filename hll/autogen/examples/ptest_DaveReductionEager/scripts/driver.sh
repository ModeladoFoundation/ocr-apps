#!/bin/bash

#
# Driver to build binaries, generate and run batch, show results
#

export JOB_PROPERTIES=${JOB_PROPERTIES-scripts/job.properties}

. ${JOB_PROPERTIES}

BINARY_BASENAME=${BINARY_BASENAME-"z_ptest_DaveReduction"}

# buildBinaryName is being passed entries and a list of corresponding
# names and build a binary filename out of them.
function buildNameWithConfig {
    local prepend="${1}"
    local entries="${2}"
    local LIST_TUPLE_VARIABLES="${3}"
    local __resultvar=${4}
    entriesArray=(`echo "${entries}"`)
    res="${prepend}"
    let i=0
    for vars in `echo ${LIST_TUPLE_VARIABLES}`; do
        res+="_${vars}${entriesArray[$i]}"
        let i=$i+1
    done
    if [[ "${REDUCTION_IMPL}" == "EAGER" ]]; then
        res+="_Eager"
    fi
    #TODO: this would be better handled in a common script
    if [[ -n "${OCR_PROFILER}" ]]; then
        res+="_Prof"
    fi
    eval $__resultvar="'$res'"
}

function tokenizeTuple {
    local input="${1}" # a comma separate list
    local __resultvar=${2}
    res=`echo ${input} | tr ',' ' '`
    eval $__resultvar="'$res'"
}

# By Convention tuple's counts are in this order (Arity, Reductions, Nodes, Cores)
# with Nodes and Cores being optional
function build {
    #ASSERT LIST_TUPLE_VARIABLES is set
    for config in `echo ${TUPLES_CONFIGURATIONS}`; do
        # TODO assert entries and var sizes are compatible
        entries=""
        tokenizeTuple "${config}" entries
        # Generate a binary name according to defined variables
        buildNameWithConfig "${BINARY_BASENAME}" "${entries}" "${LIST_TUPLE_VARIABLES}" TARGET_NAME
        # Clean-up
        rm -Rf ../../../../apps/libs/install
        rm -f ../../../../apps/libs/src/reduction/x86/*.o
        rm -f ../../../../apps/libs/src/reduction/x86/*.a
        rm -f ../../../../apps/libs/src/reductionEager/x86/*.o
        rm -f ../../../../apps/libs/src/reductionEager/x86/*.a
        rm build/x86-mpi/*
        rm -f install/x86-mpi/${TARGET_NAME}
        # Build the make command line
        if [[ "${REDUCTION_IMPL}" == "EAGER" ]]; then
            ADD_MAKEFLAGS="REDUCTION_IMPL=EAGER"
        fi
        # A and R are always provided and @ 0 and 1. We don't need N and C to generate the binary.
        entriesArray=(`echo "${entries}"`)
        A_VAL=${entriesArray[0]}
        R_VAL=${entriesArray[1]}
        T_VAL=${entriesArray[2]}
        # RUN_MODE prevents from rebuilding OCR which can happen because
        # the app's CFLAGS are different from the one OCR has been built with
        eval "CFLAGS=\"-DCOUNT_FOR_FORKJOIN=${R_VAL} -DARITY=${A_VAL} -DREDUC_OPERATION_TYPE=${T_VAL}\" ${ADD_MAKEFLAGS} RUN_MODE=buildApp TARGET=${TARGET_NAME} V=1 make install"
    done
}

function gen {
    for config in `echo ${TUPLES_CONFIGURATIONS}`; do
        # TODO assert entries and var sizes are compatible
        entries=""
        tokenizeTuple "${config}" entries
        # Generate a binary name according to defined variables
        export TPLARG_APP_NAME= # Need to be visible in the env
        buildNameWithConfig "${BINARY_BASENAME}" "${entries}" "${LIST_TUPLE_VARIABLES}" TPLARG_APP_NAME
        entriesArray=(`echo "${entries}"`)
        nbEntries=${#entriesArray[*]}
        if [[ ${nbEntries} > 3 ]]; then
            export n="${entriesArray[3]}"
        fi
        if [[ ${nbEntries} > 4 ]]; then
            export c="${entriesArray[4]}"
        fi
        export TPLARG_JOB_NAME=
        buildNameWithConfig "ws_${BATCH_TPL}." "${entries}" "${LIST_TUPLE_VARIABLES}" TPLARG_JOB_NAME
        #TODO: check validity for N and C
        ./scripts/invoke.sh gen
    done
}

function treeNodeToResourceNode {
    echo ""
}

function run {
    for config in `echo ${TUPLES_CONFIGURATIONS}`; do
        # TODO assert entries and var sizes are compatible
        entries=""
        tokenizeTuple "${config}" entries
        export BATCH_SCRIPT=
        buildNameWithConfig "ws_${BATCH_TPL}." "${entries}" "${LIST_TUPLE_VARIABLES}" BATCH_SCRIPT
        BATCH_SCRIPT+=".sh"
        ./scripts/invoke.sh run
    done
}

function clean {
    rm -Rf ${CPGN}
}

function res {
    echo "./scripts/extractResults.sh ${CPGN}"
    ./scripts/extractResults.sh ${CPGN}
}

while [[ $# -gt 0 ]]; do
    cmd="${1#-}"
    echo "Invoking $cmd"
    eval ${cmd}
    shift
done
