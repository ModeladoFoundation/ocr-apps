/** File: xeonNumaSize.c
 *
 *  Description:
 *    Run various NUMA related tests.
 *
 *    The original goal was to determine what size was the best
 *    to use for choosing whether to allocate in MCDRAM or DRAM.
 *    MCDRAM is high bandwidth, but high latency.
 *    DRAM is low bandwith and low latency.
 *    "Small" items should go in DRAM.
 *    "Large" items should go in MCDRAM.
 *    What is the size that splits "small" from "large"?
 */
#include <errno.h>
#include <math.h>
#include <numa.h>
#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ocr.h"
#include "ocr-types.h"

#include "actions.h"
#include "myOcrHelpers.h"
#include "numaHelpers.h"
#include "rdtsc.h"
#include "utils.h"

#define EXIT { ocrShutdown(); return NULL_GUID; }

// ***** CHEAT!!! *****
//
// Define structure and external variable which are really buried
// in quick_allocator.[ch].  Eventually, hints should be used to
// set the 'offset' and 'largeSize' values, and configuration
// values should be used to set the 'cacheSize' value for the platform.
typedef struct {
    u64 offset;     // Offset delta for "large" allocations to reduce cache
                    // line conflicts
    u64 largeSize;  // Size of allocation which is considered "large" in
                    // allocations.
    u64 cacheSize;  // Size of cache line address space.  In example above 2^12.
    u64 curOffset;  // Atomic! Offset for next memory allocation.
} ocrAllocatorQuick_CacheLineHints_t;
/** Only instance in this program of this structure. */
extern ocrAllocatorQuick_CacheLineHints_t ocrQuickCacheLineHints;


/** Structure to hold results from each tasks tests. */
typedef struct {
    /** Number of iterations run on sequential DRAM. */
    u64 dramSeqIterations;
    /** Time in nanoseconds of sequential DRAM test. */
    u64 dramSeqTimeNs;
    /** Number of rejected iterations during sequential DRAM test. */
    u64 dramSeqRejects;
    /** Number of iterations run on sequential MCDRAM. */
    u64 mcdramSeqIterations;
    /** Time in nanoseconds of sequential MCDRAM test. */
    u64 mcdramSeqTimeNs;
    /** Number of rejected iterations during sequential MCDRAM test. */
    u64 mcdramSeqRejects;
    /** Number of iterations run on random DRAM. */
    u64 dramRandIterations;
    /** Time in nanoseconds of random DRAM read test. */
    u64 dramRandTimeNs;
    /** Number of rejected iterations during random DRAM test. */
    u64 dramRandRejects;
    /** Number of iterations run on random MCDRAM. */
    u64 mcdramRandIterations;
    /** Time in nanoseconds of random MCDRAM read test. */
    u64 mcdramRandTimeNs;
    /** Number of rejected iterations during random MCDRAM test. */
    u64 mcdramRandRejects;
    /** Number of iterations run on cache line offset test. */
    u64 cacheOffIterations;
    /** Time in nanoseconds of cache line offset test. */
    u64 cacheOffTimeNs;
    /** Number of rejected iterations during cache line offset test. */
    u64 cacheOffRejects;

} WorkerData;

/** Results of a single test run. */
typedef struct {
    /** Number of iterations. */
    u64 numIterations;
    /** Time in nanoseconds. */
    u64 timeNs;
    /** Number of rejected iterations. */
    u64 numRejects;
} TestResults;

/** Name of this program (for debugging, etc.) */
char *programName = "xeonNumaSize";

/** Array of DBs, one per worker EDT to store results. */
ocrGuid_t *dbGuids;
/** Pointers to memory of DBs in array dbGuids. */
WorkerData **dbPtrs;


/** Test the cache line allocation offset problem.
 *
 * @param results  pointer to preallocated results structure
 * @return  0 if successful or -1 if there was an error.
 */
