#ifdef FSIM
#define RMDGLOB RMDGLOBAL
#else
#define RMDGLOB
#endif

#define SUCCESS 0

//#define rckpause (1);
#define rckpause usleep(30);

//#define rckprintf(a) printf(a)
//#define rckprintf(a) fprintf(stderr,a)
#define rckprintf(a)

//#define rckprintf1(a) printf(a)
//#define rckprintf1(a) fprintf(stderr,a)
#define rckprintf1(a)

/* -*- mode: C; mode: folding; fill-column: 70; -*- */
/* Copyright 2010,  Georgia Institute of Technology, USA. */
/* See COPYING for license. */

#include "compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <alloca.h> /* Portable enough... */
#include <fcntl.h>
/* getopt should be in unistd.h */
#include <unistd.h>

#ifdef CODELETS

#ifdef FSIM
#include "xe-codelet.h"  //note hyphen, not underscore :(
#include "xe_memory.h"
#include "xe_console.h"
#include "xe_global.h"
#else
#include "codelet.h"
#include "rmd_afl_all.h"
#endif


rmd_guid_t init_type;
rmd_guid_t generate_kron_type;
rmd_guid_t find_nv_type;
rmd_guid_t create_graph_type;
rmd_guid_t gather_edges_type;
rmd_guid_t bfs_type;
rmd_guid_t bfs_level_sync_type;
rmd_guid_t bfs_mark_nodes_type;
rmd_guid_t per_root_type;
rmd_guid_t verify_type;
rmd_guid_t end_type;
rmd_guid_t IJ_DBlock;
rmd_guid_t bfs_DBlock;
rmd_guid_t vlist_DBlock;
rmd_guid_t xoff_DBlock;
#endif

#include "graph500.h"
#include "rmat.h"
#include "kronecker.h"
#include "verify.h"
#include "prng.h"
#include "timer.h"
#include "xalloc.h"
#include "options.h"
#include "generator/splittable_mrg.h"
#include "generator/graph_generator.h"
#include "generator/make_graph.h"
#include "generator/utils.h"  // needed for LLVM implicit-decl error

typedef struct argk1k2{
	
	int64_t k1;
	int64_t k2;
}argk1k2;

// doesn't need to be static, except for conflict with same-name/same-val
// variable in make-edgelist
int64_t nvtx_scale;

int64_t bfs_root[NBFS_max];

RMDGLOB static double generation_time; //RCKStatic
RMDGLOB static double construction_time;  //RCKStatic
RMDGLOB static double bfs_time[NBFS_max];  //RCKStatic
int64_t bfs_nedge[NBFS_max];

packed_edge * restrict IJ;
RMDGLOB static int64_t nedge;  //RCKStatic

int64_t verify_bfs_tree_c (int64_t *bfs_tree_in, int64_t max_bfsvtx, int64_t root, const int64_t *IJ_in, int64_t nedge);
//RMDGLOB static void run_bfs (void);

#ifdef CODELETS
#define MINVECT_SIZE 2

int64_t sznonstatic; // no longer static
int64_t maxvtx;
int64_t nv;
int64_t * restrict xoff; /* Length 2*nv+2 */
int64_t * restrict xadjstore; /* Length MINVECT_SIZE + (xoff[nv] == nedge) */
int64_t * restrict xadj;

int64_t
_fetch_add (int64_t* p, int64_t incr) {
  return __sync_fetch_and_add (p, incr);
}
int64_t
_casval(int64_t* p, int64_t oldval, int64_t newval) {
  return __sync_val_compare_and_swap (p, oldval, newval);
}
int
_cas(int64_t* p, int64_t oldval, int64_t newval) {
  return __sync_bool_compare_and_swap (p, oldval, newval);
}

/* Generate a range of edges (from start_edge to end_edge of the total graph),
 * writing into elements [0, end_edge - start_edge) of the edges array.  This
 * code is parallel on OpenMP and XMT; it must be used with
 * separately-implemented SPMD parallelism for MPI. */

rmd_guid_t generate_kron_codelet(uint64_t arg, int n_db, void *db_ptr[],rmd_guid_t *db) {
  rmd_guid_t ret;
  rckpause;

  rckprintf1("\n--- generate_kron_codelet\n");

  int64_t ei = arg;
  //printf("ei=%d\n",arg);
  packed_edge *edges=(packed_edge *)(*(uint64_t*)db_ptr[1]);
  //printf("edges=%p\n",edges);
  mrg_state state = *(mrg_state*)(db_ptr[2]); 
  int64_t start_edge = *(int64_t*)(db_ptr[3]);
  //printf("start_edge=%d\n",start_edge);
  int logN = *(int*)(db_ptr[4]);
  // printf("logN=%d\n",logN);
  uint64_t val0 = *(uint64_t*)(db_ptr[5]);
  // printf("val0=%d\n",val0);
  uint64_t val1 = *(uint64_t*)(db_ptr[6]);
  //printf("val1=%d\n",val1);
  int64_t *counter=(int64_t*)db_ptr[0];     
  
  int64_t nverts = (int64_t)1 << logN;
  mrg_state new_state = state;
  mrg_skip(&new_state, 0, ei, 0);
  make_one_edge(nverts, 0, logN, &new_state, edges + (ei - start_edge), val0, val1);
  
  if(__sync_add_and_fetch(counter,-1)==0){
    RMD_DB_FREE(db[0]);
    RMD_DB_FREE(db[1]);
    RMD_DB_FREE(db[2]);
    RMD_DB_FREE(db[3]);
    RMD_DB_FREE(db[4]);
    RMD_DB_FREE(db[5]);
    RMD_DB_FREE(db[6]);
    if (VERBOSE) fprintf (stderr, " done.\n");
#ifdef PRINT_DEBUG_INFO
    printf("Done generate edges\n");
#endif
    rmd_guid_t find_nv_s;
    rmd_codelet_sched(&find_nv_s,0,find_nv_type);
    
    ret.data = SUCCESS;
    return ret;
    
  }
  ret.data = SUCCESS;
  rckprintf1("\n--- generate_kron_codelet end\n");
  return ret;
}

