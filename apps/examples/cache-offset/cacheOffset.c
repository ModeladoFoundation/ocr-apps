/** File: xeonNumaSize.c
 *
 *  Description:
 *    Run a cache-line offset test.
 */
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ocr.h"
#include "ocr-types.h"

#include "rdtsc.h"


#define EXIT { ocrShutdown(); return NULL_GUID; }

// ***** Run time values *****
/** The amount to offset each success cacheLine from the one before. */
const u64 cacheLineOffset = 128;
/** The definition of "large".  Only "large" allocations have the cache line
 *  offset done. See man pages malloc(3) and mallopt(3).  128K is the size at
 *  which malloc(), by default, will start using mmap() to allocate arrays and
 *  will thus put arrays on a page boundary. */
const u64 sizeOfLargeAllocation = 128 * 1024;
/** The number of cache lines.  Assume that cach of 2^N uses bottom N bits to
 *  address cache lines. 4K comes from KNL optimization manual. */
const u64 numCacheLines = 4 * 1024;
/** Size of the test array.  Needs to be at least 'sizeOfLargeAllocation'!
 *  16M is small enough that 64 CPUs could be used in each NUMA region and arrays can
 *  still fit in MCDRAM, either as memory or as MCDRAM cache. */
const u64 sizeOfTestArray = 32 * 1024L * 1024L;
/** Number of times to run the test, each time this program is executed. */
const u64 numTestIterations = 1000;

/** Number of parallel EDTs to run. */
#define NUM_CPUS 4


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
#ifdef OCR_CACHE_LINE_OFFSET_ALLOCATIONS
  /** Only instance in this program of this structure. */
  extern ocrAllocatorQuick_CacheLineHints_t ocrQuickCacheLineHints;
#endif // OCR_CACHE_LINE_OFFSET_ALLOCATIONS

/** Structure to hold results from each tasks tests. */
typedef struct {
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

/** Array of DB GUIDs, one per worker EDT to store results. */
ocrGuid_t dbGuids[NUM_CPUS];
/** Pointers to usable memory in each of the DBs whose GUIDS are
 *  stored in dbGuids. */
WorkerData *dbPtrs[NUM_CPUS];


/** Test the cache line allocation offset problem.
 *
 * @param results  pointer to preallocated results structure
 * @return  0 if successful or -1 if there was an error.
 */
int testRealCacheOffset(TestResults *results) {
    // Set up hints
    // Note: Hints not implemented yet, so we cheat and poke values
    // into an internal value in the quick_allocator.[ch] files.
#ifdef OCR_CACHE_LINE_OFFSET_ALLOCATIONS
    ocrQuickCacheLineHints.offset = cacheLineOffset;
    ocrQuickCacheLineHints.largeSize = sizeOfLargeAllocation;
    ocrQuickCacheLineHints.cacheSize = numCacheLines;
#endif // OCR_CACHE_LINE_OFFSET_ALLOCATIONS

    // Allocate memories
    u64 size = sizeOfTestArray;

    ocrGuid_t mem1Guid, mem2Guid, mem3Guid;
    void *mem1, *mem2, *mem3;

    ocrDbCreate(&mem1Guid, &mem1, size, DB_PROP_NONE, NULL_HINT, NO_ALLOC);
    u64 *mem1Longs = (u64 *)mem1;

    ocrDbCreate(&mem2Guid, &mem2, size, DB_PROP_NONE, NULL_HINT, NO_ALLOC);
    u64 *mem2Longs = (u64 *)mem2;

    ocrDbCreate(&mem3Guid, &mem3, size, DB_PROP_NONE, NULL_HINT, NO_ALLOC);
    u64 *mem3Longs = (u64 *)mem3;

    // Fill memory with indices
    u64 i;
    u64 end = size / sizeof(u64);
    for (i=0; i<end; i++)
        mem2Longs[i] = i;
    for (i=0; i<end; i++)
        mem3Longs[i] =  i;

    // Warm up by running through data one time once...
    u64 tmp;
    for (i=0; i<end; i++)
         mem1Longs[i] = mem2Longs[i] + mem3Longs[i];

    // Now time the run...
    u64 loop;
    u64 loopEnd = numTestIterations;
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
//        if ((loop & 0x7) == 0)
//            ocrPrintf(".");
    }
    // Use some of the data to try to get compiler to not opt away code.
    if (mem1Longs[14] == mem2Longs[14])
        ocrPrintf("THIS ERROR WILL NEVER OCCCUR!\n");

    results->numIterations = loopEnd;
    results->timeNs = averageTime;
    results->numRejects = numBads;

    ocrDbDestroy(mem1Guid);
    ocrDbDestroy(mem2Guid);
    ocrDbDestroy(mem3Guid);

    return 0;
}


