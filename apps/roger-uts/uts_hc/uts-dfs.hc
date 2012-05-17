#if !defined(RAG_SERIAL) && !defined(RAG_CILK) && !defined(RAG_HC)
#error must define one of RAG_SERIAL, RAG_CILK, or RAG_HC
#endif

#if defined(RAG_CILK)
#include <cilk/cilk.h>
#endif
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "uts.h"

#if   defined(RAG_HC)
/***********************************************************
 *     (Habanero-C) ANSI C compiler - parallel execution   *
 ***********************************************************/
#define PARALLEL         0
#define COMPILER_TYPE    0
#define SHARED
#define SHARED_INDEF
#define VOLATILE
#define MAXTHREADS 1
#define LOCK_T           void
#define GET_NUM_THREADS  1
#define GET_THREAD_NUM   0
#define SET_LOCK(zlk)    
#define UNSET_LOCK(zlk)  
#define INIT_LOCK(zlk) 
#define INIT_SINGLE_LOCK(zlk) 
#define SMEMCPY          memcpy
#define ALLOC            malloc
#elif defined(RAG_CILK)
/***********************************************************
 *     gcc cilkplus compiler - parallel execution    *
 ***********************************************************/
#define PARALLEL         0
#define COMPILER_TYPE    0
#define SHARED
#define SHARED_INDEF
#define VOLATILE
#define MAXTHREADS 1
#define LOCK_T           void
#define GET_NUM_THREADS  1
#define GET_THREAD_NUM   0
#define SET_LOCK(zlk)    
#define UNSET_LOCK(zlk)  
#define INIT_LOCK(zlk) 
#define INIT_SINGLE_LOCK(zlk) 
#define SMEMCPY          memcpy
#define ALLOC            malloc
#define BARRIER           
#elif defined(RAG_SERIAL)
/***********************************************************
 *     (default) ANSI C compiler - sequential execution    *
 ***********************************************************/
#define PARALLEL         0
#define COMPILER_TYPE    0
#define SHARED
#define SHARED_INDEF
#define VOLATILE
#define MAXTHREADS 1
#define LOCK_T           void
#define GET_NUM_THREADS  1
#define GET_THREAD_NUM   0
#define SET_LOCK(zlk)    
#define UNSET_LOCK(zlk)  
#define INIT_LOCK(zlk) 
#define INIT_SINGLE_LOCK(zlk) 
#define SMEMCPY          memcpy
#define ALLOC            malloc
#define BARRIER           
#endif


/***********************************************************
 *  Global state                                           *
 ***********************************************************/
counter_t nNodes  = 0;
counter_t nLeaves = 0;
counter_t maxTreeDepth = 0;


/***********************************************************
 *  UTS Implementation Hooks                               *
 ***********************************************************/

// The name of this implementation
char * impl_getName() {
#if   defined(RAG_HC)
  return "Habenero-C Parallel Recursive Search";
#elif defined(RAG_CILK)
  return "gcc cilkplus Parallel Recursive Search";
#elif defined(RAG_SERIAL)
  return "Sequential Recursive Search";
#else
#error must define one of RAG_SERIAL, RAG_CILK, or RAG_HC
#endif
}

int  impl_paramsToStr(char *strBuf, int ind) { 
  char *str = impl_getName();
  ind += sprintf(strBuf+ind, "Execution strategy:  %s\n", str);
  return ind;
}

// Not using UTS command line params, return non-success
int  impl_parseParam(char *param, char *value) { return 1; }

void impl_helpMessage() {
  printf("   none.\n");
}

void impl_abort(int err) {
  exit(err);
}


/***********************************************************
 * Recursive depth-first implementation                    *
 ***********************************************************/

typedef struct {
  counter_t maxdepth, size, leaves;
} Result;

Result parTreeSearch(int depth, Node *parent) {
  int numChildren, childType;
  counter_t parentHeight = parent->height;

  Result r = { depth, 1, 0 };

  numChildren = uts_numChildren(parent);
  childType   = uts_childType(parent);

  // record number of children in parent
  parent->numChildren = numChildren;

#if TRACE
printf("pTS depth=%d numChildren=%d\n",depth, numChildren);fflush(stdout);
#endif
  
  // Recurse on the children
#if defined(RAG_HC) || defined(RAG_CILK)
  if (numChildren > 0) {
    Node *child = (Node *)calloc(numChildren,sizeof(Node));
    Result *c = (Result *)calloc(numChildren,sizeof(Result));
    if(child == NULL)perror("malloc of child\n");
#if defined(RAG_HC)
    finish {
#endif
      for (int i = 0; i < numChildren; i++) {
        child[i].type = childType;
	child[i].height = parentHeight + 1;
	child[i].numChildren = -1;    // not yet determined
	for (int j = 0; j < computeGranularity; j++) {
		rng_spawn(parent->state.state, child[i].state.state, i);
	} // for j
#if   defined(RAG_HC)
        async IN(depth,i,c,child) c[i] =            parTreeSearch(depth+1, &child[i]);
#elif defined(RAG_CILK)
                                  c[i] = cilk_spawn parTreeSearch(depth+1, &child[i]);
#endif
      } // for i
#if   defined(RAG_HC)
    } // finish
#elif defined(RAG_CILK)
    cilk_sync;
#endif
    for (int i = 0; i < numChildren; i++) {
      if(r.maxdepth<c[i].maxdepth)
         r.maxdepth = c[i].maxdepth;
      r.size       += c[i].size;
      r.leaves     += c[i].leaves;
    }
    free(c);
    free(child);
  } else {
    r.leaves = 1;
  }
#elif defined(RAG_SERIAL)
  if (numChildren > 0) {
    for (int i = 0; i < numChildren; i++) {
      Node child;
      child.type = childType;
      child.height = parentHeight + 1;
      child.numChildren = -1;    // not yet determined
      for (int j = 0; j < computeGranularity; j++) {
        rng_spawn(parent->state.state, child.state.state, i);
      } // for j
      Result c = parTreeSearch(depth+1, &child);
      if (c.maxdepth>r.maxdepth) r.maxdepth = c.maxdepth;
      r.size   += c.size;
      r.leaves += c.leaves;
    } // for i
  } else {
    r.leaves = 1;
  }
#endif // RAG_SERIAL RAG_CILK and RAG_HC
#if TRACE
printf("return %d %d %d\n",r.maxdepth,r.size,r.leaves);fflush(stdout);
#endif
  return r;
}

int main(int argc, char *argv[]) {
  static Node root; // needs static to avoid stack allocation
  double t1, t2;

  uts_parseParams(argc, argv);

  if (GET_THREAD_NUM == 0) {
    uts_printParams();
    uts_initRoot(&root, type);
  }

  t1 = uts_wctime();

#if defined(RAG_HC)
  Result r;
  finish { r = parTreeSearch(0, &root); }
#elif defined(RAG_SERIAL) || defined(RAG_CILK)
  Result   r = parTreeSearch(0, &root);
#endif

  t2 = uts_wctime();

  maxTreeDepth = r.maxdepth;
  nNodes  = r.size;
  nLeaves = r.leaves;

  if (GET_THREAD_NUM == 0) {
    uts_showStats(GET_NUM_THREADS, 0, t2-t1, nNodes, nLeaves, maxTreeDepth);
  } 

  return 0;
}
