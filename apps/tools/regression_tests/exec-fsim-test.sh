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
# which matches each of these regular expressions in order to succeed.
# Note: these regular expressions must be encountered in order, and each
# ouput line can only provide a match for one regular expression.
#
# This script also makes a file called fsim_out in the LOGS_DIR directory
# containing a copy of all of the output of fsim

if [ -z "$TEST_NAME" ]; then
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
    [ "$VERBOSE" ] && echo $LINE

    [ -z "$OUTPUT_FILE" ] && echo $LINE >> $LOGS_DIR/fsim_out

    if [ $FOUND_REGEX -le ${#REGEXS[@]} ]; then
      echo $LINE | grep -q "${REGEXS[$FOUND_REGEX]}" && ((FOUND_REGEX++))
    fi

    if echo $LINE | grep -q "$FALURE_REGEXS"; then
      kill_fsim
      return 1
    fi
  done

  if [ $FOUND_REGEX -le ${#REGEXS[@]} ]; then
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

#
# Check that our required arguments are set
#
[ "$WORKLOAD_INSTALL" ] || { echo "WORKLOAD_INSTALL not set!" ; exit 1; }
[ "$FSIM_ARGS" ]  || { echo "FSIM_ARGS not set!" ; exit 1; }
[ "$FSIM_EXE" ]   || { echo "FSIM_EXE not set!" ; exit 1; }
[ "$LOGS_DIR" ]   || { echo "LOGS_DIR not set!" ; exit 1; }
[ "$TEST_NAME" ]  || { echo "TEST_NAME not set!" ; exit 1; }
[ "$TG_INSTALL" ] || { echo "TG_INSTALL not set!" ; exit 1; }

#
# Create the logs dir if it doesn't exist, complain if it does but isn't a dir
# Clear out any residual content
#
if [ ! -e "$LOGS_DIR" ]; then
  mkdir -p $LOGS_DIR 2>/dev/null || { echo "Unable to create log dir '$LOGS_DIR'!"; exit 1; }
fi
[ -d "$LOGS_DIR" ] || { echo "log dir '$LOGS_DIR' is NOT a directory!" ; exit 1; }
rm -rf $LOGS_DIR/*

#
# paranoia - guarantee clean start
#
kill_fsim

# Make sure that the timeout timer will get killed when the test finishes.
trap 'kill -9 $(jobs -p) 2>/dev/null' EXIT

# Start the timeout timer if there is one specified
[ "${TIMEOUT_SECONDS:-0}" != 0 ] && activate_timeout

#
# Actually run fsim
#
SECONDS=0
FSIM_CMD="$TG_INSTALL/bin/$FSIM_EXE -L $LOGS_DIR $FSIM_ARGS"

cd $WORKLOAD_INSTALL

[ "$VERBOSE" ] && echo "Running fsim command '$FSIM_CMD'"

if [ -z "$OUTPUT_FILE" ]; then
  #
  # Test fsim's stdout
  #
  $FSIM_CMD 2>&1 | fsim_success_test
  RET=$?
else
  #
  # Test log file instead of stdout
  #
  if [ -z "$VERBOSE" ]; then
    $FSIM_CMD 1>$LOGS_DIR/fsim_out 2>&1
  else
    $FSIM_CMD 2>&1 | tee $LOGS_DIR/fsim_out
  fi
  if [ -f "$LOGS_DIR/$OUTPUT_FILE" ]; then
    cat $LOGS_DIR/$OUTPUT_FILE | fsim_success_test
    RET=$?
  else
    echo "Log file '$LOGS_DIR/$OUTPUT_FILE' doesn't exist" 1>&2
    RET=1
  fi
fi
#
# Kill the timeout timer
#
[ "$TIMEOUT_PID" ] && kill ${TIMEOUT_PID} 2>/dev/null && wait $TIMEOUT_PID 2>/dev/null

[ "$VERBOSE" ] || [ "$TIME_TESTS" ] && echo "Test took $SECONDS sec"

if [ $RET -ne 0 ]; then
  echo " !!! Test $TEST_NAME failed !!!" 1>&2
  echo
fi
#
# guarantee fsim is no longer running
#
kill_fsim

exit $RET
