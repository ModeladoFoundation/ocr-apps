/*
* This file is subject to the license agreement located in the file LICENSE
* and cannot be distributed without it. This notice cannot be
* removed or modified.
*/

#ifndef MPILITE_H
#define MPILITE_H

#if 0
// ------ threading model ------
#define MPILITE_THREAD_MODEL
#else
// ------ process model --------
#define MPILITE_REQUEST_SHADOWING
#define MPILITE_INTERMEDIATE_BUFFER
#define MPILITE_SHARED_HEAP
#define MPILITE_ASYNC_COLLECTIVE
#define MPILITE_TEMP
//#define MPILITE_REFCNT_MESSAGE
#endif

//#define MPILITE_STATS
//#define MPILITE_FORTRAN

#define MPILITE_RANK		(8)			// number of rank
#define MPILITE_COMMBUFF	(256)			// pool size in MB
#define MPILITE_TAG_MAX		(~(unsigned)0 >> 1)	// inclusive

#ifdef MPILITE_THREAD_MODEL
#define __THREAD	__thread
#else
#define __THREAD
#endif

extern __THREAD int mygpid;
extern int mpilite_rank;
extern int mpilite_commbuff;
extern void *mpilite_pool;

// This adds rank number to each printf for debugging
//#define printf(format, ...)   printf("[%d]" format, mygpid, ##__VA_ARGS__)

#endif
