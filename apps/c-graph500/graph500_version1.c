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

#if !defined(__MTA__)
#include <getopt.h>
#endif

#include "codelet.h"
#include "rmd_afl_all.h"
#include "graph500.h"
#include "rmat.h"
#include "kronecker.h"
#include "verify.h"
#include "prng.h"
#include "timer.h"
#include "xalloc.h"
#include "atomics.h"
#include "options.h"
#include "generator/splittable_mrg.h"
#include "generator/graph_generator.h"
#include "generator/make_graph.h"

//#define RMD_DB_ALLOC(A,B,C,D) malloc(B)
//#define RMD_DB_FREE free

static int64_t nvtx_scale;

static int64_t bfs_root[NBFS_max];

static double generation_time;
static double construction_time;
static double bfs_time[NBFS_max];
static int64_t bfs_nedge[NBFS_max];

static packed_edge * restrict IJ;
static int64_t nedge;

#define MINVECT_SIZE 2

#define XOFF(k) (xoff[2*(k)])
#define XENDOFF(k) (xoff[1+2*(k)])

#define THREAD_BUF_LEN 16384
#define CHUNK_SIZE 256

static int64_t maxvtx, nv, sz;
static int64_t * restrict xoff; /* Length 2*nv+2 */
static int64_t * restrict xadjstore; /* Length MINVECT_SIZE + (xoff[nv] == nedge) */
static int64_t * restrict xadj;

static void run_bfs (void);
int64_t verify_bfs_tree1 (int64_t *bfs_tree_in, int64_t max_bfsvtx, int64_t root, const int64_t *IJ_in, int64_t nedge);
static void output_results (const int64_t SCALE, int64_t nvtx_scale,
			    int64_t edgefactor,
			    const double A, const double B,
			    const double C, const double D,
			    const double generation_time,
			    const double construction_time,
			    const int NBFS,
			    const double *bfs_time, const int64_t *bfs_nedge);

rmd_guid_t graph_from_edgelist;
rmd_guid_t IJ_arg;

static void
free_graph (void)
{
  xfree_large (xadjstore);
  xfree_large (xoff);
}

void
destroy_graph (void)
{
  free_graph ();
}

