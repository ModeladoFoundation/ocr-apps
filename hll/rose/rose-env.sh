#!/bin/bash

# this script sets up the ROSE environment

export ROSE_TOP=/opt/edg4x-rose
echo $ROSE_TOP

export ROSE_BLD=$ROSE_TOP/compileTree
export ROSE_INS=$ROSE_TOP/installTree
export LIBRARY_PATH=$ROSE_INS/lib
#export BOOST_INS=$HOME/boost_1_46_1/installTree

export PATH=$PATH:/$ROSE_INS/bin
export LD_LIBRARY_PATH=/etc/alternatives/jre_1.7.0_openjdk/lib/amd64/server:$LD_LIBRARY_PATH
#export LD_LIBRARY_PATH=$BOOST_INS/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$ROSE_INS/lib:$LD_LIBRARY_PATH

export ZGRV_HOME=/opt/ZGRViewer-0.8.2/zgrviewer
export ZGRVIEW="java -jar $ZGRV_HOME/target/zgrviewer-0.8.2.jar"
