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
#include "mpi.h"
#include <malloc.h>

// Note: can't hide ocrPrintf with an empty macro or these following
// warn/error message macros won't print anything. They need to be macros
// so they can execute a return.
#define MPI_INFO(s,w) {ocrPrintf("WARNING: %s, will return " #w "\n",(s));}
#define MPI_WARNING(s,w) {ocrPrintf("WARNING: %s, returning " #w "\n",(s)); return (w);}
#define MPI_ERROR(s) {ocrPrintf("ERROR: %s; exiting\n",s); exit(1);}

#define CHECK_RANGE(r,max,fn,kind)  if(r < 0 || r >= max) \
{\
    char msg[100];\
    sprintf(msg, "%s: %s %d is out of range [0..%d)",fn, kind, r,max);\
    MPI_WARNING(msg, MPI_ERR_ARG);\
}\

static void init_group_and_comm(void)
{
    rankContextP_t rankContext = getRankContext();
    const u32 numRanks = rankContext->numRanks;

    MPI_Comm mpi_comm_world = (MPI_Comm)malloc(sizeof(_MPI_Comm));
    if ( ! mpi_comm_world) {
        MPI_ERROR("Error: unable to allocate memory.\n");
    }
    MPI_Comm mpi_comm_self = (MPI_Comm)malloc(sizeof(_MPI_Comm));
    if ( ! mpi_comm_self) {
        MPI_ERROR("Error: unable to allocate memory.\n");
    }
    MPI_Group mpi_group_world = (MPI_Group)malloc(sizeof(_MPI_Group));
    if ( ! mpi_group_world) {
        MPI_ERROR("Error: unable to allocate memory.\n");
    }
    MPI_Group mpi_group_self  = (MPI_Group)malloc(sizeof(_MPI_Group));
    if ( ! mpi_group_self) {
        MPI_ERROR("Error: unable to allocate memory.\n");
    }

    mpi_group_world->gpid_array = (int *)malloc(numRanks * sizeof(int));
    if ( ! mpi_group_world->gpid_array) {
        MPI_ERROR("Error: unable to allocate memory.\n");
    }

    int i;
    for(i=0; i<numRanks; i++)
    {
        mpi_group_world->gpid_array[i] = i;
    }
    mpi_group_world->size = numRanks;
    mpi_group_world->gpid = rankContext->rank;  // unique pid
    mpi_group_world->rank = rankContext->rank;  // group-dependent pid
    mpi_group_self->size = 1;
    mpi_group_self->gpid = rankContext->rank;   // unique pid
    mpi_group_self->rank = rankContext->rank;   // group dependent pid

    mpi_comm_world->group = mpi_group_world;
    mpi_comm_self->group = mpi_group_self;

    rankContext->commArrayLen=2;
    rankContext->communicators = (MPI_Comm *)malloc(rankContext->commArrayLen*sizeof(MPI_Comm *));
    if ( ! rankContext->communicators ) {
        MPI_ERROR("Error: unable to allocate memory.\n");
    }
    rankContext->communicators[0] = (MPI_Comm)mpi_comm_world;
    rankContext->communicators[1] = (MPI_Comm)mpi_comm_self;
}


// This routine returns a pointer to a communicator.
// MPI_COMM_WORLD and MPI_COMM_SELF are just bit patterns.
// Return the real pointer to the communicator.
//
static MPI_Comm getComm(MPI_Comm comm)
{
    rankContextP_t rankContext = getRankContext();
    if (comm == MPI_COMM_WORLD)
        return rankContext->communicators[0];
    if (comm == MPI_COMM_SELF)
        return rankContext->communicators[1];
    return comm;
}


//static inline int gpid_to_rank(MPI_Comm comm, int gpid)
static int gpid_to_rank(MPI_Comm comm, int gpid)
{
	int i;
	for(i=0;i<comm->group->size;i++) {
		if (comm->group->gpid_array[i] == gpid)
			return i;
	}
	MPI_ERROR("BUG? no such gpid in this comm\n");
}


// The global PID is the physical rank number.
// The index in the array is the group rank number.
//static inline int  rank_to_gpid(MPI_Comm comm, int rank)
static int  rank_to_gpid(MPI_Comm comm, int rank)
{
    if (rank == MPI_ANY_SOURCE)
        return MPI_ANY_SOURCE;
    assert(rank >= 0);
    return comm->group->gpid_array[rank];
}

#if 0
static inline int  _rank_to_gpid(MPI_Group group, int rank)
{
    if (rank == MPI_ANY_SOURCE)
        return MPI_ANY_SOURCE;
    assert(rank >= 0);
    return group->gpid_array[rank];
}
#endif



// Have to make sure to drag in mainEdt else mpi_ocr.o does not get
// included in the linked object. MPI_Init must be called by all MPI
// programs, and it uses __mpi_ocr_TRUE from mpi_ocr.o, which will drag in
// the .o file.

int MPI_Init(int *argc, char ***argv)
{
    getRankContext()->mpiInitialized = __mpi_ocr_TRUE();
    init_group_and_comm();
    return MPI_SUCCESS;
}

int MPI_Init_thread( int *argc, char ***argv, int required, int *provided )
{
    int ret = MPI_SUCCESS;

    *provided = MPI_THREAD_SINGLE;
    if (required > *provided) {
        MPI_WARNING("INFO MPI_Init_thread required higher threading model.\n", MPI_ERR_ARG);
    }
    MPI_Init(argc, argv);
    //A threaded MPI program that does not call MPI_Init_thread is an incorrect program
    return ret;
}

int MPI_Initialized(int *flag)
{
    *flag = getRankContext()->mpiInitialized;
    return MPI_SUCCESS;
}

int MPI_Finalize(void)
{
#if DEBUG_MPI    // Only needed for debugging....
    rankContextP_t rankContext = getRankContext();
    const u32 rank = rankContext->rank;
    ocrPrintf("MPI_Finalize: rank #%d: Finalized!\n", rank);
#endif    // end of debugging

    getRankContext()->mpiInitialized = FALSE;
    return MPI_SUCCESS;
}

u64 guidIndex(int source, int dest, int tag)
{
    rankContextP_t rankContext = getRankContext();
    const u32 numRanks = rankContext->numRanks;
    const u32 maxTag = rankContext->maxTag;
    return ((numRanks*source + dest)*(maxTag+1) + tag);
}

static char * get_op_string(MPI_Request req)
{
    if (req == MPI_REQUEST_NULL)
        return "null";

    switch(req->op)
    {
      case opIsend:
        return "isend";
      case opIrecv:
        return "irecv";
      default:
        return "unknown";
    }
}


static inline void initRequest(MPI_Request p, nonBlockingOp op, int count, int
                               datatype, int tag, int rank, MPI_Comm comm, void
                               *buf, bool done)
{
    p->count = count;
    p->op = op;
    p->datatype = datatype;
    p->tag = tag;
    p->rank = rank;
    p->comm = comm;
    p->flag = 0;
    p->status = (done ? FFWD_MQ_DONE : FFWD_MQ_INIT);
    p->buf = buf;
}

#if EVENT_ARRAY

// Isend: fill out the request, but do nothing else. A later MPI_Test or
// MPI_Wait* will caus the send to occur
int MPI_Isend(void *buf, int count, MPI_Datatype datatype, int dest, int tag,
              MPI_Comm comm, MPI_Request *request)
{
    comm=getComm(comm);
    //dest = rank_to_gpid(comm, dest); //gail

#if DEBUG_MPI    // Only needed for debugging....
    rankContextP_t rankContext = getRankContext();
    const u32 source = rankContext->rank;
    const u32 numRanks = rankContext->numRanks;
    const u32 maxTag = rankContext->maxTag;
    ocrPrintf("MPI_Isend: rank #%d: Sending on index %d\n", source,
           guidIndex(source, dest, tag));
#endif    // end of debugging

    *request = (MPI_Request)malloc(sizeof(**request));
    initRequest(*request, opIsend, count, datatype, tag, dest, comm, buf, FALSE);

    return(MPI_SUCCESS);
}

// Do a send.
int MPI_Send(void *buf,int count, MPI_Datatype
              datatype, int dest, int tag, MPI_Comm comm)
{
    // Need the original rank (gpid) for communication.
    comm=getComm(comm);
    dest = rank_to_gpid(comm, dest);

    rankContextP_t rankContext = getRankContext();
    const u32 source = rankContext->rank;
    const u32 numRanks = rankContext->numRanks;
    const u32 maxTag = rankContext->maxTag;
    const u64 totalSize = count * rankContext->sizeOf[datatype];

    //ocrPrintf("MPI_Send: rank #%d: Sending on index %d\n", source, guidIndex(source, dest, tag));

    CHECK_RANGE(dest, numRanks, "MPI_Send", "dest");
    CHECK_RANGE(tag, maxTag+1, "MPI_Send", "tag");


    messageContextP_t messageContext = getMessageContext();

    // need volatile so while loop keeps loading each iteration
    volatile ocrGuid_t *eventP =
        &(messageContext->messageEvents[guidIndex(source, dest, tag)]);

    while (!ocrGuidIsNull(*eventP));  // wait till slot is not busy

    // OK, the location is free: time to create the event and DB
    ocrEventCreate((ocrGuid_t *)eventP, OCR_EVENT_STICKY_T, TRUE);


    mpiOcrMessageP_t ptr;
    ocrGuid_t DB;

    ocrDbCreate(&DB, (void **)&ptr, totalSize + sizeof(mpiOcrMessage_t),
                DB_PROP_NONE, NULL_HINT, NO_ALLOC);

#ifdef DB_ARRAY
    // Have to put the DB into a parallel array, because ocrWait does not
    // yet return the ptr value, so recipient can't get to the data!!!
    // Thus it has to be accessed from here by the receiver, who can check
    // that the DB guid delivered by ocrWait == the guid stashed in the array.
    ocrEdtDep_t *dataP =
        &(messageContext->messageData[guidIndex(source, dest, tag)]);

    dataP->ptr = (void *)ptr;
    dataP->data = DB;
#endif

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

    ocrDbRelease(DB); // make sure it's visible to receiver

    // OK, Send the DB. the Satisfy should also release it (?).
    ocrEventSatisfy(*eventP, DB);

    return MPI_SUCCESS;
}

// Irecv: fill out the request, but do nothing else. A later MPI_Test or
// MPI_Wait* will cause the recv to occur
int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source,
              int tag, MPI_Comm comm, MPI_Request *request)
{
    comm = getComm(comm);
    *request = (MPI_Request)malloc(sizeof(**request));
    initRequest(*request, opIrecv, count, datatype, tag, source, comm,
                buf, FALSE);

    return MPI_SUCCESS;
}


