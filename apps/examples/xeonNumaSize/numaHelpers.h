/** Functions that help with NUMA management.
 */

#ifndef __NUMA_HELPERS_H__
#define __NUMA_HELPERS_H__

/** Find and print which CPU and which NUMA Node the current thread
 *  is running on.
 *
 * @return  0 on success or -1 on failure.
 */
int showCpuAndNode();

/** Move the current thread to a specific NUMA node.
 *
 * @param newNode  the node this thread should be running on
 * @return  0 on success and -1 on failure
 */
int moveToNode(int newNode);

/** Move the current thread to a specific CPU.
 *
 * @param newCpu  the number of the CPU
 * @return  0 on success and -1 on failure
 */
int moveToCpu(int newCpu);

/** Return the NUMA node this thread is running on.
 *
 * @return  the NUMA node this thread is running on or -1 if error
 */
int getCurrentNumaNode();

/** Find the node that is most likely the DRAM controller which is
 *  closest to the NUMA node this thread is running on.
 *
 * @return  0 on success and -1 on failure.
 */
int findMyMcdram();

/** Allocate memory from the DRAM of the current NUMA node.
 *  The 'size' argument will be rounded up to the nearest pagesize.
 *
 *  The memory MUST be freed with numa_free()!
 *
 * @param size  the number of bytes to allocate
 * @param debug  0 for no output or 1 for printed information
 * @return  a pointer to the memory or NULL on failure
 */
void *allocateInLocalDram(size_t size, int debug);

/** Allocate memory from the MCDRAM that is closet to the current NUMA node.
 *  The 'size' argument will be rounded up to the nearest pagesize.
 *
 *  The memory MUST be freed with numa_free()!
 *
 * @param size   the number of bytes to allocate
 * @param debug  0 for no output or 1 for printed information
 * @return  a pointer to the memory or NULL on failure
 */
void *allocateInLocalMcdram(size_t size, int debug);

#endif //  __NUMA_HELPERS_H__
