/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */


/*
 * mpi_ocr_messaging - provides messaging for mpi on ocr
 *   there is an attempt to separate the messaging from MPI specifics
 *
 * The sender creates a sticky event, creates a datablock and puts the message
 * into it, and satisfies the event with the db.
 *
 * The receiver does an ocrLegacyBlockProgress on the event. When the event is
 * satisfied, the datablock flows to the receiver. The message is extracted and
 * stored into the receiver's buffer. Then the event and db are destroyed.
 *
 * The "magic" is getting the sender and receiver to agree on using the same event.
 * The is accomplished by using the ocr "labeling" feature. The triple
 * source x destination x mpi_tag is mapped to a unique guid. The sender passes this
 * "name" to eventCreate as the guid for the event. The receiver maps the same
 * triple to the same guid, and waits for it using ocrLegacyBlockProgress.
 *
 * This is straightforward for MPI_Send and MPI_Recv, which are blocking - they
 * wait until their operation can be performed.
 *
 * Non-blocking MPI_Isend, MPI_Irecv need a way to conditionally "try" to see if
 * the operation can be performed. Labeled guids has two properties which help:
 * 1. conditionally try to create an event using a guid - if the event is currently
 *    active, then the create "fails" instead of blocking.
 * 2. conditionally "try" blockProgress - if the event corresponding to the guid
 *    is not currently "created", an "error" is returned and execution can
 *    continue, non-blocked.
 *
 * [This way of implementing Isend and Irecv is "aggressive", and
 * potentially unsafe. If a source does 2 Isends in a row to same dest and
 * tag, the first "trySend" might fail and be deferred till MPI_Wait, but the
 * second "trySend" might succeed before the first Isend's Wait - causing the 2
 * messages to get OUT OF ORDER!! A similar problem can occur with two
 * Irecvs. Therefore, the command line must use "-a" (aggressive) in order
 * to hvae this behavior. The advantage (if it's safe for the usage
 * pattern) is that the Isend/Irecv can finish earlier and potentially
 * improve overall performance.]
 *
 * MPI_*Recv can have MPI_ANY_SOURCE and/or MPI_ANY_TAG as arguments. This
 * requires querying a sequence of source and/or tag values to see if there
 * is a matching message to receive. This is the second use for
 * mpiOcrTryRecv - it is called for the different possible sources and tags;
 * if one matches, then the recv will be performed.
 *
 * Note: In order to use labeling, the .cfg file needs to have "PTR" replaced:
[GuidType0]
	name	=	LABELED #PTR

[GuidInst0]
	id	=	0
	type	=	LABELED #PTR
 *
 *
 * This source file is structured with the 3 send-related routines with their
 * overview first
 * then the 3 recv-related routines and their overview.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/time.h>
#include <mpi_ocr.h>  // must come before ocr.h, as it sets defines
#include <ocr.h>
#include <ocr-types.h>

#include <extensions/ocr-legacy.h>
#include <extensions/ocr-labeling.h>

#include <mpi.h>  // get some decls and error codes


     // USE_CHANNEL can be changed when making mpilite by setting
     // USER_FLAGS='-DUSE_CHANNEL=0'
#ifndef USE_CHANNEL
    // turned off until mpiOcrTryRecv can be implemented with channels,
    // since it is needed for recv of MPI_ANY_{SOURCE,TAG}. Asking for
    // ocrChannelIsEmpty so we can decide if there is a message on a
    // particular channel to blockProgress on.
#define USE_CHANNEL 0
#endif


// For channels, to do mpiOcrTryRecv it is necessary to speculatively
// attach a localEvent to the channel and then conditionally blockProgress
// on it using Vincent's new property value. If the blockProgress does NOT
// produce a DB (nothing to receive yet), then we need to save the
// localEvent as well as the channel for the next time we recv or tryrecv
// for this index. Thus the eventPair_t type

typedef struct
{
    ocrGuid_t channelEvent;
    ocrGuid_t localEvent;
}
    eventPair_t, * eventPairP_t;

#define MPI_INFO(s,w) {PRINTF("WARNING: %s, will return" #w "\n",(s));}
#define MPI_ERROR(s) {PRINTF("ERROR: %s; exiting\n",s); exit(1);}

// HIDE all the debugging printfs if debug off
#if !DEBUG_MPI
    #define PRINTF(a,...)
    #define fflush(a)
#endif

/*
  A better way to implement Isend/Irecv, Test, and Waitall, using the 3-state events

// Send data along a created event
Static  int sendData(event, buf, bufSize){
	DB = DbCreate(bufSize+header)
l	Fill in header, and copy buffer
	Release (DB)
	Satisfy(event, DB);
	Return OK}

mpiOcrSend( the rest of the stuff){
	Guid event = getTheEventGuid(src,dst,tag,HashGuid);
	blockingEventCreate()
	sendData()
	}

mpiOcrTrySend(the rest of the stuff; bool *done){
	Guid event = getTheEventGuid(src,dst,tag,HashGuid);
	Res = ConditionalEventCreate()
	If (res == OK){
		Ret = sendData();
		*done = true;}
	Else {
		*done = false;
		Ret = success}
	Return ret
	}

*/

