#!/bin/sh
set -x
valgrind --verbose --leak-check=summary --undef-value-errors=no ./lulesh 2>&1 | tee nohup.out
exit
