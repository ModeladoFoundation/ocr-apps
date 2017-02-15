#!/bin/bash

# run-fsim-swtest-tests.sh
#
# Run all the single thread fsim-swtest tests. Compatable with OCR and TGR.
#
# The tests come from
#
#   tg/fsim/swtest               - prefixed with tg_
#   tg/xe-llvm/test/PIE          - prefixed with PIE_
#   tg/xe-llvm/test/CodeGen/XSTG - prefixed with TCO_
#   apps/legacy/tg-xe            - prefixed with legacy_
#
# For usage and environmental variables run with the -h argument.
#

source ./setup-test-env.sh
[ $? -ne 0 ] && exit 1

export FSIM_EXE="fsim-swtest"


# Tests to run
TESTS="\
tg_cdemo tg_cdemo.p tg_cdemo_nonewlib tg_cdemo_nonewlib.p
tg_cxxdemo tg_cxxdemo.p tg_throw tg_throw.p
legacy_cxxhello legacy_cxxhello.p
legacy_hello legacy_hello.p legacy_iotest legacy_iotest.p
legacy_float_tests legacy_float_tests.p
legacy_double_tests legacy_double_tests.p
legacy_float_complex_tests legacy_float_complex_tests.p
legacy_double_complex_tests legacy_double_complex_tests.p
PIE_fptr_simple PIE_fptr_simple.p PIE_multi_seg PIE_multi_seg.p
TCO_tailcall1 TCO_tailcall1.p"

if [ "$1" == "-h" ]; then
  print_help
fi