int MPI_Recv(void *buf,int count, MPI_Datatype
              datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
{
    int ret = MPI_SUCCESS;

    // Need the original rank for communication.
    comm=getComm(comm);
    source = rank_to_gpid(comm, source);

    rankContextP_t rankContext = getRankContext();
    const u32 dest = rankContext->rank;
    const u32 numRanks = rankContext->numRanks;
    const u32 maxTag = rankContext->maxTag;
    const u64 totalSize = count * rankContext->sizeOf[datatype];

    messageContextP_t messageContext = getMessageContext();
    ocrGuid_t *eventP;

    // This large IF stmt will keep looking for the appropriate event to
    // wait on, and when the if completes, eventP will point at the non
    // NULL_GUID
    if (!(MPI_ANY_SOURCE == source || MPI_ANY_TAG == tag)) {

        CHECK_RANGE(source, numRanks, "MPI_Recv", "source");
        CHECK_RANGE(tag, maxTag+1, "MPI_Recv", "tag");

        // need volatile so while loop keeps loading each iteration
        volatile ocrGuid_t *vEventP =
            &(messageContext->messageEvents[guidIndex(source, dest, tag)]);

        while (!ocrGuidIsNull(*vEventP));  // wait till slot has event guid

        // OK, the location is full: time to wait on the event
        eventP = (ocrGuid_t *)vEventP;

    } else { // *ANY* for source, tag, or both

        int done = FALSE;  // used by one of the while loops below
        if (MPI_ANY_SOURCE == source && MPI_ANY_TAG == tag){

            while(!done){
                for (u32 source = 0; source < numRanks; source++) {
                    for (u32 tag = 0; tag <= maxTag; tag++){
                        eventP =
                            &(messageContext->messageEvents[guidIndex(source, dest, tag)]);
                        if (!ocrGuidIsNull(*eventP) {
                            done = TRUE;
                            break;
                        }
                    }
                    if (done) {
                        break;
                    }
                }
            }  // end while
        } // end both use ANY
        else if (MPI_ANY_SOURCE == source) {

            CHECK_RANGE(tag, maxTag+1, "MPI_Recv", "tag");

            while(!done){
                for (u32 source = 0; source < numRanks; source++) {
                    eventP =
                        &(messageContext->messageEvents[guidIndex(source, dest, tag)]);
                    if (!ocrGuidIsNull(*eventP)){
                        done = TRUE;
                        break;
                    }
                }
            }

        } else { // ANY_TAG

            CHECK_RANGE(source, numRanks, "MPI_Recv", "source");

            while(!done){
                for (u32 tag = 0; tag <= maxTag; tag++){
                    eventP =
                        &(messageContext->messageEvents[guidIndex(source, dest, tag)]);
                    if (!ocrGuidIsNull(*eventP)){
                        done = TRUE;
                        break;
                    }
                }
            }

        }
    }
    // Here *eventP is not NULL_GUID, so something to wait on
    ocrGuid_t DB; //= ocrWait(*eventP);
    void *myPtr;
    ocrGuid_t newDB;
    u64 dbSize;

    ocrLegacyBlockProgress(*eventP, &DB, &myPtr, &dbSize, 0);

    //assert(DB==newDB);


#ifdef DB_ARRAY
    // The DB is in a parallel array, because ocrWait does not
    // yet return the ptr value, so recipient can't get to the data!!!
    // Thus it has to be accessed from here by the receiver, who can check
    // that the DB guid delivered by ocrWait == the guid stashed in the array.
    ocrEdtDep_t *dataP =
        &(messageContext->messageData[guidIndex(source, dest, tag)]);
    //    assert(dataP == myPtr);

    ocrGuid_t receivedGuid = dataP->guid;

    if (DB != receivedGuid)
        {
            char msg[100];
            sprintf((char *)&msg, "MPI_Recv: datablock from ocrWait %llx != datablock in array %llx ", DB, receivedGuid);
            MPI_WARNING(msg, MPI_ERR_INTERN);
        }

    mpiOcrMessageP_t ptr = (mpiOcrMessageP_t) dataP->ptr;

    assert(ptr == myPtr);
#else
    // ocrLegacyBlockProgress provides the db ptr.
    mpiOcrMessageP_t ptr = (mpiOcrMessageP_t) myPtr;

#endif
    // Destroy event and set array entry to NULL_GUID so sender can send
    // again.

    // OK< now we are finished with the ocrEdtDep_t in the data array, as
    // well as the event.
    // So we can delete the event, replace the array event entry with NULL_GUID
    // so the sender can send again, which also means the array data element is
    // available. Note: we are not done with the db, but we have extracted
    // the ptr and guid from the data array.

#if DEBUG_MPI
    ocrPrintf("Rank %d recving from %d tag %d DB %p on event %p\n",dest, source, tag, DB, *eventP);

#endif

    ocrEventDestroy(*eventP);
    *eventP = NULL_GUID;

    // Check message header components against the args. If they're wrong,
    // then the system got the wrong message!

    // checked explicitly below    ptr->header.count = count;
    // OK if user is dishonest about datatype as long as the recv size
    // isn't smaller than send size
    // ptr->header.datatype != datatype;

    assert(ptr->header.source == source || MPI_ANY_SOURCE == source);
    assert(ptr->header.dest == dest);
    assert(ptr->header.tag == tag || MPI_ANY_TAG == tag);

    // comm probably doesn't matter as long as src/dest match
    // ptr->header.comm      != comm;
    // OK, checked by count comparison ptr->header.totalSize != totalSize;

    u64 sizeToCopy = MIN(ptr->header.totalSize, totalSize);

    // Make sure receiving buffer is big enough, else warning with truncation
    if (count < ptr->header.count)
        {
            char msg[100];
            sprintf((char*)&msg, "MPI_Recv: count %d < message count %d ", count, ptr->header.count);
            MPI_INFO(msg, ret = MPI_ERR_TRUNCATE);
        }
    else if (totalSize < ptr->header.totalSize)
        {
            char msg[100];
            sprintf((char*)&msg, "MPI_Recv: buf size %d < message size %d ", totalSize,
                    ptr->header.totalSize);
            MPI_INFO(msg, ret = MPI_ERR_TRUNCATE);
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
    #if DESTROY_NEEDS_RELEASE
        ocrDbRelease(DB); // temporary till bug 879 fixed
    #endif

    ocrDbDestroy(DB);

    return ret;
}

#else

/********** Use OCR labeling *********/

// Isend: If doing aggressive non-blocking, Try to send. If can't do immediately,
//  fill out the request.
//  A later MPI_Test or MPI_Wait* will cause the send to occur
int MPI_Isend(void *buf, int count, MPI_Datatype datatype, int dest, int tag,
              MPI_Comm comm, MPI_Request *request)

{
    //  Need original rank for communication
    comm = getComm(comm);

    rankContextP_t rankContext = getRankContext();
    const bool aggressiveNB = rankContext->aggressiveNB;

    if (!aggressiveNB)
        {
#if DEBUG_MPI    // Only needed for debugging....
            const u32 source = rankContext->rank;
            ocrPrintf("MPI_Isend: rank #%d: dest:%d, tag:%d %d\n", source, dest, tag);
            fflush(stdout);
#endif    // end of debugging

            *request = (MPI_Request)malloc(sizeof(**request));
            initRequest(*request, opIsend, count, datatype, tag, dest, comm, buf, FALSE);

            return(MPI_SUCCESS);
        }

    const u32 source = rankContext->rank;
    const u32 numRanks = rankContext->numRanks;
    const u32 maxTag = rankContext->maxTag;
    const u64 totalSize = count * rankContext->sizeOf[datatype];
    bool done;

#if DEBUG_MPI    // Only needed for debugging....
    ocrPrintf("MPI_Isend_a: rank #%d: dest:%d, tag:%d %d\n", source, dest, tag);
    fflush(stdout);
#endif    // end of debugging

    CHECK_RANGE(dest, numRanks, "MPI_Isend", "dest");
    CHECK_RANGE(tag, maxTag+1, "MPI_Isend", "tag");

    int ret = mpiOcrTrySend(buf, count, datatype, source, dest, tag, comm,
                            totalSize, &done);

    *request = (MPI_Request)malloc(sizeof(**request));
    initRequest(*request, opIsend, count, datatype, tag, dest, comm, buf, done);

    return(ret);
}

// Do a send.
int MPI_Send(void *buf,int count, MPI_Datatype
              datatype, int dest, int tag, MPI_Comm comm)
{
    //  Need original rank for communication
    comm = getComm(comm);
    dest = rank_to_gpid(comm, dest);

    rankContextP_t rankContext = getRankContext();
    const u32 source = rankContext->rank;
    const u32 numRanks = rankContext->numRanks;
    const u32 maxTag = rankContext->maxTag;
    const u64 totalSize = count * rankContext->sizeOf[datatype];

#if DEBUG_MPI  // debug
    ocrPrintf("MPI_Send: rank #%d: Sending to %d tag:%d \n", source, dest, tag);
    fflush(stdout);
#endif

    CHECK_RANGE(dest, numRanks, "MPI_Send", "dest");
    CHECK_RANGE(tag, maxTag+1, "MPI_Send", "tag");

    return mpiOcrSend(buf, count, datatype, source, dest, tag, comm, totalSize);
}

// Irecv: If doing aggressive non-blocking, try doing a recv. Otherwise, fill out the request,
// but do nothing else. A later MPI_Test or
// MPI_Wait* will cause the recv to occur
int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source,
              int tag, MPI_Comm comm, MPI_Request *request)
{
    //  Need original rank for communication
    comm = getComm(comm);

    rankContextP_t rankContext = getRankContext();
    const u32 dest = rankContext->rank;
    const bool aggressiveNB = rankContext->aggressiveNB;
    bool done = FALSE;
    int ret = MPI_SUCCESS;

#if DEBUG_MPI    // Only needed for debugging....
    ocrPrintf("MPI_Irecv: rank #%d: source:%d, tag:%d %d\n", dest, source, tag);
    fflush(stdout);
#endif    // end of debugging

    if (aggressiveNB)
        {  //
           // Aggressive Non-Blocking: see if the Recv can be done via mpiTryRecv. If so,
           //
           // complete it. In either case, package up results in *request,
           // where done will be TRUE or FALSE, respectively.

            // This large IF stmt distinguishes the easy case of no wildcards,
            // versus wildcards.
            const u32 numRanks = rankContext->numRanks;
            const u32 maxTag = rankContext->maxTag;
            const u64 totalSize = count * rankContext->sizeOf[datatype];
            MPI_Status status;

            if (!(MPI_ANY_SOURCE == source || MPI_ANY_TAG == tag)) {

                CHECK_RANGE(source, numRanks, "MPI_Irecv", "source");
                CHECK_RANGE(tag, maxTag+1, "MPI_Irecv", "tag");

                ret = mpiOcrTryRecv(buf, count, datatype, source, dest, tag, comm,
                                 totalSize, &status, &done);

            } else { //
                // *ANY* for source, tag, or both. Try once for each
                // combination; stop if you find a
                // successful recv using mpiOcrTryRecv.
                //
                // Use one doubly nested loop for the 3 conditions, using xLb
                // and xUb as appropriate range for src and tag

                u32 srcLb, srcUb, tagLb, tagUb;

                if (MPI_ANY_SOURCE == source)
                    {
                        srcLb = 0;
                        srcUb = comm->group->size;
                    }
                else
                    {   // Real source value
                        CHECK_RANGE(source, numRanks, "MPI_Irecv", "source");
                        srcLb = source;
                        srcUb = source+1;
                    }

                if (MPI_ANY_TAG == tag)
                    {
                        tagLb = 0;
                        tagUb = maxTag+1;
                    }
                else
                    {   // real tag value
                        CHECK_RANGE(tag, (maxTag+1), "MPI_Irecv", "tag");
                        tagLb = tag;
                        tagUb = tag+1;
                    }

                for (u32 trySource = srcLb; trySource < srcUb; trySource++) {
                    for (u32 tryTag = tagLb; tryTag < tagUb; tryTag++){
                        ret = mpiOcrTryRecv(buf, count, datatype, trySource, dest,
                                            tryTag, comm, totalSize, &status, &done);
                        if (done) {
                            // Found a match! set source & tag to the
                            // values that worked, so they get filled into
                            // request properly
                            source = trySource;
                            tag = tryTag;
                            break;
                        }
                    } // end inner loop
                    if (done) {
                        break;
                    }
                } // end outer loop
            } // end Else one or both use ANY
        } // end if aggressiveNB
    else
        {
            // non-aggressive non-blocking - just fall out the bottom,
            // creating a request with all the args, and
            // done==FALSE. ret is already MPI_SUCCESS.
            // Operation will be completed by MPI_Wait or MPI_Test
        }

    *request = (MPI_Request)malloc(sizeof(**request));
    initRequest(*request, opIrecv, count, datatype, tag, source, comm, buf, done);

    return ret;
}


int MPI_Recv(void *buf,int count, MPI_Datatype
              datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
{
    int ret = MPI_SUCCESS;

    // Need original rank for communication
    comm=getComm(comm);
    source = rank_to_gpid(comm, source);

    rankContextP_t rankContext = getRankContext();
    const u32 dest = rankContext->rank;
    const u32 numRanks = rankContext->numRanks;
    const u32 maxTag = rankContext->maxTag;
    const u64 totalSize = count * rankContext->sizeOf[datatype];

#if DEBUG_MPI
    ocrPrintf("MPI_Recv: rank #%d: Recv from %d tag:%d\n",  dest, source, tag);
    fflush(stdout);
#endif

    // This large IF stmt distinguishes the easy case of no wildcards,
    // versus wildcards.
    if (!(MPI_ANY_SOURCE == source || MPI_ANY_TAG == tag)) {

        CHECK_RANGE(source, numRanks, "MPI_Recv", "source");
        CHECK_RANGE(tag, maxTag+1, "MPI_Recv", "tag");

        ret = mpiOcrRecv(buf, count, datatype, source, dest, tag, comm,
                         totalSize, status);

    } else {
        // *ANY* for source, tag, or both. Try once for each
        // combination; stop if you find a
        // successful recv using mpiOcrTryRecv; else go around the
        // while loop again, trying all combinations. Don't stop
        // till someone matches!
        //
        // Use one doubly nested loop for the 3 possible "*ANY*" conditions, using xLb
        // and xUb as appropriate range for src and tag

        u32 srcLb, srcUb, tagLb, tagUb;

        if (MPI_ANY_SOURCE == source)
            {
                srcLb = 0;
                srcUb = comm->group->size;
            }
        else
            {   // Real source value
                CHECK_RANGE(source, numRanks, "MPI_Recv", "source");
                srcLb = source;
                srcUb = source+1;
            }

        if (MPI_ANY_TAG == tag)
            {
                tagLb = 0;
                tagUb = maxTag+1;
            }
        else
            {   // real tag value
                CHECK_RANGE(tag, (maxTag+1), "MPI_Recv", "tag");
                tagLb = tag;
                tagUb = tag+1;
            }

        bool done = FALSE;
        while (!done)
            {
                // Note: it is OK to hide the source and tag args to
                // MPI_Recv with the loop args, because the correct value will be filled into
                // the status struct once a match is found.
                for (u32 trySource = srcLb; trySource < srcUb; trySource++) {
                    for (u32 tryTag = tagLb; tryTag < tagUb; tryTag++){
                        ret = mpiOcrTryRecv(buf, count, datatype, trySource, dest,
                                            tryTag, comm, totalSize, status, &done);
                        if (done) {
                            break;
                        }
                    } // end inner for loop
                    if (done) {
                        break;
                    }
                } // end outer for loop
            }  // end while(!done)

    } // end Else one or both use ANY

    // Here buf is filled and if status isn't IGNORE it has been filled
    // in. Also, ret is set.

    return ret;
}

#endif // if EVENT_ARRAY

int MPI_Sendrecv(void *sendbuf, int sendcount, MPI_Datatype sendtype,
                 int dest, int sendtag,
                 void *recvbuf, int recvcount, MPI_Datatype recvtype,
                 int source, int recvtag,
                 MPI_Comm comm, MPI_Status *status)
{
    int retSend = MPI_Send(sendbuf, sendcount, sendtype, dest, sendtag, comm);
    int retRecv = MPI_Recv(recvbuf, recvcount, recvtype, source, recvtag,
                           comm, status);
    if (MPI_SUCCESS == retSend){
        return retRecv;
    } else {
        return retSend;
    }

}

// Fill in status using info from a satisfied request
static void fillStatusFromRequest(MPI_Status *status, MPI_Request request)
{
    if (status != MPI_STATUS_IGNORE)
        {
            // fill in status struct
            status->mq_status.count    = request->count;
            status->mq_status.datatype = request->datatype;
            status->mq_status.source   = request->rank;
            status->mq_status.tag      = request->tag;
        }
}

int MPI_Wait(MPI_Request *request, MPI_Status *status)
{
    // for debugging
#if DEBUG_MPI
    rankContextP_t rankContext = getRankContext();
    const u32 rank = rankContext->rank;
#endif

    MPI_Request r;

    if (NULL == request || MPI_REQUEST_NULL == (r = *request))
        {
            //MPI_REQUEST_NULL is legitimate.
            //MPI_WARNING("MPI_Wait: bad value for request", MPI_ERR_REQUEST);
#if DEBUG_MPI
            ocrPrintf("MPI_Wait: rank #%d: MPI_NULL_REQUEST\n", rank);
#endif
            return MPI_SUCCESS;
        }

    const bool done = FFWD_MQ_DONE == r->status;

#if DEBUG_MPI
    ocrPrintf("MPI_Wait: rank #%d: request->(op=%s, done=%s)\n", rank, get_op_string(r),
           (done ? "yes" : "no"));
#endif

    int ret = MPI_SUCCESS;
    switch(r->op)
        {
        case opIsend:
            {
                // If wasn't done before, force it now. If was done,
                // nothing to do now. Note: status arg is left alone.
                if (!done){
                    // didn't happen at the isend, so force a send now
                    ret = MPI_Send(r->buf,r-> count, r-> datatype, r->rank, r->tag,
                                   r->comm);
                }
                break;
            }
        case opIrecv:
            {
                // If was done, just fill in status from request.
                // If wasn't done before, force it now.
                if (done){
                    // irecv was successful; transfer info from request to status
                    fillStatusFromRequest(status, r);
                }
                else {
                    // have to actually do the recv
                    ret = MPI_Recv(r->buf,r-> count, r-> datatype, r->rank, r->tag,
                                   r->comm, status);
                }
                break;
            }
        default:
            {
                free(r);
                *request = MPI_REQUEST_NULL;
                // does a return
                MPI_WARNING("MPI_Wait: unknown request operation type",MPI_ERR_REQUEST);
            }
        }

    free(r);
    *request = MPI_REQUEST_NULL;
    return ret;
}

int MPI_Waitall(int count, MPI_Request *array_of_requests,
    MPI_Status *array_of_statuses)
{
    u32 i;
#if DEBUG_MPI
    rankContextP_t rankContext = getRankContext();
    const u32 rank = rankContext->rank;

    ocrPrintf("MPI_Waitall: rank #%d: %d aor %p aos %p\n", rank, count,
           array_of_requests, array_of_statuses);
#endif

    if (array_of_statuses == MPI_STATUSES_IGNORE)
        for(i=0;i<count;i++) {
            MPI_Wait(&array_of_requests[i], MPI_STATUS_IGNORE);
        }
    else
        {
            // First try waiting only on the Sends, in case the Recvs need
            // the send to happen
            for(i=0;i<count;i++) {
                MPI_Request *r = &array_of_requests[i];
                if (MPI_REQUEST_NULL != *r && opIsend == (*r)->op)
                    {
                        MPI_Wait(r, &array_of_statuses[i]);
                    }
            }
            for(i=0;i<count;i++) {
                MPI_Wait(&array_of_requests[i], &array_of_statuses[i]);
            }
        }
    return MPI_SUCCESS;
}


// This is a marginal implementation of Waitany: it ALWAYS does a wait on
// the first non-null Send request; and when there are no Sends left, it
// then picks the first non-null Recv. This could cause hangs compared with
// a more flexible implementation. For example, run throught the array of
// requests and apply MPI_Test, and only do a Wait when Test says that the
// request is already satisfied.
//    However, unless mpilite is being used in -a "aggressive" mode, Isend
// and Irecv don't actually do anything, so NO requests are complete....

int MPI_Waitany(int count, MPI_Request *array_of_requests,
                int * index, MPI_Status *status)
{
    u32 i;
#if DEBUG_MPI
    rankContextP_t rankContext = getRankContext();
    const u32 rank = rankContext->rank;

    ocrPrintf("MPI_Waitany: rank #%d: %d aor %p s %p\n", rank, count,
           array_of_requests, status);
#endif

    // First try waiting only on the Sends, in case the Recvs need
    // the send to happen. Only wait for the first non-null send, if any
    // (Recall that Wait() will set the request to NULL.)
    for(i=0; i<count; i++) {
        MPI_Request *r = &array_of_requests[i];
        if (MPI_REQUEST_NULL != *r && opIsend == (*r)->op)
            {
                MPI_Wait(r, status);
                *index = i;
                return MPI_SUCCESS;
            }
    }

    // If we fall out of the previous loop, all Sends have ween waited for,
    // so the only things left (if any) are recvs
    for(i=0; i<count; i++) {
        MPI_Request *r = &array_of_requests[i];
        // Find first non-null request and wait on it.
        if (MPI_REQUEST_NULL != *r)
            {
                MPI_Wait(r, status);
                *index = i;
                return MPI_SUCCESS;
            }
    }

   // *** All requests were null, so set ret value of index to "Undefined"
    *index = MPI_UNDEFINED;

    return MPI_SUCCESS;
}


int MPI_Test(MPI_Request *request, int *flag, MPI_Status *status)
{
    rankContextP_t rankContext = getRankContext();
    const u32 rank = rankContext->rank;
    bool aggressiveNB = rankContext->aggressiveNB;

    if (!aggressiveNB)
        {
#if DEBUG_MPI
            ocrPrintf("MPI_Test: rank #%d: request->(op=%s)\n", rank, (NULL == request? "NULL" :
                   get_op_string(*request)));
#endif
            *flag = TRUE;
            return MPI_Wait(request, status);
        }

    // Aggressive Non-blocking. See if we can do the send or recv NOW.
    // If not, don't block.

    MPI_Request r, newR;

    if (NULL == request || MPI_REQUEST_NULL == (r = *request))
        {
            //MPI_REQUEST_NULL is legitimate. E.g., previous TEST succeeded.
#if DEBUG_MPI
            ocrPrintf("MPI_Test: rank #%d: MPI_NULL_REQUEST, set flag=TRUE\n", rank);
#endif

            *flag = TRUE;
            return MPI_SUCCESS;
        }


    bool done = (FFWD_MQ_DONE == r->status);

#if DEBUG_MPI
    ocrPrintf("MPI_Test: rank #%d: request->(op=%s, done=%s)\n", rank, get_op_string(r),
           (done ? "yes" : "no"));
#endif

    int ret = MPI_SUCCESS;
    switch(r->op)
        {
        case opIsend:
            {
                // If wasn't done before, try it again. If was done,
                // nothing to do now. Note: status arg is left alone.
                if (!(done)){
                    // didn't happen at the isend, so try an Isend again
                    ret = MPI_Isend(r->buf,r-> count, r-> datatype, r->rank, r->tag,
                                    r->comm, &newR);

                    // This gives us another "newer" filled in request. So
                    // free the old one, and assign newR to r
                    free(r);
                    r = newR;

                    // see if it's done now!
                    done = (FFWD_MQ_DONE == r->status);
                }
                break;
            }
        case opIrecv:
            {
                // If wasn't done before, try it again
                if (!(done)){
                    // didn't complete at the Irecv, so try another Irecv
                    ret = MPI_Irecv(r->buf, r->count, r->datatype, r->rank, r->tag,
                                    r->comm, &newR);

                    // This gives us another "newer" filled in request. So
                    // free the old one, and assign newR to r
                    free(r);
                    r = newR;

                    // see if it's done now!
                    done = (FFWD_MQ_DONE == r->status);
                }
                if (done){
                    // irecv was successful; or the original irecv was
                    // successful, so transfer info from request to status
                    fillStatusFromRequest(status, r);
                }
                break;
            }
        default:
            {
                free(r);
                *request = MPI_REQUEST_NULL;
                // does a return
                MPI_WARNING("MPI_Test: unknown request operation type", MPI_ERR_REQUEST);
            }
        }

    *flag = done;
    if(done)
        {
            free(r);
            *request = MPI_REQUEST_NULL;
        }
    else
        {
            *request = r;
        }

    return ret;
}

int MPI_Testall(int count, MPI_Request *array_of_requests, int *flag,
                MPI_Status *array_of_statuses)
{
    int i;
    int temp;
#ifdef DEBUG_MPI
    printf("testall: %d aor %p aos %p\n", count, array_of_requests, array_of_statuses);
#endif
    *flag = 1;  // True if all requests have completed; false otherwise (logical)
    if (array_of_statuses == MPI_STATUSES_IGNORE)
        for(i=0;i<count;i++) {
            MPI_Test(&array_of_requests[i], &temp, MPI_STATUS_IGNORE);
            *flag = *flag && temp;
        }
    else
        for(i=0;i<count;i++) {
            MPI_Test(&array_of_requests[i], &temp, &array_of_statuses[i]);
            *flag = *flag && temp;
        }
    return MPI_SUCCESS;
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



// can use other values; could even have as an argument
#ifndef ARITY
     #define ARITY 2
#endif

// Use a tree for collectives
// "root" of tree is at 0; 2nd level is 0*ARITY+1, 0*ARITY+2, .., 0*ARITY+(ARITY),
// which is {1,2,..,ARITY}. third level is 1*ARITY+1, 1*ARITY+2, ... which is
// {ARITY+1, ARITY+2, .. 2*ARITY} {2*ARITY+1, 2*ARITY+2 ..}..{.. ARITY*ARITY}

//  Normally this adjustment occurs inside the computation of parents (subtract 1)
// or children (start by adding 1, instead of starting at 0).
//
// The one complicating factor is that the tree computations expect to be rooted
// at zero. However, the user can specify a root value. So if the root is not
// zero, we have to "swap" the behavior of zero and root. But not only when they
// are the actor, but also when they are the source or dest of one of the other ranks.
// So there need to be explicit checks for this case.
//
// Another approach would be to ALWAYS use 0 as the virtual root, and
// exchange values with root (bcast: before; reduce: after) so the reight
// answer is gotten. Some possible complication with MPI_IN_PLACE..
//
// The other wrinkle is that the tree is not necessarily complete, so when you
// look toward the leaves, you need to check that the child's "rank" number you are
// expecting is < numRanks. Otherwise you need to behave accordingly. For bcast
// toward the leaves, you just don't send. For reduce, you don't receive from
// that child , AND
// you have to adjust the reduce operation by iterating fewer times.
// And leaves don't combine any values from other ranks, they just send their
// sendbuf to the parent.
//
// Two other wrinkles:
// 1. sendbuf may == MPI_IN_PLACE, which means resultbuf is both input and
// output, so need to get sendbuf to point to resultbuf. Note, this is
// supposed to only be for the ROOT, because other ranks don't use their
// resultbuf; but it's easy to handle it correctly if the user mis-uses this
// feature.
// 2. Since non-root doesn't need resultbuf, user may pass in NULL, or put in
// a pointer to data that SHOULD NOT BE WRITTEN ON! Therefore, never use
// resultbuf as a target for Recv-ing data from children (except for root).
//
// The algorithm works for any base for the number of descendents of each tree node:
// collective base, or ARITY.

#ifndef TREE_REDUCE
    #define TREE_REDUCE 1
#endif

#if  TREE_REDUCE

int MPI_Reduce (void *sendbuf, void *resultbuf, int count,
    MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
{
    // Need original rank # for communication
    comm = getComm(comm);

    rankContextP_t rankContext = getRankContext();
    const u32 rank = gpid_to_rank(comm, rankContext->rank);
    const u32 groupSize = comm->group->size;
    const u32 maxTag = rankContext->maxTag;
    const u64 totalSize = count * rankContext->sizeOf[datatype];

    if (root >= groupSize) {
        MPI_ERROR("MPI_Reduce: root >= group size? invalid rank\n");
    }

    // usually only do IN_PLACE for root, because resultbuf is not used in
    // non-root ranks
    if (sendbuf == MPI_IN_PLACE) {
        sendbuf = resultbuf;
    }
    else if (root == rank) {
        // Root needs to initialize resultbuf from sendbuf, because the
        // reduce operation will be "sendbuf op= <data from child>".
        //      But if
        // sendbuf is MPI_IN_PLACE, we already have the effect of the copy
        // from the assignment in the first part of the if.
        memcpy(resultbuf, sendbuf, totalSize);
    }

    if (1 == groupSize) {
        // If only 1 rank, then root == rank, and we just made sure
        // resultbuf[*] == sendbuf[*], so we're done!
#if DEBUG_MPI
        ocrPrintf("Reduce: root rank#%d returns value\n", rank);
        fflush(stdout);
#endif

        return MPI_SUCCESS;
    }

    // swap root and zero (and if root==0, vRank will be 0)
    const s32 vRank = (root == rank ? 0 : (0 == rank ? root : rank));

    // make vRank signed, so if vRank==0, this divide doesn't become
    // (uintMax / ARITY)=> huge positive number. If it is signed, it
    // results in 0.
    s32 vDest = (vRank - 1) / ARITY;

    // Have to set the real dest to the actual rank for the Send.
    vDest = ((root == vDest)? 0 : ((0 == vDest)? root : vDest));

    // If this is a leaf rank, there is nothing to recv from children,
    // so no computation needed. It's a leaf if its first
    // child is >= numRanks.
    // The value is still in the sendbuf, so send that and we're done.

    //if ((vRank * ARITY + 1) >= numRanks){
    if ((vRank * ARITY + 1) >= groupSize){
        MPI_Send(sendbuf, count, datatype, vDest, 0 /*tag*/, comm);

#if DEBUG_MPI
        ocrPrintf("Reduce: rank#%d leaf sends to %d\n", rank, vDest);
        fflush(stdout);
#endif

        return MPI_SUCCESS;
    }

#if DEBUG_MPI
    int srcs[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
#endif

    // make small buffers to receive childens's data, if too small will use
    // malloc later
    const int smallSize = 64;
    char buffer1[smallSize],buffer2[smallSize];
    void * buf1Ptr, * buf2Ptr;

    // Mark apologizes for using "p" and "recvbuf", but I stole the pages of
    // reduction computation code and did not want to risk introducing
    // errors by doing renamings. (e.g., we never mpi_recv into "recvbuf",
    // instead "receiver" is used.)
    void * p;        // source: right-hand side of reduction
    void * recvbuf;  // target: left-hand side of reduction
    void * receiver; // where the MPI_Recv will put its value


    if (totalSize <= smallSize){
        // we can use the stack buffers
        buf1Ptr = & buffer1;
        buf2Ptr = & buffer2;
    } else {
        // We'll need more space. We KNOW there is at least one child, so
        // we'll need buf1Ptr, but don't know
        // yet if we'll need buf2Ptr, so make it NULL so we'll know whether to
        // free it after the loop
        buf1Ptr = malloc(totalSize);
        buf2Ptr = NULL;
    }

    // Lots of pointer shuffling to minimize amount of copying needed,
    // and the amount of mallocing if the buffers are big (e.g.,
    // Tempest has 500k buffers)

    if (root == rank){
        // If we're at the root, resultbuf has to be the target, and buf1Ptr
        // will be the receiver and the reduce src. buf2Ptr will not be needed
        // This mapping will NOT change in the loop.
        receiver = p = buf1Ptr;
        recvbuf = resultbuf;
    } else {
        // Otherwise, avoid the extra copy that would have to be done to
        // sendbuf on the first iteration to make it suitable for being
        // the target of the reduction. Instead, make the first buffer
        // received from MPI_Recv be the target.
        //      This mapping WILL BE CHANGED on the second iteration, if there
        // is more than 1 child: can't receive into buf1Ptr because that's holding
        // the intermediate result; now we'll need buf2Ptr. See inside the loop
        p = sendbuf;
        recvbuf = receiver = buf1Ptr;
    }

    // Get the values from your children
    // Here is the right shift, the +1 by iterating 1..ARITY
    for (u32 i = 1; i <= ARITY; i++){
        u32 vSrc = vRank * ARITY + i;

        //if (vSrc >= numRanks){
        if (vSrc >= groupSize){
            // If vSrc is not a tree node, none of the successive
            // vSrc+1, +2, ... are in the tree, so we can stop
            break;
        }

        // root will never be a source; make sure to take the value from 0
        if (root == vSrc) {
            vSrc = 0;
        }
#if DEBUG_MPI
        srcs[i-1] = vSrc;
#endif

        // Arrange the src "p" and target "recvptr" pointers
        if (2 == i && root != rank){
            // If it's the second iteration and not the root, we need
            // to set up buf2Ptr, and use it as receiver and src:p for
            // the rest of the iterations (think ARITY=16)
            p = receiver = buf2Ptr = (totalSize <= smallSize ? &buffer2 : malloc(totalSize));
        }

        MPI_Recv(receiver, count, datatype, vSrc, 0 /*tag*/, comm, MPI_STATUS_IGNORE);

        // *** "Reduce" result into recvbuf  ***

        int j;

        if (datatype == MPI_INT) {
            int *a = recvbuf;
            int *b = p;
            if (op == MPI_SUM) {
                for(j=0;j<count;j++) {
                    a[j] += b[j];
                }
            } else if (op == MPI_MIN) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j]>b[j]) ? b[j] : a[j];
                }
            } else if (op == MPI_MAX) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j]<b[j]) ? b[j] : a[j];
                }
                // no MINLOC/MAXLOC
            } else if (op == MPI_PROD) {
                for(j=0;j<count;j++) {
                    a[j] *= b[j];
                }
            } else if (op == MPI_LAND) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] && b[j];
                }
            } else if (op == MPI_BAND) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] & b[j];
                }
            } else if (op == MPI_LOR) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] || b[j];
                }
            } else if (op == MPI_BOR) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] | b[j];
                }
            } else if (op == MPI_LXOR) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j] && b[j]) || (!a[j] && !b[j]);
                }
            } else if (op == MPI_BXOR) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] ^ b[j];
                }
            } else {
                char msg[100];
                sprintf((char *)&msg, "MPI_Reduce: Unsupported MPI_Op: %d, type: %d\n", op, datatype);
                MPI_ERROR(msg);
            }
        } else if (datatype == MPI_LONG) {
            long *a = recvbuf;
            long *b = p;
            if (op == MPI_SUM) {
                for(j=0;j<count;j++) {
                    a[j] += b[j];
                }
            } else if (op == MPI_MIN) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j]>b[j]) ? b[j] : a[j];
                }
            } else if (op == MPI_MAX) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j]<b[j]) ? b[j] : a[j];
                }
                // no MINLOC/MAXLOC
            } else if (op == MPI_PROD) {
                for(j=0;j<count;j++) {
                    a[j] *= b[j];
                }
            } else if (op == MPI_LAND) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] && b[j];
                }
            } else if (op == MPI_BAND) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] & b[j];
                }
            } else if (op == MPI_LOR) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] || b[j];
                }
            } else if (op == MPI_BOR) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] | b[j];
                }
            } else if (op == MPI_LXOR) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j] && b[j]) || (!a[j] && !b[j]);
                }
            } else if (op == MPI_BXOR) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] ^ b[j];
                }
            } else {
                char msg[100];
                sprintf((char *)&msg, "MPI_Reduce: Unsupported MPI_Op: %d, type: %d\n", op, datatype);
                MPI_ERROR(msg);
            }
        } else if (datatype == MPI_LONG_LONG || datatype == MPI_LONG_LONG_INT) {
            long long *a = recvbuf;
            long long *b = p;
            if (op == MPI_SUM) {
                for(j=0;j<count;j++) {
                    a[j] += b[j];
                }
            } else if (op == MPI_MIN) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j]>b[j]) ? b[j] : a[j];
                }
            } else if (op == MPI_MAX) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j]<b[j]) ? b[j] : a[j];
                }
                // no MINLOC/MAXLOC
            } else if (op == MPI_PROD) {
                for(j=0;j<count;j++) {
                    a[j] *= b[j];
                }
            } else if (op == MPI_LAND) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] && b[j];
                }
            } else if (op == MPI_BAND) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] & b[j];
                }
            } else if (op == MPI_LOR) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] || b[j];
                }
            } else if (op == MPI_BOR) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] | b[j];
                }
            } else if (op == MPI_LXOR) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j] && b[j]) || (!a[j] && !b[j]);
                }
            } else if (op == MPI_BXOR) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] ^ b[j];
                }
            } else {
                char msg[100];
                sprintf((char *)&msg, "MPI_Reduce: Unsupported MPI_Op: %d, type: %d\n", op, datatype);
                MPI_ERROR(msg);
            }
        } else if (datatype == MPI_UNSIGNED) {
            unsigned *a = recvbuf;
            unsigned *b = p;
            if (op == MPI_SUM) {
                for(j=0;j<count;j++) {
                    a[j] += b[j];
                }
            } else if (op == MPI_MIN) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j]>b[j]) ? b[j] : a[j];
                }
            } else if (op == MPI_MAX) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j]<b[j]) ? b[j] : a[j];
                }
                // no MINLOC/MAXLOC
            } else if (op == MPI_PROD) {
                for(j=0;j<count;j++) {
                    a[j] *= b[j];
                }
            } else if (op == MPI_LAND) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] && b[j];
                }
            } else if (op == MPI_BAND) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] & b[j];
                }
            } else if (op == MPI_LOR) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] || b[j];
                }
            } else if (op == MPI_BOR) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] | b[j];
                }
            } else if (op == MPI_LXOR) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j] && b[j]) || (!a[j] && !b[j]);
                }
            } else if (op == MPI_BXOR) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] ^ b[j];
                }
            } else {
                char msg[100];
                sprintf((char *)&msg, "MPI_Reduce: Unsupported MPI_Op: %d, type: %d\n", op, datatype);
                MPI_ERROR(msg);
            }
        } else if (datatype == MPI_UNSIGNED_LONG) {
            unsigned long *a = recvbuf;
            unsigned long *b = p;
            if (op == MPI_SUM) {
                for(j=0;j<count;j++) {
                    a[j] += b[j];
                }
            } else if (op == MPI_MIN) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j]>b[j]) ? b[j] : a[j];
                }
            } else if (op == MPI_MAX) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j]<b[j]) ? b[j] : a[j];
                }
                // no MINLOC/MAXLOC
            } else if (op == MPI_PROD) {
                for(j=0;j<count;j++) {
                    a[j] *= b[j];
                }
            } else if (op == MPI_LAND) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] && b[j];
                }
            } else if (op == MPI_BAND) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] & b[j];
                }
            } else if (op == MPI_LOR) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] || b[j];
                }
            } else if (op == MPI_BOR) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] | b[j];
                }
            } else if (op == MPI_LXOR) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j] && b[j]) || (!a[j] && !b[j]);
                }
            } else if (op == MPI_BXOR) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] ^ b[j];
                }
            } else {
                char msg[100];
                sprintf((char *)&msg, "MPI_Reduce: Unsupported MPI_Op: %d, type: %d\n", op, datatype);
                MPI_ERROR(msg);
            }
        } else if (datatype == MPI_UNSIGNED_LONG_LONG) {
            unsigned long long *a = recvbuf;
            unsigned long long *b = p;
            if (op == MPI_SUM) {
                for(j=0;j<count;j++) {
                    a[j] += b[j];
                }
            } else if (op == MPI_MIN) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j]>b[j]) ? b[j] : a[j];
                }
            } else if (op == MPI_MAX) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j]<b[j]) ? b[j] : a[j];
                }
                // no MINLOC/MAXLOC
            } else if (op == MPI_PROD) {
                for(j=0;j<count;j++) {
                    a[j] *= b[j];
                }
            } else if (op == MPI_LAND) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] && b[j];
                }
            } else if (op == MPI_BAND) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] & b[j];
                }
            } else if (op == MPI_LOR) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] || b[j];
                }
            } else if (op == MPI_BOR) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] | b[j];
                }
            } else if (op == MPI_LXOR) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j] && b[j]) || (!a[j] && !b[j]);
                }
            } else if (op == MPI_BXOR) {
                for(j=0;j<count;j++) {
                    a[j] = a[j] ^ b[j];
                }
            } else {
                char msg[100];
                sprintf((char *)&msg, "MPI_Reduce: Unsupported MPI_Op: %d, type: %d\n", op, datatype);
                MPI_ERROR(msg);
            }
        } else if (datatype == MPI_FLOAT) {
            float *a = recvbuf;
            float *b = p;
            if (op == MPI_SUM) {
                for(j=0;j<count;j++) {
                    a[j] += b[j];
                }
            } else if (op == MPI_MIN) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j]>b[j]) ? b[j] : a[j];
                }
            } else if (op == MPI_MAX) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j]<b[j]) ? b[j] : a[j];
                }
                // no MINLOC/MAXLOC
            } else if (op == MPI_PROD) {
                for(j=0;j<count;j++) {
                    a[j] *= b[j];
                }
                // no LAND/BAND/LOR/BOR/LXOR/BXOR
            } else {
                char msg[100];
                sprintf((char *)&msg, "MPI_Reduce: Unsupported MPI_Op: %d, type: %d\n", op, datatype);
                MPI_ERROR(msg);
            }
        } else if (datatype == MPI_DOUBLE) {
            double *a = recvbuf;
            double *b = p;
            if (op == MPI_SUM) {
                for(j=0;j<count;j++) {
                    a[j] += b[j];
                }
            } else if (op == MPI_MIN) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j]>b[j]) ? b[j] : a[j];
                }
            } else if (op == MPI_MAX) {
                for(j=0;j<count;j++) {
                    a[j] = (a[j]<b[j]) ? b[j] : a[j];
                }
                // no MINLOC/MAXLOC
            } else if (op == MPI_PROD) {
                for(j=0;j<count;j++) {
                    a[j] *= b[j];
                }
                // no LAND/BAND/LOR/BOR/LXOR/BXOR
            } else {
                char msg[100];
                sprintf((char *)&msg, "MPI_Reduce: Unsupported MPI_Op: %d, type: %d\n", op, datatype);
                MPI_ERROR(msg);
            }
        } else if (datatype == MPI_DOUBLE_INT) {
            double_int *a = recvbuf;
            double_int *b = p;
            if (op == MPI_MINLOC) {
                for(j=0;j<count;j++) {
                    if (a[j].a > b[j].a) {
                        a[j].a = b[j].a;
                        a[j].b = b[j].b;
                    } else if (a[j].a == b[j].a) {  // get min index if equals
                        if (a[j].b > b[j].b)
                            a[j].b = b[j].b;
                    }
                }
            } else if (op == MPI_MAXLOC) {
                for(j=0;j<count;j++) {
                    if (a[j].a < b[j].a) {
                        a[j].a = b[j].a;
                        a[j].b = b[j].b;
                    } else if (a[j].a == b[j].a) {  // get min index if equal
                        if (a[j].b > b[j].b)
                            a[j].b = b[j].b;
                    }
                }
            } else {
                char msg[100];
                sprintf((char *)&msg, "MPI_Reduce: Unsupported MPI_Op: %d, type: %d\n", op, datatype);
                MPI_ERROR(msg);
            }
        } else {
            char msg[100];
            sprintf((char *)&msg, "MPI_Reduce: Unsupported MPI_Op: %d, type: %d\n", op, datatype);
            MPI_ERROR(msg);
        }
    }

    if (rank != root){
        MPI_Send(recvbuf, count, datatype, vDest, 0 /*tag*/, comm);

#if DEBUG_MPI
        ocrPrintf("Reduce: rank#%d sends to %d recvs from %d %d %d %d\n", rank, vDest,
               srcs[0], srcs[1], srcs[2], srcs[3]);
        fflush(stdout);
    } else {
        ocrPrintf("Reduce: root rank#%d returns value\n", rank);
        fflush(stdout);
#endif
    }

    if (totalSize > smallSize){
        // need to free stuff that was malloc-ed
        free(buf1Ptr);
        if (NULL != buf2Ptr){
            free(buf2Ptr);
        }
    }

    return MPI_SUCCESS;
}