void generate_kronecker_range_c(
       const uint_fast32_t seed[5] /* All values in [0, 2^31 - 1), not all zero */,
       int logN /* In base 2 */,
       int64_t start_edge, int64_t end_edge,
       packed_edge* edges) {
  mrg_state state;
  int64_t nverts = (int64_t)1 << logN;
  int64_t ei;

  mrg_seed(&state, seed);

  uint64_t val0, val1; /* Values for scrambling */
  {
    mrg_state new_state = state;
    mrg_skip(&new_state, 50, 7, 0);
    val0 = mrg_get_uint_orig(&new_state);
    val0 *= UINT64_C(0xFFFFFFFF);
    val0 += mrg_get_uint_orig(&new_state);
    val1 = mrg_get_uint_orig(&new_state);
    val1 *= UINT64_C(0xFFFFFFFF);
    val1 += mrg_get_uint_orig(&new_state);
  }

  //#pragma omp parallel for
  //printf("se=%d-ee=%d\n",start_edge,end_edge);
  rmd_guid_t counter_arg; 
  rmd_guid_t edges_arg; 
  rmd_guid_t state_arg; 
  rmd_guid_t start_edge_arg; 
  rmd_guid_t logN_arg; 
  rmd_guid_t val0_arg; 
  rmd_guid_t val1_arg; 
  rmd_guid_t generate_iter; 
  rmd_location_t iter;
  iter.type = RMD_LOC_TYPE_RELATIVE;
  iter.data.relative.level = RMD_LOCATION_BLOCK;//data dependencies must be at the block level.
  int64_t* counter_addr = RMD_DB_ALLOC (&counter_arg, sizeof (int64_t) ,0,&iter);
  *counter_addr = end_edge - start_edge; 
  uint64_t* edges_addr = RMD_DB_ALLOC (&edges_arg, sizeof (uint64_t) ,0,&iter);
  *edges_addr = (uint64_t)edges;
  mrg_state* state_addr = RMD_DB_ALLOC (&state_arg, sizeof (mrg_state) ,0,&iter);
  *state_addr = state;
  int64_t* start_edge_addr = RMD_DB_ALLOC (&start_edge_arg, sizeof (int64_t) ,0,&iter);
  *start_edge_addr = start_edge;
  int* logN_addr = RMD_DB_ALLOC (&logN_arg, sizeof (int) ,0,&iter);
  *logN_addr = logN;
  uint64_t* val0_addr = RMD_DB_ALLOC (&val0_arg, sizeof (uint64_t) ,0,&iter);
  *val0_addr = val0;
  uint64_t* val1_addr = RMD_DB_ALLOC (&val1_arg, sizeof (uint64_t) ,0,&iter);
  *val1_addr = val1;
  //printf("args:%d\n",*counter_addr); 
  RMD_DB_RELEASE(counter_arg);
  RMD_DB_RELEASE(edges_arg);
  RMD_DB_RELEASE(state_arg);
  RMD_DB_RELEASE(start_edge_arg);
  RMD_DB_RELEASE(logN_arg);
  RMD_DB_RELEASE(val0_arg);
  RMD_DB_RELEASE(val1_arg);
  //printf("args6 %d %p %d %d %ld %ld\n",*counter_addr,*edges_addr,*start_edge_addr,*logN_addr,*val0_addr,*val1_addr);
  for (ei = start_edge; ei < end_edge; ++ei) {
    //generate_kron_codelet(ei,edges,state,start_edge,logN,val0,val1);
    rmd_codelet_sched(&generate_iter,ei,generate_kron_type);
    rmd_codelet_satisfy(generate_iter,counter_arg,0);
    rmd_codelet_satisfy(generate_iter,edges_arg,1);
    rmd_codelet_satisfy(generate_iter,state_arg,2);
    rmd_codelet_satisfy(generate_iter,start_edge_arg,3);
    rmd_codelet_satisfy(generate_iter,logN_arg,4);
    rmd_codelet_satisfy(generate_iter,val0_arg,5);
    rmd_codelet_satisfy(generate_iter,val1_arg,6);
  }
}

void make_graph_c(int log_numverts, int64_t M, uint64_t userseed1, uint64_t userseed2, int64_t* nedges_ptr_in, packed_edge** result_ptr_in) {
  /* Add restrict to input pointers. */
  int64_t* restrict nedges_ptr = nedges_ptr_in;
  packed_edge* restrict* restrict result_ptr = result_ptr_in;
  //printf("make_graph_c\n");
  /* Spread the two 64-bit numbers into five nonzero values in the correct
   * range. */
  uint_fast32_t seed[5];
  make_mrg_seed(userseed1, userseed2, seed);

  *nedges_ptr = M;
#ifdef RMD_DB_MEM
  rmd_location_t loc;
  loc.type = RMD_LOC_TYPE_RELATIVE;
  loc.data.relative.level = RMD_LOCATION_DRAM;
  packed_edge* edges = (packed_edge*)RMD_DB_ALLOC(&IJ_DBlock, M* sizeof(packed_edge),0,&loc);
#else
  packed_edge* edges = (packed_edge*)xmalloc(M * sizeof(packed_edge));
#endif
  *result_ptr = edges;

  /* In OpenMP and XMT versions, the inner loop in generate_kronecker_range is
   * parallel.  */
  generate_kronecker_range_c(seed, log_numverts, 0, M, edges);
}


static int alloc_graph_c (int64_t nedge) {
  sznonstatic = (2*nv+2) * sizeof (*xoff);
#ifdef RMD_DB_MEM
  rmd_location_t loc;
  loc.type = RMD_LOC_TYPE_RELATIVE;
  loc.data.relative.level = RMD_LOCATION_DRAM;
  xoff = RMD_DB_ALLOC(&xoff_DBlock,sznonstatic,0,&loc);
#else
  xoff = xmalloc_large_ext (sznonstatic);
#endif
#ifdef MEM_INFO
  printf("xoff Memory=%ld\n",sznonstatic);
#endif
  if (!xoff) return -1;
  return 0;
}

static void free_graph_c(void) {
  xfree_large (xadjstore);
#ifdef RMD_DB_MEM
  RMD_DB_FREE(xoff_DBlock);
#else
  xfree_large (xoff);
#endif
}

#define XOFF(k) (xoff[2*(k)])
#define XENDOFF(k) (xoff[1+2*(k)])

static int64_t prefix_sum_c (int64_t *buf) {
  int nt, tid;
  int64_t slice_begin, slice_end, t1, t2, k;
  
  nt = 1;//omp_get_num_threads ();
  tid = 0;//omp_get_thread_num ();
  
  t1 = nv / nt;
  t2 = nv % nt;
  slice_begin = t1 * tid + (tid < t2? tid : t2);
  slice_end = t1 * (tid+1) + ((tid+1) < t2? (tid+1) : t2);
  
  buf[tid] = 0;
  for (k = slice_begin; k < slice_end; ++k)
    buf[tid] += XOFF(k);
  //OMP("omp barrier");
  //OMP("omp single")
  for (k = 1; k < nt; ++k)
    buf[k] += buf[k-1];
  if (tid)
    t1 = buf[tid-1];
  else
    t1 = 0;
  for (k = slice_begin; k < slice_end; ++k) {
    int64_t tmp = XOFF(k);
    XOFF(k) = t1;
    t1 += tmp;
  }
  //OMP("omp flush (xoff)");
  //OMP("omp barrier");
  return buf[nt-1];
}

