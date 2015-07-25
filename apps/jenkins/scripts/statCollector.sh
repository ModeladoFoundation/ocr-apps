#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage <directory containing running times> <number of samples>"
    exit 1
fi

SCRIPT_FOLDER=${JJOB_SHARED_HOME}/xstack/apps/jenkins/scripts

# Create archive dirs if the framework is being run manually ( i.e. local run and not by jenkin)
if [ -z $WORKSPACE ]; then
    echo "---- Local execution of Regression Framework . Creating Archive dirs ----"
    mkdir ${JJOB_SHARED_HOME}/../regressionResults
    mkdir ${JJOB_SHARED_HOME}/../regressionResults/NightlyRegressionStat
    mkdir ${JJOB_SHARED_HOME}/../regressionResults/NightlyScalingStat
fi

python ${SCRIPT_FOLDER}/extractStat.py $1 $2
RET_VAL=$?
if [ $RET_VAL -eq 0 ]; then
    echo " ---- Successfully extractd statistic from time files ----"
else
    echo " ---- Failure in statistic extraction from time files  ----"
    exit $RET_VAL
fi

# Generate plotGraph.py parsable stat file

if [ -z $WORKSPACE ]; then
    #  Manually execution of framework( i.e. local run and not by jenkin)
    python ${SCRIPT_FOLDER}/statFileFolderParser.py ${JJOB_SHARED_HOME}/../regressionResults/NightlyRegressionStat/ ${JJOB_SHARED_HOME}/../NightlyRegressionStat.txt
else
    python ${SCRIPT_FOLDER}/statFileFolderParser.py ${WORKSPACE}/regressionResults/NightlyRegressionStat/ ${WORKSPACE}/NightlyRegressionStat.txt
fi
RET_VAL=$?
if [ $RET_VAL -eq 0 ]; then
    echo " ---- Successfully generated plot script parsable regression input file ----"
else
    echo " ---- Failure in generation of plot script parsable regression input file ----"
    exit $RET_VAL
fi

if [ -z $WORKSPACE ]; then
    #  Manually execution of framework( i.e. local run and not by jenkin)
    python ${SCRIPT_FOLDER}/statFileFolderParser.py ${JJOB_SHARED_HOME}/../regressionResults/NightlyScalingStat/ ${JJOB_SHARED_HOME}/../NightlyScalingStat.txt
else
    python ${SCRIPT_FOLDER}/statFileFolderParser.py ${WORKSPACE}/regressionResults/NightlyScalingStat/ ${WORKSPACE}/NightlyScalingStat.txt
fi
RET_VAL=$?
if [ $RET_VAL -eq 0 ]; then
    echo " ---- Successfully generated plot script parsable scaling input file ----"
else
    echo " ---- Failure in generation of plot script parsable scaling input file ----"
    exit $RET_VAL
fi

# Plot these stat files

if [ -z $WORKSPACE ]; then
    #  Manually execution of framework( i.e. local run and not by jenkin)
    cat ${JJOB_SHARED_HOME}/../NightlyRegressionStat.txt
    python ${SCRIPT_FOLDER}/plotGraph.py ${JJOB_SHARED_HOME}/../NightlyRegressionStat.txt "Regression Trend Line" "Build" "Normalized Execution Time" "${JJOB_SHARED_HOME}/../RegressionTrendlineplot.png"
else
    cat ${WORKSPACE}/NightlyRegressionStat.txt
    python ${SCRIPT_FOLDER}/plotGraph.py ${WORKSPACE}/NightlyRegressionStat.txt "Regression Trend Line" "Build" "Normalized Execution Time" "${WORKSPACE}/RegressionTrendlineplot.png"
fi
RET_VAL=$?
if [ $RET_VAL -eq 0 ]; then
    echo " ---- Successfully generated Nightly Regression plot ----"
else
    echo " ---- Failure in generation of Nightly Regression plot ----"
    exit $RET_VAL
fi

if [ -z $WORKSPACE ]; then
    #  Manually execution of framework( i.e. local run and not by jenkin)
    cat ${JJOB_SHARED_HOME}/../NightlyScalingStat.txt
    python ${SCRIPT_FOLDER}/plotGraph.py ${JJOB_SHARED_HOME}/../NightlyScalingStat.txt "Scaling Trend Line" "Build" "Scaling Slope [ Higher the better ]" "${JJOB_SHARED_HOME}/../ScalingTrendlineplot.png"
else
    cat ${WORKSPACE}/NightlyScalingStat.txt
    python ${SCRIPT_FOLDER}/plotGraph.py ${WORKSPACE}/NightlyScalingStat.txt "Scaling Trend Line" "Build" "Scaling Slope [ Higher the better ]" "${WORKSPACE}/ScalingTrendlineplot.png"
fi
RET_VAL=$?
if [ $RET_VAL -eq 0 ]; then
    echo " ---- Successfully generated Scaling plot ----"
else
    echo " ---- Failure in generation of Scaling plot ----"
    exit $RET_VAL
fi
exit $RET_VAL

