/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
#include <ocr.h>

#define ENABLE_EXTENSION_LEGACY 1
#define ENABLE_EXTENSION_RTITF 1
#include <extensions/ocr-legacy.h>
#include <extensions/ocr-runtime-itf.h>
#include <mpi_ocr.h>

#define DEBUG_MPI 1

void ERROR(char *s)
{
    PRINTF("ERROR: %s; exiting\n",s); exit(1);
}

void WARNING(char *s)
{
    PRINTF("WARNING: %s\n",s);
}

// HIDE all the debugging printfs
#define PRINTF(a,...)

// fillArgv: create argc/argv for main() from OCR
static void fillArgv(u64 argc, char *argv[],void *argcArgvPtr)
{
    // argcArgvPtr points at the data in a datablock, and those addresses are in
    // the current address space. The getArgv takes what is probably an
    // offset relative to the beginning of the data block (to get to the
    // char string), and converts it into a pointer, which gets put into the
    // "native" C argv. Note: the char string does NOT need to be copied,
    // since the DB and the string are in this EDT's address space, and the
    // user's main() is going to run in the same address space, so references
    // through argv[k] will get the correct string.

    for (int i = 0; i < argc; i++)
        {
            argv[i] = getArgv(argcArgvPtr, i);
        }
}


// rankEdtFn: The EDT for one rank. sets up per-rank data, saves it in EDT
// Local Storage, and calls user's main()
//
// messageContext: events and data-block arrays for passing messages;
//   shared by all ranks
// rankContext: "MPI" info for this rank
// get argc/argv ready, and call user's main(argc, argv);

typedef struct {
    double a;
    int b;
} double_int;

static ocrGuid_t rankEdtFn(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
    const u64 rank = paramv[0];
    rankContext_t rankContext =
    { .rank = rank,
      .numRanks = paramv[1],
      .maxTag = paramv[2],
      .mpiInitialized = FALSE,
      .sizeOf = {0,  // not-a-datatype
                 sizeof(char), // MPI_CHAR
                 sizeof(signed char), // MPI_SIGNED_CHAR
                 sizeof(short), // MPI_SHORT
                 sizeof(int), // MPI_INT
                 sizeof(long), // MPI_LONG
                 sizeof(long long), // MPI_LONG_LONG
                 sizeof(long long int), // MPI_LONG_LONG_INT
                 sizeof(char), // MPI_BYTE
                 sizeof(unsigned char), // MPI_UNSIGNED_CHAR
                 sizeof(unsigned short), // MPI_UNSIGNED_SHORT
                 sizeof(unsigned), // MPI_UNSIGNED
                 sizeof(unsigned long), // MPI_UNSIGNED_LONG
                 sizeof(unsigned long long), // MPI_UNSIGNED_LONG_LONG
                 sizeof(float), // MPI_FLOAT
                 sizeof(double), // MPI_DOUBLE
                 sizeof(double_int)}, // MPI_DOUBLE_INT
      .maxComm = 1,
      .commArrayLen = 1,
      .communicators = NULL //
      // TBD real communicators
    };

    PRINTF("Starting rankEdtFn %d of %d\n",rank, rankContext.numRanks);

    const ocrEdtDep_t *argcArgv = &depv[0];

    messageContext_t messageContext =
    {
        .messageEvents = (ocrGuid_t *)(depv[1].ptr),
        .messageData = (ocrEdtDep_t *)(depv[2].ptr)
    };

    // Save the contexts in ELS so they can be retrieved when the user calls
    // back into this library. Note: this rankEdtFn does NOT return until the
    // users code completes, so this stack frame, and these *context structs
    // addresses remain valid during the whole program, e.g., when the MPI_*
    // functions are called.
    ocrElsUserSet(RANK_CONTEXT_SLOT, (ocrGuid_t) &rankContext);
    ocrElsUserSet(MESSAGE_CONTEXT_SLOT, (ocrGuid_t) &messageContext);

    // Turn the argcArgv datablock into a "native" C argc & argv
    const u64 argc = getArgc(argcArgv->ptr);
    char *argv[argc];
    fillArgv(argc, argv, argcArgv->ptr);

    PRINTF("rankEdtFn %d calling main()\n",rank);
    extern int  __mpiOcrMain(int, char **);
    __mpiOcrMain(argc, argv);

    PRINTF("rankEdtFn %d:  main() is finished, returning\n",rank);

    return NULL_GUID;
}


static ocrGuid_t endEdtFn(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    PRINTF("endEdt: Done\n");
    ocrShutdown();
    return NULL_GUID;
}


