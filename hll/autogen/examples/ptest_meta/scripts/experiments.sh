
export OCR_INSTALL=${OCR_INSTALL-"$SCRATCH/ocr_install_mpich-080117-redevt-nodebug"}

export META_CPGN="reduction-meta-sweep"
export LOG_CPGN="$PWD/log_cpgn"

export NODE_SCALING=${NODE_SCALING-"1 3 7 15 31 63"}
export CORE_SCALING=${CORE_SCALING-"2"}

function clean {
    echo "clean"
    rm -Rf ${META_CPGN}
    rm -Rf ${LOG_CPGN}
}

function build {
    echo "build-none"
}

function invokeSelf {
    # This driver must go over the super set of node configurations
    # Sub drivers only do their local configurations
    cmd=$1
    TPLARG_QUEUE="regular" JOB_PROPERTIES="./scripts/job.properties" TPLARG_MIN=59 WORKLOAD_ARGS_NAMES=" " REPO_TOP="$PWD/../../../../.." BATCH_TPL="reduction-meta-nersc" CPGN="${META_CPGN}" n=${n-${NODE_SCALING}} c=${c-${CORE_SCALING}} ./scripts/invoke.sh ${cmd}
}

function gen {
    mkdir install 2>/dev/null # work around checks in driver since this is not really an application
    invokeSelf gen
}

function run {
    # invokeSelf run
    #do not provide $n otherwise it doesn't work
    for n in `echo ${NODE_SCALING}`; do
        TPLARG_QUEUE="regular" BATCH_SCRIPT="ws_reduction-meta-nersc.${n}N.sh" JOB_PROPERTIES="./scripts/job.properties" WORKLOAD_ARGS_NAMES=" " REPO_TOP="$PWD/../../../../.." BATCH_TPL="reduction-meta-nersc" CPGN="${META_CPGN}" n=${n} ./scripts/invoke.sh run
    done
}

function res {
    BASE=${PWD}
    export LOG_CPGN=${PWD}/log_cpgn
    let i=0
    for cpgn in `cat ${LOG_CPGN}`; do
        cd ${cpgn}/..
        echo ${BASE}/out-${i}
        let i=$i+1
    done
}

function invoke {
    cmd=$1
    if [[ "$cmd" != "run" ]]; then # build gen res
        echo "ptest_MpiReduction"
        cd ../ptest_MpiReduction
        ./scripts/driver.sh -${cmd}

        echo "ptest_ReductionEvent"
        cd ../ptest_ReductionEvent
        FORCE_CC=yes OCR_TYPE=x86-mpi ./scripts/experiments.sh -${cmd}

        echo "ptest_DaveReductionEager"
        cd ../ptest_DaveReductionEager
        FORCE_CC=yes OCR_TYPE=x86-mpi ./scripts/experiments.sh -${cmd}
    fi
    if [[ "$cmd" != "res" ]]; then # build gen run
        echo "ptest_meta"
        cd ../ptest_meta
        eval ${cmd}
    fi
}

echo "START DATE:"
date

while [[ $# -gt 0 ]]; do
    cmd="${1#-}"
    invoke "$cmd"
    shift
done

echo "STOP DATE:"
date

