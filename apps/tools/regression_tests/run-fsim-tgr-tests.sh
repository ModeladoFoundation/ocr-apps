#!/bin/bash

# run-fsim-tgr-tests.sh
#
# Run all the fsim with tgkrnl-tgr tests. Only compatable with TGR.
#
# The tests come from
#
#   apps/libs/src/libtgr/tg-ce/test - prefixed with c_test_1
#   apps/legacy/tg-xe               - prefixed with legacy_ or pthread_
#
# For usage and environmental variables run with the -h argument.
#

source ./setup-test-env.sh
[ $? -ne 0 ] && exit 1

export FSIM_EXE="fsim"

# Tests to run
TESTS="\
c_test_1 c_test_1.p c_test_2 c_test_2.p c_test_3 c_test_3.p
legacy_hello legacy_hello.p
legacy_cxxhello legacy_cxxhello.p
legacy_float_tests legacy_float_tests.p
legacy_double_tests legacy_double_tests.p
tg_cdemo tg_cdemo.p
tg_cdemo_nonewlib tg_cdemo_nonewlib.p
tg_cxxdemo tg_cxxdemo.p
tg_throw tg_throw.p
pthread_simple pthread_simple.p
pthread_detach pthread_detach.p
pthread_malloc pthread_malloc.p
pthread_mutex_recursive pthread_mutex_recursive.p
pthread_cancel pthread_cancel.p
PIE_fptr_simple PIE_fptr_simple.p
PIE_multi_seg PIE_multi_seg.p
TCO_tailcall1 TCO_tailcall1.p"

# Note: the tests legacy_iotest legacy_iotest.p have been disabled for now since
# they are not supported by tgkrnl-tgr yet. (They require fsim tgkrnl-tgr to
# pass command line arguments to main argv)

if [ "$1" == "-h" ]; then
  print_help
fi