int testCacheOffset(TestResults *results) {
    if (clVals.cacheLineLoops == 0) {
        ocrPrintf("ERROR! testCacheOffset() - clVals.cacheLineLoops == 0!\n");
        return -1;
    }

    // Allocate memories
    u64 base = (clVals.dramSize == 0) ? 1LL<<30 : clVals.dramSize;
    u64 offset = clVals.cacheLineOffset;
    u64 size = base;
    void *mem1 = allocateInLocalDram(size, 0);
    u64 *mem1Longs = (u64 *)mem1;
//    ocrPrintf("Size1=%lu, mem1=%p, mem1Longs=%p\n",
//           size, mem1, mem1Longs);

    u64 size2 = base + offset;
    void *mem2 = allocateInLocalDram(size2, 0);
    u64 *mem2Longs = (u64 *)(mem2 + offset);
//    ocrPrintf("Size2=%lu, mem2=%p, mem2Longs=%p\n",
//           size2, mem2, mem2Longs);

    u64 size3 = base + offset + offset;
    void *mem3 = allocateInLocalDram(size3, 0);
    u64 *mem3Longs = (u64 *)(mem3 + offset + offset);
//    ocrPrintf("Size3=%lu, mem3=%p, mem3Longs=%p\n",
//           size3, mem3, mem3Longs);

    // Fill memory with indices
    u64 i;
    u64 end = base / sizeof(u64);
    for (i=0; i<end; i++)
        mem2Longs[i] = i;
    for (i=0; i<end-(2*clVals.cacheLineOffset); i++)
        mem3Longs[i] =  i;

    // Warm up reading once...
    // Probably useless, since it is random, but doing once should help
    // stablize the hardware.
    u64 tmp;
    for (i=0; i<end; i++)
         mem1Longs[i] = mem2Longs[i] + mem3Longs[i];

    // Now time the run...
    u64 loop;
    u64 loopEnd = clVals.cacheLineLoops;
    u64 errors = 0;
    u64 tstart, tend, tdelta, ttotal = 0;
    u64 averageTime;
    u64 numBads = 0;
    u64 tot = 0;
    u64 next = 0;
    for (loop=0; loop<loopEnd; loop++) {
        tstart = rdtsc();
        for (i=0; i<end; i++) {
            mem1Longs[i] = mem2Longs[i] + mem3Longs[i];
        }
        tend = rdtsc();
        tdelta = tend - tstart;
        // After we get going, discard any values that are 10%
        // larger than the average
        if ((loop > 5) && (tdelta > (averageTime * 1.1))) {
            numBads++;
            continue;
        }

        ttotal += tdelta;
        averageTime = ttotal / (loop+1);
        if ((loop & 0x3) == 0)
            ocrPrintf(".");
    }
    if (mem1Longs[14] == mem2Longs[14])
        ocrPrintf("THIS ERROR WILL NEVER OCCCUR!\n");

    results->numIterations = loopEnd;
    results->timeNs = averageTime;
    results->numRejects = numBads;

    return 0;
}

/** Test the cache line allocation offset problem using the real allocator.
 *
 * @param results  pointer to preallocated results structure
 * @return  0 if successful or -1 if there was an error.
 */