int64_t
int64_casval1(int64_t* p, int64_t oldval, int64_t newval)
{
  return __sync_val_compare_and_swap (p, oldval, newval);
}
static int64_t
prefix_sum1 (int64_t *buf)
{
  int nt, tid;
  int64_t slice_begin, slice_end, t1, t2, k;

  nt = omp_get_num_threads ();
  tid = omp_get_thread_num ();

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


static int
setup_deg_off1 (const struct packed_edge * restrict IJ, int64_t nedge)
{
  int err = 0;
  int64_t *buf = NULL;
  xadj = NULL;
 // OMP("omp parallel")
  {
    int64_t k, accum;
   // OMP("omp for")
      for (k = 0; k < 2*nv+2; ++k)
	xoff[k] = 0;
   // OMP("omp for")
      for (k = 0; k < nedge; ++k) {
        int64_t i = get_v0_from_edge(&IJ[k]);
        int64_t j = get_v1_from_edge(&IJ[k]);
	if (i != j) { /* Skip self-edges. */
	  if (i >= 0)
	  //  OMP("omp atomic")
	      ++XOFF(i);
	  if (j >= 0)
	  //  OMP("omp atomic")
	      ++XOFF(j);
	}
      }
    //OMP("omp single")
   {
      buf = alloca (omp_get_num_threads () * sizeof (*buf));
      if (!buf) {
	perror ("alloca for prefix-sum hosed");
	abort ();
      }
    }
   // OMP("omp for")
      for (k = 0; k < nv; ++k)
	if (XOFF(k) < MINVECT_SIZE) XOFF(k) = MINVECT_SIZE;

    accum = prefix_sum1 (buf);

   // OMP("omp for")
      for (k = 0; k < nv; ++k)
	XENDOFF(k) = XOFF(k);
   // OMP("omp single") 
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
  }
  return !xadj;
}


static void
scatter_edge1 (const int64_t i, const int64_t j)
{
  int64_t where;
  where = hc_fetch_and_add ((int*)&XENDOFF(i), 1);
  xadj[where] = j;
}

static int
i64cmp1 (const void *a, const void *b)
{
  const int64_t ia = *(const int64_t*)a;
  const int64_t ib = *(const int64_t*)b;
  if (ia < ib) return -1;
  if (ia > ib) return 1;
  return 0;
}

static void
pack_vtx_edges1 (const int64_t i)
{
  int64_t kcur, k;
  if (XOFF(i)+1 >= XENDOFF(i)) return;
  qsort (&xadj[XOFF(i)], XENDOFF(i)-XOFF(i), sizeof(*xadj), i64cmp1);
  kcur = XOFF(i);
  for (k = XOFF(i)+1; k < XENDOFF(i); ++k)
    if (xadj[k] != xadj[kcur])
      xadj[++kcur] = xadj[k];
  ++kcur;
  for (k = kcur; k < XENDOFF(i); ++k)
    xadj[k] = -1;
  XENDOFF(i) = kcur;
}

static void
pack_edges1 (void)
{
  int64_t v;

 // OMP("omp for")
    for (v = 0; v < nv; ++v)
      pack_vtx_edges1 (v);
}

static int
alloc_graph1 (int64_t nedge)
{
  sz = (2*nv+2) * sizeof (*xoff);
  xoff = xmalloc_large_ext (sz);
  if (!xoff) return -1;
  return 0;
}

static void
find_nv1 (const struct packed_edge * restrict IJ, const int64_t nedge)
{
  maxvtx = -1;
 {
    int64_t k, gmaxvtx, tmaxvtx = -1;

    //OMP("omp for")
      for (k = 0; k < nedge; ++k) {
	if (get_v0_from_edge(&IJ[k]) > tmaxvtx)
	  tmaxvtx = get_v0_from_edge(&IJ[k]);
	if (get_v1_from_edge(&IJ[k]) > tmaxvtx)
	  tmaxvtx = get_v1_from_edge(&IJ[k]);
      }
    gmaxvtx = maxvtx;
    while (tmaxvtx > gmaxvtx)
      gmaxvtx = int64_casval1 (&maxvtx, gmaxvtx, tmaxvtx);
  }
  nv = 1+maxvtx;
}


static void
gather_edges1 (const struct packed_edge * restrict IJ, int64_t nedge)
{
 // OMP("omp parallel")
 {
    int64_t k;

   // OMP("omp for")
      for (k = 0; k < nedge; ++k) {
        int64_t i = get_v0_from_edge(&IJ[k]);
        int64_t j = get_v1_from_edge(&IJ[k]);
	if (i >= 0 && j >= 0 && i != j) {
	  scatter_edge1 (i, j);
	  scatter_edge1 (j, i);
	}
      }

    pack_edges1 ();
  }
}

int 
create_graph_from_edgelist1 (struct packed_edge *IJ, int64_t nedge)
{
  find_nv1 (IJ, nedge);
 //allocates XOFF
  if (alloc_graph1 (nedge)) return -1;
  if (setup_deg_off1 (IJ, nedge)) {
    xfree_large (xoff);
    return -1;
  }
  gather_edges1 (IJ, nedge);
  return 0;
}

void make_graph1(int log_numverts, int64_t M, uint64_t userseed1, uint64_t userseed2, int64_t* nedges_ptr_in, packed_edge** result_ptr_in) {
  /* Add restrict to input pointers. */
  int64_t* restrict nedges_ptr = nedges_ptr_in;
  packed_edge* restrict* restrict result_ptr = result_ptr_in;

  /* Spread the two 64-bit numbers into five nonzero values in the correct
   * range. */
  uint_fast32_t seed[5];
  make_mrg_seed(userseed1, userseed2, seed);

  *nedges_ptr = M;
  packed_edge* edges = (packed_edge*)xmalloc(M * sizeof(packed_edge));
  *result_ptr = edges;

  /* In OpenMP and XMT versions, the inner loop in generate_kronecker_range is
   * parallel.  */
  generate_kronecker_range(seed, log_numverts, 0, M, edges);
}

int main_code(int argc, char **argv)
{
  int64_t desired_nedge;
  if (sizeof (int64_t) < 8) {
    fprintf (stderr, "No 64-bit support.\n");
    return EXIT_FAILURE;
  }

  if (argc > 1)
    get_options (argc, argv);

  nvtx_scale = ((int64_t)1)<<SCALE;

  init_random ();

  desired_nedge = nvtx_scale * edgefactor;
  /* Catch a few possible overflows. */
  assert (desired_nedge >= nvtx_scale);
  assert (desired_nedge >= edgefactor);

     rmd_location_t loc;
  /*
    If running the benchmark under an architecture simulator, replace
    the following if () {} else {} with a statement pointing IJ
    to wherever the edge list is mapped into the simulator's memory.
  */
  if (!dumpname) {
    if (VERBOSE) fprintf (stderr, "Generating edge list...");
    if (use_RMAT) {
      nedge = desired_nedge;
     // loc.type = RMD_LOC_TYPE_RELATIVE;
     // loc.data.relative.level = RMD_LOCATION_CORE;
      //IJ = xmalloc_large_ext (nedge * sizeof (*IJ));
      IJ = RMD_DB_ALLOC (&IJ_arg, nedge * sizeof (*IJ),0,&loc);
     printf("Size1 =%ld\n",nedge*sizeof (*IJ)); 
     TIME(generation_time, rmat_edgelist (IJ, nedge, SCALE, A, B, C));
    } else {
      TIME(generation_time, make_graph1 (SCALE, desired_nedge, userseed, userseed, &nedge, (packed_edge**)(&IJ)));
    }
    if (VERBOSE) fprintf (stderr, " done.\n");
  } else {
    int fd;
    ssize_t sz;
    if ((fd = open (dumpname, O_RDONLY)) < 0) {
      perror ("Cannot open input graph file");
      return EXIT_FAILURE;
    }
    sz = nedge * sizeof (*IJ);
    if (sz != read (fd, IJ, sz)) {
      perror ("Error reading input graph file");
      return EXIT_FAILURE;
    }
    close (fd);
  }

  run_bfs ();

  //xfree_large (IJ);
    if (use_RMAT) {//was allocated if RMAT is set
  	RMD_DB_FREE(IJ_arg);
    }
  output_results (SCALE, nvtx_scale, edgefactor, A, B, C, D,
		  generation_time, construction_time, NBFS, bfs_time, bfs_nedge);

  return EXIT_SUCCESS;
}

rmd_guid_t init_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db)
{

  rmd_guid_t ret;
  rmd_cmd_line_t *ptr = db_ptr[0];
   rmd_guid_t in_dep={.data=arg}; 
#ifdef PRINT_DEBUG_INFO
   printf("Init START\n");
#endif
  int64_t desired_nedge;
  if (sizeof (int64_t) < 8) {
    fprintf (stderr, "No 64-bit support.\n");
    ret.data= EXIT_FAILURE;
     return ret;
  }

  if (ptr->argc > 1)
    get_options (ptr->argc, (char**)ptr->argv);

  nvtx_scale = ((int64_t)1)<<SCALE;

  init_random ();

  desired_nedge = nvtx_scale * edgefactor;
  /* Catch a few possible overflows. */
  assert (desired_nedge >= nvtx_scale);
  assert (desired_nedge >= edgefactor);

     rmd_location_t loc;
  /*
    If running the benchmark under an architecture simulator, replace
    the following if () {} else {} with a statement pointing IJ
    to wherever the edge list is mapped into the simulator's memory.
  */
  if (!dumpname) {
    if (VERBOSE) fprintf (stderr, "Generating edge list...");
    if (use_RMAT) {
      nedge = desired_nedge;
     // loc.type = RMD_LOC_TYPE_RELATIVE;
     // loc.data.relative.level = RMD_LOCATION_CORE;
      //IJ = xmalloc_large_ext (nedge * sizeof (*IJ));
      IJ = RMD_DB_ALLOC (&IJ_arg, nedge * sizeof (*IJ),0,&loc);
     printf("Size1 =%ld\n",nedge*sizeof (*IJ)); 
     TIME(generation_time, rmat_edgelist (IJ, nedge, SCALE, A, B, C));
    } else {
      TIME(generation_time, make_graph1 (SCALE, desired_nedge, userseed, userseed, &nedge, (packed_edge**)(&IJ)));
    }
    if (VERBOSE) fprintf (stderr, " done.\n");
  } else {
    int fd;
    ssize_t sz;
    if ((fd = open (dumpname, O_RDONLY)) < 0) {
      perror ("Cannot open input graph file");
      ret.data= EXIT_FAILURE;
	return ret;
    }
    sz = nedge * sizeof (*IJ);
    if (sz != read (fd, IJ, sz)) {
      perror ("Error reading input graph file");
      ret.data= EXIT_FAILURE;
	return ret;
    }
    close (fd);
    }
#ifdef PRINT_DEBUG_INFO
    printf("Init Done\n");
#endif
    ret.data= EXIT_SUCCESS;
    rmd_event_satisfy(in_dep,db[0]); 
    //rmd_complete();
    return ret;
}
rmd_guid_t bfs_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db)
{
#ifdef PRINT_DEBUG_INFO
    printf("BFS Start\n");
#endif
    rmd_guid_t ret;
    rmd_guid_t in_dep={.data=arg}; 

    run_bfs();
 
    rmd_event_satisfy(in_dep,db[0]); 
    ret.data= EXIT_SUCCESS;
#ifdef PRINT_DEBUG_INFO
    printf("BFS END\n");
#endif
    //rmd_complete();
    return ret;
}
rmd_guid_t end_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db)
{

#ifdef PRINT_DEBUG_INFO
   printf("End Begin\n");
#endif
  rmd_guid_t ret;
  //xfree_large (IJ);
    if (use_RMAT) {//was allocated if RMAT is set
  	RMD_DB_FREE(IJ_arg);
    }
  output_results (SCALE, nvtx_scale, edgefactor, A, B, C, D,
		  generation_time, construction_time, NBFS, bfs_time, bfs_nedge);

#ifdef PRINT_DEBUG_INFO
   printf("The End\n");
#endif
    rmd_complete();
      ret.data= EXIT_SUCCESS;
	return ret;
}

