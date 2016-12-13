#!/bin/bash

function success_test() {
  FOUND_REGEX=0
  while read LINE; do
    [[ -n $VERBOSE ]] && echo $LINE

    echo $LINE >> $LOGS_DIR/exec_out

    if [[ $FOUND_REGEX -le ${#REGEXS[@]} ]]; then
      echo $LINE | grep -q "${REGEXS[$FOUND_REGEX]}" && ((FOUND_REGEX++))
    fi
  done

  if [[ $FOUND_REGEX -lt ${#REGEXS[@]} ]]; then
    echo "Output never matched '${REGEXS[$FOUND_REGEX]}'" 1>&2
  else
     echo "===>>>SUCCESS<<<==="
  fi
}

function runtest() {
echo
if [ ! -x $EXEC ]; then
 echo "ERROR: $EXEC not executable"
else
  echo "Running test $TEST"
  # Checking that $LOGS_DIR is not empty just for sanity. We do not want to try to remove /
  [[ -n $LOGS_DIR ]] && rm -rf $LOGS_DIR/*
  if [[ -n $SCAFFOLD ]]; then
    $EXEC 2>&1 | success_test
  else
    $EXEC -ocr:cfg ${APPS_ROOT}/legacy/x86/test_config.cfg 2>&1 | success_test
  fi
fi
}

source ./setup-test-env.sh

# Tests to run
TESTS="legacy_chello legacy_cxxhello"

if [[ $1 == "-h" ]]; then
  echo -e "You may specify one or more of:\n\n$TESTS\n\nDefaults to all tests"
  exit
fi

# If there are command line parameters, use those instead.
[[ $# -ne 0 ]] && TESTS=$@

for TEST in $TESTS; do

  # Set up the env for the test
  case $TEST in
    legacy_cxxhello)
      EXEC=${APPS_ROOT}/legacy/x86/cxxhello
      REGEXS=("Hello World!")
      REGEXS+=("cplus output: 20")
      REGEXS+=("main: Trying blah(7)")
      REGEXS+=("may_throw: Throwing error = 7")
      REGEXS+=("foo::blah: Caught throw (error = 7)")
      REGEXS+=("main: Caught throw (error = 10)")
    ;;
    legacy_chello)
      EXEC=${APPS_ROOT}/legacy/x86/chello
      REGEXS=("Hello World!")
      REGEXS+=("time = $(date -u +'%a %b %-d %H:%M:.. %Y')")
      REGEXS+=("float = 1\.000000")
    ;;
    *)
      echo "Invalid test name '$TEST'" 1>&2
      continue
      ;;
  esac

  # Execute the test, moving on if unsuccessful
  runtest
done
