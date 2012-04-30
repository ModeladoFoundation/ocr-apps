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

#include "hc.h"

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
#include "generator/utils.h"

static int nproc;

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

static int create_graph_from_edgelist (struct packed_edge *IJ, int64_t nedge);
static int make_bfs_tree (int64_t *bfs_tree_out, int64_t *max_vtx_out, int64_t srcvtx);
static int64_t verify_bfs_tree (int64_t *bfs_tree_in, int64_t max_bfsvtx, int64_t root, struct packed_edge * IJ_in, int64_t nedge);
static void destroy_graph (void);
static void make_graph(int log_numverts, int64_t M, uint64_t userseed1, uint64_t userseed2, int64_t* nedges_ptr_in, packed_edge** result_ptr_in);

static void run_bfs (void);
static void output_results (const int64_t SCALE, int64_t nvtx_scale,
			    int64_t edgefactor,
			    const double A, const double B,
			    const double C, const double D,
			    const double generation_time,
			    const double construction_time,
			    const int NBFS,
			    const double *bfs_time, const int64_t *bfs_nedge);

/*********** Worksharing FOR construct *********/
typedef struct _WSFOR {
	volatile int count;
	int lb, ub; /* [lb, ub) */
} WSFOR;

static WSFOR * wsf;

void SET_WSFOR(WSFOR * w, int lower, int upper) {
	w->count = lower;
	w->lb = lower;
	w->ub = upper;
}

int GET_CHUNK_WSFOR(WSFOR * w, int chunk) {
	return hc_fetch_and_add((int *)&w->count, chunk);
}

/***********************************************/

int
main (int argc, char **argv)
{
  int64_t desired_nedge;
  if (sizeof (int64_t) < 8) {
    fprintf (stderr, "No 64-bit support.\n");
    return EXIT_FAILURE;
  }

  if (argc > 1)
    get_options (argc, argv);

  wsf = (WSFOR *)hc_malloc(sizeof(struct _WSFOR));
  nproc = hc_nproc();

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
      fprintf (stderr, "No RMAT support.\n");
      return EXIT_FAILURE;/*
      nedge = desired_nedge;
      IJ = xmalloc_large_ext (nedge * sizeof (*IJ));
      TIME(generation_time, rmat_edgelist (IJ, nedge, SCALE, A, B, C));*/
    } else {
      TIME(generation_time, make_graph (SCALE, desired_nedge, userseed, userseed, &nedge, (packed_edge**)(&IJ)));
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

  xfree_large (IJ);

  output_results (SCALE, nvtx_scale, edgefactor, A, B, C, D,
		  generation_time, construction_time, NBFS, bfs_time, bfs_nedge);

  return EXIT_SUCCESS;
}