#else // ! TREE_REDUCE

// send directly to root
int MPI_Reduce(void *sendbuf, void *recvbuf, int count,
    MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
{
    // Need original rank # for communication
    comm = getComm(comm);

    rankContextP_t rankContext = getRankContext();
    const u32 rank = gpid_to_rank(comm, rankContext->rank);
    const u32 groupSize = comm->group_size;
    const u32 maxTag = rankContext->maxTag;
    const u64 totalSize = count * rankContext->sizeOf[datatype];


    if (sendbuf == MPI_IN_PLACE) {
        sendbuf = recvbuf;
    }

    if (root == rank) {
        u32 i, j;
        memcpy(recvbuf, sendbuf, totalSize);

        void *p = malloc(totalSize);    // temporary buffer for recv
        if (!p)
            MPI_ERROR("Temporary buffer allocation for MPI_Reduce failed.");

        for(i=0;i<groupSize;i++) {
            if (i==root)
                continue;
            MPI_Recv(p, count, datatype, i, 0 /*tag*/, comm, MPI_STATUS_IGNORE);

            if (datatype == MPI_INT) {
                int *a = recvbuf;
                int *b = p;
                if (op == MPI_SUM) {
                    for(j=0;j<count;j++) {
                        a[j] += b[j];
                    }
                } else if (op == MPI_MIN) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j]>b[j]) ? b[j] : a[j];
                    }
                } else if (op == MPI_MAX) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j]<b[j]) ? b[j] : a[j];
                    }