static int setup_deg_off_c (const struct packed_edge * restrict IJ,
			    int64_t nedge) {

  int err = 0;
  int64_t *buf = NULL;
  xadj = NULL;
  //OMP("omp parallel")
  int64_t k, accum;
  //OMP("omp for")
  for (k = 0; k < 2*nv+2; ++k)
    xoff[k] = 0;
  //OMP("omp for")
  for (k = 0; k < nedge; ++k) {
    int64_t i = get_v0_from_edge(&IJ[k]);
    int64_t j = get_v1_from_edge(&IJ[k]);
    if (i != j) { /* Skip self-edges. */
      if (i >= 0)
	//OMP("omp atomic")
	++XOFF(i);
      if (j >= 0)
	//OMP("omp atomic")
	++XOFF(j);
    }
  }
  //OMP("omp single")
  //{
#ifdef RMD_DB_MEM
  rmd_guid_t buf_tag;
  rmd_location_t iter;
  iter.type = RMD_LOC_TYPE_RELATIVE;
  iter.data.relative.level = RMD_LOCATION_BLOCK;//.
  buf = RMD_DB_ALLOC (&buf_tag,/*omp_get_num_threads ()*/1 * sizeof (*buf),0,&iter);
#else
  buf = alloca (/*omp_get_num_threads ()*/1 * sizeof (*buf));
#endif
  if (!buf) {
    perror ("alloca for prefix-sum hosed");
    abort ();
  }
  //OMP("omp for")
  for (k = 0; k < nv; ++k)
    if (XOFF(k) < MINVECT_SIZE) XOFF(k) = MINVECT_SIZE;

  accum = prefix_sum_c (buf);
#ifdef RMD_DB_MEM
  RMD_DB_FREE(buf_tag);
#endif
  //printf("accum%d\n",accum);
  //OMP("omp for")
  for (k = 0; k < nv; ++k)
    XENDOFF(k) = XOFF(k);
  //OMP("omp single")
  {
    XOFF(nv) = accum;
    if (!(xadjstore = xmalloc_large_ext ((XOFF(nv) + MINVECT_SIZE) * sizeof (*xadjstore))))
      err = -1;
    if (!err) {
      xadj = &xadjstore[MINVECT_SIZE]; /* Cheat and permit xadj[-1] to work. */
      for (k = 0; k < XOFF(nv) + MINVECT_SIZE; ++k)
	xadjstore[k] = -1;
    }
  }
  return !xadj;
}

static void scatter_edge_c (const int64_t i, const int64_t j) {
  int64_t where;
  where = _fetch_add (&XENDOFF(i), 1);
  xadj[where] = j;
}

static int i64cmp_c (const void *a, const void *b) {
  const int64_t ia = *(const int64_t*)a;
  const int64_t ib = *(const int64_t*)b;
  if (ia < ib) return -1;
  if (ia > ib) return 1;
  return 0;
}

static void pack_vtx_edges_c(const int64_t i) {
  int64_t kcur, k;
  if (XOFF(i)+1 >= XENDOFF(i)) return;
  qsort (&xadj[XOFF(i)], XENDOFF(i)-XOFF(i), sizeof(*xadj), i64cmp_c);
  kcur = XOFF(i);
  for (k = XOFF(i)+1; k < XENDOFF(i); ++k)
    if (xadj[k] != xadj[kcur])
      xadj[++kcur] = xadj[k];
  ++kcur;
  for (k = kcur; k < XENDOFF(i); ++k)
    xadj[k] = -1;
  XENDOFF(i) = kcur;
}

static void pack_edges_c (void) {
  int64_t v;
	
  //OMP("omp for")
  for (v = 0; v < nv; ++v)
    pack_vtx_edges_c (v);
}

rmd_guid_t gather_edges_codelet (uint64_t arg, int n_db, void *db_ptr[],
			   rmd_guid_t *db) {
  rckpause;
  rckprintf1("\n--- gather_edges_codelet\n");
#ifdef PRINT_DEBUG_INFO
  //printf("Gather edges Start\n");
#endif
  //OMP("omp parallel")
  rmd_guid_t ret;
  const struct packed_edge * restrict IJ=(const struct packed_edge *restrict)(*(uint64_t*)(db_ptr[0]));
  int64_t k=arg;
  int64_t *counter=(int64_t*)db_ptr[1];     
  //printf("args got:%u_%d\n",IJ,*counter); 
  //OMP("omp for")
  //for (k = 0; k < nedge; ++k)
  int64_t i = get_v0_from_edge(&IJ[k]);
  int64_t j = get_v1_from_edge(&IJ[k]);
  if (i >= 0 && j >= 0 && i != j) {
    scatter_edge_c (i, j);
    scatter_edge_c (j, i);
  }
  if(__sync_add_and_fetch(counter,-1)==0) {
#ifdef PRINT_DEBUG_INFO
    printf("Packed Edges\n"); 
#endif
    pack_edges_c ();
    RMD_DB_FREE(db[0]);
    RMD_DB_FREE(db[1]);
    rmd_guid_t bfs_s;
    rmd_codelet_sched(&bfs_s,0,bfs_type);
  }
  ret.data= EXIT_SUCCESS;
  rckprintf1("\n--- gather_edges_codelet end\n");
  return ret;
}

int create_graph_from_edgelist_c (struct packed_edge *IJ, int64_t nedge) {
  //OMP("omp parallel")
  //	 printf("Running OMP on %d threads\n", omp_get_num_threads());
  int64_t k;
  if (alloc_graph_c (nedge)) return -1;
  if (setup_deg_off_c (IJ, nedge)) {
    xfree_large (xoff);
    return -1;
  }
  rmd_guid_t counter_arg; 
  rmd_guid_t IJ_arg; 
  rmd_guid_t gather_iter; 
  rmd_location_t iter;
  iter.type = RMD_LOC_TYPE_RELATIVE;
  iter.data.relative.level = RMD_LOCATION_BLOCK;//Data dependecies go on the block level cache.
  int64_t* counter_addr = RMD_DB_ALLOC (&counter_arg, sizeof (int64_t) ,0,&iter);
  *counter_addr = nedge; 
  uint64_t* IJ_addr = RMD_DB_ALLOC (&IJ_arg, sizeof (uint64_t) ,0,&iter);
  *IJ_addr = (uint64_t)IJ;
  //printf("args:%u_%d\n",*IJ_addr,*counter_addr); 
  RMD_DB_RELEASE(counter_arg);
  RMD_DB_RELEASE(IJ_arg);
  for (k = 0; k < nedge; ++k){
    rmd_codelet_sched(&gather_iter,k, gather_edges_type);
    rmd_codelet_satisfy(gather_iter,IJ_arg,0);
    rmd_codelet_satisfy(gather_iter,counter_arg,1);
    //gather_edges_c (IJ, nedge); 
  }
  return 0;
}