int testRealCacheOffset(TestResults *results) {
    if (clVals.cacheAllocLoops == 0) {
        ocrPrintf("ERROR! testRealCacheOffset() - clVals.cacheAllocLoops == 0!\n");
        return -1;
    }

    // Set up hints
    // Note: Hints not implemented yet, so we cheat and poke values
    // into an internal value in the quick_allocator.[ch] files.
    ocrQuickCacheLineHints.offset = clVals.cacheLineOffset;
    ocrQuickCacheLineHints.largeSize = 1024*1024;
    ocrQuickCacheLineHints.cacheSize = 4096;

    // Allocate memories
    u64 size = (clVals.dramSize == 0) ? 1LL<<30 : clVals.dramSize;

    ocrGuid_t mem1Guid, mem2Guid, mem3Guid;
    void *mem1, *mem2, *mem3;

    ocrDbCreate(&mem1Guid, &mem1, size, DB_PROP_NONE, NULL_HINT, NO_ALLOC);
    u64 *mem1Longs = (u64 *)mem1;
    ocrPrintf("Size1=%lu, mem1=%p, mem1Longs=%p\n",
           size, mem1, mem1Longs);

    ocrDbCreate(&mem2Guid, &mem2, size, DB_PROP_NONE, NULL_HINT, NO_ALLOC);
    u64 *mem2Longs = (u64 *)mem2;
    ocrPrintf("Size=%lu, mem2=%p, mem2Longs=%p\n",
           size, mem2, mem2Longs);

    ocrDbCreate(&mem3Guid, &mem3, size, DB_PROP_NONE, NULL_HINT, NO_ALLOC);
    u64 *mem3Longs = (u64 *)mem3;
    ocrPrintf("Size=%lu, mem3=%p, mem3Longs=%p\n",
           size, mem3, mem3Longs);

    // Fill memory with indices
    u64 i;
    u64 end = size / sizeof(u64);
    for (i=0; i<end; i++)
        mem2Longs[i] = i;
    for (i=0; i<end-(2*clVals.cacheLineOffset); i++)
        mem3Longs[i] =  i;

    // Warm up reading once...
    // Probably useless, since it is random, but doing once should help
    // stablize the hardware.
    u64 tmp;
    for (i=0; i<end; i++)
         mem1Longs[i] = mem2Longs[i] + mem3Longs[i];

    // Now time the run...
    u64 loop;
    u64 loopEnd = clVals.cacheAllocLoops;
    u64 errors = 0;
    u64 tstart, tend, tdelta, ttotal = 0;
    u64 averageTime;
    u64 numBads = 0;
    u64 tot = 0;
    u64 next = 0;
    for (loop=0; loop<loopEnd; loop++) {
        tstart = rdtsc();
        for (i=0; i<end; i++) {
            mem1Longs[i] = mem2Longs[i] + mem3Longs[i];
        }
        tend = rdtsc();
        tdelta = tend - tstart;
        // After we get going, discard any values that are 10%
        // larger than the average
        if ((loop > 5) && (tdelta > (averageTime * 1.1))) {
            numBads++;
            continue;
        }

        ttotal += tdelta;
        averageTime = ttotal / (loop+1);
        if ((loop & 0x3) == 0)
            ocrPrintf(".");
    }
    if (mem1Longs[14] == mem2Longs[14])
        ocrPrintf("THIS ERROR WILL NEVER OCCCUR!\n");

    results->numIterations = loopEnd;
    results->timeNs = averageTime;
    results->numRejects = numBads;

    return 0;
}



/** Test reading memory.
 *
 * @param kind     should be 0 of DRAM or 1 for MCDRAM
 * @param results  pointer to preallocated results structure
 * @return  0 if successful or -1 if there was an error.
 */
int testReadingMem(int kind, TestResults *results) {
    u64 size;

    // Allocate memory in this node's DRAM
    void *mem;
    if (kind) {
        size = (clVals.mcdramSize == 0) ? 1LL<<30 : clVals.mcdramSize;
        mem = allocateInLocalMcdram(size, 0);
    }
    else {
        size = (clVals.dramSize == 0) ? 1LL<<30 : clVals.dramSize;
        mem = allocateInLocalDram(size, 0);
    }
    if (mem == NULL) {
        ocrPrintf("ERROR: testReadingMem(%s) failed to allocate %lu bytes "
               "of DRAM\n", (kind?"MCDRAM":"DRAM"), size);
        return -1;
    }

    // Fill memory with known good values.
    u64 i;
    u64 end = size / sizeof(u64);
    u64 *memLongs = (u64 *)mem;
    for (i=0; i<end; i++)
        memLongs[i] = i;

    // Warm up reading once...
    u64 tmp;
    for (i=0; i<end; i++)
        tmp += memLongs[i];

    // Now time the run...
    u64 loop;
    u64 loopEnd = kind ? clVals.mcdramRLoops : clVals.dramRLoops;
    u64 errors = 0;
    u64 tstart, tend, tdelta, ttotal = 0;
    u64 averageTime;
    u64 numBads = 0;
    for (loop=0; loop<loopEnd; loop++) {
        tstart = rdtsc();
        for (i=0; i<end; i++) {
            if (memLongs[i] != i) {
                ocrPrintf("ERROR: testReadingMem(%s) loop %d failed! "
                       "memLongs[%ld] was %ld!\n",
                       loop, (kind?"MCDRAM":"DRAM"), memLongs[i], i);
                errors++;
            }
        }
        tend = rdtsc();
        tdelta = tend - tstart;
        // After we get going, discard any values that are 10%
        // larger than the average
        if ((loop > 5) && (tdelta > (averageTime * 1.1))) {
            numBads++;
            continue;
        }

        ttotal += tdelta;
        averageTime = ttotal / (loop+1);

        if ((loop & 0x3) == 0)
            ocrPrintf(".");
    }

    results->numIterations = loopEnd;
    results->timeNs = averageTime;
    results->numRejects = numBads;

    return 0;
}


/** Test reading memory randomly
 *
 * @param kind  should be 0 of DRAM or 1 for MCDRAM
 * @param results  pointer to preallocated results structure
 * @return  0 if successful or -1 if there was an error.
 */
