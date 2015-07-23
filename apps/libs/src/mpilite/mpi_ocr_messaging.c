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
#include <sys/time.h>
#include <mpi_ocr.h>  // must come before ocr.h, as it sets defines
#include <ocr.h>

#include <extensions/ocr-legacy.h>
#include <extensions/ocr-labeling.h>

#include <mpi.h>  // get some decls and error codes

#define MPI_INFO(s,w) {PRINTF("WARNING: %s, will return" #w "\n",(s));}

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
	Fill in header, and copy buffer
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

    ocrDbCreate(&DB, (void **)&ptr, totalSize + sizeof(mpiOcrMessage_t),
                DB_PROP_NONE, NULL_GUID, NO_ALLOC);
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

    // OK, Send the DB. [It's only going to be used in DB_MODE_CONST by the
    // receiver, but you can only say that when adding a dependence
    // directly to an EDT; not to an Event....]
    ocrEventSatisfy(messageEvent, DB);

    return MPI_SUCCESS;
}


/* Do a Send. Many of the args are just for packing in the message header.
   source and dest are physical - having been mapped through the
   Comm. Totalsize has also been computed.
*/
int mpiOcrSend(void *buf, int count, /*MPI_Datatype*/ int
               datatype, int source, int dest, int tag, MPI_Comm comm, u64 totalSize)
{
    const ocrGuid_t messageEventMap = getMessageContext()->messageEventMap;
    ocrGuid_t messageEvent;

    s64 indexes[3] = {source, dest, tag};
    s32 err;

    if (err=ocrGuidFromLabel(&messageEvent, messageEventMap, indexes))
        {
            char msg[150];
            sprintf((char*)&msg, "mpiOcrSend:rank:%d, error %d from ocrGuidFromLabel",
                    source, err);
            ERROR(msg); // exits
        }

     PRINTF("mpiOcrSend: rank #%d: Sending to %d tag:%d on event guid %ld\n",
            source,  dest, tag, messageEvent); fflush(stdout);

    // Block until it is "legal" to create or recreate an event using the
    // messageEvent guid.
    if (err=ocrEventCreate(&messageEvent, OCR_EVENT_STICKY_T,
                   EVT_PROP_TAKES_ARG | GUID_PROP_BLOCK | GUID_PROP_IS_LABELED))
        {
            char msg[150];
            sprintf((char*)&msg, "mpiOcrSend:rank:%d, error %d from ocrEventCreate(%ld)",
                    source, err, messageEvent);
            ERROR(msg); // exits
        }

    // We've got a valid event, so do the send
    return sendData(buf, count, datatype, source, dest, tag, comm,
                    totalSize, messageEvent);
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
    const ocrGuid_t messageEventMap = getMessageContext()->messageEventMap;
    ocrGuid_t messageEvent;

    s64 indexes[3] = {source, dest, tag};
    u32 err;

    if (err=ocrGuidFromLabel(&messageEvent, messageEventMap, indexes))
        {
            char msg[150];
            sprintf((char*)&msg, "mpiOcrTrySend:rank:%d, error %d from ocrGuidFromLabel",
                    source, err);
            ERROR(msg); // exits
        }


    // Don't block if messageEvent already exists (error will be
    // returned). Send will have to be done some other time.
    if (err=ocrEventCreate(&messageEvent, OCR_EVENT_STICKY_T,
                   EVT_PROP_TAKES_ARG | GUID_PROP_IS_LABELED | GUID_PROP_CHECK))
        {
            PRINTF("*mpiOcrTrySend: rank #%d: Sending to %d tag:%d on event guid %ld: fail:%d\n",
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
                    sprintf((char*)&msg, "mpiOcrTrySend:rank:%d, error %d from ocrEventCreate(%ld)",
                            source, err, messageEvent);
                    ERROR(msg); // exits
                }
        }
    PRINTF("mpiOcrTrySend: rank #%d: Sending to %d tag:%d on event guid %ld: success\n",
           source, dest, tag, messageEvent);  fflush(stdout);

    *done = TRUE;
    // We've got a valid event, so do the send
    return sendData(buf, count, datatype, source, dest, tag, comm,
                           totalSize, messageEvent);
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
	vecvData(DB, ptr, inbuf, src,dst,tag,count);
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
   the Send and/or Recv have indexed incorrectly into the messageEventMap
   and someone has the wrong messageEvent guid!)
