#ifndef _C99OCR_H
#define _C99OCR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ocr.h>
#include <inttypes.h>
#include <assert.h>
#include <alloca.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

/*
 * c99 on OCR support
 * Roger Golliver 2015
 *
 * pointers -- ocrPtr_t
 *
 * memory management -- ocrPtrMalloc(), ocrPtrFree()
 *
 * global data -- (lhs|rhs)gblOcrPtr()
 *
 * ABI calling convention support -- __continuation_args_t and __continuation_guids_t
 *
 * parallel loops -- iter_work(), iter_wait()
 *
 */

#ifdef DEBUG
#if 0
#define CHECK_RET_VAL(str,val) {printf("%s __ret_val = %d\n",(str),(val)); fflush(stdout);}
#else
#define CHECK_RET_VAL(str,val) assert((val)==0)
#endif
#else
#define CHECK_RET_VAL(str,val) assert((val)==0)
#endif
#ifdef TRACE
#define TRACE0(str) {printf("%s\n",str);fflush(stdout);}
#define TRACE1(str) {printf("    %s\n",str);fflush(stdout);}
#else
#define TRACE0(str)
#define TRACE1(str)
#endif

/*
 * pointers
 */

typedef struct ocrPtr_t { size_t offset; ocrGuid_t guid; } ocrPtr_t;
const static ocrPtr_t __ocrNULL  = {	.offset = (size_t)NULL,
					.guid   = NULL_GUID_INITIALIZER };

/*
 * memory management
 */

static void *ocrPtrMalloc(ocrPtr_t *ocrPtr, size_t size) {
   int __ret_val;
   void *ptr = NULL;
   ocrPtr->offset = (size_t)NULL;
   __ret_val = ocrDbCreate( &ocrPtr->guid, &ptr, size, DB_PROP_NONE, NULL_HINT, NO_ALLOC );
   CHECK_RET_VAL("ocrPtrMalloc()->ocrDbCreate",__ret_val);
#ifdef DEBUG
printf("ocrPtrMalloc -- guid:base:offset %16.16" PRIx64 ":%p:%16.16" PRIx64 "\n",
       ocrPtr->guid, ptr, ocrPtr->offset);fflush(stdout);
#endif
   return ptr + ocrPtr->offset;
} /* ocrPtrMalloc() */

static void ocrPtrFree(ocrPtr_t ocrPtr ) {
  int __ret_val;
  __ret_val = ocrDbRelease( ocrPtr.guid );
  CHECK_RET_VAL("ocrPtrFree-ocrDbRelease",__ret_val);
  __ret_val = ocrDbDestroy( ocrPtr.guid );
  CHECK_RET_VAL("ocrPtrFree-ocrDbDestroy",__ret_val);
  return;
} /* ocrPtrFree() */

/*
 * global data
 */

static inline ocrPtr_t ocrPtrFromSelectorOffset(ocrGuid_t selector, size_t offset) {
    ocrPtr_t ptr = { .offset = offset, .guid = selector };
    return ptr;
} /* ocrPtrFromSelectorOffset() */

#define __rhsGblOcrPtr(type,ocrPtr) (*(type *)__gblOcrPtr(ocrPtr))
#define __lhsGblOcrPtr(type,ocrPtr) (*(type *)__gblOcrPtr(ocrPtr))

static inline void *__gblOcrPtr(ocrPtr_t ocrPtr) {
    /* NULL_GUID is used for globals */
    assert( IS_GUID_NULL( ocrPtr.guid ) );
#ifdef DEBUG
printf("gblOcrPtr -- guid:base:offset %16.16" PRIx64 ":%p:%16.16" PRIx64 "\n",
       ocrPtr.guid, NULL, ocrPtr.offset);fflush(stdout);
#endif
    return NULL + ocrPtr.offset;
} /* gblOcrPtr() */

static inline void *castablePtrFromSegmentOffset(ocrEdtDep_t segment, size_t offset) {
/* NULL_GUID is for globals, C's NULL is ocrPtr (NULL_GUID:NULL) */
#ifdef DEBUG
printf("castablePtrFromSegmentOffset -- guid:base:offset %16.16" PRIx64 ":%p:%16.16" PRIx64 "\n",segment.guid,segment.ptr,offset);fflush(stdout);
#endif
    void *ptr = ( IS_GUID_NULL( segment.guid) ) ? NULL + offset : segment.ptr + offset;
    return ptr;
} /* castablePtrFromSegmentOffset() */

