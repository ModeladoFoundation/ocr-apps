/*
 *         ---- The Unbalanced Tree Search (UTS) Benchmark ----
 *
 *  Copyright (c) 2010 See AUTHORS file for copyright holders
 *
 *  This file is part of the unbalanced tree search benchmark.  This
 *  project is licensed under the MIT Open Source license.  See the LICENSE
 *  file for copyright and licensing information.
 *
 *  UTS is a collaborative project between researchers at the University of
 *  Maryland, the University of North Carolina at Chapel Hill, and the Ohio
 *  State University.  See AUTHORS file for more information.
 *
 */

#include <ocr.h>

#if 1
#define ALLOC(size) malloc(size)
#define FREE(ptr)   free(ptr)
#include <malloc.h>
#else
#define ALLOC(size)  alloca(size)
#define FREE(ptr)
#endif
#include "c99onOCR.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

#include "uts.h"

/***********************************************************
 *  tree generation and search parameters                  *
 *                                                         *
 *  Tree generation strategy is controlled via various     *
 *  parameters set from the command line.  The parameters  *
 *  and their default values are given below.              *
 ***********************************************************/

/* RAG -- TODO: these need to be globals too */
char * uts_trees_str[]     = { "Binomial", "Geometric", "Hybrid", "Balanced" };
char * uts_geoshapes_str[] = { "Linear decrease", "Exponential decrease", "Cyclic",
                               "Fixed branching factor" };

/* Tree type
 *   Trees are generated using a Galton-Watson process, in
 *   which the branching factor of each node is a random
 *   variable.
 *
 *   The random variable can follow a binomial distribution
 *   or a geometric distribution.  Hybrid tree are
 *   generated with geometric distributions near the
 *   root and binomial distributions towards the leaves.
 */
tree_t type  = GEO; // Default tree type
const ocrPtr_t __type = { .offset = (size_t)&type, .guid = RAG_NULL_GUID };

double b_0   = 4.0; // default branching factor at the root
const ocrPtr_t __b_0 = { .offset = (size_t)&b_0, .guid = RAG_NULL_GUID };

int   rootId = 0;   // default seed for RNG state at root
const ocrPtr_t __rootId = { .offset = (size_t)&rootId, .guid = RAG_NULL_GUID };

/*  Tree type BIN (BINOMIAL)
 *  The branching factor at the root is specified by b_0.
 *  The branching factor below the root follows an
 *     identical binomial distribution at all nodes.
 *  A node has m children with prob q, or no children with
 *     prob (1-q).  The expected branching factor is q * m.
 *
 *  Default parameter values
 */
int    nonLeafBF   = 4;            // m
const ocrPtr_t __nonLeafBF = { .offset = (size_t)&nonLeafBF, .guid = RAG_NULL_GUID };

double nonLeafProb = 15.0 / 64.0;  // q
const ocrPtr_t __nonLeafProb = { .offset = (size_t)&nonLeafProb, .guid = RAG_NULL_GUID };

/*  Tree type GEO (GEOMETRIC)
 *  The branching factor follows a geometric distribution with
 *     expected value b.
 *  The probability that a node has 0 <= n children is p(1-p)^n for
 *     0 < p <= 1. The distribution is truncated at MAXNUMCHILDREN.
 *  The expected number of children b = (1-p)/p.  Given b (the
 *     target branching factor) we can solve for p.
 *
 *  A shape function computes a target branching factor b_i
 *     for nodes at depth i as a function of the root branching
 *     factor b_0 and a maximum depth gen_mx.
 *
 *  Default parameter values
 */
int        gen_mx   = 6;      // default depth of tree
const ocrPtr_t __gen_mx = { .offset = (size_t)&gen_mx, .guid = RAG_NULL_GUID };

geoshape_t shape_fn = LINEAR; // default shape function (b_i decr linearly)
const ocrPtr_t __shape_fn = { .offset = (size_t)&shape_fn, .guid = RAG_NULL_GUID };

/*  In type HYBRID trees, each node is either type BIN or type
 *  GEO, with the generation strategy changing from GEO to BIN
 *  at a fixed depth, expressed as a fraction of gen_mx
 */
double shiftDepth = 0.5;
const ocrPtr_t __shiftDepth = { .offset = (size_t)&shiftDepth, .guid = RAG_NULL_GUID };

/* compute granularity - number of rng evaluations per tree node */
int computeGranularity = 1;
const ocrPtr_t __computeGranularity = { .offset = (size_t)&computeGranularity, .guid = RAG_NULL_GUID };

/* display parameters */
int verbose  = 1;
const ocrPtr_t __verbose = { .offset = (size_t)&verbose, .guid = RAG_NULL_GUID };

/***********************************************************
 *                                                         *
 *  FUNCTIONS                                              *
 *                                                         *
 ***********************************************************/

static void uts_error(char *str) {
  printf("%s\n",str);
  exit(1);
}

/*
 * wall clock time
 *   for detailed accounting of work, this needs
 *   high resolution
 */
static double uts_wctime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec + 1E-6 * tv.tv_usec);
}


// Interpret 32 bit positive integer as value on [0,1)
static double rng_toProb(int n) {
  if (n < 0) {
    printf("*** toProb: rand n = %d out of range\n",n);
  }
  return ((n<0)? 0.0 : ((double) n)/2147483648.0);
}

static int uts_numChildren_bin(Node * parent) {
  // distribution is identical everywhere below root
  int    v = rng_rand(parent->state.state);
  double d = rng_toProb(v);

  return (d < __rhsGblOcrPtr(double,__nonLeafProb)) ? __rhsGblOcrPtr(int,__nonLeafBF) : 0;
}

static int uts_numChildren_geo(Node * parent) {
  double b_i = __rhsGblOcrPtr(double,__b_0);
  int depth = parent->height;
  int numChildren, h;
  double p, u;

  // use shape function to compute target b_i
  if (depth > 0){
    switch ( __rhsGblOcrPtr(geoshape_t,__shape_fn) ) {

      // expected size polynomial in depth
    case EXPDEC:
      b_i = __rhsGblOcrPtr(double,__b_0)
            * pow((double) depth, -log(__rhsGblOcrPtr(double,__b_0))/log((double) __rhsGblOcrPtr(int,__gen_mx)));
      break;

      // cyclic tree size
    case CYCLIC:
      if (depth > 5 * __rhsGblOcrPtr(int,__gen_mx)){
        b_i = 0.0;
        break;
      }
      b_i = pow(__rhsGblOcrPtr(double,__b_0),
                sin(2.0*3.141592653589793*(double) depth / (double) __rhsGblOcrPtr(int,__gen_mx)));
      break;

      // identical distribution at all nodes up to max depth
    case FIXED:
      b_i = (depth < __rhsGblOcrPtr(int,__gen_mx))? __rhsGblOcrPtr(double,__b_0) : 0;
      break;

      // linear decrease in b_i
    case LINEAR:
    default:
      b_i =  __rhsGblOcrPtr(double,__b_0) * (1.0 - (double)depth / (double) __rhsGblOcrPtr(int,__gen_mx));
      break;
    }
  }

  // given target b_i, find prob p so expected value of
  // geometric distribution is b_i.
  p = 1.0 / (1.0 + b_i);

  // get uniform random number on [0,1)
  h = rng_rand(parent->state.state);
  u = rng_toProb(h);

  // max number of children at this cumulative probability
  // (from inverse geometric cumulative density function)
  numChildren = (int) floor(log(1 - u) / log(1 - p));

  return numChildren;
}

static int uts_numChildren(Node *parent) {
  int numChildren = 0;

  /* Determine the number of children */
  switch ( __rhsGblOcrPtr(tree_t,__type) ) {
    case BIN:
      if (parent->height == 0)
        numChildren = (int) floor(__rhsGblOcrPtr(double,__b_0));
      else
        numChildren = uts_numChildren_bin(parent);
      break;

    case GEO:
      numChildren = uts_numChildren_geo(parent);
      break;

    case HYBRID:
#ifdef EARLY_TERM
if(parent->height >= 4) {
  numChildren=0;
  break;
}
#endif
      if (parent->height < __rhsGblOcrPtr(double,__shiftDepth) * __rhsGblOcrPtr(int,__gen_mx))
        numChildren = uts_numChildren_geo(parent);
      else
        numChildren = uts_numChildren_bin(parent);
      break;
    case BALANCED:
      if (parent->height < __rhsGblOcrPtr(int,__gen_mx))
        numChildren = (int) __rhsGblOcrPtr(double,__b_0);
      break;
    default:
      uts_error("parTreeSearch(): Unknown tree type");
  }

  // limit number of children
  // only a BIN root can have more than MAXNUMCHILDREN
  if (parent->height == 0 && parent->type == BIN) {
    int rootBF = (int) ceil(__rhsGblOcrPtr(double,__b_0));
    if (numChildren > rootBF) {
      printf("*** Number of children of root truncated from %d to %d\n",
             numChildren, rootBF);
      numChildren = rootBF;
    }
  } else if (__rhsGblOcrPtr(tree_t,__type) != BALANCED) {
    if (numChildren > MAXNUMCHILDREN) {
      printf("*** Number of children truncated from %d to %d\n",
             numChildren, MAXNUMCHILDREN);
      numChildren = MAXNUMCHILDREN;
    }
  }

  return numChildren;
}


static int uts_childType(Node *parent) {
  switch ( __rhsGblOcrPtr(tree_t,__type) ) {
    case BIN:
      return BIN;
    case GEO:
      return GEO;
    case HYBRID:
      if (parent->height < __rhsGblOcrPtr(double,__shiftDepth) * __rhsGblOcrPtr(int,__gen_mx))
        return GEO;
      else
        return BIN;
    case BALANCED:
      return BALANCED;
    default:
      uts_error("uts_get_childtype(): Unknown tree type");
      return -1;
  }
}


// construct string with all parameter settings
static int uts_paramsToStr(char *strBuf, int ind) {
  // version + execution model
  ind += sprintf(strBuf+ind, "UTS - Unbalanced Tree Search %s (%s)\n", UTS_VERSION, "Open Community Runtime");

  // tree type
  ind += sprintf(strBuf+ind, "Tree type:  %d (%s)\n", __rhsGblOcrPtr(tree_t,__type), uts_trees_str[__rhsGblOcrPtr(tree_t,__type)]);

  // tree shape parameters
  ind += sprintf(strBuf+ind, "Tree shape parameters:\n");
  ind += sprintf(strBuf+ind, "  root branching factor b_0 = %.1f, root seed = %d\n", __rhsGblOcrPtr(double,__b_0), __rhsGblOcrPtr(int,__rootId));

  if (__rhsGblOcrPtr(tree_t,__type) == GEO || __rhsGblOcrPtr(tree_t,__type) == HYBRID) {
    ind += sprintf(strBuf+ind, "  GEO parameters: gen_mx = %d, shape function = %d (%s)\n",
            __rhsGblOcrPtr(int,__gen_mx), __rhsGblOcrPtr(geoshape_t,__shape_fn), uts_geoshapes_str[__rhsGblOcrPtr(geoshape_t,__shape_fn)]);
  }

  if (__rhsGblOcrPtr(tree_t,__type) == BIN || __rhsGblOcrPtr(tree_t,__type) == HYBRID) {
    double q = __rhsGblOcrPtr(double,__nonLeafProb);
    int    m = __rhsGblOcrPtr(int,__nonLeafBF);
    double es  = (1.0 / (1.0 - q * m));
    ind += sprintf(strBuf+ind, "  BIN parameters:  q = %f, m = %d, E(n) = %f, E(s) = %.2f\n",
            q, m, q * m, es);
  }

  if (__rhsGblOcrPtr(tree_t,__type) == HYBRID) {
    ind += sprintf(strBuf+ind, "  HYBRID:  GEO from root to depth %d, then BIN\n",
            (int) ceil(__rhsGblOcrPtr(double,__shiftDepth) * __rhsGblOcrPtr(int,__gen_mx)));
  }

  if (__rhsGblOcrPtr(tree_t,__type) == BALANCED) {
    ind += sprintf(strBuf+ind, "  BALANCED parameters: gen_mx = %d\n", __rhsGblOcrPtr(int,__gen_mx));
    ind += sprintf(strBuf+ind, "        Expected size: %" PRIu64 " nodes, %" PRIu64 " leaves\n",
        (counter_t) ((pow(__rhsGblOcrPtr(double,__b_0), __rhsGblOcrPtr(int,__gen_mx)+1) - 1.0)/(__rhsGblOcrPtr(double,__b_0) - 1.0)) /* geometric series */,
        (counter_t) pow(__rhsGblOcrPtr(double,__b_0), __rhsGblOcrPtr(int,__gen_mx)));
  }

  // random number generator
  ind += sprintf(strBuf+ind, "Random number generator: ");
  ind  = rng_showtype(strBuf, ind);
  ind += sprintf(strBuf+ind, "\nCompute granularity: %d\n", __rhsGblOcrPtr(int,__computeGranularity));

  return ind;
}

/*
 * void uts_printParams();
 * // show parameter settings
 */

