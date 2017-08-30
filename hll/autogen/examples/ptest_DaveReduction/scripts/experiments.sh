
# jobs are serialized with -N 63 in the batch file

#
# Driver to go over a set of experiments parameters
# and build, generate, run, show results
#

CMD_BUILD="-build"
CMD_GEN="-gen"

# that means now I need generate binaries for a precise configuration since
# everything must match Nx_Cy_Ak_TNk
# Returns the number of resource nodes used for a given treeNode count / index
# this should be in experiments now

#
# This is mostly to factorize code
function treeNodeToResourceNode {
    # The list of resource nodes counts
    local RNODES="$1"
    # The current index of the treeNodes in the treeNodes list
    local i="$2"
    # The current treeNodes count
    local treeNodes="$3"
    # Where to put the result
    local __resultvar=$4
    rnodes=""
    if [[ -n "${RNODES}" ]]; then
        read -r -a rnodes <<< "${RNODES}"
    fi
    if [[ -z "${rnodes}" ]]; then
        n=${treeNodes}
    else
        n=${rnodes[$i]}
    fi
    eval $__resultvar="'$n'"
}

# We want to build a two-level
function fatTwoLevelConfig {
    # We want to build a 2 level tree that uses all the given resources.
    # Hence a root, along with RNODES-1 children

    # No over-subscription
    export REDUCTION_PER_CORE=${REDUCTION_PER_CORE-"1"}

    # For a given number of (node,core) compute how many reduction nodes are needed
    export TUPLES_CONFIGURATIONS=""
    export LIST_TUPLE_VARIABLES="A R T N C"
    local tuples=""
    for nodes in `echo ${NODE_SCALING}`; do
        for cores in `echo ${CORE_SCALING}`; do
            countReductionNodes=$(( ((${cores}-1)*${REDUCTION_PER_CORE})*(${nodes}-1)+1 )) # +1 for the root
            # The tree arity is the number of reduction nodes since we want only root+children
            arity=${countReductionNodes}
            tuples+="${arity},${countReductionNodes},${REDUC_OPERATION_TYPE},${nodes},${cores} "
        done
    done
    TUPLES_CONFIGURATIONS=${tuples}
}

# We want each core to contribute one reduction operation and have the distributed
# reduction organized as a two-level tree. Hence, arity is set to number of cluster node - 1.
function fatTwoLevelConfigFixedArity {
    # We want to build a 2 level tree that uses all the given resources.
    # Hence a root, along with RNODES-1 children

    # No over-subscription
    export REDUCTION_PER_CORE=${REDUCTION_PER_CORE-"1"}

    # For a given number of (node,core) compute how many reduction nodes are needed
    export TUPLES_CONFIGURATIONS=""
    export LIST_TUPLE_VARIABLES="A R T N C"
    local tuples=""
    for nodes in `echo ${NODE_SCALING}`; do
        for cores in `echo ${CORE_SCALING}`; do
            # One reduction per core (excl comms), per cluster node.
            countReductionNodes=$(( ((${cores}-1)*${REDUCTION_PER_CORE}) * ${nodes} ))
            # The tree arity is the number of cluster nodes -1 so that we always have two levels.
            arity=$((${nodes}-1))
            tuples+="${arity},${countReductionNodes},${REDUC_OPERATION_TYPE},${nodes},${cores} "
        done
    done
    TUPLES_CONFIGURATIONS=${tuples}
    export BINARY_BASENAME="z_ptest_reductionEventFixedArity"
}

# We want to build a two-level
function fatTwoLevelConfigUnderSubscribe {
    # No over-subscription
    export REDUCTION_PER_NODE=${REDUCTION_PER_NODE-"1"}
    export REDUCTION_PER_CORE=${REDUCTION_PER_CORE-"1"}

    # For a given number of (node,core) compute how many reduction nodes are needed
    export TUPLES_CONFIGURATIONS=""
    export LIST_TUPLE_VARIABLES="A R T N C"
    local tuples=""
    for nodes in `echo ${NODE_SCALING}`; do
        for cores in `echo ${CORE_SCALING}`; do
            countReductionNodes=$(( ${REDUCTION_PER_NODE} * (${nodes}-1)+1 )) # +1 for the root
            arity=${countReductionNodes}
            tuples+="${arity},${countReductionNodes},${REDUC_OPERATION_TYPE},${nodes},${cores} "
        done
    done
    TUPLES_CONFIGURATIONS=${tuples}
    export BINARY_BASENAME="z_ptest_DaveReductionUnderSubscribed"
}

