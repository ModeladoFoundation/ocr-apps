#!/bin/bash

# DO NOT CALL THIS SCRIPT DIRECTLY

# This script expects env vars:
#   TG_INSTALL       - The install directory of the tg repo
#   FSIM_EXE         - The fsim varient to use (no path)
#   LOGS_DIR         - The directory for fsim to place its logs
#   TEST_NAME        - The name of the test
#   WORKLOAD_INSTALL - The path to the workload
#   FSIM_ARGS        - The args (except for -L) to pass to fsim.
#
# This script optionally accepts:
#   VERBOSE          - If set, then write all of fsim's output to stdout
#   TIME_TESTS       - If this or VERBOSE is set, then the execution time in
#                      seconds of the test is output.
#   OUTPUT_FILE      - Log file (no path) to use as output instead of stdout
#   TIMEOUT_SECONDS  - Number of seconds to allow the test to execute before
#                      aborting. If empty, 0 or unset then no timeout.
#
# This script accepts command line parameters which are all regular
# expressions. The output of the fsim simulation must contain lines
# which matche each of these regular expressions in order to succeed.
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
unset TIMEOUT_PID

function kill_fsim() {
  killall -e -9 -u $(whoami) $TG_INSTALL/bin/$FSIM_EXE 2>/dev/null
}

FALURE_REGEXS="\("\
"FAILURE in Simulation\|"\
"Can't open XE executable file\|"\
"ABEND CALLED:\|"\
"fsim: Unable to load config file\|"\
"fsim: Unable to find XE ELF input filename\)"

function fsim_success_test() {
  FOUND_REGEX=1
  while read LINE; do
    [[ -n $VERBOSE ]] && echo $LINE

    [[ -z $OUTPUT_FILE ]] && echo $LINE >> $LOGS_DIR/fsim_out

    if [[ $FOUND_REGEX -le ${#REGEXS[@]} ]]; then
      echo $LINE | grep -q "${REGEXS[$FOUND_REGEX]}" && ((FOUND_REGEX++))
    fi

    if echo $LINE | grep -q "$FALURE_REGEXS"; then
      kill_fsim
      return 1
    fi
  done

  if [[ $FOUND_REGEX -le ${#REGEXS[@]} ]]; then
    echo "Output never matched '${REGEXS[$FOUND_REGEX]}'" 1>&2
    return 1
  fi
}

function run_timeout() {
  sleep ${TIMEOUT_SECONDS}
  echo "Test timed out." 1>&2
  kill_fsim
}

function activate_timeout() {
  run_timeout &
  TIMEOUT_PID=$!
}

echo "Running test $TEST_NAME"

# Checking that $LOGS_DIR is not empty just for sanity. We do not want to try to remove /
[[ -n $LOGS_DIR ]] && rm -rf $LOGS_DIR/*

kill_fsim

OLD_DIR=$(pwd)
cd $WORKLOAD_INSTALL

# Make sure that the timeout timer will get killed when the test finishes.
trap 'kill -9 $(jobs -p) 2>/dev/null' EXIT

# Start the timeout timer if there is one specified
[[ ${TIMEOUT_SECONDS:-0} != 0 ]] && activate_timeout
SECONDS=0

# Actually run fsim
FSIM_CMD="$TG_INSTALL/bin/$FSIM_EXE -L $LOGS_DIR $FSIM_ARGS"
[[ -n $VERBOSE ]] && echo "Running command '$FSIM_CMD'"
if [[ -z $OUTPUT_FILE ]]; then
  # Test fsim's output
  $FSIM_CMD 2>&1 | fsim_success_test
  RET=$?
else
  # Test log file instead of output
  if [[ -z $VERBOSE ]]; then
    $FSIM_CMD 1>$LOGS_DIR/fsim_out 2>&1
  else
    $FSIM_CMD 2>&1 | tee $LOGS_DIR/fsim_out
  fi
  if [[ -f $LOGS_DIR/$OUTPUT_FILE ]]; then
    cat $LOGS_DIR/$OUTPUT_FILE | fsim_success_test
    RET=$?
  else
    echo "Log file '$LOGS_DIR/$OUTPUT_FILE' doesn't exist" 1>&2
    RET=1
  fi
fi

# Kill the timeout timer
[[ -n $TIMEOUT_PID ]] && kill ${TIMEOUT_PID} 2>/dev/null && wait $TIMEOUT_PID 2>/dev/null

[[ -n $VERBOSE || -n $TIME_TESTS ]] && echo "Test took $SECONDS sec"

cd $OLD_DIR

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
