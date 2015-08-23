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
#include <mpi_ocr.h>  // must come before ocr.h, as it sets defines
#include <ocr.h>

#include <extensions/ocr-legacy.h>
#include <extensions/ocr-labeling.h>

extern ocrGuid_t __ffwd_init(void * ffwd_add_ptr);

void ERROR(char *s)
{
    PRINTF("ERROR: %s; exiting\n",s); exit(1);
}

void WARNING(char *s)
{
    PRINTF("WARNING: %s\n",s);
}

// HIDE all the debugging printfs
#if !DEBUG_MPI
     #define PRINTF(a,...)
     #define fflush(a)
#endif

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
      .aggressiveNB = paramv[4],
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
#if EVENT_ARRAY
        .messageEvents = (ocrGuid_t *)(depv[1].ptr)
#else
        .messageEventMap = (ocrGuid_t) (paramv[3])
#endif
#ifdef DB_ARRAY
        , .messageData = (ocrEdtDep_t *)(depv[2].ptr)
#endif
    };

    // Save the contexts in ELS so they can be retrieved when the user calls
    // back into this library. Note: this rankEdtFn does NOT return until the
    // users code completes, so this stack frame, and these *context structs
    // addresses remain valid during the whole program, e.g., when the MPI_*
    // functions are called.
    ocrElsUserSet(RANK_CONTEXT_SLOT, (ocrGuid_t) &rankContext);
    ocrElsUserSet(MESSAGE_CONTEXT_SLOT, (ocrGuid_t) &messageContext);

    // globals2db support
    void * ffwd_addr_p=NULL;
    ocrGuid_t ffwd_db_guid = NULL_GUID;
    ffwd_db_guid = __ffwd_init(&ffwd_addr_p);
    globalDBContext_t globalDBContext =
        {
            .dbGuid = ffwd_db_guid,
            .addrPtr = ffwd_addr_p
        };
    ocrElsUserSet(GLOBAL_DB_SLOT, (ocrGuid_t)&globalDBContext);


    // Turn the argcArgv datablock into a "native" C argc & argv
    const u64 argc = getArgc(argcArgv->ptr);
    char *argv[argc];
    fillArgv(argc, argv, argcArgv->ptr);

    PRINTF("rankEdtFn %d calling main()\n",rank);

    extern int  __mpiOcrMain(int, char **);
    __mpiOcrMain(argc, argv);

    if (0 == rank) {
        if (NULL_GUID != argcArgv->guid) {
            ocrDbDestroy(argcArgv->guid);
        }

        if (NULL_GUID != ffwd_db_guid) {
            ocrDbDestroy(ffwd_db_guid);
        }
    }

    PRINTF("rankEdtFn %d:  main() is finished, returning\n",rank);

    return NULL_GUID;
}


static ocrGuid_t endEdtFn(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    PRINTF("endEdt: Done\n");
    ocrShutdown();
    return NULL_GUID;
}


