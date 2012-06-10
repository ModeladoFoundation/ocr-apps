#!/bin/sh
set -x
rm -rf /tmp/roger_f_sim.????
make logclean powerclean
#make RAG_CACHE=1 RAG_ATOMIC=0 clean gups gups.s run power
#./j.sh
make RUN_SUFFIX=Cache RAG_CACHE=0 RAG_ATOMIC=0 CACHE_RUN=1 clean gups gups.s run power
./j.sh
#make RAG_CACHE=0 RAG_ATOMIC=1 clean gups gups.s run power
#./j.sh
#make RUN_SUFFIX=SpadAtomic RAG_CACHE=0 RAG_ATOMIC=1 CACHE_RUN=0 clean gups gups.s run power
#./j.sh