ocrGuid_t uts_printParams( uint32_t __paramc, __uts_printParams_args_t  *__paramv,
                           uint32_t __depc,   __uts_printParams_guids_t *__depv ) {
  int __ret_val;
  assert( __paramc == WORDS_uts_printParams_args_t);
  assert( __depc   == WORDS_uts_printParams_guids_t);
  assert( IS_GUID_EQUAL(__paramv->vars.std.context.new_pc.guid, __template_uts_printParams.guid) );
  printf("Enter uts_printParams (paramc = %d, depc = %d) %s\n",
         __paramc, __depc,
         IS_GUID_NULL(__depv->segment.new_frame.guid) ? "new frame" : "reuse frame" );fflush(stdout);

#define WORDS_uts_printParams_locals_t ((sizeof(__uts_printParams_locals_t)+sizeof(uint64_t)-1)/sizeof(uint64_t))
  typedef struct __uts_printParams_locals_t {
  /* paramv */
    __uts_printParams_args_t paramv;
  /* depv */
    __uts_printParams_guids_t depv;
  /* parameters */
  /* locals */
    char strBuf[5000]; /* = ""; */
    int  ind;          /* = 0; */
  /* more locals */
  } __uts_printParams_locals_t;

  ocrPtr_t __FRAME = __ocrNULL;
  __uts_printParams_locals_t *__frame =
 (__uts_printParams_locals_t *) castablePtrFromSegmentOffset( __depv->segment.new_frame,
                                                              __paramv->vars.std.context.new_frame);
  if ( __frame == NULL ) {
#ifdef DEBUG
printf("uts_printParams() build new frame\n");fflush(stdout);
#endif
    __frame = (__uts_printParams_locals_t *)ocrPtrMalloc( &__FRAME, /* room for paramv, depv, parameters, locals, and more locals */
              WORDS_uts_printParams_locals_t*sizeof(uint64_t)+__paramc*sizeof(uint64_t)+__depc*sizeof(ocrGuid_t) );
    /* paramv */
    __frame->paramv.vars.std.context.new_pc    = __paramv->vars.std.context.new_pc;
    __frame->paramv.vars.std.context.old_pc    = __paramv->vars.std.context.old_pc;
    __frame->paramv.vars.std.context.new_frame = __FRAME.offset;
    __frame->paramv.vars.std.context.old_frame = __paramv->vars.std.context.old_frame;
    __frame->paramv.vars.std.context.paramc    = __paramc;
    __frame->paramv.vars.std.context.depc      = __depc;
    /* depv */
    __frame->depv.selector.new_frame  = __FRAME.guid;
    __frame->depv.selector.old_frame  = __depv->segment.old_frame.guid;
    /* parameters */
    /* locals */
    memset(&__frame->strBuf[0],0,5000); /* = ""  */
    __frame->ind = 0;
    /* more locals */
  } else {
#ifdef DEBUG
printf("uts_printParams() reuse frame\n");fflush(stdout);
#endif
    /* depv */
    __FRAME = ocrPtrFromSelectorOffset( __depv->segment.new_frame.guid, __paramv->vars.std.context.new_frame );
  /* locals */
  }

  assert( ((int64_t)__paramv->vars.std.context.old_pc.offset) >= ((int64_t)NULL) ); // RAG -- new check
  assert( ((int64_t)__paramv->vars.std.context.new_pc.offset) >= ((int64_t)NULL) ); // RAG -- new check

  goto *(void *)( &&__uts_printParams_offset_zero + __paramv->vars.std.context.new_pc.offset );
__uts_printParams_offset_zero:
TRACE0("uts_printParams()@offset_zero");

  if (__rhsGblOcrPtr(int,__verbose) > 0) {
    __frame->ind = uts_paramsToStr(__frame->strBuf, __frame->ind);
#ifdef PARALLEL
    __frame->ind += sprintf(__frame->strBuf+__frame->ind, "Execution strategy:  Parallel Recursive Search\n");
#else /* not PARALLEL */
    __frame->ind += sprintf(__frame->strBuf+__frame->ind, "Execution strategy:  Serial Recursive Search\n");
#endif /* PARALLEL */
    printf("%s\n",__frame->strBuf);
  }

TRACE1("uts_printParams about to create edt for for continuation");
  assert(  IS_GUID_EQUAL(__frame->paramv.vars.std.context.old_pc.guid, __template_uts_main.guid) );

  __continuation_args_t *__old_frame = (__continuation_args_t *)castablePtrFromSegmentOffset(__depv->segment.old_frame,
                                       __paramv->vars.std.context.old_frame);
  assert(__old_frame != NULL);

  __old_frame->context.new_pc = __frame->paramv.vars.std.context.old_pc;
  ocrGuid_t __edt_continuation_guid;
  ocrPtrFree(__FRAME);
  __ret_val = ocrEdtCreate(&__edt_continuation_guid, __old_frame->context.new_pc.guid,
                         __old_frame->context.paramc, (uint64_t *)((void *)__old_frame),
                         __old_frame->context.depc,   (ocrGuid_t *)((void *)__old_frame+ __old_frame->context.paramc*sizeof(uint64_t)),
                         EDT_PROP_NONE, NULL_HINT, NULL );
  CHECK_RET_VAL("uts_printParams->ocrEdtCreate",__ret_val);

TRACE0("uts_printParams()<>offset_zero");
  return NULL_GUID;
}

//----------------------------------------------------------------------------
//  uts_parseParams() function for "ocr"
//----------------------------------------------------------------------------

