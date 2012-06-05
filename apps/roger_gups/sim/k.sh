#!/bin/sh
set -x
make logclean powerclean
make RAG=0 clean gups gups.s run power
./j.sh
make RAG=1 clean gups gups.s run power
./j.sh
