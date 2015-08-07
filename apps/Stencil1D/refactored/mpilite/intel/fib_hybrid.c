/*
* This file is subject to the license agreement located in the file LICENSE
* and cannot be distributed without it. This notice cannot be
* removed or modified.
*/

/*
 * fib_hybrid.c
 * Does not use finish EDTs
 * Originally written in Feb 2012 by Justin Teller
 * Modified for OCR 0.9 by Romain Cledat
 * Copied from ocr/examples/fib 7/28/15 by Mark Davis
 * to be an example of hybrid MPI-Lite and OCR:
 * Use 2 mpi ranks: 0 does fib(n-1), 1 does what rank 1 tells it (n-2);
 * "reduces" the results back, to produce fib(n).
 * Fib is computed using the OCR version.
 */

#include <mpi.h>

#define ENABLE_EXTENSION_LEGACY 1

#include "ocr.h"
#include "ocr-std.h"
#include <extensions/ocr-legacy.h>

#include "stdlib.h"

ocrGuid_t complete(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    u64 arg = (u64)paramv[0];
    int my_ID = paramv[1];
    ocrGuid_t inDep;
    u32 in1, in2;
    u32 out;

    inDep = (ocrGuid_t)arg;

    /* When we run, we got our inputs from fib(n-1) and fib(n-2) */
    in1 = *(u32*)depv[0].ptr;
    in2 = *(u32*)depv[1].ptr;
    out = *(u32*)depv[2].ptr;
    PRINTF("r%d Done with %d (%d + %d)\n", my_ID, out, in1, in2);
    /* we return our answer in the 3rd db passed in as an argument */
    *((u32*)(depv[2].ptr)) = in1 + in2;

    /* The app is done with the answers from fib(n-1) and fib(n-2) */
    ocrDbDestroy(depv[0].guid);
    ocrDbDestroy(depv[1].guid);

    /* and let our parent's completion know we're done with fib(n) */
    ocrEventSatisfy(inDep, depv[2].guid);

    return NULL_GUID;
}

