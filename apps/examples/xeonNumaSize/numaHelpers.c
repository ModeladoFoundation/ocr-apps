/** Functions that help with NUMA management.
 */

#include <errno.h>
#include <numa.h>
#define __USE_GNU
#include <sched.h>

#include "numaHelpers.h"

/** Find and print which CPU and which NUMA Node the current thread
 *  is running on.
 *
 * @return  0 on success or -1 on failure.
 */
int showCpuAndNode() {
    int curCpu = sched_getcpu();
    if (curCpu == -1) {
        ocrPrintf("  Call to sched_getcpu() failed returning %d [%s]\n",
               errno, strerror(errno));
        return -1;
    }
    int curNode = numa_node_of_cpu(curCpu);
    ocrPrintf("  Running on cpu %d, node %d\n", curCpu, curNode);
    return 0;
}

/** Move the current thread to a specific NUMA node.
 *
 * @param newNode  the node this thread should be running on
 * @return  0 on success and -1 on failure
 */
int moveToNode(int newNode) {
    int iRet = numa_run_on_node(newNode);
    if (iRet != 0) {
        ocrPrintf("ERROR: Call to numa_run_on_node(%d) failed returning %d\n",
               newNode, iRet);
        return -1;
    }
    // Need to call twice to force change to occur now.
    iRet = numa_run_on_node(newNode);
    if (iRet != 0) {
        ocrPrintf("ERROR:  Call to numa_run_on_node(%d) failed returning %d\n",
               newNode, iRet);
        return -1;
    }
    return 0;
}

/** Move the current thread to a specific CPU.
 *
 * @param newCpu  the number of the CPU
 * @return  0 on success and -1 on failure
 */
int moveToCpu(int newCpu) {
    int iRet;

    cpu_set_t affinityMask;
    CPU_ZERO(&affinityMask);
    CPU_SET(newCpu, &affinityMask);

    iRet = sched_setaffinity(0, sizeof(affinityMask), &affinityMask);
    if (iRet != 0) {
        ocrPrintf("ERROR: Call to sched_setaffinity() failed! %d '%s'\n",
               errno, strerror(errno));
        return -1;
    }

    return 0;
}

/** Return the NUMA node this thread is running on.
 *
 * @return  the NUMA node this thread is running on or -1 if error
 */
int getCurrentNumaNode() {
   int curCpu = sched_getcpu();
    if (curCpu == -1) {
        ocrPrintf("ERROR: getCurrentNumaNode(): Call to sched_getcpu() failed "
               "returning %d [%s]\n", errno, strerror(errno));
        return -1;
    }
    int curNode = numa_node_of_cpu(curCpu);
    if (curNode < 0) {
        ocrPrintf("ERROR: getCurrentNumaNode(): Failed to get node for cpu %d\n",
               curCpu);
        return -1;
    }
    return curNode;
}

/** Find the node that is most likely the DRAM controller which is
 *  closest to the NUMA node this thread is running on.
 *
 * @return  0 on success and -1 on failure.
 */
int findMyMcdram() {
    int i;
    // Make sure we have access to all nodes.
    int maxNode = numa_max_node();
    int numMemNodes = numa_num_task_nodes();
    if ((maxNode +1) != numMemNodes) {
        ocrPrintf("ERROR:  findMyMcdram(): there are %d nodes, but this program "
               "can only use %d of them!\n", maxNode+1, numMemNodes);
        return -1;
    }

    // Find out which NUMA node this thread is running on.
    int curNode = getCurrentNumaNode();
    if (curNode < 0)
        return curNode;

    // Find the closest neighbor node, and assume it is the MCDRAM
    // for the current node.

    int mcdramNode = -1;
    int numAtThisDistance = 0;
    unsigned int minDist = -1;
    int node;
    struct bitmask *cpuMask = numa_allocate_cpumask();
    for (node=0; node<numMemNodes; node++) {
        if (node == curNode)  // Skip distance to self.
            continue;
        // Skip any nodes that have CPUs on them
        if (numa_node_to_cpus(node, cpuMask) != 0) {
            ocrPrintf("ERROR: findMyMcdram() was unable to fill cpuMask for "
                   "node %d\n", node);
            return -1;
        }
        int numCpus = numa_bitmask_weight(cpuMask);
        if (numCpus > 0) {
            continue;
        }

        int distance = numa_distance(curNode, node);
        if (distance == 0) {
            ocrPrintf("WARNING: findMyMcdram() was unable to find distance "
                   "between NUMA nodes %d and %d. Continuing...\n",
                   curNode, node);
            continue;
        }
        if (distance > minDist)
            continue;
        if (distance == minDist) {
            numAtThisDistance++;
            continue;
        }
        // Save candidate for closest neighbor
        minDist = distance;
        numAtThisDistance = 1;
        mcdramNode = node;
    }
    if (mcdramNode == -1) {
        ocrPrintf("ERROR: findMyMcdram() failed to find MCDRAM NUMA node!\n");
        return -1;
    }
    if (numAtThisDistance > 1)
        ocrPrintf("WARNING: findMyMcdram() Found %d NUMA nodes at minimum "
               "distance of %d, selecting %d as the MCDRAM node.\n",
               numAtThisDistance, minDist, mcdramNode);

    return mcdramNode;
}

/** Allocate memory from the DRAM of the current NUMA node.
 *  The 'size' argument will be rounded up to the nearest pagesize.
 *
 *  The memory MUST be freed with numa_free()!
 *
 * @param size  the number of bytes to allocate
 * @param debug  0 for no output or 1 for printed information
 * @return  a pointer to the memory or NULL on failure
 */
void *allocateInLocalDram(size_t size, int debug) {
    int curNode = getCurrentNumaNode();
    if (curNode < 0)
        return NULL;

    void *mem = numa_alloc_onnode(size, curNode);
    if (debug) {
        ocrPrintf("DEBUG: allocateInLocalDram(%lu) allocate NUMA node %d\n",
               size, curNode);
        ocrPrintf("DEBUG: allocateInLocalDram(%lu) returns address %p\n",
               size, mem);
    }
    return mem;
}

/** Allocate memory from the MCDRAM that is closet to the current NUMA node.
 *  The 'size' argument will be rounded up to the nearest pagesize.
 *
 *  The memory MUST be freed with numa_free()!
 *
 * @param size  the number of bytes to allocate
 * @param debug  0 for no output or 1 for printed information
 * @return  a pointer to the memory or NULL on failure
 */
void *allocateInLocalMcdram(size_t size, int debug) {
    int mcdramNode = findMyMcdram();
    if (mcdramNode < 0)
        return NULL;

    void *mem = numa_alloc_onnode(size, mcdramNode);
    if (debug) {
        int curNode = getCurrentNumaNode();
        ocrPrintf("DEBUG: allocateInLocalMcdram(%lu) curNode=%d, allocate node=%d\n",
               size, curNode, mcdramNode);
        ocrPrintf("DEBUG: allocateInLocalMcdram(%lu) returns address %p\n",
               size, mem);
    }
    return mem;
}

