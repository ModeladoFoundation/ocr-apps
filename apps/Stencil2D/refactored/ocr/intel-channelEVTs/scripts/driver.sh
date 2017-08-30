
JOB_PROPERTIES=${JOB_PROPERTIES-./scripts/job.properties}

. ${JOB_PROPERTIES}

BINARY_BASENAME=${BINARY_BASENAME-"stencil2d"}

function buildNameWithConfig {
    local prepend="${1}"
    local entries="${2}"
    local LIST_TUPLE_VARIABLES="${3}"
    local __resultvar=${4}
    res="${prepend}_"
    if [[ -n "${USE_EAGER_DB_HINT}" ]]; then
        res+="Eager"
    fi
    if [[ -n "${USE_STATIC_SCHEDULER}" ]]; then
        res+="Static"
    fi
    eval $__resultvar="'$res'"
}

function clean {
    rm -Rf ${CPGN}
}

function build {
    entries=""
    LIST_TUPLE_VARIABLES=""
    # Generate a binary name according to defined variables
    export TARGET_NAME= # Need to be visible in the env
    buildNameWithConfig "${BINARY_BASENAME}" "${entries}" "${LIST_TUPLE_VARIABLES}" TARGET_NAME
    # Clean-up
    rm -Rf ../../../../apps/libs/install
    rm -f ../../../../apps/libs/src/reduction/x86/*.o
    rm -f ../../../../apps/libs/src/reduction/x86/*.a
    rm -f ../../../../apps/libs/src/reductionEager/x86/*.o
    rm -f ../../../../apps/libs/src/reductionEager/x86/*.a
    rm -f build/x86-mpi/*
    rm -f install/x86-mpi/${TARGET_NAME}
    CFLAGS_EXT=""
    if [[ -n "${USE_EAGER_DB_HINT}" ]]; then
        CFLAGS_EXT+="-DUSE_EAGER_DB_HINT "
    fi
    if [[ -n "${USE_STATIC_SCHEDULER}" ]]; then
        CFLAGS_EXT+="-DUSE_STATIC_SCHEDULER "
    fi

    eval "CFLAGS=\"${CFLAGS_EXT} \" ${ADD_MAKEFLAGS} RUN_MODE=buildApp TARGET=${TARGET_NAME} V=1 make install"
}

function gen {
    # Generate a binary name according to defined variables
    entries=""
    LIST_TUPLE_VARIABLES=""
    export TPLARG_APP_NAME= # Need to be visible in the env
    buildNameWithConfig "${BINARY_BASENAME}" "${entries}" "${LIST_TUPLE_VARIABLES}" TPLARG_APP_NAME
    . ./scripts/invoke.sh gen
}

function run {
    entries=""
    LIST_TUPLE_VARIABLES=""
    for rank in `echo ${n}`; do
        export BATCH_SCRIPT=
        BATCH_SCRIPT="ws_${BATCH_TPL}.${rank}N.sh"
        . ./scripts/invoke.sh run
    done
}

function res {
    for i in `echo ${iter}`; do
        for t in `echo ${tile}`; do
            for node in `echo ${n}`; do
                export outdir=`echo ${CPGN}/jobdir.iter_$i.tile_$t/*.${node}N*/`
                #TODO: I had to change that because the mode where we segregate worker count
                # for d in `ls -d ${outdir}/job_output_*`; do
                    echo "outdir=${outdir}"
                    if [[ -n "${SHOW_SCALING}" ]]; then
                        ranks="${n}" ${REPO_TOP}/apps/apps/tools/execution_tools/batch/showMultiResults.sh "${PWD}/${outdir}" "${c}" pes_comp
                    fi
                    # if [[ -n "${SHOW_SCALING}" ]]; then
                    #     ranks="${n}" ${REPO_TOP}/apps/apps/tools/execution_tools/batch/showMultiResults.sh "${d}" "${c}" pes_comp
                    #     # ranks="${n}" ./scripts/batch/showMultiResults.sh "${d}" "${c}" pes_comp
                    # fi
                    if [[ -n "${SHOW_PSL}" ]]; then
                        ${REPO_TOP}/apps/apps/tools/analysis_tools/profilerStatisticsDriver.sh ${PWD}/${outdir}
                    fi
                # done
            done
        done
    done
}

while [[ $# -gt 0 ]]; do
    cmd="${1#-}"
    eval ${cmd}
    shift
done
