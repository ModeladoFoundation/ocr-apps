#* This file is subject to the license agreement located in the file LICENSE
#* and cannot be distributed without it. This notice cannot be
#* removed or modified.

make OCR_TYPE=x86 CONFIG_NUM_THREADS=8 TARGET=fib_hybrid WORKLOAD_ARGS='-r 2 -t 0 10' clean install run
