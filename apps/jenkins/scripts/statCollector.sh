#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage <directory containing running times> <number of samples>"
    exit 1
fi

python ${JJOB_SHARED_HOME}/xstack/apps/jenkins/scripts/extractStat.py $1 $2
RET_VAL=$?
if [ $RET_VAL -eq 0 ]; then
    echo " ---- Statistic extraction SUCCESSFUL ----"
else
    echo " ---- Statistic extraction FAILED ----"
fi
exit $RET_VAL