ocrGuid_t uts_parseParams( uint32_t __paramc, __uts_parseParams_args_t  *__paramv,
                    uint32_t __depc,   __uts_parseParams_guids_t *__depv ) {
  int __ret_val;

#define WORDS_uts_parseParams_locals_t ((sizeof(__uts_parseParams_locals_t)+sizeof(uint64_t)-1)/sizeof(uint64_t))
  typedef struct __uts_parseParams_locals_t {
  /* paramv */
    __uts_parseParams_args_t  paramv;
  /* depv */
    __uts_parseParams_guids_t depv;
  /* parameters */
    int argc;
    char **argv;
  /* locals */
    int i, err;
  /* more locals */
    ocrPtr_t *__argv;
    char *__argv_i;
    char *__argv_ip1;
  } __uts_parseParams_locals_t;

  assert( IS_GUID_EQUAL(__paramv->vars.std.context.new_pc.guid, __template_uts_parseParams.guid) );
  printf("Enter uts_parseParams (paramc = %d, depc = %d) %s\n",
         __paramc, __depc,
         IS_GUID_NULL(__depv->segment.new_frame.guid) ? "new frame" : "reuse frame" );fflush(stdout);

  ocrPtr_t  __FRAME = __ocrNULL;
  __uts_parseParams_locals_t *__frame = (__uts_parseParams_locals_t *) castablePtrFromSegmentOffset( __depv->segment.new_frame,
                                  __paramv->vars.std.context.new_frame);
  if( __frame == NULL ) {
#ifdef DEBUG
printf("uts_parseParams() build new frame\n");fflush(stdout);
#endif
    __frame = (__uts_parseParams_locals_t *)ocrPtrMalloc( &__FRAME, /* room for paramv, depv, parameters, locals, and more locals */
              WORDS_uts_parseParams_locals_t*sizeof(uint64_t)+__paramc*sizeof(uint64_t)+__depc*sizeof(ocrGuid_t) );
  /* paramv */
    assert( __paramc == WORDS_uts_parseParams_args_t );
    __frame->paramv.vars.std.context.new_pc    = __paramv->vars.std.context.new_pc;
    __frame->paramv.vars.std.context.old_pc    = __paramv->vars.std.context.old_pc;
    __frame->paramv.vars.std.context.new_frame = __FRAME.offset;
    __frame->paramv.vars.std.context.old_frame = __paramv->vars.std.context.old_frame;
    __frame->paramv.vars.std.context.paramc    = __paramc;
    __frame->paramv.vars.std.context.depc      = __depc;
    __frame->paramv.vars.argv                     = __paramv->vars.argv;
  /* more paramv */
  // offset for __arg_i
  // offset for __arg_ip1
  /* depv */
    assert( __depc   == WORDS_uts_parseParams_guids_t );
    __frame->depv.selector.new_frame = __FRAME.guid;
    __frame->depv.selector.old_frame = __depv->segment.old_frame.guid;
    __frame->depv.selector.argv      = __depv->segment.argv.guid;
  /* more depv */
  // guid for __arg_i
  // guid for __arg_ip1
  /* parameters */
    __frame->argc  = (const int)__paramv->vars.argc;
    __frame->argv  = (char **)NULL;
  /* locals */
    __frame->i   = 0;
    __frame->err = 0;
  /* more locals */
    __frame->__argv     = (ocrPtr_t *) castablePtrFromSegmentOffset( __depv->segment.argv, __paramv->vars.argv);
    __frame->__argv_i   = NULL;
    __frame->__argv_ip1 = NULL;
  }  else {
#ifdef DEBUG
printf("uts_parseParams() reuse frame\n");fflush(stdout);
#endif
    __FRAME = ocrPtrFromSelectorOffset( __depv->segment.new_frame.guid , __paramv->vars.std.context.new_frame );
  /* paramv */
  /* more paramv */
  /* depv */
  /* more depv */
  /* parameters */
  /* locals */
  /* more locals */
  }

#ifdef DEBUG
printf("new_pc      %16.16" PRIx64 ":%16.16" PRIx64 "\n",
       __frame->paramv.vars.std.context.new_pc.guid,
       __frame->paramv.vars.std.context.new_pc.offset); fflush(stdout);
printf("old_pc      %16.16" PRIx64 ":%16.16" PRIx64 "\n",
       __frame->paramv.vars.std.context.old_pc.guid,
       __frame->paramv.vars.std.context.old_pc.offset); fflush(stdout);
printf("old_frame   %16.16" PRIx64 ":%16.16" PRIx64 "\n",
       __frame->depv.selector.old_frame,
       __frame->paramv.vars.std.context.old_frame);     fflush(stdout);
printf("new_frame   %16.16" PRIx64 ":%16.16" PRIx64 "\n",
       __frame->depv.selector.new_frame,
       __frame->paramv.vars.std.context.new_frame);     fflush(stdout);
printf("paramc      %8" PRIu32 "\n", __paramc);         fflush(stdout);
printf("depc        %8" PRIu32 "\n", __depc);           fflush(stdout);
#endif

  assert( ((int64_t)__paramv->vars.std.context.old_pc.offset) >= ((int64_t)NULL) ); // RAG -- new check
  assert( ((int64_t)__paramv->vars.std.context.new_pc.offset) >= ((int64_t)NULL) ); // RAG -- new check

  goto *(void *)( &&__uts_parseParams_offset_zero + __paramv->vars.std.context.new_pc.offset );
/*---------------------------------------------------------------------------------------*/
__uts_parseParams_offset_zero:
TRACE0("uts_parseParams()@offset_zero\n");


TRACE0("/* determine benchmark parameters (all PEs) */");
  __frame->i = 1;
  __frame->err = -1;
  while (__frame->i < ((const int)__frame->argc) && __frame->err == -1) {
 {
TRACE1("uts_parseParams about to create __paramv_0 for uts_parseParams for argv_i");
  __uts_parseParams_args_t *__paramv_0 = (__uts_parseParams_args_t *) ALLOC( (WORDS_uts_parseParams_args_t+1)*sizeof(uint64_t) );
  CHECK_RET_VAL("ALLOC __paramv_0[]",(__paramv_0 == NULL));
  /* paramv */
  __paramv_0->vars.std.context.new_pc   = ocrPtrFromSelectorOffset( __frame->paramv.vars.std.context.new_pc.guid,
                                       (size_t)( &&__uts_parseParams_argv_i - &&__uts_parseParams_offset_zero ));
  /* Calling ourself, so copy origonal caller's return pc and old_frame */
  __paramv_0->vars.std.context.old_pc    = __frame->paramv.vars.std.context.old_pc;
#if 1
// RAG -- why new __FRAME.offset here, but new_frame.guid below?
assert(__frame->paramv.vars.std.context.new_frame == __FRAME.offset);
#endif
  __paramv_0->vars.std.context.new_frame = __frame->paramv.vars.std.context.new_frame;
  __paramv_0->vars.std.context.old_frame = __frame->paramv.vars.std.context.old_frame;
  __paramv_0->vars.std.context.paramc    = -1;
  __paramv_0->vars.std.context.depc      = -1;
  /* parameters */
  __paramv_0->vars.argc = (const int)__frame->argc;
  __paramv_0->vars.argv = __frame->paramv.vars.argv;
  /* more paramv */
  __paramv_0->paramv[WORDS_uts_parseParams_args_t+0] = __frame->__argv[__frame->i].offset;

TRACE1("uts_parseParams about to __depv_0 for uts_parseParams for argv_i");
  __uts_parseParams_guids_t *__depv_0 = (__uts_parseParams_guids_t *) ALLOC( (WORDS_uts_parseParams_guids_t+1)*sizeof(ocrGuid_t) );
  CHECK_RET_VAL("ALLOC __depv_0[]",(__depv_0 == NULL));
  /* depv */
#if 1
// RAG -- why new __FRAME.offset above, but new_frame.guid here?
assert( IS_GUID_EQUAL(__frame->depv.selector.new_frame, __FRAME.guid) );
#endif
  __depv_0->selector.new_frame = __frame->depv.selector.new_frame;
  __depv_0->selector.old_frame = __frame->depv.selector.old_frame;
  __depv_0->selector.argv      = __frame->depv.selector.argv;
  /* more depv */
  __depv_0->guids[WORDS_uts_parseParams_guids_t+0] = __frame->__argv[__frame->i].guid;

TRACE1("uts_parseParams about to create edt for uts_parseParams for argv_i");
  ocrGuid_t __edt_uts_parseParams_guid;
  __ret_val = ocrEdtCreate(&__edt_uts_parseParams_guid, __template_uts_parseParams.guid,
            WORDS_uts_parseParams_args_t  + 1, &__paramv_0->paramv[0],
            WORDS_uts_parseParams_guids_t + 1, &__depv_0->guids[0],
            EDT_PROP_NONE, NULL_HINT, NULL );
  CHECK_RET_VAL("uts_parseParams()->ocrEdtCreate",__ret_val);

TRACE0("uts_parseParams()<>offset_zero");
  FREE(__depv_0);
  FREE(__paramv_0);
  return NULL_GUID;
 }
/*---------------------------------------------------------------------------------------*/
__uts_parseParams_argv_i:
    __frame->__argv     = (ocrPtr_t *) castablePtrFromSegmentOffset( __depv->segment.argv,
                          __paramv->vars.argv);
    __frame->__argv_i   = (char *) castablePtrFromSegmentOffset( __depv->depv[WORDS_uts_parseParams_guids_t+0],
                          __paramv->paramv[WORDS_uts_parseParams_args_t+0]);
    __frame->__argv_ip1 = NULL;
TRACE0("uts_parseParams()@argv_i\n");
    if ( __frame->__argv_i[0] == '-' && __frame->__argv_i[1] == 'h') {
        uts_helpMessage();
        exit(0);

    } else if ( __frame->__argv_i[0] != '-' || strlen( __frame->__argv_i) != 2 || __frame->argc <= __frame->i+1) {
      __frame->err = __frame->i; break;
    }

#ifdef DEBUG
printf("The %d switch is %sn",__frame->i,__frame->__argv_i);fflush(stdout);
#endif

 {
TRACE1("uts_parseParams about to create __paramv_0 for uts_parseParams for argv_i and argv_ip1");
  __uts_parseParams_args_t *__paramv_0 = (__uts_parseParams_args_t *) ALLOC( (WORDS_uts_parseParams_args_t+2)*sizeof(uint64_t) );
  CHECK_RET_VAL("ALLOC __paramv_0[]",(__paramv_0 == NULL));
  /* paramv */
  __paramv_0->vars.std.context.new_pc   = ocrPtrFromSelectorOffset( __frame->paramv.vars.std.context.new_pc.guid,
                                       (size_t)( &&__uts_parseParams_argv_ip1 - &&__uts_parseParams_offset_zero ));
  /* Calling ourself, so copy origonal caller's return pc and old_frame */
  __paramv_0->vars.std.context.old_pc    = __frame->paramv.vars.std.context.old_pc;
#if 1
// RAG -- why new __FRAME.offset here, but new_frame.guid below?
assert(__frame->paramv.vars.std.context.new_frame == __FRAME.offset);
#endif
  __paramv_0->vars.std.context.new_frame = __frame->paramv.vars.std.context.new_frame;
  __paramv_0->vars.std.context.old_frame = __frame->paramv.vars.std.context.old_frame;
  __paramv_0->vars.std.context.paramc    = -1;
  __paramv_0->vars.std.context.depc      = -1;
  /* parameters */
  __paramv_0->vars.argc = (const int)__frame->argc;
  __paramv_0->vars.argv = __frame->paramv.vars.argv;
  /* more paramv */
  __paramv_0->paramv[WORDS_uts_parseParams_args_t+0] = __frame->__argv[__frame->i].offset;
  __paramv_0->paramv[WORDS_uts_parseParams_args_t+1] = __frame->__argv[__frame->i+1].offset;

TRACE1("uts_parseParams about to __depv_0 for uts_parseParams for argv_i and argv_ip1");
  __uts_parseParams_guids_t *__depv_0 = (__uts_parseParams_guids_t *) ALLOC( (WORDS_uts_parseParams_guids_t+2)*sizeof(ocrGuid_t) );
  CHECK_RET_VAL("ALLOC __depv_0[]",(__depv_0 == NULL));
  /* depv */
#if 1
// RAG -- why new __FRAME.offset above, but new_frame.guid here?
assert( IS_GUID_EQUAL(__frame->depv.selector.new_frame, __FRAME.guid) );
#endif
  __depv_0->selector.new_frame = __frame->depv.selector.new_frame;
  __depv_0->selector.old_frame = __frame->depv.selector.old_frame;
  __depv_0->selector.argv      = __frame->depv.selector.argv;
  /* more depv */
  __depv_0->guids[WORDS_uts_parseParams_guids_t+0] = __frame->__argv[__frame->i].guid;
  __depv_0->guids[WORDS_uts_parseParams_guids_t+1] = __frame->__argv[__frame->i+1].guid;

TRACE1("uts_parseParams about to create edt for uts_parseParams for argv_i and argv_ip1");
  ocrGuid_t __edt_uts_parseParams_guid;
  __ret_val = ocrEdtCreate(&__edt_uts_parseParams_guid, __template_uts_parseParams.guid,
            WORDS_uts_parseParams_args_t  + 2, &__paramv_0->paramv[0],
            WORDS_uts_parseParams_guids_t + 2, &__depv_0->guids[0],
            EDT_PROP_NONE, NULL_HINT, NULL );
  CHECK_RET_VAL("uts_parseParams()->ocrEdtCreate",__ret_val);

TRACE0("uts_parseParams()<>argv_i");
  FREE(__depv_0);
  FREE(__paramv_0);
  return NULL_GUID;
 }
/*---------------------------------------------------------------------------------------*/
__uts_parseParams_argv_ip1:
    __frame->__argv     = (ocrPtr_t *) castablePtrFromSegmentOffset( __depv->segment.argv,
                          __paramv->vars.argv);
    __frame->__argv_i   = (char *) castablePtrFromSegmentOffset( __depv->depv[WORDS_uts_parseParams_guids_t+0],
                          __paramv->paramv[WORDS_uts_parseParams_args_t+0]);
    __frame->__argv_ip1 = (char *) castablePtrFromSegmentOffset( __depv->depv[WORDS_uts_parseParams_guids_t+1],
                          __paramv->paramv[WORDS_uts_parseParams_args_t+1]);
TRACE0("uts_parseParams()@argv_ip1\n");

#ifdef DEBUG
printf("the %d switch is %s with option %s\n",__frame->i,__frame->__argv_i,__frame->__argv_ip1);fflush(stdout);
#endif

    switch ( __frame->__argv_i[1] ) {
      case 'q':
        __lhsGblOcrPtr(double,__nonLeafProb) = atof(__frame->__argv_ip1); break;
      case 'm':
        __lhsGblOcrPtr(int,__nonLeafBF)      = atoi(__frame->__argv_ip1); break;
      case 'r':
        __lhsGblOcrPtr(int,__rootId)         = atoi(__frame->__argv_ip1); break;
      case 'v':
        __lhsGblOcrPtr(int,__verbose)        = atoi(__frame->__argv_ip1); break;
      case 't':
        __lhsGblOcrPtr(tree_t,__type)        = (tree_t) atoi(__frame->__argv_ip1);
        if ( __rhsGblOcrPtr(tree_t,__type) != BIN
          && __rhsGblOcrPtr(tree_t,__type) != GEO
          && __rhsGblOcrPtr(tree_t,__type) != HYBRID
          && __rhsGblOcrPtr(tree_t,__type) != BALANCED)
	  __frame->err = __frame->i;
        break;
      case 'a':
        __lhsGblOcrPtr(geoshape_t,__shape_fn) = (geoshape_t) atoi(__frame->__argv_ip1);
        if (__rhsGblOcrPtr(geoshape_t,__shape_fn) > FIXED) __frame->err = __frame->i;
        break;
      case 'b':
        __lhsGblOcrPtr(double,__b_0) = atof(__frame->__argv_ip1); break;
      case 'd':
        __lhsGblOcrPtr(int,__gen_mx) = atoi(__frame->__argv_ip1); break;
      case 'f':
        __lhsGblOcrPtr(double,__shiftDepth) = atof(__frame->__argv_ip1); break;
      case 'g':
        __lhsGblOcrPtr(int,__computeGranularity) = max(1,atoi(__frame->__argv_ip1)); break;
      default:
        __frame->err = __frame->i;
    } /* switch */

    if (__frame->err != -1) break;

    __frame->i += 2;
  } /* while */

  if (__frame->err != -1) {
    printf("Unrecognized parameter or incorrect/missing value: '%s %s'\n", __frame->__argv_i, (__frame->i+1 < __frame->argc) ? __frame->__argv_ip1 : "[none]");
    printf("Try -h for help.\n");
    exit(4);
  }

TRACE1("uts_parseParams about to create edt for continuation");
  assert(  IS_GUID_EQUAL(__frame->paramv.vars.std.context.old_pc.guid, __template_uts_main.guid) );

  __continuation_args_t *__old_frame = (__continuation_args_t *)castablePtrFromSegmentOffset(__depv->segment.old_frame,
                                       __paramv->vars.std.context.old_frame);
  assert(__old_frame != NULL);

  __old_frame->context.new_pc = __frame->paramv.vars.std.context.old_pc;
  ocrGuid_t __edt_continuation_guid;
  ocrPtrFree(__FRAME);
  __ret_val = ocrEdtCreate(&__edt_continuation_guid, __old_frame->context.new_pc.guid,
                         __old_frame->context.paramc, (uint64_t *)((void *)__old_frame),
                         __old_frame->context.depc,   (ocrGuid_t *)((void *)__old_frame+ __old_frame->context.paramc*sizeof(uint64_t)),
                         EDT_PROP_NONE, NULL_HINT, NULL );
  CHECK_RET_VAL("uts_parseParams->ocrEdtCreate",__ret_val);

TRACE0("uts_parseParams()<>argv_ip1\n");
  return NULL_GUID;
} /* end uts_parseParams() */

static void uts_helpMessage() {
  printf("  UTS - Unbalanced Tree Search %s (%s)\n\n", UTS_VERSION, "OCR");
  printf("    usage:  uts-bin [parameter value] ...\n\n");
  printf("  parm type  description\n");
  printf("  ==== ====  =========================================\n");
  printf("\n  Benchmark Parameters:\n");
  printf("   -t  int   tree type (0: BIN, 1: GEO, 2: HYBRID, 3: BALANCED)\n");
  printf("   -b  dble  root branching factor\n");
  printf("   -r  int   root seed 0 <= r < 2^31 \n");
  printf("   -a  int   GEO: tree shape function \n");
  printf("   -d  int   GEO, BALANCED: tree depth\n");
  printf("   -q  dble  BIN: probability of non-leaf node\n");
  printf("   -m  int   BIN: number of children for non-leaf node\n");
  printf("   -f  dble  HYBRID: fraction of depth for GEO -> BIN transition\n");
  printf("   -g  int   compute granularity: number of rng_spawns per node\n");
  printf("   -v  int   nonzero to set verbose output\n");
  printf("\n");
}

/*----------------------------------------------------------------------------*/

