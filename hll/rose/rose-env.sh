#!/bin/bash

# this script sets up the ROSE environment

ROSE_ROOT=/opt/bamboo_6_0/bin/rose
BOOST_ROOT=/opt/bamboo_6_0/bin/boost
echo $ROSE_ROOT

export ROSE_INS=$ROSE_ROOT
export LIBRARY_PATH=$ROSE_INS/lib
export ROSE_TOP=$ROSE_ROOT

export PATH=$PATH:/$ROSE_INS/bin
export LD_LIBRARY_PATH=/etc/alternatives/jre_1.7.0_openjdk/lib/amd64/server:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$BOOST_INS/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$ROSE_INS/lib:$LD_LIBRARY_PATH

export ZGRV_HOME=/opt/ZGRViewer-0.8.2/zgrviewer
export ZGRVIEW="java -jar $ZGRV_HOME/target/zgrviewer-0.8.2.jar"