/* All checks have been done, and messageEvent has been created. Package up
   the data into a DataBlock, and satisfy the event with it
*/
static int sendData(void *buf, int count, MPI_Datatype
               datatype, int source, int dest, int tag, MPI_Comm comm, u64
                  totalSize, ocrGuid_t messageEvent)
{
    mpiOcrMessageP_t ptr;
    ocrGuid_t DB;
    s32 err;

    if (err = ocrDbCreate(&DB, (void **)&ptr, totalSize + sizeof(mpiOcrMessage_t),
                          DB_PROP_NONE, NULL_HINT, NO_ALLOC))
        {
            char msg[150];
            sprintf((char*)&msg, "sendData:rank:%d, error %d from ocrDbCreate",
                    source, err);
            ERROR(msg); // exits
        }

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

    PRINTF("sendData: rank #%d: Send to %d tag:%d on event guid %p, DB %p, ptr %p\n"
           "\tptr->{%lx,%lx,%lx,%lx}\n",
           source,  dest, tag, messageEvent, DB, ptr,
           ((u64*)ptr)[0],((u64*)ptr)[1],((u64*)ptr)[2],((u64*)ptr)[3]); fflush(stdout);

    ocrDbRelease(DB); // make sure it's visible to receiver

    // OK, Send the DB. [It's only going to be used in DB_MODE_CONST by the
    // receiver, but you can only say that when adding a dependence
    // directly to an EDT; not to an Event....]
    if (err = ocrEventSatisfy(messageEvent, DB))
        {
            char msg[150];
            sprintf((char*)&msg, "sendData:rank:%d, error %d from ocrEventSatisfy",
                    source, err);
            ERROR(msg); // exits
        }

    return MPI_SUCCESS;
}