void
run_bfs (void)
{
  int * restrict has_adj = NULL;
  int m = 0; 
  int err = 0;
  int64_t t = 0;

  if (VERBOSE) fprintf (stderr, "Creating graph...");
  TIME(construction_time, err = create_graph_from_edgelist (IJ, nedge));
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
    has_adj = xmalloc_large (nvtx_scale * sizeof (*has_adj));

	finish {
		phaser *ph = PHASER_CREATE(SIGNAL_WAIT_MODE);
		for (int tid = 1; tid < nproc; tid++) {
			async phased IN(tid, has_adj) {
				int loop_limit, loop_chunk, lb, ub;

				loop_limit = nvtx_scale;
				loop_chunk = loop_limit / nproc;
				NEXT;
				do {
					lb = GET_CHUNK_WSFOR(wsf, loop_chunk);
					ub = lb + loop_chunk;
					ub = (ub < loop_limit) ? ub : loop_limit;
					for (int k = lb; k < ub; ++k)
						has_adj[k] = 0;
				} while (ub < loop_limit);
				NEXT;

				loop_limit = nedge;
				loop_chunk = loop_limit / nproc;
				NEXT;
				do {
					lb = GET_CHUNK_WSFOR(wsf, loop_chunk);
					ub = lb + loop_chunk;
					ub = (ub < loop_limit) ? ub : loop_limit;
					for (int k = lb; k < ub; ++k) {
						const int64_t i = get_v0_from_edge(&IJ[k]);
						const int64_t j = get_v1_from_edge(&IJ[k]);
						if (i != j)
							has_adj[i] = has_adj[j] = 1;
					}
				} while (ub < loop_limit);
				NEXT;
			}
		}
		{
				int loop_limit, loop_chunk, lb, ub;

				loop_limit = nvtx_scale;
				loop_chunk = loop_limit / nproc;
				SET_WSFOR(wsf, 0, loop_limit);
				NEXT;
				do {
					lb = GET_CHUNK_WSFOR(wsf, loop_chunk);
					ub = lb + loop_chunk;
					ub = (ub < loop_limit) ? ub : loop_limit;
					for (int k = lb; k < ub; ++k)
						has_adj[k] = 0;
				} while (ub < loop_limit);
				NEXT;

				loop_limit = nedge;
				loop_chunk = loop_limit / nproc;
				SET_WSFOR(wsf, 0, loop_limit);
				NEXT;
				do {
					lb = GET_CHUNK_WSFOR(wsf, loop_chunk);
					ub = lb + loop_chunk;
					ub = (ub < loop_limit) ? ub : loop_limit;
					for (int k = lb; k < ub; ++k) {
						const int64_t i = get_v0_from_edge(&IJ[k]);
						const int64_t j = get_v1_from_edge(&IJ[k]);
						if (i != j)
							has_adj[i] = has_adj[j] = 1;
					}
				} while (ub < loop_limit);
				NEXT;
		}
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
	exit (EXIT_FAILURE);
      }
    }

    xfree_large (has_adj);
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

  int64_t *max_bfsvtx = hc_malloc(sizeof(int64_t));

  for (m = 0; m < NBFS; ++m) {
    int64_t *bfs_tree;

    /* Re-allocate. Some systems may randomize the addres... */
    bfs_tree = xmalloc_large (nvtx_scale * sizeof (*bfs_tree));
    assert (bfs_root[m] < nvtx_scale);

    if (VERBOSE) fprintf (stderr, "Running bfs %d...", m);
    TIME(bfs_time[m], err = make_bfs_tree (bfs_tree, max_bfsvtx, bfs_root[m]));
    if (VERBOSE) fprintf (stderr, "done\n");

    if (err) {
      perror ("make_bfs_tree failed");
      abort ();
    }

    if (VERBOSE) fprintf (stderr, "Verifying bfs %d...", m);
    bfs_nedge[m] = verify_bfs_tree (bfs_tree, *max_bfsvtx, bfs_root[m], IJ, nedge);
    if (VERBOSE) fprintf (stderr, "done\n");
    if (bfs_nedge[m] < 0) {
      fprintf (stderr, "bfs %d from %" PRId64 " failed verification (%" PRId64 ")\n",
	       m, bfs_root[m], bfs_nedge[m]);
      abort ();
    }

    xfree_large (bfs_tree);
  }
  hc_free(max_bfsvtx);

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

  tm = hc_malloc (NBFS * sizeof (*tm));
  stats = hc_malloc (NSTAT * sizeof (*stats));
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

  hc_free(tm);
  hc_free(stats);
}


static void
find_nv_kernel (int tid, struct packed_edge * restrict IJ, int64_t nedge)
{
				int loop_limit, loop_chunk, lb, ub;
				int64_t gmaxvtx, tmaxvtx = -1;

				loop_limit = nedge;
				loop_chunk = loop_limit / nproc;
				if (tid == 0) SET_WSFOR(wsf, 0, loop_limit);
				NEXT;
				do {
					lb = GET_CHUNK_WSFOR(wsf, loop_chunk);
					ub = lb + loop_chunk;
					ub = (ub < loop_limit) ? ub : loop_limit;
					for (int k = lb; k < ub; ++k) {
						if (get_v0_from_edge(&IJ[k]) > tmaxvtx)
							tmaxvtx = get_v0_from_edge(&IJ[k]);
						if (get_v1_from_edge(&IJ[k]) > tmaxvtx)
							tmaxvtx = get_v1_from_edge(&IJ[k]);
					}
				} while (ub < loop_limit);
				NEXT;

				gmaxvtx = maxvtx;
				while (tmaxvtx > gmaxvtx) {
					if (hc_cas ((volatile int *)&maxvtx, gmaxvtx, tmaxvtx)) break;
					gmaxvtx = maxvtx;
				}
}

static void
find_nv (struct packed_edge * restrict IJ, int64_t nedge)
{
	maxvtx = 0;
	finish {
		phaser *ph = PHASER_CREATE(SIGNAL_WAIT_MODE);
		for (int tid = 1; tid < nproc; tid++) {
			async phased IN(tid, IJ, nedge) {
				find_nv_kernel(tid, IJ, nedge);
			}
		}
		find_nv_kernel(0, IJ, nedge);
	}
	nv = 1+maxvtx;
}

