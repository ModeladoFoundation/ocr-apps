/** File: utils.c
 *
 *  Description:
 *    Utility functions to help at startup or if there are problems.
 */

#ifndef __XEON_NUMA_SIZE_UTILS_H__
#define __XEON_NUMA_SIZE_UTILS_H__

struct CmdLineVals {
    /** Program should enable command line debugging. */
    int cmdLineDebug;
    /** Test moving to all nodes. */
    int tstSwitchNodes;
    /** Program should count number of CPUs per node. */
    int countNumCpus;
    /** Flag to enable running 1M calls to rdtsc() on current CPU. */
    int rdtscCpu;
    /** Program should test time of rdtsc() calls. */
    int timeRdtsc;
    /** Test finding MCDRAM for all nodes */
    int findMcdram;
    /** CPU(s) to run on.  Array must hold list of CPUs which should
     *  have threads started on them. */
    int *runCpus;
    /** Number of CPUs threads should be started on. */
    int numCpus;
    /** Size of DRAM chunk. */
    u64 dramSize;
    /** Number of DRAM write loops. */
    unsigned dramWLoops;
    /** Number of DRAM read loops. */
    unsigned dramRLoops;
    /** Number of DRAM random read loops. */
    unsigned dramRLoopRand;
    /** Size of MCDRAM chunk. */
    u64 mcdramSize;
    /** Number of MCDRAM write loops. */
    unsigned mcdramWLoops;
    /** Number of MCRAM read loops. */
    unsigned mcdramRLoops;
    /** Number of MCDRAM random read loops. */
    u64 mcdramRLoopRand;
    /** Number of cache line offset loops. */
    unsigned cacheLineLoops;
    /** Cache line offset amount in bytes. */
    unsigned cacheLineOffset;
    /** Number of cache line allocation test loops to run. */
    u64 cacheAllocLoops;
    /** Definition of "large" for cache line offset code. */
    u64 cacheLineMinLarge;
};

/** Name of this program (for debugging, etc.) */
extern char *programName;

/** Command line values. */
extern struct CmdLineVals clVals;


/** Output a help message.
 *
 */
void sendHelp();

/** Parse the command line arguments.
 *
 * @param argc  the number of arguments on the command line
 * @param argv  an array of command line arguments
 * @return 0 on success or -1 if program should exit
 */
int parseCmdLineArgs(int argc, char **argv);

#endif // __XEON_NUMA_SIZE_UTILS_H__