// parseAndShiftArgv extract numRanks "-r <int>" required,
// and maxTag "-t <int>" optional
// arguments from argcArgv DB, and shift argv to left by number of items
// extracted (reducing argc accordingly)
static void parseAndShiftArgv(u64 *argcArgv, u32 *numRanks, u32 *maxTag)
{
    *numRanks = 4;  // default
    *maxTag = 0;  // default
    int shift = 0;      // amount to shift argv

    int argc = getArgc(argcArgv);

#if 0
#undef PRINTF

    PRINTF("argcArgv before shift\n");

    for (int k = 0; k<argc; k++){
        PRINTF("argv[%d]= %s\n", k, getArgv(argcArgv, k));
    }
#endif

    if (argc < 3 || strcmp("-r", getArgv(argcArgv,1)))
        {
            char msg[150];
            sprintf(msg, "Command line should be 'exe -r <numRanks> [-t <maxTag>]\n"
                    "Continuing with %d ranks and max tag %d", *numRanks,
                    *maxTag);
            WARNING(msg);
            return;
        }
    // should check that is digits
    *numRanks = atoi(getArgv(argcArgv, 2));
    shift += 2;

    if (argc >= 5 && !strcmp("-t", getArgv(argcArgv, 3)))
        {
            // should check that is digits
            *maxTag = atoi(getArgv(argcArgv, 4));
            shift += 2;
        }

    // do the shift
    // argcArgv[0] is argc
    // argcArgv[1..argc] "ptrs" in argv
    // copy argArgv[2+skip .. argc] to argcArgv[2..argc-shift]
    // Start at 2 because 1 has the .exe name

    argcArgv[0] = argc -= shift;       // this is argc
    for (int i = 1; i < argc; i++){
        argcArgv[1 + i] = argcArgv[1 + shift + i];
    }
#if 0
    PRINTF("\nargcArgv after shift\n");

    for (int k = 0; k<argc; k++){
        PRINTF("argv[%d]= %s\n", k, getArgv(argcArgv, k));
    }
#define PRINTF(a,...)
#endif
}


static ocrGuid_t createTaggedEvents(u32 ranks, u32 maxTag)
{
    // Tagging is NYI in OCR

    return NULL_GUID;
}

// createMessageEventsAndData: until tagging is working, need to use array
// of event guids, and array of datablock ocrEdtDep_t's. Create and
// initialize these arrays, which will be shared by all ranks.
static void createMessageEventsAndData(ocrGuid_t *messageEventsDB,
                       ocrGuid_t *messageDataDB,
                       const u32 numRanks,const u32 maxTags)
{
    // Creates and initializes:
    //   events  messageEventsDB [numRanks, numRanks, maxTags+1];
    //   struct{guid, ptr}  messageDataDB [numRanks, numRanks, maxTags+1];

    const u32 numElements = numRanks * numRanks * (maxTags + 1);
    ocrGuid_t *events;
    ocrEdtDep_t *data ;

    ocrDbCreate(messageEventsDB, (void*)&events,
                numElements * sizeof (ocrGuid_t), DB_PROP_NONE, NULL_GUID, NO_ALLOC);

    ocrDbCreate(messageDataDB, (void*)&data,
                numElements * sizeof (ocrEdtDep_t), DB_PROP_NONE, NULL_GUID, NO_ALLOC);


    for (int i = 0; i < numElements; i++)
        {
            events[i] = NULL_GUID;
            data[i].guid = NULL_GUID;
            data[i].ptr = NULL;
        }
}

