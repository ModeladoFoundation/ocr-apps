#!/bin/bash

# DO NOT CALL THIS SCRIPT DIRECTLY

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
    pwd
  else
    echo "Directory doesn't exist:" 1>&2
    echo $1 1>&2
    exit 1
  fi
}

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
  if find $LOGS_DIR -exec basename {} \; | grep -q -v "log\|out\|err\|cfg"; then
    echo "Logs directory '$LOGS_DIR' exists and appears to contain non-log files." 1>&2
    echo "Cowardly refusing to delete." 1>&2
    echo "If you wish to use this directory, please remove the files it contains first." 1>&2
    exit 1
  fi
else
  mkdir -p $LOGS_DIR
fi
