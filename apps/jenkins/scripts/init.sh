#!/bin/bash

# We start out in JJOB_PRIVATE_HOME

# Make sure the Jenkins system is fully accessible in the shared home
# We also copy the CnC-OCR and Rose toolchains
mkdir -p ${JJOB_SHARED_HOME}/apps/jenkins
mkdir -p ${JJOB_SHARED_HOME}/apps/apps
mkdir -p ${JJOB_SHARED_HOME}/apps/apps/jenkins
mkdir -p ${JJOB_SHARED_HOME}/apps/hll/cnc
mkdir -p ${JJOB_SHARED_HOME}/apps/hll/rose

cp -r ${JJOB_PRIVATE_HOME}/apps/jenkins/* ${JJOB_SHARED_HOME}/apps/jenkins/
cp -r ${JJOB_PRIVATE_HOME}/apps/apps/jenkins/* ${JJOB_SHARED_HOME}/apps/apps/jenkins/
rsync -aq -r ${JJOB_PRIVATE_HOME}/apps/apps/ ${JJOB_SHARED_HOME}/apps/apps/ --exclude libs
mkdir -p ${JJOB_SHARED_HOME}/apps/apps/libs
rsync -aq -r ${JJOB_PRIVATE_HOME}/apps/apps/libs/src ${JJOB_SHARED_HOME}/apps/apps/libs --exclude trilinos --exclude libcxx --exclude libswtest --exclude libunwind --exclude libcxxabi --exclude ocrscaffold --exclude "ocr-glibc"
# --exclude newlib
#find ${JJOB_PRIVATE_HOME}/apps/apps/libs/src -maxdepth 1 -type d -not -name "trilinos" -print0 | xargs -0 cp -r -t ${JJOB_SHARED_HOME}/apps/apps/libs/src
cp -rT ${JJOB_PRIVATE_HOME}/apps/hll/cnc ${JJOB_SHARED_HOME}/apps/hll/cnc
cp -rT ${JJOB_PRIVATE_HOME}/apps/hll/rose ${JJOB_SHARED_HOME}/apps/hll/rose

mkdir -p ${JJOB_SHARED_HOME}/apps/apps/libs/x86/lib
mkdir -p ${JJOB_SHARED_HOME}/apps/apps/libs/x86/include