ocrGuid_t uts_TreeSearch( uint32_t __paramc, __uts_TreeSearch_args_t  *__paramv,
                          uint32_t __depc,   __uts_TreeSearch_guids_t *__depv ) {
  int __ret_val;
  assert( __paramc >= WORDS_uts_TreeSearch_args_t);
  assert( __depc   >= WORDS_uts_TreeSearch_guids_t);
  assert( IS_GUID_EQUAL(__paramv->vars.std.context.new_pc.guid, __template_uts_TreeSearch.guid) );
/* only do "Enter" prinf for first (non-recursive) call */
  if ( ( !IS_GUID_EQUAL(__paramv->vars.std.context.old_pc.guid, __template_uts_TreeSearch.guid) )
    && ( __paramv->vars.std.context.new_pc.offset == (size_t)NULL ) )
  printf("Enter uts_TreeSearch (paramc = %d, depc = %d) %s\n",
         __paramc, __depc,
         IS_GUID_NULL(__depv->segment.new_frame.guid) ? "new frame" : "reuse frame" );fflush(stdout);

#define WORDS_uts_TreeSearch_locals_t ((sizeof(__uts_TreeSearch_locals_t)+sizeof(uint64_t)-1)/sizeof(uint64_t))
  typedef struct __uts_TreeSearch_locals_t {
  /* paramv */
    __uts_TreeSearch_args_t paramv;
  /* more paramv */
  /* depv */
    __uts_TreeSearch_guids_t depv;
  /* more depv */
  /* parameters */
    Node *parent;
  /* locals */
    int childHeight;
    int numChildren;
    union { struct { int childType;                   /* = uts_childType(parent); */
                     int i, iFirst, iLast, iInc;      /* for(int i=iFirst; i<iLast; i+=iInc){};  */
                     int j, jFirst, jLast, jInc;      /* for(int j=jFirst; j<jLast; j+=jInc){};  */
                     Node child;                      /* for() {Node child; ...}; */
                   } blk1;
          } indent1;
    int oldVal;
  /* more locals */
    ocrPtr_t __CHILD;
  } __uts_TreeSearch_locals_t;

  ocrPtr_t __FRAME = __ocrNULL;
  __uts_TreeSearch_locals_t *__frame =
 (__uts_TreeSearch_locals_t *) castablePtrFromSegmentOffset( __depv->segment.new_frame,
                                                             __paramv->vars.std.context.new_frame);
  if ( __frame == NULL ) {
#ifdef DEBUG
printf("uts_TreeSearch() build new frame\n");fflush(stdout);
#endif
    __frame = (__uts_TreeSearch_locals_t *)ocrPtrMalloc( &__FRAME, /* room for paramv, depv, parameters, locals, and more locals */
              WORDS_uts_TreeSearch_locals_t*sizeof(uint64_t)+__paramc*sizeof(uint64_t)+__depc*sizeof(ocrGuid_t) );
    /* paramv */
    __frame->paramv.vars.std.context.new_pc    = __paramv->vars.std.context.new_pc;
    __frame->paramv.vars.std.context.old_pc    = __paramv->vars.std.context.old_pc;
    __frame->paramv.vars.std.context.new_frame = __FRAME.offset;
    __frame->paramv.vars.std.context.old_frame = __paramv->vars.std.context.old_frame;
    __frame->paramv.vars.std.context.paramc    = __paramc;
    __frame->paramv.vars.std.context.depc      = __depc;
    __frame->paramv.vars.parent                = __paramv->vars.parent;
    /* depv */
    __frame->depv.selector.new_frame = __FRAME.guid;
    __frame->depv.selector.old_frame = __depv->segment.old_frame.guid;
    __frame->depv.selector.parent    = __depv->segment.parent.guid;
    /* parameters */
    __frame->parent = (Node *) castablePtrFromSegmentOffset( __depv->segment.parent, __paramv->vars.parent);
    /* locals */
    /* more locals */
  } else {
#ifdef DEBUG
printf("uts_TreeSearch() reuse frame\n");fflush(stdout);
#endif
    __FRAME = ocrPtrFromSelectorOffset( __depv->segment.new_frame.guid, __paramv->vars.std.context.new_frame );
    /* paramv */
    /* depv */
    /* parameters */
    __frame->parent = (Node *) castablePtrFromSegmentOffset( __depv->segment.parent, __paramv->vars.parent);
  /* locals */
  /* more locals */
  }
#ifdef DEBUG
printf("new_pc      %16.16" PRIx64 ":%16.16" PRIx64 "\n",
       __frame->paramv.vars.std.context.new_pc.guid,
       __frame->paramv.vars.std.context.new_pc.offset); fflush(stdout);
printf("old_pc      %16.16" PRIx64 ":%16.16" PRIx64 "\n",
       __frame->paramv.vars.std.context.old_pc.guid,
       __frame->paramv.vars.std.context.old_pc.offset); fflush(stdout);
printf("old_frame   %16.16" PRIx64 ":%16.16" PRIx64 "\n",
       __frame->depv.selector.old_frame,
       __frame->paramv.vars.std.context.old_frame);     fflush(stdout);
printf("new_frame   %16.16" PRIx64 ":%16.16" PRIx64 "\n",
       __frame->depv.selector.new_frame,
       __frame->paramv.vars.std.context.new_frame);     fflush(stdout);
printf("paramc      %8" PRIu32 "\n", __paramc);         fflush(stdout);
printf("depc        %8" PRIu32 "\n", __depc);           fflush(stdout);
#endif
#ifdef DEBUG
printf("&parent = %p\n",__frame->parent); fflush(stdout);
printf("parent->type = %d\n",__frame->parent->type); fflush(stdout);
printf("parent->height = %d\n",__frame->parent->height); fflush(stdout);
printf("parent->numChildren = %d\n",__frame->parent->numChildren); fflush(stdout);
#endif

assert(
  ((__paramv->vars.std.context.new_pc.offset) == ((size_t)(&&__uts_TreeSearch_offset_zero         - &&__uts_TreeSearch_offset_zero)) )
||((__paramv->vars.std.context.new_pc.offset) == ((size_t)(&&__uts_TreeSearch_TreeSearch_returned - &&__uts_TreeSearch_offset_zero)) )
#ifdef PARALLEL
||((__paramv->vars.std.context.new_pc.offset) == ((size_t)(&&__uts_TreeSearch_top_of_loop_i - &&__uts_TreeSearch_offset_zero))       )
||((__paramv->vars.std.context.new_pc.offset) == ((size_t)(&&__uts_TreeSearch_bottom_of_loop_i - &&__uts_TreeSearch_offset_zero))    )
||((__paramv->vars.std.context.new_pc.offset) == ((size_t)(&&__uts_TreeSearch_top_of_loop_j - &&__uts_TreeSearch_offset_zero))       )
||((__paramv->vars.std.context.new_pc.offset) == ((size_t)(&&__uts_TreeSearch_bottom_of_loop_j - &&__uts_TreeSearch_offset_zero))    )
#endif
); // RAG -- new check

  assert( ((int64_t)__paramv->vars.std.context.old_pc.offset) >= ((int64_t)NULL) ); // RAG -- new check
  assert( ((int64_t)__paramv->vars.std.context.new_pc.offset) >= ((int64_t)NULL) ); // RAG -- new check

  goto *(void *)( &&__uts_TreeSearch_offset_zero + __paramv->vars.std.context.new_pc.offset );
__uts_TreeSearch_offset_zero:
TRACE0("uts_TreeSearch()@offset_zero");

  __frame->childHeight = __frame->parent->height + 1;      /* RAG -- lost const for local childHeight */
  __frame->numChildren = uts_numChildren(__frame->parent); /* RAG -- lost const for local numChildren */

TRACE1("// record number of children in parent");

  __frame->parent->numChildren = ((const int)__frame->numChildren);

#ifdef STATS
#ifdef PARALLEL
  __sync_fetch_and_add( &__frame->parent->numNodes, ((const int)__frame->numChildren) );
#else /* not PARALLEL */
  __frame->parent->numNodes += ((const int) __frame->numChildren);
#endif /* PARALLEL */
#endif /* STATS */

TRACE1("// recursivily construct and visit the children");
  if ( ((const int)__frame->numChildren) > 0 ) {
    __frame->indent1.blk1.childType = uts_childType(__frame->parent);  /* RAG -- lost const for local childType */

    __frame->__CHILD = ocrPtrFromSelectorOffset( __FRAME.guid,  __FRAME.offset + offsetof(__uts_TreeSearch_locals_t, indent1.blk1.child) );

    __frame->indent1.blk1.iFirst = 0;
    __frame->indent1.blk1.iLast  = ((const int)__frame->numChildren);
    __frame->indent1.blk1.iInc   = 1;
 { /* OPEN body for_i */
        __frame->indent1.blk1.child.type   = ((const int)__frame->indent1.blk1.childType);
        __frame->indent1.blk1.child.height = ((const int)__frame->childHeight);
        __frame->indent1.blk1.child.numChildren = -1;    // not yet determined
#ifdef STATS
        __frame->indent1.blk1.child.maxDepth  = ((const int)__frame->childHeight);
        __frame->indent1.blk1.child.numNodes  = 0;
        __frame->indent1.blk1.child.numLeaves = 0;
#endif // STATS

#ifdef PARALLEL
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 { /* OPEN prep for_i */
TRACE1("uts_TreeSearch about to pass params on to __iter_work for_i");
        __continuation_args_t *__paramv_i = (__continuation_args_t *) ALLOC( ( __paramc + WORDS_iter_work_args_t )*sizeof(uint64_t) );
        CHECK_RET_VAL("paramv_i ALLOC",(__paramv_i==NULL));
        __iter_work_args_t *__iter_i = (__iter_work_args_t *) &__paramv_i->paramv[__paramc];
        assert(&__iter_i->paramv[0] == &__paramv_i->paramv[__paramc]);
        /* paramv */
        for(int i=0;i<__frame->paramv.vars.std.context.paramc;i++) {
          __paramv_i->paramv[i] = __frame->paramv.paramv[i];
        }
/* RAG -- need to account for __paramc possibily being bigger than __frame->vars.std.context.paramc */
        for(int i=__frame->paramv.vars.std.context.paramc;i<__paramc;i++) {
          __paramv_i->paramv[i] = __paramv->paramv[i];
        }
        __paramv_i->context.new_pc    = ocrPtrFromSelectorOffset( __template_iter_work.guid, (size_t)NULL ),
        __paramv_i->context.old_pc    = ocrPtrFromSelectorOffset( __template_uts_TreeSearch.guid,
                                        (size_t)(&&__uts_TreeSearch_top_of_loop_i - &&__uts_TreeSearch_offset_zero) );

        /* parameters */
        __iter_i->vars.first = 0;
        __iter_i->vars.last  = __frame->numChildren;
        __iter_i->vars.inc   = 1;
        __iter_i->vars.blk   = -1;

TRACE1("uts_TreeSearch about to create finish edt for __iter_work for_i");

        ocrGuid_t __edt_iter_work, __finish_event_iter_work;
        __ret_val = ocrEdtCreate(&__edt_iter_work, __template_iter_work.guid,
                               (uint32_t)(__paramc+WORDS_iter_work_args_t), &__paramv_i->paramv[0],
                               __depc, NULL,
                               EDT_PROP_FINISH, NULL_HINT, &__finish_event_iter_work);
        CHECK_RET_VAL("uts_TreeSearch()->ocrEdtCreate",__ret_val);
#ifdef DEBUG
printf("__finish_event_iter_work = %" PRIx64 "\n",__finish_event_iter_work);fflush(stdout);
#endif

TRACE1("uts_TreeSearch about to pass params on to __iter_wait for for_i");
        __paramv_i->context.new_pc = ocrPtrFromSelectorOffset( __template_iter_wait.guid, (size_t)NULL ),
        __paramv_i->context.old_pc = ocrPtrFromSelectorOffset( __template_uts_TreeSearch.guid,
                                     (size_t)(&&__uts_TreeSearch_bottom_of_loop_i - &&__uts_TreeSearch_offset_zero) );
TRACE1("uts_TreeSearch about to create edt for __iter_wait i");

        ocrGuid_t __edt_iter_wait;
        __ret_val = ocrEdtCreate(&__edt_iter_wait, __template_iter_wait.guid,
                               __paramc, &__paramv_i->paramv[0],
                               __depc+1, NULL,
                               EDT_PROP_NONE, NULL_HINT, NULL );
        CHECK_RET_VAL("uts_TreeSearch()->ocrEdtCreate",__ret_val);

TRACE1("uts_TreeSearch about to pass guids on to __iter_(wait|work) i");
/* RAG -- need to have __iter_wait ready before starting __iter_work, */
/* RAG -- so __iter_wait is ready to can catch __iter_work's finish event */
        __ret_val = ocrAddDependence( __finish_event_iter_work, __edt_iter_wait, __depc, DB_DEFAULT_MODE);
        CHECK_RET_VAL("uts_TreeSearch->__iter_wait",__ret_val);
	/* depv */
        for(int i=0;i<__frame->paramv.vars.std.context.depc;i++) {
            __ret_val = ocrAddDependence( __frame->depv.guids[i], __edt_iter_wait, i, DB_DEFAULT_MODE);
            CHECK_RET_VAL("uts_TreeSearch->__iter_wait",__ret_val);
            __ret_val = ocrAddDependence( __frame->depv.guids[i], __edt_iter_work, i, DB_DEFAULT_MODE);
            CHECK_RET_VAL("uts_TreeSearch->__iter_work",__ret_val);
        }
/* RAG -- need to account for __depc possibily being bigger than __frame->vars.std.context.depc */
        for(int i=__frame->paramv.vars.std.context.depc;i<__depc;i++) {
            __ret_val = ocrAddDependence( __depv->depv[i].guid, __edt_iter_wait, i, DB_DEFAULT_MODE);
            CHECK_RET_VAL("uts_TreeSearch->__iter_wait",__ret_val);
            __ret_val = ocrAddDependence( __depv->depv[i].guid, __edt_iter_work, i, DB_DEFAULT_MODE);
            CHECK_RET_VAL("uts_TreeSearch->__iter_work",__ret_val);
        }
#ifdef DEBUG
TRACE0("uts_TreeSearch()<->iter_(work|wait) i");
#endif
        FREE(__paramv_i);
        return NULL_GUID;
 } /* CLOSE prep for_i */
__uts_TreeSearch_top_of_loop_i:
 { /* OPEN prep for_j */
TRACE0("uts_TreeSearch()@top_of_loop_i");
        __iter_work_args_t *__iter_i = (__iter_work_args_t *) &__paramv->paramv[__paramc-WORDS_iter_work_args_t];
        /* paramv */
assert(__iter_i->vars.first==0);
assert(__iter_i->vars.last!=0);
assert(__iter_i->vars.inc==1);
        __frame->indent1.blk1.iFirst = __iter_i->vars.first;
        __frame->indent1.blk1.iLast  = __iter_i->vars.last;
        __frame->indent1.blk1.iInc   = __iter_i->vars.inc;
#ifdef DEBUG
printf("FOR_I(%d;%d;%d)\n",__frame->indent1.blk1.iFirst,__frame->indent1.blk1.iLast,__frame->indent1.blk1.iInc);fflush(stdout);
#endif
#ifdef RAG_FOR_I
__frame->indent1.blk1.i = __frame->indent1.blk1.iFirst;
__uts_TreeSearch_top_of_LOOP_I:
if( __frame->indent1.blk1.i >= __frame->indent1.blk1.iLast)
goto __uts_TreeSearch_bottom_of_LOOP_I;
        __frame->indent1.blk1.child.type   = ((const int)__frame->indent1.blk1.childType);
        __frame->indent1.blk1.child.height = ((const int)__frame->childHeight);
        __frame->indent1.blk1.child.numChildren = -1;    // not yet determined
#ifdef STATS
        __frame->indent1.blk1.child.maxDepth  = ((const int)__frame->childHeight);
        __frame->indent1.blk1.child.numNodes  = 0;
        __frame->indent1.blk1.child.numLeaves = 0;
#endif // STATS
#else // RAG_FOR_I
        for (__frame->indent1.blk1.i = ((const int)__frame->indent1.blk1.iFirst);
             __frame->indent1.blk1.i < ((const int)__frame->indent1.blk1.iLast);
             __frame->indent1.blk1.i +=((const int)__frame->indent1.blk1.iInc)) {
        __frame->indent1.blk1.child.type   = ((const int)__frame->indent1.blk1.childType);
        __frame->indent1.blk1.child.height = ((const int)__frame->childHeight);
        __frame->indent1.blk1.child.numChildren = -1;    // not yet determined
#ifdef STATS
        __frame->indent1.blk1.child.maxDepth  = ((const int)__frame->childHeight);
        __frame->indent1.blk1.child.numNodes  = 0;
        __frame->indent1.blk1.child.numLeaves = 0;
#endif // STATS
#endif // RAG_FOR_I
#ifdef DEBUG
printf("BEGIN BODY FOR_I %d\n",__frame->indent1.blk1.i);fflush(stdout);
#endif
TRACE1("// computeGranularity controls number of rng_spawn calls per node");
TRACE1("uts_TreeSearch about to pass params on to __iter_work for_j");
        __continuation_args_t *__paramv_j = (__continuation_args_t *) ALLOC( ( __paramc + WORDS_iter_work_args_t )*sizeof(uint64_t) );
        CHECK_RET_VAL("paramv_j ALLOC",(__paramv_j==NULL));
        __iter_work_args_t *__iter_j = (__iter_work_args_t *) &__paramv_j->paramv[__paramc];
        assert(&__iter_j->paramv[0] == &__paramv_j->paramv[__paramc]);
        /* paramv */
        for(int i=0;i<__frame->paramv.vars.std.context.paramc;i++) {
          __paramv_j->paramv[i] = __frame->paramv.paramv[i];
        }
/* RAG -- need to account for __paramc possibily being bigger than __frame->vars.std.context.paramc */
        for(int i=__frame->paramv.vars.std.context.paramc;i<__paramc;i++) {
          __paramv_j->paramv[i] = __paramv->paramv[i];
        }
        __paramv_j->context.new_pc    = ocrPtrFromSelectorOffset( __template_iter_work.guid, (size_t)NULL ),
        __paramv_j->context.old_pc    = ocrPtrFromSelectorOffset( __template_uts_TreeSearch.guid,
                                        (size_t)(&&__uts_TreeSearch_top_of_loop_j - &&__uts_TreeSearch_offset_zero) );

        /* parameters */
        __iter_j->vars.first = 0;
        __iter_j->vars.last  = __rhsGblOcrPtr(int,__computeGranularity);
        __iter_j->vars.inc   = 1;
        __iter_j->vars.blk   = -1;

TRACE1("uts_TreeSearch about to create finish edt for __iter_work for_j");

        ocrGuid_t __edt_iter_work, __finish_event_iter_work;
        __ret_val = ocrEdtCreate(&__edt_iter_work, __template_iter_work.guid,
                               (uint32_t)(__paramc+WORDS_iter_work_args_t), &__paramv_j->paramv[0],
                               __depc, NULL,
                               EDT_PROP_FINISH, NULL_HINT, &__finish_event_iter_work);
        CHECK_RET_VAL("uts_TreeSearch()->ocrEdtCreate",__ret_val);
#ifdef DEBUG
printf("__finish_event_iter_work = %" PRIx64 "\n",__finish_event_iter_work);fflush(stdout);
#endif

TRACE1("uts_TreeSearch about to pass params on to __iter_wait for for_j");
        __paramv_j->context.new_pc = ocrPtrFromSelectorOffset( __template_iter_wait.guid, (size_t)NULL ),
        __paramv_j->context.old_pc = ocrPtrFromSelectorOffset( __template_uts_TreeSearch.guid,
                                     (size_t)(&&__uts_TreeSearch_bottom_of_loop_j - &&__uts_TreeSearch_offset_zero) );
TRACE1("uts_TreeSearch about to create edt for __iter_wait j");

        ocrGuid_t __edt_iter_wait;
        __ret_val = ocrEdtCreate(&__edt_iter_wait, __template_iter_wait.guid,
                               __paramc, &__paramv_j->paramv[0],
                               __depc+1, NULL,
                               EDT_PROP_NONE, NULL_HINT, NULL );
        CHECK_RET_VAL("uts_TreeSearch()->ocrEdtCreate",__ret_val);

TRACE1("uts_TreeSearch about to pass guids on to __iter_(wait|work) j");
/* RAG -- need to have __iter_wait ready before starting __iter_work, */
/* RAG -- so __iter_wait is ready to can catch __iter_work's finish event */
        __ret_val = ocrAddDependence( __finish_event_iter_work, __edt_iter_wait, __depc, DB_DEFAULT_MODE);
        CHECK_RET_VAL("uts_TreeSearch->__iter_wait",__ret_val);
	/* depv */
        for(int i=0;i<__frame->paramv.vars.std.context.depc;i++) {
            __ret_val = ocrAddDependence( __frame->depv.guids[i], __edt_iter_wait, i, DB_DEFAULT_MODE);
            CHECK_RET_VAL("uts_TreeSearch->__iter_wait",__ret_val);
            __ret_val = ocrAddDependence( __frame->depv.guids[i], __edt_iter_work, i, DB_DEFAULT_MODE);
            CHECK_RET_VAL("uts_TreeSearch->__iter_work",__ret_val);
        }
/* RAG -- need to account for __depc possibily being bigger than __frame->vars.std.context.depc */
        for(int i=__frame->paramv.vars.std.context.depc;i<__depc;i++) {
            __ret_val = ocrAddDependence( __depv->depv[i].guid, __edt_iter_wait, i, DB_DEFAULT_MODE);
            CHECK_RET_VAL("uts_TreeSearch->__iter_wait",__ret_val);
            __ret_val = ocrAddDependence( __depv->depv[i].guid, __edt_iter_work, i, DB_DEFAULT_MODE);
            CHECK_RET_VAL("uts_TreeSearch->__iter_work",__ret_val);
        }
#ifdef DEBUG
TRACE0("uts_TreeSearch()<->iter_(work|wait) j");
#endif
        FREE(__paramv_j);
        return NULL_GUID;
 } /* CLOSE prep for_j */
__uts_TreeSearch_top_of_loop_j:
 { /* OPEN body for_j */
TRACE0("uts_TreeSearch()@top_of_loop_j");
        __iter_work_args_t *__iter_j = (__iter_work_args_t *) &__paramv->paramv[__paramc-WORDS_iter_work_args_t];
        /* paramv */
assert(__iter_j->vars.first==0);
assert(__iter_j->vars.last!=__iter_j->vars.first);
assert(__iter_j->vars.inc==1);
        __frame->indent1.blk1.jFirst = __iter_j->vars.first;
        __frame->indent1.blk1.jLast  = __iter_j->vars.last;
        __frame->indent1.blk1.jInc   = __iter_j->vars.inc;
#ifdef DEBUG
printf("FOR_J(%d;%d;%d)\n",__frame->indent1.blk1.jFirst,__frame->indent1.blk1.jLast,__frame->indent1.blk1.jInc);fflush(stdout);
#endif
        for (__frame->indent1.blk1.j = ((const int)__frame->indent1.blk1.jFirst);
             __frame->indent1.blk1.j < ((const int)__frame->indent1.blk1.jLast);
             __frame->indent1.blk1.j +=((const int)__frame->indent1.blk1.jInc)) {
#ifdef DEBUG
printf("  BEGIN BODY FOR_J %d\n",__frame->indent1.blk1.j);fflush(stdout);
#endif
        // TBD:  add parent height to spawn
            rng_spawn(__frame->parent->state.state, __frame->indent1.blk1.child.state.state, __frame->indent1.blk1.i);
#ifdef DEBUG
printf("  END   BODY FOR_J %d\n",__frame->indent1.blk1.j);fflush(stdout);
#endif
        } /* end for j */
TRACE0("uts_TreeSearch()<->top_of_loop_j");
        return NULL_GUID;
 } /* CLOSE body for_j */
__uts_TreeSearch_bottom_of_loop_j:
TRACE0("uts_TreeSearch()@bottom_of_loop_j");
 { /* OPEN call TreeSearch */
#ifdef DEBUG
printf("&child = %p\n",&__frame->indent1.blk1.child);fflush(stdout);
printf("child.type = %d\n",__frame->indent1.blk1.child.type);fflush(stdout);
printf("child.height = %d\n",__frame->indent1.blk1.child.height);fflush(stdout);
printf("child.numChildren = %d\n",__frame->indent1.blk1.child.numChildren);fflush(stdout);
#endif /* DEBUG */

TRACE1("uts_TreeSearch about to pass params on to uts_TreeSearch");

        __uts_TreeSearch_args_t  __paramv_0 = {
        /* paramv */
          .vars.std.context.new_pc    = ocrPtrFromSelectorOffset( __template_uts_TreeSearch.guid, (size_t)NULL ),
          .vars.std.context.old_pc    = ocrPtrFromSelectorOffset( __template_uts_TreeSearch.guid,
                                           (size_t)(&&__uts_TreeSearch_TreeSearch_returned - &&__uts_TreeSearch_offset_zero) ),
          .vars.std.context.new_frame = __ocrNULL.offset,
          .vars.std.context.old_frame = __FRAME.offset,
          .vars.std.context.paramc    = WORDS_uts_TreeSearch_args_t,
          .vars.std.context.depc      = WORDS_uts_TreeSearch_guids_t,
        /* parameters */
          .vars.parent = __frame->__CHILD.offset
        };

TRACE1("uts_TreeSearch about to pass guids on to uts_TreeSearch");

        /* depv */
        __uts_TreeSearch_guids_t __depv_0 = {
          .selector.new_frame = __ocrNULL.guid,
          .selector.old_frame = __FRAME.guid,
          .selector.parent    = __frame->__CHILD.guid };

TRACE1("uts_TreeSearch about to create edt for uts_TreeSearch");

        ocrGuid_t __edt_uts_TreeSearch;
        __ret_val = ocrEdtCreate(&__edt_uts_TreeSearch,             __paramv_0.vars.std.context.new_pc.guid,
                               __paramv_0.vars.std.context.paramc, &__paramv_0.paramv[0],
                               __paramv_0.vars.std.context.depc,   &__depv_0.guids[0],
                               EDT_PROP_NONE, NULL_HINT, NULL );
        CHECK_RET_VAL("uts_TreeSearch()->ocrEdtCreate",__ret_val);

TRACE0("uts_TreeSearch()<->TreeSearch");
 } /* CLOSE call TreeSearch */
        return NULL_GUID;
/*---------------------------------------------------------------------------------------*/
__uts_TreeSearch_TreeSearch_returned:
TRACE0("uts_TreeSearch()@TreeSearch_returned");
#ifdef STATS
#ifdef DEBUG
        printf("numNode %" PRIu64 " += %" PRIu64 "\n",__frame->parent->numNodes,  __frame->indent1.blk1.child.numNodes );fflush(stdout);
#endif
        __sync_fetch_and_add( &__frame->parent->numNodes,  __frame->indent1.blk1.child.numNodes );
        __sync_fetch_and_add( &__frame->parent->numLeaves, __frame->indent1.blk1.child.numLeaves );
        __frame->oldVal = __frame->parent->maxDepth;
        while( __frame->oldVal < __frame->indent1.blk1.child.maxDepth )
               __frame->oldVal = __sync_val_compare_and_swap ( &__frame->parent->maxDepth, __frame->oldVal, __frame->indent1.blk1.child.maxDepth);
#endif /* STATS */

#ifdef DEBUG
printf("END   BODY FOR_I %d\n",__frame->indent1.blk1.i);fflush(stdout);
#endif
#ifdef RAG_FOR_I
__frame->indent1.blk1.i+=__frame->indent1.blk1.iInc;
goto __uts_TreeSearch_top_of_LOOP_I;
__uts_TreeSearch_bottom_of_LOOP_I:
#else // RAG_FOR_I
    } /* end for i */
#endif // RAG_FOR_I
TRACE0("uts_TreeSearch()@end_for_i");
return NULL_GUID;
 } /* CLOSE body for_i */
__uts_TreeSearch_bottom_of_loop_i:
TRACE0("uts_TreeSearch()@bottom_of_loop_i");
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#else /* not PARALLEL */
//------------------------------------------------------------------------------------------------------------------------------
    __frame->indent1.blk1.iInc   = 1;
#ifdef DEBUG
printf("FOR_I(%d;%d;%d)\n",__frame->indent1.blk1.iFirst,__frame->indent1.blk1.iLast,__frame->indent1.blk1.iInc);fflush(stdout);
#endif
    __frame->indent1.blk1.i = ((const int)__frame->indent1.blk1.iFirst);
#ifdef DEBUG
printf("TOP body FOR_I\n");fflush(stdout);
#endif
/* for(i=first;i<last;i+=inc) */
{
rag_top_loop_i:
    if(__frame->indent1.blk1.i >= ((const int)__frame->indent1.blk1.iLast) ) goto rag_bottom_loop_i;
#ifdef DEBUG
printf("BEGIN body FOR_I %d\n",__frame->indent1.blk1.i);fflush(stdout);
#endif
        __frame->indent1.blk1.child.type   = ((const int)__frame->indent1.blk1.childType);
        __frame->indent1.blk1.child.height = ((const int)__frame->childHeight);
        __frame->indent1.blk1.child.numChildren = -1;    // not yet determined
#ifdef STATS
        __frame->indent1.blk1.child.maxDepth  = ((const int)__frame->childHeight);
        __frame->indent1.blk1.child.numNodes  = 0;
        __frame->indent1.blk1.child.numLeaves = 0;
#endif /* STATS */

TRACE1("// computeGranularity controls number of rng_spawn calls per node");

        __frame->indent1.blk1.jFirst = 0;
        __frame->indent1.blk1.jLast  = __rhsGblOcrPtr(int,__computeGranularity);
        __frame->indent1.blk1.jInc   = 1;
#ifdef DEBUG
printf("FOR_J(%d;%d;%d)\n",__frame->indent1.blk1.jFirst,__frame->indent1.blk1.jLast,__frame->indent1.blk1.jInc);fflush(stdout);
#endif
        for (__frame->indent1.blk1.j = ((const int)__frame->indent1.blk1.jFirst);
             __frame->indent1.blk1.j < ((const int)__frame->indent1.blk1.jLast);
             __frame->indent1.blk1.j +=((const int)__frame->indent1.blk1.jInc)) {
#ifdef DEBUG
printf("  BEGIN body FOR_J %d\n",__frame->indent1.blk1.j);fflush(stdout);
#endif
        // TBD:  add parent height to spawn
            rng_spawn(__frame->parent->state.state, __frame->indent1.blk1.child.state.state, __frame->indent1.blk1.i);
#ifdef DEBUG
printf("  END   body FOR_J %d\n",__frame->indent1.blk1.j);fflush(stdout);
#endif
        } /* end for j */
 { /* OPEN call TreeSearch */
#ifdef DEBUG
printf("&child = %p\n",&__frame->indent1.blk1.child);fflush(stdout);
printf("child.type = %d\n",__frame->indent1.blk1.child.type);fflush(stdout);
printf("child.height = %d\n",__frame->indent1.blk1.child.height);fflush(stdout);
printf("child.numChildren = %d\n",__frame->indent1.blk1.child.numChildren);fflush(stdout);
#endif /* DEBUG */

TRACE1("uts_TreeSearch about to pass params on to uts_TreeSearch");

  assert( &&__uts_TreeSearch_TreeSearch_returned > &&__uts_TreeSearch_offset_zero ); // RAG -- new check

        __uts_TreeSearch_args_t  __paramv_0 = {
        /* paramv */
          .vars.std.context.new_pc    = ocrPtrFromSelectorOffset( __template_uts_TreeSearch.guid, (size_t)NULL ),
          .vars.std.context.old_pc    = ocrPtrFromSelectorOffset( __template_uts_TreeSearch.guid,
                                           (size_t)(&&__uts_TreeSearch_TreeSearch_returned - &&__uts_TreeSearch_offset_zero) ),
          .vars.std.context.new_frame = __ocrNULL.offset,
          .vars.std.context.old_frame = __FRAME.offset,
          .vars.std.context.paramc    = WORDS_uts_TreeSearch_args_t,
          .vars.std.context.depc      = WORDS_uts_TreeSearch_guids_t,
        /* parameters */
          .vars.parent = __frame->__CHILD.offset
        };

  assert( ((int64_t)__paramv_0.vars.std.context.old_pc.offset) >= ((int64_t)NULL) ); // RAG -- new check
  assert( ((int64_t)__paramv_0.vars.std.context.new_pc.offset) >= ((int64_t)NULL) ); // RAG -- new check

TRACE1("uts_TreeSearch about to pass guids on to uts_TreeSearch");

        /* depv */
        __uts_TreeSearch_guids_t __depv_0 = {
          .selector.new_frame = __ocrNULL.guid,
          .selector.old_frame = __FRAME.guid,
          .selector.parent    = __frame->__CHILD.guid };

TRACE1("uts_TreeSearch about to create edt for uts_TreeSearch");

        ocrGuid_t __edt_uts_TreeSearch;
        __ret_val = ocrEdtCreate(&__edt_uts_TreeSearch,             __paramv_0.vars.std.context.new_pc.guid,
                               __paramv_0.vars.std.context.paramc, &__paramv_0.paramv[0],
                               __paramv_0.vars.std.context.depc,   &__depv_0.guids[0],
                               EDT_PROP_NONE, NULL_HINT, NULL );
        CHECK_RET_VAL("uts_TreeSearch()->ocrEdtCreate",__ret_val);

TRACE0("uts_TreeSearch()<->TreeSearch");
        return NULL_GUID;
 } /* CLOSE call TreeSearch */
/*---------------------------------------------------------------------------------------*/
__uts_TreeSearch_TreeSearch_returned:
TRACE0("uts_TreeSearch()@TreeSearch_returned");
#ifdef STATS
#ifdef DEBUG
        printf("numNode %" PRIu64 " += %" PRIu64 "\n",__frame->parent->numNodes,  __frame->indent1.blk1.child.numNodes );fflush(stdout);
#endif
        __frame->parent->numNodes  += __frame->indent1.blk1.child.numNodes;
        __frame->parent->numLeaves += __frame->indent1.blk1.child.numLeaves;
        if( __frame->parent->maxDepth < __frame->indent1.blk1.child.maxDepth ) {
            __frame->parent->maxDepth = __frame->indent1.blk1.child.maxDepth;
//printf("Max Depth = %d\n",__frame->parent->maxDepth);fflush(stdout);
        }
#endif /* STATS */
#ifdef DEBUG
printf("END   body FOR_I %d\n",__frame->indent1.blk1.i);fflush(stdout);
#endif
          __frame->indent1.blk1.i +=((const int)__frame->indent1.blk1.iInc);
goto rag_top_loop_i;
    } /* end for i */
rag_bottom_loop_i:;
#ifdef DEBUG
printf("BOTTOM body FOR_I\n");fflush(stdout);
#endif
 } /* CLOSE body for_i */
TRACE0("uts_TreeSearch()@end_for_i");
//------------------------------------------------------------------------------------------------------------------------------
#endif /* PARALLEL */
  } else { /* numChildren */
TRACE0("uts_TreeSearch()@update_numLeaves");
#ifdef STATS
#ifdef PARALLEL
      __sync_fetch_and_add( &__frame->parent->numLeaves,1 );
#else /* PARALLEL */
      __frame->parent->numLeaves++;
#endif /* PARALLEL */
#endif /* STATS */
  } /* end if numChildren */

