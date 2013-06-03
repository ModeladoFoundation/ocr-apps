////////////////////////////////////////////////////////////////////////
// RAG some generic macros to address language restrictions
// RAG and/or generallize some basic operatiosn like memory management
////////////////////////////////////////////////////////////////////////

#if  defined(HAB_C)

#define INLINE
#define HC_UPC_CONST
#define HAB_CONST
#define SHARED

#include "hc.h"

#elif defined(CILK)

#define INLINE inline
#define HC_UPC_CONST const
#define HAB_CONST const
#define SHARED

#include <cilk/cilk.h>

#elif defined(UPC)

#define INLINE inline
#define HC_UPC_CONST
#define HAB_CONST const
#define SHARED shared

#include <upc_strict.h>
#include <upc_collective.h>

#elif defined(FSIM)

#define INLINE inline
#define HC_UPC_CONST const
#define HAB_CONST const
#define SHARED

#else // DEFAULT is C99

#define INLINE inline
#define HC_UPC_CONST const
#define HAB_CONST const
#define SHARED

#endif // HAB_C, CILK, UPC, FSIM or C99

////////////////////////////////////////////////////////////////////////
// RAG some generic macros to address parallelization 
// RAG and stripmapping and tiling in a general manner
////////////////////////////////////////////////////////////////////////

#if   defined(HAB_C)

// RAG good choice might be a small multiple of a cacheline size
// otherwise i think there is to much false sharing or runtime overhead.
#define HAB_C_BLK_SIZE (16)

#define PAR_FOR_0xNx1(index,len, ... ) \
  Index_t index ## _len = (len); \
  Index_t index ## _blk = (HAB_C_BLK_SIZE); \
  for ( Index_t index ## _out=0; index ## _out < index ## _len; index ## _out  += index ## _blk ) { \
    Index_t index ## _end = ( index ## _out + index ## _blk) < index ## _len?( index ## _out + index ## _blk) : index ## _len; \
    async IN (index ## _out, index ## _end, __VA_ARGS__ ) { \
      for ( Index_t index = index ## _out ; index < index ## _end; ++index ) {

#define END_PAR_FOR(index) \
  } } } // forasync(index,len) IN(...)

#define FINISH \
  finish {

#define END_FINISH \
  } // finish

#elif defined(CILK)

#define PAR_FOR_0xNx1(index,len, ... ) \
  cilk_for ( Index_t index = 0 ; index < len ; ++index ) {

#define END_PAR_FOR(index) \
  } // cilk_for (index=0, index<len, ++index) 

#define FINISH {
#define END_FINISH cilk_sync; }

#elif defined(UPC)

#define PAR_FOR_0xNx1(index,len, ... ) \
  for ( Index_t index = 0 ; index < len ; ++index /*; index */ ) {

#define END_PAR_FOR(index) \
  } // upc_forall( index=0 ; index < len ; ++index, index )

#define FINISH {
#define END_FINISH upc_barrier;}

#elif defined(FSIM)

#define PAR_FOR_0xNx1(index,len, ... ) \
  for ( Index_t index = 0 ; index < len ; ++index ) {

#define END_PAR_FOR(index) \
  } // c99 for( index=0 ; index < len ; ++index )

#define FINISH {
#define END_FINISH }

#else // DEFAULT is C99

#define PAR_FOR_0xNx1(index,len, ... ) \
  for ( Index_t index = 0 ; index < len ; ++index ) {

#define END_PAR_FOR(index) \
  } // c99 for( index=0 ; index < len ; ++index )

#define FINISH {
#define END_FINISH }

#endif // HAB_C, CILK, UPC, FSIM or C99
