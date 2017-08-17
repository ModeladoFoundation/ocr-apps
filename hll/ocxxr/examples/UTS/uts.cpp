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

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <cinttypes>

extern "C" {
// XXX - use C++ timer class instead
#include <sys/time.h>
}

#include "ocxxr-main.hpp"
#include "uts.hpp"

/***********************************************************
 *  tree generation and search parameters                  *
 *                                                         *
 *  Tree generation strategy is controlled via various     *
 *  parameters set from the command line.  The parameters  *
 *  and their default values are given below.              *
 ***********************************************************/

const char *const uts_trees_str[]     = { "Binomial", "Geometric", "Hybrid", "Balanced" };
const char *const uts_geoshapes_str[] = { "Linear decrease", "Exponential decrease", "Cyclic",
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
double b_0   = 4.0; // default branching factor at the root
int   rootId = 0;   // default seed for RNG state at root

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
double nonLeafProb = 15.0 / 64.0;  // q

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
geoshape_t shape_fn = LINEAR; // default shape function (b_i decr linearly)

/*  In type HYBRID trees, each node is either type BIN or type
 *  GEO, with the generation strategy changing from GEO to BIN
 *  at a fixed depth, expressed as a fraction of gen_mx
 */
double shiftDepth = 0.5;

/* compute granularity - number of rng evaluations per tree node */
int computeGranularity = 1;

/* display parameters */
int verbose  = 1;


/***********************************************************
 *                                                         *
 *  FUNCTIONS                                              *
 *                                                         *
 ***********************************************************/

void uts_error(const char *str) {
  printf("%s\n",str);
  exit(1);
}

/*
 * wall clock time
 *   for detailed accounting of work, this needs
 *   high resolution
 */
double uts_wctime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec + 1E-6 * tv.tv_usec);
}


// Interpret 32 bit positive integer as value on [0,1)
double rng_toProb(int n) {
  if (n < 0) {
    printf("*** toProb: rand n = %d out of range\n",n);
  }
  return ((n<0)? 0.0 : ((double) n)/2147483648.0);
}


void uts_initTreeRoot(Node * root, int type) {
  root->type = type;
  root->height = 0;
  root->numChildren = -1; /* means not yet determined */
#ifdef STATS
  root->maxDepth  = 0; /* my starting height */
  root->numNodes  = 1; /* me */
  root->numLeaves = 0; /* no leaf nodes yet */
#endif
  rng_init(root->state.state, rootId);
}


int uts_numChildren_bin(Node * parent) {
  // distribution is identical everywhere below root
  int    v = rng_rand(parent->state.state);
  double d = rng_toProb(v);

  return (d < nonLeafProb) ? nonLeafBF : 0;
}