int make_bfs_tree_c (int64_t *bfs_tree_out, int64_t srcvtx,int m) {
#ifdef PRINT_DEBUG_INFO
  //printf("Make_bfs_tree\n");
#endif
  int64_t * restrict bfs_tree = bfs_tree_out;
  rmd_guid_t ret;
  
  int64_t * restrict vlist = NULL;

#ifdef RMD_DB_MEM
  rmd_location_t loc4;
  loc4.type = RMD_LOC_TYPE_RELATIVE;
  loc4.data.relative.level = RMD_LOCATION_BLOCK;
  vlist = RMD_DB_ALLOC (&vlist_DBlock, nv * sizeof (*vlist) ,0,&loc4);
#else
  vlist = xmalloc_large (nv * sizeof (*vlist));
#endif
#ifdef MEM_INFO
  printf("Vlist Memory =%ld\n",nv*sizeof (*vlist)); 
#endif
	
  if (!vlist) return -1;
	
  vlist[0] = srcvtx;
	
  bfs_tree[srcvtx] = srcvtx;
	
#define THREAD_BUF_LEN 16384
  //OMP("omp parallel shared(k1, k2)")
  int64_t k;
  //OMP("omp for")
  for (k = 0; k < srcvtx; ++k)
    bfs_tree[k] = -1;
  //OMP("omp for")
  for (k = srcvtx+1; k < nv; ++k)
    bfs_tree[k] = -1;
  
  rmd_guid_t bfs_tree_arg;
  rmd_guid_t vlist_arg;
  rmd_guid_t k_arg;
  rmd_location_t iter;
  iter.type = RMD_LOC_TYPE_RELATIVE;
  iter.data.relative.level = RMD_LOCATION_BLOCK;
  uint64_t* bfs_tree_addr = RMD_DB_ALLOC (&bfs_tree_arg, sizeof (uint64_t) ,0,&iter);
  uint64_t* vlist_addr = RMD_DB_ALLOC (&vlist_arg, sizeof (uint64_t) ,0,&iter);
  argk1k2* k_addr = RMD_DB_ALLOC (&k_arg, sizeof (argk1k2) ,0,&iter);
  RMD_DB_RELEASE(bfs_tree_arg);
  RMD_DB_RELEASE(vlist_arg);
  RMD_DB_RELEASE(k_arg);
  *bfs_tree_addr=(uint64_t)bfs_tree;
  *vlist_addr=(uint64_t)vlist;
  k_addr->k1 = 0;
  k_addr->k2 = 1;
  rmd_guid_t next_iter;
  rmd_codelet_sched(&next_iter,m,bfs_level_sync_type);
  rmd_codelet_satisfy(next_iter,vlist_arg,0);
  rmd_codelet_satisfy(next_iter,bfs_tree_arg,1);
  rmd_codelet_satisfy(next_iter,k_arg,2);
  ret.data= EXIT_SUCCESS;
  return 0;
}