rmd_guid_t main_codelet(uint64_t arg, int n_db, void *db_ptr[], rmd_guid_t *db)
{

    rmd_guid_t ret;
    rmd_guid_t init_type;
    rmd_guid_t init_s;
    rmd_guid_t bfs_traverse_type;
    rmd_guid_t bfs_s;
    rmd_guid_t end_type;
    rmd_guid_t end_s;
    rmd_guid_t bfs_done_event;
    rmd_guid_t init_done_event;
  
    rmd_location_t loc;
    rmd_cmd_line_t *ptr = db_ptr[0];
  
    rmd_codelet_create(&init_type,init_codelet,0,0,1,1,false,0);  
    rmd_codelet_create(&bfs_traverse_type,bfs_codelet,0,0,1,1,false,0);  
    rmd_codelet_create(&end_type,end_codelet,0,0,1,1,false,0);  
 
    rmd_event_create(&init_done_event, ID_EVENT_TYPE,false); 
    //main_code(ptr->argc,(char**)ptr->argv);
    rmd_event_create(&bfs_done_event, ID_EVENT_TYPE,false); 

    rmd_codelet_sched(&end_s,0,end_type);
    rmd_dep_add(bfs_done_event,end_s,0);
    rmd_codelet_sched(&bfs_s,bfs_done_event.data,bfs_traverse_type);
    rmd_dep_add(init_done_event,bfs_s,0);
    rmd_codelet_sched(&init_s,init_done_event.data,init_type);
    rmd_codelet_satisfy(init_s,db[0],0);
 
    //rmd_complete();
    ret.data = 0;
    return ret;
}


