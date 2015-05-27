#!/bin/bash

ROOT="${CNCOCR_ROOT-"${XSTACK_ROOT?Missing CNCOCR_ROOT or XSTACK_ROOT environment variable}/hll/cnc"}"
APP_PATH="$1"

case "$1" in
    /*) # already full path
        ;;
    *)  # relative example path
        APP_PATH="${ROOT}/examples/$1"
esac

echo "$APP_PATH"
