#!/bin/bash

# DO NOT CALL THIS SCRIPT DIRECTLY

# This script expects env vars:
#   TG_INSTALL       - The install directory of the tg repo
#   FSIM_EXE         - The fsim varient to use (no path)
#   LOGS_DIR         - The directory for fsim to place its logs
#   VERBOSE          - If set, then write all of fsim's output to stdout
#   TEST_NAME        - The name of the test
#   WORKLOAD_INSTALL - The path to the workload
#   FSIM_ARGS        - The args (except for -L) to pass to fsim.
#
# This script accepts command line parameters which are all regular
# expressions. The output of the fsim simulation must contain a line
# which matches each of these regular expressions in order to succeed.
# Note: these regular expressions must be encountered in order, and each
# ouput line can only provide a match for one regular expression.
#
# This script also makes a file called fsim_out in the LOGS_DIR directory
# containing a copy of all of the output of fsim

if [[ -z $TEST_NAME ]]; then
  echo "Do not call this script directly."
  echo "Instead, call one of the run-fsim*-tests.sh files"
  exit 1
fi

REGEXS=("$@")

function kill_fsim() {
  killall -e -9 -u $(whoami) $TG_INSTALL/bin/$FSIM_EXE 2>/dev/null
}

function fsim_success_test() {
  FOUND_REGEX=1
  while read LINE; do
    [[ -n $VERBOSE ]] && echo $LINE

    echo $LINE >> $LOGS_DIR/fsim_out

    if [[ $FOUND_REGEX -le ${#REGEXS[@]} ]]; then
      echo $LINE | grep -q "${REGEXS[$FOUND_REGEX]}" && ((FOUND_REGEX++))
    fi

    if echo $LINE | grep -q "FAILURE in Simulation"; then
      kill_fsim
      return 1
    fi
  done

  if [[ $FOUND_REGEX -le ${#REGEXS[@]} ]]; then
    echo "Output never matched '${REGEXS[$FOUND_REGEX]}'" 1>&2
    return 1
  fi
}

echo "Running test $TEST_NAME"

# Checking that $LOGS_DIR is not empty just for sanity. We do not want to try to remove /
[[ -n $LOGS_DIR ]] && rm -rf $LOGS_DIR/*

kill_fsim

# Actually run fsim
[[ -n $VERBOSE ]] && echo "Running command '$TG_INSTALL/bin/$FSIM_EXE -L $LOGS_DIR $FSIM_ARGS'"
$TG_INSTALL/bin/$FSIM_EXE -L $LOGS_DIR $FSIM_ARGS 2>&1 | fsim_success_test
RET=$?

if [[ $RET -ne 0 ]]; then
  echo " !!! Test $TEST_NAME failed !!!" 1>&2
  echo
fi

#Unset the env variables for the test
unset TEST_NAME
unset WORKLOAD_INSTALL
unset FSIM_ARGS
unset FSIM_CMD
unset FSIM_STDIN

kill_fsim

exit $RET
