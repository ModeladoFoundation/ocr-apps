#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage <directory containing running times> <number of samples>"
    exit 1
fi

SCRIPT_FOLDER=${JJOB_SHARED_HOME}/apps/apps/jenkins/scripts

# Create archive dirs if the framework is being run manually ( i.e. local run and not by jenkins)
if [ -z $WORKSPACE ]; then
    echo "---- Local execution of Regression Framework . Creating Archive dirs ----"
    mkdir ${JJOB_SHARED_HOME}/../performanceResults
    mkdir ${JJOB_SHARED_HOME}/../performanceResults/NightlyPerfStatApps
    mkdir ${JJOB_SHARED_HOME}/../performanceResults/NightlyPerfStatAppsScaling
fi

python ${SCRIPT_FOLDER}/extractStat.py $1 $2
RET_VAL=$?
if [ $RET_VAL -eq 0 ]; then
    echo " ---- Successfully extracted statistic from time files ----"
else
    echo " ---- Failure in statistic extraction from time files  ----"
    exit $RET_VAL
fi

# Generate plotGraph.py parsable stat file

if [ -z $WORKSPACE ]; then
    #  Manually execution of framework( i.e. local run and not by jenkins)
    python ${SCRIPT_FOLDER}/statFileFolderParser.py ${JJOB_SHARED_HOME}/../performanceResults/NightlyPerfStatApps/ ${JJOB_SHARED_HOME}/../NightlyPerfStatApps.txt
else
    python ${SCRIPT_FOLDER}/statFileFolderParser.py ${WORKSPACE}/performanceResults/NightlyPerfStatApps/ ${WORKSPACE}/NightlyPerfStatApps.txt
fi
RET_VAL=$?
if [ $RET_VAL -eq 0 ]; then
    echo " ---- Successfully generated plot script parsable regression input file ----"
else
    echo " ---- Failure in generation of plot script parsable regression input file ----"
    exit $RET_VAL
fi

if [ -z $WORKSPACE ]; then
    #  Manually execution of framework( i.e. local run and not by jenkins)
    python ${SCRIPT_FOLDER}/statFileFolderParser.py ${JJOB_SHARED_HOME}/../performanceResults/NightlyPerfStatAppsScaling/ ${JJOB_SHARED_HOME}/../NightlyPerfStatAppsScaling.txt
else
    python ${SCRIPT_FOLDER}/statFileFolderParser.py ${WORKSPACE}/performanceResults/NightlyPerfStatAppsScaling/ ${WORKSPACE}/NightlyPerfStatAppsScaling.txt
fi
RET_VAL=$?
if [ $RET_VAL -eq 0 ]; then
    echo " ---- Successfully generated plot script parsable scaling input file ----"
else
    echo " ---- Failure in generation of plot script parsable scaling input file ----"
    exit $RET_VAL
fi

# Plot these stat files
YMIN="0"
YMAX="4"

if [ -z $WORKSPACE ]; then
    #  Manually execution of framework( i.e. local run and not by jenkins)
    cat ${JJOB_SHARED_HOME}/../NightlyPerfStatApps.txt
    python ${SCRIPT_FOLDER}/plotGraph.py ${JJOB_SHARED_HOME}/../NightlyPerfStatApps.txt "Application Performance Trend Line" "Build" "Normalized Execution Time" "${YMIN}" "${YMAX}" "${JJOB_SHARED_HOME}/../perfStatAppsTrend.png"
else
    cat ${WORKSPACE}/NightlyPerfStatApps.txt
    python ${SCRIPT_FOLDER}/plotGraph.py ${WORKSPACE}/NightlyPerfStatApps.txt "Application Performance Trend Line" "Build" "Normalized Execution Time" "${YMIN}" "${YMAX}" "${WORKSPACE}/perfStatAppsTrend.png"
fi
RET_VAL=$?
if [ $RET_VAL -eq 0 ]; then
    echo " ---- Successfully generated Nightly Application Performance plot ----"
else
    echo " ---- Failure in generation of Nightly Application Performance plot ----"
    exit $RET_VAL
fi

#Adjusted y-axis range for scaling lines (perfect scaling should have a slope of 1)
YMAX="1"

if [ -z $WORKSPACE ]; then
    #  Manually execution of framework( i.e. local run and not by jenkins)
    cat ${JJOB_SHARED_HOME}/../NightlyPerfStatAppsScaling.txt
    python ${SCRIPT_FOLDER}/plotGraph.py ${JJOB_SHARED_HOME}/../NightlyPerfStatAppsScaling.txt "Application Scaling Trend Line" "Build" "Scaling Slope [ Higher the better ]"  "${YMIN}" "${YMAX}" "${JJOB_SHARED_HOME}/../perfStatAppsScalingTrend.png"
else
    cat ${WORKSPACE}/NightlyPerfStatAppsScaling.txt
    python ${SCRIPT_FOLDER}/plotGraph.py ${WORKSPACE}/NightlyPerfStatAppsScaling.txt "Application Scaling Line" "Build" "Scaling Slope [ Higher the better ]"  "${YMIN}" "${YMAX}" "${WORKSPACE}/perfStatAppsScalingTrend.png"
fi
RET_VAL=$?
if [ $RET_VAL -eq 0 ]; then
    echo " ---- Successfully generated Nightly Application Scaling plot ----"
else
    echo " ---- Failure in generation of Nightly Application Scaling plot ----"
    exit $RET_VAL
fi
exit $RET_VAL

