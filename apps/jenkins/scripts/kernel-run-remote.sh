#!/bin/bash
#Arg def
#$1 app
#$2 OCR_TYPE
#$3 job name
#$4 number of iterations for obtaining avg. execution time

if [ $# -ne 4 ]; then
    echo "Usage <sub-directory> <arch>"
    exit 1
fi

WORKLOAD_INSTALL=${WORKLOAD_INSTALL_ROOT}/$2

echo "Running kernel '$1' for architecture '$2'"
rm -f ${WORKLOAD_INSTALL}/runlog
timeFile=$(mktemp)
export RUN_TOOL=/usr/bin/time\ \-o\ $timeFile\ \-\-append
for i in `seq 1 $4`; do
    WORKLOAD_EXEC=${WORKLOAD_INSTALL} RUN_JENKINS=runApp make -f ${WORKLOAD_INSTALL}/Makefile.$2 run 2>&1 > ${WORKLOAD_INSTALL}/runlog
    RETURN_CODE=$?
    if [ $RETURN_CODE -ne 0 ]; then
        break
    fi
done


if [ $RETURN_CODE -eq 0 ]; then
    echo "**** Run SUCCESS ****"
else
    echo "**** Run FAILURE ****"
fi

if [ -d "${JJOB_SHARED_HOME}/xstack/runtime" ]; then
    echo "${JJOB_SHARED_HOME}/xstack/runtime already exists! Skipping mkdir .."
else
    mkdir ${JJOB_SHARED_HOME}/xstack/runtime
fi

# copying timing file from local to shared runtime dir
cp $timeFile ${JJOB_SHARED_HOME}/xstack/runtime/time_$3.txt
rm -f $timeFile
unset RUN_TOOL

exit $RETURN_CODE