int testReadingRandMem(int kind, TestResults *results) {
    u64 size;

    // Allocate memory in this node's DRAM
    void *mem;
    if (kind) {
        size = (clVals.mcdramSize == 0) ? 1LL<<30 : clVals.mcdramSize;
        mem = allocateInLocalMcdram(size, 0);
    }
    else {
        size = (clVals.dramSize == 0) ? 1LL<<30 : clVals.dramSize;
        mem = allocateInLocalDram(size, 0);
    }
    if (mem == NULL) {
        ocrPrintf("ERROR: testReadingRandMem(%s) failed to allocate %lu bytes "
               "of DRAM\n", (kind?"MCDRAM":"DRAM"), size);
        return -1;
    }

    // Fill memory with indices
    u64 i;
    u64 end = size / sizeof(u64);
    u64 *memLongs = (u64 *)mem;

    for (i=0; i<end; i++)
        memLongs[i] = i;

    // Shuffle values around.  By shuffling we can guarantee there are
    // no loops longer than a reference to self.
    double num = 1.0 * end;
    double denom = pow(2.0, 31.0);
    double scale = num / denom;
    u64 pageSize = sysconf(_SC_PAGESIZE);
    lrand48(); lrand48();
    for (i=0; i<end; i++) {
        u64 r1 = lrand48();
        u64 dst = r1 * scale;
        u64 src = dst;
        // Pick a source from a different page
        while ((src<(dst+pageSize)) && (src>(dst-pageSize))) {
            u64 r2 = lrand48();
            src = r2 * scale;
            // Avoid endless loop - don't reference self
            if (memLongs[src] == dst)
                src = dst; // Fail while test and try again
        }
        memLongs[dst] = memLongs[src];
    }

    // Warm up reading once...
    // Probably useless, since it is random, but doing once should help
    // stablize the hardware.
    u64 tmp;
    for (i=0; i<end; i++)
        tmp += memLongs[i];

    // Now time the run...
    u64 loop;
    u64 loopEnd = kind ? clVals.mcdramRLoopRand : clVals.dramRLoopRand;
    u64 errors = 0;
    u64 tstart, tend, tdelta, ttotal = 0;
    u64 averageTime;
    u64 numBads = 0;
    u64 tot = 0;
    u64 next = 0;
    ocrPrintf("loopEnd is %d\n", loopEnd);
    for (loop=0; loop<loopEnd; loop++) {
        tstart = rdtsc();
        for (i=0; i<end; i++) {
            next = memLongs[next];
            tot += next;
        }
        tend = rdtsc();
        tdelta = tend - tstart;
        // After we get going, discard any values that are 10%
        // larger than the average
        if ((loop > 5) && (tdelta > (averageTime * 1.1))) {
            numBads++;
            continue;
        }

        ttotal += tdelta;
        averageTime = ttotal / (loop+1);
//        ocrPrintf("(%lu)  %lu / %lu = %lu\n", tdelta, ttotal, loop+1, averageTime);
        if ((loop & 0x3) == 0)
            ocrPrintf(".");
    }
    if (tot == -2)
        ocrPrintf("Ignore this value! %ld\n", tot);

    results->numIterations = loopEnd;
    results->timeNs = averageTime;
    results->numRejects = numBads;

    return 0;
}