#ifdef DEBUG
printf("&PARENT = %p\n",__frame->parent);fflush(stdout);
printf("PARENT->type = %d\n",__frame->parent->type);fflush(stdout);
printf("PARENT->height = %d\n",__frame->parent->height);fflush(stdout);
printf("PARENT->numChildren = %d\n",__frame->parent->numChildren);fflush(stdout);
#endif

TRACE1("uts_TreeSearch about to create edt for for continuation");
  assert(  IS_GUID_EQUAL(__frame->paramv.vars.std.context.old_pc.guid, __template_uts_main.guid)
        || IS_GUID_EQUAL(__frame->paramv.vars.std.context.old_pc.guid, __template_uts_TreeSearch.guid) );

  __continuation_args_t *__old_frame = (__continuation_args_t *)castablePtrFromSegmentOffset(__depv->segment.old_frame,
                                       __paramv->vars.std.context.old_frame);
  assert(__old_frame != NULL);

  __old_frame->context.new_pc = __frame->paramv.vars.std.context.old_pc;
  ocrGuid_t __edt_continuation_guid;
  ocrPtrFree(__FRAME);
  __ret_val = ocrEdtCreate(&__edt_continuation_guid, __old_frame->context.new_pc.guid,
                         __old_frame->context.paramc, (uint64_t *)((void *)__old_frame),
                         __old_frame->context.depc,   (ocrGuid_t *)((void *)__old_frame+ __old_frame->context.paramc*sizeof(uint64_t)),
                         EDT_PROP_NONE, NULL_HINT, NULL );
  CHECK_RET_VAL("uts_TreeSearch->ocrEdtCreate",__ret_val);

