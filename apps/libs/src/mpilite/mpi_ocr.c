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

#if 1
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
    // copy argArgv[1+skip .. argc] to argcArgv[1..argc-shift]

    argcArgv[0] = argc -= shift;       // this is argc
    for (int i = 0; i < argc; i++){
        argcArgv[1 + i] = argcArgv[1 + shift + i];
    }

#if 1
    PRINTF("\nargcArgv after shift\n");

    for (int k = 0; k<argc; k++){
        PRINTF("argv[%d]= %s\n", k, getArgv(argcArgv, k));
    }
#endif

}
//#define PRINTF(a,...)

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

#if 0
/////////////////// Should be in mpilite.c
#include "mpi.h"
#include <malloc.h>

#undef PRINTF
#define MPI_WARNING(s,w) {PRINTF("WARNING: %s, returning" #w "\n",(s)); return (w);}


int MPI_Init(int *argc, char ***argv)
{
    getRankContext()->mpiInitialized = TRUE;
    return MPI_SUCCESS;
}

int MPI_Initialized(int *flag)
{
    *flag = getRankContext()->mpiInitialized;
    return MPI_SUCCESS;
}



int MPI_Finalize(void)
{
    getRankContext()->mpiInitialized = FALSE;
    return MPI_SUCCESS;
}

int MPI_Comm_rank(MPI_Comm comm, int *rank)
{
    *rank = getRankContext()->rank;
    return(MPI_SUCCESS);
}

int MPI_Comm_size(MPI_Comm comm, int *size)
{
    *size = getRankContext()->numRanks;
    return(MPI_SUCCESS);
}

static inline u64 index(int source, int dest, int tag, int numRanks, int maxTag)
{
    return ((numRanks*source + dest)*(maxTag+1) + tag);
}

static inline void initRequest(MPI_Request p, int op, int count, int datatype, int tag, int rank, int comm, void *buf)
{
    p->count = count;
    p->op = op;
    p->datatype = datatype;
    p->tag = tag;
    p->rank = rank;
    p->comm = comm;
    p->flag = 0;
    p->status = FFWD_MQ_INIT;
    p->buf = buf;
}

// Isend: fill out the request, but do nothing else. A later MPI_Test or
// MPI_Wait* will caus the send to occur
int MPI_Isend(void *buf, int count, MPI_Datatype datatype, int dest, int tag,
              MPI_Comm comm, MPI_Request *request)
{
    *request = (MPI_Request)malloc(sizeof(**request));
    initRequest(*request, OP_ISEND, count, datatype, tag, dest, comm, buf);

    return(MPI_SUCCESS);
}

// Do a send.
int MPI_Send (void *buf,int count, MPI_Datatype
          datatype, int dest, int tag, MPI_Comm comm)
{
    // TBD: use comm to determine real "rank"

    rankContextP_t rankContext = getRankContext();
    const u32 source = rankContext->rank;
    const u32 numRanks = rankContext->numRanks;
    const u32 maxTag = rankContext->maxTag;
    const u64 totalSize = count * rankContext->sizeOf[datatype];

    messageContextP_t messageContext = getMessageContext();

    // need volatile so while loop keeps loading each iteration
    volatile ocrGuid_t *eventP =
        &(messageContext->messageEvents[index(source, dest, tag, numRanks, maxTag)]);

    while (NULL_GUID != *eventP);  // wait till slot is not busy

    // OK, the location is free: time to create the event and DB
    ocrEventCreate((ocrGuid_t *)eventP, OCR_EVENT_STICKY_T, TRUE);

    // Have to put the DB into a parallel array, because ocrWait does not
    // yet return the ptr value, so recipient can't get to the data!!!
    // Thus it has to be accessed from here by the receiver, who can check
    // that the DB guid delivered by ocrWait == the guid stashed in the array.
    ocrEdtDep_t *dataP =
        &(messageContext->messageData[index(source, dest, tag, numRanks, maxTag)]);

    mpiOcrMessageP_t ptr;

    ocrDbCreate(&(dataP->guid), (void **)&ptr, totalSize + sizeof(mpiOcrMessage_t),
        DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    dataP->ptr = (void *)ptr;

    // fill message header
    ptr->header.count = count;
    ptr->header.datatype = datatype;
    ptr->header.source = source;
    ptr->header.dest = dest;
    ptr->header.tag = tag;
    ptr->header.comm = comm;
    ptr->header.totalSize = totalSize;

    // copy buf starting at .data
    memcpy(&(ptr->data), buf, totalSize);

    ocrDbRelease(dataP->guid); // make sure it's visible to receiver

    // OK, Send the DB. the Satisfy should also release it (?).
    ocrEventSatisfy(*eventP, dataP->guid);

    return MPI_SUCCESS;
}

// Irecv: fill out the request, but do nothing else. A later MPI_Test or
// MPI_Wait* will cause the recv to occur
int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source,
              int tag, MPI_Comm comm, MPI_Request *request)
{
    *request = (MPI_Request)malloc(sizeof(**request));
    initRequest(*request, OP_IRECV, count, datatype, tag, source, comm, buf);

    return MPI_SUCCESS;
}


