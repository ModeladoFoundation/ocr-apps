#!/bin/bash

# DO NOT CALL THIS SCRIPT DIRECTLY
#
# This script initializes defaults for environment vars used by
# the test scripts, primarially exec-fsim-test.sh
#
# The current vars and defaults are:
#	TG_INSTALL	- up the dir tree to just outside the repo and then down to tg/tg/install
#	APPS_ROOT   - up the tree to apps/apps
#	LOGS_DIR    - logs/ in the current directory
#
# This script may optionally use env vars:
#   TG_INSTALL - The install directory of the tg repo
#   LOGS_DIR   - The directory for fsim to place its logs

# Try to make sure that we are in the directory where this
# script is located
cd "$( dirname "${BASH_SOURCE[0]}" )"

# This function tests if a directory exists. If it does
# then it prints out an absolute path to the directory.
function clean_path() {
  if [[ -d $1 ]]; then
    cd $1;
    pwd -P
  else
    echo "Directory doesn't exist:" 1>&2
    echo $1 1>&2
    exit 1
  fi
}

# This function prints out the help message for the
# test script
function print_help() {
  echo "usage: $(basename $0) [test ...]  execute tests"
  echo "   or: $(basename $0) -h          show this help"
  echo
  echo "For tests you may specify one or more of:"

  if [[ "$0" =~ "ocr" ]]; then
    echo -e "\n$TESTS OCRcholesky OCRsmith-waterman irqstress\n"
    echo "Defaults to all tests except OCRcholesky, OCRsmith-waterman, and irqstress."
  else
    echo -e "You may specify one or more of:\n\n$TESTS\n\nDefaults to all tests."
  fi

  echo
  echo "This test uses the following environmental variables."
  echo "All are optional."
  echo
  echo "TG_INSTALL - The install directory of the tg repo"
  echo "               Default: ../../../../tg/tg/install"
  echo "LOGS_DIR   - The directory for fsim to place its logs"
  echo "               Default: ./logs"
  echo "VERBOSE    - If set, then write all test output to stdout"
  echo "               Default: unset"

  if [[ "$0" =~ "gdb" ]]; then
    # run-gdb-tests.sh help
    echo
    echo "Note: all VERBOSE non-gdb output will be annotated with 'PYTHON OUTPUT:'"
  else
    # run-fsim-*.sh help
    echo "TIME_TESTS - If set, then display test runtime on completion"
    echo "               Redundant if VERBOSE is set"
    echo "               Default: unset"
    #echo "TIMEOUT_SECONDS - If set, then abort test after this many seconds"
    echo "TIMEOUT_SECONDS"
    echo "           - If set, then abort test after this many seconds"
    echo "               If set to 0 or unset then there is no timeout"
    echo "               Default: ${TIMEOUT_SECONDS-unset}"
    if [[ "$0" =~ openmp ]]; then
      echo "CTEST      - If set, then run the cross test version of the tests"
      echo "               The cross test versions of the tests omit whatever"
      echo "               feature of openmp is being tested. These tests typically"
      echo "               fail. If they don't fail, then a success in the non-cross"
      echo "               test may be a false positive."
      echo "               Default: unset"
    fi
  fi
  exit
}

# This function handles ctrl-c interrupts. It cancels
# the current test. It has a .4 sec sleep timeout
# so if two ctrl-c presses happen in quick succession,
# it aborts the script.
function ctrl_c() {
  echo "** Interrupt **"
  killall -e -9 -u $(whoami) $TG_INSTALL/bin/xstg-linux-elf-gdb 2>/dev/null
  killall -9 -u $(whoami) $TG_INSTALL/bin/fsim 2>/dev/null
  echo "Test aborted"
  echo
  sleep .4 || exit 1
}

# The ctrl-c handler is only supported for fsim tests
[[ "${BASH_SOURCE[1]}" =~ "fsim" ]] && trap ctrl_c INT

# Attempt to determine locations of repos and install directories for tests.
export TG_INSTALL=$(clean_path ${TG_INSTALL:-$(pwd)/../../../../tg/tg/install})
[[ -z $TG_INSTALL ]] && exit 1;

export APPS_ROOT=$(clean_path "$TG_INSTALL/../../../apps/apps")
[[ -z $APPS_ROOT ]] && exit 1;

export LOGS_DIR=${LOGS_DIR:-$(pwd)/logs}

if [[ -e $LOGS_DIR ]]; then
  if [[ ! -d $LOGS_DIR ]]; then
    echo "Logs directory '$LOGS_DIR' exists and is not a directory." 1>&2
    echo "Cowardly refusing to delete." 1>&2
    exit 1
  fi

  # Check if the logs directory contains non-log files.
  LOG_FILES_RE="log\|out\|err\|cfg\|rck\|top\.network\|core"
  if find $LOGS_DIR -exec basename {} \; | grep -q -v "$LOG_FILES_RE"; then
    echo "Logs directory '$LOGS_DIR' exists and appears to contain non-log files." 1>&2
    echo "Cowardly refusing to delete." 1>&2
    echo "If you wish to use this directory, please remove the files it contains first." 1>&2
    exit 1
  fi
else
  mkdir -p $LOGS_DIR
fi