int
make_bfs_tree1 (int64_t *bfs_tree_out, int64_t *max_vtx_out,
	       int64_t srcvtx)
{
#ifdef PRINT_DEBUG_INFO
   printf("Make_bfs_tree1\n");
#endif
  int64_t * restrict bfs_tree = bfs_tree_out;
  int err = 0;

  int64_t * restrict vlist = NULL;
  int64_t k1, k2;

  *max_vtx_out = maxvtx;

  rmd_guid_t vlist_arg;
  rmd_location_t loc4;
  loc4.type = RMD_LOC_TYPE_RELATIVE;
  loc4.data.relative.level = RMD_LOCATION_DRAM;
  vlist = RMD_DB_ALLOC (&vlist_arg, nv * sizeof (*vlist) ,0,&loc4);
#ifdef PRINT_DEBUG_INFO
   printf("Size4 =%ld\n",nv*sizeof (*vlist)); 
#endif
 // vlist = xmalloc_large (nv * sizeof (*vlist));
  if (!vlist) return -1;

  vlist[0] = srcvtx;
  k1 = 0; k2 = 1;
  bfs_tree[srcvtx] = srcvtx;

#define THREAD_BUF_LEN 16384

 // OMP("omp parallel shared(k1, k2)") {
    int64_t k;
    int64_t nbuf[THREAD_BUF_LEN];
   // OMP("omp for")
      for (k = 0; k < srcvtx; ++k)
	bfs_tree[k] = -1;
    //OMP("omp for")
      for (k = srcvtx+1; k < nv; ++k)
	bfs_tree[k] = -1;

    while (k1 != k2) {
      const int64_t oldk2 = k2;
      int64_t kbuf = 0;
      //OMP("omp barrier");
      //OMP("omp for")
	for (k = k1; k < oldk2; ++k) {
	  const int64_t v = vlist[k];
	  const int64_t veo = XENDOFF(v);
	  int64_t vo;
	  for (vo = XOFF(v); vo < veo; ++vo) {
	    const int64_t j = xadj[vo];
	    if (bfs_tree[j] == -1) {
	      if (hc_cas ((volatile int*)&bfs_tree[j], -1, v)) {
		if (kbuf < THREAD_BUF_LEN) {
		  nbuf[kbuf++] = j;
		} else {
		  int64_t voff = hc_fetch_and_add ((int*)&k2, THREAD_BUF_LEN), vk;
		  assert (voff + THREAD_BUF_LEN <= nv);
		  for (vk = 0; vk < THREAD_BUF_LEN; ++vk)
		    vlist[voff + vk] = nbuf[vk];
		  nbuf[0] = j;
		  kbuf = 1;
		}
	      }
	    }
	  }
	}
      if (kbuf) {
	int64_t voff = hc_fetch_and_add ((int *)&k2, kbuf), vk;
	assert (voff + kbuf <= nv);
	for (vk = 0; vk < kbuf; ++vk)
	  vlist[voff + vk] = nbuf[vk];
      }
    //  OMP("omp single")
	k1 = oldk2;
    //  OMP("omp barrier");
    }
 // }

  //xfree_large (vlist);
   RMD_DB_FREE(vlist_arg);
#ifdef PRINT_DEBUG_INFO
   printf("BFS_TREE1_END\n");
#endif
  return err;
}