int MPI_Recv (void *buf,int count, MPI_Datatype
          datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
{
    // TBD: use comm to determine real "rank"

    rankContextP_t rankContext = getRankContext();
    const u32 dest = rankContext->rank;
    const u32 numRanks = rankContext->numRanks;
    const u32 maxTag = rankContext->maxTag;
    const u64 totalSize = count * rankContext->sizeOf[datatype];

    if (MPI_ANY_SOURCE == source || MPI_ANY_TAG == tag)
        {
            MPI_WARNING("MPI_Recv does not yet support MPI_ANY_*", MPI_ERR_ARG);
        }

    messageContextP_t messageContext = getMessageContext();

    // need volatile so while loop keeps loading each iteration
    volatile ocrGuid_t *eventP =
        &(messageContext->messageEvents[index(source, dest, tag, numRanks, maxTag)]);

    while (NULL_GUID == *eventP);  // wait till slot has event guid

    // OK, the location is full: time to wait on the event
    ocrGuid_t DB = ocrWait(*(ocrGuid_t *)eventP);



    // The DB is in a parallel array, because ocrWait does not
    // yet return the ptr value, so recipient can't get to the data!!!
    // Thus it has to be accessed from here by the receiver, who can check
    // that the DB guid delivered by ocrWait == the guid stashed in the array.
    ocrEdtDep_t *dataP =
        &(messageContext->messageData[index(source, dest, tag, numRanks, maxTag)]);

    ocrGuid_t receivedGuid = dataP->guid;

    if (DB != receivedGuid)
        {
            char msg[100];
            sprintf((char *)&msg, "MPI_Recv: datablock from ocrWait %llx != datablock in array %llx ", DB, receivedGuid);
            MPI_WARNING(msg, MPI_ERR_INTERN);
        }

    mpiOcrMessageP_t ptr = (mpiOcrMessageP_t) dataP->ptr;

    // Destroy event and set array entry to NULL_GUID so sender can send
    // again.

    // OK< now we are finished with the ocrEdtDep_t in the data array, as
    // well as the event.
    // So we can delete the event, replace the array event entry with NULL_GUID
    // so the sender can send again, which also means the array data element is
    // available. Note: we are not done with the db, but we have extracted
    // the ptr and guid from the data array.

    ocrEventDestroy(*eventP);
    *eventP = NULL_GUID;

#if 0 //MPI_DEBUG
    // TBD: check message header components against the args.

    // checked explicitly below    ptr->header.count = count;
    ptr->header.datatype != datatype;
    ptr->header.source    != source;
    ptr->header.dest      != dest;
    ptr->header.tag       != tag;
    ptr->header.comm      != comm;
    // OK, checked by count comparison ptr->header.totalSize != totalSize;
#endif

    u64 sizeToCopy;

    // Make sure receiving buffer is big enough, else error
    if (count < ptr->header.count)
        {
            char msg[100];
            sprintf((char*)&msg, "MPI_Recv: count %d < message count %d ", count, ptr->header.count);
            MPI_WARNING(msg, MPI_ERR_TRUNCATE);
        }
    else if (totalSize < ptr->header.totalSize)
        {
            char msg[100];
            sprintf((char*)&msg, "MPI_Recv: buf size %d < message size %d ", totalSize,
                    ptr->header.totalSize);
            MPI_WARNING(msg, MPI_ERR_TRUNCATE);
        }
    else
        {
            // so message is <= buf size
            sizeToCopy = ptr->header.totalSize;
        }

    // copy into buf starting at .data
    memcpy(buf, &(ptr->data), sizeToCopy);

    if (status != MPI_STATUS_IGNORE)
        {
            // fill in status struct
            status->mq_status.count    = ptr->header.count;
            status->mq_status.datatype = ptr->header.datatype;
            status->mq_status.source   = ptr->header.source;
            status->mq_status.tag      = ptr->header.tag;
        }

    // OK, finished with DB, delete it
    ocrDbDestroy(receivedGuid);

    return MPI_SUCCESS;
}

int MPI_Wait(MPI_Request *request, MPI_Status *status)
{
    MPI_Request r;

    if (NULL == request || NULL == (r = *request))
        {
            MPI_WARNING("MPI_Wait: bad value for request", MPI_ERR_REQUEST);
        }

    int ret = MPI_SUCCESS;
    switch(r->op)
        {
        case OP_ISEND:
            {
                ret = MPI_Send(r->buf,r-> count,r-> datatype, r->rank, r->tag,
                               r->comm);
                break;
            }
        case OP_IRECV:
            {
                ret = MPI_Recv(r->buf,r-> count,r-> datatype, r->rank, r->tag, r->comm, status);
                break;
            }
        case OP_IPROBE:
            {
                free(r);
                r = MPI_REQUEST_NULL;

                // does a return
                MPI_WARNING("MPI_Wait: MPI_Probe NYI",MPI_ERR_INTERN);
            }
        default:
            {
                free(r);
                r = MPI_REQUEST_NULL;
                // does a return
                MPI_WARNING("MPI_Wait: unknown request operation type",MPI_ERR_REQUEST);

            }
        }

    free (r);
    r = MPI_REQUEST_NULL;
    return ret;
}

int MPI_Test(MPI_Request *request, int *flag, MPI_Status *status)
{
    *flag = TRUE;
    return MPI_Wait(request, status);
}

// NYI
int MPI_Barrier( MPI_Comm comm )
{
    return MPI_ERR_INTERN;
}

int MPI_Get_count(
                  MPI_Status *status,
                  MPI_Datatype datatype,
                  int *count )
{
    if (datatype != status->mq_status.datatype)
        {
            *count = MPI_UNDEFINED;
            MPI_WARNING("MPI_Get_count: datatype differs between status and datatype arguments", MPI_ERR_TYPE);
        }

    if (status == MPI_STATUS_IGNORE) {
        *count = MPI_UNDEFINED;
        MPI_WARNING("MPI_Get_count status MPI_STATUS_IGNORE?", MPI_ERR_ARG);
    }
    *count = status->mq_status.count;
    return MPI_SUCCESS;
}
#endif