static int
alloc_graph (int64_t nedge)
{
  sz = (2*nv+2) * sizeof (int64_t);
  xoff = xmalloc_large_ext (sz);
  if (!xoff) return -1;
  return 0;
}

static void
free_graph (void)
{
  xfree_large (xadjstore);
  xfree_large (xoff);
}

#define XOFF(k) (xoff[2*(k)])
#define XENDOFF(k) (xoff[1+2*(k)])

static int64_t
prefix_sum (int64_t *buf, int tid)
{
  int nt;
  int64_t slice_begin, slice_end, t1, t2, k;

  nt = nproc;

  t1 = nv / nt;
  t2 = nv % nt;
  slice_begin = t1 * tid + (tid < t2? tid : t2);
  slice_end = t1 * (tid+1) + ((tid+1) < t2? (tid+1) : t2);

  buf[tid] = 0;
  for (k = slice_begin; k < slice_end; ++k)
    buf[tid] += XOFF(k);
  NEXT;
  if(tid == 0) {
    for (k = 1; k < nt; ++k)
      buf[k] += buf[k-1];
  }
  NEXT;
  if (tid)
    t1 = buf[tid-1];
  else
    t1 = 0;
  for (k = slice_begin; k < slice_end; ++k) {
    int64_t tmp = XOFF(k);
    XOFF(k) = t1;
    t1 += tmp;
  }
  NEXT;
  return buf[nt-1];
}

static void
setup_deg_off_kernel(int tid, struct packed_edge * restrict IJ, int64_t nedge, int64_t *buf, int err)
{
				int loop_limit, loop_chunk, lb, ub;
				int64_t accum;

				loop_limit = 2*nv+2;
				loop_chunk = loop_limit / nproc;
				if (tid == 0) SET_WSFOR(wsf, 0, loop_limit);
				NEXT;
				do {
					lb = GET_CHUNK_WSFOR(wsf, loop_chunk);
					ub = lb + loop_chunk;
					ub = (ub < loop_limit) ? ub : loop_limit;
					for (int k = lb; k < ub; ++k) {
						xoff[k] = 0;
					}
				} while (ub < loop_limit);
				NEXT;

				loop_limit = nedge;
				loop_chunk = loop_limit / nproc;
				if (tid == 0) SET_WSFOR(wsf, 0, loop_limit);
				NEXT;
				do {
					lb = GET_CHUNK_WSFOR(wsf, loop_chunk);
					ub = lb + loop_chunk;
					ub = (ub < loop_limit) ? ub : loop_limit;
					for (int k = lb; k < ub; ++k) {
						int64_t i = get_v0_from_edge(&IJ[k]);
						int64_t j = get_v1_from_edge(&IJ[k]);
						if (i != j) { /* Skip self-edges. */
							if (i >= 0)
								hc_atomic_inc((volatile int *)&XOFF(i));
							if (j >= 0)
								hc_atomic_inc((volatile int *)&XOFF(j));
						}
					}
				} while (ub < loop_limit);
				NEXT;

				loop_limit = nv;
				loop_chunk = loop_limit / nproc;
				if (tid == 0) SET_WSFOR(wsf, 0, loop_limit);
				NEXT;
				do {
					lb = GET_CHUNK_WSFOR(wsf, loop_chunk);
					ub = lb + loop_chunk;
					ub = (ub < loop_limit) ? ub : loop_limit;
					for (int k = lb; k < ub; ++k) {
						if (XOFF(k) < MINVECT_SIZE) XOFF(k) = MINVECT_SIZE;
					}
				} while (ub < loop_limit);
				NEXT;

				accum = prefix_sum (buf, tid);

				loop_limit = nv;
				loop_chunk = loop_limit / nproc;
				if (tid == 0) SET_WSFOR(wsf, 0, loop_limit);
				NEXT;
				do {
					lb = GET_CHUNK_WSFOR(wsf, loop_chunk);
					ub = lb + loop_chunk;
					ub = (ub < loop_limit) ? ub : loop_limit;
					for (int k = lb; k < ub; ++k) {
						XENDOFF(k) = XOFF(k);
					}
				} while (ub < loop_limit);
				NEXT;

				if(tid == 0) {
					XOFF(nv) = accum;
					if (!(xadjstore = xmalloc_large_ext ((XOFF(nv) + MINVECT_SIZE) * sizeof (*xadjstore))))
						err = -1;
					if (!err) {
						xadj = &xadjstore[MINVECT_SIZE]; /* Cheat and permit xadj[-1] to work. */
						for (int k = 0; k < XOFF(nv) + MINVECT_SIZE; ++k)
							xadjstore[k] = -1;
					}
				}
				NEXT;
}