rmd_guid_t bfs_level_sync_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db) {
  rckpause;

  //arg has the iteration number
  rmd_guid_t ret;
  //#define THREAD_BUF_LEN 16384
  //int64_t nbuf[THREAD_BUF_LEN];
  int64_t k;
  int64_t k1, k2;
  k1 = ((argk1k2 *)(db_ptr[2]))->k1;
  k2 = ((argk1k2 *)(db_ptr[2]))->k2;
  //k1=oldk2 
  ((argk1k2 *)(db_ptr[2]))->k1=((argk1k2 *)(db_ptr[2]))->k2;
  //int64_t * restrict bfs_tree = (int64_t*)(*(uint64_t*)db_ptr[1]);
  int64_t * restrict vlist = (int64_t*)(*(uint64_t*)db_ptr[0]);
  
  if (k1 == k2){

#ifdef RMD_DB_MEM
    RMD_DB_FREE(vlist_DBlock);
#else
    xfree_large (vlist);
#endif
		
#ifdef PRINT_DEBUG_INFO
		//printf("bfs_level_Done\n");
#endif
    RMD_DB_FREE(db[0]);
    RMD_DB_FREE(db[2]);
    rmd_guid_t next_iter;
    rmd_codelet_sched(&next_iter,arg,verify_type);
    rmd_codelet_satisfy(next_iter,db[1],0);
    ret.data= EXIT_SUCCESS;
    return ret;
  }

  rmd_guid_t bfs_kern;
  
  rmd_guid_t counter_arg;
  rmd_guid_t previous_ctr_arg;
  rmd_location_t iter;
  iter.type = RMD_LOC_TYPE_RELATIVE;
  iter.data.relative.level = RMD_LOCATION_BLOCK;
  int* counter_addr = RMD_DB_ALLOC (&counter_arg, sizeof (int) ,0,&iter);
  int* prev_ctr_addr = RMD_DB_ALLOC (&previous_ctr_arg, sizeof (int) ,0,&iter);
  *counter_addr=k2-k1;
#ifdef PRINT_DEBUG_INFO
  //printf("k1=%d,k2=%d,c=%d ",k1,k2,*counter_addr);
#endif
  *prev_ctr_addr=arg;
  //rmd_guid_t finish_kern;
  //rmd_codelet_sched(&finish_kern,k,finish_kernel_type);
#ifdef SERIAL_BFS	
  {
    rmd_codelet_sched(&bfs_kern,k1,bfs_mark_nodes_type);
#else
    for (k = k1; k < k2; ++k)
      {
	rmd_codelet_sched(&bfs_kern,k,bfs_mark_nodes_type);
#endif
	rmd_codelet_satisfy(bfs_kern,db[0],0);
	rmd_codelet_satisfy(bfs_kern,db[1],1);
	rmd_codelet_satisfy(bfs_kern,db[2],2);
	rmd_codelet_satisfy(bfs_kern,counter_arg,3);
	rmd_codelet_satisfy(bfs_kern,previous_ctr_arg,4);
      }
    ret.data= EXIT_SUCCESS;
    return ret;
}
 
rmd_guid_t bfs_mark_nodes_codelet(uint64_t arg, int n_db, void *db_ptr[],
				  rmd_guid_t *db)  {
  rckpause;

  rckprintf1("\n--- bfs_mark_nodes_codelet\n");
  rmd_guid_t ret;
#ifdef SERIAL_BFS
#define THREAD_BUF 16384
#else
  #define THREAD_BUF 128
#endif
//#define THREAD_BUF_LEN 16384
/*#ifdef RMD_DB_MEM
  rmd_guid_t nbuf_arg
  rmd_location_t iter;
  iter.type = RMD_LOC_TYPE_RELATIVE;
  iter.data.relative.level = RMD_LOCATION_CORE;
  int* nbuf = RMD_DB_ALLOC (&nbuf_arg, THREAD_BUF,0,&iter);
#else*/
  int64_t nbuf[THREAD_BUF];
//#endif
  int64_t k=arg;
  int64_t *k2;
 // k1 = ((argk1k2 *)(db_ptr[2]))->k1;
  k2 = &((argk1k2 *)(db_ptr[2]))->k2;

  int64_t * restrict bfs_tree = (int64_t*)(*(uint64_t*)db_ptr[1]);
  int64_t * restrict vlist = (int64_t*)(*(uint64_t*)db_ptr[0]);
     
  // const int64_t oldk2 = k2;
  int64_t kbuf = 0;
  //OMP("omp barrier");
  //OMP("omp for")
#ifdef SERIAL_BFS
  int64_t k2_temp;
  k2_temp=*k2;
  for (k = arg; k < k2_temp; ++k)
#endif
    {

      //rmd_guid_t vlistk_arg;
      //rmd_location_t iter;
      //iter.type = RMD_LOC_TYPE_RELATIVE;
      //iter.data.relative.level = RMD_LOCATION_CORE;
      //int64_t* vlistk = RMD_DB_ALLOC (&vlistk_arg, sizeof(vlist[k]),0,&iter);
      
      //memcpy(vlistk,&vlist[k],sizeof(vlist[k]));
      //printf("vlist%d_%d_%d\n",*vlistk,vlist[k],sizeof(vlist[k]));
      const int64_t v = vlist[k];
      //const int64_t v = *vlistk;
      const int64_t veo = XENDOFF(v);
      int64_t vo;
      for (vo = XOFF(v); vo < veo; ++vo) {
	const int64_t j = xadj[vo];
	if (bfs_tree[j] == -1) {
	  if (_cas (&bfs_tree[j], -1, v)) {
	    if (kbuf < THREAD_BUF) {
	      nbuf[kbuf++] = j;
	    } else {
	      int64_t voff = _fetch_add (k2, THREAD_BUF),vk;
	      assert (voff + THREAD_BUF <= nv);
	      for (vk = 0; vk < THREAD_BUF; ++vk)
		vlist[voff + vk] = nbuf[vk];
	      nbuf[0] = j;
	      kbuf = 1;
	    }
	  }
	}
      }
      //RMD_DB_FREE(vlistk_arg);
    }
  if (kbuf) {
    int64_t voff = _fetch_add (k2, kbuf), vk;
    assert (voff + kbuf <= nv);
    for (vk = 0; vk < kbuf; ++vk)
      vlist[voff + vk] = nbuf[vk];
  }
  
  //OMP("omp single")
  //	k1 = oldk2;
  //OMP("omp barrier");
  //  ((argk1k2 *)(db_ptr[2]))->k1=k1;
  //((argk1k2 *)(db_ptr[2]))->k2=k2;
  int *counter=(int*)db_ptr[3];
#ifdef PRINT_DEBUG_INFO
    //printf("ic=%d_%d ",*counter,k);
#endif
#ifndef SERIAL_BFS
		if(__sync_add_and_fetch(counter,-1)==0)
#endif
      {	
   	RMD_DB_FREE(db[3]);
   	RMD_DB_FREE(db[4]);
	//  printf("COUNTER\n");
 	rmd_guid_t bfs_sync;
    	rmd_codelet_sched(&bfs_sync,*(int*)db_ptr[4],bfs_level_sync_type);
    	rmd_codelet_satisfy(bfs_sync,db[0],0);
    	rmd_codelet_satisfy(bfs_sync,db[1],1);
    	rmd_codelet_satisfy(bfs_sync,db[2],2);
    }
    ret.data= EXIT_SUCCESS;
    rckprintf1("\n--- bfs_mark_nodes_codelet end\n");
    return ret;
}

void destroy_graph_c (void) {
  free_graph_c ();
}

rmd_guid_t init_codelet(uint64_t arg, int n_db, void *db_ptr[],
			rmd_guid_t *db) {

  rckprintf1("\n--- init_codelet\n");
  rmd_guid_t ret;

		
  int64_t desired_nedge;
  if (sizeof (int64_t) < 8) {
    fprintf (stderr, "No 64-bit support.\n");
    ret.data= EXIT_FAILURE;
    return ret;
  }
		
  nvtx_scale = ((int64_t)1)<<SCALE;
		
  init_random ();
		
  desired_nedge = nvtx_scale * edgefactor;
  /* Catch a few possible overflows. */
  assert (desired_nedge >= nvtx_scale);
  assert (desired_nedge >= edgefactor);
  
  /*
    If running the benchmark under an architecture simulator, replace
    the following if () {} else {} with a statement pointing IJ
    to wherever the edge list is mapped into the simulator's memory.
  */
  if (!dumpname) {
    if (VERBOSE) fprintf (stderr, "Generating edge list...");
    if (use_RMAT) {
      nedge = desired_nedge;
#ifdef RMD_DB_MEM
      rmd_location_t loc;
      loc.type = RMD_LOC_TYPE_RELATIVE;
      loc.data.relative.level = RMD_LOCATION_DRAM;
      IJ = RMD_DB_ALLOC (&IJ_DBlock,nedge * sizeof (*IJ),0,&loc);
#else
      IJ = xmalloc_large_ext (nedge * sizeof (*IJ));
#endif
      TIME(generation_time, rmat_edgelist (IJ, nedge, SCALE, A, B, C));
#ifdef PRINT_DEBUG_INFO
      printf("Init Done\n");
#endif
      ret.data= EXIT_SUCCESS;
      rmd_guid_t find_nv_s;
      rmd_codelet_sched(&find_nv_s,0,find_nv_type);
      rckprintf1("\n--- init_codelet 1 end\n");
      return ret;
    } else {
      //TIME(generation_time, make_graph_c (SCALE, desired_nedge, userseed, userseed, &nedge, (packed_edge**)(&IJ)));
      tic();
      make_graph_c (SCALE, desired_nedge, userseed, userseed, &nedge, (packed_edge**)(&IJ));
      //printf("Nedge %d\n",nedge);
    }
    ret.data= EXIT_SUCCESS;
    rckprintf1("\n--- init_codelet 2 end\n");
    return ret;
  } else {
    int fd;
    ssize_t sz;
    if ((fd = open (dumpname, O_RDONLY)) < 0) {
      perror ("Cannot open input graph file");
      ret.data= EXIT_FAILURE;
      rckprintf1("\n--- init_codelet 3 end\n");
      return ret;
    }
    sz = nedge * sizeof (*IJ);
    if (sz != read (fd, IJ, sz)) {
      perror ("Error reading input graph file");
      ret.data= EXIT_FAILURE;
      rckprintf1("\n--- init_codelet 4 end\n");
      return ret;
    }
    close (fd);
#ifdef PRINT_DEBUG_INFO
		printf("Init Done\n");
#endif
    ret.data= EXIT_SUCCESS;
    rmd_guid_t find_nv_s;
    rmd_codelet_sched(&find_nv_s,0,find_nv_type);
    rckprintf1("\n--- init_codelet 5 end\n");
    return ret;
  }

}

rmd_guid_t find_nv_codelet(uint64_t arg, int n_db, void *db_ptr[],
			   rmd_guid_t *db) {
  rckpause;
  rckprintf1("\n--- find_nv_codelet\n");
  generation_time=toc();
#ifdef PRINT_DEBUG_INFO
  printf("find nv Start\n");
#endif
  rmd_guid_t ret;

  if (VERBOSE) fprintf (stderr, "Creating graph...");
  //TIME(construction_time, err = create_graph_from_edgelist_c (IJ, nedge));
  tic();
  // find_nv_c (IJ, nedge);
    
  maxvtx = -1;

  int64_t k, gmaxvtx, tmaxvtx = -1;

  //OMP("omp for")
  for (k = 0; k < nedge; ++k) {
    if (get_v0_from_edge(&IJ[k]) > tmaxvtx)
      tmaxvtx = get_v0_from_edge(&IJ[k]);
    if (get_v1_from_edge(&IJ[k]) > tmaxvtx)
      tmaxvtx = get_v1_from_edge(&IJ[k]);
    
    gmaxvtx = maxvtx;
    while (tmaxvtx > gmaxvtx)
      gmaxvtx = _casval (&maxvtx, gmaxvtx, tmaxvtx);
  }
  
  nv = 1+maxvtx;
  rmd_guid_t create_graph_s;
  rmd_codelet_sched(&create_graph_s,0,create_graph_type);
  
  ret.data= EXIT_SUCCESS;
  rckprintf1("\n--- find_nv_codelet end\n");
  return ret;
}

rmd_guid_t create_graph_codelet(uint64_t arg, int n_db, void *db_ptr[],
				rmd_guid_t *db) {
  
  rckprintf1("\n--- create_graph_codelet\n");
#ifdef PRINT_DEBUG_INFO
  printf("create graph Start\n");
#endif
  rmd_guid_t ret;
  int err;

  err = create_graph_from_edgelist_c (IJ, nedge);
  if (VERBOSE) fprintf (stderr, "done.\n");
  if (err) {
    fprintf (stderr, "Failure creating graph.\n");
    ret.data= EXIT_FAILURE;
    return ret;
  }
#ifdef PRINT_DEBUG_INFO
  printf("Created Graph\n");
#endif
  ret.data= EXIT_SUCCESS;
  rckprintf1("\n--- create_graph_codelet end\n");
  return ret;
}

rmd_guid_t bfs_codelet(uint64_t arg, int n_db, void *db_ptr[],
		       rmd_guid_t *db) {
  rckpause;
  rckprintf1("\n--- bfs_codelet\n");
  construction_time=toc();
#ifdef PRINT_DEBUG_INFO
  printf("BFS Start\n");
#endif
  rmd_guid_t ret;

  int * restrict has_adj;
  int m;
  int64_t k, t;

  /*
    If running the benchmark under an architecture simulator, replace
    the following if () {} else {} with a statement pointing bfs_root
    to wherever the BFS roots are mapped into the simulator's memory.
  */
  if (!rootname) {
 
#ifdef RMD_DB_MEM
    rmd_guid_t has_arg;
    rmd_location_t loc1;
    loc1.type = RMD_LOC_TYPE_RELATIVE;
    loc1.data.relative.level = RMD_LOCATION_DRAM;
    has_adj = RMD_DB_ALLOC (&has_arg, nvtx_scale * sizeof (*has_adj) ,0,&loc1);
#else 
    has_adj = xmalloc_large (nvtx_scale * sizeof (*has_adj));
#endif 
#ifdef MEM_INFO
    printf("has_adj Memory =%ld\n",nvtx_scale*sizeof (*has_adj));
#endif
    //OMP("omp parallel")
    //OMP("omp for")
    for (k = 0; k < nvtx_scale; ++k)
      has_adj[k] = 0;
    MTA("mta assert nodep") //OMP("omp for")
      for (k = 0; k < nedge; ++k) {
	const int64_t i = get_v0_from_edge(&IJ[k]);
	const int64_t j = get_v1_from_edge(&IJ[k]);
	if (i != j)
	  has_adj[i] = has_adj[j] = 1;
      }
    
    /* Sample from {0, ..., nvtx_scale-1} without replacement. */
    m = 0;
    t = 0;
    while (m < NBFS && t < nvtx_scale) {
      double R = mrg_get_double_orig (prng_state);
      if (!has_adj[t] || (nvtx_scale - t)*R > NBFS - m) ++t;
      else bfs_root[m++] = t++;
    }
    if (t >= nvtx_scale && m < NBFS) {
      if (m > 0) {
	fprintf (stderr, "Cannot find %d sample roots of non-self degree > 0, using %d.\n",
		 NBFS, m);
	NBFS = m;
      } else {
	fprintf (stderr, "Cannot find any sample roots of non-self degree > 0.\n");
	//exit (EXIT_FAILURE);
        ret.data= EXIT_FAILURE;
	return ret;
      }
    }
#ifdef RMD_DB_MEM
			RMD_DB_FREE (has_arg);
#else
			xfree_large (has_adj);
#endif
  } else {
    int fd;
    ssize_t sz;
    if ((fd = open (rootname, O_RDONLY)) < 0) {
      perror ("Cannot open input BFS root file");
      //exit (EXIT_FAILURE);
      ret.data= EXIT_FAILURE;
	return ret;
    }
    sz = NBFS * sizeof (*bfs_root);
    if (sz != read (fd, bfs_root, sz)) {
      perror ("Error reading input BFS root file");
      //exit (EXIT_FAILURE);
      ret.data= EXIT_FAILURE;
	return ret;
    }
    close (fd);
  }
  ret.data= EXIT_SUCCESS;
#ifdef PRINT_DEBUG_INFO
  printf("BFS Part1 End\n");
#endif

  rmd_guid_t per_root_s;
  rmd_codelet_sched(&per_root_s,0,per_root_type);
  //rmd_complete();
  rckprintf1("\n--- bfs_codelet end\n");
  return ret;
}

rmd_guid_t per_root_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db) {
  rckpause;
  rckprintf1("\n--- per_root_codelet\n");
#ifdef PRINT_DEBUG_INFO
  // printf("Traverse ");
#endif
  rmd_guid_t ret;
  //rmd_guid_t in_dep={.data=arg}; 
  int m,err;
  m=(int)arg;

#ifdef PRINT_DEBUG_INFO
  printf("iter=%d ",m);
#endif
  //RMD_DB_FREE(db);
  if(m == NBFS ){	
#ifdef PRINT_DEBUG_INFO
    printf("Traverse Done\n");
#endif
    //rmd_event_satisfy(in_dep,db[0]); 
    rmd_guid_t end_s;
    rmd_codelet_sched(&end_s,0,end_type);
    ret.data= EXIT_SUCCESS;
    return ret;
  }
 
  //for (m = 0; m < NBFS; ++m)
  //{
  int64_t *bfs_tree; //, max_bfsvtx;

  // Re-allocate. Some systems may randomize the addres... 
#ifdef RMD_DB_MEM
  rmd_location_t loc2;
  loc2.type = RMD_LOC_TYPE_RELATIVE;
  loc2.data.relative.level = RMD_LOCATION_BLOCK;
  bfs_tree = RMD_DB_ALLOC (&bfs_DBlock, nvtx_scale * sizeof (*bfs_tree) ,0,&loc2);
#else
  bfs_tree = xmalloc_large (nvtx_scale * sizeof (*bfs_tree));
  // printf("bfs_addr=%p\n",*bfs_tree_addr);
#endif  
#ifdef MEM_INFO
  printf("bfs_tree memory =%ld\n",nvtx_scale*sizeof (*bfs_tree)); 
#endif
  assert (bfs_root[m] < nvtx_scale);

  if (VERBOSE) fprintf (stderr, "Running bfs %d...", m);
  //TIME(bfs_time[m], err = make_bfs_tree (bfs_tree, &max_bfsvtx, bfs_root[m]));
  tic();
  err = make_bfs_tree_c (bfs_tree, bfs_root[m],m);

  ret.data= EXIT_SUCCESS;
#ifdef PRINT_DEBUG_INFO
  // printf("Traverse End\n");
#endif
  rckprintf1("\n--- per_root_codelet end\n");
  return ret;
}

