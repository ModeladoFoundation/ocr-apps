#!/bin/sh
set -x
make logclean powerclean
make RAG_CACHE=0 RAG_ATOMIC=0 clean gups gups.s run power
./j.sh
make RAG_CACHE=0 RAG_ATOMIC=1 clean gups gups.s run power
./j.sh
make RAG_CACHE=1 RAG_ATOMIC=0 clean gups gups.s run power
./j.sh
make RAG_CACHE=1 RAG_ATOMIC=1 clean gups gups.s run power
./j.sh