void
run_bfs (void)
{
  int * restrict has_adj;
  int m, err;
  int64_t k, t;
#ifdef PRINT_DEBUG_INFO
  printf("Edgelist Start\n");
#endif
  if (VERBOSE) fprintf (stderr, "Creating graph...");
  //TIME(construction_time, err = create_graph_from_edgelist (IJ, nedge));
  tic();
  err = create_graph_from_edgelist1 (IJ, nedge);
  construction_time=toc(); 
#ifdef PRINT_DEBUG_INFO
  printf("Edgelist End\n");
#endif
  if (VERBOSE) fprintf (stderr, "done.\n");
  if (err) {
    fprintf (stderr, "Failure creating graph.\n");
    exit (EXIT_FAILURE);
  }

  /*
    If running the benchmark under an architecture simulator, replace
    the following if () {} else {} with a statement pointing bfs_root
    to wherever the BFS roots are mapped into the simulator's memory.
  */
  if (!rootname) {
    //has_adj = xmalloc_large (nvtx_scale * sizeof (*has_adj));
    rmd_guid_t has_arg;
    rmd_location_t loc1;
    loc1.type = RMD_LOC_TYPE_RELATIVE;
    loc1.data.relative.level = RMD_LOCATION_DRAM;
    has_adj = RMD_DB_ALLOC (&has_arg, nvtx_scale * sizeof (*has_adj) ,0,&loc1);
     printf("Size2 =%ld\n",nvtx_scale*sizeof (*has_adj)); 
   // OMP("omp parallel") {
    //  OMP("omp for")
	for (k = 0; k < nvtx_scale; ++k)
	  has_adj[k] = 0;
      //MTA("mta assert nodep")// OMP("omp for")
	for (k = 0; k < nedge; ++k) {
	  const int64_t i = get_v0_from_edge(&IJ[k]);
	  const int64_t j = get_v1_from_edge(&IJ[k]);
	  if (i != j)
	    has_adj[i] = has_adj[j] = 1;
	}
   // }

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
	exit (EXIT_FAILURE);
      }
    }

    //xfree_large (has_adj);
    //RMD_DB_RELEASE (has_arg);
    RMD_DB_FREE (has_arg);
  } else {
    int fd;
    ssize_t sz;
    if ((fd = open (rootname, O_RDONLY)) < 0) {
      perror ("Cannot open input BFS root file");
      exit (EXIT_FAILURE);
    }
    sz = NBFS * sizeof (*bfs_root);
    if (sz != read (fd, bfs_root, sz)) {
      perror ("Error reading input BFS root file");
      exit (EXIT_FAILURE);
    }
    close (fd);
  }