*/
static int recvData(void *buf, int count, MPI_Datatype
               datatype, int source, int dest, int tag, MPI_Comm comm, u64
                  totalSize, MPI_Status *status, ocrGuid_t DB, void *messagePtr)
{
    int ret = MPI_SUCCESS;
    mpiOcrMessageP_t ptr = (mpiOcrMessageP_t) messagePtr;

    // Check message header components against the args. If they're wrong,
    // then the system got the wrong message!

    // checked explicitly below    ptr->header.count = count;
    // OK if user is dishonest about datatype as long as the recv size
    // isn't smaller than send size
    // ptr->header.datatype != datatype;

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
    ocrDbDestroy(DB);

    return ret;
}


/* Do a Recv. Many of the args are just for packing in the message header.
   source and dest are physical - having been mapped through the
   Comm. Totalsize has also been computed.
*/
int mpiOcrRecv(void *buf, int count, /*MPI_Datatype*/ int
               datatype, int source, int dest, int tag, /*MPI_Comm*/ int comm, u64
               totalSize, /*MPI_Status*/ void *status)
{
    const ocrGuid_t messageEventMap = getMessageContext()->messageEventMap;
    ocrGuid_t messageEvent;

    s64 indexes[3] = {source, dest, tag};
    s32 err;

    if (err=ocrGuidFromLabel(&messageEvent, messageEventMap, indexes))
        {
            char msg[150];
            sprintf((char*)&msg, "mpiOcrRecv:rank:%d, error %d from ocrGuidFromLabel",
                    dest, err);
            ERROR(msg); // exits
        }

    PRINTF("mpiOcrRecv: rank #%d: Recv from %d tag:%d on event guid %ld\n",
           dest, source, tag, messageEvent);  fflush(stdout);

    /* PRINTF("mpiOcrRecv: rank #%d: Recving on event guid %ld\n",
               source, messageEvent); */

    // Block until event is "legal" , and has been satisfied
    ocrGuid_t DB;
    void *myPtr;
    u64 dbSize;

    if(err=ocrLegacyBlockProgress(messageEvent, &DB, &myPtr, &dbSize,
                              LEGACY_PROP_WAIT_FOR_CREATE))
        {
            char msg[150];
            sprintf((char*)&msg, "mpiOcrRecv:rank:%d, error %d from ocrLegacyBlockProgress(%ld)",
                    dest, err, messageEvent);
            ERROR(msg); // exits
        }
    ocrEventDestroy(messageEvent);

    return recvData(buf, count, datatype, source, dest, tag, comm,
                    totalSize, status, DB, myPtr);
}

/* Try to do a Recv. If the event hasn't been created, return done=FALSE.
   Many of the args are just for packing in the message header.
   source and dest are physical - having been mapped through the
   Comm. Totalsize has also been computed.
 */
int mpiOcrTryRecv(void *buf, int count, /*MPI_Datatype*/ int
                  datatype, int source, int dest, int tag, /*MPI_Comm*/ int comm, u64
                  totalSize, /*MPI_Status*/ void *status, bool *done)
{
    const ocrGuid_t messageEventMap = getMessageContext()->messageEventMap;
    ocrGuid_t messageEvent;

    s64 indexes[3] = {source, dest, tag};
    s32 err;

    if (err=ocrGuidFromLabel(&messageEvent, messageEventMap, indexes))
        {
            char msg[150];
            sprintf((char*)&msg, "mpiOcrTryRecv:rank:%d, error %d from ocrGuidFromLabel",
                    dest, err);
            ERROR(msg); // exits
        }


    /* PRINTF("mpiOcrTryRecv: rank #%d: Recving on event guid %ld\n",
               source, messageEvent); */

    // If event has been created, stay in BlockProgress, expecting doSend
    // to satisfy the event with the DB. Otherwise, if an error code is the
    // return value, exit immediately and set *done == false
    ocrGuid_t DB;
    void *myPtr;
    u64 dbSize;

    if(err=ocrLegacyBlockProgress(messageEvent, &DB, &myPtr, &dbSize,
                              LEGACY_PROP_NONE))
        {
    PRINTF("*mpiOcrTryRecv: rank #%d: Recv from %d tag:%d on event guid %ld fail:%d\n",
           dest, source, tag, messageEvent, err);  fflush(stdout);

            *done = FALSE;
            return MPI_SUCCESS;
        }

    PRINTF("mpiOcrTryRecv: rank #%d: Recv from %d tag:%d on event guid %ld success\n",
           dest, source, tag, messageEvent);  fflush(stdout);

    // We've got the message! Don't need event any more - delete. And now
    // extract the data to complete the recv.
    ocrEventDestroy(messageEvent);

    *done = TRUE;

    return recvData(buf, count, datatype, source, dest, tag, comm,
                    totalSize, status, DB, myPtr);
}