# If there are command line parameters, use those instead.
[ $# -ne 0 ] && TESTS=$@

for TEST in $TESTS; do

  export TEST_NAME=$TEST

  declare -a REGEXS=("ready alarm")

  # Remove the location prefix (eg. tg_) to get the test filename.
  TEST_FILE=${TEST#*_}

  # Non-pie programs need a .swtest extension
  case "$TEST" in
    *.p) ;;
    *) TEST_FILE+=.swtest ;;
  esac

  # Set up the env for the test
  case $TEST in
    tg_cdemo_nonewlib|tg_cdemo_nonewlib.p)
      export WORKLOAD_INSTALL=$TG_INSTALL/../fsim/swtest
      export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/sw1.cfg -- $WORKLOAD_INSTALL/$TEST_FILE"
      REGEXS+=("XE0 stdout: 16" "ce_write to fd 1" "XE0 stdout: end")
      REGEXS+=("XE0 stderr: 16" "ce_write to fd 2" "XE0 stderr: end")
      REGEXS+=("XE0 info: terminate alarm")
    ;;
    tg_cdemo|tg_cdemo.p)
      export WORKLOAD_INSTALL=$TG_INSTALL/../fsim/swtest
      export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/sw1.cfg -- $WORKLOAD_INSTALL/$TEST_FILE"
      REGEXS+=("XE0 stdout: 13" "Write to fd 1" "XE0 stdout: end")
      REGEXS+=("XE0 stderr: 13" "Write to fd 2" "XE0 stderr: end")
      REGEXS+=("XE0 info: terminate alarm")
    ;;
    tg_cxxdemo|tg_cxxdemo.p)
      export WORKLOAD_INSTALL=$TG_INSTALL/../fsim/swtest
      export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/sw1l2.cfg -- $WORKLOAD_INSTALL/$TEST_FILE foo bar"
      REGEXS+=("App $WORKLOAD_INSTALL/$TEST_FILE: 3 args")
    ;;
    tg_throw|tg_throw.p)
      export WORKLOAD_INSTALL=$TG_INSTALL/../fsim/swtest
      export FSIM_ARGS="-q -c $TG_INSTALL/../fsim/swtest/sw1l2.cfg -- $WORKLOAD_INSTALL/$TEST_FILE"
      REGEXS+=("29" "do_try: Calling middle(4)" "16" "middle e(10)")
      REGEXS+=("52" "struct error(.*) = 10 destructor" "28" "do_try: middle returns 5")
      REGEXS+=("29" "do_try: Calling middle(7)" "16" "middle e(10)")
      REGEXS+=("33" "may_throw: Throwing error = 7" "52" "struct error(.*) = 10 destructor")
      REGEXS+=("36" "Caught throw (error = 7)" "51" "struct error(.*) = 7 destructor")
      REGEXS+=("51" "struct error(.*) = 7 destructor")
    ;;
    legacy_cxxhello|legacy_cxxhello.p)
      export WORKLOAD_INSTALL=$APPS_ROOT/legacy/tg-xe
      export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/c++cfg.cfg -- $WORKLOAD_INSTALL/$TEST_FILE"
      REGEXS+=("CE: XE0 stdout: 13" "Hello World!" "CE: XE0 stdout: end")
      REGEXS+=("CE: XE0 stdout: 17" "cplus output: 20" "CE: XE0 stdout: end")
    ;;
    legacy_hello|legacy_hello.p)
      # NOTE: It is very unlikly, but if this test is run just as the UTS time is turning midnight, it may
      # fail as the day will change as it is running.
      export WORKLOAD_INSTALL=$APPS_ROOT/legacy/tg-xe
      export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/ccfg.cfg -- $WORKLOAD_INSTALL/$TEST_FILE"
      REGEXS+=("CE: XE0 stdout: 13" "Hello World!" "CE: XE0 stdout: end" "time = $(date -u +'%a %b %-d %H:%M:.. %Y')")
      REGEXS+=("float = 1\.000000" "ret = 15" "terminate alarm")
    ;;
    legacy_iotest|legacy_iotest.p)
      TEST_DIR=/tmp/${USER}_iotest
      rm -rf $TEST_DIR
      mkdir $TEST_DIR
      export WORKLOAD_INSTALL=$APPS_ROOT/legacy/tg-xe
      export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/ccfg.cfg -- $WORKLOAD_INSTALL/$TEST_FILE $TEST_DIR"
      REGEXS+=("all good." "terminate alarm")
    ;;
    legacy_float_tests|legacy_float_tests.p)
      export WORKLOAD_INSTALL=$APPS_ROOT/legacy/tg-xe
      export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/ccfg.cfg -- $WORKLOAD_INSTALL/$TEST_FILE"
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
      export WORKLOAD_INSTALL=$APPS_ROOT/legacy/tg-xe
      export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/ccfg.cfg -- $WORKLOAD_INSTALL/$TEST_FILE"
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
      export WORKLOAD_INSTALL=$APPS_ROOT/legacy/tg-xe
      export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/ccfg.cfg -- $WORKLOAD_INSTALL/$TEST_FILE"
      REGEXS+=("float complex ccosf(0.00000000 0.00000000) = 1.00000000 -0.00000000")
      REGEXS+=("float complex cexpf(0.00000000 0.00000000) = 1.00000000 0.00000000")
      REGEXS+=("float complex ccosf(0.00000000 3.14159274) = 11.59195423 -0.00000000")
      REGEXS+=("float complex ccosf(3.14159274 0.00000000) = -1.00000000 0.00000000")
      REGEXS+=("float complex conjf(3.14159274 0.00000000) = 3.14159274 -0.00000000")
      REGEXS+=("float complex conjf(nan nan) = nan nan")
    ;;
    legacy_double_complex_tests|legacy_double_complex_tests.p)
      export WORKLOAD_INSTALL=$APPS_ROOT/legacy/tg-xe
      export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/ccfg.cfg -- $WORKLOAD_INSTALL/$TEST_FILE"
      REGEXS+=("double complex ccosf(0.0000000000000000 0.0000000000000000) = 1.0000000000000000 -0.0000000000000000")
      REGEXS+=("double complex cexpf(0.0000000000000000 0.0000000000000000) = 1.0000000000000000 0.0000000000000000")
      REGEXS+=("double complex ccosf(0.0000000000000000 3.1415926535897931) = 11.5919542312622070 -0.0000000000000000")
      REGEXS+=("double complex ccosf(3.1415926535897931 0.0000000000000000) = -1.0000000000000000 0.0000000000000000")
      REGEXS+=("double complex conjf(3.1415926535897931 0.0000000000000000) = 3.1415927410125732 -0.0000000000000000")
      REGEXS+=("double complex conjf(nan nan) = nan nan")
    ;;
    PIE_fptr_simple|PIE_fptr_simple.p)
      export WORKLOAD_INSTALL=$TG_INSTALL/../xe-llvm/test/PIE
      export FSIM_ARGS="-q -c $TG_INSTALL/../fsim/swtest/sw1.cfg -- $WORKLOAD_INSTALL/$TEST_FILE"
      REGEXS+=("v2 is: 6" "PASSED: v2 == 6" "terminate alarm")
    ;;
    PIE_multi_seg|PIE_multi_seg.p)
      export WORKLOAD_INSTALL=$TG_INSTALL/../xe-llvm/test/PIE
      export FSIM_ARGS="-q -c $TG_INSTALL/../fsim/swtest/sw1.cfg -- $WORKLOAD_INSTALL/$TEST_FILE"
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
       export FSIM_ARGS="-q -c $WORKLOAD_INSTALL/../swtest.cfg -- $WORKLOAD_INSTALL/$TEST_FILE"
       REGEXS+=("tc_func1 - SUCCESS")
       REGEXS+=("tc_vfunc1 - SUCCESS")
       REGEXS+=("tc_vfunc2 - SUCCESS")
       REGEXS+=("tc_fn_tc - SUCCESS")
       REGEXS+=("XE0 info: terminate alarm")
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

    legacy_iotest|legacy_iotest.p)
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

  if [ "$SUCCESS" -eq 0 ]; then
    echo " !!! Test $TEST_NAME failed !!!" 1>&2
  else
    echo "Test $TEST_NAME succeeded"
  fi
  echo
done
