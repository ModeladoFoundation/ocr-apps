#!/bin/sh
set -x
make clean all

cp -f ./Parameter0.txt ./Parameters.txt
RUN_TOOL=time make run
mv -f ./Detects.txt ./Detect0.txt

cp -f ./Parameter1.txt ./Parameters.txt
RUN_TOOL=time make run
mv -f ./Detects.txt ./Detect1.txt

cp -f ./Parameter2.txt ./Parameters.txt
RUN_TOOL=time make run
mv -f ./Detects.txt ./Detect2.txt

cp -f ./Parameter3.txt ./Parameters.txt
RUN_TOOL=time make run
mv -f ./Detects.txt ./Detect3.txt

cp -f ./Parameter4.txt ./Parameters.txt
RUN_TOOL=time make run
mv -f ./Detects.txt ./Detect4.txt

cp -f ./Parameter5.txt ./Parameters.txt
RUN_TOOL=time make run
mv -f ./Detects.txt ./Detect5.txt

cp -f ./Parameter6.txt ./Parameters.txt
RUN_TOOL=time make run
mv -f ./Detects.txt ./Detect6.txt

cp -f ./Parameter7.txt ./Parameters.txt
RUN_TOOL=time make run
mv -f ./Detects.txt ./Detect7.txt

cp -f ./Parameter8.txt ./Parameters.txt
RUN_TOOL=time make run
mv -f ./Detects.txt ./Detect8.txt

cp -f ./Parameter9.txt ./Parameters.txt
RUN_TOOL=time make run
mv -f ./Detects.txt ./Detect9.txt

wc Detect?.txt