/** Run tests on a specific CPU. */
ocrGuid_t workerEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u32 i, j;
    u64 workerNum = paramv[0];

    TestResults results;

    if (clVals.dramRLoops > 0) {
        testReadingMem(0, &results);
        dbPtrs[workerNum]->dramSeqIterations = results.numIterations;
        dbPtrs[workerNum]->dramSeqTimeNs = results.timeNs;
        dbPtrs[workerNum]->dramSeqRejects = results.numRejects;
    }
    if (clVals.mcdramRLoops > 0) {
        testReadingMem(1, &results);
        dbPtrs[workerNum]->mcdramSeqIterations = results.numIterations;
        dbPtrs[workerNum]->mcdramSeqTimeNs = results.timeNs;
        dbPtrs[workerNum]->mcdramSeqRejects = results.numRejects;
    }
    if (clVals.dramRLoopRand > 0) {
        testReadingRandMem(0, &results);
        dbPtrs[workerNum]->dramRandIterations = results.numIterations;
        dbPtrs[workerNum]->dramRandTimeNs = results.timeNs;
        dbPtrs[workerNum]->dramRandRejects = results.numRejects;
    }
    if (clVals.mcdramRLoopRand > 0) {
        testReadingRandMem(1, &results);
        dbPtrs[workerNum]->mcdramRandIterations = results.numIterations;
        dbPtrs[workerNum]->mcdramRandTimeNs = results.timeNs;
        dbPtrs[workerNum]->mcdramRandRejects = results.numRejects;
    }
    if (clVals.cacheLineLoops > 0) {
        int iRet = testCacheOffset(&results);
        dbPtrs[workerNum]->cacheOffIterations = results.numIterations;
        dbPtrs[workerNum]->cacheOffTimeNs = results.timeNs;
        dbPtrs[workerNum]->cacheOffRejects = results.numRejects;
    }
    if (clVals.cacheAllocLoops > 0) {
        int iRet = testRealCacheOffset(&results);
        dbPtrs[workerNum]->cacheOffIterations = results.numIterations;
        dbPtrs[workerNum]->cacheOffTimeNs = results.timeNs;
        dbPtrs[workerNum]->cacheOffRejects = results.numRejects;
    }


    return NULL_GUID;
}

ocrGuid_t finishEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    struct timeval tv;
    int i;

    if (clVals.dramRLoops > 0) {
        ocrPrintf("\nRunning DRAM sequential read test:\n");
        for (i=0; i<clVals.numCpus; i++) {
            WorkerData *data = dbPtrs[i];
            ocrPrintf("    CPU %03d : %d iterations over %ld bytes took %ld ns, "
                   "with %d bad iterations\n",
                   clVals.runCpus[i], data->dramSeqIterations,
                   clVals.dramSize, data->dramSeqTimeNs,
                   data->dramSeqRejects);
        }
    }
    if (clVals.dramRLoopRand > 0) {
        ocrPrintf("\nRunning DRAM random read test:\n");
        for (i=0; i<clVals.numCpus; i++) {
            WorkerData *data = dbPtrs[i];
            ocrPrintf("    CPU %03d : %d iterations over %ld bytes took %ld ns, "
                   "with %d bad iterations\n",
                   clVals.runCpus[i], data->dramRandIterations,
                   clVals.dramSize, data->dramRandTimeNs,
                   data->dramRandRejects);
        }
    }

    if (clVals.mcdramRLoops > 0) {
        ocrPrintf("\nRunning MCDRAM sequential read test:\n");
        for (i=0; i<clVals.numCpus; i++) {
            WorkerData *data = dbPtrs[i];
            ocrPrintf("    CPU %03d : %d iterations over %ld bytes took %ld ns, "
                   "with %d bad iterations\n",
                   clVals.runCpus[i], data->mcdramSeqIterations,
                   clVals.mcdramSize, data->mcdramSeqTimeNs,
                   data->mcdramSeqRejects);
        }
    }
    if (clVals.mcdramRLoopRand > 0) {
        ocrPrintf("\nRunning MCDRAM random read test:\n");
        for (i=0; i<clVals.numCpus; i++) {
            WorkerData *data = dbPtrs[i];
            ocrPrintf("    CPU %03d : %d iterations over %ld bytes took %ld ns, "
                   "with %d bad iterations\n",
                   clVals.runCpus[i], data->mcdramRandIterations,
                   clVals.mcdramSize, data->mcdramRandTimeNs,
                   data->mcdramRandRejects);
        }
    }
    if (clVals.cacheLineLoops > 0) {
        ocrPrintf("\nRunning cache line offset test with offset = %ld:\n",
               clVals.cacheLineOffset);
        for (i=0; i<clVals.numCpus; i++) {
            WorkerData *data = dbPtrs[i];
            ocrPrintf("    CPU %03d : %d iterations over %ld bytes took %ld ns, "
                   "with %d bad iterations\n",
                   clVals.runCpus[i], data->cacheOffIterations,
                   clVals.mcdramSize, data->cacheOffTimeNs,
                   data->cacheOffRejects);
        }
    }
    if (clVals.cacheAllocLoops > 0) {
        ocrPrintf("\nRunning cache line offset test with real allocator, "
               "offset = %ld, large = %ld\n",
               clVals.cacheLineOffset, clVals.cacheLineMinLarge);
        for (i=0; i<clVals.numCpus; i++) {
            WorkerData *data = dbPtrs[i];
            ocrPrintf("    CPU %03d : %d iterations over %ld bytes took %ld ns, "
                   "with %d bad iterations\n",
                   clVals.runCpus[i], data->cacheOffIterations,
                   clVals.mcdramSize, data->cacheOffTimeNs,
                   data->cacheOffRejects);
        }
    }

    ocrPrintf("\nDONE!\n");

    ocrShutdown();
    return NULL_GUID;
}


