#!/bin/bash
#
# This file is subject to the license agreement located in the file LICENSE
# and cannot be distributed without it. This notice cannot be
# removed or modified.
#
if [ -f $1 ]
then
    export PASSED=`grep "PASSED" $1 | wc -l`
    export FAILED=`grep "FAILED" $1 | wc -l`
    export TOTAL=`grep "ED" $1 | wc -l`
    echo "Total tests = " $TOTAL ": PASSED = " $PASSED", FAILED = " $FAILED
else
    echo "File $1 not found."
fi
