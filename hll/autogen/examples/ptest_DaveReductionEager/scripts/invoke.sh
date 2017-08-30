#!/bin/bash

#
# Sets up variables pertaining to the current application
# to configure and invoke the generic driver in execution_tool's
#

#
# Define application parameters here
#
# The driver script will interpret 'WORKLOAD_ARGS_NAMES' as a list of workload arguments names
# It is expected there will be environment variables with the exact same names defining
# a list of values for that workload argument.
#

. ${JOB_PROPERTIES}

. ${REPO_TOP}/apps/apps/tools/execution_tools/batch/driver.sh $1