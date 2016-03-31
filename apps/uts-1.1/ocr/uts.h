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

#ifndef _UTS_H
#define _UTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rng.h"

#define UTS_VERSION "2.1"

/***********************************************************
 *  Tree node descriptor and statistics                    *
 ***********************************************************/

/* For stats generation: */
typedef uint64_t counter_t;

#define MAXNUMCHILDREN    100  // cap on children (BIN root is exempt)

struct node_t {
  int type;          // distribution governing number of children
  int height;        // depth of this node in the tree
  int numChildren;   // number of children, -1 => not yet determined
#ifdef STATS
  int maxDepth;
  counter_t numNodes;
  counter_t numLeaves;
#endif
  /* for RNG state associated with this node */
  struct state_t state;
};

typedef struct node_t Node;

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
enum   uts_trees_e    { BIN = 0, GEO, HYBRID, BALANCED };
enum   uts_geoshape_e { LINEAR = 0, EXPDEC, CYCLIC, FIXED };

typedef enum uts_trees_e    tree_t;
typedef enum uts_geoshape_e geoshape_t;

/* Strings for the above enums */
extern char * uts_trees_str[];
extern char * uts_geoshapes_str[];


/* Tree  parameters */
extern tree_t     type;            extern const ocrPtr_t __type;
extern double     b_0;             extern const ocrPtr_t __b_0;
extern int        rootId;          extern const ocrPtr_t __rootId;
extern int        nonLeafBF;       extern const ocrPtr_t __nonLeafBF;
extern double     nonLeafProb;     extern const ocrPtr_t __nonLeafProb;
extern int        gen_mx;          extern const ocrPtr_t __gen_mx;
extern geoshape_t shape_fn;        extern const ocrPtr_t __shape_fn;
extern double     shiftDepth;      extern const ocrPtr_t __shiftDepth;

/* Benchmark parameters */
extern int    computeGranularity;  extern const ocrPtr_t __computeGranularity;
extern int    verbose;             extern const ocrPtr_t __verbose;

/* Utility Functions */
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

/*
 * int uts_main(int argc, char **argv);
 */

#define WORDS_uts_main_args_t ((int)((sizeof(__continuation_args_t)+sizeof(int)+sizeof(size_t)+sizeof(uint64_t)-1)/sizeof(uint64_t)))
typedef union __uts_main_args_t {
  struct { __continuation_args_t std; int argc; size_t argv; } vars;
  uint64_t paramv[WORDS_uts_main_args_t] ;
} __uts_main_args_t;

#define WORDS_uts_main_guids_t ((int)((3*sizeof(ocrGuid_t)+sizeof(ocrGuid_t)-1)/sizeof(ocrGuid_t)))
typedef union __uts_main_guids_t {
  struct { ocrGuid_t    new_frame, old_frame, argv; } selector;
  struct { ocrEdtDep_t  new_frame, old_frame, argv; } segment;
  ocrGuid_t  guids[WORDS_uts_main_guids_t];
  ocrEdtDep_t depv[WORDS_uts_main_guids_t];
} __uts_main_guids_t;

ocrGuid_t uts_main( uint32_t __paramc, __uts_main_args_t  *__paramv,
                    uint32_t __depc,   __uts_main_guids_t *__depv );
ocrPtr_t  __template_uts_main = { .offset = (size_t)uts_main, .guid = RAG_NULL_GUID };

/*
 * void uts_parseParams(int argc, char **argv);
 */

#define WORDS_uts_parseParams_args_t ((int)((sizeof(__continuation_args_t)+sizeof(int)+sizeof(size_t)+sizeof(uint64_t)-1)/sizeof(uint64_t)))
typedef union __uts_parseParams_args_t {
  struct { __continuation_args_t std; int argc; size_t argv; } vars;
  uint64_t paramv[WORDS_uts_parseParams_args_t] ;
} __uts_parseParams_args_t;

#define WORDS_uts_parseParams_guids_t ((int)((3*sizeof(ocrGuid_t)+sizeof(ocrGuid_t)-1)/sizeof(ocrGuid_t)))
typedef union __uts_parseParams_guids_t {
  struct { ocrGuid_t    new_frame, old_frame, argv; } selector;
  struct { ocrEdtDep_t  new_frame, old_frame, argv; } segment;
  ocrGuid_t  guids[WORDS_uts_parseParams_guids_t];
  ocrEdtDep_t depv[WORDS_uts_parseParams_guids_t];
} __uts_parseParams_guids_t;

ocrGuid_t uts_parseParams( uint32_t __paramc, __uts_parseParams_args_t  *__paramv,
                    uint32_t __depc,   __uts_parseParams_guids_t *__depv );