// no MINLOC/MAXLOC
                } else if (op == MPI_PROD) {
                    for(j=0;j<count;j++) {
                        a[j] *= b[j];
                    }
                } else if (op == MPI_LAND) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] && b[j];
                    }
                } else if (op == MPI_BAND) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] & b[j];
                    }
                } else if (op == MPI_LOR) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] || b[j];
                    }
                } else if (op == MPI_BOR) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] | b[j];
                    }
                } else if (op == MPI_LXOR) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j] && b[j]) || (!a[j] && !b[j]);
                                        }
                } else if (op == MPI_BXOR) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] ^ b[j];
                    }
                } else {
                    char msg[100];
                    sprintf((char *)&msg, "MPI_Reduce: Unsupported MPI_Op: %d, type: %d\n", op, datatype);
                    MPI_ERROR(msg);
                }
            } else if (datatype == MPI_LONG) {
                long *a = recvbuf;
                long *b = p;
                if (op == MPI_SUM) {
                    for(j=0;j<count;j++) {
                        a[j] += b[j];
                    }
                } else if (op == MPI_MIN) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j]>b[j]) ? b[j] : a[j];
                    }
                } else if (op == MPI_MAX) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j]<b[j]) ? b[j] : a[j];
                    }
// no MINLOC/MAXLOC
                } else if (op == MPI_PROD) {
                    for(j=0;j<count;j++) {
                        a[j] *= b[j];
                    }
                } else if (op == MPI_LAND) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] && b[j];
                    }
                } else if (op == MPI_BAND) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] & b[j];
                    }
                } else if (op == MPI_LOR) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] || b[j];
                    }
                } else if (op == MPI_BOR) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] | b[j];
                    }
                } else if (op == MPI_LXOR) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j] && b[j]) || (!a[j] && !b[j]);
                    }
                } else if (op == MPI_BXOR) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] ^ b[j];
                    }
                } else {
                    char msg[100];
                    sprintf((char *)&msg, "MPI_Reduce: Unsupported MPI_Op: %d, type: %d\n", op, datatype);
                    MPI_ERROR(msg);
                }
            } else if (datatype == MPI_LONG_LONG || datatype == MPI_LONG_LONG_INT) {
                long long *a = recvbuf;
                long long *b = p;
                if (op == MPI_SUM) {
                    for(j=0;j<count;j++) {
                        a[j] += b[j];
                    }
                } else if (op == MPI_MIN) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j]>b[j]) ? b[j] : a[j];
                    }
                } else if (op == MPI_MAX) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j]<b[j]) ? b[j] : a[j];
                    }
