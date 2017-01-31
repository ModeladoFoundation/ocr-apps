#!/bin/bash

# This script may accept command line parameters of which test(s) to run.
# Possible tests:
#  c_test_1 c_test_1.p c_test_2 c_test_2.p c_test_3 c_test_3.p
#
# Defaults to running all tests (except OCRcholesky, OCRsmith-waterman, and irqstress)
#
# This script may optionally use env vars:
#   TG_INSTALL - The install directory of the tg repo
#   LOGS_DIR   - The directory for fsim to place its logs
#   VERBOSE    - If set, then write all of fsim's output to stdout

source ./setup-test-env.sh
[[ $? -ne 0 ]] && exit 1

export FSIM_EXE="fsim"

# Tests to run
TESTS="c_test_1 c_test_1.p c_test_2 c_test_2.p c_test_3 c_test_3.p"
if [[ $1 == "-h" ]]; then
  echo "You may specify one or more of:"
  echo "$TESTS"
  echo "Defaults to all tests"
  exit
fi

# If there are command line parameters, use those instead.
[[ $# -ne 0 ]] && TESTS=$@

for TEST in $TESTS; do
  export TEST_NAME=$TEST

  declare -a REGEXS=(
    "TGR-MAIN: starting"
    "TGR-MAIN: initialized subsystems"
    "TGR-MAIN: initialized datastructures"
    "TGR-ELF: - Valid XE ELF64 header \((PIE format) \)\?found"
    "TGR-RUN: Starting XE 0x1 at"
    "TGR-ALARMS: XE1 ready alarm")

  # Set up the env for the test
  case $TEST in
    c_test_1*)
      export WORKLOAD_INSTALL="$APPS_ROOT/libs/src/libtgr/tg-ce/test"
      export FSIM_ARGS="-s -c $WORKLOAD_INSTALL/fsim.cfg"
      export TGKRNL="$APPS_ROOT/libs/install/tg-xe/lib/tgkrnl-tgr"
      export OUTPUT_FILE="$TEST.rck000.cub00.skt0.cls000.blk00.CE.00"
      export TEST
      REGEXS+=("TGR-CONSOLE: (XE1): c_test_1 starting"
               "TGR-CONSOLE: (XE1): tgr_mmap succeeded"
               "TGR-CONSOLE: (XE1): OK_1"
               "TGR-CONSOLE: (XE1): OK_2"
               "CE-OS: argc = 1"
               "TGR-ALARMS: XE 0x1 terminate alarm, terminating xe")
      ;;
    c_test_2*)
      export WORKLOAD_INSTALL="$APPS_ROOT/libs/src/libtgr/tg-ce/test"
      export FSIM_ARGS="-s -c $WORKLOAD_INSTALL/fsim.cfg"
      export TGKRNL="$APPS_ROOT/libs/install/tg-xe/lib/tgkrnl-tgr"
      export OUTPUT_FILE="$TEST.rck000.cub00.skt0.cls000.blk00.CE.00"
      export TEST
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
    c_test_3*)
      export WORKLOAD_INSTALL="$APPS_ROOT/libs/src/libtgr/tg-ce/test"
      export FSIM_ARGS="-s -c $WORKLOAD_INSTALL/fsim.cfg"
      export TGKRNL="$APPS_ROOT/libs/install/tg-xe/lib/tgkrnl-tgr"
      export OUTPUT_FILE="$TEST.rck000.cub00.skt0.cls000.blk00.CE.00"
      export TEST
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
    *)
      echo "Invalid test name '$TEST'" 1>&2
      continue
    ;;
  esac

  REGEXS+=("TGR-MAIN: all blocks done"
           "TG Kernel Terminating\.\.\.")

  # Execute the test, moving on if unsuccessful
  ./exec-fsim-test.sh "${REGEXS[@]}" || continue

  echo "Test $TEST_NAME succeeded"
  echo
done