/*
 * ABI calling convention support
 */

#define WORDS_continuation_args_t ((int)((2*(sizeof(ocrPtr_t))+2*sizeof(size_t)+2*sizeof(uint32_t)+sizeof(uint64_t)-1)/sizeof(uint64_t)))
typedef union __continuation_args_t {
  struct { ocrPtr_t new_pc, old_pc; size_t new_frame, old_frame; uint32_t paramc, depc; } context;
  uint64_t paramv[WORDS_continuation_args_t] ;
} __continuation_args_t;

#define WORDS_continuation_guids_t ((int)((2*sizeof(ocrGuid_t)+sizeof(ocrGuid_t)-1)/sizeof(ocrGuid_t)))
typedef union __continuation_guids_t {
  struct { ocrGuid_t    new_frame, old_frame; } selector;
  struct { ocrEdtDep_t  new_frame, old_frame; } segment;
  ocrGuid_t  guids[WORDS_continuation_guids_t];
  ocrEdtDep_t depv[WORDS_continuation_guids_t];
} __continuation_guids_t;

/*
 * parallel loop support
 */

/*
 * void __iter_work(..., int first, int last, int inc, int blk)(...);
 */

#define WORDS_iter_work_args_t ((int)((4*sizeof(int)+sizeof(uint64_t)-1)/sizeof(uint64_t)))
typedef union __iter_work_args_t {
  struct { int first, last, inc, blk; } vars;
  uint64_t paramv[WORDS_iter_work_args_t];
} __iter_work_args_t;

ocrGuid_t __iter_work( uint32_t __paramc, __continuation_args_t  *__paramv,
                       uint32_t __depc,   __continuation_guids_t *__depv );
ocrPtr_t  __template_iter_work = { .offset = (size_t)__iter_work, .guid = NULL_GUID_INITIALIZER };

/*
 * void __iter_wait(...)(..., ocrGuid_t finish_event);
 */

#define WORDS_iter_wait_guids_t ((int)((1*sizeof(ocrGuid_t)+sizeof(ocrGuid_t)-1)/sizeof(ocrGuid_t)))
typedef union __iter_wait_guids_t {
  struct { ocrGuid_t    finish_event; } selector;
  struct { ocrEdtDep_t  finish_event; } segment;
  ocrGuid_t  guids[WORDS_iter_wait_guids_t];
  ocrEdtDep_t depv[WORDS_iter_wait_guids_t];
} __iter_wait_guids_t;

ocrGuid_t __iter_wait( uint32_t __paramc, __continuation_args_t  *__paramv,
                       uint32_t __depc,   __continuation_guids_t *__depv );
ocrPtr_t  __template_iter_wait = { .offset = (size_t)__iter_wait, .guid = NULL_GUID_INITIALIZER };

//-------------------------------------------------------------------
// void __iter_work(..., int first, int last, int inc, int blk)(...);
//-------------------------------------------------------------------