ocrGuid_t fibEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    void* ptr;
    ocrGuid_t inDep;
    ocrGuid_t fib0, fib1, comp;
    ocrGuid_t fibDone[2];
    ocrGuid_t fibArg[2];

    inDep = (ocrGuid_t)paramv[0];
    int my_ID = paramv[1];

    u32 n = *(u32*)(depv[0].ptr);
    PRINTF("r%d Starting fibEdt(%u)\n", my_ID, n);
    if (n < 2) {
        PRINTF("r%d In fibEdt(%d) -- done (sat %lx)\n", my_ID, n, inDep);
        ocrEventSatisfy(inDep, depv[0].guid);
        return NULL_GUID;
    }
    PRINTF("r%d In fibEdt(%d) -- spawning children\n", my_ID, n);

    /* create the completion EDT and pass it the in/out argument as a dependency */
    /* create the EDT with the done_event as the argument */
    {
        u64 paramv[] = {(u64)inDep, my_ID};

        ocrGuid_t templateGuid;
        ocrEdtTemplateCreate(&templateGuid, complete, 2, 3);
        ocrEdtCreate(&comp, templateGuid, 2, paramv, 3, NULL, EDT_PROP_NONE,
                     NULL_GUID, NULL);
        ocrEdtTemplateDestroy(templateGuid);
    }
    PRINTF("r%d In fibEdt(%u) -- spawned complete EDT GUID 0x%llx\n", my_ID, n, (u64)comp);
    ocrAddDependence(depv[0].guid, comp, 2, DB_DEFAULT_MODE);

    /* create the events that the completion EDT will "wait" on */
    ocrEventCreate(&fibDone[0], OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
    ocrEventCreate(&fibDone[1], OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
    ocrAddDependence(fibDone[0], comp, 0, DB_DEFAULT_MODE);
    ocrAddDependence(fibDone[1], comp, 1, DB_DEFAULT_MODE);
    /* allocate the argument to pass to fib(n-1) */

    ocrDbCreate(&fibArg[0], (void**)&ptr, sizeof(u32), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    PRINTF("r%d In fibEdt(%u) -- created arg DB GUID 0x%llx\n", my_ID, n, fibArg[0]);
    *((u32*)ptr) = n-1;
    /* sched the EDT, passing the fibDone event as it's argument */
    {
        u64 paramv[] = {(u64)fibDone[0], my_ID};
        ocrGuid_t depv = fibArg[0];

        ocrGuid_t templateGuid;
        ocrEdtTemplateCreate(&templateGuid, fibEdt, 2, 1);
        ocrEdtCreate(&fib0, templateGuid, 2, paramv, 1, &depv, EDT_PROP_NONE,
                     NULL_GUID, NULL);
        ocrEdtTemplateDestroy(templateGuid);
    }

    PRINTF("r%d In fibEdt(%u) -- spawned first sub-part EDT GUID 0x%llx\n", my_ID, n, fib0);
    /* then do the exact same thing for n-2 */
    ocrDbCreate(&fibArg[1], (void**)&ptr, sizeof(u32), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    PRINTF("r%d In fibEdt(%u) -- created arg DB GUID 0x%llx\n", my_ID, n, fibArg[1]);
    *((u32*)ptr) = n-2;
    {
        u64 paramv[] = {(u64)fibDone[1], my_ID};
        ocrGuid_t depv = fibArg[1];

        ocrGuid_t templateGuid;
        ocrEdtTemplateCreate(&templateGuid, fibEdt, 2, 1);
        ocrEdtCreate(&fib1, templateGuid, 2, paramv, 1, &depv, EDT_PROP_NONE,
                     NULL_GUID, NULL);
        ocrEdtTemplateDestroy(templateGuid);
    }
    PRINTF("r%d In fibEdt(%u) -- spawned first sub-part EDT GUID 0x%llx\n", my_ID, n, fib1);

    PRINTF("r%d Returning from fibEdt(%u)\n", my_ID, n);
    return NULL_GUID;

}

u64 fib(u32 n)
{
    if(n<=0) return 0;
    if(n<=2) return 1;
    else return fib(n-1) + fib(n-2);
}

/* just define the main EDT function */
//ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
int main(int argc, char **argv)
{
    int my_ID;
    int Num_procs;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_ID);
    MPI_Comm_size(MPI_COMM_WORLD, &Num_procs);
    PRINTF("Starting main on rank %d\n", my_ID);

    u32 input;
    u32 myN;

    if (0 == my_ID)
        {
            if((argc != 2)) {
                PRINTF("Usage: fib <num>, defaulting to 10\n");
                input = 10;
            } else {
                input = atoi(argv[1]);
            }

            myN = input-1;
            u32 yourN = input-2;

            MPI_Send(&yourN, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        }
    else
        if (1 == my_ID)
            {
                MPI_Recv(&myN, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            }
        else  // only need 2
            {
                MPI_Finalize();
                return 0;
            }


    u64 correctAns = fib(myN);


    ocrGuid_t fibC, totallyDoneEvent, absFinalEdt, templateGuid;

    /* create a db for the results */
    ocrGuid_t fibArg;
    u32* res;

    PRINTF("Before 1st DB create\n");
    ocrDbCreate(&fibArg, (void**)&res, sizeof(u32), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    PRINTF("Got DB created\n");

    /* DB is in/out */
    *res = myN;
    /* and an event for when the results are finished */
    ocrEventCreate(&totallyDoneEvent, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);

    /* create the EDT with the done_event as the argument */
    {
        u64 paramv[] = {(u64)totallyDoneEvent, my_ID};
        ocrGuid_t depv = fibArg;

        ocrGuid_t templateGuid;
        ocrEdtTemplateCreate(&templateGuid, fibEdt, 2, 1);
        ocrEdtCreate(&fibC, templateGuid, 2, paramv, 1, &depv, EDT_PROP_NONE,
                     NULL_GUID, NULL);
        ocrEdtTemplateDestroy(templateGuid);
    }

    ocrGuid_t DB;
    void *myPtr;
    u64 dbSize;

    ocrLegacyBlockProgress(totallyDoneEvent, &DB, &myPtr, &dbSize, LEGACY_PROP_NONE);
    u32 myAns = *(u32*)myPtr;
    u32 ourAns;

    // get the results
    if (2 == Num_procs)
        {
            MPI_Reduce(&myAns, &ourAns, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        }
    else
        {
            // extra ranks, need to ignore them else reduce would hang
            // because they have all returned
            if (0 == my_ID)
                {
                    MPI_Recv(&ourAns, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    ourAns += myAns;
                }
            else
                {
                    MPI_Send(&myAns, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                }
        }

    if (0 == my_ID)
        {
            u64 correctAns = fib(input);

            if (correctAns == ourAns)
                {
                    PRINTF("\nFinal Answer Correct fib(%d) = %d\n", input, ourAns);
                }
            else
                {
                    PRINTF("\nFinal Answer **WRONG** fib(%d) = %d, should be %d\n", input, ourAns, correctAns);
                }
        }
    MPI_Finalize();

    return 0;
}
