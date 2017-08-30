/** File:  actions.h
 *
 *  Description:
 *    Interface definitions for the action functions in actions.c.
 */

#ifndef __ACTIONS_H__
#define __ACTIONS_H__

/** Count and display the number of CPUs per NUMA node.
 *
 * @return  0 if successful or -1
 */
int countNumberOfCpusPerNode();

/** Run the timing loop for rdtsc().
 *  If the 'loopMax' argument is less than 1000, then individual
 *  values will be saved.
 *
 * @param loopMax  number of times the loop should be run
 * @return  0 if successful or -1 if failure
 */
int runRdtscLoop(int loopMax);

/** Test calling rdtsc() for the current CPU.
 *
 * @return  0 if successful or -1 if failure
 */
int testRdtscCpu();

/** Test calling the rdtsc() call.
 *
 * @return  0 on success and -1 on failure
 */
int testRdtscTiming();

/** Function to test moving the current thread to all nodes.
 *
 * @return  0 on success and -1 on failure.
 */
int testMovingToNodes();

/** Identify which MCDRAM node is closest to the current node.
 *  Test on all nodes that have CPUs.
 *
 * @return  0 if successful or -1 if there is a problem
 */
int testFindingMcdramOnAllNodes();

#endif   // __ACTIONS_H__