// no MINLOC/MAXLOC
                } else if (op == MPI_PROD) {
                    for(j=0;j<count;j++) {
                        a[j] *= b[j];
                    }
                } else if (op == MPI_LAND) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] && b[j];
                    }
                } else if (op == MPI_BAND) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] & b[j];
                    }
                } else if (op == MPI_LOR) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] || b[j];
                    }
                } else if (op == MPI_BOR) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] | b[j];
                    }
                } else if (op == MPI_LXOR) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j] && b[j]) || (!a[j] && !b[j]);
                    }
                } else if (op == MPI_BXOR) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] ^ b[j];
                    }
                } else {
                    char msg[100];
                    sprintf((char *)&msg, "MPI_Reduce: Unsupported MPI_Op: %d, type: %d\n", op, datatype);
                    MPI_ERROR(msg);
                }
            } else if (datatype == MPI_UNSIGNED) {
                unsigned *a = recvbuf;
                unsigned *b = p;
                if (op == MPI_SUM) {
                    for(j=0;j<count;j++) {
                        a[j] += b[j];
                    }
                } else if (op == MPI_MIN) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j]>b[j]) ? b[j] : a[j];
                    }
                } else if (op == MPI_MAX) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j]<b[j]) ? b[j] : a[j];
                    }
// no MINLOC/MAXLOC
                } else if (op == MPI_PROD) {
                    for(j=0;j<count;j++) {
                        a[j] *= b[j];
                    }
                } else if (op == MPI_LAND) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] && b[j];
                    }
                } else if (op == MPI_BAND) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] & b[j];
                    }
                } else if (op == MPI_LOR) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] || b[j];
                    }
                } else if (op == MPI_BOR) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] | b[j];
                    }
                } else if (op == MPI_LXOR) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j] && b[j]) || (!a[j] && !b[j]);
                    }
                } else if (op == MPI_BXOR) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] ^ b[j];
                    }
                } else {
                    char msg[100];
                    sprintf((char *)&msg, "MPI_Reduce: Unsupported MPI_Op: %d, type: %d\n", op, datatype);
                    MPI_ERROR(msg);
                }
            } else if (datatype == MPI_UNSIGNED_LONG) {
                unsigned long *a = recvbuf;
                unsigned long *b = p;
                if (op == MPI_SUM) {
                    for(j=0;j<count;j++) {
                        a[j] += b[j];
                    }
                } else if (op == MPI_MIN) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j]>b[j]) ? b[j] : a[j];
                    }
                } else if (op == MPI_MAX) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j]<b[j]) ? b[j] : a[j];
                    }
