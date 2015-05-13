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

/****************************************************/
/* MPI_OCR                                          */
/****************************************************/

/**
 * @brief Abstracts the interface between MPI and OCR
 *
 * Until all legacy support is implemented in OCR, it will have to be
 * simulated here.
 */

// mpi_ocr.h must be included before any ocr*.h files.

// Turn on legacy capabilities of OCR.
#define ENABLE_EXTENSION_LEGACY 1
#define ENABLE_EXTENSION_RTITF 1

// Defines to simulate OCR tagging for events, and ocrLegacyBlockProgress
// for datablocks

// Can define as 0 on compile line to test: -DEVENT_ARRAY=0
#ifndef EVENT_ARRAY
    #define EVENT_ARRAY 1
#endif

/* 4/21/15, ocrLegacyBlockProgress() has been working for a while, turn
       off the DB Array.
// #define DB_ARRAY 1
*/

#if ! EVENT_ARRAY
    // use OCR labeling
    #define ENABLE_EXTENSION_LABELING  1
#endif

// Include this AFTER all defines have been done.
// This is used for u64 and friends
#include <ocr-types.h>

// Values for MPI_Request->op
// doesn't work? typedef enum( opIsend, opIrecv, opIprobe) nonBlockingOp;
#define OP_ISEND    1
#define OP_IRECV    2
#define OP_IPROBE   3

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
        /*MPI_Comm*/ int comm;
        u64 totalSize;
    } header;
    u64 data[];
} mpiOcrMessage_t, *mpiOcrMessageP_t;



typedef struct rankContext_t
{
    u32 rank, numRanks, maxTag, mpiInitialized;
    unsigned char sizeOf[17];  // sizeof each datatype

    u32 maxComm,    // communicators range from 0..maxComm
        commArrayLen; // current length of the "communicators" array ( > maxComm)
    void ** communicators;  // array of ptrs to communicator structure
} rankContext_t, *rankContextP_t;

typedef struct messageContext_t
{
#if  EVENT_ARRAY
    // 3 D array of events
    ocrGuid_t *messageEvents;
#else
    // 3 D map of event guid s
    ocrGuid_t messageEventMap;
#endif
#ifdef DB_ARRAY
    ocrEdtDep_t *messageData;
#endif
} messageContext_t, *messageContextP_t;

#include <extensions/ocr-runtime-itf.h>

#define RANK_CONTEXT_SLOT 0
#define MESSAGE_CONTEXT_SLOT 1

static inline rankContextP_t getRankContext() {
    return ((rankContextP_t)( ocrElsUserGet(RANK_CONTEXT_SLOT)));
}
static inline messageContextP_t getMessageContext() {
    return ((messageContextP_t)( ocrElsUserGet(MESSAGE_CONTEXT_SLOT)));
}

int __mpi_ocr_TRUE(void);

#define MIN(x,y) ((x)<(y) ? (x) : (y) )

#ifdef __cplusplus
}
#endif

#endif // __MPI_OCR_H__