ocrGuid_t __iter_work( uint32_t __paramc, __continuation_args_t  *__paramv,
                       uint32_t __depc,   __continuation_guids_t *__depv ) {
  int __ret_val;

  assert( IS_GUID_EQUAL(__paramv->context.new_pc.guid, __template_iter_work.guid) );
  assert( __paramv->context.new_pc.offset == __ocrNULL.offset );
  assert( !IS_GUID_EQUAL(__paramv->context.old_pc.guid, __ocrNULL.guid) );
  assert( ((int64_t)__paramv->context.old_pc.offset) >= ((int64_t)NULL) ); // RAG -- new check
  assert( ((int64_t)__paramv->context.new_pc.offset) >= ((int64_t)NULL) ); // RAG -- new check

  __iter_work_args_t *__iter_work_paramv =
 (__iter_work_args_t *)  &__paramv->paramv[__paramc - WORDS_iter_work_args_t];
  assert(__iter_work_paramv->vars.first == 0);
  assert(__iter_work_paramv->vars.last  != __iter_work_paramv->vars.first);
  assert(__iter_work_paramv->vars.inc   == 1);
  assert(__iter_work_paramv->vars.blk   == -1);
#ifdef DEBUG
  printf("Enter __iter_work (paramc = %d, depc = %d) %s\n",
         __paramc, __depc,
         IS_GUID_NULL(__depv->segment.new_frame.guid) ? "new frame" : "reuse frame" );fflush(stdout);
#else
//putchar('<');fflush(stdout);
#endif

#define WORDS_iter_wait_locals_t ((sizeof(__iter_wait_locals_t)+sizeof(uint64_t)-1)/sizeof(uint64_t))
  typedef struct __continuation_locals_t {
  /* paramv */
    __continuation_args_t paramv;
  /* depv */
    __continuation_guids_t depv;
  /* parameters */
  /* locals */
  /* more locals */
  } __iter_wait_locals_t;

  __iter_wait_locals_t *__frame =
 (__iter_wait_locals_t *) castablePtrFromSegmentOffset( __depv->segment.new_frame,
                                                        __paramv->context.new_frame);
#ifdef DEBUG
printf("new_pc      %16.16" PRIx64 ":%16.16" PRIx64 "\n",
       __paramv->context.new_pc.guid,
       __paramv->context.new_pc.offset);        fflush(stdout);
printf("old_pc      %16.16" PRIx64 ":%16.16" PRIx64 "\n",
       __paramv->context.old_pc.guid,
       __paramv->context.old_pc.offset);        fflush(stdout);
printf("old_frame   %16.16" PRIx64 ":%16.16" PRIx64 "\n",
       __depv->segment.old_frame.guid,
       __paramv->context.old_frame);            fflush(stdout);
printf("new_frame   %16.16" PRIx64 ":%16.16" PRIx64 "\n",
       __depv->segment.new_frame.guid,
       __paramv->context.new_frame);            fflush(stdout);
printf("paramc      %8" PRIu32 "\n", __paramc); fflush(stdout);
printf("depc        %8" PRIu32 "\n", __depc);   fflush(stdout);
#endif

  assert( ((int64_t)__paramv->context.old_pc.offset) >= ((int64_t)NULL) ); // RAG -- new check
  assert( ((int64_t)__paramv->context.new_pc.offset) >= ((int64_t)NULL) ); // RAG -- new check

  goto *(void *)( &&__iter_work_offset_zero + __paramv->context.new_pc.offset );
/*---------------------------------------------------------------------------------------*/
__iter_work_offset_zero:
TRACE1("__iter_work()@offset_zero\n");

TRACE1("__iter_work about to pass params on to top_of_loop");

  __paramv->context.new_pc = __paramv->context.old_pc;
  __paramv->context.old_pc = __frame->paramv.context.old_pc; /* RAG was ocrNULL */

TRACE1("__iter_work about to pass guids on to top_of_loop");
  __continuation_guids_t *__guids = (__continuation_guids_t *) ALLOC( ( __depc )*sizeof(ocrGuid_t) );
        CHECK_RET_VAL("guids ALLOC",(__guids == NULL));
	/* depv */
        for(int i=0;i<__depc;i++)
          __guids->guids[i] = __depv->depv[i].guid;

TRACE1("__iter_work about to create edt for top_of_loop");
  ocrGuid_t __edt_top_of_loop_guid;
  __ret_val = ocrEdtCreate(&__edt_top_of_loop_guid, __paramv->context.new_pc.guid,
                         __paramc, &__paramv->paramv[0],
                         __depc,   &__guids->guids[0],
                         EDT_PROP_NONE, NULL_HINT, NULL );
  CHECK_RET_VAL("__iter_work()->ocrEdtCreate",__ret_val);

TRACE1("__iter_work()<>offset_zero");
  FREE(__guids);
  return NULL_GUID;
} /* end __iter_work() */


//----------------------------------------------------------------------------
// void __iter_wait(...)(..., ocrGuid_t finish_event);
//----------------------------------------------------------------------------

