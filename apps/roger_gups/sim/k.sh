#!/bin/sh
set -x
#next four lines just compile and link to make sure all of the define combinations work
#make RUN_SUFFIX=DRAM--RMW RAG_CACHE=0 RAG_ATOMIC=0 CACHE_RUN=0 clean gups gups.s
#make RUN_SUFFIX=DRAM--AMO RAG_CACHE=0 RAG_ATOMIC=1 CACHE_RUN=0 clean gups gups.s
#make RUN_SUFFIX=Cache-RMW RAG_CACHE=0 RAG_ATOMIC=0 CACHE_RUN=1 clean gups gups.s
#make RUN_SUFFIX=Cache-AMO RAG_CACHE=0 RAG_ATOMIC=1 CACHE_RUN=1 clean gups gups.s
#exit
#these lines run the four GUPS options
rm -rf /tmp/roger_f_sim.????
make logclean powerclean
#make RUN_SUFFIX=DRAM--RMW RAG_CACHE=0 RAG_ATOMIC=0 CACHE_RUN=0 clean gups gups.s run power
#./j.sh
#make RUN_SUFFIX=DRAM--AMO RAG_CACHE=0 RAG_ATOMIC=1 CACHE_RUN=0 clean gups gups.s run power
#./j.sh
make RUN_SUFFIX=Cache-RMW RAG_CACHE=0 RAG_ATOMIC=0 CACHE_RUN=1 clean gups gups.s run power
./j.sh
#make RUN_SUFFIX=Cache-AMO RAG_CACHE=0 RAG_ATOMIC=1 CACHE_RUN=1 clean gups gups.s run power
#./j.sh
