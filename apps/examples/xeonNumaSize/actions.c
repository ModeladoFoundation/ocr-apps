/** File:  actions.c
 *
 *  Description:
 *    Actions functions to be called by main()
 */

#include <errno.h>
#include <numa.h>
#include <sched.h>
#include <string.h>
#include <time.h>

#include "ocr.h"
#include "rdtsc.h"
#include "numaHelpers.h"
#include "actions.h"

u64 times[1000];

/** Count and display the number of CPUs per NUMA node.
 *
 * @return  0 if successful or -1
 */
int countNumberOfCpusPerNode() {
    ocrPrintf("\n===== Count number of CPUs in each Node.\n");
    clockid_t clockid;

    int maxNode = numa_max_node();
    struct bitmask *cpuMask = numa_allocate_cpumask();
    int node;
    for (node=0; node<=maxNode; node++) {
        if (moveToNode(node) == -1) {
            ocrPrintf("ERROR: countNumberOfCpusPerNode() failed to switch to "
                   "node %d!\n", node);
            return -1;
        }
        if (numa_node_to_cpus(node, cpuMask) != 0) {
            ocrPrintf("ERROR: countNumberOfCpusPerNode) was unable to fill "
                   "cpuMask for node %d\n", node);
            return -1;
        }
        int numCpus = numa_bitmask_weight(cpuMask);
        if (numCpus < 0) {
            ocrPrintf("WARNING: countNumberOfCpusPerNode) was unable to get "
                   "# CPUs for NUMA node %d\n", node);
            continue;
        }
        ocrPrintf("  Node %d has %d CPUs\n", node, numCpus);
    }

    return 0;
}

/** Run the timing loop for rdtsc().
 *  If the 'loopMax' argument is less than 1000, then individual
 *  values will be saved.
 *
 * @param loopMax  number of times the loop should be run
 * @return  0 if successful or -1 if failure
 */
int runRdtscLoop(int loopMax) {
    u64 start, end;
    int i;
    start = rdtsc();
    for (i=0; i<loopMax; i++) {
        times[i] = rdtsc();
    }
    end = rdtsc();
    ocrPrintf("  %d calls to getNanos took %ld nanos, or %ld nanos/call\n",
           loopMax, end - start, (end-start)/(1LL+loopMax));
    if (loopMax <= 1000) {
        int avg = (end - start) / (loopMax + 1);
        if (avg > 30) {
            for (i=0; i<loopMax; i++) {
                u64 diff = (i==0) ? (times[i]-start) : (times[i]-times[i-1]);
                if (diff > 30) {
                    ocrPrintf("........times[%03d] = %ld, delta=%ld\n",
                           i, times[i], diff);
                }
            }
        }
    }
}

/** Test calling rdtsc() for the current CPU.
 *
 * @return  0 if successful or -1 if failure
 */
int testRdtscCpu() {
    int i;

    // Warm up by calling once.
    runRdtscLoop(1);

    // Run rdtsc 1,000,000 times in chunks of 100
    for (i=0; i<10000; i++) {
        ocrPrintf("%03d: ", i);
        runRdtscLoop(100);
    }
    return 0;
}

/** Test calling the rdtsc() call.
 *
 * @return  0 on success and -1 on failure
 */
int testRdtscTiming() {
    u64 start, end;
    int i;
    ocrPrintf("\n===== Test rdtsc() timing on all nodes\n");

    int maxNode = numa_max_node();
    struct bitmask *cpuMask = numa_allocate_cpumask();
    int node;
    for (node=0; node<=maxNode; node++) {
        // Only test on nodes that have CPUs
        if (numa_node_to_cpus(node, cpuMask) != 0) {
            ocrPrintf("ERROR: testRdtscTiming() was unable to fill cpuMask for "
                   "node %d\n", node);
            return -1;
        }
        int numCpus = numa_bitmask_weight(cpuMask);
        if (numCpus < 0) {
            ocrPrintf("WARNING: testRdtscTiming() was unable to get # CPUs for "
                   "NUMA node %d\n", node);
            continue;
        }
        if (numCpus == 0)
            continue;

        ocrPrintf("\n### Node %d\n", node);
        if (moveToNode(node) == -1) {
            ocrPrintf("ERROR: testRdtscTiming() failed to switch to "
                   "node %d!\n", node);
            return -1;
        }

        runRdtscLoop(1000);
    }

    start = rdtsc();
    end = rdtsc();
    ocrPrintf("\n  1 call after warmup took %ld nanos\n", (end-start));

    return 0;
}

/** Function to test moving the current thread to all nodes.
 *
 * @return  0 on success and -1 on failure.
 */
int testMovingToNodes() {
    ocrPrintf("\n===== Test moving to all nodes with CPUs\n");
    u64 start, end, tim, sum = 0, j;
    clockid_t clockid;

    int maxNode = numa_max_node();
    struct bitmask *cpuMask = numa_allocate_cpumask();
    int node;
    for (node=0; node<=maxNode; node++) {
        if (numa_node_to_cpus(node, cpuMask) != 0) {
            ocrPrintf("ERROR: testMovingToNodes() was unable to fill "
                   "cpuMask for node %d\n", node);
            return -1;
        }
        int numCpus = numa_bitmask_weight(cpuMask);
        if (numCpus < 0) {
            ocrPrintf("WARNING: testMovingToNodes() was unable to get "
                   "# CPUs for NUMA node %d\n", node);
            continue;
        }
        // Don't try moving to nodes with no CPUs
        if (numCpus < 1)
            continue;
        if (moveToNode(node) == -1) {
            ocrPrintf("ERROR: testMovingToNodes() failed to switch to "
                   "node %d!\n", node);
            return -1;
        }

        ocrPrintf("\n### Node %d\n", node);
        if (showCpuAndNode() == -1)
            return -1;
    }

    return 0;
}

/** Identify which MCDRAM node is closest to the current node.
 *  Test on all nodes that have CPUs.
 *
 * @return  0 if successful or -1 if there is a problem
 */
int testFindingMcdramOnAllNodes() {
    ocrPrintf("====== Test finding MCDRAM for each node\n");
    int mcdramNode;
    u64 start, end, tim, sum = 0, j;
    clockid_t clockid;

    int maxNode = numa_max_node();
    struct bitmask *cpuMask = numa_allocate_cpumask();
    int node;
    for (node=0; node<=maxNode; node++) {
        if (numa_node_to_cpus(node, cpuMask) != 0) {
            ocrPrintf("ERROR: testFindingMcdramOnAllNodes() was unable to fill "
                   "cpuMask for node %d\n", node);
            return -1;
        }
        int numCpus = numa_bitmask_weight(cpuMask);
        if (numCpus < 0) {
            ocrPrintf("WARNING: testFindingMcdramOnAllNodes() was unable to get "
                   "# CPUs for NUMA node %d\n", node);
            continue;
        }
        // Don't try to find MCDRAM for nodes with no CPUs
        if (numCpus == 0)
            continue;
        if (moveToNode(node) == -1) {
            ocrPrintf("ERROR: testFindingMcdramOnAllNodes() failed to switch to "
                   "node %d!\n", node);
            return -1;
        }

        ocrPrintf("\n### Node %d\n", node);
        mcdramNode = findMyMcdram();
        ocrPrintf("  Guessing that MCDRAM is node %d.\n", mcdramNode);
    }

    return 0;
}