ocrPtr_t  __template_uts_parseParams = { .offset = (size_t)uts_parseParams, .guid = RAG_NULL_GUID };

/*
 * void uts_printParams();
 */
#define WORDS_uts_printParams_args_t ((int)((sizeof(__continuation_args_t)+sizeof(uint64_t)-1)/sizeof(uint64_t)))
typedef union __uts_printParams_args_t {
  struct { __continuation_args_t std; } vars;
  uint64_t paramv[WORDS_uts_printParams_args_t] ;
} __uts_printParams_args_t;

#define WORDS_uts_printParams_guids_t ((int)((2*sizeof(ocrGuid_t)+sizeof(ocrGuid_t)-1)/sizeof(ocrGuid_t)))
typedef union __uts_printParams_guids_t {
  struct { ocrGuid_t    new_frame, old_frame; } selector;
  struct { ocrEdtDep_t  new_frame, old_frame; } segment;
  ocrGuid_t  guids[WORDS_uts_printParams_guids_t];
  ocrEdtDep_t depv[WORDS_uts_printParams_guids_t];
} __uts_printParams_guids_t;

ocrGuid_t uts_printParams( uint32_t __paramc, __uts_printParams_args_t  *__paramv,
                           uint32_t __depc,   __uts_printParams_guids_t *__depv );
ocrPtr_t  __template_uts_printParams = { .offset = (size_t)uts_printParams, .guid = RAG_NULL_GUID };

/*
 * void uts_initTreeRoot( Node *root, int type );
 */

#define WORDS_uts_initTreeRoot_args_t ((int)((sizeof(__continuation_args_t)+sizeof(size_t)+sizeof(int)+sizeof(uint64_t)-1)/sizeof(uint64_t)))
typedef union __uts_initTreRoot_args_t {
  struct { __continuation_args_t std; size_t root; int type; } vars;
  uint64_t paramv[WORDS_uts_initTreeRoot_args_t] ;
} __uts_initTreeRoot_args_t;

#define WORDS_uts_initTreeRoot_guids_t ((int)((3*sizeof(ocrGuid_t)+sizeof(ocrGuid_t)-1)/sizeof(ocrGuid_t)))
typedef union __uts_initTreRoot_guids_t {
  struct { ocrGuid_t    new_frame, old_frame, root; } selector;
  struct { ocrEdtDep_t  new_frame, old_frame, root; } segment;
  ocrGuid_t  guids[WORDS_uts_initTreeRoot_guids_t];
  ocrEdtDep_t depv[WORDS_uts_initTreeRoot_guids_t];
} __uts_initTreeRoot_guids_t;

ocrGuid_t uts_initTreeRoot( uint32_t paramc, __uts_initTreeRoot_args_t  *paramv,
                            uint32_t depc  , __uts_initTreeRoot_guids_t *depv );
ocrPtr_t  __template_uts_initTreeRoot = { .offset = (size_t)uts_initTreeRoot, .guid = RAG_NULL_GUID };

/*
 * void uts_TreeSearch( Node *parent );
 */

#define WORDS_uts_TreeSearch_args_t ((int)((sizeof(__continuation_args_t)+sizeof(size_t)+sizeof(uint64_t)-1)/sizeof(uint64_t)))
typedef union __uts_TreeSearch_args_t {
  struct { __continuation_args_t std; size_t  parent; } vars;
  uint64_t paramv[WORDS_uts_TreeSearch_args_t] ;
} __uts_TreeSearch_args_t;

#define WORDS_uts_TreeSearch_guids_t ((int)((3*sizeof(ocrGuid_t)+sizeof(ocrGuid_t)-1)/sizeof(ocrGuid_t)))
typedef union __uts_TreeSearch_guids_t {
  struct { ocrGuid_t    new_frame, old_frame, parent; } selector;
  struct { ocrEdtDep_t  new_frame, old_frame, parent; } segment;
  ocrGuid_t  guids[WORDS_uts_TreeSearch_guids_t];
  ocrEdtDep_t depv[WORDS_uts_TreeSearch_guids_t];
} __uts_TreeSearch_guids_t;

ocrGuid_t uts_TreeSearch( uint32_t paramc, __uts_TreeSearch_args_t  *__paramv,
                          uint32_t depc,   __uts_TreeSearch_guids_t *__depv );
ocrPtr_t  __template_uts_TreeSearch = { .offset = (size_t)uts_TreeSearch, .guid = RAG_NULL_GUID };

/* RAG -- TODO: make the rest of the uts_ functions edts */

static void uts_helpMessage(void);
#ifdef __cplusplus
}
#endif

#endif /* _UTS_H */
