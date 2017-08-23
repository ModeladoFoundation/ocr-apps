#!/bin/bash

#
# Driver to build binaries, generate and run batch, show results
#

export JOB_PROPERTIES=${JOB_PROPERTIES-scripts/job.properties}

. ${JOB_PROPERTIES}

function build {
    APP_NAME=allreduce
    for NB_RANKS in `echo ${NODE_SCALING}`; do
        NAME="${APP_NAME}_N${NB_RANKS}"
        BTPL_NAME="${NAME}.sh"
        echo "NB_RANKS=${NB_RANKS}"
        rm -f *.o
        echo "cc -O3 -DNB_RANKS=${NB_RANKS} -DNB_MSG=100000 -I../inc ${APP_NAME}.c x86.c -o ${NAME}"
        cc -O3 -DNB_RANKS=${NB_RANKS} -DNB_MSG=100000 -I../inc ${APP_NAME}.c x86.c -o ${NAME}
    done
}

function gen {
    APP_NAME=allreduce
   for NB_RANKS in `echo ${NODE_SCALING}`; do
        NAME="${APP_NAME}_N${NB_RANKS}"
        BTPL_NAME="${NAME}.sh"
        echo "NB_RANKS=${NB_RANKS}"
        TPLARG_MIN=${TPLARG_MIN-15}
        TPLARG_HOUR=${TPLARG_HOUR-00}
        TPLARG_NB_RUN=${TPLARG_NB_RUN-3}
        mkdir jobdir 2>/dev/null
        more ./scripts/batch/reduction-nersc.btpl | sed -e "s|TPLARG_REPO_TOP|$PWD/../../../../..|g" -e "s/TPLARG_JOB_NAME/${NAME}/g" -e "s/TPLARG_NODE_SCALING/${NB_RANKS}/g" \
                                         -e "s/TPLARG_SRUN_ARGS/${TPLARG_SRUN_ARGS}/g" -e "s/TPLARG_QUEUE/${TPLARG_QUEUE}/g" \
                                         -e "s/TPLARG_NB_NODES/${NB_RANKS}/g" -e "s/TPLARG_HOUR/${TPLARG_HOUR}/g" -e "s/TPLARG_MIN/${TPLARG_MIN}/g" \
                                         -e "s/TPLARG_NB_RUN/${TPLARG_NB_RUN}/g" > jobdir/${BTPL_NAME}
        chmod u+x jobdir/${BTPL_NAME}
    done
    if [[ -n "${LOG_CPGN}" ]]; then
        echo "$PWD" >> ${LOG_CPGN}
    fi
}

function run {
  echo "run"
}

function clean {
    echo "clean"
    rm -Rf jobdir
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
