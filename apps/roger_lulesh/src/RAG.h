////////////////////////////////////////////////////////////////////////
// RAG some generic macros to address language restrictions
// RAG and/or generallize some basic operatiosn like memory management
////////////////////////////////////////////////////////////////////////

#if  defined(HAB_C)

#define INLINE
#define HC_UPC_CONST
#define HAB_CONST

#define SHARED
#define FREE(ptr) free(ptr)
#define DRAM_MALLOC(count,sizeof_type) malloc((count)*(sizeof_type))

#include "hc.h"
#include <math.h>

#elif defined(CILK) // CILK

#define INLINE inline
#define HC_UPC_CONST const
#define HAB_CONST const

#define SHARED
#define FREE(ptr) free(ptr)
#define DRAM_MALLOC(count,sizeof_type) malloc((count)*(sizeof_type))

#include <cilk/cilk.h>

#include <math.h>

#elif defined(UPC) // UPC

#define INLINE inline
#define HC_UPC_CONST
#define HAB_CONST const

#define SHARED shared
#define FREE(ptr) upc_free(ptr)
#define DRAM_MALLOC(count,sizeof_type) upc_global_alloc((count),(sizeof_type))

#include <upc_strict.h>
#include <upc_collective.h>

#include <math.h>

#else // NOT HAB_C, CILK or UPC, i.e. C99

#define INLINE inline
#define HC_UPC_CONST const
#define HAB_CONST const

#define SHARED
#define FREE(ptr) free(ptr)
#define DRAM_MALLOC(count,sizeof_type) malloc((count)*(sizeof_type))

#include <math.h>

#endif //    HAB_C, CILK, UPC or C99

////////////////////////////////////////////////////////////////////////
// RAG some generic macros to address parallelization 
// RAG and stripmapping and tiling in a general manner
////////////////////////////////////////////////////////////////////////

#ifdef       HAB_C

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

#else // NOT HAB_C or CILK, i.e. C99

#define PAR_FOR_0xNx1(index,len, ... ) \
  for ( Index_t index = 0 ; index < len ; ++index ) {

#define END_PAR_FOR(index) \
  } // c99 for( index=0 ; index < len ; ++index )

#define FINISH {
#define END_FINISH }

#endif //    HAB_C, CILK, UPC or C99