TRACE0("uts_TreeSearch()<>offset_zero");
  return NULL_GUID;
} /* end uts_TreeSearch() */

/*
 * void uts_initTreeRoot( Node *root, int type ){}
 */
ocrGuid_t uts_initTreeRoot( uint32_t __paramc, __uts_initTreeRoot_args_t  *__paramv,
                            uint32_t __depc,   __uts_initTreeRoot_guids_t *__depv ) {
  int __ret_val;
  assert( __paramc == WORDS_uts_initTreeRoot_args_t );
  assert( __depc   == WORDS_uts_initTreeRoot_guids_t );
  assert( IS_GUID_EQUAL(__paramv->vars.std.context.new_pc.guid, __template_uts_initTreeRoot.guid) );
  printf("Enter uts_initTreeRoot (paramc = %d, depc = %d) %s\n",
         __paramc, __depc,
         IS_GUID_NULL(__depv->segment.new_frame.guid) ? "new frame" : "reuse frame" );fflush(stdout);

#define WORDS_uts_initTreeRoot_locals_t ((sizeof(__uts_initTreeRoot_locals_t)+sizeof(uint64_t)-1)/sizeof(uint64_t))
  typedef struct __uts_initTreeRoot_locals_t {
  /* paramv */
    __uts_initTreeRoot_args_t paramv;
  /* depv */
    __uts_initTreeRoot_guids_t depv;
  /* parameters */
    Node *root;
    int   type;
  /* locals */
  /* more locals */
    ocrPtr_t __ROOT;
  } __uts_initTreeRoot_locals_t;

  ocrPtr_t __FRAME = __ocrNULL;
  __uts_initTreeRoot_locals_t *__frame = (__uts_initTreeRoot_locals_t *) castablePtrFromSegmentOffset( __depv->segment.new_frame,
                                          __paramv->vars.std.context.new_frame);
  if ( __frame == NULL ) {
#ifdef DEBUG
printf("uts_initTreeRoot() build new frame\n");fflush(stdout);
#endif
    __frame = (__uts_initTreeRoot_locals_t *)ocrPtrMalloc( &__FRAME, /* room for paramv, depv, parameters, locals, and more locals */
              WORDS_uts_initTreeRoot_locals_t*sizeof(uint64_t)+__paramc*sizeof(uint64_t)+__depc*sizeof(ocrGuid_t) );
  /* paramv */
    __frame->paramv.vars.std.context.new_pc    = __paramv->vars.std.context.new_pc;
    __frame->paramv.vars.std.context.old_pc    = __paramv->vars.std.context.old_pc;
    __frame->paramv.vars.std.context.new_frame = __FRAME.offset;
    __frame->paramv.vars.std.context.old_frame = __paramv->vars.std.context.old_frame;
    __frame->paramv.vars.std.context.paramc    = __paramc;
    __frame->paramv.vars.std.context.depc      = __depc;
    __frame->paramv.vars.root                  = __paramv->vars.root;
  /* depv */
    __frame->depv.selector.new_frame = __FRAME.guid;
    __frame->depv.selector.old_frame = __depv->segment.old_frame.guid;
    __frame->depv.selector.root      = __depv->segment.root.guid;
  /* parameters */
    __frame->root = (Node *) castablePtrFromSegmentOffset( __depv->segment.root, __paramv->vars.root);
    __frame->type = __paramv->vars.type;
  /* locals */
  /* more locals */
    __frame->__ROOT = ocrPtrFromSelectorOffset( __depv->segment.root.guid, __paramv->vars.root);
  } else {
#ifdef DEBUG
printf("uts_initTreeRoot() reuse frame\n");fflush(stdout);
#endif
    __FRAME = ocrPtrFromSelectorOffset( __depv->segment.new_frame.guid, __paramv->vars.std.context.new_frame );
  /* paramv */
  /* depv */
  /* parameters */
    __frame->root = (Node *) castablePtrFromSegmentOffset( __depv->segment.root, __paramv->vars.root);
  /* locals */
  /* more locals */
  }
#ifdef DEBUG
printf("&root = %p\n",__frame->root);fflush(stdout);
printf("root->type = %d\n",__frame->root->type);fflush(stdout);
printf("root->height = %d\n",__frame->root->height);fflush(stdout);
printf("root->numChildren = %d\n",__frame->root->numChildren);fflush(stdout);
printf("root->state.state = "); for(int i=0;i<20;i++)printf("%2.2x",__frame->root->state.state[i]); printf("\n");fflush(stdout);
#endif

  assert( ((int64_t)__frame->paramv.vars.std.context.old_pc.offset) >= ((int64_t)NULL) ); // RAG -- new check
  assert( ((int64_t)__frame->paramv.vars.std.context.new_pc.offset) >= ((int64_t)NULL) ); // RAG -- new check

  assert( (__frame->paramv.vars.std.context.old_pc.offset) == (__paramv->vars.std.context.old_pc.offset) ); // RAG -- new check
  assert( (__frame->paramv.vars.std.context.new_pc.offset) == (__paramv->vars.std.context.new_pc.offset) ); // RAG -- new check

  goto *(void *)( &&__uts_initTreeRoot_offset_zero + __frame->paramv.vars.std.context.new_pc.offset );
__uts_initTreeRoot_offset_zero:
TRACE0("uts_initTreeRoot()@offset_zero\n");

  __frame->root->type = __frame->type;
  __frame->root->height = 0;
  __frame->root->numChildren = -1;      // means not yet determined
#ifdef STATS
  __frame->root->maxDepth  = 0; /* my starting height */
  __frame->root->numNodes  = 1; /* me */
  __frame->root->numLeaves = 0; /* no leaf nodes yet */
#endif
  rng_init(__frame->root->state.state, __rhsGblOcrPtr(int,__rootId));

#ifdef DEBUG
printf("&root = %p\n",__frame->root);fflush(stdout);
printf("root->type = %d\n",__frame->root->type);fflush(stdout);
printf("root->height = %d\n",__frame->root->height);fflush(stdout);
printf("root->numChildren = %d\n",__frame->root->numChildren);fflush(stdout);
printf("root->state.state = "); for(int i=0;i<20;i++)printf("%2.2x",__frame->root->state.state[i]); printf("\n");fflush(stdout);
#endif

TRACE1("uts_initTreeRoot about to create edt for for continuation");
  assert( IS_GUID_EQUAL(__frame->paramv.vars.std.context.old_pc.guid, __template_uts_main.guid) );

  __continuation_args_t *__old_frame = (__continuation_args_t *)castablePtrFromSegmentOffset(__depv->segment.old_frame,
                                       __paramv->vars.std.context.old_frame);
  assert(__old_frame != NULL);

  __old_frame->context.new_pc = __frame->paramv.vars.std.context.old_pc;
  ocrGuid_t __edt_continuation_guid;
  ocrPtrFree(__FRAME);
  __ret_val = ocrEdtCreate(&__edt_continuation_guid, __old_frame->context.new_pc.guid,
                         __old_frame->context.paramc, (uint64_t *)((void *)__old_frame),
                         __old_frame->context.depc,   (ocrGuid_t *)((void *)__old_frame+ __old_frame->context.paramc*sizeof(uint64_t)),
                         EDT_PROP_NONE, NULL_HINT, NULL );
  CHECK_RET_VAL("uts_initTreeRoot->ocrEdtCreate",__ret_val);

TRACE0("uts_initTreeRoot()<>offset_zero");
  return NULL_GUID;
} /* uts_initTreeRoot() */