// no MINLOC/MAXLOC
                } else if (op == MPI_PROD) {
                    for(j=0;j<count;j++) {
                        a[j] *= b[j];
                    }
                } else if (op == MPI_LAND) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] && b[j];
                    }
                } else if (op == MPI_BAND) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] & b[j];
                    }
                } else if (op == MPI_LOR) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] || b[j];
                    }
                } else if (op == MPI_BOR) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] | b[j];
                    }
                } else if (op == MPI_LXOR) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j] && b[j]) || (!a[j] && !b[j]);
                    }
                } else if (op == MPI_BXOR) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] ^ b[j];
                    }
                } else {
                    char msg[100];
                    sprintf((char *)&msg, "MPI_Reduce: Unsupported MPI_Op: %d, type: %d\n", op, datatype);
                    MPI_ERROR(msg);
                }
            } else if (datatype == MPI_UNSIGNED_LONG_LONG) {
                unsigned long long *a = recvbuf;
                unsigned long long *b = p;
                if (op == MPI_SUM) {
                    for(j=0;j<count;j++) {
                        a[j] += b[j];
                    }
                } else if (op == MPI_MIN) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j]>b[j]) ? b[j] : a[j];
                    }
                } else if (op == MPI_MAX) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j]<b[j]) ? b[j] : a[j];
                    }
// no MINLOC/MAXLOC
                } else if (op == MPI_PROD) {
                    for(j=0;j<count;j++) {
                        a[j] *= b[j];
                    }
                } else if (op == MPI_LAND) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] && b[j];
                    }
                } else if (op == MPI_BAND) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] & b[j];
                    }
                } else if (op == MPI_LOR) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] || b[j];
                    }
                } else if (op == MPI_BOR) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] | b[j];
                    }
                } else if (op == MPI_LXOR) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j] && b[j]) || (!a[j] && !b[j]);
                    }
                } else if (op == MPI_BXOR) {
                    for(j=0;j<count;j++) {
                        a[j] = a[j] ^ b[j];
                    }
                } else {
                    char msg[100];
                    sprintf((char *)&msg, "MPI_Reduce: Unsupported MPI_Op: %d, type: %d\n", op, datatype);
                    MPI_ERROR(msg);
                }
            } else if (datatype == MPI_FLOAT) {
                float *a = recvbuf;
                float *b = p;
                if (op == MPI_SUM) {
                    for(j=0;j<count;j++) {
                        a[j] += b[j];
                    }
                } else if (op == MPI_MIN) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j]>b[j]) ? b[j] : a[j];
                    }
                } else if (op == MPI_MAX) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j]<b[j]) ? b[j] : a[j];
                    }
// no MINLOC/MAXLOC
                } else if (op == MPI_PROD) {
                    for(j=0;j<count;j++) {
                        a[j] *= b[j];
                    }
// no LAND/BAND/LOR/BOR/LXOR/BXOR
                } else {
                    char msg[100];
                    sprintf((char *)&msg, "MPI_Reduce: Unsupported MPI_Op: %d, type: %d\n", op, datatype);
                    MPI_ERROR(msg);
                }
            } else if (datatype == MPI_DOUBLE) {
                double *a = recvbuf;
                double *b = p;
                if (op == MPI_SUM) {
                    for(j=0;j<count;j++) {
                        a[j] += b[j];
                    }
                } else if (op == MPI_MIN) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j]>b[j]) ? b[j] : a[j];
                    }
                } else if (op == MPI_MAX) {
                    for(j=0;j<count;j++) {
                        a[j] = (a[j]<b[j]) ? b[j] : a[j];
                    }
// no MINLOC/MAXLOC
                } else if (op == MPI_PROD) {
                    for(j=0;j<count;j++) {
                        a[j] *= b[j];
                    }
// no LAND/BAND/LOR/BOR/LXOR/BXOR
                } else {
                    char msg[100];
                    sprintf((char *)&msg, "MPI_Reduce: Unsupported MPI_Op: %d, type: %d\n", op, datatype);
                    MPI_ERROR(msg);
                }
            } else if (datatype == MPI_DOUBLE_INT) {
                double_int *a = recvbuf;
                double_int *b = p;
                if (op == MPI_MINLOC) {
                    for(j=0;j<count;j++) {
                        if (a[j].a > b[j].a) {
                            a[j].a = b[j].a;
                            a[j].b = b[j].b;
                        } else if (a[j].a == b[j].a) {  // get min index if equals
                            if (a[j].b > b[j].b)
                                a[j].b = b[j].b;
                        }
                    }
                } else if (op == MPI_MAXLOC) {
                    for(j=0;j<count;j++) {
                        if (a[j].a < b[j].a) {
                            a[j].a = b[j].a;
                            a[j].b = b[j].b;
                        } else if (a[j].a == b[j].a) {  // get min index if equal
                            if (a[j].b > b[j].b)
                                a[j].b = b[j].b;
                        }
                    }
                } else {
                    char msg[100];
                    sprintf((char *)&msg, "MPI_Reduce: Unsupported MPI_Op: %d, type: %d\n", op, datatype);
                    MPI_ERROR(msg);
                }
            } else {
                char msg[100];
                sprintf((char *)&msg, "MPI_Reduce: Unsupported MPI_Op: %d, type: %d\n", op, datatype);
                MPI_ERROR(msg);
            }
        } // for

        free(p);
    } else {
        MPI_Send(sendbuf, count, datatype, root, 0 /*tag*/, comm);
    }

    return MPI_SUCCESS;
}

