#!/bin/bash

# This script may accept command line parameters of which test(s) to run.
# For a list of possible tests, run it with the -h argument
#
# Defaults to running all tests
#
# This script may optionally use env vars:
#   TG_INSTALL - The install directory of the tg repo
#   LOGS_DIR   - The directory for fsim to place its logs
#   VERBOSE    - If set, then write all of fsim's output to stdout
#   CTEST      - If set, then run the cross test version of the tests

source ./setup-test-env.sh
[[ $? -ne 0 ]] && exit 1

export FSIM_EXE="fsim-swtest"

# Tests to run
TESTS="
orph_test_omp_task_shared
orph_test_omp_parallel_for_firstprivate
orph_test_omp_critical
test_omp_parallel_sections_firstprivate
orph_test_omp_test_nest_lock
test_omp_for_schedule_guided
orph_test_omp_get_wtime
test_omp_parallel_for_ordered
test_omp_single_copyprivate
orph_test_omp_parallel_sections_firstprivate
test_omp_master_3
test_omp_single
test_omp_task_shared
test_omp_section_private
orph_test_omp_task_if
test_omp_test_lock
test_omp_get_wtick
test_omp_for_private
test_omp_for_schedule_static
orph_test_omp_parallel_sections_lastprivate
test_omp_task_if
test_omp_taskyield
test_omp_parallel_sections_lastprivate
test_omp_parallel_for_private
test_omp_parallel_for_reduction
test_omp_sections_reduction
orph_test_omp_single_nowait
orph_test_omp_for_schedule_static
orph_test_omp_for_firstprivate
test_omp_parallel_if
test_omp_for_firstprivate
test_omp_parallel_private
orph_test_omp_in_parallel
test_omp_get_num_threads
orph_test_has_openmp
orph_test_omp_section_firstprivate
test_omp_threadprivate
orph_test_omp_section_private
orph_test_omp_parallel_if
orph_test_omp_task
test_omp_parallel_reduction
orph_test_omp_parallel_for_ordered
orph_test_omp_task_private
orph_test_omp_parallel_for_private
test_omp_in_parallel
test_omp_for_ordered
orph_test_omp_for_collapse
orph_test_omp_parallel_sections_private
test_omp_parallel_num_threads
test_omp_section_firstprivate
test_omp_barrier
orph_test_omp_sections_reduction
test_omp_parallel_sections_private
orph_test_omp_get_wtick
orph_test_omp_lock
orph_test_omp_single
test_omp_parallel_shared
test_omp_for_nowait
test_omp_parallel_copyin
test_omp_lock
test_omp_for_reduction
test_omp_master
test_omp_atomic
test_omp_task_untied
test_omp_for_lastprivate
test_omp_sections_nowait
orph_test_omp_section_lastprivate
test_omp_for_schedule_dynamic
test_omp_parallel_default
test_omp_for_collapse
test_omp_task
test_omp_critical
orph_test_omp_parallel_reduction
orph_test_omp_for_ordered
test_omp_task_final
orph_test_omp_task_firstprivate
orph_test_omp_parallel_for_reduction
orph_test_omp_test_lock
test_omp_flush
test_omp_task_firstprivate
orph_test_omp_master
orph_test_omp_parallel_default
orph_test_omp_barrier
orph_test_omp_taskyield
orph_test_omp_task_untied
orph_test_omp_parallel_for_if
test_omp_parallel_for_if
orph_test_omp_parallel_firstprivate
orph_test_omp_for_nowait
test_omp_single_nowait
orph_test_omp_parallel_num_threads
orph_test_omp_for_schedule_guided
orph_test_omp_for_private
test_omp_test_nest_lock
orph_test_omp_parallel_private
test_omp_parallel_firstprivate
test_omp_nest_lock
orph_test_omp_get_num_threads
orph_test_omp_for_schedule_dynamic
orph_test_omp_flush
orph_test_omp_parallel_shared
orph_test_omp_for_lastprivate
test_omp_parallel_for_lastprivate
orph_test_omp_task_final
test_omp_parallel_for_firstprivate
orph_test_omp_atomic
orph_test_omp_nest_lock
orph_test_omp_for_reduction
orph_test_omp_parallel_for_lastprivate
orph_test_omp_taskwait
test_omp_get_wtime
test_omp_section_lastprivate
test_omp_parallel_sections_reduction
orph_test_omp_single_private
test_omp_taskwait
orph_test_omp_master_3
orph_test_omp_parallel_copyin
orph_test_omp_single_copyprivate
orph_test_omp_sections_nowait
test_omp_single_private
test_has_openmp
orph_test_omp_parallel_sections_reduction
test_omp_task_private
"
export TIMEOUT_SECONDS=${TIMEOUT_SECONDS-420} # Default timeout of 7 min

if [[ $1 == "-h" ]]; then
  echo -e "You may specify one or more of:\n\n$TESTS\n\nDefaults to all tests"
  exit
fi

# If there are command line parameters, use those instead.
[[ $# -ne 0 ]] && TESTS=$@

for TEST in $TESTS; do

  # Add ctest_ to the name to use the crosstest version
  NORMAL_TEST=$TEST
  [[ -n $CTEST ]] && TEST=${TEST/test/ctest}

  export TEST_NAME=$TEST

  declare -a REGEXS=("ready alarm")

  # Using the static (not pie) executables
  EXECUTABLE_INSTALL="$APPS_ROOT/libs/src/libomp/build-tg-xe-static/testsuite/bin/c"

  # The test programs output log files to the directory where they were run from.
  # We want these to go in the LOGS_DIR directory
  export WORKLOAD_INSTALL=$LOGS_DIR

  # Set up the env for the test
  case $TEST in
    *)
      if echo $TESTS | grep -q "\<$NORMAL_TEST\>"; then
        # There is no custom verification for this test. Use the default.
        export FSIM_ARGS="-q -c $APPS_ROOT/legacy/tg-xe/ccfg.cfg -- $EXECUTABLE_INSTALL/$TEST_NAME"
        REGEXS+=("Directive worked without errors\.")
      else
        echo "Invalid test name '$TEST'" 1>&2
        continue
      fi
      ;;
  esac

  REGEXS+=("Client shutdown has been approved")

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
