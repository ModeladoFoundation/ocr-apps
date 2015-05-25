#!/bin/bash
###############################################################################
#
# Script to test hpgmg installation on known good problem sizes
#
# Author: Tom Henretty <henretty@reservoir.com
#
###############################################################################

# $1 - 'modern' or 'exascale'
# $2 - log2 box dim
# $3 - target boxes
test() {
    OCR_CONFIG=$OCR_INSTALL/config/mach-hc-16w.cfg
    echo "Testing $1 with log2 box dim = $2 and target boxes = $3, please wait..."
    ./run.sh $1 $OCR_CONFIG $2 $3 | grep error | grep h

    if [ "$1" == "modern" ] ; then
    	echo "Expected:"
	MODERN_RESULT=' h =  7.812500000000000e-03  ||error|| =  5.797955958337405e-08'
	echo "$MODERN_RESULT"
    else
    	echo "Expected ('x' values may vary):"
	EXASCALE_RESULT=' h =  7.812500000000000e-03  ||error|| =  5.7749xxxxxxxxxxxe-08'
	echo "$EXASCALE_RESULT"
    fi
    echo ""
}

. ./env.sh

# Test modern
test modern 4 512
test modern 5 64
test modern 6 8

# Test exascale
test exascale 4 512
test exascale 5 64
test exascale 6 8
