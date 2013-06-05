#!/bin/sh
set -x
rm -rf nohup.out /tmp/roger-fsim.???? /tmp/fsim.cfgfile.??????

make run 2>&1 | tee nohup.out
#../../../ss/xe-sim/fsim -s -c lulesh.cfg -c ../../../ss/xe-sim/configs/localhost.cfg

grep CONSOLE logs/*.CE.00 | grep -i RAG

grep CONSOLE /tmp/roger-fsim*/*.CE.00 | grep -i RAG
