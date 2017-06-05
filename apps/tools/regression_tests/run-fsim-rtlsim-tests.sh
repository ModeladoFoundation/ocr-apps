#!/bin/bash

# run-fsim-rtlsim-tests.sh
#
# Run the simple rtlsim tests. Compatable with OCR and TGR.
#
# These test fsim-rtlsim with 1, 2 and 8 XE cores.
#
# The tests come from tg/fsim/tests/rtl-asm*
#
# For usage and environmental variables run with the -h argument.

source ./setup-test-env.sh
[ $? -ne 0 ] && exit 1

export FSIM_EXE="fsim-rtlsim"

# Tests to run
TESTS="rtl-asm rtl-asm2 rtl-asm8"
if [ "$1" == "-h" ]; then
  print_help
fi

# If there are command line parameters, use those instead.
[ $# -ne 0 ] && TESTS=$@

for TEST in $TESTS; do

  export TEST_NAME=$TEST

  declare -a REGEXS=("SUCCESS in Simulation" "SUCCESS in Simulation" "SUCCESS in Simulation")
  REGEXS+=("SUCCESS in Simulation" "SUCCESS in Simulation")

  # Set up the env for the test
  case $TEST in
    rtl-asm)
      export WORKLOAD_INSTALL=$TG_INSTALL/../fsim/tests/rtl-asm
      export FSIM_ARGS="-c $TG_INSTALL/fsim-configs/dvfs-default.cfg -c $TG_INSTALL/fsim-configs/localhost.cfg -c $WORKLOAD_INSTALL/rtl-asm.cfg"
      ;;
    rtl-asm2)
      export WORKLOAD_INSTALL=$TG_INSTALL/../fsim/tests/rtl-asm2
      export FSIM_ARGS="-c $TG_INSTALL/fsim-configs/dvfs-default.cfg -c $TG_INSTALL/fsim-configs/localhost.cfg -c $WORKLOAD_INSTALL/rtl-asm.cfg"
      ;;
    rtl-asm8)
      export WORKLOAD_INSTALL=$TG_INSTALL/../fsim/tests/rtl-asm8
      export FSIM_ARGS="-c $TG_INSTALL/fsim-configs/dvfs-default.cfg -c $TG_INSTALL/fsim-configs/localhost.cfg -c $WORKLOAD_INSTALL/rtl-asm.cfg"
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

  if [ "$SUCCESS" -eq 0 ]; then
    echo " !!! Test $TEST_NAME failed !!!" 1>&2
  else
    echo "Test $TEST_NAME succeeded"
  fi
  echo
done