/** Run tests on different CPUs. */
ocrGuid_t workerEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u32 i, j;
    u64 workerNum = paramv[0];
    ocrPrintf("Start EDT %ld\n", workerNum);

    TestResults results;

    int iRet = testRealCacheOffset(&results);
    dbPtrs[workerNum]->cacheOffIterations = results.numIterations;
    dbPtrs[workerNum]->cacheOffTimeNs = results.timeNs;
    dbPtrs[workerNum]->cacheOffRejects = results.numRejects;

    ocrPrintf("Finish EDT %ld\n", workerNum);
    return NULL_GUID;
}

ocrGuid_t finishEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    struct timeval tv;
    int i;
    ocrPrintf("Start finishEdt()\n");

    for (i=0; i<NUM_CPUS; i++) {
        WorkerData *data = dbPtrs[i];
        ocrPrintf("    EDT %03d : %d iterations over %ld bytes took %ld ns, "
               "with %d bad iterations\n",
               i, data->cacheOffIterations, sizeOfTestArray,
               data->cacheOffTimeNs, data->cacheOffRejects);
    }

    ocrPrintf("\nDONE! ( calling ocrShutdown() )\n");

    ocrShutdown();
    return NULL_GUID;
}

/** Create a DB with no hints.
 *  This function handles errors.
 *
 * @param db   Where to store the GUID that is created.
 * @param len  The number of bytes to allocate in this DB.
 * @return  Pointer to memory or NULL if failure
 */
u8 *myDbCreate(ocrGuid_t *db, u64 len) {
    void *ptr;
    u8 iRet;

    iRet = ocrDbCreate(db, &ptr, len, DB_PROP_NONE, NULL_HINT, NO_ALLOC);
    if (iRet != 0) {
        if (iRet == OCR_ENOMEM) {
            ocrPrintf("ERROR: mainEdt(): ocrDbCreate() failed to "
                   "allocate %d byte block - OCR_ENOMEM !\n", len);
            return NULL;
        }
        if (iRet == OCR_EINVAL) {
            ocrPrintf("ERROR: mainEdt(): ocrDbCreate() failed to "
                   "to allocate %d byte block - OCR_EINVAL !\n", len);
            return NULL;
        }
        if (iRet == OCR_EBUSY) {
            ocrPrintf("ERROR: mainEdt(): ocrDbCreate() failed to "
                   "to allocate %d byte block - OCR_EBUSY !\n", len);
            // Could retry here...
            return NULL;
        }
        ocrPrintf("ERROR: mainEdt(): ocrDbCreate() failed to "
               "to allocate %d byte block - status = %d !\n", len);
        return NULL;
    }
    return (u8 *)ptr;
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

    ocrPrintf("\n=====================================================\n");
    ocrPrintf("Running %s, starting on:\n", programName);


    int wkr;
    ocrGuid_t finishTemplate, finishGuid;
    ocrGuid_t workerTemplate, workerGuid;
    ocrGuid_t workedt, event;
    u64 childParamv[1];
    u8 *ptr;

    // Fill array with DBs
    for (wkr=0; wkr<NUM_CPUS; wkr++) {
        ptr = myDbCreate(&(dbGuids[wkr]),sizeof(WorkerData));
        if (ptr == NULL)
            EXIT;
        dbPtrs[wkr] = (WorkerData *)ptr;
    }

    ocrPrintf("There are %d EDTs to be created\n", NUM_CPUS);
    ocrEdtTemplateCreate(&finishTemplate, finishEdt, 0, NUM_CPUS);
    ocrEdtCreate(&finishGuid, finishTemplate, 0, NULL, NUM_CPUS,
                 NULL, EDT_PROP_NONE, NULL_HINT, NULL);

    ocrEdtTemplateCreate(&workerTemplate, workerEdt, 1, 1);
    for (wkr=0; wkr<NUM_CPUS; wkr++) {
        childParamv[0] = wkr;
        ocrEdtCreate(&workerGuid, workerTemplate, 1, childParamv,
                     1, &dbGuids[wkr], EDT_PROP_NONE, NULL_HINT, &event);
        ocrAddDependence(event, finishGuid, wkr, DB_MODE_NULL);
    }

    return NULL_GUID;
}
