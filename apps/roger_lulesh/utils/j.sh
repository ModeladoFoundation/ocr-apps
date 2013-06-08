#!/bin/sh
set -x
egrep '(Final|time =)'  /tmp/*/*.CE.00 | sed -e 's/^.*>>> //' | ./hex2sci
