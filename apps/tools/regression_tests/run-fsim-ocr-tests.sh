#!/bin/bash

# run-fsim-ocr-tests.sh
#
# Run the Intel tests for fsim: OCR examples, mb-printf and irqstress.
# Only relevent when building with ./Makefile.ocr
#
# Note that OCRcholesky, OCRsmith-waterman and irqstress are not run by
# default. This is because OCRcholesky and OCRsmith-waterman have
# broken makefiles which prevent them from being built by ./Makefile.ocr
# and irqstress runs indefinitally.
#
# For usage and environmental variables run with the -h argument.
#

source ./setup-test-env.sh
[[ $? -ne 0 ]] && exit 1

export FSIM_EXE="fsim"

# Tests to run
TESTS="OCRfib OCRprintf mb-printf"

if [[ $1 == "-h" ]]; then
  print_help
fi

# If there are command line parameters, use those instead.
[[ $# -ne 0 ]] && TESTS=$@

for TEST in $TESTS; do
  export TEST_NAME=$TEST

  declare -a REGEXS=("SUCCESS in Simulation")

  # Set up the env for the test
  case $TEST in
    OCRcholesky)
      export WORKLOAD_INSTALL="$APPS_ROOT/examples/cholesky/install/tg";
      export FSIM_ARGS="-s -c $TG_INSTALL/fsim-configs/localhost.cfg -c $TG_INSTALL/fsim-configs/dvfs-default.cfg -c $WORKLOAD_INSTALL/config.cfg"
      ;;
    OCRfib)
      export WORKLOAD_INSTALL="$APPS_ROOT/examples/fib/install/tg";
      export FSIM_ARGS="-s -c $TG_INSTALL/fsim-configs/localhost.cfg -c $TG_INSTALL/fsim-configs/dvfs-default.cfg -c $WORKLOAD_INSTALL/config.cfg"
      ;;
    OCRprintf)
      export WORKLOAD_INSTALL="$APPS_ROOT/examples/printf/install/tg";
      export FSIM_ARGS="-s -c $TG_INSTALL/fsim-configs/localhost.cfg -c $TG_INSTALL/fsim-configs/dvfs-default.cfg -c $WORKLOAD_INSTALL/config.cfg"
      ;;
    OCRsmith-waterman)
      export WORKLOAD_INSTALL="$APPS_ROOT/examples/smith-waterman/install/tg";
      export FSIM_ARGS="-s -c $TG_INSTALL/fsim-configs/localhost.cfg -c $TG_INSTALL/fsim-configs/dvfs-default.cfg -c $WORKLOAD_INSTALL/config.cfg"
      ;;
    irqstress)
      export WORKLOAD_INSTALL="$TG_INSTALL/workloads/irqstress";
      export FSIM_ARGS="-s -c $TG_INSTALL/fsim-configs/dvfs-default.cfg -c $TG_INSTALL/fsim-configs/localhost.cfg -c $WORKLOAD_INSTALL/irqstress.cfg"
      ;;
    mb-printf)
      export WORKLOAD_INSTALL="$TG_INSTALL/workloads/mb-printf";
      export FSIM_ARGS="-s -c $TG_INSTALL/fsim-configs/dvfs-default.cfg -c $TG_INSTALL/fsim-configs/localhost.cfg -c $WORKLOAD_INSTALL/mb-printf.cfg"
      ;;
    *)
      echo "Invalid test name '$TEST'" 1>&2
      continue
      ;;
  esac

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