int uts_numChildren_geo(Node * parent) {
  double b_i = b_0;
  int depth = parent->height;
  int numChildren, h;
  double p, u;

  // use shape function to compute target b_i
  if (depth > 0){
    switch (shape_fn) {

      // expected size polynomial in depth
    case EXPDEC:
      b_i = b_0 * pow((double) depth, -log(b_0)/log((double) gen_mx));
      break;

      // cyclic tree size
    case CYCLIC:
      if (depth > 5 * gen_mx){
        b_i = 0.0;
        break;
      }
      b_i = pow(b_0,
                sin(2.0*3.141592653589793*(double) depth / (double) gen_mx));
      break;

      // identical distribution at all nodes up to max depth
    case FIXED:
      b_i = (depth < gen_mx)? b_0 : 0;
      break;

      // linear decrease in b_i
    case LINEAR:
    default:
      b_i =  b_0 * (1.0 - (double)depth / (double) gen_mx);
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


int uts_numChildren(Node *parent) {
  int numChildren = 0;

  /* Determine the number of children */
  switch (type) {
    case BIN:
      if (parent->height == 0)
        numChildren = (int) floor(b_0);
      else
        numChildren = uts_numChildren_bin(parent);
      break;

    case GEO:
      numChildren = uts_numChildren_geo(parent);
      break;

    case HYBRID:
      if (parent->height < shiftDepth * gen_mx)
        numChildren = uts_numChildren_geo(parent);
      else
        numChildren = uts_numChildren_bin(parent);
      break;
    case BALANCED:
      if (parent->height < gen_mx)
        numChildren = (int) b_0;
      break;
    default:
      uts_error("parTreeSearch(): Unknown tree type");
  }

  // limit number of children
  // only a BIN root can have more than MAXNUMCHILDREN
  if (parent->height == 0 && parent->type == BIN) {
    int rootBF = (int) ceil(b_0);
    if (numChildren > rootBF) {
      printf("*** Number of children of root truncated from %d to %d\n",
             numChildren, rootBF);
      numChildren = rootBF;
    }
  }
  else if (type != BALANCED) {
    if (numChildren > MAXNUMCHILDREN) {
      printf("*** Number of children truncated from %d to %d\n",
             numChildren, MAXNUMCHILDREN);
      numChildren = MAXNUMCHILDREN;
    }
  }

  return numChildren;
}


int uts_childType(Node *parent) {
#if 0
/* RAG_DEBUG */
  printf("T %1d\n",type);
#endif
  switch (type) {
    case BIN:
      return BIN;
    case GEO:
      return GEO;
    case HYBRID:
      if (parent->height < shiftDepth * gen_mx)
        return GEO;
      else
        return BIN;
    case BALANCED:
      return BALANCED;
    default:
      uts_error("uts_get_childtype(): Unknown tree type");
      return -1;
  }
/* RAG */ return -1; /* To stop warning */
}


// construct string with all parameter settings
int uts_paramsToStr(char *strBuf, int ind) {
  // version + execution model
  ind += sprintf(strBuf+ind, "UTS - Unbalanced Tree Search %s (%s)\n", UTS_VERSION, "Sequential C");

  // tree type
  ind += sprintf(strBuf+ind, "Tree type:  %d (%s)\n", type, uts_trees_str[type]);

  // tree shape parameters
  ind += sprintf(strBuf+ind, "Tree shape parameters:\n");
  ind += sprintf(strBuf+ind, "  root branching factor b_0 = %.1f, root seed = %d\n", b_0, rootId);

  if (type == GEO || type == HYBRID) {
    ind += sprintf(strBuf+ind, "  GEO parameters: gen_mx = %d, shape function = %d (%s)\n",
            gen_mx, shape_fn, uts_geoshapes_str[shape_fn]);
  }

  if (type == BIN || type == HYBRID) {
    double q = nonLeafProb;
    int    m = nonLeafBF;
    double es  = (1.0 / (1.0 - q * m));
    ind += sprintf(strBuf+ind, "  BIN parameters:  q = %f, m = %d, E(n) = %f, E(s) = %.2f\n",
            q, m, q * m, es);
  }

  if (type == HYBRID) {
    ind += sprintf(strBuf+ind, "  HYBRID:  GEO from root to depth %d, then BIN\n",
            (int) ceil(shiftDepth * gen_mx));
  }

  if (type == BALANCED) {
    ind += sprintf(strBuf+ind, "  BALANCED parameters: gen_mx = %d\n", gen_mx);
    ind += sprintf(strBuf+ind, "        Expected size: %" PRIu64 " nodes, %" PRIu64" leaves\n",
        (counter_t) ((pow(b_0, gen_mx+1) - 1.0)/(b_0 - 1.0)) /* geometric series */,
        (counter_t) pow(b_0, gen_mx));
  }

  // random number generator
  ind += sprintf(strBuf+ind, "Random number generator: ");
  ind  = rng_showtype(strBuf, ind);
  ind += sprintf(strBuf+ind, "\nCompute granularity: %d\n", computeGranularity);

  return ind;
}

// show parameter settings
void uts_printParams() {
  char strBuf[5000] = "";
  int  ind = 0;

  if (verbose > 0) {
    ind = uts_paramsToStr(strBuf, ind);
    ind += sprintf(strBuf+ind, "Execution strategy:  Sequential Recursive Search\n");
    printf("%s\n",strBuf);
  }
}

void uts_parseParams(int argc, char *argv[]){
  int i = 1;
  int err = -1;
  while (i < argc && err == -1) {
    if (argv[i][0] == '-' && argv[i][1] == 'h') {
        uts_helpMessage();
        exit(0);

    } else if (argv[i][0] != '-' || strlen(argv[i]) != 2 || argc <= i+1) {
      err = i; break;
    }

    switch (argv[i][1]) {
      case 'q':
        nonLeafProb = atof(argv[i+1]); break;
      case 'm':
        nonLeafBF = atoi(argv[i+1]); break;
      case 'r':
        rootId = atoi(argv[i+1]); break;
      case 'v':
        verbose = atoi(argv[i+1]); break;
      case 't':
        type = (tree_t) atoi(argv[i+1]);
        if (type != BIN && type != GEO && type!= HYBRID && type != BALANCED)
	  err = i;
        break;
      case 'a':
        shape_fn = (geoshape_t) atoi(argv[i+1]);
        if (shape_fn > FIXED) err = i;
        break;
      case 'b':
        b_0 = atof(argv[i+1]); break;
      case 'd':
        gen_mx = atoi(argv[i+1]); break;
      case 'f':
        shiftDepth = atof(argv[i+1]); break;
      case 'g':
        computeGranularity = max(1,atoi(argv[i+1])); break;
      default:
        err = i;
    }

    if (err != -1) break;

    i += 2;
  }

  if (err != -1) {
    printf("Unrecognized parameter or incorrect/missing value: '%s %s'\n", argv[i], (i+1 < argc) ? argv[i+1] : "[none]");
    printf("Try -h for help.\n");
    exit(4);
  }
}

void uts_helpMessage() {
  printf("  UTS - Unbalanced Tree Search %s (%s)\n\n", UTS_VERSION, "Sequential C");
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

/* uts_TreeSearch(Node *parent) */

void uts_TreeSearch(Node * parent) {
  const int childHeight = parent->height + 1;
  const int numChildren = uts_numChildren(parent);

  // record number of children in parent

  parent->numChildren = numChildren;

#ifdef STATS
  parent->numNodes   += numChildren;
#endif

  // recursivily construct and visit the children
  if (numChildren > 0) {
    const int childType = uts_childType(parent);

    for (int i = 0; i < numChildren; i++) {
        Node child;
        child.type      = childType;
        child.height    = childHeight;
        child.numChildren = -1;    // not yet determined
#ifdef STATS
        child.maxDepth  = childHeight;
        child.numNodes  = 0;
        child.numLeaves = 0;
#endif

        for (int j = 0; j < computeGranularity; j++) {
        // TBD:  add parent height to spawn
        // computeGranularity controls number of rng_spawn calls per node
            rng_spawn(parent->state.state, child.state.state, i);
        } /* end for j */

        uts_TreeSearch(&child);
#ifdef STATS
        parent->numNodes  += child.numNodes;
        parent->numLeaves += child.numLeaves;
        if(parent->maxDepth < child.maxDepth) parent->maxDepth = child.maxDepth;
#endif

    } /* end for i */

  } else {
#ifdef STATS
      parent->numLeaves++;
#endif
  } /* end if */
} /* end seqTreeSearch(Node *parent) */

/*----------------------------------------------------------------------------*/

ocxxr::Arena<void> NewBlock() {
    //PRINTF("NEW BLOCK\n");
    return ocxxr::Arena<void>::Create(sizeof(Node) * (1<<10));
}

/*----------------------------------------------------------------------------*/

void InitChildren(Node *parent, Node *child) {
    const int num_children = parent->numChildren;
    if (num_children > 0) {
        assert(child);
        const int child_height = parent->height + 1;
        const int child_type = uts_childType(parent);
        for (int i=0; i<num_children; i++, child++) {
            child->type      = child_type;
            child->height    = child_height;
            child->numChildren = -1;    // not yet determined
            child->children = nullptr;
#ifdef STATS
            child->maxDepth  = child_height;
            child->numNodes  = 0;
            child->numLeaves = 0;
#endif

            for (int j = 0; j < computeGranularity; j++) {
                // TBD:  add parent height to spawn
                // computeGranularity controls number of rng_spawn calls per node
                rng_spawn(parent->state.state, child->state.state, i);
            } /* end for j */

            child->numChildren = uts_numChildren(child);
            assert(child->numChildren > -1);
        }
    }
}

struct BuildTreeParams;

void BuildTreeTask(BuildTreeParams &params, ocxxr::Arena<void> block);

struct BuildTreeParams {
    ocxxr::BasedPtr<Node> current;
    ocxxr::BasedPtr<Node> last;
    ocxxr::TaskTemplate<decltype(BuildTreeTask)> task_template;
};

void BuildTreeTask(BuildTreeParams &params, ocxxr::Arena<void> block) {
    Node *current = params.current;
    Node *last = params.last;
    while (!last || current < last) {
        const int num_children = current->numChildren;
        assert(num_children >= 0);
        assert(!current->children);
        if (num_children > 0) {
            Node *children = block.NewArray<Node>(num_children);
            if (!children) {
                // Ran out of space
                ocxxr::Arena<void> child_block = NewBlock();
                children = child_block.NewArray<Node>(num_children);
                assert(children);
            } else {
                last = children + num_children;
            }
            InitChildren(current, children);
            current->children = children;
            if (!current->children.target_is_local()) {
                // Need to make continuation tasks
                // for current block
                params.current = current+1;
                params.last = last;
                if (params.current < params.last) {
                    params.task_template().CreateTask(params, block);
                }
                // for new block
                params.current = children;
                params.last = children + num_children;
                auto next = params.current.target_handle();
                params.task_template().CreateTask(params, next);
                // Done
                return;
            }
        }
        // breadth-first advance to next node
        ++current;
    }
}

void BuildRootTask(BuildTreeParams &params, ocxxr::NullHandle) {
    // initialize Tree Root
    ocxxr::Arena<void> root_block = NewBlock();
    Node *root = root_block.New<Node>();
    uts_initTreeRoot(root, type);
    root->children = nullptr;
    root->numChildren = uts_numChildren(root);

    // build the rest
    params.current = root;
    params.last = root + 1;
    params.task_template().CreateTask(params, root_block);
}

/*----------------------------------------------------------------------------*/

void ShutdownTask(ocxxr::Datablock<void>) {
    PRINTF("Done\n");
    ocxxr::Shutdown();
}

/*----------------------------------------------------------------------------*/

void ocxxr::Main(ocxxr::Datablock<ocxxr::MainTaskArgs> args) {
    // Unpack argc/argv
    int argc = args->argc();
    char *argv[argc+1];
    for (int i=0; i<argc; i++) {
        argv[i] = args->argv(i);
    }
    argv[argc] = nullptr;


    /* determine benchmark parameters (all PEs) */
    uts_parseParams(argc, argv);

    /* show parameter settings */
    uts_printParams();

    BuildTreeParams params;
    params.task_template = OCXXR_TEMPLATE_FOR(BuildTreeTask);
    // XXX - need to destroy this template later

    auto root_template = OCXXR_TEMPLATE_FOR(BuildRootTask);
    auto build_future = root_template(EDT_PROP_FINISH).CreateFuturePartial(params);

    auto build_done = build_future.event();

    // set up shutdown task
    auto shutdown_template = OCXXR_TEMPLATE_FOR(ShutdownTask);
    shutdown_template().CreateTask(build_done);
    shutdown_template.Destroy();

    // start building task
    build_future.task().DependOn<0>(NullHandle());
    root_template.Destroy();

#if 0
    /* time search */
    t1 = uts_wctime();
    uts_TreeSearch(&root);
    t2 = uts_wctime();
    et = t2 - t1;

#ifdef STATS
    printf("UTS Tree size = %" PRIu64 ", ",root.numNodes);
    printf("tree depth  = %d, ",           root.maxDepth);
    printf("num leaves = %" PRIu64 ", ",   root.numLeaves);
    printf("(%5.2f%%) time = %f s\n",((double)root.numLeaves/(double)root.numNodes)*100.0,et);
#endif
#endif

}