/* Do a Send. Many of the args are just for packing in the message header.
   source and dest are physical - having been mapped through the
   Comm. Totalsize has also been computed.
*/
int mpiOcrSend(void *buf, int count, /*MPI_Datatype*/ int
               datatype, int source, int dest, int tag, MPI_Comm comm, u64 totalSize)
{
    const ocrGuid_t messageEventRange = getMessageContext()->messageEventRange;
    ocrGuid_t messageEvent,   // eventually passed to sendData
        labeledEvent,         // sticky event in the labeled map
        channelEvent;         // cannot add dependency to a channel event

    rankContextP_t rankContext = getRankContext();
    const u32 numRanks = rankContext->numRanks;
    const u32 maxTag = rankContext->maxTag;
    s32 err;
    ocrGuid_t DB;
    void *myPtr;
    u64 dbSize;

    u32 index = guidIndex(source, dest, tag);

#if ! USE_CHANNEL

    // Old style where message is put on the labeled sticky event

    if (err=ocrGuidFromIndex(&labeledEvent, messageEventRange, index))
    {
        char msg[150];
        sprintf((char*)&msg, "mpiOcrSend:rank:%d, error %d from ocrGuidFromIndex",
                source, err);
        ERROR(msg); // exits
    }

    // Block until it is "legal" to create or recreate an event using the
    // labeledEvent guid.
    if (err=ocrEventCreate(&labeledEvent, OCR_EVENT_STICKY_T,
                           EVT_PROP_TAKES_ARG | GUID_PROP_BLOCK | GUID_PROP_IS_LABELED))
    {
        char msg[150];
        sprintf((char*)&msg, "mpiOcrSend:rank:%d, error %d from ocrEventCreate(%p)",
                source, err, labeledEvent);
        ERROR(msg); // exits
    }

    PRINTF("mpiOcrSend: rank #%d: Sending to %d tag:%d on event guid %p\n",
           source,  dest, tag, labeledEvent); fflush(stdout);

    // We've got a valid event, so use it for the send
    messageEvent = labeledEvent;

    return sendData(buf, count, datatype, source, dest, tag, comm,
                    totalSize, messageEvent);

#else  // USE_CHANNEL

   // see if the labeled event is in the local hashtable.
    ocrGuid_t * labeledEventPtr = (ocrGuid_t *)mpiHashtableGet(rankContext->labeledEventsCache,
                                                 index);
    if (labeledEventPtr == NULL){
        labeledEvent = NULL_GUID;
    }
    else {
        labeledEvent = *labeledEventPtr;
    }

    if ( ! ocrGuidIsNull(labeledEvent))
    {
        // Extract the DB from labeled event via BlockProgress
        // and then extract the channelEvent GUID from the DB.
        // Note, don't have to wait for create in the blockProgress...

        PRINTF("mpiOcrSend: rank #%d labeled event %p already exists\n",
               source, labeledEvent);
        fflush(stdout);

        if(err=ocrLegacyBlockProgress(labeledEvent, &DB, &myPtr, &dbSize,
                                      LEGACY_PROP_NONE))
        {
            char msg[150];
            sprintf((char*)&msg, "mpiOcrSend num:%d, error %d from ocrLegacyBlockProgress on labeled event(%p)",
                    source, err, labeledEvent);
            ERROR(msg); // exits
        }

        // DB has single element - the channel's Guid
        channelEvent = ((eventPairP_t) myPtr) -> channelEvent;
        ocrDbRelease(DB);


        PRINTF("mpiOcrSend: rank #%d, got channel %p from ocrLegacyBlockProgress(%p)\n",
               source ,channelEvent, labeledEvent);
        fflush(stdout);

    }

    else   // labeledEvent == 0
    {
        // First time for this map index for labeled event, so
        // set everything up.

        if (err=ocrGuidFromIndex(&labeledEvent, messageEventRange, index))
        {
            char msg[150];
            sprintf((char*)&msg, "mpiOcrSend:rank:%d, error %d from ocrGuidFromIndex",
                    source, err);
            ERROR(msg); // exits
        }

        PRINTF("mpiOcrSend: rank #%d got labeled event guid %p\n", source, labeledEvent);
        fflush(stdout);


        // This event create checks if the labeled event has already been
        // created. It will never be destroyed, so if it already exists, we
        // don't have to create a channel or attach a DB to
        // labeledEvent. (Unlike the case above)

        if (err=ocrEventCreate(&labeledEvent, OCR_EVENT_STICKY_T,
                               EVT_PROP_TAKES_ARG | GUID_PROP_CHECK | GUID_PROP_IS_LABELED))
        {
            if (err != OCR_EGUIDEXISTS)
            {
                // Oops, something's wrong
                char msg[150];
                sprintf((char*)&msg, "mpiOcrSend: rank #%d error %d from STICKY ocrEventCreate(%p)",
                        source, err, labeledEvent);
                ERROR(msg); // exits
            }
        }

        PRINTF("mpiOcrSend: rank #%d created labeled event %p\n",source, labeledEvent);
        fflush(stdout);

        // Create the channel, put its Guid in a db, and attach to the labeled
        // sticky event.
        ocrEventParams_t channelParams = { .EVENT_CHANNEL =
                                           {
                                               .maxGen = 16 /* EVENT_CHANNEL_UNBOUNDED */,
                                               .nbSat =   1 ,
                                               .nbDeps =  1
                                           }};

        if (err=ocrEventCreateParams(&channelEvent, OCR_EVENT_CHANNEL_T, EVT_PROP_TAKES_ARG,
                                     &channelParams))
        {
            char msg[150];
            sprintf((char*)&msg, "mpiOcrSend: rank #%d  error %d from CHANNEL ocrEventCreateParams()",
                    source, err);
            ERROR(msg); // exits
        }

        PRINTF("mpiOcrSend: rank #%d created channelEvent %p\n", source, channelEvent); fflush(stdout);


        if (err = ocrDbCreate(&DB, (void **)&myPtr,  sizeof(eventPair_t),
                              DB_PROP_NONE, NULL_HINT, NO_ALLOC))
        {
            char msg[150];
            sprintf((char*)&msg, "mpiOcrSend: rank #%d  error %d from ocrDbCreate",
                    source, err);
            ERROR(msg); // exits
        }

        // eventPair in DB: fill in channelEvent, and NULL_GUID for localEvent
        ((eventPairP_t) myPtr) -> channelEvent = channelEvent;
        ((eventPairP_t) myPtr) -> localEvent = NULL_GUID;
        ocrDbRelease(DB);

        // only need to satisfy the event once. it will always return the same DB.
        if (err = ocrEventSatisfy(labeledEvent, DB))
        {
            char msg[150];
            sprintf((char*)&msg, "mpiOcrSend: rank #%d error %d from ocr(labeled)EventSatisfy(%p)",
                    source, err, labeledEvent);
            ERROR(msg); // exits
        }
        PRINTF("mpiOcrSend: rank #%d satisfied labeled event %p with DB %p containing channel %p\n",
               source, labeledEvent, DB, channelEvent); fflush(stdout);


        // place the labeled event in a local hashtable for later communication
        ocrGuid_t * labeledEventPtr = malloc(sizeof(ocrGuid_t));
        *labeledEventPtr = labeledEvent;
        mpiHashtablePut(rankContext->labeledEventsCache, index,
                        labeledEventPtr);

    }   // endif err=createLabeledEvent(labeledEvent)

    messageEvent = channelEvent;

    return sendData(buf, count, datatype, source, dest, tag, comm,
                    totalSize, messageEvent);

#endif  // USE_CHANNEL

}