#endif // TREE_REDUCE

#define ALLREDUCE_ROOT 0
int MPI_Allreduce ( void *sendbuf, void *recvbuf, int count,
                    MPI_Datatype datatype, MPI_Op op, MPI_Comm comm )
{

    if (sendbuf == MPI_IN_PLACE) {
        sendbuf = recvbuf;
    }

    MPI_Reduce(sendbuf, recvbuf, count, datatype, op, ALLREDUCE_ROOT, comm);
    MPI_Bcast(recvbuf, count, datatype, ALLREDUCE_ROOT, comm);

    return MPI_SUCCESS;
}


#ifndef TREE_BCAST
    #define  TREE_BCAST 1
#endif

#if TREE_BCAST

int MPI_Bcast (void *buffer, int count, MPI_Datatype datatype, int root,
               MPI_Comm comm )
{
    // get communicator
    comm = getComm(comm);

    rankContextP_t rankContext = getRankContext();
    const u32 rank = gpid_to_rank(comm, rankContext->rank);
    const u32 groupSize = comm->group->size;

    if (groupSize == 1)  // e.g. comm_self
        return MPI_SUCCESS;

    if (root >= groupSize) {
        MPI_ERROR("MPI_Bcast: root >= group size? invalid rank\n");
    }

    // swap root and zero (and if root==0, vRank will be 0)
    const u32 vRank = ((root == rank) ? 0 : (0 == rank ? root : rank));

    // vRank 0 gets the value in buffer in the arg list; everyone else
    // has to receive it from the parent.
    if (vRank != 0) {
        u32 sender = (vRank - 1) / ARITY;
        u32 vSender = ((root == sender) ? 0 : (0 == sender ? root : sender));

        MPI_Recv(buffer, count, datatype, vSender, 0 /* tag */, comm, MPI_STATUS_IGNORE);
    }

    // Now send it to your children
    // Here is the right shift, the +1 by iterating 1..ARITY
    // vDest will never be computed as 0, always starts at least at 1
    for (u32 i = 1; i <= ARITY; i++){
        u32 vDest = vRank * ARITY + i;

        // Is vDest still "in" the tree?
        if (vDest >= groupSize){
            // If vDest is not a tree node, none of the successive
            // vDest+1, +2, ... are in the tree, so we can stop
            break;
        }

        // root already has the buffer, make sure to send buffer to zero instead!
        if (root == vDest) {
            vDest = 0;
        }

        MPI_Send(buffer, count, datatype, vDest, 0 /* tag */, comm);
    }

    return MPI_SUCCESS;
}


#else // non TREE_BCAST: root sends to everyone

    #if 1   // this works

int MPI_Bcast (void *buffer, int count, MPI_Datatype datatype, int root,
               MPI_Comm comm )
{
    // Need communicator
    comm = getComm(comm);

    rankContextP_t rankContext = getRankContext();
    const u32 rank = gpid_to_rank(rankContext->rank);
    const u32 groupSize = comm->group->size;

    if (groupSize == 1)  // e.g. comm_self
        return MPI_SUCCESS;

    if (root >= groupSize) {
        MPI_ERROR("MPI_Bcast: root >= group size? invalid rank\n");
    }

    if (root == rank) {
        u32 i;
        for(i=0;i<groupSize;i++) {
            if (i != root) {
                MPI_Send(buffer, count, datatype, i, 0 /* tag */, comm);
            }
        }
    } else {
        MPI_Recv(buffer, count, datatype, root, 0 /* tag */, comm, MPI_STATUS_IGNORE);
    }
    return MPI_SUCCESS;
}

    #else  //this works, too.

int MPI_Bcast (void *buffer, int count, MPI_Datatype datatype, int root,
               MPI_Comm comm )
{
    // Need original rank # for communication
    comm = getComm(comm);

    rankContextP_t rankContext = getRankContext();
    const u32 rank = gpid_to_rank(comm, rankContext->rank);
    const u32 groupSize = comm->group->size;

    if (groupSize == 1)  // e.g. comm_self
        return MPI_SUCCESS;

    if (root >= groupSize) {
        MPI_ERROR("MPI_Bcast: root >= group size? invalid rank\n");
    }
    if (root == rank) {
        u32 i;
        MPI_Request req[groupSize];
        for(i=0;i<groupSize;i++) {
            if (i != root) {
                MPI_Isend(buffer, count, datatype, i, 0 /* tag */, comm, &req[i]);
            } else {
                req[i] = (MPI_Request)MPI_REQUEST_NULL;
            }
        }
        MPI_Waitall(groupSize, req, MPI_STATUSES_IGNORE);

    } else {
        MPI_Recv(buffer, count, datatype, root, 0 /* tag */, comm, MPI_STATUS_IGNORE);

    }
    return MPI_SUCCESS;
}
    #endif
#endif // TREE_BCAST

int MPI_Barrier(MPI_Comm comm)
{
    long buf[1];
    long sum=0;
    long one=1;

    MPI_Reduce(&sum, &one, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Bcast(buf, 1, MPI_LONG, 0 /*root*/, MPI_COMM_WORLD);

    return MPI_SUCCESS;
}

int MPI_Scatter(void *sendbuf, int sendcount, MPI_Datatype sendtype,
                void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
                MPI_Comm comm)
{
    // root is the group-relative root.  MPI_Send/Recv will do the translation.
    comm = getComm(comm);
    const u32 groupSize = comm->group->size;

    rankContextP_t rankContext = getRankContext();
    const u32 rank = rankContext->rank;
    const u64 sendSize = sendcount * rankContext->sizeOf[sendtype];
    const u64 recvSize = recvcount * rankContext->sizeOf[recvtype];

    if (recvSize < sendSize) {
        char msg[100];
        sprintf((char*)&msg, "MPI_Scatter: recv size %d < send size %d ", recvSize, sendSize);
        MPI_WARNING(msg, MPI_ERR_TRUNCATE);
    }
    if (rank_to_gpid(comm, root) == rank) {
         void *target = (char *)recvbuf;
        memcpy(target, sendbuf + sendSize*root, sendSize);

        for(int i=0; i<groupSize; i++) {
            if (i == root)
                continue;
            void *p = (char *)sendbuf + sendSize*i;
            MPI_Send(p, sendcount, sendtype, i, 0 /* i tag*/, comm);
        }
    } else {
        MPI_Recv(recvbuf, recvcount, recvtype, root, 0, comm, MPI_STATUS_IGNORE);
    }
    return MPI_SUCCESS;
}

int MPI_Scatterv(const void *sendbuf, const int *sendcounts, const int *displs,
                 MPI_Datatype sendtype, void *recvbuf, int recvcount,
                 MPI_Datatype recvtype,
                 int root, MPI_Comm comm)
{
    // root is relative to the group.  MPI_Send/Recv will do the translation.
    comm = getComm(comm);
    const u32 groupSize = comm->group->size;

    rankContextP_t rankContext = getRankContext();
    const u32 rank = rankContext->rank;
    const u64 sendSize =             rankContext->sizeOf[sendtype];
    const u64 recvSize = recvcount * rankContext->sizeOf[recvtype];

    if (rank_to_gpid(comm, root) == rank) {
        void *target = (char *)recvbuf;
        memcpy(target, sendbuf + sendSize * displs[root], sendSize * sendcounts[root]);

        for(int i=0; i<groupSize; i++) {
            if (i == root)
                continue;
            void *p = (char *)sendbuf + sendSize * displs[i];
            MPI_Send(p, sendcounts[i], sendtype, i, 0 /*i tag*/, comm);
        }
    } else {
        MPI_Recv(recvbuf, recvcount, recvtype, root, 0, comm, MPI_STATUS_IGNORE);
    }
    return MPI_SUCCESS;
}

int MPI_Gather(void *sendbuf, int sendcount, MPI_Datatype sendtype,
               void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
               MPI_Comm comm)
{
    // root is relative to the group.  MPI_Send/Recv will do the translation.
    comm = getComm(comm);
    const u32 groupSize = comm->group->size;

    rankContextP_t rankContext = getRankContext();
    const u32 rank = rankContext->rank;
    const u64 sendSize = sendcount * rankContext->sizeOf[sendtype];
    const u64 recvSize = recvcount * rankContext->sizeOf[recvtype];

    if (recvSize < sendSize) {
        char msg[100];
        sprintf((char*)&msg, "MPI_Gather: recv size %d < send size %d ", recvSize, sendSize);
        MPI_WARNING(msg, MPI_ERR_TRUNCATE);
    }

    if (rank_to_gpid(comm,root) == rank) {
        void *target = (char *)recvbuf + recvSize*root;
        memcpy(target, sendbuf, sendSize);

        for(int i=0; i<groupSize; i++) {
            if (i == root)
                continue;
            void *p = (char *)recvbuf + recvSize*i;
            MPI_Recv(p, recvcount, recvtype, i, 0 /* i tag*/, comm, MPI_STATUS_IGNORE);
        }
    } else {
        MPI_Send(sendbuf, sendcount, sendtype, root, 0, comm);
    }
    return MPI_SUCCESS;
}

int MPI_Gatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                void *recvbuf, const int *recvcounts, const int *displs,
                MPI_Datatype recvtype, int root, MPI_Comm comm)
{
    // root is relative to the group.  MPI_Send/Recv will do the translation.
    comm = getComm(comm);
    const u32 groupSize = comm->group->size;

    rankContextP_t rankContext = getRankContext();
    const u32 rank = rankContext->rank;
    const u64 sendSize = sendcount * rankContext->sizeOf[sendtype];
    const u64 recvSize =             rankContext->sizeOf[recvtype];

    if (rank_to_gpid(comm,root) == rank) {
        void *target = (char *)recvbuf + recvSize * displs[root];
        memcpy(target, sendbuf, sendSize);

        for(int i=0; i<groupSize; i++) {
            if (i == root)
                continue;
            void *p = (char *)recvbuf + recvSize * displs[i];
            MPI_Recv(p, recvcounts[i], recvtype, i, 0 /* i tag*/, comm, MPI_STATUS_IGNORE);
        }
    } else {
        MPI_Send((void *)sendbuf, sendcount, sendtype, root, 0, comm);
    }
    return MPI_SUCCESS;
}

