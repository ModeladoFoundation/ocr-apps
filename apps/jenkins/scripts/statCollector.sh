#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage <statDir_src> <localMach_dest>"
    exit 1
fi

python ${JJOB_SHARED_HOME}/apps/jenkins/scripts/extractStat.py $1 $2
RET_VAL=$?
if [ $RET_VAL -eq 0 ]; then
    echo " ---- Statistic extraction SUCCESSFUL ----"
else
    echo " ---- Statistic extraction FAILED ----"
fi
exit $RET_VAL

