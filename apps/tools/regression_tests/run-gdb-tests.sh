#!/bin/bash

# This script may accept command line parameters of which test set(s) to run.
# Possible test sets:
#   legacy-hello legacy-hello-8xe mb-printf rtl-asm
#
# Defaults to running all test sets.
#
# This script may optionally use env vars:
#   TG_INSTALL - The install directory of the tg repo
#   LOGS_DIR   - The directory for fsim to place its logs
#   VERBOSE    - If set, then write all of gdb's output to stdout

source ./setup-test-env.sh
[[ $? -ne 0 ]] && exit 1

# Tests to run
TESTS="legacy-hello legacy-hello-8xe mb-printf mb-printf-8xe rtl-asm rtl-asm-8xe"
if [[ $1 == "-h" ]]; then
  echo -e "You may specify one or more of:\n$TESTS\nDefaults to all tests"
  exit
fi

# If there are command line parameters, use those instead.
[[ $# -ne 0 ]] && TESTS=$@

function filter_output() {
  if [[ -n $VERBOSE ]]; then
    sed 's/__PYTHON_OUTPUT__: /PYTHON OUTPUT: /'
  else
    grep "__PYTHON_OUTPUT__: " | sed 's/.*__PYTHON_OUTPUT__: //'
  fi
}

function kill_gdb_and_fsim() {
  killall -e -9 -u $(whoami) $TG_INSTALL/bin/xstg-linux-elf-gdb 2>/dev/null
  ls $TG_INSTALL/bin/fsim* | xargs killall -e -9 -u $(whoami) 2>/dev/null \;
}

for TEST in $TESTS; do

  # Execute all of the subtests

  #Itterate over a list of all the subtests
  grep "^test: " gdb_tests/$TEST.gdbtest | sed 's/^test: //' | while read -r SUBTEST; do

    # Checking that $LOGS_DIR is not empty just for sanity. We do not want to try to remove /
    [[ -n $LOGS_DIR ]] && rm -rf $LOGS_DIR/*

    if [[ -n $VERBOSE ]]; then
      echo "============================================================"
    fi

    echo -n "Executing $TEST -> $SUBTEST ... "

    if [[ -n $VERBOSE ]]; then
      echo
      echo
    fi

    {

      $TG_INSTALL/bin/xstg-linux-elf-gdb -nh \
        -x gdb_tests/python_tester.py \
        -ex "python run_test('gdb_tests/$TEST.gdbtest', '$SUBTEST')" 2>&1 | filter_output

    } < <(tail -f /dev/null --pid=$$) # Make sure gdb doesn't use our stdin.

    if [[ ${PIPESTATUS[0]} -ne 0 ]]; then
      echo "GDB chrashed!!!"
      echo
    fi

    kill_gdb_and_fsim
  done

  if [[ -n $VERBOSE ]]; then
    echo
    echo
  fi
done