/* Try to do a Send. If the event can't be created, return done=FALSE.
   Many of the args are just for packing in the message header.
   source and dest are physical - having been mapped through the
   Comm. Totalsize has also been computed.
 */
int mpiOcrTrySend(void *buf, int count, /*MPI_Datatype*/ int
               datatype, int source, int dest, int tag, MPI_Comm comm, u64
                  totalSize, bool *done)
{
#if USE_CHANNEL
    // **** Vincent said he would give us unbounded channels, so mpiOcrSend will
    // always succeed and so done will be TRUE;

    *done = TRUE;
    return mpiOcrSend(buf, count, datatype, source, dest, tag, comm, totalSize);

#else // ! USE_CHANNEL
    const ocrGuid_t messageEventRange = getMessageContext()->messageEventRange;
    ocrGuid_t messageEvent;

    rankContextP_t rankContext = getRankContext();
    const u32 numRanks = rankContext->numRanks;
    const u32 maxTag = rankContext->maxTag;

    u32 err;
    if (err=ocrGuidFromIndex(&messageEvent, messageEventRange, guidIndex(source, dest, tag)))
        {
            char msg[150];
            sprintf((char*)&msg, "mpiOcrTrySend:rank:%d, error %d from ocrGuidFromIndex",
                    source, err);
            ERROR(msg); // exits
        }


    // Don't block if messageEvent already exists (error will be
    // returned). Send will have to be done some other time.
    if (err=ocrEventCreate(&messageEvent, OCR_EVENT_STICKY_T,
                   EVT_PROP_TAKES_ARG | GUID_PROP_IS_LABELED | GUID_PROP_CHECK))
        {
            PRINTF("*mpiOcrTrySend: rank #%d: Sending to %d tag:%d on event guid %p: fail:%d\n",
                   source, dest, tag, messageEvent, err);  fflush(stdout);
            if (OCR_EGUIDEXISTS == err)
                {
                    // Previous recv hasn't completed, so return having
                    // done no send.

                    *done = FALSE;
                    return MPI_SUCCESS;
                }
            else
                {
                    // some unexpected error
                    char msg[150];
                    sprintf((char*)&msg, "mpiOcrTrySend:rank:%d, error %d from ocrEventCreate(%p)",
                            source, err, messageEvent);
                    ERROR(msg); // exits
                }
        }
    PRINTF("mpiOcrTrySend: rank #%d: Sending to %d tag:%d on event guid %p: success\n",
           source, dest, tag, messageEvent);  fflush(stdout);

    *done = TRUE;
    // We've got a valid event, so do the send
    return sendData(buf, count, datatype, source, dest, tag, comm,
                           totalSize, messageEvent);
#endif  // ! USE_CHANNEL
}


/*
Static recvData(){
	// compare counts, sizes of ptr->header
	Outbuf = ptr->header.data
	Destroy DB
	}

mpiOcrRecv(){
	Guid event = getTheGuid(src,dst,tag,hashGuid);
	ocrLegacyBlockProgress()
	Destroy event
	RecvData(DB, ptr, inbuf, src,dst,tag,count);
	}

mpiOcrTryRecv( bool *done){
	Guid event = getTheGuid(src,dst,tag,hashGuid);
	ocrLegacyTryBlockProgress()
	If (isReady(event)) // created or satisfied
	{
	Destroy event
	recvData(DB, ptr, inbuf, src,dst,tag,count);
	*done = true
	} else{
	*done = false}
	}
*/