#ifdef PRINT_DEBUG_INFO
  printf("Traverse Start\n");
#endif
  for (m = 0; m < NBFS; ++m) {
    int64_t *bfs_tree, max_bfsvtx;

    /* Re-allocate. Some systems may randomize the addres... */
    //bfs_tree = xmalloc_large (nvtx_scale * sizeof (*bfs_tree));
    rmd_guid_t bfs_arg;
    rmd_location_t loc2;
    loc2.type = RMD_LOC_TYPE_RELATIVE;
    loc2.data.relative.level = RMD_LOCATION_DRAM;
    bfs_tree = RMD_DB_ALLOC (&bfs_arg, nvtx_scale * sizeof (*bfs_tree) ,0,&loc2);
#ifdef PRINT_DEBUG_INFO
    printf("Size3 =%ld\n",nvtx_scale*sizeof (*bfs_tree)); 
#endif
    assert (bfs_root[m] < nvtx_scale);

    if (VERBOSE) fprintf (stderr, "Running bfs %d...", m);
    TIME(bfs_time[m], err = make_bfs_tree1 (bfs_tree, &max_bfsvtx, bfs_root[m]));
    if (VERBOSE) fprintf (stderr, "done\n");

    if (err) {
      perror ("make_bfs_tree failed");
      abort ();
    }

    if (VERBOSE) fprintf (stderr, "Verifying bfs %d...", m);
    bfs_nedge[m] = verify_bfs_tree1 (bfs_tree, max_bfsvtx, bfs_root[m], IJ, nedge);
    if (VERBOSE) fprintf (stderr, "done\n");
    if (bfs_nedge[m] < 0) {
      fprintf (stderr, "bfs %d from %" PRId64 " failed verification (%" PRId64 ")\n",
	       m, bfs_root[m], bfs_nedge[m]);
      abort ();
    }

    //xfree_large (bfs_tree);
    //RMD_DB_RELEASE (bfs_arg);
    RMD_DB_FREE (bfs_arg);
  }

#ifdef PRINT_DEBUG_INFO
  printf("Traverse End\n");
#endif
  destroy_graph ();
}

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


static int
dcmp (const void *a, const void *b)
{
  const double da = *(const double*)a;
  const double db = *(const double*)b;
  if (da > db) return 1;
  if (db > da) return -1;
  if (da == db) return 0;
  fprintf (stderr, "No NaNs permitted in output.\n");
  abort ();
  return 0;
}