// mainEdtHelperFn: a FINISH edt that does most of the work of starting the
// rankEdts.
// 1. gets numRanks and maxTag from depv[0]
// [2. creates the tagged events : TBD when OCR implements feature]
// 2. Alternative: creates parallel arrays of event guids and datablock
// ocrEdtDep_ts
// 3. Create and start the rankEdts. They have the same paramv and depv, except for
// their rank (paramv[0]).
// 4. Wait until they all "finish", and then return to let endEdt close up shop.
static ocrGuid_t mainEdtHelperFn(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    PRINTF("Starting mainEdtHelper\n");

    const ocrEdtDep_t *argcArgvDB = &depv[0];
    u64 *argcArgv = (u64*)(argcArgvDB->ptr);
    u32 newArgc = 0; // temp hack
    char ** newArgv = NULL;
    u32 numRanks = 0;
    u32 maxTag = 0;


    PRINTF("mainEdtHelper: Parse argv\n");
    parseAndShiftArgv(argcArgv, &numRanks, &maxTag);

    // CHECK that the number of threads in the ocr context is >= numRanks,
    // or might get hangs. [Don't know how to ask this]
    const u64 numWorkers = ocrNbWorkers();

    if (numRanks > numWorkers)
        {
            char msg[150];
            sprintf((char*)&msg, "numRanks %d exceeds number of OCR workers %d\n"
                    "Either use few ranks, or use a cfg with more workers.",
                    numRanks, numWorkers);
            ERROR(msg); // exits
        }

    PRINTF("mainEdtHelper: numRanks = %d; maxTag = %d\n", numRanks, maxTag);

    // create rankEDTs - but don't add depv until endEdt has had their output
    // events added as its depv; so a rank doesn't complete before endEdt is ready.

    PRINTF("mainEdtHelper: creating rank edts\n");fflush(stdout);

    ocrGuid_t rankEdtTemplate;
    ocrEdtTemplateCreate(&rankEdtTemplate, rankEdtFn, 4, 3);

#if DONT_USE_FINISH_EDT
    ocrGuid_t ranks[numRanks];     // needed so dependences can be added to
    // all of them after endEdt is created

    ocrGuid_t outputEvents[numRanks];  // will be depv for endEdt
#endif

    // Mapping from [sourceRank x destRank x maxTag+1] to a tagged event
    ocrGuid_t eventTagGuid = createTaggedEvents(numRanks, maxTag);

    // But, Until OCR tagging is implemented, we need the shared array of message
    // events in a DB. And until ocrLegacyProgressBlock() is implemented to
    // allow getting the pointer associated with a DB, we need a parallel
    // array of ocrEdtDep_t's from which to extract the .ptr (after
    // checking that the guid has the same value as the DB received from
    // ocrWait.

    ocrGuid_t messageEventsDB, messageDataDB;
    createMessageEventsAndData(&messageEventsDB, &messageDataDB, numRanks, maxTag);

    u64 rankParamv[] = {0, numRanks, maxTag, eventTagGuid};  //most params are the same
    ocrGuid_t rankDepv[] = {argcArgvDB->guid, messageEventsDB, messageDataDB };

    // create the rankEdts. since their deps are all satisfied, they can
    // start immediately.
    for (int rank = 0; rank<numRanks; rank++)
        {
            rankParamv[0] = rank;  // only param that changes
            ocrGuid_t rankEdt;

            ocrEdtCreate(& rankEdt, rankEdtTemplate, EDT_PARAM_DEF, rankParamv,
                         EDT_PARAM_DEF, rankDepv,  EDT_PROP_NONE, NO_ALLOC, NULL);
            PRINTF("  rank %d edt 0x%llx\n",rank, rankEdt);fflush(stdout);

    }

    ocrEdtTemplateDestroy(rankEdtTemplate);


#if DONT_USE_FINISH_EDT
    //Now that the endEdt has the outputEvent dependences, the dependences
    //for the ranks can be addes so they can start
    PRINTF("mainEdt: adding deps to rank edts\n");


    for (int rank = 0; rank<numRanks; rank++)
        {
            PRINTF("  rank %d edt 0x%llx\n",rank, ranks[rank]);

            // argcArgvDB: will only be read by ranks
            ocrAddDependence(argcArgvDB->guid, ranks[rank], 0, DB_MODE_RO);

            ocrAddDependence(messgeEventsDB, ranks[rank], 0, DB_MODE_ITW);

            ocrAddDependence(messageDataDB, ranks[rank], 0, DB_MODE_ITW);
        }
#endif
}


// mainEdt: make mainEdtHelper be EDT_PROP_FINISH, so that endEdt can just
// wait for mainEdtHelper to finish. It will not complete until all the
// ranks return. [Otherwise would have to make endEdt depend on output
// events of All of the rankEdts.]
//    mainEdtHelper does the startup work.
ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    PRINTF("Starting mainEdt\n");

    ocrGuid_t mainEdtHelperTemplate, mainEdtHelper, outputEvent;
    ocrEdtTemplateCreate(&mainEdtHelperTemplate, mainEdtHelperFn, paramc, 1);
    ocrGuid_t helperDepv[] = {depv[0].guid};
    ocrEdtCreate(&mainEdtHelper, mainEdtHelperTemplate, EDT_PARAM_DEF ,
         paramv, EDT_PARAM_DEF, helperDepv, EDT_PROP_FINISH,
         NULL_GUID, &outputEvent);

    ocrEdtTemplateDestroy(mainEdtHelperTemplate);

    // Now endEdt can be created with all ouputEvent of mainEdtHelper
    ocrGuid_t endEdtTemplate, endEdt;
    ocrGuid_t endDepv[] = {outputEvent};
    ocrEdtTemplateCreate(&endEdtTemplate, endEdtFn, 0, 1);
    ocrEdtCreate(&endEdt, endEdtTemplate, 0, NULL, EDT_PARAM_DEF, endDepv,
         EDT_PROP_NONE, NULL_GUID, NULL);

    ocrEdtTemplateDestroy(endEdtTemplate);

    PRINTF("mainEdt: finished\n");
    return NULL_GUID;
}

// This function's only purpose is to be called by MPI_Init in mpilite.c,
// which will cause mpi_ocr.o to be dragged into the linked
// executable. Otherwise it does not, and mainEdt is not defined when the
// OCR runtime starts up.

int __mpi_ocr_TRUE(void) {
    return TRUE;
}


