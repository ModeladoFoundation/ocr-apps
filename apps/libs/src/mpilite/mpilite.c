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
#include "mpi.h"
#include <malloc.h>

#define MPI_WARNING(s,w) {PRINTF("WARNING: %s, returning" #w "\n",(s)); return (w);}

// Have to make sure to drag in mainEdt else mpi_ocr.o does not get
// included in the linked object. We assume at least one of these MPI
// routines will be called, causing miplite.o to be included in the link.
void mainEdt(void);
void __drag_in_mainEdt(void){mainEdt();}
    
    

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
#if 0
            if (MPI_ANY_SOURCE == source && MPI_ANY_TAG == tag){
                int done = FALSE;
                ocrGuid_t *eventP;
                
                while(!done){
                    for (u32 source = 0; source < numRanks; source++){
                        for (u32 tag = 0; tag <= maxTag; tag++){
                            eventP =
                                &(messageContext->messageEvents[index(source, dest, tag, numRanks, maxTag)]);
                            if (NULL_GUID != eventP){
                                done = TRUE;
                                break;
                            }
                        }
                        if (done){
                            break;
                        }
                    }
                }
                else if (MPI_ANY_SOURCE == source){
#endif
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
