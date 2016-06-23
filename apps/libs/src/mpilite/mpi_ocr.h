/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */


#ifndef __MPI_OCR_H__
#define __MPI_OCR_H__

#ifdef __cplusplus
extern "C" {
#endif

// mpi_ocr.h must be included before any ocr*.h files.

// Turn on legacy capabilities of OCR.
#define ENABLE_EXTENSION_LEGACY 1
#define ENABLE_EXTENSION_RTITF 1
#define ENABLE_EXTENSION_LABELING  1
#define ENABLE_EXTENSION_AFFINITY  1
#define ENABLE_EXTENSION_CHANNEL_EVT 1
#define ENABLE_EXTENSION_PARAMS_EVT 1

// 12/9/15 ocrDbDestroy neglects to Release DBs for mpilite, even though
// it is supposed to. bug 879. So temporarily, add them.
#ifndef DESTROY_NEEDS_RELEASE
    #define DESTROY_NEEDS_RELEASE 1
#endif

#include <ocr-types.h>
#include "mpi.h"
#include "mpi_ocr_hashtable.h"

/****************************************************/
/* MPI_OCR                                          */
/****************************************************/

/**
 * @brief Abstracts the interface between MPI and OCR
 *
 * Until all legacy support is implemented in OCR, it will have to be
 * simulated here.
 */

// debugging for mpi_ocr{,_messaging}.c, mpilite.c
#ifndef DEBUG_MPI
    #define DEBUG_MPI 0
#endif

// Defines to simulate OCR tagging for events, and ocrLegacyBlockProgress
// for datablocks

// Can define as 0 on compile line to test: -DEVENT_ARRAY=0
// or set to 0 to turn off permanently in favor of Labeled Guids
// 11-aug-2105: turn on labeled GUIDs or turn off the Event Array.
// 22-sept-2015: Temporarily turn off labeled GUIDS for the Workshop.
// 10-Nov-2015: Turn labeled GUIDs back on
#ifndef EVENT_ARRAY
    #define EVENT_ARRAY 0   // Labeled guids is ON
#endif

/* 4/21/15, ocrLegacyBlockProgress() has been working for a while, turn
       off the DB Array.
// #define DB_ARRAY 1
*/

// Include this AFTER all defines have been done.
// This is used for u64 and friends
#include <ocr-types.h>

// Values for MPI_Request->op
typedef enum {
    opIsend,
    opIrecv
} nonBlockingOp;

// Values for MPI_Request->status
#define FFWD_MQ_MAX             (0)
#define FFWD_MQ_NULL            (-1)
#define FFWD_MQ_INIT            (-2)
#define FFWD_MQ_SEND            (-3)
#define FFWD_MQ_SEND_MATCHED    (-4)    // used for push-only qeng, but not necessarily enabled
#define FFWD_MQ_RECV            (-5)
#define FFWD_MQ_RECV_MATCHED    (-6)    // used for pull-only spad. mandatory.
#define FFWD_MQ_DONE            (-7)
#define FFWD_MQ_MIN             (-8)


typedef struct mpiOcrMessage_t
{
    struct // same args as send, plus source and totalSize
    {
        int count;
        /*MPI_Datatype*/ int datatype;
        int source;
        int dest;
        int tag;
        MPI_Comm comm;
        u64 totalSize;
    } header;
    u64 data[];
} mpiOcrMessage_t, *mpiOcrMessageP_t;



typedef struct rankContext_t
{
    u32 rank, numRanks, maxTag, mpiInitialized;
    bool aggressiveNB; // aggressive non-blocking implementation
    unsigned char sizeOf[17];  // sizeof each datatype
    hashtable_t * channelEventsCache;  // local hash table

    u32 maxComm,    // communicators range from 0..maxComm
        commArrayLen; // current length of the "communicators" array ( > maxComm)
    MPI_Comm * communicators;  // array of ptrs to communicator structure
} rankContext_t, *rankContextP_t;

typedef struct messageContext_t
{
#if  EVENT_ARRAY
    // 3 D array of events
    ocrGuid_t *messageEvents;
#endif

    // Range of event guids
    ocrGuid_t messageEventRange;

#ifdef DB_ARRAY
    ocrEdtDep_t *messageData;
#endif
} messageContext_t, *messageContextP_t;

typedef struct globalDBContext_t
{
    ocrGuid_t dbGuid;
    void * addrPtr;
} globalDBContext_t, *globalDBContextP_t;


#include <extensions/ocr-runtime-itf.h>

#define RANK_CONTEXT_SLOT 0
#define MESSAGE_CONTEXT_SLOT 1
#define GLOBAL_DB_SLOT 2

typedef union {
    void * ptr;
    ocrGuid_t guid;
} elsUnion;

static inline rankContextP_t getRankContext() {
    elsUnion u;
    u.guid = ocrElsUserGet(RANK_CONTEXT_SLOT);
    return ((rankContextP_t)(u.ptr));
}

static inline messageContextP_t getMessageContext() {
    elsUnion u;
    u.guid = ocrElsUserGet(MESSAGE_CONTEXT_SLOT);
    return ((messageContextP_t)(u.ptr));
}


int mpiOcrSend(void *buf, int count, /*MPI_Datatype*/ int
               datatype, int source, int dest, int tag, MPI_Comm  comm, u64 totalSize);

int mpiOcrTrySend(void *buf, int count, /*MPI_Datatype*/ int
               datatype, int source, int dest, int tag, MPI_Comm comm, u64
                  totalSize, bool *done);

int mpiOcrRecv(void *buf, int count, /*MPI_Datatype*/ int
               datatype, int source, int dest, int tag, MPI_Comm comm, u64
               totalSize, /*MPI_Status*/ void *status);

int mpiOcrTryRecv(void *buf, int count, /*MPI_Datatype*/ int
                  datatype, int source, int dest, int tag, MPI_Comm comm, u64
                  totalSize, /*MPI_Status*/ void *status, bool *done);

u64 guidIndex(int source, int dest, int tag);


/* prints error and exits program */
void ERROR(char *s);

/* prints warning */
void WARNING(char *s);

int __mpi_ocr_TRUE(void);

#define MIN(x,y) ((x)<(y) ? (x) : (y) )

#ifdef __cplusplus
}
#endif

#endif // __MPI_OCR_H__
