#!/bin/bash

set -e

test_count=0
divider='\n================================================================\n'
error_divider=`tr '=' '>' <<< "$divider"`

negative_tests=""

default_cxx=`make -p -f /dev/null 2>&1 | sed -n 's/^CXX = //p'`

if $default_cxx --version | fgrep -q clang; then
    #using_clang=true
    error_check="sed -f check.sed"
else
    #using_clang=false
    error_check="fgrep error:"
fi

for dir in *; do
    if [ -d "$dir" ] && [ $dir != makefiles ]; then
        if [[ $dir == bad* ]]; then
            negative_tests+="$dir "
            continue
        fi
        printf $divider
        printf "> Running test %s\n\n" $dir
        pushd $dir
        make -f Makefile.x86 clean run WORKLOAD_ARGS=""
        popd
        test_count=$((test_count+1))
    fi
done

for dir in $negative_tests; do
    printf $divider
    printf "> Running negative test %s\n\n" $dir
    pushd $dir
    ! make -f Makefile.x86 clean run WORKLOAD_ARGS="" 2>&1 | tee raw_output.tmp
    tr -d '\r\n' < raw_output.tmp | $error_check > filtered_output.tmp
    if ! [ -s filtered_output.tmp ]; then
        printf $error_divider
        printf "> ERROR: Missing expected output from failure test.\n\n"
        exit 1
    fi
    popd
    test_count=$((test_count+1))
done

printf $divider
printf "> All %d tests passed!\n\n" $test_count