void
statistics (double *out, double *data, int64_t n)
{
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

void
output_results (const int64_t SCALE, int64_t nvtx_scale, int64_t edgefactor,
		const double A, const double B, const double C, const double D,
		const double generation_time,
		const double construction_time,
		const int NBFS, const double *bfs_time, const int64_t *bfs_nedge)
{
  int k;
  int64_t sz;
  double *tm;
  double *stats;

  tm = alloca (NBFS * sizeof (*tm));
  stats = alloca (NSTAT * sizeof (*stats));
  if (!tm || !stats) {
    perror ("Error allocating within final statistics calculation.");
    abort ();
  }

  sz = (1L << SCALE) * edgefactor * 2 * sizeof (int64_t);
  printf ("SCALE: %" PRId64 "\nnvtx: %" PRId64 "\nedgefactor: %" PRId64 "\n"
	  "terasize: %20.17e\n",
	  SCALE, nvtx_scale, edgefactor, sz/1.0e12);
  printf ("A: %20.17e\nB: %20.17e\nC: %20.17e\nD: %20.17e\n", A, B, C, D);
  printf ("generation_time: %20.17e\n", generation_time);
  printf ("construction_time: %20.17e\n", construction_time);
  printf ("nbfs: %d\n", NBFS);

  memcpy (tm, bfs_time, NBFS*sizeof(tm[0]));
  statistics (stats, tm, NBFS);
  PRINT_STATS("time", 0);

  for (k = 0; k < NBFS; ++k)
    tm[k] = bfs_nedge[k];
  statistics (stats, tm, NBFS);
  PRINT_STATS("nedge", 0);

  for (k = 0; k < NBFS; ++k)
    tm[k] = bfs_nedge[k] / bfs_time[k];
  statistics (stats, tm, NBFS);
  PRINT_STATS("TEPS", 1);
}

static int
compute_levels1 (int64_t * level,
		int64_t nv, const int64_t * restrict bfs_tree, int64_t root)
{
  int err = 0;

  //OMP("omp parallel shared(err)")
  {
    int terr;
    int64_t k;

   // OMP("omp for")
      for (k = 0; k < nv; ++k)
	level[k] = (k == root? 0 : -1);

   // OMP("omp for") MTA("mta assert parallel") MTA("mta use 100 streams")
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
	}
	if (terr) { err = terr;	OMP("omp flush (err)"); }
      }
  }
  return err;
}

int64_t
verify_bfs_tree1 (int64_t *bfs_tree_in, int64_t max_bfsvtx,
		 int64_t root,
		 const int64_t *IJ_in, int64_t nedge)
{
  int64_t * restrict bfs_tree = bfs_tree_in;
  const int64_t * restrict IJ = IJ_in;

  int err, nedge_traversed;
  int64_t * restrict seen_edge, * restrict level;

  const int64_t nv = max_bfsvtx+1;
#ifdef PRINT_DEBUG_INFO
printf("Verify_bfs_tree\n");
#endif
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
  {
    int64_t k;
    int terr = 0;
    //OMP("omp for")
      for (k = 0; k < nv; ++k)
	seen_edge[k] = 0;

    //OMP("omp for reduction(+:nedge_traversed)")
    //MTA("mta assert parallel") MTA("mta use 100 streams")
      for (k = 0; k < 2*nedge; k+=2) {
	const int64_t i = IJ[k];
	const int64_t j = IJ[k+1];
	int64_t lvldiff;
	terr = err;

	if (i < 0 || j < 0) continue;
	if (i > max_bfsvtx && j <= max_bfsvtx) terr = -10;
	if (j > max_bfsvtx && i <= max_bfsvtx) terr = -11;
	if (terr) { err = terr; OMP("omp flush(err)"); }
	if (terr || i > max_bfsvtx /* both i & j are on the same side of max_bfsvtx */)
	  continue;

#ifdef PRINT_DEBUG_INFO
//printf("Verify_bfs_tree -All Neighbors\n");
#endif
	/* All neighbors must be in the tree. */
	if (bfs_tree[i] >= 0 && bfs_tree[j] < 0) terr = -12;
	if (bfs_tree[j] >= 0 && bfs_tree[i] < 0) terr = -13;
	if (terr) {printf("k=%ld ",k); err = terr; }//OMP("omp flush(err)"); }
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
	if (terr) { err = terr; OMP("omp flush(err)"); }
      }

    if (!terr) {
      /* Check that every BFS edge was seen and that there's only one root. */
     // OMP("omp for") MTA("mta assert parallel") MTA("mta use 100 streams")
	for (k = 0; k < nv; ++k) {
	  terr = err;
	  if (!terr && k != root) {
	    if (bfs_tree[k] >= 0 && !seen_edge[k])
	      terr = -15;
	    if (bfs_tree[k] == k)
	      terr = -16;
	    if (terr) { err = terr; OMP("omp flush(err)"); }
	  }
	}
    }
  }
 done:

  xfree_large (seen_edge);
  if (err) return err;
  return nedge_traversed;
}