//----------------------------------------------------------------------------
//  uts_main() function for "ocr"
//----------------------------------------------------------------------------

ocrGuid_t uts_main( uint32_t __paramc, __uts_main_args_t  *__paramv,
                    uint32_t __depc,   __uts_main_guids_t *__depv ) {
  int __ret_val;

#define WORDS_uts_main_locals_t ((sizeof(__uts_main_locals_t)+sizeof(uint64_t)-1)/sizeof(uint64_t))
  typedef struct __uts_main_locals_t {
  /* paramv */
    __uts_main_args_t  paramv;
  /* depv */
    __uts_main_guids_t depv;
  /* parameters */
    int argc;
    char **argv;
  /* locals */
    Node root;
    double t1, t2, et;
  /* more locals */
    ocrPtr_t *__argv;
  } __uts_main_locals_t;

  assert( __paramc == WORDS_uts_main_args_t );
  assert( __depc   == WORDS_uts_main_guids_t );
  assert( IS_GUID_EQUAL(__paramv->vars.std.context.new_pc.guid, __template_uts_main.guid) );
  printf("Enter uts_main (paramc = %d, depc = %d) %s\n",
         __paramc, __depc,
         IS_GUID_NULL(__depv->segment.new_frame.guid) ? "new frame" : "reuse frame" );fflush(stdout);

  ocrPtr_t  __FRAME = __ocrNULL;
  __uts_main_locals_t *__frame =
 (__uts_main_locals_t *) castablePtrFromSegmentOffset( __depv->segment.new_frame,
                                                       __paramv->vars.std.context.new_frame);
  if( __frame == NULL ) {
#ifdef DEBUG
printf("uts_main() build new frame\n");fflush(stdout);
#endif
    __frame = (__uts_main_locals_t *)ocrPtrMalloc( &__FRAME, /* room for paramv, depv, parameters, locals, and more locals */
              WORDS_uts_main_locals_t*sizeof(uint64_t)+__paramc*sizeof(uint64_t)+__depc*sizeof(ocrGuid_t) );
  /* paramv */
    __frame->paramv.vars.std.context.new_pc    = __paramv->vars.std.context.new_pc;
    __frame->paramv.vars.std.context.old_pc    = __paramv->vars.std.context.old_pc;
    __frame->paramv.vars.std.context.new_frame = __FRAME.offset;
    __frame->paramv.vars.std.context.old_frame = __paramv->vars.std.context.old_frame;
    __frame->paramv.vars.std.context.paramc    = __paramc;
    __frame->paramv.vars.std.context.depc      = __depc;
    __frame->paramv.vars.argv                  = __paramv->vars.argv;
  /* depv */
    __frame->depv.selector.new_frame = __FRAME.guid;
    __frame->depv.selector.old_frame = __depv->segment.old_frame.guid;
    __frame->depv.selector.argv      = __depv->segment.argv.guid;
  /* parameters */
    __frame->argc = __paramv->vars.argc;
    __frame->argv = (char **)NULL;
  /* locals */
    __frame->root.type        = 0;
    __frame->root.height      = 0;
    __frame->root.numChildren = 0;
    memset(&__frame->root.state.state[0],0,sizeof(__frame->root.state.state));
    __frame->t1               = 0.0;
    __frame->t2               = 0.0;
    __frame->et               = 0.0;
  /* more locals */
    __frame->__argv = (ocrPtr_t *) castablePtrFromSegmentOffset( __depv->segment.argv, __paramv->vars.argv);
  }  else {
#ifdef DEBUG
printf("uts_main() reuse frame\n");fflush(stdout);
#endif
    __FRAME = ocrPtrFromSelectorOffset( __depv->segment.new_frame.guid , __paramv->vars.std.context.new_frame );
  /* paramv */
  /* depv */
  /* parameters */
  /* locals */
  /* more locals */
    __frame->__argv                   = (ocrPtr_t *) castablePtrFromSegmentOffset( __depv->segment.argv, __paramv->vars.argv);
  }

#ifdef DEBUG
printf("new_pc      %16.16" PRIx64 ":%16.16" PRIx64 "\n",
       __frame->paramv.vars.std.context.new_pc.guid,
       __frame->paramv.vars.std.context.new_pc.offset); fflush(stdout);
printf("old_pc      %16.16" PRIx64 ":%16.16" PRIx64 "\n",
       __frame->paramv.vars.std.context.old_pc.guid,
       __frame->paramv.vars.std.context.old_pc.offset); fflush(stdout);
printf("old_frame   %16.16" PRIx64 ":%16.16" PRIx64 "\n",
       __frame->depv.selector.old_frame,
       __frame->paramv.vars.std.context.old_frame);     fflush(stdout);
printf("new_frame   %16.16" PRIx64 ":%16.16" PRIx64 "\n",
       __frame->depv.selector.new_frame,
       __frame->paramv.vars.std.context.new_frame);     fflush(stdout);
printf("paramc      %8" PRIu32 "\n", __paramc);         fflush(stdout);
printf("depc        %8" PRIu32 "\n", __depc);           fflush(stdout);
#endif
#ifdef DEBUG
printf("&root = %p\n",&__frame->root); fflush(stdout);
printf("root.type = %d\n",__frame->root.type); fflush(stdout);
printf("root.height = %d\n",__frame->root.height); fflush(stdout);
printf("root.numChildren = %d\n",__frame->root.numChildren); fflush(stdout);
printf("root.state.state = "); for(int i=0;i<20;i++)printf("%2.2x",__frame->root.state.state[i]); printf("\n"); fflush(stdout);
#endif

  ocrPtr_t  __ROOT  = ocrPtrFromSelectorOffset( __FRAME.guid,
                      __FRAME.offset + offsetof(__uts_main_locals_t, root) );

  assert( ((int64_t)__paramv->vars.std.context.old_pc.offset) >= ((int64_t)NULL) ); // RAG -- new check
  assert( ((int64_t)__paramv->vars.std.context.new_pc.offset) >= ((int64_t)NULL) ); // RAG -- new check

  goto *(void *)( &&__uts_main_offset_zero + __paramv->vars.std.context.new_pc.offset );
/*---------------------------------------------------------------------------------------*/
__uts_main_offset_zero:
 {
TRACE0("uts_main()@offset_zero\n");
TRACE1("uts_main about to pass params on to uts_parseParams");
  __uts_parseParams_args_t __paramv_0 = {
  /* paramv */
    .vars.std.context.new_pc    = ocrPtrFromSelectorOffset( __template_uts_parseParams.guid,
                                     (size_t)( NULL) ),
    .vars.std.context.old_pc    = ocrPtrFromSelectorOffset( __template_uts_main.guid,
                                     (size_t)(&&__uts_main_parseParams_returned - &&__uts_main_offset_zero) ),
    .vars.std.context.new_frame = __ocrNULL.offset,
    .vars.std.context.old_frame = __FRAME.offset,
    .vars.std.context.paramc    = WORDS_uts_parseParams_args_t,
    .vars.std.context.depc      = WORDS_uts_parseParams_guids_t,
  /* parameters */
    .vars.argc = (const int)__frame->argc,
    .vars.argv = __frame->paramv.vars.argv,
  /* locals */
  /* more locals */
  };

TRACE1("uts_main about to pass guids on to uts_parseParams");

  /* depv */
  __uts_parseParams_guids_t __depv_0 = {
    .selector.new_frame = __ocrNULL.guid,
    .selector.old_frame = __FRAME.guid,
    .selector.argv      = __frame->depv.selector.argv,
  };

  ocrGuid_t __edt_uts_parseParams_guid;
  __ret_val = ocrEdtCreate(&__edt_uts_parseParams_guid, __template_uts_parseParams.guid,
                         WORDS_uts_main_args_t,  &__paramv_0.paramv[0],
                         WORDS_uts_main_guids_t, &__depv_0.guids[0],
                         EDT_PROP_NONE, NULL_HINT, NULL );
  printf("uts_main()->ocrEdtCreate (paramc = %d, depc = %d)\n",
                         WORDS_uts_parseParams_args_t,
                         WORDS_uts_parseParams_guids_t);fflush(stdout);
  CHECK_RET_VAL("uts_main()->ocrEdtCreate",__ret_val);

TRACE0("uts_main()<>offset_zero");
  return NULL_GUID;
 }
/*---------------------------------------------------------------------------------------*/
__uts_main_parseParams_returned:
 {
TRACE0("uts_main()@parseParams_returned\n");
TRACE0("/* show parameter settings */");
TRACE1("uts_main about to pass params on to uts_printParams");
   __uts_printParams_args_t  __paramv_0 = {
   /* paramv */
     .vars.std.context.new_pc    = ocrPtrFromSelectorOffset( __template_uts_printParams.guid, (size_t)NULL ),
     .vars.std.context.old_pc    = ocrPtrFromSelectorOffset( __template_uts_main.guid,
                                      (size_t)(&&__uts_main_printParams_returned - &&__uts_main_offset_zero) ),
     .vars.std.context.new_frame = __ocrNULL.offset,
     .vars.std.context.old_frame = __FRAME.offset,
     .vars.std.context.paramc    = WORDS_uts_printParams_args_t,
     .vars.std.context.depc      = WORDS_uts_printParams_guids_t,
   /* parameters */
  };

TRACE1("uts_main about to pass guids on to uts_printParams");

  /* depv */
  __uts_printParams_guids_t __depv_0 = {
    .selector.new_frame = __ocrNULL.guid,
    .selector.old_frame = __FRAME.guid,
  };

TRACE1("uts_main about to create edt for uts_printParams");

  ocrGuid_t __edt_uts_printParams;
  __ret_val = ocrEdtCreate(&__edt_uts_printParams,  __paramv_0.vars.std.context.new_pc.guid,
                         __paramv_0.vars.std.context.paramc, &__paramv_0.paramv[0],
                         __paramv_0.vars.std.context.depc,   &__depv_0.guids[0],
                         EDT_PROP_NONE, NULL_HINT, NULL );
  CHECK_RET_VAL("uts_main()->ocrEdtCreate",__ret_val);

TRACE0("uts_main()<->printParams");
  return NULL_GUID;
 }
/*---------------------------------------------------------------------------------------*/
__uts_main_printParams_returned:
 {
TRACE0("uts_main()@printParams_returned");

TRACE0("/* initialize Tree Root */");
TRACE1("uts_main about to pass params on to uts_initTreeRoot");
  __uts_initTreeRoot_args_t __paramv_0 = {
  /* paramv */
    .vars.std.context.new_pc    = ocrPtrFromSelectorOffset( __template_uts_initTreeRoot.guid, (size_t)NULL ),
    .vars.std.context.old_pc    = ocrPtrFromSelectorOffset( __template_uts_main.guid,
                                     (size_t)(&&__uts_main_initTreeRoot_returned - &&__uts_main_offset_zero) ),
    .vars.std.context.new_frame = __ocrNULL.offset,
    .vars.std.context.old_frame = __FRAME.offset,
    .vars.std.context.paramc    = WORDS_uts_initTreeRoot_args_t,
    .vars.std.context.depc      = WORDS_uts_initTreeRoot_guids_t,
  /* parameters */
    .vars.root = __ROOT.offset,
    .vars.type = __rhsGblOcrPtr(tree_t,__type)
  };

TRACE1("uts_main about to pass guids on to uts_initTreeRoot");
  /* depv */
  __uts_initTreeRoot_guids_t __depv_0 = { .selector.new_frame = __ocrNULL.guid,
                                          .selector.old_frame = __FRAME.guid,
                                          .selector.root      = __ROOT.guid };

TRACE1("uts_main about to create edt for uts_initTreeRoot");

  ocrGuid_t __edt_uts_initTreeRoot;
  __ret_val = ocrEdtCreate(&__edt_uts_initTreeRoot, __paramv_0.vars.std.context.new_pc.guid, //__template_uts_initTreeRoot.guid,
                         __paramv_0.vars.std.context.paramc, &__paramv_0.paramv[0],
                         __paramv_0.vars.std.context.depc,   &__depv_0.guids[0],
                         EDT_PROP_NONE, NULL_HINT, NULL );
  CHECK_RET_VAL("uts_main()->ocrEdtCreate",__ret_val);

TRACE0("uts_main()<->initTreeRoot");
  return NULL_GUID;
 }
/*---------------------------------------------------------------------------------------*/
__uts_main_initTreeRoot_returned:
TRACE0("uts_main()@initTreeRoot_returned");
#ifdef DEBUG
printf("&root = %p\n",&__frame->root);fflush(stdout);
printf("root.type = %d\n",__frame->root.type);fflush(stdout);
printf("root.height = %d\n",__frame->root.height);fflush(stdout);
printf("root.numChildren = %d\n",__frame->root.numChildren);fflush(stdout);
printf("root.state.state = "); for(int i=0;i<20;i++)printf("%2.2x",__frame->root.state.state[i]); printf("\n");fflush(stdout);
#endif

TRACE0("/* time search */");

  __frame->t1 = uts_wctime();

TRACE1("uts_main about to pass params on to uts_TreeSearch");
{
  __uts_TreeSearch_args_t  __paramv_0 = {
  /* paramv */
    .vars.std.context.new_pc    = ocrPtrFromSelectorOffset( __template_uts_TreeSearch.guid, (size_t)NULL ),
    .vars.std.context.old_pc    = ocrPtrFromSelectorOffset( __template_uts_main.guid,
                                     (size_t)(&&__uts_main_TreeSearch_returned - &&__uts_main_offset_zero) ),
    .vars.std.context.old_frame = __FRAME.offset,
    .vars.std.context.new_frame = __ocrNULL.offset,
    .vars.std.context.paramc    = WORDS_uts_TreeSearch_args_t,
    .vars.std.context.depc      = WORDS_uts_TreeSearch_guids_t,
  /* parameters */
    .vars.parent = __ROOT.offset
  };

TRACE1("uts_main about to pass guids on to uts_TreeSearch");
  /* depv */
  __uts_TreeSearch_guids_t __depv_0 = {
    .selector.new_frame = __ocrNULL.guid,
    .selector.old_frame = __FRAME.guid,
    .selector.parent = __ROOT.guid
  };

TRACE1("uts_main about to create edt for uts_TreeSearch");

  ocrGuid_t __edt_uts_TreeSearch;
  __ret_val = ocrEdtCreate(&__edt_uts_TreeSearch,             __paramv_0.vars.std.context.new_pc.guid,
                         __paramv_0.vars.std.context.paramc, &__paramv_0.paramv[0],
                         __paramv_0.vars.std.context.depc,   &__depv_0.guids[0],
                         EDT_PROP_NONE, NULL_HINT, NULL );
  CHECK_RET_VAL("uts_main()->ocrEdtCreate",__ret_val);
TRACE0("uts_main()<->TreeSearch");
  return NULL_GUID;
}
/*---------------------------------------------------------------------------------------*/
__uts_main_TreeSearch_returned:
TRACE0("uts_main()@TreeSearch_returned");

  __frame->t2 = uts_wctime();
  __frame->et = __frame->t2 - __frame->t1;

#ifdef DEBUG
printf("&root = %p\n",&__frame->root);fflush(stdout);
printf("root.type = %d\n",__frame->root.type);fflush(stdout);
printf("root.height = %d\n",__frame->root.height);fflush(stdout);
printf("root.numChildren = %d\n",__frame->root.numChildren);fflush(stdout);
printf("root.state.state = "); for(int i=0;i<20;i++)printf("%2.2x",__frame->root.state.state[i]); printf("\n");fflush(stdout);
#endif
#ifdef STATS
  printf("UTS Tree size = %" PRIu64 ", ",__frame->root.numNodes);
  printf("tree depth  = %d, ",  __frame->root.maxDepth);
  printf("num leaves = %" PRIu64 ", (%5.2f%%) ",   __frame->root.numLeaves,
         100.0*(double)__frame->root.numLeaves/(double)__frame->root.numNodes);
#endif
  printf("time = %f s\n",__frame->et);

  ocrPtrFree(__FRAME);
  ocrShutdown();
  return NULL_GUID;
} /* end uts_main() */