/** Main entry point of the program.
 *
 * @param paramc  unused count of parameters passed from creating EDT
 * @param paramv  unused array of parameters passed from creating EDT
 * @param depc    unused number of items in depv (maybe)
 * @param depv    startup arguments including command line arguments
 * @return  a NULL GUID
 */
ocrGuid_t mainEdt ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 argc = ocrGetArgc(depv[0].ptr);
    u32 i;
    int iRet;
    char *argv[argc];

    for(i=0;i<argc;i++) {
        argv[i] = ocrGetArgv(depv[0].ptr,i);
    }
    programName = argv[0];
    if (parseCmdLineArgs(argc, argv) == -1)
        EXIT;

    ocrPrintf("\n=====================================================\n");
    ocrPrintf("Running xeonNumaSize\n");
    if (showCpuAndNode() == -1)
        EXIT;
    ocrPrintf("\n");


    // Make sure NUMA interface is available.
    int tf = numa_available();
    if (tf == -1) {
        ocrPrintf("Function numa_available() returned -1! libnuma not working!\n");
        EXIT;
    }

    // If a specific CPU is requested, then use it
    if (clVals.numCpus > 0)
        moveToCpu(clVals.runCpus[0]);
    else {
        // If a specified CPU was not requested, then use the current one
        clVals.runCpus[0] = sched_getcpu();
        clVals.numCpus = 1;
    }
    if (showCpuAndNode() == -1)
        EXIT;

    if (clVals.countNumCpus) {
        countNumberOfCpusPerNode();
    }
    if (clVals.rdtscCpu) {
        testRdtscCpu();
    }
    if (clVals.timeRdtsc) {
        testRdtscTiming();
    }
    if (clVals.tstSwitchNodes) {
        testMovingToNodes();
    }
    if (clVals.findMcdram) {
        testFindingMcdramOnAllNodes();
    }


    {
        int wkr;
        ocrGuid_t finishTemplate, finishGuid;
        ocrGuid_t workerTemplate, workerGuid;
        ocrGuid_t workedt, event;
        u64 paramv[1];
        u8 *ptr;

        // Create array to hold DBs
        dbGuids = (ocrGuid_t *)malloc(clVals.numCpus * sizeof(ocrGuid_t));
        if (dbGuids == NULL) {
            ocrPrintf("ERROR: mainEdt() failed to malloc(%d) for worker Guids!\n",
                   (clVals.numCpus * sizeof(ocrGuid_t)));
            EXIT;
        }
        dbPtrs = (WorkerData **)malloc(clVals.numCpus * sizeof(u8 *));
        if (dbPtrs == NULL) {
            ocrPrintf("ERROR: mainEdt() failed to malloc(%d) for worker pointers!\n",
                   (clVals.numCpus * sizeof(u8 *)));
        }
        // Fill array with DBs
        for (wkr=0; wkr<clVals.numCpus; wkr++) {
            ptr = myDbCreate(&(dbGuids[wkr]),sizeof(WorkerData));
            if (ptr == NULL)
                EXIT;
            dbPtrs[wkr] = (WorkerData *)ptr;
        }

        ocrPrintf("There are %d cpus to be created\n", clVals.numCpus);
        ocrEdtTemplateCreate(&finishTemplate, finishEdt, 0, clVals.numCpus);
        ocrEdtCreate(&finishGuid, finishTemplate, 0, NULL, clVals.numCpus,
                     NULL, EDT_PROP_NONE, NULL_HINT, NULL);

        ocrEdtTemplateCreate(&workerTemplate, workerEdt, 1, 1);
        for (wkr=0; wkr<clVals.numCpus; wkr++) {
            paramv[0] = wkr;
            ocrEdtCreate(&workerGuid, workerTemplate, 1, paramv,
                         1, &dbGuids[wkr], EDT_PROP_NONE, NULL_HINT, &event);
            ocrAddDependence(event, finishGuid, wkr, DB_MODE_NULL);
        }
    }

    return NULL_GUID;
}
