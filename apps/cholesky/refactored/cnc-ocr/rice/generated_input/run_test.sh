#!/bin/bash

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

PREFIX="Result matrix checksum"
EXPECTED="$PREFIX: d5ff728615a593f"

OUTPUT="$(make run WORKLOAD_ARGS="2500 ${TILE:-125}")"

echo "$OUTPUT"

if (echo "$OUTPUT" | fgrep -q "$EXPECTED"); then
    echo checksum OK
else
    echo FAILED
    exit 1
fi