ocrGuid_t __iter_wait( uint32_t __paramc, __continuation_args_t  *__paramv,
                       uint32_t __depc,   __continuation_guids_t *__depv ) {
  int __ret_val;

  assert( IS_GUID_EQUAL(__paramv->context.new_pc.guid, __template_iter_wait.guid) );
  assert( __paramv->context.new_pc.offset == __ocrNULL.offset );
  assert( !IS_GUID_EQUAL(__paramv->context.old_pc.guid, __ocrNULL.guid) );
  assert( ((int64_t)__paramv->context.old_pc.offset) >= ((int64_t)NULL) ); // RAG -- new check
  assert( ((int64_t)__paramv->context.new_pc.offset) >= ((int64_t)NULL) ); // RAG -- new check

  __iter_wait_guids_t *__iter_wait_depv   =
 (__iter_wait_guids_t *) &__depv->depv[__depc - WORDS_iter_wait_guids_t];
#ifdef DEBUG
//for(int i=0;i<__depc;i++)
//  printf("__iter_wait DEV[%d].GUID = %" PRIx64 "\n", i, __depv->depv[i].guid );fflush(stdout);
//printf("__iter_wait dep[%d].guid = %" PRIx64 "\n", 0, __iter_wait_depv->depv[0].guid );fflush(stdout);
#endif
  assert( IS_GUID_EQUAL(__iter_wait_depv->depv[0].guid, __ocrNULL.guid) );

#ifdef DEBUG
  printf("Enter __iter_wait (paramc = %d, depc = %d) finish_event = %" PRIx64 "\n", __paramc, __depc, __depv->depv[__depc-1].guid);fflush(stdout);
  printf("Enter __iter_wait (paramc = %d, depc = %d) finish_event = %" PRIx64 "\n", __paramc, __depc, __iter_wait_depv->depv[0].guid);fflush(stdout);
#else
//putchar('>');fflush(stdout);
#endif

#ifdef DEBUG
printf("new_pc      %16.16" PRIx64 ":%16.16" PRIx64 "\n",
       __paramv->context.new_pc.guid,
       __paramv->context.new_pc.offset);        fflush(stdout);
printf("old_pc      %16.16" PRIx64 ":%16.16" PRIx64 "\n",
       __paramv->context.old_pc.guid,
       __paramv->context.old_pc.offset);        fflush(stdout);
printf("old_frame   %16.16" PRIx64 ":%16.16" PRIx64 "\n",
       __depv->segment.old_frame.guid,
       __paramv->context.old_frame);            fflush(stdout);
printf("new_frame   %16.16" PRIx64 ":%16.16" PRIx64 "\n",
       __depv->segment.new_frame.guid,
       __paramv->context.new_frame);            fflush(stdout);
printf("paramc      %8" PRIu32 "\n", __paramc); fflush(stdout);
printf("depc        %8" PRIu32 "\n", __depc);   fflush(stdout);
#endif

  assert( ((int64_t)__paramv->context.old_pc.offset) >= ((int64_t)NULL) ); // RAG -- new check
  assert( ((int64_t)__paramv->context.new_pc.offset) >= ((int64_t)NULL) ); // RAG -- new check

  goto *(void *)( &&__iter_wait_offset_zero + __paramv->context.new_pc.offset );
/*---------------------------------------------------------------------------------------*/
__iter_wait_offset_zero:
TRACE1("__iter_wait()@offset_zero\n");

TRACE1("__iter_wait about to pass params on to bottom_of_loop");
  __paramv->context.new_pc = __paramv->context.old_pc;
  __paramv->context.old_pc = __ocrNULL;

TRACE1("__iter_wait about to create edt for bottom_of_loop continuation");
  ocrGuid_t __edt_bottom_of_loop_guid;
  __ret_val = ocrEdtCreate(&__edt_bottom_of_loop_guid, __paramv->context.new_pc.guid,
                         (uint32_t)(__paramc), &__paramv->paramv[0],
                         (uint32_t)(__depc-WORDS_iter_wait_guids_t), NULL,
                         EDT_PROP_NONE, NULL_HINT, NULL );
  CHECK_RET_VAL("__iter_wait()->ocrEdtCreate",__ret_val);

TRACE1("__iter_wait about to pass guids on to bottom_of_loop");
  for(int i=0;i<(__depc-WORDS_iter_wait_guids_t);i++) {
     __ret_val = ocrAddDependence(__depv->depv[i].guid, __edt_bottom_of_loop_guid, i, DB_DEFAULT_MODE);
     CHECK_RET_VAL("__iter_wait()->ocrAddDepence",__ret_val);
  }

TRACE1("__iter_wait()<>offset_zero");
  return NULL_GUID;
} /* end __iter_wait() */

#ifdef __cplusplus
}
#endif

#endif /* _C99ONOCR.h */