// parseAndShiftArgv extract optional aggressive Non-Blocking "-a" (default
// conservative; then numRanks "-r <int>" optional - otherwise use NbWorkers
// and maxTag "-t <int>" optional; else use 0
//     exe [-r <int>][-t <int>] <args for program>
// arguments from argcArgv DB, and shift argv to left by number of items
// extracted (reducing argc accordingly)
static void parseAndShiftArgv(u64 *argcArgv, u32 *numRanks, u32 *maxTag, bool *aggressiveNB )
{
    *numRanks = 4;  // default
    *maxTag = 0;  // default
    *aggressiveNB = FALSE; // default non-aggressive
    int shift = 0;      // amount to shift argv to remove mpilite args

    int argc = getArgc(argcArgv);

#if 0
#undef PRINTF

    PRINTF("argcArgv before shift\n");

    for (int k = 0; k<argc; k++){
        PRINTF("argv[%d]= %s\n", k, getArgv(argcArgv, k));
    }
#endif

    if (argc > 1 && ! strcmp("-a", getArgv(argcArgv,1)))
#if EVENT_ARRAY
        {
            // only supported for LABELing version of mpi-lite
            *aggressiveNB = FALSE;
            shift += 1;
            WARNING("MPI startup: '-a' Aggressive Non-Blocking not supported in this version\n");
        }
#else
        {
            *aggressiveNB = TRUE;
            shift += 1;
        }
#endif

    if (argc < (3 + shift) || strcmp("-r", getArgv(argcArgv,1 + shift)))
        {
            char msg[150];
            *numRanks = ocrNbWorkers();
            sprintf(msg, "MPI startup: numRanks not specified, using %d \n"
                    , *numRanks);
            WARNING(msg);
        }
    else
        {
            // should check that is digits
            *numRanks = atoi(getArgv(argcArgv, 2 + shift));
            shift += 2;
        }

    // "-t" may be specified without "-a" and/or "-r", so have to look in position 1,2 or
    // 3,4
    if (argc >= (3+shift) && !strcmp("-t", getArgv(argcArgv, (1+shift))))
        {
            // should check that is digits
            *maxTag = atoi(getArgv(argcArgv, (2+shift)));
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

static ocrGuid_t messageEventMapFunc(ocrGuid_t startGuid, u64 skipGuid,
                                     s64* params, s64* tuple)
{
    const s64 dim0=params[0], dim1=params[1], dim2=params[2];
    const s64 i0=tuple[0], i1=tuple[1], i2=tuple[2];
    // bounds check
    for(int i=0; i<3; i++) {
        if (tuple[i]<0 || tuple[i] >= params[i])
            ERROR("messageEventMapFunc: index out of range");
    }

    s64 index = ((dim1*i0 + i1)*dim2 + i2);
    return startGuid + skipGuid * index;
}

static ocrGuid_t createTaggedEvents(u32 numRanks, u32 maxTag)
{
#if EVENT_ARRAY
    // Tagging is NYI in OCR
    return NULL_GUID;
#else
    ocrGuid_t messageEventMap;
    s64 dimensions[] = {numRanks, numRanks, maxTag+1};

    ocrGuidMapCreate(&messageEventMap, 3, messageEventMapFunc, dimensions,
                     numRanks*numRanks*(maxTag+1), GUID_USER_EVENT_STICKY);
    return messageEventMap;
#endif
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

#if EVENT_ARRAY
    const u32 numElements = numRanks * numRanks * (maxTags + 1);
    ocrGuid_t *events;

    ocrDbCreate(messageEventsDB, (void*)&events,
                numElements * sizeof (ocrGuid_t), DB_PROP_NONE, NULL_GUID, NO_ALLOC);

#ifdef DB_ARRAY
    ocrEdtDep_t *data ;
    ocrDbCreate(messageDataDB, (void*)&data,
                numElements * sizeof (ocrEdtDep_t), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
#endif

    for (int i = 0; i < numElements; i++)
        {
            events[i] = NULL_GUID;
#ifdef DB_ARRAY
            data[i].guid = NULL_GUID;
            data[i].ptr = NULL;
#endif
        }
#endif  // EVENT_ARRAY
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
    bool aggressiveNB = FALSE;

    PRINTF("mainEdtHelper: Parse argv\n");
    parseAndShiftArgv(argcArgv, &numRanks, &maxTag, &aggressiveNB);

    // Don't need to touch argcArgv any more, so release it
    ocrDbRelease(argcArgvDB->guid);


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

    PRINTF("mainEdtHelper: numRanks = %d; maxTag = %d, aggressiveNB = %d\n", numRanks, maxTag, aggressiveNB);

    // create rankEDTs - but don't add depv until endEdt has had their output
    // events added as its depv; so a rank doesn't complete before endEdt is ready.

    PRINTF("mainEdtHelper: creating rank edts\n");fflush(stdout);

    ocrGuid_t rankEdtTemplate;
    ocrEdtTemplateCreate(&rankEdtTemplate, rankEdtFn, 5, 3);

#if DONT_USE_FINISH_EDT
    ocrGuid_t ranks[numRanks];     // needed so dependences can be added to
    // all of them after endEdt is created

    ocrGuid_t outputEvents[numRanks];  // will be depv for endEdt
#endif

    // Mapping from [sourceRank x destRank x maxTag+1] to a tagged event
    ocrGuid_t messageEventMap = createTaggedEvents(numRanks, maxTag);

    // But, Until OCR tagging is implemented, we need the shared array of message
    // events in a DB. And until ocrLegacyProgressBlock() is implemented to
    // allow getting the pointer associated with a DB, we need a parallel
    // array of ocrEdtDep_t's from which to extract the .ptr (after
    // checking that the guid has the same value as the DB received from
    // ocrWait.

    ocrGuid_t messageEventsDB=NULL_GUID, messageDataDB=NULL_GUID;
    createMessageEventsAndData(&messageEventsDB, &messageDataDB, numRanks, maxTag);

    u64 rankParamv[] = {0, numRanks, maxTag, messageEventMap, aggressiveNB};  //most params are the same
    //    ocrGuid_t rankDepv[] = {UNINITIALIZED_GUID, messageEventsDB,
    //    messageDataDB };
    ocrGuid_t rankDepv[] = {argcArgvDB->guid, messageEventsDB, messageDataDB };

    // create the rankEdts. Add argcArgv dep explicitly so argcArgv can be made
    // CONST. When LABELing is used, the other two are NULL_GUID, so
    // rankEdts in different Policy Domains (e.g., in x86-mpi) can have
    // copies of the argcArgv DB; instead of taking turns if it were RW (which would not work since
    // a rankEdt is long lived, and never releases the DB so only 1 PD would
    // "own" argcArgv).
    // Since the rankEdts' deps are all satisfied, they can start immediately.
    for (int rank = 0; rank<numRanks; rank++)
        {
            rankParamv[0] = rank;  // only param that changes
            ocrGuid_t rankEdt;

#if 0
            ocrEdtCreate(& rankEdt, rankEdtTemplate, EDT_PARAM_DEF, rankParamv,
                         EDT_PARAM_DEF, rankDepv,  EDT_PROP_NONE, NO_ALLOC, NULL);

#endif
            // having trouble getting argcArgvDB added as CONST - hangs: bug
            // 664
#if 1
            ocrEdtCreate(& rankEdt, rankEdtTemplate, EDT_PARAM_DEF, rankParamv,
                         EDT_PARAM_DEF, NULL,  EDT_PROP_NONE, NO_ALLOC, NULL);

            // argcArgvDB: will only be read by ranks
            ocrAddDependence(argcArgvDB->guid, rankEdt, 0, DB_MODE_RO /* CONST hangs */);
            ocrAddDependence(messageEventsDB, rankEdt, 1, DB_MODE_RW);
            ocrAddDependence(messageDataDB, rankEdt, 2, DB_MODE_RW);
#endif

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

            ocrAddDependence(messgeEventsDB, ranks[rank], 0, DB_MODE_RW);

            ocrAddDependence(messageDataDB, ranks[rank], 0, DB_MODE_RW);
        }
#endif

    PRINTF("mainEdtHelper: finished\n");fflush(stdout);

    return NULL_GUID;
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
    ocrGuid_t argcArgv = depv[0].guid;
    ocrDbRelease(argcArgv);

    ocrEdtCreate(&mainEdtHelper, mainEdtHelperTemplate, EDT_PARAM_DEF ,
         paramv, EDT_PARAM_DEF, NULL, EDT_PROP_FINISH,
         NULL_GUID, &outputEvent);

    ocrEdtTemplateDestroy(mainEdtHelperTemplate);

    // Now endEdt can be created with all ouputEvent of mainEdtHelper
    ocrGuid_t endEdtTemplate, endEdt;
    ocrGuid_t endDepv[] = {outputEvent};
    ocrEdtTemplateCreate(&endEdtTemplate, endEdtFn, 0, 1);
    ocrEdtCreate(&endEdt, endEdtTemplate, 0, NULL, EDT_PARAM_DEF, endDepv,
         EDT_PROP_NONE, NULL_GUID, NULL);

    // Now that endEdt is started, we can fire up mainEdtHelper by
    // satisfying it's dep. (Otherwise it could finish before endEdt gets
    // started)
    ocrAddDependence(argcArgv, mainEdtHelper, 0, DB_MODE_RW);

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


ocrGuid_t __getGlobalDBGuid()
{
    globalDBContextP_t globalDBContext = (globalDBContextP_t)(ocrElsUserGet(GLOBAL_DB_SLOT));

    return globalDBContext->dbGuid;
}

u64 * __getGlobalDBAddr()
{
    globalDBContextP_t globalDBContext = (globalDBContextP_t)(ocrElsUserGet(GLOBAL_DB_SLOT));

    return globalDBContext->addrPtr;
}



// check the results from an OCR API call.
void __ocrCheckStatus(u8 status, char * functionName)
{
    switch (status)
    {
      case OCR_ENXIO:
        printf("%s: Error: Affinity is invalid.\n", functionName);
        exit(1);
        break;

      case OCR_ENOMEM:
        printf("%s: Error: Allocation failed because of insufficent memory.\n", functionName);
        exit(1);
        break;

      case OCR_EINVAL:
        printf("%s: Error: Invalid Arguments.\n", functionName);
        exit(1);
        break;

      case OCR_EBUSY:
        printf("%s: Error: The agent that is needed to process this request is busy.\n", functionName);
        exit(1);
        break;

      case OCR_EPERM:
        printf("%s: Error: Trying to allocate a restricted area of memory.\n", functionName);
        exit(1);
        break;

      default:
        ;  //  it is OK
    }
}
