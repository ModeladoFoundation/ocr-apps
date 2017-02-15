#!/bin/bash

# run-fsim-swtest-pthread-tests.sh
#
# Run all the fsim-swtest tgr-only tests. Only compatable with TGR.
#
# The tests come from
#   apps/legacy/tg-xe/pthread_*     - prefixed with pthread_
#   apps/libs/src/libtgr/tg-ce/test - prefixed with c_test_
#
# For usage and environmental variables run with the -h argument.
#

source ./setup-test-env.sh
[ $? -ne 0 ] && exit 1

export FSIM_EXE="fsim-swtest"

# Tests to run
TESTS="\
c_test_1 c_test_1.p c_test_2 c_test_2.p c_test_3 c_test_3.p
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

  # We must add a .swtest if the test isn't PIE
  case "$TEST" in
    *.p) ;;
    *) TEST_FILE+=.swtest ;;
  esac

  # Set up the env for the test
  case $TEST in
    c_test_1|c_test_1.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/libs/src/libtgr/tg-ce/test"
      REGEXS+=("c_test_1 starting"
               "tgr_mmap succeeded"
               "OK_1"
               "OK_2"
               "argc = 1")
      ;;
    c_test_2|c_test_2.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/libs/src/libtgr/tg-ce/test"
      # Make sure that the testing environment from last time was cleaned up
      rm -rf $WORKLOAD_INSTALL/ce-os-subdir
      # NOTE: clean_path comes from setup-test-env.sh
      REGEXS+=("cwd = $(clean_path $WORKLOAD_INSTALL)"
               "cwd = $(clean_path $WORKLOAD_INSTALL)/ce-os-subdir"
               "opened ce-os-test"
               "write testdata to ce-os-test"
               "write succeeded"
               "closed fd"
               "stat matches"
               "re-opened ce-os-test"
               "data compared equal"
               "re-closed fd"
               "test successful")
      ;;
    c_test_3|c_test_3.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/libs/src/libtgr/tg-ce/test"
      REGEXS+=("out: Clone XE 0x[0-9a-f]\+: 0x[0-9a-f]\+"
               "out: In clone: Arg to clone"
               "CE-OS-TEST: clone test successful"
               "Clone XE 0x[0-9a-f]\+: 0x[0-9a-f]\+"
               "In clone: Arg to clone"
               "In clone: Arg to clone"
               "In clone: Arg to clone"
               "waitall test successful"
               "err: XE with id 0x[0-9a-f]\+ is detached\."
               "err: XE with id 0x[0-9a-f]\+ is detached\."
               "CE-OS-TEST: detach test successful")
      ;;
    pthread_simple|pthread_simple.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/legacy/tg-xe"
      REGEXS+=("Child done" "Child knew who it was" "Child returned 1234" "Global val was 42")
    ;;
    pthread_detach|pthread_detach.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/legacy/tg-xe"
      REGEXS+=("making thread 1 (detached)" "making thread 2" "detaching thread 2" "parent done")
    ;;
    pthread_malloc|pthread_malloc.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/legacy/tg-xe"
      REGEXS+=("Child1 done" "Child2 done" "Parent done")
    ;;
    pthread_mutex_recursive|pthread_mutex_recursive.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/legacy/tg-xe"
      REGEXS+=("TEST: recursive" "main: creating child" "child: recursing" "child: done"
               "main: joined child" "main: locking mutex" "SUCCESS! Value was 42"
               "TEST: error check" "main: creating child" "child: recursing"
               "child: mutex already locked" "child: done" "main: joined child"
               "main: locking mutex" "SUCCESS! Value was 1")
    ;;
    pthread_cancel|pthread_cancel.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/legacy/tg-xe"
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

  export FSIM_ARGS="-q -c $APPS_ROOT/legacy/tg-xe/ccfg.cfg -- $WORKLOAD_INSTALL/$TEST_FILE"

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