# Creates as many reduction tree nodes as there are physical cluster node. Independently from the cores count.
# Input:
# - ARITY
# Output:
# - TUPLES_CONFIGURATIONS containing tuples representing (Arity, Reduction nodes count, Type of Reduciton, Cluster nodes count)
function binaryTreeConfig {
    # Binary tree of resources
    #TODO: depr here ?
    export RNODES=${RNODES-"1 3 7 15 31 63"}

    # Reduction tree arity
    export ARITY=${ARITY-2}

    export CORES=${CORES-"2 3 5 9 19 36"}
    export TUPLES_CONFIGURATIONS=""
    if [[ "$c" != "" ]]; then
        export LIST_TUPLE_VARIABLES="A R T N"
    else
        export LIST_TUPLE_VARIABLES="A R T N C"
    fi
    local tuples=""
    for nodes in `echo ${NODE_SCALING}`; do
        countReductionNodes=${nodes} # +1 for the root
        arity=${ARITY}
        if [[ "$c" != "" ]]; then
            tuples+="${arity},${countReductionNodes},${REDUC_OPERATION_TYPE},${nodes} "
        else
            for cores in `echo ${CORE_SCALING}`; do
                tuples+="${arity},${countReductionNodes},${REDUC_OPERATION_TYPE},${nodes},${cores} "
            done
        fi
    done
    TUPLES_CONFIGURATIONS=${tuples}
    export BINARY_BASENAME="z_ptest_DaveReductionBinary"
}

function doExperiment {
    # One of the configuration function must have been previously called
    if [[ "${CMD}" = "${CMD_BUILD}" ]]; then
        ./scripts/driver.sh ${CMD}
    else
        export CPGN=${CPGN-xp-benchmark-reduction}
        echo "TUPLES_CONFIGURATIONS=${TUPLES_CONFIGURATIONS}"
        ./scripts/driver.sh ${CMD}
    fi
}

# Generate
RNODES=${RNODES-"1 3 7 15 31 63"}
R_ARITY=${R_ARITY-2}
ARITIES=${ARITIES-"2 4 8 18 36"}

if [[ $# == 0 ]]; then
    CMDS=${CMDS-"-clean -build -gen -run"}
else
    CMDS="$*"
fi

NODE_SCALING=${NODE_SCALING-"1 3 7 15 31 63"}
CORE_SCALING=${CORE_SCALING-"2 3 5 9 18 36"}

# To sweep over cores:
export c=${CORE_SCALING}

export OCR_INSTALL=${OCR_INSTALL-$HOME/ocr_install_mpiicc_redlib-nodebug}

export REDUC_OPERATION_TYPE=${REDUC_OPERATION_TYPE-ALLREDUCE}

fatTwoLevelConfigFixedArity
PRFIX="$HOME/ocr_install_mpiicc_"
SIMPLE_OCR_NAME=`echo $OCR_INSTALL | sed -e "s|${PRFIX}||g"`
CPGN=${CPGN-binary-tree-${SIMPLE_OCR_NAME}-${REDUC_OPERATION_TYPE}-redlib-fatTwoLevelConfigFixedArity}
for CMD in `echo ${CMDS}`; do
    doExperiment
done

binaryTreeConfig
PRFIX="$HOME/ocr_install_mpiicc_"
SIMPLE_OCR_NAME=`echo $OCR_INSTALL | sed -e "s|${PRFIX}||g"`
CPGN=${CPGN-binary-tree-${SIMPLE_OCR_NAME}-${REDUC_OPERATION_TYPE}-redlib}
for CMD in `echo ${CMDS}`; do
    doExperiment
done

fatTwoLevelConfigUnderSubscribe
PRFIX="$HOME/ocr_install_mpiicc_"
SIMPLE_OCR_NAME=`echo $OCR_INSTALL | sed -e "s|${PRFIX}||g"`
CPGN=${CPGN-binary-tree-${SIMPLE_OCR_NAME}-${REDUC_OPERATION_TYPE}-fatTwoLevelConfigUnderSubscribe-redlib}
for CMD in `echo ${CMDS}`; do
    doExperiment
done