rmd_guid_t verify_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db) {
  rckpause;
  rckprintf1("\n--- verify_codelet\n");
#ifdef PRINT_DEBUG_INFO
  // printf("Traverse rest\n");
#endif

  int64_t *bfs_tree=(int64_t*)(*(uint64_t *)db_ptr[0]); 
  RMD_DB_FREE(db[0]);
  //printf("bfs_tree=%p\n",bfs_tree);
  int m;//,err=0;
  m= (int)arg;
  bfs_time[m]= toc();
  //increment value for the next iteration
#ifdef PRINT_DEBUG_INFO
  printf("m=%d\n",m);
#endif
  rmd_guid_t ret;
  
  if (VERBOSE) fprintf (stderr, "done\n");
/*
  if (err) {
  perror ("make_bfs_tree failed");
  abort ();
  }
*/
  //printf("Verify %p=%p\n",bfs_tree[bfs_root[m]],bfs_root[m]);

  if (VERBOSE) fprintf (stderr, "Verifying bfs %d...", m);
  bfs_nedge[m] = verify_bfs_tree_c (bfs_tree, maxvtx, bfs_root[m], IJ, nedge);
  if (VERBOSE) fprintf (stderr, "done\n");
  if (bfs_nedge[m] < 0) {
    fprintf (stderr, "bfs %d from %" PRId64 " failed verification (%" PRId64 ")\n", 
	     m, bfs_root[m], bfs_nedge[m]);
    abort ();
  }
  //printf("Verify Done\n");
#ifdef RMD_DB_MEM
  RMD_DB_FREE (bfs_DBlock);
#else
  xfree_large (bfs_tree);
#endif
  //}

  ret.data= EXIT_SUCCESS;
#ifdef PRINT_DEBUG_INFO
  //printf("Traverse Rest End\n");
#endif

  rmd_guid_t next_iter;
  rmd_codelet_sched(&next_iter,m+1,per_root_type);
  //rmd_codelet_satisfy(next_iter,db[0],0);
  //  rmd_event_satisfy(in_dep,db[0]); 
  rckprintf1("\n--- verify_codelet end\n");
  return ret;
}

