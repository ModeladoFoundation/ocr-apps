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
TESTS="tg_cdemo_nonewlib tg_cdemo tg_cxxdemo tg_throw
legacy_cxxhello legacy_hello legacy_iotest
PIE_fptr_simple PIE_multi_seg"

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
    tg_cdemo_nonewlib)
      export WORKLOAD_INSTALL=$TG_INSTALL/../fsim/swtest
      export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/sw1.cfg -- $WORKLOAD_INSTALL/cdemo_nonewlib"
      REGEXS+=("XE0 stdout: 13" "ce_write to f" "XE0 stdout: end")
      REGEXS+=("XE0 stderr: 13" "ce_write to f" "XE0 stderr: end")
      REGEXS+=("XE0 info: terminate alarm")
    ;;
    tg_cdemo)
      export WORKLOAD_INSTALL=$TG_INSTALL/../fsim/swtest
      export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/sw1.cfg -- $WORKLOAD_INSTALL/cdemo"
      REGEXS+=("XE0 stdout: 13" "Write to fd 1" "XE0 stdout: end")
      REGEXS+=("XE0 stderr: 13" "Write to fd 2" "XE0 stderr: end")
      REGEXS+=("XE0 info: terminate alarm")
    ;;
    tg_cxxdemo)
      export WORKLOAD_INSTALL=$TG_INSTALL/../fsim/swtest
      export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/sw1l2.cfg -- $WORKLOAD_INSTALL/cxxdemo foo bar"
      REGEXS+=("App $WORKLOAD_INSTALL/cxxdemo: 3 args")
    ;;
    tg_throw)
      export WORKLOAD_INSTALL=$TG_INSTALL/../fsim/swtest
      export FSIM_ARGS="-q -c $TG_INSTALL/../fsim/swtest/sw1l2.cfg -- $WORKLOAD_INSTALL/throw"
      REGEXS+=("29" "do_try: Calling middle(4)" "16" "middle e(10)")
      REGEXS+=("52" "struct error(.*) = 10 destructor" "28" "do_try: middle returns 5")
      REGEXS+=("29" "do_try: Calling middle(7)" "16" "middle e(10)")
      REGEXS+=("33" "may_throw: Throwing error = 7" "52" "struct error(.*) = 10 destructor")
      REGEXS+=("36" "Caught throw (error = 7)" "51" "struct error(.*) = 7 destructor")
      REGEXS+=("51" "struct error(.*) = 7 destructor")
    ;;
    legacy_cxxhello)
      export WORKLOAD_INSTALL=$APPS_ROOT/legacy/tg-xe
      export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/c++cfg.cfg -- $WORKLOAD_INSTALL/cxxhello"
      REGEXS+=("CE: XE0 stdout: 13" "Hello World!" "CE: XE0 stdout: end")
      REGEXS+=("CE: XE0 stdout: 17" "cplus output: 20" "CE: XE0 stdout: end")
    ;;
    legacy_hello)
      # NOTE: It is very unlikly, but if this test is run just as the UTS time is turning midnight, it may
      # fail as the day will change as it is running.
      export WORKLOAD_INSTALL=$APPS_ROOT/legacy/tg-xe
      export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/ccfg.cfg -- $WORKLOAD_INSTALL/hello"
      REGEXS+=("CE: XE0 stdout: 13" "Hello World!" "CE: XE0 stdout: end" "time = $(date +'%a %b %-d')")
      REGEXS+=("float = 1\.000000" "ret = 15" "terminate alarm")
    ;;
    legacy_iotest)
      TEST_DIR=/tmp/${USER}_iotest
      rm -rf $TEST_DIR
      mkdir $TEST_DIR
      export WORKLOAD_INSTALL=$APPS_ROOT/legacy/tg-xe
      export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/ccfg.cfg -- $WORKLOAD_INSTALL/iotest $TEST_DIR"
      REGEXS+=("all good." "terminate alarm")
    ;;
    PIE_fptr_simple)
      export WORKLOAD_INSTALL=$TG_INSTALL/../xe-llvm/test/PIE
      export FSIM_ARGS="-q -c $TG_INSTALL/../fsim/swtest/sw1.cfg -- $WORKLOAD_INSTALL/fptr_simple"
      REGEXS+=("v2 is: 6" "PASSED: v2 == 6" "terminate alarm")
    ;;
    PIE_multi_seg)
      export WORKLOAD_INSTALL=$TG_INSTALL/../xe-llvm/test/PIE
      export FSIM_ARGS="-q -c $TG_INSTALL/../fsim/swtest/sw1.cfg -- $WORKLOAD_INSTALL/multi_seg"
      REGEXS+=("\*b is: 42, \*y\[0] is: 42, xint is: 42"
               "\*b is: 42, \*y\[0] is: 7, xint is: 42"
               "\*b is: 7, \*y\[0] is: 7, xint is: 42"
               "\*b is: 42, \*y\[0] is: 7, xint is: 42"
               "\*b is: 43, \*y\[0] is: 7, xint is: 43"
               "\*\*a = 43"
               "\*\*a is: 10, \*y\[0] is: 10"
               "PASSED" "terminate alarm")
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

    legacy_iotest)
      # Check to see if it changed the permissions on the file
      if ! ls -l $TEST_DIR/test_file.txt | grep -q ^----------; then
        echo "Did not actually chmod file!" 1>&2
        SUCCESS=0
      fi
      rm -rf $TEST_DIR
      ;;

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
