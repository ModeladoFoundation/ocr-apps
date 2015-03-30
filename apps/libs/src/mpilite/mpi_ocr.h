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

// doesn't work? typedef enum( opIsend, opIrecv, opIprobe) nonBlockingOp;
#define OP_ISEND 1
#define OP_IRECV 2
#define OP_IPROBE 3


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
        ocrGuid_t *messageEvents;
        ocrEdtDep_t *messageData;
    } messageContext_t, *messageContextP_t;
  

#define RANK_CONTEXT_SLOT 0
#define MESSAGE_CONTEXT_SLOT 1

    static inline rankContextP_t getRankContext()
    {
        return ((rankContextP_t)( ocrElsUserGet(RANK_CONTEXT_SLOT)));
    }
    messageContextP_t getMessageContext();
    
  
#define MIN(x,y) ((x)<(y)? (x) : (y))

#ifdef __cplusplus
}
#endif

#endif // __MPI_OCR_H__