/* source, dest, and tag are in range. Message DB and messagePtr have the sent
   data. Need to check that the length of the sent data isn't too long for
   buf. Then copy the data into buf, and fill in the status struct if it
   isn't MPI_STATUS_IGNORE.

   Note: MPI_ANY_{SOURCE,TAG} have been eliminated by caller, so source and tag
   are real values, and should match what is in the message. (If not, then
   the Send and/or Recv have indexed incorrectly into the messageEventRange
   and someone has the wrong messageEvent guid!)
*/
static int recvData(void *buf, int count, MPI_Datatype
               datatype, int source, int dest, int tag, MPI_Comm comm, u64
                  totalSize, MPI_Status *status, ocrGuid_t DB, void *messagePtr)
{
    int ret = MPI_SUCCESS;
    mpiOcrMessageP_t ptr = (mpiOcrMessageP_t) messagePtr;

    PRINTF("recvData: rank #%d: Recved from %d tag:%d, DB %p, ptr %p\n"
           "\tptr->{%lx,%lx,%lx,%lx}\n",
           dest, source, tag, DB, ptr,
           ((u64*)ptr)[0],((u64*)ptr)[1],((u64*)ptr)[2],((u64*)ptr)[3]);  fflush(stdout);

    // Check message header components against the args. If they're wrong,
    // then the system got the wrong message!

    // checked explicitly below    ptr->header.count = count;
    // OK if user is dishonest about datatype as long as the recv size
    // isn't smaller than send size
    // ptr->header.datatype != datatype;

    if (ptr->header.dest != dest)
    {
        printf("recvData: rank #%d: ptr->header.dest=%d, dest=%d.\n", dest, ptr->header.dest, dest);
    }

    assert(ptr->header.source == source);
    assert(ptr->header.dest == dest);
    assert(ptr->header.tag == tag);

    // comm probably doesn't matter as long as src/dest match
    // ptr->header.comm      != comm;
    // OK, checked by count comparison ptr->header.totalSize != totalSize;

    u64 sizeToCopy = MIN(ptr->header.totalSize, totalSize);

    // Make sure receiving buffer is big enough, else warning with truncation
    if (count < ptr->header.count)
        {
            char msg[150];
            sprintf((char*)&msg, "MPI_Recv: rank #%d: count %d < message count %d ",
                    dest, count, ptr->header.count);
            MPI_INFO(msg, ret = MPI_ERR_TRUNCATE);
        }
    else if (totalSize < ptr->header.totalSize)
        {
            char msg[150];
            sprintf((char*)&msg, "MPI_Recv: rank #%d: buf size %d < message size %d ",
                    dest, totalSize, ptr->header.totalSize);
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


/* Do a Recv. Many of the args are just for packing in the message header.
   source and dest are physical - having been mapped through the
   Comm. Totalsize has also been computed.
*/
int mpiOcrRecv(void *buf, int count, /*MPI_Datatype*/ int
               datatype, int source, int dest, int tag, MPI_Comm comm, u64
               totalSize, /*MPI_Status*/ void *status)
{
    const ocrGuid_t messageEventRange = getMessageContext()->messageEventRange;
    ocrGuid_t labeledEvent;

    rankContextP_t rankContext = getRankContext();
    const u32 numRanks = rankContext->numRanks;
    const u32 maxTag = rankContext->maxTag;
    u32 index = guidIndex(source, dest, tag);

    s32 err;
    ocrGuid_t DB;
    void *myPtr;
    u64 dbSize;

#if ! USE_CHANNEL

    // Old style where message is put on the labeled sticky event

    if (err=ocrGuidFromIndex(&labeledEvent, messageEventRange, index))
    {
        char msg[150];
        sprintf((char*)&msg, "mpiOcrRecv:rank:%d, error %d from ocrGuidFromIndex",
                dest, err);
        ERROR(msg); // exits
    }

    PRINTF("mpiOcrRecv: rank #%d: Recving from %d tag:%d on event guid %p\n",
           dest, source, tag, labeledEvent);  fflush(stdout);

    // Block until event is "legal" , and has been satisfied
    if(err=ocrLegacyBlockProgress(labeledEvent, &DB, &myPtr, &dbSize,
                                  LEGACY_PROP_WAIT_FOR_CREATE))
    {
        char msg[150];
        sprintf((char*)&msg, "mpiOcrRecv:rank:%d, error %d from ocrLegacyBlockProgress(%p)",
                dest, err, labeledEvent);
        ERROR(msg); // exits
    }

    // Old style: we've got everything we need
    PRINTF("mpiOcrRecv before recvData: rank #%d: Recved from %d tag:%d on event guid %p, DB %p, ptr %p\n"
           "\tptr->{%lx,%lx,%lx,%lx}\n",
           dest, source, tag, labeledEvent, DB, myPtr,
           ((u64*)myPtr)[0],((u64*)myPtr)[1],((u64*)myPtr)[2],((u64*)myPtr)[3]);  fflush(stdout);

    // Destroy labeledEvent so it can be reused by sender
    ocrEventDestroy(labeledEvent);

    return recvData(buf, count, datatype, source, dest, tag, comm,
                   totalSize, status, DB, myPtr);

#else  // USE_CHANNEL

    // see if the labeled event is in the local hashtable.
    ocrGuid_t * labeledEventPtr = (ocrGuid_t *)mpiHashtableGet(rankContext->labeledEventsCache,
                                                             index);
    if (labeledEventPtr == NULL){
        labeledEvent = NULL_GUID;
    }
    else {
        labeledEvent = *labeledEventPtr;
    }


    if ( ocrGuidIsNull(labeledEvent))
    {
        if (err=ocrGuidFromIndex(&labeledEvent, messageEventRange, index))
        {
            char msg[150];
            sprintf((char*)&msg, "mpiOcrRecv:rank:%d, error %d from ocrGuidFromIndex",
                    dest, err);
            ERROR(msg); // exits
        }

        ocrGuid_t * labeledEventPtr = malloc(sizeof(ocrGuid_t));
        *labeledEventPtr = labeledEvent;
        mpiHashtablePut(rankContext->labeledEventsCache, index,
                      labeledEventPtr);
    }



    PRINTF("mpiOcrRecv: rank #%d: Recving from %d tag:%d on event guid %p\n",
           dest, source, tag, labeledEvent);  fflush(stdout);


    // Block until event is "legal" , and has been satisfied
    if(err=ocrLegacyBlockProgress(labeledEvent, &DB, &myPtr, &dbSize,
                                  LEGACY_PROP_WAIT_FOR_CREATE))
        {
            char msg[150];
            sprintf((char*)&msg, "mpiOcrRecv:rank:%d, error %d from ocrLegacyBlockProgress(%p)",
                    dest, err, labeledEvent);
            ERROR(msg); // exits
        }

    // DB has eventPair - extract the two fields. do the DBrelease in the IF below
    ocrGuid_t channelEvent = ((eventPairP_t) myPtr) -> channelEvent;
    ocrGuid_t localEvent = ((eventPairP_t) myPtr) -> localEvent;

    PRINTF( "mpiOcrRecv: rank #%d , got channel %p from ocrLegacyBlockProgress(%p)\n",
            dest ,channelEvent, labeledEvent);
    fflush(stdout);

    // In order to use the channel, it first has to be added as a dependence
    // on something, that's what gets the next data block in the channel ready
    // to be taken. We only have blockProgress, and you can't add a dependence
    // to THAT. So instead, we create a localEvent (sticky), and add the
    // channelEvent as a dependence on That; and then blockProgress on
    // localEvent. When the channel is satisfied by the sender (or has been and
    // already has the datablock in it), the blockProgress will deliver the DB
    // and pointer.

    // Has to be sticky event because otherwise it might be satisfied immediately
    // by the addDep, and when BlockProgress happens, localEvent will already be gone...

   if (! ocrGuidIsNull(localEvent))
        {
            // The localEvent in the eventPair exists, and has already been
            // "primed" via the channel. This was done in mpiOcrTryRecv.
            // It will be "consumed" below, so null it out in DB and
            // release DB

            ((eventPairP_t) myPtr) -> localEvent = NULL_GUID;
            ocrDbRelease(DB);
            // Now just fall through and use localEvent.
        }
    else
        {
            // localEvent doesn't exist, so have to do some work

            ocrDbRelease(DB);
            if (err=ocrEventCreate(&localEvent, OCR_EVENT_STICKY_T,
                                   EVT_PROP_TAKES_ARG ))
            {
                char msg[150];
                sprintf((char*)&msg, "mpiOcrRecv: rank #%d, error %d from ocrEventCreate(&localEvent)",
                        dest, err);
                ERROR(msg); // exits
            }

            PRINTF("mpiOcrRecv: rank:#%d, created localEvent %p \n",dest, localEvent); fflush(stdout);

            // Unfortunately the RO mode only has effect if destination is an EDT,
            // not an event...
            if (err=ocrAddDependence(channelEvent, localEvent, 0, DB_MODE_RO))
            {
                char msg[150];
                sprintf((char*)&msg, "mpiOcrRecv:rank:%d, error %d from addDep(%p,%p)",
                        dest, err, channelEvent, localEvent);
                ERROR(msg); // exits
            }
        }  // endif ! ocrGuidIsNull

   // Block until event has been satisfied via channel

   if (err=ocrLegacyBlockProgress(localEvent, &DB, &myPtr, &dbSize,
                                  LEGACY_PROP_NONE))
   {
       char msg[150];
       sprintf((char*)&msg, "mpiOcrRecv: rank:#%d, error %d from ocrLegacyBlockProgress on localEvent %p",
               dest, err, localEvent);
       ERROR(msg); // exits
   }

   PRINTF("mpiOcrRecv before recvData: rank #%d: Recved from %d tag:%d on local guid %p, DB %p, ptr %p\n"
          "\tptr->{%lx,%lx,%lx,%lx}\n",
          dest, source, tag, localEvent, DB, myPtr,
          ((u64*)myPtr)[0],((u64*)myPtr)[1],((u64*)myPtr)[2],((u64*)myPtr)[3]);  fflush(stdout);

   // Done with localEvent...
   //ocrEventDestroy(localEvent);

   return recvData(buf, count, datatype, source, dest, tag, comm,
                   totalSize, status, DB, myPtr);
#endif   // USE_CHANNEL

}

/* Try to do a Recv. If the event hasn't been created, return done=FALSE.
   Many of the args are just for packing in the message header.
   source and dest are physical - having been mapped through the
   Comm. Totalsize has also been computed.
 */
int mpiOcrTryRecv(void *buf, int count, /*MPI_Datatype*/ int
                  datatype, int source, int dest, int tag, MPI_Comm comm, u64
                  totalSize, /*MPI_Status*/ void *status, bool *done)
{
#if USE_CHANNEL
    ERROR("MPI-Lite based on channel events does not support MPI_ANY_SOURCE or MPI_ANY_TAG\n"
          "Rebuild apps/apps/libs/src/mpilite by moving to that directory and doing: \n"
          "  make ARCH=x86 USER_FLAGS=-DUSE_CHANNEL=0 clean uninstall install\n");
    // exits
#endif

    const ocrGuid_t messageEventRange = getMessageContext()->messageEventRange;
    ocrGuid_t labeledEvent;

    rankContextP_t rankContext = getRankContext();
    const u32 numRanks = rankContext->numRanks;
    const u32 maxTag = rankContext->maxTag;

    s32 err;
    if (err=ocrGuidFromIndex(&labeledEvent, messageEventRange, guidIndex(source, dest, tag)))
        {
            char msg[150];
            sprintf((char*)&msg, "mpiOcrTryRecv:rank:%d, error %d from ocrGuidFromIndex",
                    dest, err);
            ERROR(msg); // exits
        }


    /* PRINTF("mpiOcrTryRecv: rank #%d: Recving on event guid %p\n",
               source, messageEvent); */

    // If event has been created, stay in BlockProgress, expecting doSend
    // to satisfy the event with the DB. Otherwise, if an error code is the
    // return value, exit immediately and set *done == false
    ocrGuid_t DB;
    void *myPtr;
    u64 dbSize;

#if ! USE_CHANNEL
    // Old style

    if(err=ocrLegacyBlockProgress(labeledEvent, &DB, &myPtr, &dbSize,
                                  LEGACY_PROP_NONE))
    {
        PRINTF("*mpiOcrTryRecv: rank #%d: Recv from %d tag:%d on event guid %p fail:%d\n",
               dest, source, tag, labeledEvent, err);  fflush(stdout);

        *done = FALSE;
        return MPI_SUCCESS;
    }

    PRINTF("mpiOcrTryRecv: rank #%d: Recv from %d tag:%d on event guid %p success\n",
           dest, source, tag, labeledEvent);  fflush(stdout);

    // We've got the message! Don't need event any more - delete. And now
    // extract the data to complete the recv.
    ocrEventDestroy(labeledEvent);

#else  // USE_CHANNEL

// *********** NEED SPECIAL VALUE FROM VINCENT *********
#ifndef LEGACY_PROP_CHECK
#define LEGACY_PROP_CHECK LEGACY_PROP_NONE
#endif

    // SEE mpiOcrRecv for details

    // If use blockProgress(check) to see if labelEvent exists. Do NOT
    // BLOCK. If doesn't exist, there's no message.
    if(err=ocrLegacyBlockProgress(labeledEvent, &DB, &myPtr, &dbSize,
                                  // NEED SPECIAL VALUE FROM VINCENT
                                  LEGACY_PROP_CHECK))
    {
        // There's no created labeled event, so there is certainly
        // nothing to be received. Nothing to clean up, either.
        PRINTF("*mpiOcrTryRecv: rank #%d: Recv from %d tag:%d on labeledEvent guid %p fail:%d\n",
               dest, source, tag, labeledEvent, err);  fflush(stdout);

        *done = FALSE;
        return MPI_SUCCESS;
    }


    // DB has eventPair - extract the two fields. do the DBrelease in the IF below
    ocrGuid_t channelEvent = ((eventPairP_t) myPtr) -> channelEvent;
    ocrGuid_t localEvent = ((eventPairP_t) myPtr) -> localEvent;

    PRINTF( "mpiOcrTryRecv: rank #%d , got channel %p and localEvent %p from ocrLegacyBlockProgress(%p)\n",
            dest ,channelEvent, localEvent, labeledEvent);

    if (ocrGuidIsNull(localEvent))
    {
        // localEvent doesn't exist, so have to do some work

        if (err=ocrEventCreate(&localEvent, OCR_EVENT_STICKY_T,
                               EVT_PROP_TAKES_ARG ))
        {
            char msg[150];
            sprintf((char*)&msg, "mpiOcrTryRecv: rank #%d, error %d from ocrEventCreate(&localEvent)",
                    dest, err);
            ERROR(msg); // exits
        }

        // Store it in the DB on labeledEvent, in case recv fails
        ((eventPairP_t) myPtr) -> localEvent = localEvent;

        PRINTF("mpiOcrTryRecv: rank:#%d, created localEvent %p \n",dest, localEvent); fflush(stdout);

        // Unfortunately the RO mode only has effect if destination is an EDT,
        // not an event...
        if (err=ocrAddDependence(channelEvent, localEvent, 0, DB_MODE_RO))
        {
            char msg[150];
            sprintf((char*)&msg, "mpiOcrTryRecv:rank:%d, error %d from addDep(%p,%p)",
                    dest, err, channelEvent, localEvent);
            ERROR(msg); // exits
        }

    }  // endif ! ocrGuidIsNull


    // Check if there's anything there, satisfied via channel

    ocrGuid_t messageDB;
    void *messagePtr;

    if (err=ocrLegacyBlockProgress(localEvent, &messageDB, &messagePtr, &dbSize,
                                   // NEED SPECIAL VALUE FROM VINCENT
                                   LEGACY_PROP_CHECK))
    {
        PRINTF("*mpiOcrTryRecv: rank #%d: Recv from %d tag:%d on localEvent guid %p fail:%d\n",
               dest, source, tag, localEvent, err);  fflush(stdout);

        *done = FALSE;
        return MPI_SUCCESS;
    }

    PRINTF("mpiOcrTryRecv before recvData: rank #%d: Recved from %d tag:%d on local guid %p, DB %p, ptr %p\n"
           "\tptr->{%lx,%lx,%lx,%lx}\n",
           dest, source, tag, localEvent, messageDB, messagePtr,
           ((u64*)messagePtr)[0],((u64*)messagePtr)[1],((u64*)messagePtr)[2],((u64*)messagePtr)[3]);  fflush(stdout);

    // Note: localEvent was used above and will be destroyed below, so need to NULL
    // it out in the eventPair. Do it NOW so DB can be released ASAP.

    ((eventPairP_t) myPtr) -> localEvent = NULL_GUID;
    ocrDbRelease(DB);

    // Done with localEvent...
    ocrEventDestroy(localEvent);

    // Get right values into DB and myPtr
    DB = messageDB;
    myPtr = messagePtr;

#endif // USE_CHANNEL


    *done = TRUE;

    return recvData(buf, count, datatype, source, dest, tag, comm,
                    totalSize, status, DB, myPtr);
}