static int
setup_deg_off (struct packed_edge * restrict IJ, int64_t nedge)
{
	int err = 0;
	int64_t *buf = hc_malloc (nproc * sizeof (*buf));
	if (!buf) {
		perror ("alloca for prefix-sum hosed");
		abort ();
	}
	xadj = NULL;
	finish {
		phaser *ph = PHASER_CREATE(SIGNAL_WAIT_MODE);
		for (int tid = 1; tid < nproc; tid++) {
			async phased IN(tid, IJ, nedge, buf, err) {
				setup_deg_off_kernel(tid, IJ, nedge, buf, err);
			}
		}
		setup_deg_off_kernel(0, IJ, nedge, buf, err);
	}
	return !xadj;
}

static void
scatter_edge (int64_t i, int64_t j)
{
  int64_t where;
  where = hc_fetch_and_add ((int *)&XENDOFF(i), 1);
  xadj[where] = j;
}

static int
i64cmp (const void *a, const void *b)
{
  int64_t ia = *(int64_t*)a;
  int64_t ib = *(int64_t*)b;
  if (ia < ib) return -1;
  if (ia > ib) return 1;
  return 0;
}

static void
pack_vtx_edges (int64_t i)
{
  int64_t kcur, k;
  if (XOFF(i)+1 >= XENDOFF(i)) return;
  qsort (&xadj[XOFF(i)], XENDOFF(i)-XOFF(i), sizeof(*xadj), i64cmp);
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
gather_edges_kernel (int tid, struct packed_edge * restrict IJ, int64_t nedge)
{
				int loop_limit, loop_chunk, lb, ub;

				loop_limit = nedge;
				loop_chunk = loop_limit / nproc;
				if (tid == 0) SET_WSFOR(wsf, 0, loop_limit);
				NEXT;
				do {
					lb = GET_CHUNK_WSFOR(wsf, loop_chunk);
					ub = lb + loop_chunk;
					ub = (ub < loop_limit) ? ub : loop_limit;
					for (int k = lb; k < ub; ++k) {
						int64_t i = get_v0_from_edge(&IJ[k]);
						int64_t j = get_v1_from_edge(&IJ[k]);
						if (i >= 0 && j >= 0 && i != j) {
							scatter_edge (i, j);
							scatter_edge (j, i);
						}
					}
				} while (ub < loop_limit);
				NEXT;

				/* pack_edges */
				loop_limit = nv;
				loop_chunk = loop_limit / nproc;
				if (tid == 0) SET_WSFOR(wsf, 0, loop_limit);
				NEXT;
				do {
					lb = GET_CHUNK_WSFOR(wsf, loop_chunk);
					ub = lb + loop_chunk;
					ub = (ub < loop_limit) ? ub : loop_limit;
					for (int k = lb; k < ub; ++k) {
						pack_vtx_edges (k);
					}
				} while (ub < loop_limit);
				NEXT;
}

static void
gather_edges (struct packed_edge * restrict IJ, int64_t nedge)
{
	finish {
		phaser *ph = PHASER_CREATE(SIGNAL_WAIT_MODE);
		for (int tid = 1; tid < nproc; tid++) {
			async phased IN(tid, IJ, nedge) {
				gather_edges_kernel(tid, IJ, nedge);
			}
		}
		gather_edges_kernel(0, IJ, nedge);
	}
}

static int 
create_graph_from_edgelist (struct packed_edge *IJ, int64_t nedge)
{
  printf("Running HC on %d threads\n", nproc);
  find_nv (IJ, nedge);
  if (alloc_graph (nedge)) return -1;
  if (0 != setup_deg_off (IJ, nedge)) {
    xfree_large (xoff);
    return -1;
  }
  gather_edges (IJ, nedge);
  return 0;
}

typedef struct sharedArgs {
	volatile int64_t k1, k2;
} sharedArgs;

static void
make_bfs_tree_kernel (int tid, sharedArgs * shared, int64_t * restrict bfs_tree, int64_t srcvtx, int64_t * restrict vlist) 
{
				int loop_limit, loop_chunk, lb, ub;
				int64_t nbuf[THREAD_BUF_LEN];

				loop_limit = nv;
				loop_chunk = loop_limit / nproc;
				if (tid == 0) SET_WSFOR(wsf, 0, loop_limit);
				NEXT;
				do {
					lb = GET_CHUNK_WSFOR(wsf, loop_chunk);
					ub = lb + loop_chunk;
					ub = (ub < loop_limit) ? ub : loop_limit;
					for (int k = lb; k < ub; ++k) {
						bfs_tree[k] = -1;
					}
				} while (ub < loop_limit);
				NEXT;
				if (tid == 0) bfs_tree[srcvtx] = srcvtx;
				NEXT;

				while (shared->k1 != shared->k2) {
					const int64_t oldk2 = shared->k2;
					int64_t kbuf = 0;
					NEXT;
					loop_limit = oldk2;
					loop_chunk = CHUNK_SIZE;
					if (tid == 0) SET_WSFOR(wsf, shared->k1, oldk2);
					NEXT;
					do {
						lb = GET_CHUNK_WSFOR(wsf, loop_chunk);
						ub = lb + loop_chunk;
						ub = (ub < loop_limit) ? ub : loop_limit;
						for (int k = lb; k < ub; ++k) {
							const int64_t v = vlist[k];
							const int64_t veo = XENDOFF(v);
							int64_t vo;
							for (vo = XOFF(v); vo < veo; ++vo) {
            					const int64_t j = xadj[vo];
            					if (bfs_tree[j] == -1) {
              						if (hc_cas ((volatile int *)&bfs_tree[j], -1, v)) {
                						if (kbuf < THREAD_BUF_LEN) {
                  							nbuf[kbuf++] = j;
                						} else {
                  							int64_t voff = hc_fetch_and_add ((int *)&shared->k2, THREAD_BUF_LEN), vk;
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
					} while (ub < loop_limit);
					NEXT;

      				if (kbuf) {
        				int64_t voff = hc_fetch_and_add ((int *)&shared->k2, kbuf), vk;
        				assert (voff + kbuf <= nv);
        				for (vk = 0; vk < kbuf; ++vk)
          					vlist[voff + vk] = nbuf[vk];
      				}
      				if(tid == 0)
        				shared->k1 = oldk2;
      				NEXT;
    			}
}

static int
make_bfs_tree (int64_t *bfs_tree_out, int64_t *max_vtx_out,
               int64_t srcvtx)
{
  int64_t * restrict bfs_tree = bfs_tree_out;
  int err = 0;

  int64_t * restrict vlist = NULL;
  sharedArgs * shared = (sharedArgs *)hc_malloc(sizeof(sharedArgs));

  *max_vtx_out = maxvtx;

  vlist = xmalloc_large (nv * sizeof (*vlist));
  if (!vlist) return -1;

  vlist[0] = srcvtx;
  shared->k1 = 0; shared->k2 = 1;

	finish {
		phaser *ph = PHASER_CREATE(SIGNAL_WAIT_MODE);
		for (int tid = 1; tid < nproc; tid++) {
			async phased IN(tid, shared, bfs_tree, srcvtx, vlist) {
				make_bfs_tree_kernel(tid, shared, bfs_tree, srcvtx, vlist);
			}
		}
		make_bfs_tree_kernel(0, shared, bfs_tree, srcvtx, vlist);
	}

  hc_free(shared);
  xfree_large (vlist);

  return err;
}

static void
compute_levels_kernel (int64_t lb, int64_t ub, int64_t * level,
				int64_t nv, int64_t * restrict bfs_tree, int64_t root, volatile int * err)
{
  int terr;
  int64_t k;
  for (k = lb; k < ub; k++) {
	if (level[k] >= 0) continue;
	terr = *err;
	if (terr) break;
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
	if (terr) { *err = terr; hc_mfence(); }
  }
}

static int
compute_levels (int64_t * level,
				int64_t nv, int64_t * restrict bfs_tree, int64_t root, int64_t p)
{
  volatile int * err = (volatile int *)hc_malloc(sizeof(int));
  *err = 0;

  finish {
	int64_t i;
	int64_t c = nv / p;
	for (i = 0; i < (p - 1); i++) {
		int64_t lb = i * c;
		int64_t ub = lb + c;
		async IN(lb, ub, level) {
			int64_t k;
			for (k = lb; k < ub; k++) {
				level[k] = -1;
			}
		}
	}
	int64_t k;
	int64_t lb = i * c;
	for (k = lb; k < nv; k++) {
		level[k] = -1;
	}
  }
  level[root] = 0;

  finish {
	int64_t i;
	int64_t c = nv / p;
	for (i = 0; i < (p - 1); i++) {
		int64_t lb = i * c;
		int64_t ub = lb + c;
		async IN(lb, ub, level, nv, bfs_tree, root, err) {
			compute_levels_kernel(lb, ub, level, nv, bfs_tree, root, err);
		}
	}
	int64_t lb = i * c;
	compute_levels_kernel(lb, nv, level, nv, bfs_tree, root, err);
  }

  int ret = *err;
  hc_free((void *)err);
  return ret;
}

static void
verify_kernel (int id, int64_t lb, int64_t ub, int64_t * restrict bfs_tree, int64_t max_bfsvtx,
				int64_t * IJ, int64_t * restrict seen_edge, int64_t * restrict level, 
				volatile int * nedgeArray, volatile int * err)
{
  int terr;
  int64_t k;
  int nedge_traversed = 0;

  for (k = lb; k < ub; k++) {
	int64_t i = IJ[2*k];
	int64_t j = IJ[2*k+1];
	int64_t lvldiff;
	terr = *err;
	if (terr) break;

	if (i < 0 || j < 0) continue;
	if (i > max_bfsvtx && j <= max_bfsvtx) terr = -10;
	if (j > max_bfsvtx && i <= max_bfsvtx) terr = -11;
	if (terr) { *err = terr; hc_mfence(); }
	if (terr || i > max_bfsvtx /* both i & j are on the same side of max_bfsvtx */)
	  continue;

	/* All neighbors must be in the tree. */
	if ((int)bfs_tree[i] >= 0 && (int)bfs_tree[j] < 0) terr = -12;
	if ((int)bfs_tree[j] >= 0 && (int)bfs_tree[i] < 0) terr = -13;
	if (terr) { *err = terr; hc_mfence(); }
	if (terr || (int)bfs_tree[i] < 0 /* both i & j have the same sign */)
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
	if (terr) { *err = terr; hc_mfence(); }
  }
  nedgeArray[id] = nedge_traversed;
}

static int64_t
verify_bfs_tree (int64_t *bfs_tree_in, int64_t max_bfsvtx,
				 int64_t root,
				 struct packed_edge * IJ_in, int64_t nedge)
{
  int64_t * restrict bfs_tree = bfs_tree_in;
  struct packed_edge * restrict IJ = IJ_in;

  int cerr, nedge_traversed;
  int64_t * restrict seen_edge, * restrict level;

  int64_t nv = max_bfsvtx+1;

  volatile int * err = (volatile int *)hc_malloc(sizeof(int));
  *err = 0;

  int p = hc_nproc();
  volatile int * nedgeArray = (int *)hc_malloc(p * sizeof(int));

  /*
	This code is horrifically contorted because many compilers
	complain about continue, return, etc. in parallel sections.
  */

  if (root > max_bfsvtx || bfs_tree[root] != root)
	return -999;

  cerr = 0;
  nedge_traversed = 0;
  seen_edge = xmalloc_large (2 * (nv) * sizeof (*seen_edge));
  level = &seen_edge[nv];

  cerr = compute_levels (level, nv, bfs_tree, root, p);

  if (cerr) goto done;

  finish {
	int64_t i;
	int64_t c = nv / p;
	for (i = 0; i < (p - 1); i++) {
		int64_t lb = i * c;
		int64_t ub = lb + c;
		async IN(lb, ub, seen_edge) {
			int64_t k;
			for (k = lb; k < ub; k++) {
				seen_edge[k] = 0;
			}
		}
	}
	int64_t k;
	int64_t lb = i * c;
	for (k = lb; k < nv; k++) {
		seen_edge[k] = 0;
	}
  }

  finish {
	int64_t i;
	int64_t c = nedge / p;
	for (i = 0; i < (p - 1); i++) {
		int64_t lb = i * c;
		int64_t ub = lb + c;
		async IN(i, lb, ub, bfs_tree, max_bfsvtx, IJ, seen_edge, level, nedgeArray, err) {
			verify_kernel(i, lb, ub, bfs_tree, max_bfsvtx, (int64_t *)IJ, seen_edge, level, nedgeArray, err);
		}
	}
	int64_t lb = i * c;
	verify_kernel(i, lb, nedge, bfs_tree, max_bfsvtx, (int64_t *)IJ, seen_edge, level, nedgeArray, err);
  }

  if (*err) goto done;

  int k;
  for (k = 0; k < p; k++)
	nedge_traversed += nedgeArray[k];

  hc_free((void *)nedgeArray);

  finish {
	int64_t i;
	int64_t c = nv / p;
	for (i = 0; i < (p - 1); i++) {
		int64_t lb = i * c;
		int64_t ub = lb + c;
		async IN(lb, ub, bfs_tree, seen_edge, root, err) {
			int64_t k;
			for (k = lb; k < ub; k++) {
				int terr = *err;
				if (terr) break;
				if (k != root) {
					if (bfs_tree[k] >= 0 && !seen_edge[k])
						terr = -15;
					if (bfs_tree[k] == k)
						terr = -16;
					if (terr) { *err = terr; hc_mfence(); }
				}
			}
		}
	}
	int64_t k;
	int64_t lb = i * c;
	for (k = lb; k < nv; k++) {
		int terr = *err;
		if (terr) break;
		if (k != root) {
			if (bfs_tree[k] >= 0 && !seen_edge[k])
				terr = -15;
			if (bfs_tree[k] == k)
				terr = -16;
			if (terr) { *err = terr; hc_mfence(); }
		}
	}
  }

 done:

  xfree_large (seen_edge);
  int ret = *err; 
  hc_free((void *)err);
  if (ret) return ret; 
  return nedge_traversed;
}

static void
destroy_graph (void)
{
  free_graph ();
}

/* Generate a range of edges (from start_edge to end_edge of the total graph),
 * writing into elements [0, end_edge - start_edge) of the edges array.  This
 * code is parallel on OpenMP and XMT; it must be used with
 * separately-implemented SPMD parallelism for MPI. */
static void generate_kronecker_range(
       const uint_fast32_t seed[5] /* All values in [0, 2^31 - 1), not all zero */,
       int logN /* In base 2 */,
       int64_t start_edge, int64_t end_edge,
       packed_edge* edges) {
  mrg_state state;
  int64_t nverts = (int64_t)1 << logN;

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

  int64_t p = hc_nproc();
  finish {
	int64_t i;
	int64_t n = end_edge - start_edge;
	int64_t c = n / p;
	for (i = 0; i < (p - 1); i++) {
		int64_t lb = start_edge + (i * c);
		int64_t ub = lb + c;
		async IN(lb, ub, state, nverts, logN, edges, start_edge, val0, val1) {
			int64_t ei;
			for (ei = lb; ei < ub; ei++) {
				mrg_state new_state = state;
				mrg_skip(&new_state, 0, ei, 0);
				make_one_edge(nverts, 0, logN, &new_state, edges + (ei - start_edge), val0, val1);
			}
		}
	}
	int64_t ei;
	int64_t lb = start_edge + (i * c);
	for (ei = lb; ei < end_edge; ei++) {
		mrg_state new_state = state;
		mrg_skip(&new_state, 0, ei, 0);
		make_one_edge(nverts, 0, logN, &new_state, edges + (ei - start_edge), val0, val1);
	}
  }
}

/* Simplified interface to build graphs with scrambled vertices. */
static void make_graph(int log_numverts, int64_t M, uint64_t userseed1, uint64_t userseed2, int64_t* nedges_ptr_in, packed_edge** result_ptr_in) {
  /* Add restrict to input pointers. */
  int64_t* restrict nedges_ptr = nedges_ptr_in;
  packed_edge* restrict* restrict result_ptr = result_ptr_in;

  /* Spread the two 64-bit numbers into five nonzero values in the correct
   * range. */
  uint_fast32_t * seed = (uint_fast32_t *)hc_malloc(sizeof(uint_fast32_t) * 5);
  make_mrg_seed(userseed1, userseed2, seed);

  *nedges_ptr = M;
  packed_edge* edges = (packed_edge*)xmalloc(M * sizeof(packed_edge));
  *result_ptr = edges;

  /* In OpenMP and XMT versions, the inner loop in generate_kronecker_range is
   * parallel.  */
  generate_kronecker_range(seed, log_numverts, 0, M, edges);
  hc_free(seed);
}