rmd_guid_t end_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db) {
  rckprintf1("\n--- end_codelet\n");

  //RMD_DB_FREE(db[0]);
#ifdef PRINT_DEBUG_INFO
  printf("End Begin\n");
#endif
  rmd_guid_t ret;
  destroy_graph_c ();
#ifdef RMD_DB_MEM  	
  RMD_DB_FREE(IJ_DBlock);
#else
  xfree_large (IJ);
#endif

#ifdef PRINT_DEBUG_INFO
  printf("The End\n");
#endif
  rmd_complete();
  ret.data= EXIT_SUCCESS;
  rckprintf1("\n--- end_codelet end\n");
  return ret;
}
#else
// ifdef codelet above
#endif
	
#ifdef CODELETS
rmd_guid_t main_codelet(uint64_t arg, int n_db, void *db_ptr[],
			rmd_guid_t *db) {
  
  rmd_guid_t ret;
  rmd_guid_t init_s;

  //RCK initialize_core_logging("log_output.txt");
  printf("Scale=%ld\n",SCALE);
  
  rmd_codelet_create(&init_type,init_codelet,0,0,1,1,false,0);  
  rmd_codelet_create(&generate_kron_type,generate_kron_codelet,0,0,7,1,false,0); 
  rmd_codelet_create(&find_nv_type,find_nv_codelet,0,0,0,1,false,0);  
  rmd_codelet_create(&create_graph_type,create_graph_codelet,0,0,0,1,false,0); 

  rmd_codelet_create(&gather_edges_type,gather_edges_codelet,0,0,2,1,false,0);  
  rmd_codelet_create(&bfs_type,bfs_codelet,0,0,0,1,false,0);  
  rmd_codelet_create(&bfs_level_sync_type,bfs_level_sync_codelet,0,0,3,1,false,0);  
  rmd_codelet_create(&bfs_mark_nodes_type,bfs_mark_nodes_codelet,0,0,5,1,false,0);  
  rmd_codelet_create(&per_root_type,per_root_codelet,0,0,0,1,false,0);  
  rmd_codelet_create(&verify_type,verify_codelet,0,0,1,1,false,0);  
  rmd_codelet_create(&end_type,end_codelet,0,0,0,1,false,0);  
  
  rmd_codelet_sched(&init_s,0,init_type);
  rmd_codelet_satisfy(init_s,db[0],0);
  
  ret.data = 0;
  return ret;
}
#endif

#define NSTAT 9
#define PRINT_STATS(lbl, israte)					\
  do {									\
    printf ("min_%s: %20.17e\n", lbl, stats[0]);			\
    printf ("firstquartile_%s: %20.17e\n", lbl, stats[1]);		\
    printf ("median_%s: %20.17e\n", lbl, stats[2]);			\
    printf ("thirdquartile_%s: %20.17e\n", lbl, stats[3]);		\
    printf ("max_%s: %20.17e\n", lbl, stats[4]);			\
    if (!israte) {							\
      printf ("mean_%s: %20.17e\n", lbl, stats[5]);			\
      printf ("stddev_%s: %20.17e\n", lbl, stats[6]);			\
    } else {								\
      printf ("harmonic_mean_%s: %20.17e\n", lbl, stats[7]);		\
      printf ("harmonic_stddev_%s: %20.17e\n", lbl, stats[8]);	\
    }									\
  } while (0)


static int dcmp (const void *a, const void *b) {
  const double da = *(const double*)a;
  const double db = *(const double*)b;
  if (da > db) return 1;
  if (db > da) return -1;
  if (da == db) return 0;
  fprintf (stderr, "No NaNs permitted in output.\n");
  abort ();
  return 0;
}

