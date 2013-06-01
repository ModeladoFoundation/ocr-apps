// RAG better choice might be a small multiple of a cacheline size
// otherwise i think there is to much false sharing or runtiem overhead.

#ifdef HAB_C

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

#else // NOT HAB_C or CILK, i.e. C99

#define C99_BLK_SIZE (16)

#define PAR_FOR_0xNx1(index,len, ... ) \
    for ( Index_t index = 0 ; index < len ; ++index ) {

#define END_PAR_FOR(index) \
  } // c99 for( index=0 ; index < len ; ++index )

#define FINISH {
#define END_FINISH }

#endif
