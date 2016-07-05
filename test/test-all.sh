#!/bin/bash

set -e

test_count=0
divider='\n================================\n'

for dir in *; do
    if [ -d "$dir" ] && [ $dir != makefiles ]; then
        printf $divider
        printf "> Running test %s\n\n" $dir
        pushd $dir
        make -f Makefile.x86 clean run WORKLOAD_ARGS=""
        popd
        test_count=$((test_count+1))
    fi
done

printf $divider
printf "> All %d tests passed!\n\n" $test_count