void statistics (double *out, double *data, int64_t n) {
  long double s, mean;
  double t;
  int k;

  /* Quartiles */
  qsort (data, n, sizeof (*data), dcmp);
  out[0] = data[0];
  t = (n+1) / 4.0;
  k = (int) t;
  if (t == k)
    out[1] = data[k];
  else
    out[1] = 3*(data[k]/4.0) + data[k+1]/4.0;
  t = (n+1) / 2.0;
  k = (int) t;
  if (t == k)
    out[2] = data[k];
  else
    out[2] = data[k]/2.0 + data[k+1]/2.0;
  t = 3*((n+1) / 4.0);
  k = (int) t;
  if (t == k)
    out[3] = data[k];
  else
    out[3] = data[k]/4.0 + 3*(data[k+1]/4.0);
  out[4] = data[n-1];

  s = data[n-1];
  for (k = n-1; k > 0; --k)
    s += data[k-1];
  mean = s/n;
  out[5] = mean;
  s = data[n-1] - mean;
  s *= s;
  for (k = n-1; k > 0; --k) {
    long double tmp = data[k-1] - mean;
    s += tmp * tmp;
  }
  out[6] = sqrt (s/(n-1));

  s = (data[0]? 1.0L/data[0] : 0);
  for (k = 1; k < n; ++k)
    s += (data[k]? 1.0L/data[k] : 0);
  out[7] = n/s;
  mean = s/n;

  /*
    Nilan Norris, The Standard Errors of the Geometric and Harmonic
    Means and Their Application to Index Numbers, 1940.
    http://www.jstor.org/stable/2235723
  */
  s = (data[0]? 1.0L/data[0] : 0) - mean;
  s *= s;
  for (k = 1; k < n; ++k) {
    long double tmp = (data[k]? 1.0L/data[k] : 0) - mean;
    s += tmp * tmp;
  }
  s = (sqrt (s)/(n-1)) * out[7] * out[7];
  out[8] = s;
}


static int compute_levels1 (int64_t * level, int64_t nv,
			    const int64_t * restrict bfs_tree, int64_t root) {
  int err = 0;

  //OMP("omp parallel shared(err)") 
  int terr;
  int64_t k;
  
  //OMP("omp for")
  for (k = 0; k < nv; ++k)
    level[k] = (k == root? 0 : -1);

  //OMP("omp for") MTA("mta assert parallel") MTA("mta use 100 streams")
  for (k = 0; k < nv; ++k) {
    if (level[k] >= 0) continue;
    terr = err;
    if (!terr && bfs_tree[k] >= 0 && k != root) {
      int64_t parent = k;
      int64_t nhop = 0;
      /* Run up the tree until we encounter an already-leveled vertex. */
      while (parent >= 0 && level[parent] < 0 && nhop < nv) {
	assert (parent != bfs_tree[parent]);
	parent = bfs_tree[parent];
	++nhop;
      }
      if (nhop >= nv) terr = -1; /* Cycle. */
      if (parent < 0) terr = -2; /* Ran off the end. */
      
      if (!terr) {
	/* Now assign levels until we meet an already-leveled vertex */
	/* NOTE: This permits benign races if parallelized. */
	nhop += level[parent];
	parent = k;
	while (level[parent] < 0) {
	  assert (nhop > 0);
	  level[parent] = nhop--;
	  parent = bfs_tree[parent];
	}
	assert (nhop == level[parent]);
	
	/* Internal check to catch mistakes in races... */
#if !defined(NDEBUG)
	nhop = 0;
	parent = k;
	int64_t lastlvl = level[k]+1;
	while (level[parent] > 0) {
	  assert (lastlvl == 1 + level[parent]);
	  lastlvl = level[parent];
	  parent = bfs_tree[parent];
	  ++nhop;
	}
#endif
      }

      if (terr) { err = terr;}	//OMP("omp flush (err)"); }
    }
  }
  return err;
}

int64_t verify_bfs_tree_c (int64_t *bfs_tree_in, int64_t max_bfsvtx,
		   int64_t root, const int64_t *IJ_in, int64_t nedge) {
  int64_t * restrict bfs_tree = bfs_tree_in;
  const int64_t * restrict IJ = IJ_in;

  int err, nedge_traversed;
  int64_t * restrict seen_edge, * restrict level;

  const int64_t nv = max_bfsvtx+1;

  /*
    This code is horrifically contorted because many compilers
    complain about continue, return, etc. in parallel sections.
  */

  if (root > max_bfsvtx || bfs_tree[root] != root)
    return -999;

  err = 0;
  nedge_traversed = 0;
  seen_edge = xmalloc_large (2 * (nv) * sizeof (*seen_edge));
  level = &seen_edge[nv];

  err = compute_levels1 (level, nv, bfs_tree, root);

  if (err) goto done;

  //OMP("omp parallel shared(err)") 
  int64_t k;
  int terr = 0;
  //OMP("omp for")
  for (k = 0; k < nv; ++k)
    seen_edge[k] = 0;
  
  //OMP("omp for reduction(+:nedge_traversed)")
  // MTA("mta assert parallel") MTA("mta use 100 streams")
  for (k = 0; k < 2*nedge; k+=2) {
    const int64_t i = IJ[k];
    const int64_t j = IJ[k+1];
    int64_t lvldiff;
    terr = err;
    
    if (i < 0 || j < 0) continue;
    if (i > max_bfsvtx && j <= max_bfsvtx) terr = -10;
    if (j > max_bfsvtx && i <= max_bfsvtx) terr = -11;
    if (terr) { err = terr; }//OMP("omp flush(err)"); }
    if (terr || i > max_bfsvtx /* both i & j are on the same side of max_bfsvtx */)
      continue;
    
    /* All neighbors must be in the tree. */
    if (bfs_tree[i] >= 0 && bfs_tree[j] < 0) terr = -12;
    if (bfs_tree[j] >= 0 && bfs_tree[i] < 0) terr = -13;
    if (terr) { err = terr; }//OMP("omp flush(err)"); }
    if (terr || bfs_tree[i] < 0 /* both i & j have the same sign */)
      continue;
    
    /* Both i and j are in the tree, count as a traversed edge.
       
       NOTE: This counts self-edges and repeated edges.  They're
       part of the input data.
    */
    ++nedge_traversed;
    /* Mark seen tree edges. */
    if (i != j) {
      if (bfs_tree[i] == j)
	seen_edge[i] = 1;
      if (bfs_tree[j] == i)
	seen_edge[j] = 1;
    }
    lvldiff = level[i] - level[j];
    /* Check that the levels differ by no more than one. */
    if (lvldiff > 1 || lvldiff < -1)
      terr = -14;
    if (terr) { err = terr; }//OMP("omp flush(err)"); }
  }
  
  if (!terr) {
    //Check that every BFS edge was seen and that there's only one root.
    //OMP("omp for") MTA("mta assert parallel") MTA("mta use 100 streams")
    for (k = 0; k < nv; ++k) {
      terr = err;
      if (!terr && k != root) {
	if (bfs_tree[k] >= 0 && !seen_edge[k])
	  terr = -15;
	if (bfs_tree[k] == k)
	  terr = -16;
	if (terr) { err = terr; }//OMP("omp flush(err)"); }
      }
    }
  }

 done:
  
  xfree_large (seen_edge);
  if (err) return err;
  return nedge_traversed;
}
