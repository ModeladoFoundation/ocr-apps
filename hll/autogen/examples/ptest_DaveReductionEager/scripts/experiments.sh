
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
            # For the reduction library we can't have arity zero in the single node case. Default to the number of cores instead
            arity=$((${nodes}-1))
            if [[ $arity -eq 0 ]]; then
                arity=$((${cores}-1))
            fi
            tuples+="${arity},${countReductionNodes},${REDUC_OPERATION_TYPE},${nodes},${cores} "
        done
    done
    TUPLES_CONFIGURATIONS=${tuples}
    export BINARY_BASENAME="z_ptest_reductionEventFixedArityEager"
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
    export BINARY_BASENAME="z_ptest_DaveReductionUnderSubscribedEager"
}


# Creates as many reduction tree nodes as there are physical cluster node. Independently of the core count.
# Input:
# - ARITY
# Output:
# - TUPLES_CONFIGURATIONS containing tuples representing (Arity, Reduction nodes count, Type of Reduciton, Cluster nodes count)
function kAryTreeConfig {
    local CORE_SCALING_TYPE="$1"
    local ARITY="$2"

    export TUPLES_CONFIGURATIONS=""
    if [[ "$c" != "" ]]; then
        export LIST_TUPLE_VARIABLES="A R T N"
    else
        export LIST_TUPLE_VARIABLES="A R T N C"
    fi
    local tuples=""
    for nodes in `echo ${NODE_SCALING}`; do
        countReductionNodes=${nodes}
        arity=${ARITY}
        if [[ "$c" != "" ]]; then
            tuples+="${arity},${countReductionNodes},${REDUC_OPERATION_TYPE},${nodes} "
        else
            for cores in `echo ${CORE_SCALING}`; do
                if [[ "$CORE_SCALING_TYPE" == "CoreScaling" ]]; then
                    adjCountReductionNodes=$(( $countReductionNodes * (${cores} -1) ))
                else
                    adjCountReductionNodes=$countReductionNodes
                fi
                tuples+="${arity},${adjCountReductionNodes},${REDUC_OPERATION_TYPE},${nodes},${cores} "
            done
        fi
    done
    TUPLES_CONFIGURATIONS=${tuples}
    export BINARY_BASENAME="z_ptest_DaveReductionBinaryEager"
}

# Creates as many reduction tree nodes as there are physical cluster node. Independently from the cores count.
# Input:
# - ARITY
# Output:
# - TUPLES_CONFIGURATIONS containing tuples representing (Arity, Reduction nodes count, Type of Reduciton, Cluster nodes count)
function binaryTreeConfigSingleCore {
    kAryTreeConfig "CoreFixed" 2
    export BINARY_BASENAME="z_ptest_reductionEventBinarySC"
}

# Creates as many reduction tree nodes as there are physical cluster node times cores
# Input:
# - ARITY
# Output:
# - TUPLES_CONFIGURATIONS containing tuples representing (Arity, Reduction nodes count, Type of Reduciton, Cluster nodes count)
function binaryTreeConfigCoreScaling {
    unset c
    kAryTreeConfig "CoreScaling" 2
    export BINARY_BASENAME="z_ptest_reductionEventBinaryCS"
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

function nerscRun {
    export TPLARG_QUEUE=regular
    export BATCH_TPL=reduction-nersc
    export TPLARG_MIN=05
    export CORE_SCALING_SAVE="${CORE_SCALING}"
    export OCR_INSTALL=${SCRATCH}/ocr_install_mpiicc-072717-redevt-nodebug
    export REDUC_OPERATION_TYPE=${REDUC_OPERATION_TYPE-ALLREDUCE}
    export REDUCTION_IMPL="EAGER"

    fatTwoLevelConfigFixedArity
    PRFIX="${SCRATCH}/ocr_install_mpiicc-"
    SIMPLE_OCR_NAME=`echo $OCR_INSTALL | sed -e "s|${PRFIX}||g"`
    CPGN=binary-tree-${SIMPLE_OCR_NAME}-${REDUC_OPERATION_TYPE}-redlib-edison-fatTwoLevelConfigFixedArity
    for CMD in `echo ${CMDS}`; do
        doExperiment
    done
    if [[ -n "${LOG_CPGN}" ]]; then
        echo "$CPGN" >> ${LOG_CPGN}
    fi

    for mc in `echo ${CORE_SCALING}`; do
        CORE_SCALING=${mc}
        binaryTreeConfigCoreScaling
        PRFIX="${SCRATCH}/ocr_install_mpiicc-"
        SIMPLE_OCR_NAME=`echo $OCR_INSTALL | sed -e "s|${PRFIX}||g"`
        CPGN=cpgn-${SIMPLE_OCR_NAME}-${REDUC_OPERATION_TYPE}-redlib-edison-binaryTreeConfigCoreScaling-${mc}c
        for CMD in `echo ${CMDS}`; do
            doExperiment
        done
        if [[ -n "${LOG_CPGN}" ]]; then
            echo "$CPGN" >> ${LOG_CPGN}
        fi
    done

    export CORE_SCALING=${CORE_SCALING_SAVE}
    export c=${CORE_SCALING}
    binaryTreeConfigSingleCore
    PRFIX="${SCRATCH}/ocr_install_mpiicc-"
    SIMPLE_OCR_NAME=`echo $OCR_INSTALL | sed -e "s|${PRFIX}||g"`
    CPGN=binary-tree-${SIMPLE_OCR_NAME}-${REDUC_OPERATION_TYPE}-redlib-edison-binaryTreeConfigSingleCore
    export c=${CORE_SCALING}
    for CMD in `echo ${CMDS}`; do
        doExperiment
    done
    if [[ -n "${LOG_CPGN}" ]]; then
        echo "$CPGN" >> ${LOG_CPGN}
    fi
}

function edisonRun {
    export CORE_SCALING=${CORE_SCALING-"2 3 5 9 17 24"}
    nerscRun
}

function thorRun {
    export OCR_INSTALL=${OCR_INSTALL-$HOME/ocr_install_mpiicc_redlib-nodebug}

    export REDUC_OPERATION_TYPE=${REDUC_OPERATION_TYPE-ALLREDUCE}
    export REDUCTION_IMPL="EAGER"

    for mc in `echo ${CORE_SCALING}`; do
        CORE_SCALING=${mc}
        binaryTreeConfigCoreScaling
        PRFIX="$HOME/ocr_install_mpiicc_"
        SIMPLE_OCR_NAME=`echo $OCR_INSTALL | sed -e "s|${PRFIX}||g"`
        CPGN=cpgn-${SIMPLE_OCR_NAME}-${REDUC_OPERATION_TYPE}-redlib-eager-binaryTreeConfigCoreScaling3-${mc}c
        for CMD in `echo ${CMDS}`; do
            doExperiment
        done
    done

    export CORE_SCALING="2 3 5 9 18 36"
    export c=${CORE_SCALING}
    binaryTreeConfigSingleCore
    PRFIX="$HOME/ocr_install_mpiicc_"
    SIMPLE_OCR_NAME=`echo $OCR_INSTALL | sed -e "s|${PRFIX}||g"`
    CPGN=cpgn-${SIMPLE_OCR_NAME}-${REDUC_OPERATION_TYPE}-redlib-eager-binaryTreeConfigSingleCore3
    for CMD in `echo ${CMDS}`; do
        doExperiment
    done
}

edisonRun