ocrGuid_t mainEdt( uint32_t paramc, uint64_t paramv[], uint32_t depc, ocrEdtDep_t depv[] ) {
  int __ret_val;
#if 1
/* test problem */
#if 0
T1="-t 1 -a 3 -d 10 -b 4 -r 19"
T5="-t 1 -a 0 -d 20 -b 4 -r 34"
T2="-t 1 -a 2 -d 16 -b 6 -r 502"
T3="-t 0 -b 2000 -q 0.124875 -m 8 -r 42"
T4="-t 2 -a 0 -d 16 -b 6 -r 1 -q 0.234375 -m 4 -r 1"
T1L="-t 1 -a 3 -d 13 -b 4 -r 29"
T2L="-t 1 -a 2 -d 23 -b 7 -r 220"
T3L="-t 0 -b 2000 -q 0.200014 -m 5 -r 7"
T1XL="-t 1 -a 3 -d 15 -b 4 -r 29"
T1XXL="-t 1 -a 3 -d 15 -b 4 -r 19"
T3XXL="-t 0 -b 2000 -q 0.499995 -m 2 -r 316"
T2XXL="-t 0 -b 2000 -q 0.499999995 -m 2 -r 0"
T1WL="-t 1 -a 3 -d 18 -b 4 -r 19"
T2WL="-t 0 -b 2000 -q 0.4999999995 -m 2 -r 559"
T3WL="-t 0 -b 2000 -q 0.4999995 -m 2 -r 559"
#endif
#if 0
/* T4 */
    int argc = 17;
    char *argv[] = { "uts",
	"-g", "1", "-t", "2", "-a", "0", "-d", "16", "-b", "6", "-r", "1", "-q", "0.234375", "-m", "4"
    };
#else
/* T2L */
    int argc = 13;
    char *argv[] = { "uts", /* RAG "-d" was "23" but made smaller to have a tiny problem */
	"-g", "1", "-t", "1", "-a", "2", "-d", "10", "-b", "7", "-r", "220",
    };
#endif
#elif 0
/* help */
  int argc = 2;
  char *argv[] = { "uts", "-h"};
#elif 0
  int argc = 4;
/* error */
  char *argv[] = { "uts", "-t","2","-g"};
#else
#error One of the above must be selected!
#endif

TRACE0("mainEdt about to build ocrPtr_t (i.e datablock) version of argv");

  ocrPtr_t __ARGV;
  if ( argc == 0 ) {
    __ARGV = __ocrNULL;
  } else {
    ocrPtr_t *__argv = ocrPtrMalloc( &__ARGV, ((const int)argc)*sizeof(ocrPtr_t) );
    for( int i=0;i<((const int)argc);i++) {
      strcpy( (char *)ocrPtrMalloc( &__argv[i], strlen( argv[i]+1 ) ), argv[i] );
    }
  }

TRACE0("mainEdt about to create templates");

/* c99 on OCR support */

  __ret_val = ocrEdtTemplateCreate(&__template_iter_work.guid,
				 (ocrEdt_t)__template_iter_work.offset,
                                 EDT_PARAM_UNK, EDT_PARAM_UNK );
  CHECK_RET_VAL("mainEdt()->ocrEdtTemplateCreate",__ret_val);

  __ret_val = ocrEdtTemplateCreate(&__template_iter_wait.guid,
				 (ocrEdt_t)__template_iter_wait.offset,
                                 EDT_PARAM_UNK, EDT_PARAM_UNK );
  CHECK_RET_VAL("mainEdt()->ocrEdtTemplateCreate",__ret_val);

/* users application */

  __ret_val = ocrEdtTemplateCreate(&__template_uts_main.guid,
				 (ocrEdt_t)__template_uts_main.offset,
                                 EDT_PARAM_UNK, EDT_PARAM_UNK );
  CHECK_RET_VAL("mainEdt()->ocrEdtTemplateCreate",__ret_val);

  __ret_val = ocrEdtTemplateCreate(&__template_uts_parseParams.guid,
				 (ocrEdt_t)__template_uts_parseParams.offset,
                                 EDT_PARAM_UNK, EDT_PARAM_UNK );
  CHECK_RET_VAL("mainEdt()->ocrEdtTemplateCreate",__ret_val);

  __ret_val = ocrEdtTemplateCreate(&__template_uts_printParams.guid,
				 (ocrEdt_t)__template_uts_printParams.offset,
                                 EDT_PARAM_UNK, EDT_PARAM_UNK );
  CHECK_RET_VAL("mainEdt()->ocrEdtTemplateCreate",__ret_val);

  __ret_val = ocrEdtTemplateCreate(&__template_uts_initTreeRoot.guid,
				 (ocrEdt_t)__template_uts_initTreeRoot.offset,
                                 EDT_PARAM_UNK, EDT_PARAM_UNK );
  CHECK_RET_VAL("mainEdt()->ocrEdtTemplateCreate",__ret_val);

  __ret_val = ocrEdtTemplateCreate(&__template_uts_TreeSearch.guid,
				 (ocrEdt_t)__template_uts_TreeSearch.offset,
                                 EDT_PARAM_UNK, EDT_PARAM_UNK );
  CHECK_RET_VAL("mainEdt()->ocrEdtTemplateCreate",__ret_val);

TRACE0("mainEdt about to pass parameters on to uts_main");

  __uts_main_args_t __paramv_0 = {
  /* paramv */
    .vars.std.context.new_pc    = ocrPtrFromSelectorOffset( __template_uts_main.guid, (size_t)NULL ),
    .vars.std.context.old_pc    = __ocrNULL,
    .vars.std.context.new_frame = __ocrNULL.offset,
    .vars.std.context.old_frame = __ocrNULL.offset,
    .vars.std.context.paramc    = WORDS_uts_main_args_t,
    .vars.std.context.depc      = WORDS_uts_main_guids_t,
  /* parameters */
    .vars.argc = argc,
    .vars.argv = __ARGV.offset
  };

TRACE0("mainEdt about to pass guids on to uts_main");
  /* depv */
  __uts_main_guids_t __depv_0 = {
   .selector.new_frame = __ocrNULL.guid,
   .selector.old_frame = __ocrNULL.guid,
   .selector.argv      = __ARGV.guid
  };

TRACE0("mainEdt about to create edt for uts_main");

  ocrGuid_t __edt_uts_main_guid;
  __ret_val = ocrEdtCreate(&__edt_uts_main_guid, __paramv_0.vars.std.context.new_pc.guid, //__template_uts_main.guid,
                         __paramv_0.vars.std.context.paramc, &__paramv_0.paramv[0],
                         __paramv_0.vars.std.context.depc,   &__depv_0.guids[0],
                         EDT_PROP_NONE, NULL_HINT, NULL );
  CHECK_RET_VAL("mainEdt()->ocrEdtCreate",__ret_val);

  return NULL_GUID;
} /* end mainEdt() */
