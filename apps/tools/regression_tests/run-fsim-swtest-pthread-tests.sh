#!/bin/bash

# This script may accept command line parameters of which test(s) to run.
# Possible tests:
#  tg_cdemo_nonewlib tg_cdemo tg_cxxdemo tg_throw
#  legacy_cxxhello legacy_hello legacy_iotest
#  pthread_simple pthread_detach pthread_mutex_recursive pthread_cancel
#  PIE_fptr_simple PIE_multi_seg
#
# Defaults to running all tests
#
# This script may optionally use env vars:
#   TG_INSTALL - The install directory of the tg repo
#   LOGS_DIR   - The directory for fsim to place its logs
#   VERBOSE    - If set, then write all of fsim's output to stdout

source ./setup-test-env.sh
[[ $? -ne 0 ]] && exit 1

export FSIM_EXE="fsim-swtest"

# Tests to run
TESTS="pthread_simple pthread_detach pthread_mutex_recursive pthread_cancel"

if [[ $1 == "-h" ]]; then
  echo -e "You may specify one or more of:\n\n$TESTS\n\nDefaults to all tests"
  exit
fi

# If there are command line parameters, use those instead.
[[ $# -ne 0 ]] && TESTS=$@

for TEST in $TESTS; do

  export TEST_NAME=$TEST

  declare -a REGEXS=("ready alarm")

  # Set up the env for the test
  case $TEST in
    pthread_simple)
      export WORKLOAD_INSTALL=$APPS_ROOT/legacy/tg-xe
      export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/ccfg.cfg -- $WORKLOAD_INSTALL/pthread_simple"
      REGEXS+=("Child done" "Child knew who it was" "Child returned 1234" "Global val was 42")
    ;;
    pthread_detach)
      export WORKLOAD_INSTALL=$APPS_ROOT/legacy/tg-xe
      export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/ccfg.cfg -- $WORKLOAD_INSTALL/pthread_detach"
      REGEXS+=("making thread 1 (detached)" "making thread 2" "detaching thread 2" "parent done")
    ;;
    pthread_mutex_recursive)
      export WORKLOAD_INSTALL=$APPS_ROOT/legacy/tg-xe
      export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/ccfg.cfg -- $WORKLOAD_INSTALL/pthread_mutex_recursive"
      REGEXS+=("TEST: recursive" "main: creating child" "child: recursing" "child: done"
               "main: joined child" "main: locking mutex" "SUCCESS! Value was 42"
               "TEST: error check" "main: creating child" "child: recursing"
               "child: mutex already locked" "child: done" "main: joined child"
               "main: locking mutex" "SUCCESS! Value was 1")
    ;;
    pthread_cancel)
      export WORKLOAD_INSTALL=$APPS_ROOT/legacy/tg-xe
      export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/ccfg.cfg -- $WORKLOAD_INSTALL/pthread_cancel"
      REGEXS+=("testing Enable Deferred" "joined thread"
               "testing Disable Deferred" "testing cancel" "enabling cancel" "joined thread"
               "testing Enable Async" "joined thread"
               "testing Disable Async" "testing cancel" "enabling cancel" "joined thread")
    ;;
    *)
      echo "Invalid test name '$TEST'" 1>&2
      continue
      ;;
  esac

  REGEXS+=("Client shutdown has been approved")

  # Execute the test, moving on if unsuccessful
  ./exec-fsim-test.sh "${REGEXS[@]}" || continue

  # Check to see if the test was successful & cleanup
  SUCCESS=1
  case $TEST in
    *) #If there is no extra test/cleanup, assume success
      ;;
  esac

  if [[ $SUCCESS -eq 0 ]]; then
    echo " !!! Test $TEST_NAME failed !!!" 1>&2
  else
    echo "Test $TEST_NAME succeeded"
  fi
  echo
done
