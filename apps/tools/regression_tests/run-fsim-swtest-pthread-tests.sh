#!/bin/bash

# run-fsim-swtest-pthread-tests.sh
#
# Run all the fsim-swtest pthread tests. Only compatable with TGR.
#
# The tests come from apps/legacy/tg-xe/pthread_*
#
# For usage and environmental variables run with the -h argument.
#

source ./setup-test-env.sh
[ $? -ne 0 ] && exit 1

export FSIM_EXE="fsim-swtest"

# Tests to run
TESTS="\
pthread_simple pthread_simple.p
pthread_detach pthread_detach.p
pthread_malloc pthread_malloc.p
pthread_mutex_recursive pthread_mutex_recursive.p
pthread_cancel pthread_cancel.p"

if [ "$1" == "-h" ]; then
  print_help
fi

# If there are command line parameters, use those instead.
[ $# -ne 0 ] && TESTS=$@

for TEST in $TESTS; do

  export TEST_NAME=$TEST

  declare -a REGEXS=("ready alarm")

  TEST_FILE=$TEST

  # Since this is from legacy, we must add a .swtest if the test isn't PIE
  case "$TEST" in
    *.p) ;;
    *) TEST_FILE+=.swtest ;;
  esac

  export WORKLOAD_INSTALL=$APPS_ROOT/legacy/tg-xe
  export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/ccfg.cfg -- $WORKLOAD_INSTALL/$TEST_FILE"

  # Set up the env for the test
  case $TEST in
    pthread_simple|pthread_simple.p)
      REGEXS+=("Child done" "Child knew who it was" "Child returned 1234" "Global val was 42")
    ;;
    pthread_detach|pthread_detach.p)
      REGEXS+=("making thread 1 (detached)" "making thread 2" "detaching thread 2" "parent done")
    ;;
    pthread_malloc|pthread_malloc.p)
      REGEXS+=("Child1 done" "Child2 done" "Parent done")
    ;;
    pthread_mutex_recursive|pthread_mutex_recursive.p)
      REGEXS+=("TEST: recursive" "main: creating child" "child: recursing" "child: done"
               "main: joined child" "main: locking mutex" "SUCCESS! Value was 42"
               "TEST: error check" "main: creating child" "child: recursing"
               "child: mutex already locked" "child: done" "main: joined child"
               "main: locking mutex" "SUCCESS! Value was 1")
    ;;
    pthread_cancel|pthread_cancel.p)
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

  if [ "$SUCCESS" -eq 0 ]; then
    echo " !!! Test $TEST_NAME failed !!!" 1>&2
  else
    echo "Test $TEST_NAME succeeded"
  fi
  echo
done
