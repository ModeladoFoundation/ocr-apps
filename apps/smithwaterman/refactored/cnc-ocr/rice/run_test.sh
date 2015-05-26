#!/bin/bash

TMPFILE=.tmp.out

# Guess XSTACK_ROOT and CNCOCR_ROOT if they're not already set
export XSTACK_ROOT="${XSTACK_ROOT-${PWD%/xstack/*}/xstack}"
export CNCOCR_ROOT="${CNCOCR_ROOT-${XSTACK_ROOT}/hll/cnc}"
CNCOCR_BIN="${CNCOCR_ROOT}/bin"

# Sanity check on the above paths
if ! [ -f "${CNCOCR_ROOT}/bin/cncocr_t" ]; then
    echo "ERROR: Failed to find CNCOCR_ROOT (looking in '${CNCOCR_ROOT}')"
    exit 1
fi

# Run the translator if needed
[ -d "cncocr_support" ] || "${CNCOCR_BIN}/cncocr_t"

DATA_DIR="${XSTACK_ROOT}/apps/smithwaterman/datasets"

echo "Testing string1-small.txt vs string2-small.txt with width=3, height=3"
make run WORKLOAD_ARGS="3 3 ${DATA_DIR}/string1-small.txt ${DATA_DIR}/string2-small.txt" 2>&1 | tee $TMPFILE && fgrep -q 'score: 10' < $TMPFILE
RET1=$?
[ $RET1 = 0 ] && echo OK || echo FAIL

echo "Testing string1-large.txt vs string2-large.txt with width=569, height=1983"
make run WORKLOAD_ARGS="569 1983 ${DATA_DIR}/string1-large.txt ${DATA_DIR}/string2-large.txt" 2>&1 | tee $TMPFILE && fgrep -q 'score: 65386' < $TMPFILE
RET2=$?
[ $RET2 = 0 ] && echo OK || echo FAIL

rm $TMPFILE

[ $RET1 = 0 ] && [ $RET2 = 0 ]