#define ALLGATHER_ROOT  0
int MPI_Allgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                   void *recvbuf, const int *recvcounts, const int *displs,
                   MPI_Datatype recvtype, MPI_Comm comm)
{
    // root is relative to the group.  MPI_Send/Recv will do the translation.
    comm = getComm(comm);
    const u32 groupSize = comm->group->size;

    MPI_Gatherv(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, ALLGATHER_ROOT, comm);

    // total bcast count is displacement start of last rank + count of the
    // last rank
    MPI_Bcast(recvbuf, displs[groupSize-1]+recvcounts[groupSize-1] /* assumes..  */ , recvtype, ALLGATHER_ROOT, comm);
    return MPI_SUCCESS;
}



int MPI_Allgather(void *sendbuf, int  sendcount,
                  MPI_Datatype sendtype, void *recvbuf, int recvcount,
                  MPI_Datatype recvtype, MPI_Comm comm)
{
    // root is relative to the group.  MPI_Send/Recv will do the translation.
    comm = getComm(comm);
    const u32 groupSize = comm->group->size;

    MPI_Gather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, ALLGATHER_ROOT, comm);
    MPI_Bcast(recvbuf, recvcount*groupSize, recvtype, ALLGATHER_ROOT, comm);
    return MPI_SUCCESS;
}


int MPI_Alltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                 void *recvbuf, int recvcount, MPI_Datatype recvtype,
                 MPI_Comm comm)
{
    // root is relative to the group.  MPI_Send/Recv will do the translation.
    comm = getComm(comm);
    const u32 groupSize = comm->group->size;

    rankContextP_t rankContext = getRankContext();
    const u32 rank = rankContext->rank;
    const u64 sendSize = sendcount * rankContext->sizeOf[sendtype];
    const u64 recvSize = recvcount * rankContext->sizeOf[recvtype];

    // Send before receiving so don't hang!
    for(int i=0; i<groupSize; i++) {
        if (rank_to_gpid(comm,i) == rank) {
        } else {
            MPI_Send((void *)sendbuf, sendcount, sendtype, i, 0 /* i */, comm);
        }
        sendbuf = (char *)sendbuf + sendSize;
    }
    for(int i=0; i<groupSize; i++) {
        if (rank_to_gpid(comm,i) == rank) {
            memcpy(recvbuf, (char*)sendbuf + sendSize*i, recvSize);
        } else {
            MPI_Recv(recvbuf, recvcount, recvtype, i, 0 /* rank */, comm, MPI_STATUS_IGNORE);
        }
        recvbuf = (char *)recvbuf + recvSize;
    }
    return MPI_SUCCESS;

}


int MPI_Group_size(MPI_Group group, int *size)
{
	*size = group->size;
	return MPI_SUCCESS;
}

int MPI_Group_rank(MPI_Group group, int *rank)
{
	if (group->rank == -1)
		return MPI_UNDEFINED;
	*rank = group->rank;
	return MPI_SUCCESS;
}

int MPI_Comm_group(MPI_Comm comm, MPI_Group *group)
{
    comm = getComm(comm);
    *group = comm->group;
    return MPI_SUCCESS;
}


int MPI_Group_incl(MPI_Group group, int count, const int ranks[], MPI_Group *newgroup)
{
    int i;
    MPI_Group ret = (MPI_Group)malloc(sizeof(_MPI_Group));
    if ( ! ret){
	MPI_ERROR("Error: Unable to allocate memory.\n");
        return MPI_ERR_INTERN;
    }
    ret->gpid_array = (int *)malloc(sizeof(int)*count);
    if ( ! ret->gpid_array){
	MPI_ERROR("Error: Unable to allocate memory.\n");
        return MPI_ERR_INTERN;
    }

    ret->rank = -1;
    ret->size = count;
    ret->gpid = group->gpid;

    for(i=0;i<count;i++) {
        ret->gpid_array[i] = ranks[i];
        if (ret->gpid_array[i] == ret->gpid) {
            ret->rank = i;
        }
    }

    *newgroup = ret;
    return MPI_SUCCESS;
}


// The gpid_array contains the original, physical rank number.
// The index of the physical rank number in the array indicates
// its "group" rank.
//
static int get_my_group_rank(MPI_Group group)
{
    int i;
    for(i=0;i<group->size;i++)
        if (group->gpid_array[i] == group->rank) {
            return i;
	}
    return -1;
}


static int MPI_Group_dup(MPI_Group group, MPI_Group *newgroup)
{
    MPI_Group ret = malloc(sizeof(_MPI_Group));
    if (!ret) {
        MPI_ERROR("Error: unable to allocate memory.\n");
        return MPI_ERR_INTERN;
    }

    *ret = *group;
    ret->rank = get_my_group_rank(group);
    if (ret->rank == -1) {
        MPI_ERROR("Error: invalid rank.\n");
        return MPI_ERR_INTERN;
    }

    *newgroup = ret;
    return MPI_SUCCESS;
}


int MPI_Group_free(MPI_Group *group)
{
	free(*group);
	*group = MPI_GROUP_NULL;
	return MPI_SUCCESS;
}


int MPI_Comm_create(MPI_Comm comm, MPI_Group group, MPI_Comm *newcomm)
{
    MPI_Comm ret = (MPI_Comm)malloc(sizeof(_MPI_Comm));
    if (!ret) {
	MPI_ERROR("Error: unable to allocate memory.\n");
        return MPI_ERR_INTERN;
    }

    ret->group = group;
    ret->topology = NULL;
    *newcomm = ret;
    return MPI_SUCCESS;
}

int MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm)
{
    comm = getComm(comm);

    // deep-copy. first, copy group
    MPI_Group newgroup;
    if (MPI_Group_dup(comm->group, &newgroup) != MPI_SUCCESS)
        return MPI_ERR_INTERN;

    if (MPI_Comm_create(comm, newgroup, newcomm) != MPI_SUCCESS) {
        MPI_Group_free(&newgroup);
        return MPI_ERR_INTERN;
    }

#ifdef DEBUG_MPI
    ocrPrintf("MPI_Comm_dup: success, comm: 0x%p, *newcomm:%p\n", comm, *newcomm);
    fflush(stdout);
#endif
    return MPI_SUCCESS;
}


// Creates new communicators based on colors and keys
int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm)
{
    comm = getComm(comm);

    rankContextP_t rankContext = getRankContext();
    const u32 gpid = rankContext->rank;

    int size = comm->group->size;
    int colors[size];
    int keys[size];

    // TODO: you can cut the amount of communication in half
    // if you make a struct with the <color,key> duple
    // instead of allreducing each one separately.
    MPI_Allgather(&color, 1, MPI_INT, colors, 1, MPI_INT, comm);
    MPI_Allgather(&key, 1, MPI_INT, keys, 1, MPI_INT, comm);

    if (color == MPI_UNDEFINED)
    {
        *newcomm = MPI_COMM_NULL;
        return MPI_SUCCESS;
    }
    assert(color >= 0); //color must be non-negaive or MPI_UNDEFINED

    int i,j,tmp,swapped;
    int ranks[size];
    int keys2[size];
    int count = 0;
    for(i=0;i<size;i++) {
        if (colors[i] != color)
            continue;
        keys2[count  ] = keys[i];
        ranks[count++] = comm->group->gpid_array[i];
    }

    // sorting by key values -- yes, it's O(n^2)
    // This reserves rank ordering in the old group
    for(i=count-1;i>0;i--) {
        swapped = 0;
        for(j=0;j<i;j++) {
            if (keys2[j]>keys2[j+1]) {
                tmp = ranks[j];
                ranks[j] = ranks[j+1];
                ranks[j+1] = tmp;
                tmp = keys2[j];
                keys2[j] = keys2[j+1];
                keys2[j+1] = tmp;
                swapped = 1;
            }
        }
        if (!swapped)	// exit early
            break;
    }
    MPI_Group newgroup;
    MPI_Group_incl(comm->group, count, ranks, &newgroup);

#if DEBUG_MPI
    ocrPrintf("MPI_Comm_split %d: newgroup: size:%d gpid:%d gpid_array={",
           gpid, newgroup->size, newgroup->gpid);
    int ii;
    for(ii=0;ii<newgroup->size;ii++)
        ocrPrintf("%d ", newgroup->gpid_array[ii]);
    ocrPrintf("}\n");
#endif

    MPI_Comm_create(comm, newgroup, newcomm);
}


// TODO maybe: in real MPI, the Comm is not freed until all references
// to it are removed. We do not do reference counting in MPI-Lite.
//
int MPI_Comm_free(MPI_Comm *comm)
{
    // We are not going to free MPI_COMM_WORLD or MPI_COMM_SELF.
    MPI_Group_free(&(*comm)->group);
    MPI_Topology t = ((*comm)->topology);
    if (t)
        free(t);
    free((*comm));
    *comm = MPI_COMM_NULL;
    return MPI_SUCCESS;
}

int MPI_Comm_rank(MPI_Comm comm, int *rank)
{
    comm = getComm(comm);
    MPI_Group_rank(comm->group, rank);
    return MPI_SUCCESS;
}

int MPI_Comm_size(MPI_Comm comm, int *size)
{
    comm = getComm(comm);
    MPI_Group_size(comm->group, size);
    return MPI_SUCCESS;
}


int MPI_Errhandler_set(MPI_Comm comm, MPI_Errhandler errhandler)
{
    // Hack: do nothing
    return MPI_SUCCESS;
}


int MPI_Abort(MPI_Comm comm, int errorcode)
{
    exit(errorcode);
    return 0;
}

double MPI_Wtick( void )	// Accuracy in seconds of Wtime
{
    return .000001;    // generated on x86 testing successive values of Wtime
}

double MPI_Wtime( void )	// Time in seconds since an arbitrary time in the past.
{
#ifndef __TG
    struct timeval now;
    int rc;
    double ret;
    rc = gettimeofday(&now, NULL);	// rc always 0 for now
    ret = (double) now.tv_sec + (double) now.tv_usec * 1.0e-6;
    return ret;
#else
    // cheat, race condition, but will be monotonically increasing so
    // people won't get div/by/zero if they do ratios of time diffs
    static double ret = 0;

    return (ret += 0.001);
#endif
}


// Dummy init routine.  When using ROSE translation tool for handling globals,
// this routine is replaced by a real function that will create the DB for each
// rank.
ocrGuid_t __attribute__((weak)) __ffwd_init(void ** ffwd_addr_p) {
    *ffwd_addr_p = NULL;
    return NULL_GUID;
}
