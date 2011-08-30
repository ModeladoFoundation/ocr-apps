#!/bin/bash
echo $#
if [ ! $# == 6 ];
then
    echo "Format: ./run.sh <input graph> <log_v> <start vertex id> <nthreads> <nsockets> <ntasks_per_thread>"
    exit
fi
LOG_V=$2

#first clear large pages..
ipcs | tail -n+4 | grep root | cut -d" " -f2 | xargs -n1 ipcrm -m

if [ -f /var/lib/hugetlbfs/pagesize-2097152/mem0 ];
then
    echo "Removing file /var/lib/hugetlbfs/pagesize-2097152/mem0"
    rm -f /var/lib/hugetlbfs/pagesize-2097152/mem0
fi

if [ -f /var/lib/hugetlbfs/pagesize-2097152/mem1 ];
then
    echo "Removing file /var/lib/hugetlbfs/pagesize-2097152/mem1"
    rm -f /var/lib/hugetlbfs/pagesize-2097152/mem1
fi

echo 0 > /proc/sys/vm/nr_hugepages

#next run with small pages
#MULTI_SOCKET=1, NUMA_AWARENESS=1
printf "\n****** Compiling and running small page code to get large page sizes\n\n"
echo icpc   -I./TaskQ/include/ ./TaskQ/lib/taskQ_numa.o -DLOG_V=$LOG_V -DEVENLY_DIVIDE_BS -DBLOOM_FILTER -DBIT_LEVEL -DMULTI_SOCKET -DNUMA_AWARENESS -DMAX_PAGES_REQUESTED=0 -DMAX_PAGES_REQUESTED_MMAP_SOCKET0=0 -DMAX_PAGES_REQUESTED_MMAP_SOCKET1=0 -O3 -xHost main.cpp -L/usr/lib64 rdtsc.s -lpthread -lnuma -o a.out.bit.1
icpc   -I./TaskQ/include/ ./TaskQ/lib/taskQ_numa.o  -DLOG_V=$LOG_V -DEVENLY_DIVIDE_BS -DBLOOM_FILTER -DBIT_LEVEL -DMULTI_SOCKET -DNUMA_AWARENESS -DMAX_PAGES_REQUESTED=0 -DMAX_PAGES_REQUESTED_MMAP_SOCKET0=0 -DMAX_PAGES_REQUESTED_MMAP_SOCKET1=0 -O3 -xHost main.cpp -L/usr/lib64 rdtsc.s -lpthread -lnuma -o a.out.bit.1 || {
    echo "Compile failed!" && exit 
}

./a.out.bit.1 $1 $3 $4 $5 $6 >& outsmallpage
MAX_LARGE_MEMORY_MMAP_SOCKET0=`sed -n 's/NUMA large memory (mmap) ::: socket 0 = \([0-9]*\) pages ::: socket 1 = \([0-9]*\) pages/\1/p' < outsmallpage`
MAX_LARGE_MEMORY_MMAP_SOCKET1=`sed -n 's/NUMA large memory (mmap) ::: socket 0 = \([0-9]*\) pages ::: socket 1 = \([0-9]*\) pages/\2/p' < outsmallpage`
MAX_LARGE_MEMORY=`sed -n 's/Regular large memory (shmget)  = \([0-9]*\) pages/\1/p' < outsmallpage`

MAX_LARGE_MEMORY_MMAP_SOCKET0=$(($MAX_LARGE_MEMORY_MMAP_SOCKET0 /100 *100 + 100))
MAX_LARGE_MEMORY_MMAP_SOCKET1=$(($MAX_LARGE_MEMORY_MMAP_SOCKET1 /100 *100 + 100))
MAX_LARGE_MEMORY=$(($MAX_LARGE_MEMORY/100 *100 + 100))

echo "Rounded numbers:"
echo "MAX_LARGE_MEMORY_MMAP_SOCKET0=$MAX_LARGE_MEMORY_MMAP_SOCKET0"
echo "MAX_LARGE_MEMORY_MMAP_SOCKET1=$MAX_LARGE_MEMORY_MMAP_SOCKET1"
echo "MAX_LARGE_MEMORY=$MAX_LARGE_MEMORY"

TOTAL_MAX_LARGE_MEMORY=$(( MAX_LARGE_MEMORY + MAX_LARGE_MEMORY_MMAP_SOCKET0 + MAX_LARGE_MEMORY_MMAP_SOCKET1 ))

MAX_LARGE_MEMORY_SIZE=$(( MAX_LARGE_MEMORY*2 ))
MAX_LARGE_MEMORY_MMAP_SOCKET0_SIZE=$(( MAX_LARGE_MEMORY_MMAP_SOCKET0*2 ))
MAX_LARGE_MEMORY_MMAP_SOCKET1_SIZE=$(( MAX_LARGE_MEMORY_MMAP_SOCKET1*2 ))


#Now set up new large pages
numactl --membind=0 echo $MAX_LARGE_MEMORY_MMAP_SOCKET0 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
numactl --membind=1 echo $MAX_LARGE_MEMORY_MMAP_SOCKET1 > /sys/devices/system/node/node1/hugepages/hugepages-2048kB/nr_hugepages
numactl --interleave=all -- hugeadm --create-mounts --obey-mempolicy
numactl  --huge --length ${MAX_LARGE_MEMORY_MMAP_SOCKET0_SIZE}m --file /var/lib/hugetlbfs/pagesize-2097152/mem0  --touch --membind=0 --strict
numactl  --huge --length ${MAX_LARGE_MEMORY_MMAP_SOCKET1_SIZE}m --file /var/lib/hugetlbfs/pagesize-2097152/mem1  --touch --membind=1 --strict

echo $TOTAL_MAX_LARGE_MEMORY > /proc/sys/vm/nr_hugepages

#Probably need to compile the code...
printf "\n****** Now compiling and running with large pages\n\n"

#MULTI_SOCKET=1, NUMA_AWARENESS=1
echo icpc   -I./TaskQ/include/ ./TaskQ/lib/taskQ_numa.o -DLOG_V=$LOG_V -DEVENLY_DIVIDE_BS -DUSE_LARGE_PAGE -DBLOOM_FILTER -DBIT_LEVEL -DMULTI_SOCKET -DNUMA_AWARENESS -DMAX_PAGES_REQUESTED=$MAX_LARGE_MEMORY -DMAX_PAGES_REQUESTED_MMAP_SOCKET0=$MAX_LARGE_MEMORY_MMAP_SOCKET0 -DMAX_PAGES_REQUESTED_MMAP_SOCKET1=$MAX_LARGE_MEMORY_MMAP_SOCKET1 -O3 -xHost main.cpp -L/usr/lib64 rdtsc.s -lpthread -lnuma -o a.out.bit.1
icpc   -I./TaskQ/include/ ./TaskQ/lib/taskQ_numa.o  -DLOG_V=$LOG_V -DEVENLY_DIVIDE_BS -DUSE_LARGE_PAGE -DBLOOM_FILTER -DBIT_LEVEL -DMULTI_SOCKET -DNUMA_AWARENESS -DMAX_PAGES_REQUESTED=$MAX_LARGE_MEMORY -DMAX_PAGES_REQUESTED_MMAP_SOCKET0=$MAX_LARGE_MEMORY_MMAP_SOCKET0 -DMAX_PAGES_REQUESTED_MMAP_SOCKET1=$MAX_LARGE_MEMORY_MMAP_SOCKET1 -O3 -xHost main.cpp -L/usr/lib64 rdtsc.s -lpthread -lnuma -o a.out.bit.1 || {
    echo "Compile failed!" && exit 
}

#Now run the code
echo "Running code..."
./a.out.bit.1 $1 $3 $4 $5 $6
ipcs | tail -n+4 | grep root | cut -d" " -f2 | xargs -n1 ipcrm -m