# If there are command line parameters, use those instead.
[ $# -ne 0 ] && TESTS=$@

for TEST in $TESTS; do
  export TEST_NAME=$TEST

  TEST_FILE=$TEST

  # If a test is not a pthread_* or c_test_* then its prefix needs to be removed
  case "$TEST" in
    pthread_*) ;;
    c_test_*) ;;
    *)TEST_FILE=${TEST#*_} ;;
  esac

  #If it is non-PIE and not a c_test_*, then it needs a .fsim extension
  case "$TEST" in
    *.p) ;;
    c_test_*) ;;
    *) TEST_FILE+=.fsim ;;
  esac

  # We examine output from the CE log file.
  export OUTPUT_FILE="$TEST_FILE.rck000.cub00.skt0.cls000.blk00.CE.00"

  # We need to export this for the fsim.cfg file.
  export TGKRNL="$APPS_ROOT/libs/install/tg-xe/lib/tgkrnl-tgr"

  declare -a REGEXS=(
    "TGR-MAIN: starting"
    "TGR-MAIN: initialized subsystems"
    "TGR-MAIN: initialized datastructures"
    "TGR-ELF: - Valid XE ELF64 header \((PIE format) \)\?found"
    "TGR-RUN: Starting XE 0x1 at"
    "TGR-ALARMS: XE1 ready alarm")

  # Set up the env for the test
  case $TEST in
    c_test_1|c_test_1.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/libs/src/libtgr/tg-ce/test"
      export FSIM_ARGS="-s -c $WORKLOAD_INSTALL/fsim.cfg"
      REGEXS+=("TGR-CONSOLE: (XE1): c_test_1 starting"
               "TGR-CONSOLE: (XE1): tgr_mmap succeeded"
               "TGR-CONSOLE: (XE1): OK_1"
               "TGR-CONSOLE: (XE1): OK_2"
               "CE-OS: argc = 1"
               "TGR-ALARMS: XE 0x1 terminate alarm, terminating xe")
      ;;
    c_test_2|c_test_2.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/libs/src/libtgr/tg-ce/test"
      export FSIM_ARGS="-s -c $WORKLOAD_INSTALL/fsim.cfg"
      # Make sure that the testing environment from last time was cleaned up
      rm -rf $WORKLOAD_INSTALL/ce-os-subdir
      # NOTE: clean_path comes from setup-test-env.sh
      REGEXS+=("TGR-CONSOLE: (XE1): CE-OS-TEST: cwd = $(clean_path $WORKLOAD_INSTALL)"
               "TGR-CONSOLE: (XE1): CE-OS-TEST: cwd = $(clean_path $WORKLOAD_INSTALL)/ce-os-subdir"
               "TGR-CONSOLE: (XE1): CE-OS-TEST: opened ce-os-test"
               "TGR-CONSOLE: (XE1): CE-OS-TEST: write testdata to ce-os-test"
               "TGR-CONSOLE: (XE1): CE-OS-TEST: write succeeded"
               "TGR-CONSOLE: (XE1): CE-OS-TEST: closed fd"
               "TGR-CONSOLE: (XE1): CE-OS-TEST: stat matches"
               "TGR-CONSOLE: (XE1): CE-OS-TEST: re-opened ce-os-test"
               "TGR-CONSOLE: (XE1): CE-OS-TEST: data compared equal"
               "TGR-CONSOLE: (XE1): CE-OS-TEST: re-closed fd"
               "TGR-CONSOLE: (XE1): CE-OS-TEST: test successful"
               "TGR-ALARMS: XE 0x1 terminate alarm, terminating xe")
      ;;
    c_test_3|c_test_3.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/libs/src/libtgr/tg-ce/test"
      export FSIM_ARGS="-s -c $WORKLOAD_INSTALL/fsim.cfg"
      REGEXS+=("TGR-MSG: clone entry"
               "TGR-MSG: XE 0x1: Cloned XE 0x2, entry 0x[0-9a-f]\+, stack top 0x[0-9a-f]\+"
               "TGR-CONSOLE: (XE2): Clone XE 0x2: 0x[0-9a-f]\+"
               "TGR-CONSOLE: (XE2): In clone: Arg to clone"
               "TGR-CONSOLE: (XE1): CE-OS-TEST: clone test successful"
               "TGR-MSG: clone entry"
               "TGR-MSG: XE 0x1: Cloned XE 0x2, entry 0x[0-9a-f]\+, stack top 0x[0-9a-f]\+"
               "TGR-CONSOLE: (XE\([234]\)): Clone XE 0x\1: 0x[0-9a-f]\+"
               "TGR-CONSOLE: (XE\([234]\)): In clone: Arg to clone"
               "TGR-CONSOLE: (XE\([234]\)): In clone: Arg to clone"
               "TGR-CONSOLE: (XE\([234]\)): In clone: Arg to clone"
               "TGR-CONSOLE: (XE1): CE-OS-TEST: waitall test successful"
               "TGR-MSG: clone entry"
               "TGR-MSG: XE 0x1: Cloned XE 0x2, entry 0x[0-9a-f]\+, stack top 0x[0-9a-f]\+"
               "TGR-ERROR: MSG - XE 0x1 waitpid err: XE 0x2 is detached\."
               "TGR-ERROR: MSG - XE 0x1 waitpid err: XE 0x2 is detached\."
               "TGR-ERROR: MSG - XE 0x1 waitpid err: XE 0x2 is unused\."
               "TGR-CONSOLE: (XE1): CE-OS-TEST: detach test successful"
               "TGR-ALARMS: XE 0x1 terminate alarm, terminating xe")
      ;;
    legacy_hello|legacy_hello.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/legacy/tg-xe"
      export FSIM_ARGS="-s -c $WORKLOAD_INSTALL/fsim.cfg"
      # NOTE: It is very unlikly, but if this test is run just as the UTS time is
      # turning midnight, it may fail as the day will change as it is running.
      REGEXS+=("Hello World!" "time = $(date -u +'%a %b %-d ..:..:.. %Y')")
      REGEXS+=("float = 1\.000000" "ret = 15" "terminate alarm")
      ;;
    legacy_cxxhello|legacy_cxxhello.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/legacy/tg-xe"
      export FSIM_ARGS="-s -c $WORKLOAD_INSTALL/fsim.cfg"
      REGEXS+=("Hello World!" "cplus output: 20")
      ;;
    legacy_iotest|legacy_iotest.p)
      TEST_DIR=/tmp/${USER}_iotest
      rm -rf $TEST_DIR
      mkdir $TEST_DIR
      export WORKLOAD_INSTALL=$APPS_ROOT/legacy/tg-xe
      export TEST_ARGS="$TEST_DIR"
      export FSIM_ARGS="-s -c $WORKLOAD_INSTALL/fsim.cfg"
      REGEXS+=("all good." "terminate alarm")
    ;;
    legacy_float_tests|legacy_float_tests.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/legacy/tg-xe"
      export FSIM_ARGS="-s -c $WORKLOAD_INSTALL/fsim.cfg"
      REGEXS+=("float sqrtf(2.00000000) = 1.41421354")
      REGEXS+=("float cosf(0.00000000) = 1.00000000")
      REGEXS+=("float expf(0.00000000) = 1.00000000")
      REGEXS+=("float cosf(0.78539819) = 0.70710677")
      REGEXS+=("float cosf(1.57079637) = -0.00000004")
      REGEXS+=("float sincosf(1.57079637) = 1.00000000 -0.00000004")
      REGEXS+=("float ceilf(1.57079637) = 2.00000000")
      REGEXS+=("float floorf(1.57079637) = 1.00000000")
      REGEXS+=("float fabsf(-1.00000000) = 1.00000000")
      REGEXS+=("float frexpf(2560.00000000) = 0.62500000 12")
      REGEXS+=("float modff(3.14159274) = 0.14159274 3.00000000")
      REGEXS+=("float remquof(5.00000000,2.00000000) = 1.00000000 2")
      REGEXS+=("float fmaf(5.00000000,2.00000000,3.00000000) = 13.00000000")
      REGEXS+=("float scalbnf(5.00000000,2) = 20.00000000")
      REGEXS+=("float cosf(nan) = nan")
      REGEXS+=("float ceilf(nan) = nan")
      REGEXS+=("float floorf(nan) = nan")
      REGEXS+=("float fabsf(nan) = nan")
      REGEXS+=("float frexpf(nan) = nan 0")
      REGEXS+=("float modff(nan) = nan nan")
      REGEXS+=("float remquof(nan,nan) = nan 0")
      REGEXS+=("float fmaf(nan,nan,nan) = nan")
      REGEXS+=("float scalbnf(nan,0) = nan")
      REGEXS+=("float atanf(inf) = 1.57079637")
    ;;
    legacy_double_tests|legacy_double_tests.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/legacy/tg-xe"
      export FSIM_ARGS="-s -c $WORKLOAD_INSTALL/fsim.cfg"
      REGEXS+=("double sqrt(2.0000000000000000) = 1.4142135623730951")
      REGEXS+=("double cos(0.0000000000000000) = 1.0000000000000000")
      REGEXS+=("double exp(0.0000000000000000) = 1.0000000000000000")
      REGEXS+=("double cos(0.7853981633974483) = 0.7071067811865476")
      REGEXS+=("double cos(1.5707963267948966) = 0.0000000000000001")
      REGEXS+=("double sincos(1.5707963267948966) = 1.0000000000000000 0.0000000000000001")
      REGEXS+=("double ceil(1.5707963267948966) = 2.0000000000000000")
      REGEXS+=("double floor(1.5707963267948966) = 1.0000000000000000")
      REGEXS+=("double fabs(-1.0000000000000000) = 1.0000000000000000")
      REGEXS+=("double frexp(2560.0000000000000000) = 0.6250000000000000 12")
      REGEXS+=("double modf(3.1415926535897931) = 0.1415926535897931 3.0000000000000000")
      REGEXS+=("double remquo(5.0000000000000000,2.0000000000000000) = 1.0000000000000000 2")
      REGEXS+=("double fma(5.0000000000000000,2.0000000000000000,3.0000000000000000) = 13.0000000000000000")
      REGEXS+=("double scalbnf(5.0000000000000000,2) = 20.0000000000000000")
      REGEXS+=("double cos(nan) = nan")
      REGEXS+=("double ceil(nan) = nan")
      REGEXS+=("double floor(nan) = nan")
      REGEXS+=("double fabs(nan) = nan")
      REGEXS+=("double frexp(nan) = nan 0")
      REGEXS+=("double modf(nan) = nan nan")
      REGEXS+=("double remquo(nan,nan) = nan 0")
      REGEXS+=("double fma(nan,nan,nan) = nan")
      REGEXS+=("double scalbn(nan,0) = nan")
      REGEXS+=("double atan(inf) = 1.5707963267948966")
    ;;
    legacy_float_complex_tests|legacy_float_complex_tests.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/legacy/tg-xe"
      export FSIM_ARGS="-s -c $WORKLOAD_INSTALL/fsim.cfg"
      REGEXS+=("float complex ccosf(0.00000000 0.00000000) = 1.00000000 -0.00000000")
      REGEXS+=("float complex cexpf(0.00000000 0.00000000) = 1.00000000 0.00000000")
      REGEXS+=("float complex ccosf(0.00000000 3.14159274) = 11.59195518 -0.00000000")
      REGEXS+=("float complex ccosf(3.14159274 0.00000000) = -1.00000000 0.00000000")
      REGEXS+=("float complex conjf(3.14159274 0.00000000) = 3.14159274 0.00000000")
      REGEXS+=("float complex conjf(nan nan) = nan nan")
    ;;
    legacy_double_complex_tests|legacy_double_complex_tests.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/legacy/tg-xe"
      export FSIM_ARGS="-s -c $WORKLOAD_INSTALL/fsim.cfg"
      REGEXS+=("double complex ccosf(0.0000000000000000 0.0000000000000000) = 1.0000000000000000 -0.0000000000000000")
      REGEXS+=("double complex cexpf(0.0000000000000000 0.0000000000000000) = 1.0000000000000000 0.0000000000000000")
      REGEXS+=("double complex ccosf(0.0000000000000000 3.1415926535897931) = 11.5919551849365234 -0.0000000000000000")
      REGEXS+=("double complex ccosf(3.1415926535897931 0.0000000000000000) = -1.0000000000000000 0.0000000000000000")
      REGEXS+=("double complex conjf(3.1415926535897931 0.0000000000000000) = 3.1415927410125732 -0.0000000000000000")
      REGEXS+=("double complex conjf(nan nan) = nan nan")
    ;;
    tg_cdemo_nonewlib|tg_cdemo_nonewlib.p)
      export WORKLOAD_INSTALL="$TG_INSTALL/../fsim/swtest"
      export FSIM_ARGS="-s -c $APPS_ROOT/legacy/tg-xe/fsim.cfg"
      REGEXS+=("ce_write to fd 1")
      REGEXS+=("ce_write to fd 2")
    ;;
    tg_cdemo|tg_cdemo.p)
      export WORKLOAD_INSTALL="$TG_INSTALL/../fsim/swtest"
      export FSIM_ARGS="-s -c $APPS_ROOT/legacy/tg-xe/fsim.cfg"
      REGEXS+=("Write to fd 1")
      REGEXS+=("Write to fd 2")
    ;;
    tg_cxxdemo|tg_cxxdemo.p)
      export WORKLOAD_INSTALL="$TG_INSTALL/../fsim/swtest"
      export FSIM_ARGS="-s -c $APPS_ROOT/legacy/tg-xe/fsim.cfg"
      export TEST_ARGS="foo bar"
      REGEXS+=("App $WORKLOAD_INSTALL/$TEST_FILE: 3 args")
    ;;
    tg_throw|tg_throw.p)
      export WORKLOAD_INSTALL="$TG_INSTALL/../fsim/swtest"
      export FSIM_ARGS="-s -c $APPS_ROOT/legacy/tg-xe/fsim.cfg"
      REGEXS+=("29" "do_try: Calling middle(4)" "16" "middle e(10)")
      REGEXS+=("52" "struct error(.*) = 10 destructor" "28" "do_try: middle returns 5")
      REGEXS+=("29" "do_try: Calling middle(7)" "16" "middle e(10)")
      REGEXS+=("33" "may_throw: Throwing error = 7" "52" "struct error(.*) = 10 destructor")
      REGEXS+=("36" "Caught throw (error = 7)" "51" "struct error(.*) = 7 destructor")
      REGEXS+=("51" "struct error(.*) = 7 destructor")
    ;;
    pthread_simple|pthread_simple.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/legacy/tg-xe"
      export FSIM_ARGS="-s -c $WORKLOAD_INSTALL/fsim.cfg"
      REGEXS+=("Child done" "Child knew who it was" "Child returned 1234" "Global val was 42")
    ;;
    pthread_detach|pthread_detach.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/legacy/tg-xe"
      export FSIM_ARGS="-s -c $WORKLOAD_INSTALL/fsim.cfg"
      REGEXS+=("making thread 1 (detached)" "making thread 2" "detaching thread 2" "parent done")
    ;;
    pthread_malloc|pthread_malloc.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/legacy/tg-xe"
      export FSIM_ARGS="-s -c $WORKLOAD_INSTALL/fsim.cfg"
      REGEXS+=("Child1 done" "Child2 done" "Parent done")
    ;;
    pthread_mutex_recursive|pthread_mutex_recursive.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/legacy/tg-xe"
      export FSIM_ARGS="-s -c $WORKLOAD_INSTALL/fsim.cfg"
      REGEXS+=("TEST: recursive" "main: creating child" "child: recursing" "child: done"
               "main: joined child" "main: locking mutex" "SUCCESS! Value was 42"
               "TEST: error check" "main: creating child" "child: recursing"
               "child: mutex already locked" "child: done" "main: joined child"
               "main: locking mutex" "SUCCESS! Value was 1")
    ;;
    pthread_cancel|pthread_cancel.p)
      export WORKLOAD_INSTALL="$APPS_ROOT/legacy/tg-xe"
      export FSIM_ARGS="-s -c $WORKLOAD_INSTALL/fsim.cfg"
      REGEXS+=("testing Enable Deferred" "joined thread"
               "testing Disable Deferred" "testing cancel" "enabling cancel" "joined thread"
               "testing Enable Async" "joined thread"
               "testing Disable Async" "testing cancel" "enabling cancel" "joined thread")
    ;;
    PIE_fptr_simple|PIE_fptr_simple.p)
      export WORKLOAD_INSTALL=$TG_INSTALL/../xe-llvm/test/PIE
      export FSIM_ARGS="-s -c $APPS_ROOT/legacy/tg-xe/fsim.cfg"
      REGEXS+=("v2 is: 6" "PASSED: v2 == 6" "terminate alarm")
    ;;
    PIE_multi_seg|PIE_multi_seg.p)
      export WORKLOAD_INSTALL=$TG_INSTALL/../xe-llvm/test/PIE
      export FSIM_ARGS="-s -c $APPS_ROOT/legacy/tg-xe/fsim.cfg"
      REGEXS+=("\*b is: 42, \*y\[0] is: 42, xint is: 42"
               "\*b is: 42, \*y\[0] is: 7, xint is: 42"
               "\*b is: 7, \*y\[0] is: 7, xint is: 42"
               "\*b is: 42, \*y\[0] is: 7, xint is: 42"
               "\*b is: 43, \*y\[0] is: 7, xint is: 43"
               "\*\*a = 43"
               "\*\*a is: 10, \*y\[0] is: 10"
               "PASSED" "terminate alarm")
    ;;
    TCO_tailcall1|TCO_tailcall1.p)
      export WORKLOAD_INSTALL=$TG_INSTALL/../xe-llvm/test/CodeGen/XSTG/tailcall
      export FSIM_ARGS="-s -c $APPS_ROOT/legacy/tg-xe/fsim.cfg"
      REGEXS+=("tc_func1 - SUCCESS")
      REGEXS+=("tc_vfunc1 - SUCCESS")
      REGEXS+=("tc_vfunc2 - SUCCESS")
      REGEXS+=("tc_fn_tc - SUCCESS")
    ;;
    *)
      echo "Invalid test name '$TEST'" 1>&2
      continue
    ;;
  esac

  REGEXS+=("TGR-MAIN: all blocks done"
           "TG Kernel Terminating\.\.\.")

  # Execute the test, moving on if unsuccessful
  TEST=$TEST_FILE ./exec-fsim-test.sh "${REGEXS[@]}" || continue

  SUCCESS=1
  case $TEST in

    legacy_iotest|legacy_iotest.p)
      unset TEST_ARGS
      # Check to see if it changed the permissions on the file
      if ! ls -l $TEST_DIR/test_file.txt | grep -q ^----------; then
        echo "Did not actually chmod file!" 1>&2
        SUCCESS=0
      fi
      rm -rf $TEST_DIR
      ;;

    tg_cxxdemo|tg_cxxdemo.p)
      unset TEST_ARGS
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
