#!/bin/sh
set -x
grep time\ = /tmp/*/*.CE.00 >j
sed -e 's/^.*>>> //' <j >jj
./hex2sci <jj
