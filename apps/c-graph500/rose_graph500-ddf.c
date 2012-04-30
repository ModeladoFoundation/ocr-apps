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
#include "hc.h"
#include "hc.h" 
void *dummyDataVoidPtr;
double *timePtr;
static int64_t nvtx_scale;
static int64_t bfs_root[64UL];
static double generation_time;
static double construction_time;
static double bfs_time[64UL];
static int64_t bfs_nedge[64UL];
static packed_edge *__restrict__ IJ;
static int64_t nedge;
#define MINVECT_SIZE 2
#define XOFF(k) (xoff[2*(k)])
#define XENDOFF(k) (xoff[1+2*(k)])
#define THREAD_BUF_LEN 16384
#define CHUNK_SIZE 256
static int64_t maxvtx;
static int64_t nv;
static int64_t sz;
/* Length 2*nv+2 */
static int64_t *__restrict__ xoff;
/* Length MINVECT_SIZE + (xoff[nv] == nedge) */
static int64_t *__restrict__ xadjstore;
static int64_t *__restrict__ xadj;
static void make_graph(int log_numverts,int64_t M,uint64_t userseed1,uint64_t userseed2,int64_t *nedges_ptr_in,packed_edge **result_ptr_in,DDF_t *make_graph_ddf);
static void create_graph_from_edgelist(struct packed_edge *IJ,int64_t nedge,DDF_t *create_graph_ddf);
static void bfs_iter(int m,DDF_t *bfs_iter_ddf);
static void make_bfs_tree(int64_t *bfs_tree_out,int64_t srcvtx,DDF_t *bfs_tree_ddf);
static void verify_bfs_tree(int64_t *bfs_tree_in,int64_t max_bfsvtx,int64_t root,struct packed_edge *IJ_in,int64_t nedge,volatile int *err,DDF_t *verify_bfs_ddf);
static void destroy_graph();
static void run_bfs(DDF_t *run_bfs_ddf);
static void output_results(const int64_t SCALE,int64_t nvtx_scale,int64_t edgefactor,const double A,const double B,const double C,const double D,const double generation_time,const double construction_time,const int NBFS,const double *bfs_time,const int64_t *bfs_nedge);
typedef struct __hc_main_frame_t__ {
hc_frameHeader header;
hc_finishState fscache[1];
struct __hc_main_scope0_frame_t__ {
int argc;
char **argv;
int __rtval__;}scope0;
struct __hc_main_scope1_frame_t__ {
int64_t desired_nedge;
DDF_t *make_graph_ddf;}scope1;
struct __hc_main_scope2_frame_t__ {
int fd;
ssize_t sz;}scope2;}__hc_main_frame_t__;
typedef struct __hc__main_async1__frame_t__ {
hc_frameHeader header;
struct __hc__main_async1__scope0_frame_t__ {
packed_edge *__restrict__ IJ;}scope0;
struct __hc__main_async1__scope1_frame_t__ {
DDF_t *run_bfs_ddf;}scope1;}__hc__main_async1__frame_t__;
typedef struct __hc__main_async2__frame_t__ {
hc_frameHeader header;
struct __hc__main_async2__scope0_frame_t__ {
packed_edge *__restrict__ IJ;}scope0;}__hc__main_async2__frame_t__;
void __hc_main__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _main_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _main_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);

int main(int argc,char **argv)
{
						hc_options options;
						hc_context ctxt;
						read_options(&options, &argc, argv);
						
						hc_rt0(&options, &ctxt, "main");
					
  hc_workerState *ws = current_ws();
  ws -> costatus =  HC_FUNC_COMPLETE;
  struct __hc_main_frame_t__ *__hc_frame__ = (struct __hc_main_frame_t__ *)(hc_init_frame_ws(ws,sizeof(struct __hc_main_frame_t__ ),__hc_main__));
  ((hc_frameHeader *)__hc_frame__) -> rtvalp = &__hc_frame__ -> scope0.__rtval__;
  __hc_frame__ -> scope0.argc = argc;
  __hc_frame__ -> scope0.argv = argv;
					hc_rt_roll(&ctxt, (hc_frameHeader*)__hc_frame__);
				
  int __rt_value__ = __hc_frame__ -> scope0.__rtval__;
					HC_FREE(ws, __hc_frame__);
					hc_rt_cleanup(&ctxt);
				
  return __rt_value__;
}

void __hc_main__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int argc;
  char **argv;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_finish2;
    case 3:
    goto __post_func3;
  }
  argc = ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope0.argc;
  argv = ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope0.argv;
  hc_startFinish(ws);
{
    int64_t desired_nedge;
    if (0) {
      fprintf(__stderrp,"No 64-bit support.\n");
      ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope0.__rtval__ = 1;
      ws -> costatus =  HC_FUNC_COMPLETE;
      hc_setDone(ws);
      return ;
    }
    if (argc > 1) 
      get_options(argc,argv);
    nvtx_scale = (((int64_t )1) << SCALE);
    init_random();
    desired_nedge = (nvtx_scale * edgefactor);
/* Catch a few possible overflows. */
    (__builtin_expect((!(desired_nedge >= nvtx_scale)),0) != 0L)?__assert_rtn(__func__,"/Users/dm14/Documents/Habanero/hc-trunk/examples/graph500-2.1.4/graph500-ddf.hc",101,"desired_nedge >= nvtx_scale") : ((void )0);
    (__builtin_expect((!(desired_nedge >= edgefactor)),0) != 0L)?__assert_rtn(__func__,"/Users/dm14/Documents/Habanero/hc-trunk/examples/graph500-2.1.4/graph500-ddf.hc",102,"desired_nedge >= edgefactor") : ((void )0);
    dummyDataVoidPtr = hc_mm_malloc(((hc_workerState *)(pthread_getspecific(wskey))),(sizeof(char )));
    timePtr = (hc_mm_malloc(((hc_workerState *)(pthread_getspecific(wskey))),(sizeof(double ))));
    DDF_t *make_graph_ddf = __ddfCreate();
/*
    If running the benchmark under an architecture simulator, replace
    the following if () {} else {} with a statement pointing IJ
    to wherever the edge list is mapped into the simulator's memory.
  */
    if (!(dumpname != 0)) {
      if (VERBOSE != 0) 
        fprintf(__stderrp,"Generating edge list...");
      if (use_RMAT != 0) {
        fprintf(__stderrp,"No RMAT support.\n");
/*
		nedge = desired_nedge;
		IJ = xmalloc_large_ext (nedge * sizeof (*IJ));
		TIME(generation_time, rmat_edgelist (IJ, nedge, SCALE, A, B, C));*/
        ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope0.__rtval__ = 1;
        ws -> costatus =  HC_FUNC_COMPLETE;
        hc_setDone(ws);
        return ;
      }
      else {
/*TIME(generation_time, make_graph (SCALE, desired_nedge, userseed, userseed, &nedge, (packed_edge**)(&IJ), make_graph_ddf));*/
/* Start Time: generation_time */
        tic();
        ws -> costatus =  HC_FUNC_COMPLETE;
        __hc_frame__ -> pc = 3;
        ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope1.desired_nedge = desired_nedge;
        ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope1.make_graph_ddf = make_graph_ddf;
        ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope0.argc = argc;
        ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope0.argv = argv;
        make_graph(SCALE,desired_nedge,userseed,userseed,&nedge,((packed_edge **)(&IJ)),make_graph_ddf);
        if (ws -> costatus !=  HC_FUNC_COMPLETE) 
          return ;
        if (0) {
          __post_func3:
          hc_free_unwind_frame(ws);
          __hc_frame__ = ws -> frame;
          desired_nedge = ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope1.desired_nedge;
          make_graph_ddf = ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope1.make_graph_ddf;
          argc = ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope0.argc;
          argv = ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope0.argv;
        }
      }
    }
    else {
      int fd;
      ssize_t sz;
      if ((fd = open(dumpname,0)) < 0) {
        perror("Cannot open input graph file");
        ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope0.__rtval__ = 1;
        ws -> costatus =  HC_FUNC_COMPLETE;
        hc_setDone(ws);
        return ;
      }
      sz = (nedge * sizeof(( *IJ)));
      if (sz != read(fd,IJ,sz)) {
        perror("Error reading input graph file");
        ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope0.__rtval__ = 1;
        ws -> costatus =  HC_FUNC_COMPLETE;
        hc_setDone(ws);
        return ;
      }
      close(fd);
      __ddfPut(make_graph_ddf,dummyDataVoidPtr);
    }
    struct __hc__main_async1__frame_t__ *__hc__main_async1__frame__ = (struct __hc__main_async1__frame_t__ *)(hc_init_frame(sizeof(struct __hc__main_async1__frame_t__ ),_main_async1_));
    p_await_list_t __hc__main_async1_awaitList = hc_malloc(sizeof(await_list_t ));
    unsigned int __hc__main_async1_awaitList_size = 0 + 1;
    __hc__main_async1_awaitList -> array = hc_malloc((1 + __hc__main_async1_awaitList_size) * sizeof(p_DDF_t ));
    __hc__main_async1_awaitList -> array[__hc__main_async1_awaitList_size] = NULL;
    unsigned int __hc__main_async1_awaitList_curr_index = 0;
    __hc__main_async1_awaitList -> array[__hc__main_async1_awaitList_curr_index++] = make_graph_ddf;
    __hc__main_async1_awaitList -> waitingFrontier = &__hc__main_async1_awaitList -> array[0];
    __hc__main_async1__frame__ -> header.awaitList = __hc__main_async1_awaitList;
    __hc__main_async1__frame__ -> scope0.IJ = ((packed_edge *__restrict__ )IJ);
    ((hc_frameHeader *)__hc__main_async1__frame__) -> sched_policy = hc_sched_policy(ws);
    increment_counter();
    if (((hc_frameHeader *)__hc__main_async1__frame__) -> sched_policy ==  HELP_FIRST) {
      if (__iterateDDFWaitingFrontier(&__hc__main_async1__frame__ -> header)) 
        deque_push_default(__hc__main_async1__frame__);
      else 
        ws -> costatus =  HC_FINISH_SUSPENDED;
    }
    else {
      ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope1.desired_nedge = desired_nedge;
      ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope1.make_graph_ddf = make_graph_ddf;
      ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope0.argc = argc;
      ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope0.argv = argv;
      __hc_frame__ -> pc = 1;
      deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_main__, and execute async: %p in _main_async1_\n", __hc_frame__, __hc_frame__->pc, __hc__main_async1__frame__);
      if (__iterateDDFWaitingFrontier(&__hc__main_async1__frame__ -> header)) 
        _main_async1_(ws,((hc_frameHeader *)__hc__main_async1__frame__),0);
      else 
        ws -> costatus =  HC_FINISH_SUSPENDED;
      if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
        return ;
      }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
      if (0) {
        __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_main__\n", printEntry(__hc_frame__, buffer));
        desired_nedge = ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope1.desired_nedge;
        make_graph_ddf = ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope1.make_graph_ddf;
        argc = ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope0.argc;
        argv = ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope0.argv;
      }
    }
  }
  __hc_frame__ -> pc = 2;
  ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope0.argc = argc;
  ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope0.argv = argv;
  if (hc_stopFinish(ws)) 
    return ;
  if (0) {
    __post_finish2:
    argc = ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope0.argc;
    argv = ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope0.argv;
  }
  ((struct __hc_main_frame_t__ *)__hc_frame__) -> scope0.__rtval__ = 0;
  ws -> costatus =  HC_FUNC_COMPLETE;
  hc_setDone(ws);
  return ;
}

void _main_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  packed_edge *__restrict__ IJ;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  IJ = ((struct __hc__main_async2__frame_t__ *)__hc_frame__) -> scope0.IJ;
  xfree_large(IJ);
  output_results(SCALE,nvtx_scale,edgefactor,A,B,C,D,generation_time,construction_time,NBFS,bfs_time,bfs_nedge);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _main_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  packed_edge *__restrict__ IJ;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_func2;
  }
  IJ = ((struct __hc__main_async1__frame_t__ *)__hc_frame__) -> scope0.IJ;
  if (VERBOSE != 0) 
    fprintf(__stderrp," done.\n");
  DDF_t *run_bfs_ddf = __ddfCreate();
  ws -> costatus =  HC_FUNC_COMPLETE;
  __hc_frame__ -> pc = 2;
  ((struct __hc__main_async1__frame_t__ *)__hc_frame__) -> scope1.run_bfs_ddf = run_bfs_ddf;
  ((struct __hc__main_async1__frame_t__ *)__hc_frame__) -> scope0.IJ = IJ;
  run_bfs(run_bfs_ddf);
  if (ws -> costatus !=  HC_FUNC_COMPLETE) 
    return ;
  if (0) {
    __post_func2:
    hc_free_unwind_frame(ws);
    __hc_frame__ = ws -> frame;
    run_bfs_ddf = ((struct __hc__main_async1__frame_t__ *)__hc_frame__) -> scope1.run_bfs_ddf;
    IJ = ((struct __hc__main_async1__frame_t__ *)__hc_frame__) -> scope0.IJ;
  }
/* Write back value for INOUT/OUT arguments */
  struct __hc__main_async2__frame_t__ *__hc__main_async2__frame__ = (struct __hc__main_async2__frame_t__ *)(hc_init_frame(sizeof(struct __hc__main_async2__frame_t__ ),_main_async2_));
  p_await_list_t __hc__main_async2_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__main_async2_awaitList_size = 0 + 1;
  __hc__main_async2_awaitList -> array = hc_malloc((1 + __hc__main_async2_awaitList_size) * sizeof(p_DDF_t ));
  __hc__main_async2_awaitList -> array[__hc__main_async2_awaitList_size] = NULL;
  unsigned int __hc__main_async2_awaitList_curr_index = 0;
  __hc__main_async2_awaitList -> array[__hc__main_async2_awaitList_curr_index++] = run_bfs_ddf;
  __hc__main_async2_awaitList -> waitingFrontier = &__hc__main_async2_awaitList -> array[0];
  __hc__main_async2__frame__ -> header.awaitList = __hc__main_async2_awaitList;
  __hc__main_async2__frame__ -> scope0.IJ = ((packed_edge *__restrict__ )IJ);
  ((hc_frameHeader *)__hc__main_async2__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__main_async2__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__main_async2__frame__ -> header)) 
      deque_push_default(__hc__main_async2__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc__main_async1__frame_t__ *)__hc_frame__) -> scope1.run_bfs_ddf = run_bfs_ddf;
    ((struct __hc__main_async1__frame_t__ *)__hc_frame__) -> scope0.IJ = IJ;
    __hc_frame__ -> pc = 1;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _main_async1_, and execute async: %p in _main_async2_\n", __hc_frame__, __hc_frame__->pc, __hc__main_async2__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__main_async2__frame__ -> header)) 
      _main_async2_(ws,((hc_frameHeader *)__hc__main_async2__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _main_async1_\n", printEntry(__hc_frame__, buffer));
      run_bfs_ddf = ((struct __hc__main_async1__frame_t__ *)__hc_frame__) -> scope1.run_bfs_ddf;
      IJ = ((struct __hc__main_async1__frame_t__ *)__hc_frame__) -> scope0.IJ;
    }
  }
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}
typedef struct __hc_run_bfs_frame_t__ {
hc_frameHeader header;
struct __hc_run_bfs_scope0_frame_t__ {
DDF_t *run_bfs_ddf;}scope0;
struct __hc_run_bfs_scope1_frame_t__ {
int *__restrict__ has_adj;
int m;
int64_t t;
DDF_t *create_graph_ddf;}scope1;}__hc_run_bfs_frame_t__;
typedef struct __hc__run_bfs_async1__frame_t__ {
hc_frameHeader header;
struct __hc__run_bfs_async1__scope0_frame_t__ {
int *__restrict__ has_adj;
int m;
int64_t t;
DDF_t *run_bfs_ddf;}scope0;
struct __hc__run_bfs_async1__scope1_frame_t__ {
int p;
DDF_t *bfs_init_ddf;}scope1;
struct __hc__run_bfs_async1__scope2_frame_t__ {
p_DDF_list_t listOfAdjClear;
int64_t i;
int64_t c;
int64_t k;
int64_t lb;}scope2;
struct __hc__run_bfs_async1__scope3_frame_t__ {
int64_t lb;
int64_t ub;
DDF_t *d;}scope3;
struct __hc__run_bfs_async1__scope4_frame_t__ {
int fd;
ssize_t sz;}scope4;}__hc__run_bfs_async1__frame_t__;
typedef struct __hc__run_bfs_async2__frame_t__ {
hc_frameHeader header;
struct __hc__run_bfs_async2__scope0_frame_t__ {
int64_t lb;
int64_t ub;
int *__restrict__ has_adj;
DDF_t *d;}scope0;}__hc__run_bfs_async2__frame_t__;
typedef struct __hc__run_bfs_async3__frame_t__ {
hc_frameHeader header;
struct __hc__run_bfs_async3__scope0_frame_t__ {
int64_t nedge;
int p;
int *__restrict__ has_adj;
int m;
int64_t t;
packed_edge *__restrict__ IJ;
DDF_t *bfs_init_ddf;}scope0;
struct __hc__run_bfs_async3__scope1_frame_t__ {
p_DDF_list_t listOfAdjInit;
int64_t i;
int64_t c;
int64_t k;
int64_t lb;}scope1;
struct __hc__run_bfs_async3__scope2_frame_t__ {
int64_t lb;
int64_t ub;
DDF_t *d;}scope2;
struct __hc__run_bfs_async3__scope3_frame_t__ {
int64_t i;
int64_t j;}scope3;}__hc__run_bfs_async3__frame_t__;
typedef struct __hc__run_bfs_async4__frame_t__ {
hc_frameHeader header;
struct __hc__run_bfs_async4__scope0_frame_t__ {
int64_t lb;
int64_t ub;
int *__restrict__ has_adj;
packed_edge *__restrict__ IJ;
DDF_t *d;}scope0;}__hc__run_bfs_async4__frame_t__;
typedef struct __hc__run_bfs_async5__frame_t__ {
hc_frameHeader header;
struct __hc__run_bfs_async5__scope0_frame_t__ {
int *__restrict__ has_adj;
int m;
int64_t t;
DDF_t *bfs_init_ddf;}scope0;}__hc__run_bfs_async5__frame_t__;
typedef struct __hc__run_bfs_async6__frame_t__ {
hc_frameHeader header;
struct __hc__run_bfs_async6__scope0_frame_t__ {
DDF_t *run_bfs_ddf;}scope0;
struct __hc__run_bfs_async6__scope1_frame_t__ {
DDF_t *bfs_iter_ddf;}scope1;}__hc__run_bfs_async6__frame_t__;
typedef struct __hc__run_bfs_async7__frame_t__ {
hc_frameHeader header;
struct __hc__run_bfs_async7__scope0_frame_t__ {
DDF_t *run_bfs_ddf;}scope0;}__hc__run_bfs_async7__frame_t__;
void __hc_run_bfs__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _run_bfs_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _run_bfs_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _run_bfs_async3_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _run_bfs_async4_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _run_bfs_async5_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _run_bfs_async6_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _run_bfs_async7_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);

static void run_bfs(DDF_t *run_bfs_ddf)
{
  hc_workerState *ws = current_ws();
  ws -> costatus =  HC_FUNC_COMPLETE;
  struct __hc_run_bfs_frame_t__ *__hc_frame__ = (struct __hc_run_bfs_frame_t__ *)(hc_init_frame_ws(ws,sizeof(struct __hc_run_bfs_frame_t__ ),__hc_run_bfs__));
  __hc_frame__ -> scope0.run_bfs_ddf = run_bfs_ddf;
			LOG_INFO(ws, "execute frame: %p, parent: %p of function %s\n", __hc_frame__,((hc_frameHeader*) __hc_frame__)->parent, "__hc_run_bfs__" );
  __hc_run_bfs__(ws,((hc_frameHeader *)__hc_frame__),0);
  if (ws -> costatus ==  HC_FUNC_COMPLETE) 
    hc_free_unwind_frame(ws);
  return ;
}

void __hc_run_bfs__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  DDF_t *run_bfs_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_func2;
  }
  run_bfs_ddf = ((struct __hc_run_bfs_frame_t__ *)__hc_frame__) -> scope0.run_bfs_ddf;
  int *__restrict__ has_adj = (int *)((void *)0);
  int m = 0;
  int64_t t = 0;
  if (VERBOSE != 0) 
    fprintf(__stderrp,"Creating graph...");
/*TIME(construction_time, err = create_graph_from_edgelist (IJ, nedge, create_graph_ddf));*/
  DDF_t *create_graph_ddf = __ddfCreate();
/* Start Time: construction_time */
  tic();
  ws -> costatus =  HC_FUNC_COMPLETE;
  __hc_frame__ -> pc = 2;
  ((struct __hc_run_bfs_frame_t__ *)__hc_frame__) -> scope1.has_adj = has_adj;
  ((struct __hc_run_bfs_frame_t__ *)__hc_frame__) -> scope1.m = m;
  ((struct __hc_run_bfs_frame_t__ *)__hc_frame__) -> scope1.t = t;
  ((struct __hc_run_bfs_frame_t__ *)__hc_frame__) -> scope1.create_graph_ddf = create_graph_ddf;
  ((struct __hc_run_bfs_frame_t__ *)__hc_frame__) -> scope0.run_bfs_ddf = run_bfs_ddf;
  create_graph_from_edgelist(IJ,nedge,create_graph_ddf);
  if (ws -> costatus !=  HC_FUNC_COMPLETE) 
    return ;
  if (0) {
    __post_func2:
    hc_free_unwind_frame(ws);
    __hc_frame__ = ws -> frame;
    has_adj = ((struct __hc_run_bfs_frame_t__ *)__hc_frame__) -> scope1.has_adj;
    m = ((struct __hc_run_bfs_frame_t__ *)__hc_frame__) -> scope1.m;
    t = ((struct __hc_run_bfs_frame_t__ *)__hc_frame__) -> scope1.t;
    create_graph_ddf = ((struct __hc_run_bfs_frame_t__ *)__hc_frame__) -> scope1.create_graph_ddf;
    run_bfs_ddf = ((struct __hc_run_bfs_frame_t__ *)__hc_frame__) -> scope0.run_bfs_ddf;
  }
  struct __hc__run_bfs_async1__frame_t__ *__hc__run_bfs_async1__frame__ = (struct __hc__run_bfs_async1__frame_t__ *)(hc_init_frame(sizeof(struct __hc__run_bfs_async1__frame_t__ ),_run_bfs_async1_));
  p_await_list_t __hc__run_bfs_async1_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__run_bfs_async1_awaitList_size = 0 + 1;
  __hc__run_bfs_async1_awaitList -> array = hc_malloc((1 + __hc__run_bfs_async1_awaitList_size) * sizeof(p_DDF_t ));
  __hc__run_bfs_async1_awaitList -> array[__hc__run_bfs_async1_awaitList_size] = NULL;
  unsigned int __hc__run_bfs_async1_awaitList_curr_index = 0;
  __hc__run_bfs_async1_awaitList -> array[__hc__run_bfs_async1_awaitList_curr_index++] = create_graph_ddf;
  __hc__run_bfs_async1_awaitList -> waitingFrontier = &__hc__run_bfs_async1_awaitList -> array[0];
  __hc__run_bfs_async1__frame__ -> header.awaitList = __hc__run_bfs_async1_awaitList;
  __hc__run_bfs_async1__frame__ -> scope0.has_adj = ((int *__restrict__ )has_adj);
  __hc__run_bfs_async1__frame__ -> scope0.m = ((int )m);
  __hc__run_bfs_async1__frame__ -> scope0.t = ((int64_t )t);
  __hc__run_bfs_async1__frame__ -> scope0.run_bfs_ddf = ((DDF_t *)run_bfs_ddf);
  ((hc_frameHeader *)__hc__run_bfs_async1__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__run_bfs_async1__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__run_bfs_async1__frame__ -> header)) 
      deque_push_default(__hc__run_bfs_async1__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc_run_bfs_frame_t__ *)__hc_frame__) -> scope1.has_adj = has_adj;
    ((struct __hc_run_bfs_frame_t__ *)__hc_frame__) -> scope1.m = m;
    ((struct __hc_run_bfs_frame_t__ *)__hc_frame__) -> scope1.t = t;
    ((struct __hc_run_bfs_frame_t__ *)__hc_frame__) -> scope1.create_graph_ddf = create_graph_ddf;
    ((struct __hc_run_bfs_frame_t__ *)__hc_frame__) -> scope0.run_bfs_ddf = run_bfs_ddf;
    __hc_frame__ -> pc = 1;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_run_bfs__, and execute async: %p in _run_bfs_async1_\n", __hc_frame__, __hc_frame__->pc, __hc__run_bfs_async1__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__run_bfs_async1__frame__ -> header)) 
      _run_bfs_async1_(ws,((hc_frameHeader *)__hc__run_bfs_async1__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_run_bfs__\n", printEntry(__hc_frame__, buffer));
      has_adj = ((struct __hc_run_bfs_frame_t__ *)__hc_frame__) -> scope1.has_adj;
      m = ((struct __hc_run_bfs_frame_t__ *)__hc_frame__) -> scope1.m;
      t = ((struct __hc_run_bfs_frame_t__ *)__hc_frame__) -> scope1.t;
      create_graph_ddf = ((struct __hc_run_bfs_frame_t__ *)__hc_frame__) -> scope1.create_graph_ddf;
      run_bfs_ddf = ((struct __hc_run_bfs_frame_t__ *)__hc_frame__) -> scope0.run_bfs_ddf;
    }
  }
  ws -> costatus =  HC_FUNC_COMPLETE;
  return ;
}

void _run_bfs_async7_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  DDF_t *run_bfs_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  run_bfs_ddf = ((struct __hc__run_bfs_async7__frame_t__ *)__hc_frame__) -> scope0.run_bfs_ddf;
  destroy_graph();
  __ddfPut(run_bfs_ddf,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _run_bfs_async6_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  DDF_t *run_bfs_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_func2;
  }
  run_bfs_ddf = ((struct __hc__run_bfs_async6__frame_t__ *)__hc_frame__) -> scope0.run_bfs_ddf;
  DDF_t *bfs_iter_ddf = __ddfCreate();
  ws -> costatus =  HC_FUNC_COMPLETE;
  __hc_frame__ -> pc = 2;
  ((struct __hc__run_bfs_async6__frame_t__ *)__hc_frame__) -> scope1.bfs_iter_ddf = bfs_iter_ddf;
  ((struct __hc__run_bfs_async6__frame_t__ *)__hc_frame__) -> scope0.run_bfs_ddf = run_bfs_ddf;
  bfs_iter(0,bfs_iter_ddf);
  if (ws -> costatus !=  HC_FUNC_COMPLETE) 
    return ;
  if (0) {
    __post_func2:
    hc_free_unwind_frame(ws);
    __hc_frame__ = ws -> frame;
    bfs_iter_ddf = ((struct __hc__run_bfs_async6__frame_t__ *)__hc_frame__) -> scope1.bfs_iter_ddf;
    run_bfs_ddf = ((struct __hc__run_bfs_async6__frame_t__ *)__hc_frame__) -> scope0.run_bfs_ddf;
  }
/* Write back value for INOUT/OUT arguments */
  struct __hc__run_bfs_async7__frame_t__ *__hc__run_bfs_async7__frame__ = (struct __hc__run_bfs_async7__frame_t__ *)(hc_init_frame(sizeof(struct __hc__run_bfs_async7__frame_t__ ),_run_bfs_async7_));
  p_await_list_t __hc__run_bfs_async7_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__run_bfs_async7_awaitList_size = 0 + 1;
  __hc__run_bfs_async7_awaitList -> array = hc_malloc((1 + __hc__run_bfs_async7_awaitList_size) * sizeof(p_DDF_t ));
  __hc__run_bfs_async7_awaitList -> array[__hc__run_bfs_async7_awaitList_size] = NULL;
  unsigned int __hc__run_bfs_async7_awaitList_curr_index = 0;
  __hc__run_bfs_async7_awaitList -> array[__hc__run_bfs_async7_awaitList_curr_index++] = bfs_iter_ddf;
  __hc__run_bfs_async7_awaitList -> waitingFrontier = &__hc__run_bfs_async7_awaitList -> array[0];
  __hc__run_bfs_async7__frame__ -> header.awaitList = __hc__run_bfs_async7_awaitList;
  __hc__run_bfs_async7__frame__ -> scope0.run_bfs_ddf = ((DDF_t *)run_bfs_ddf);
  ((hc_frameHeader *)__hc__run_bfs_async7__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__run_bfs_async7__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__run_bfs_async7__frame__ -> header)) 
      deque_push_default(__hc__run_bfs_async7__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc__run_bfs_async6__frame_t__ *)__hc_frame__) -> scope1.bfs_iter_ddf = bfs_iter_ddf;
    ((struct __hc__run_bfs_async6__frame_t__ *)__hc_frame__) -> scope0.run_bfs_ddf = run_bfs_ddf;
    __hc_frame__ -> pc = 1;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _run_bfs_async6_, and execute async: %p in _run_bfs_async7_\n", __hc_frame__, __hc_frame__->pc, __hc__run_bfs_async7__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__run_bfs_async7__frame__ -> header)) 
      _run_bfs_async7_(ws,((hc_frameHeader *)__hc__run_bfs_async7__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _run_bfs_async6_\n", printEntry(__hc_frame__, buffer));
      bfs_iter_ddf = ((struct __hc__run_bfs_async6__frame_t__ *)__hc_frame__) -> scope1.bfs_iter_ddf;
      run_bfs_ddf = ((struct __hc__run_bfs_async6__frame_t__ *)__hc_frame__) -> scope0.run_bfs_ddf;
    }
  }
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _run_bfs_async5_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int *__restrict__ has_adj;
  int m;
  int64_t t;
  DDF_t *bfs_init_ddf;
/* Sample from {0, ..., nvtx_scale-1} without replacement. */
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  has_adj = ((struct __hc__run_bfs_async5__frame_t__ *)__hc_frame__) -> scope0.has_adj;
  m = ((struct __hc__run_bfs_async5__frame_t__ *)__hc_frame__) -> scope0.m;
  t = ((struct __hc__run_bfs_async5__frame_t__ *)__hc_frame__) -> scope0.t;
  bfs_init_ddf = ((struct __hc__run_bfs_async5__frame_t__ *)__hc_frame__) -> scope0.bfs_init_ddf;
  m = 0;
  t = 0;
  while((m < NBFS) && (t < nvtx_scale)){
    double R = mrg_get_double_orig(prng_state);
    if (!(has_adj[t] != 0) || (((nvtx_scale - t) * R) > (NBFS - m))) 
      ++t;
    else 
      bfs_root[m++] = t++;
  }
  if ((t >= nvtx_scale) && (m < NBFS)) {
    if (m > 0) {
      fprintf(__stderrp,"Cannot find %d sample roots of non-self degree > 0, using %d.\n",NBFS,m);
      NBFS = m;
    }
    else {
      fprintf(__stderrp,"Cannot find any sample roots of non-self degree > 0.\n");
      exit(1);
    }
  }
  xfree_large(has_adj);
  __ddfPut(bfs_init_ddf,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _run_bfs_async4_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t lb;
  int64_t ub;
  int *__restrict__ has_adj;
  packed_edge *__restrict__ IJ;
  DDF_t *d;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  lb = ((struct __hc__run_bfs_async4__frame_t__ *)__hc_frame__) -> scope0.lb;
  ub = ((struct __hc__run_bfs_async4__frame_t__ *)__hc_frame__) -> scope0.ub;
  has_adj = ((struct __hc__run_bfs_async4__frame_t__ *)__hc_frame__) -> scope0.has_adj;
  IJ = ((struct __hc__run_bfs_async4__frame_t__ *)__hc_frame__) -> scope0.IJ;
  d = ((struct __hc__run_bfs_async4__frame_t__ *)__hc_frame__) -> scope0.d;
  int64_t k;
  for (k = lb; k < ub; k++) {
    int64_t i = get_v0_from_edge((IJ + k));
    int64_t j = get_v1_from_edge((IJ + k));
    if (i != j) 
      has_adj[i] = (has_adj[j] = 1);
  }
  __ddfPut(d,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _run_bfs_async3_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t nedge;
  int p;
  int *__restrict__ has_adj;
  int m;
  int64_t t;
  packed_edge *__restrict__ IJ;
  DDF_t *bfs_init_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_async2;
  }
  nedge = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.nedge;
  p = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.p;
  has_adj = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.has_adj;
  m = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.m;
  t = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.t;
  IJ = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.IJ;
  bfs_init_ddf = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.bfs_init_ddf;
  p_DDF_list_t listOfAdjInit = __ddfListCreate();
  int64_t i;
  int64_t c = (nedge / p);
  for (i = 0; i < (p - 1); i++) {
    int64_t lb = (i * c);
    int64_t ub = (lb + c);
    DDF_t *d = __ddfCreate();
    __enlist(d,listOfAdjInit);
    struct __hc__run_bfs_async4__frame_t__ *__hc__run_bfs_async4__frame__ = (struct __hc__run_bfs_async4__frame_t__ *)(hc_init_frame(sizeof(struct __hc__run_bfs_async4__frame_t__ ),_run_bfs_async4_));
    __hc__run_bfs_async4__frame__ -> scope0.lb = ((int64_t )lb);
    __hc__run_bfs_async4__frame__ -> scope0.ub = ((int64_t )ub);
    __hc__run_bfs_async4__frame__ -> scope0.has_adj = ((int *__restrict__ )has_adj);
    __hc__run_bfs_async4__frame__ -> scope0.IJ = ((packed_edge *__restrict__ )IJ);
    __hc__run_bfs_async4__frame__ -> scope0.d = ((DDF_t *)d);
    ((hc_frameHeader *)__hc__run_bfs_async4__frame__) -> sched_policy = hc_sched_policy(ws);
    increment_counter();
    if (((hc_frameHeader *)__hc__run_bfs_async4__frame__) -> sched_policy ==  HELP_FIRST) {
      deque_push_default(__hc__run_bfs_async4__frame__);
    }
    else {
      ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope2.lb = lb;
      ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope2.ub = ub;
      ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope2.d = d;
      ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope1.listOfAdjInit = listOfAdjInit;
      ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope1.i = i;
      ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope1.c = c;
      ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.nedge = nedge;
      ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.p = p;
      ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.has_adj = has_adj;
      ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.m = m;
      ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.t = t;
      ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.IJ = IJ;
      ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.bfs_init_ddf = bfs_init_ddf;
      __hc_frame__ -> pc = 1;
      deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _run_bfs_async3_, and execute async: %p in _run_bfs_async4_\n", __hc_frame__, __hc_frame__->pc, __hc__run_bfs_async4__frame__);
      _run_bfs_async4_(ws,((hc_frameHeader *)__hc__run_bfs_async4__frame__),0);
      if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
        return ;
      }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
      if (0) {
        __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _run_bfs_async3_\n", printEntry(__hc_frame__, buffer));
        lb = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope2.lb;
        ub = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope2.ub;
        d = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope2.d;
        listOfAdjInit = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope1.listOfAdjInit;
        i = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope1.i;
        c = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope1.c;
        nedge = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.nedge;
        p = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.p;
        has_adj = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.has_adj;
        m = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.m;
        t = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.t;
        IJ = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.IJ;
        bfs_init_ddf = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.bfs_init_ddf;
      }
    }
  }
  int64_t k;
  int64_t lb = (i * c);
  for (k = lb; k < nedge; k++) {
    int64_t i = get_v0_from_edge((IJ + k));
    int64_t j = get_v1_from_edge((IJ + k));
    if (i != j) 
      has_adj[i] = (has_adj[j] = 1);
  }
/* Write back value for INOUT/OUT arguments */
  struct __hc__run_bfs_async5__frame_t__ *__hc__run_bfs_async5__frame__ = (struct __hc__run_bfs_async5__frame_t__ *)(hc_init_frame(sizeof(struct __hc__run_bfs_async5__frame_t__ ),_run_bfs_async5_));
  p_await_list_t __hc__run_bfs_async5_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__run_bfs_async5_awaitList_size = 0 + listOfAdjInit -> size + 0;
  __hc__run_bfs_async5_awaitList -> array = hc_malloc((1 + __hc__run_bfs_async5_awaitList_size) * sizeof(p_DDF_t ));
  __hc__run_bfs_async5_awaitList -> array[__hc__run_bfs_async5_awaitList_size] = NULL;
  unsigned int __hc__run_bfs_async5_awaitList_curr_index = 0;
  __copyInto(__hc__run_bfs_async5_awaitList,__hc__run_bfs_async5_awaitList_curr_index,listOfAdjInit);
  __hc__run_bfs_async5_awaitList_curr_index += listOfAdjInit -> size;
  __hc__run_bfs_async5_awaitList -> waitingFrontier = &__hc__run_bfs_async5_awaitList -> array[0];
  __hc__run_bfs_async5__frame__ -> header.awaitList = __hc__run_bfs_async5_awaitList;
  __hc__run_bfs_async5__frame__ -> scope0.has_adj = ((int *__restrict__ )has_adj);
  __hc__run_bfs_async5__frame__ -> scope0.m = ((int )m);
  __hc__run_bfs_async5__frame__ -> scope0.t = ((int64_t )t);
  __hc__run_bfs_async5__frame__ -> scope0.bfs_init_ddf = ((DDF_t *)bfs_init_ddf);
  ((hc_frameHeader *)__hc__run_bfs_async5__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__run_bfs_async5__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__run_bfs_async5__frame__ -> header)) 
      deque_push_default(__hc__run_bfs_async5__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope1.listOfAdjInit = listOfAdjInit;
    ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope1.i = i;
    ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope1.c = c;
    ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope1.k = k;
    ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope1.lb = lb;
    ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.nedge = nedge;
    ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.p = p;
    ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.has_adj = has_adj;
    ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.m = m;
    ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.t = t;
    ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.IJ = IJ;
    ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.bfs_init_ddf = bfs_init_ddf;
    __hc_frame__ -> pc = 2;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _run_bfs_async3_, and execute async: %p in _run_bfs_async5_\n", __hc_frame__, __hc_frame__->pc, __hc__run_bfs_async5__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__run_bfs_async5__frame__ -> header)) 
      _run_bfs_async5_(ws,((hc_frameHeader *)__hc__run_bfs_async5__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async2:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _run_bfs_async3_\n", printEntry(__hc_frame__, buffer));
      listOfAdjInit = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope1.listOfAdjInit;
      i = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope1.i;
      c = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope1.c;
      k = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope1.k;
      lb = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope1.lb;
      nedge = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.nedge;
      p = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.p;
      has_adj = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.has_adj;
      m = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.m;
      t = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.t;
      IJ = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.IJ;
      bfs_init_ddf = ((struct __hc__run_bfs_async3__frame_t__ *)__hc_frame__) -> scope0.bfs_init_ddf;
    }
  }
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _run_bfs_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t lb;
  int64_t ub;
  int *__restrict__ has_adj;
  DDF_t *d;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  lb = ((struct __hc__run_bfs_async2__frame_t__ *)__hc_frame__) -> scope0.lb;
  ub = ((struct __hc__run_bfs_async2__frame_t__ *)__hc_frame__) -> scope0.ub;
  has_adj = ((struct __hc__run_bfs_async2__frame_t__ *)__hc_frame__) -> scope0.has_adj;
  d = ((struct __hc__run_bfs_async2__frame_t__ *)__hc_frame__) -> scope0.d;
  int64_t k;
  for (k = lb; k < ub; k++) {
    has_adj[k] = 0;
  }
  __ddfPut(d,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _run_bfs_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int *__restrict__ has_adj;
  int m;
  int64_t t;
  DDF_t *run_bfs_ddf;
/* Stop Time: construction time */
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_async2;
    case 3:
    goto __post_async3;
  }
  has_adj = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.has_adj;
  m = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.m;
  t = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.t;
  run_bfs_ddf = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.run_bfs_ddf;
  construction_time = toc();
  if (VERBOSE != 0) 
    fprintf(__stderrp,"done.\n");
  int p = ( *( *((hc_workerState *)(pthread_getspecific(wskey)))).context).nproc;
  DDF_t *bfs_init_ddf = __ddfCreate();
/*
    If running the benchmark under an architecture simulator, replace
    the following if () {} else {} with a statement pointing bfs_root
    to wherever the BFS roots are mapped into the simulator's memory.
	*/
  if (!(rootname != 0)) {
    has_adj = (xmalloc_large((nvtx_scale * sizeof(( *has_adj)))));
    p_DDF_list_t listOfAdjClear = __ddfListCreate();
    int64_t i;
    int64_t c = (nvtx_scale / p);
    for (i = 0; i < (p - 1); i++) {
      int64_t lb = (i * c);
      int64_t ub = (lb + c);
      DDF_t *d = __ddfCreate();
      __enlist(d,listOfAdjClear);
      struct __hc__run_bfs_async2__frame_t__ *__hc__run_bfs_async2__frame__ = (struct __hc__run_bfs_async2__frame_t__ *)(hc_init_frame(sizeof(struct __hc__run_bfs_async2__frame_t__ ),_run_bfs_async2_));
      __hc__run_bfs_async2__frame__ -> scope0.lb = ((int64_t )lb);
      __hc__run_bfs_async2__frame__ -> scope0.ub = ((int64_t )ub);
      __hc__run_bfs_async2__frame__ -> scope0.has_adj = ((int *__restrict__ )has_adj);
      __hc__run_bfs_async2__frame__ -> scope0.d = ((DDF_t *)d);
      ((hc_frameHeader *)__hc__run_bfs_async2__frame__) -> sched_policy = hc_sched_policy(ws);
      increment_counter();
      if (((hc_frameHeader *)__hc__run_bfs_async2__frame__) -> sched_policy ==  HELP_FIRST) {
        deque_push_default(__hc__run_bfs_async2__frame__);
      }
      else {
        ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope3.lb = lb;
        ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope3.ub = ub;
        ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope3.d = d;
        ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope2.listOfAdjClear = listOfAdjClear;
        ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope2.i = i;
        ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope2.c = c;
        ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope1.p = p;
        ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope1.bfs_init_ddf = bfs_init_ddf;
        ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.has_adj = has_adj;
        ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.m = m;
        ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.t = t;
        ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.run_bfs_ddf = run_bfs_ddf;
        __hc_frame__ -> pc = 1;
        deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _run_bfs_async1_, and execute async: %p in _run_bfs_async2_\n", __hc_frame__, __hc_frame__->pc, __hc__run_bfs_async2__frame__);
        _run_bfs_async2_(ws,((hc_frameHeader *)__hc__run_bfs_async2__frame__),0);
        if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
          return ;
        }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
        if (0) {
          __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _run_bfs_async1_\n", printEntry(__hc_frame__, buffer));
          lb = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope3.lb;
          ub = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope3.ub;
          d = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope3.d;
          listOfAdjClear = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope2.listOfAdjClear;
          i = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope2.i;
          c = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope2.c;
          p = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope1.p;
          bfs_init_ddf = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope1.bfs_init_ddf;
          has_adj = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.has_adj;
          m = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.m;
          t = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.t;
          run_bfs_ddf = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.run_bfs_ddf;
        }
      }
    }
    int64_t k;
    int64_t lb = (i * c);
    for (k = lb; k < nvtx_scale; k++) {
      has_adj[k] = 0;
    }
    struct __hc__run_bfs_async3__frame_t__ *__hc__run_bfs_async3__frame__ = (struct __hc__run_bfs_async3__frame_t__ *)(hc_init_frame(sizeof(struct __hc__run_bfs_async3__frame_t__ ),_run_bfs_async3_));
    p_await_list_t __hc__run_bfs_async3_awaitList = hc_malloc(sizeof(await_list_t ));
    unsigned int __hc__run_bfs_async3_awaitList_size = 0 + listOfAdjClear -> size + 0;
    __hc__run_bfs_async3_awaitList -> array = hc_malloc((1 + __hc__run_bfs_async3_awaitList_size) * sizeof(p_DDF_t ));
    __hc__run_bfs_async3_awaitList -> array[__hc__run_bfs_async3_awaitList_size] = NULL;
    unsigned int __hc__run_bfs_async3_awaitList_curr_index = 0;
    __copyInto(__hc__run_bfs_async3_awaitList,__hc__run_bfs_async3_awaitList_curr_index,listOfAdjClear);
    __hc__run_bfs_async3_awaitList_curr_index += listOfAdjClear -> size;
    __hc__run_bfs_async3_awaitList -> waitingFrontier = &__hc__run_bfs_async3_awaitList -> array[0];
    __hc__run_bfs_async3__frame__ -> header.awaitList = __hc__run_bfs_async3_awaitList;
    __hc__run_bfs_async3__frame__ -> scope0.nedge = ((int64_t )nedge);
    __hc__run_bfs_async3__frame__ -> scope0.p = ((int )p);
    __hc__run_bfs_async3__frame__ -> scope0.has_adj = ((int *__restrict__ )has_adj);
    __hc__run_bfs_async3__frame__ -> scope0.m = ((int )m);
    __hc__run_bfs_async3__frame__ -> scope0.t = ((int64_t )t);
    __hc__run_bfs_async3__frame__ -> scope0.IJ = ((packed_edge *__restrict__ )IJ);
    __hc__run_bfs_async3__frame__ -> scope0.bfs_init_ddf = ((DDF_t *)bfs_init_ddf);
    ((hc_frameHeader *)__hc__run_bfs_async3__frame__) -> sched_policy = hc_sched_policy(ws);
    increment_counter();
    if (((hc_frameHeader *)__hc__run_bfs_async3__frame__) -> sched_policy ==  HELP_FIRST) {
      if (__iterateDDFWaitingFrontier(&__hc__run_bfs_async3__frame__ -> header)) 
        deque_push_default(__hc__run_bfs_async3__frame__);
      else 
        ws -> costatus =  HC_FINISH_SUSPENDED;
    }
    else {
      ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope2.listOfAdjClear = listOfAdjClear;
      ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope2.i = i;
      ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope2.c = c;
      ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope2.k = k;
      ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope2.lb = lb;
      ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope1.p = p;
      ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope1.bfs_init_ddf = bfs_init_ddf;
      ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.has_adj = has_adj;
      ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.m = m;
      ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.t = t;
      ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.run_bfs_ddf = run_bfs_ddf;
      __hc_frame__ -> pc = 2;
      deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _run_bfs_async1_, and execute async: %p in _run_bfs_async3_\n", __hc_frame__, __hc_frame__->pc, __hc__run_bfs_async3__frame__);
      if (__iterateDDFWaitingFrontier(&__hc__run_bfs_async3__frame__ -> header)) 
        _run_bfs_async3_(ws,((hc_frameHeader *)__hc__run_bfs_async3__frame__),0);
      else 
        ws -> costatus =  HC_FINISH_SUSPENDED;
      if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
        return ;
      }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
      if (0) {
        __post_async2:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _run_bfs_async1_\n", printEntry(__hc_frame__, buffer));
        listOfAdjClear = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope2.listOfAdjClear;
        i = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope2.i;
        c = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope2.c;
        k = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope2.k;
        lb = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope2.lb;
        p = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope1.p;
        bfs_init_ddf = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope1.bfs_init_ddf;
        has_adj = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.has_adj;
        m = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.m;
        t = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.t;
        run_bfs_ddf = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.run_bfs_ddf;
      }
    }
  }
  else {
    int fd;
    ssize_t sz;
    if ((fd = open(rootname,0)) < 0) {
      perror("Cannot open input BFS root file");
      exit(1);
    }
    sz = (NBFS * sizeof(( *bfs_root)));
    if (sz != read(fd,bfs_root,sz)) {
      perror("Error reading input BFS root file");
      exit(1);
    }
    close(fd);
    __ddfPut(bfs_init_ddf,dummyDataVoidPtr);
  }
/* Write back value for INOUT/OUT arguments */
  struct __hc__run_bfs_async6__frame_t__ *__hc__run_bfs_async6__frame__ = (struct __hc__run_bfs_async6__frame_t__ *)(hc_init_frame(sizeof(struct __hc__run_bfs_async6__frame_t__ ),_run_bfs_async6_));
  p_await_list_t __hc__run_bfs_async6_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__run_bfs_async6_awaitList_size = 0 + 1;
  __hc__run_bfs_async6_awaitList -> array = hc_malloc((1 + __hc__run_bfs_async6_awaitList_size) * sizeof(p_DDF_t ));
  __hc__run_bfs_async6_awaitList -> array[__hc__run_bfs_async6_awaitList_size] = NULL;
  unsigned int __hc__run_bfs_async6_awaitList_curr_index = 0;
  __hc__run_bfs_async6_awaitList -> array[__hc__run_bfs_async6_awaitList_curr_index++] = bfs_init_ddf;
  __hc__run_bfs_async6_awaitList -> waitingFrontier = &__hc__run_bfs_async6_awaitList -> array[0];
  __hc__run_bfs_async6__frame__ -> header.awaitList = __hc__run_bfs_async6_awaitList;
  __hc__run_bfs_async6__frame__ -> scope0.run_bfs_ddf = ((DDF_t *)run_bfs_ddf);
  ((hc_frameHeader *)__hc__run_bfs_async6__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__run_bfs_async6__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__run_bfs_async6__frame__ -> header)) 
      deque_push_default(__hc__run_bfs_async6__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope1.p = p;
    ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope1.bfs_init_ddf = bfs_init_ddf;
    ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.has_adj = has_adj;
    ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.m = m;
    ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.t = t;
    ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.run_bfs_ddf = run_bfs_ddf;
    __hc_frame__ -> pc = 3;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _run_bfs_async1_, and execute async: %p in _run_bfs_async6_\n", __hc_frame__, __hc_frame__->pc, __hc__run_bfs_async6__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__run_bfs_async6__frame__ -> header)) 
      _run_bfs_async6_(ws,((hc_frameHeader *)__hc__run_bfs_async6__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async3:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _run_bfs_async1_\n", printEntry(__hc_frame__, buffer));
      p = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope1.p;
      bfs_init_ddf = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope1.bfs_init_ddf;
      has_adj = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.has_adj;
      m = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.m;
      t = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.t;
      run_bfs_ddf = ((struct __hc__run_bfs_async1__frame_t__ *)__hc_frame__) -> scope0.run_bfs_ddf;
    }
  }
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}
typedef struct __hc_bfs_iter_frame_t__ {
hc_frameHeader header;
struct __hc_bfs_iter_scope0_frame_t__ {
int m;
DDF_t *bfs_iter_ddf;}scope0;
struct __hc_bfs_iter_scope1_frame_t__ {
int64_t *bfs_tree;
DDF_t *bfs_tree_ddf;}scope1;}__hc_bfs_iter_frame_t__;
typedef struct __hc__bfs_iter_async1__frame_t__ {
hc_frameHeader header;
struct __hc__bfs_iter_async1__scope0_frame_t__ {
int m;
int64_t *bfs_tree;
DDF_t *bfs_iter_ddf;}scope0;
struct __hc__bfs_iter_async1__scope1_frame_t__ {
DDF_t *verify_bfs_ddf;
volatile int *err;}scope1;}__hc__bfs_iter_async1__frame_t__;
typedef struct __hc__bfs_iter_async2__frame_t__ {
hc_frameHeader header;
struct __hc__bfs_iter_async2__scope0_frame_t__ {
int m;
int64_t *bfs_tree;
DDF_t *bfs_iter_ddf;
volatile int *err;
DDF_t *verify_bfs_ddf;}scope0;}__hc__bfs_iter_async2__frame_t__;
void __hc_bfs_iter__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _bfs_iter_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _bfs_iter_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);

static void bfs_iter(int m,DDF_t *bfs_iter_ddf)
{
  hc_workerState *ws = current_ws();
  ws -> costatus =  HC_FUNC_COMPLETE;
  struct __hc_bfs_iter_frame_t__ *__hc_frame__ = (struct __hc_bfs_iter_frame_t__ *)(hc_init_frame_ws(ws,sizeof(struct __hc_bfs_iter_frame_t__ ),__hc_bfs_iter__));
  __hc_frame__ -> scope0.m = m;
  __hc_frame__ -> scope0.bfs_iter_ddf = bfs_iter_ddf;
			LOG_INFO(ws, "execute frame: %p, parent: %p of function %s\n", __hc_frame__,((hc_frameHeader*) __hc_frame__)->parent, "__hc_bfs_iter__" );
  __hc_bfs_iter__(ws,((hc_frameHeader *)__hc_frame__),0);
  if (ws -> costatus ==  HC_FUNC_COMPLETE) 
    hc_free_unwind_frame(ws);
  return ;
}

void __hc_bfs_iter__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int m;
  DDF_t *bfs_iter_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_func2;
  }
  m = ((struct __hc_bfs_iter_frame_t__ *)__hc_frame__) -> scope0.m;
  bfs_iter_ddf = ((struct __hc_bfs_iter_frame_t__ *)__hc_frame__) -> scope0.bfs_iter_ddf;
  int64_t *bfs_tree;
  if (m >= NBFS) {
    __ddfPut(bfs_iter_ddf,dummyDataVoidPtr);
    ws -> costatus =  HC_FUNC_COMPLETE;
    return ;
  }
/* Re-allocate. Some systems may randomize the addres... */
  bfs_tree = (xmalloc_large((nvtx_scale * sizeof(( *bfs_tree)))));
  (__builtin_expect((!(bfs_root[m] < nvtx_scale)),0) != 0L)?__assert_rtn(__func__,"/Users/dm14/Documents/Habanero/hc-trunk/examples/graph500-2.1.4/graph500-ddf.hc",303,"bfs_root[m] < nvtx_scale") : ((void )0);
  if (VERBOSE != 0) 
    fprintf(__stderrp,"Running bfs %d...",m);
  DDF_t *bfs_tree_ddf = __ddfCreate();
/* Start time: bfs_time*/
  tic();
  ws -> costatus =  HC_FUNC_COMPLETE;
  __hc_frame__ -> pc = 2;
  ((struct __hc_bfs_iter_frame_t__ *)__hc_frame__) -> scope1.bfs_tree = bfs_tree;
  ((struct __hc_bfs_iter_frame_t__ *)__hc_frame__) -> scope1.bfs_tree_ddf = bfs_tree_ddf;
  ((struct __hc_bfs_iter_frame_t__ *)__hc_frame__) -> scope0.m = m;
  ((struct __hc_bfs_iter_frame_t__ *)__hc_frame__) -> scope0.bfs_iter_ddf = bfs_iter_ddf;
  make_bfs_tree(bfs_tree,bfs_root[m],bfs_tree_ddf);
  if (ws -> costatus !=  HC_FUNC_COMPLETE) 
    return ;
  if (0) {
    __post_func2:
    hc_free_unwind_frame(ws);
    __hc_frame__ = ws -> frame;
    bfs_tree = ((struct __hc_bfs_iter_frame_t__ *)__hc_frame__) -> scope1.bfs_tree;
    bfs_tree_ddf = ((struct __hc_bfs_iter_frame_t__ *)__hc_frame__) -> scope1.bfs_tree_ddf;
    m = ((struct __hc_bfs_iter_frame_t__ *)__hc_frame__) -> scope0.m;
    bfs_iter_ddf = ((struct __hc_bfs_iter_frame_t__ *)__hc_frame__) -> scope0.bfs_iter_ddf;
  }
  struct __hc__bfs_iter_async1__frame_t__ *__hc__bfs_iter_async1__frame__ = (struct __hc__bfs_iter_async1__frame_t__ *)(hc_init_frame(sizeof(struct __hc__bfs_iter_async1__frame_t__ ),_bfs_iter_async1_));
  p_await_list_t __hc__bfs_iter_async1_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__bfs_iter_async1_awaitList_size = 0 + 1;
  __hc__bfs_iter_async1_awaitList -> array = hc_malloc((1 + __hc__bfs_iter_async1_awaitList_size) * sizeof(p_DDF_t ));
  __hc__bfs_iter_async1_awaitList -> array[__hc__bfs_iter_async1_awaitList_size] = NULL;
  unsigned int __hc__bfs_iter_async1_awaitList_curr_index = 0;
  __hc__bfs_iter_async1_awaitList -> array[__hc__bfs_iter_async1_awaitList_curr_index++] = bfs_tree_ddf;
  __hc__bfs_iter_async1_awaitList -> waitingFrontier = &__hc__bfs_iter_async1_awaitList -> array[0];
  __hc__bfs_iter_async1__frame__ -> header.awaitList = __hc__bfs_iter_async1_awaitList;
  __hc__bfs_iter_async1__frame__ -> scope0.m = ((int )m);
  __hc__bfs_iter_async1__frame__ -> scope0.bfs_tree = ((int64_t *)bfs_tree);
  __hc__bfs_iter_async1__frame__ -> scope0.bfs_iter_ddf = ((DDF_t *)bfs_iter_ddf);
  ((hc_frameHeader *)__hc__bfs_iter_async1__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__bfs_iter_async1__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__bfs_iter_async1__frame__ -> header)) 
      deque_push_default(__hc__bfs_iter_async1__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc_bfs_iter_frame_t__ *)__hc_frame__) -> scope1.bfs_tree = bfs_tree;
    ((struct __hc_bfs_iter_frame_t__ *)__hc_frame__) -> scope1.bfs_tree_ddf = bfs_tree_ddf;
    ((struct __hc_bfs_iter_frame_t__ *)__hc_frame__) -> scope0.m = m;
    ((struct __hc_bfs_iter_frame_t__ *)__hc_frame__) -> scope0.bfs_iter_ddf = bfs_iter_ddf;
    __hc_frame__ -> pc = 1;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_bfs_iter__, and execute async: %p in _bfs_iter_async1_\n", __hc_frame__, __hc_frame__->pc, __hc__bfs_iter_async1__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__bfs_iter_async1__frame__ -> header)) 
      _bfs_iter_async1_(ws,((hc_frameHeader *)__hc__bfs_iter_async1__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_bfs_iter__\n", printEntry(__hc_frame__, buffer));
      bfs_tree = ((struct __hc_bfs_iter_frame_t__ *)__hc_frame__) -> scope1.bfs_tree;
      bfs_tree_ddf = ((struct __hc_bfs_iter_frame_t__ *)__hc_frame__) -> scope1.bfs_tree_ddf;
      m = ((struct __hc_bfs_iter_frame_t__ *)__hc_frame__) -> scope0.m;
      bfs_iter_ddf = ((struct __hc_bfs_iter_frame_t__ *)__hc_frame__) -> scope0.bfs_iter_ddf;
    }
  }
  ws -> costatus =  HC_FUNC_COMPLETE;
  return ;
}

void _bfs_iter_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int m;
  int64_t *bfs_tree;
  DDF_t *bfs_iter_ddf;
  volatile int *err;
  DDF_t *verify_bfs_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_func1;
  }
  m = ((struct __hc__bfs_iter_async2__frame_t__ *)__hc_frame__) -> scope0.m;
  bfs_tree = ((struct __hc__bfs_iter_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
  bfs_iter_ddf = ((struct __hc__bfs_iter_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_iter_ddf;
  err = ((struct __hc__bfs_iter_async2__frame_t__ *)__hc_frame__) -> scope0.err;
  verify_bfs_ddf = ((struct __hc__bfs_iter_async2__frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf;
  bfs_nedge[m] =  *((int64_t *)(__ddfGet(verify_bfs_ddf)));
  if (VERBOSE != 0) 
    fprintf(__stderrp,"done\n");
  if (bfs_nedge[m] < 0) {
    fprintf(__stderrp,"bfs %d from %lld failed verification (%lld)\n",m,bfs_root[m],bfs_nedge[m]);
    abort();
  }
  hc_mm_free(((hc_workerState *)(pthread_getspecific(wskey))),((void *)err));
  xfree_large(bfs_tree);
  m++;
  ws -> costatus =  HC_FUNC_COMPLETE;
  __hc_frame__ -> pc = 1;
  ((struct __hc__bfs_iter_async2__frame_t__ *)__hc_frame__) -> scope0.m = m;
  ((struct __hc__bfs_iter_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_tree = bfs_tree;
  ((struct __hc__bfs_iter_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_iter_ddf = bfs_iter_ddf;
  ((struct __hc__bfs_iter_async2__frame_t__ *)__hc_frame__) -> scope0.err = err;
  ((struct __hc__bfs_iter_async2__frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf = verify_bfs_ddf;
  bfs_iter(m,bfs_iter_ddf);
  if (ws -> costatus !=  HC_FUNC_COMPLETE) 
    return ;
  if (0) {
    __post_func1:
    hc_free_unwind_frame(ws);
    __hc_frame__ = ws -> frame;
    m = ((struct __hc__bfs_iter_async2__frame_t__ *)__hc_frame__) -> scope0.m;
    bfs_tree = ((struct __hc__bfs_iter_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
    bfs_iter_ddf = ((struct __hc__bfs_iter_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_iter_ddf;
    err = ((struct __hc__bfs_iter_async2__frame_t__ *)__hc_frame__) -> scope0.err;
    verify_bfs_ddf = ((struct __hc__bfs_iter_async2__frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf;
  }
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _bfs_iter_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int m;
  int64_t *bfs_tree;
  DDF_t *bfs_iter_ddf;
/* Stop Time: bfs_time */
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_func2;
  }
  m = ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope0.m;
  bfs_tree = ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
  bfs_iter_ddf = ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope0.bfs_iter_ddf;
  bfs_time[m] = toc();
  if (VERBOSE != 0) 
    fprintf(__stderrp,"done\n");
  if (VERBOSE != 0) 
    fprintf(__stderrp,"Verifying bfs %d...",m);
  DDF_t *verify_bfs_ddf = __ddfCreate();
  volatile int *err = (volatile int *)(hc_mm_malloc(((hc_workerState *)(pthread_getspecific(wskey))),(sizeof(int ))));
  ws -> costatus =  HC_FUNC_COMPLETE;
  __hc_frame__ -> pc = 2;
  ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope1.verify_bfs_ddf = verify_bfs_ddf;
  ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope1.err = err;
  ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope0.m = m;
  ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope0.bfs_tree = bfs_tree;
  ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope0.bfs_iter_ddf = bfs_iter_ddf;
  verify_bfs_tree(bfs_tree,maxvtx,bfs_root[m],IJ,nedge,err,verify_bfs_ddf);
  if (ws -> costatus !=  HC_FUNC_COMPLETE) 
    return ;
  if (0) {
    __post_func2:
    hc_free_unwind_frame(ws);
    __hc_frame__ = ws -> frame;
    verify_bfs_ddf = ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope1.verify_bfs_ddf;
    err = ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope1.err;
    m = ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope0.m;
    bfs_tree = ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
    bfs_iter_ddf = ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope0.bfs_iter_ddf;
  }
/* Write back value for INOUT/OUT arguments */
  struct __hc__bfs_iter_async2__frame_t__ *__hc__bfs_iter_async2__frame__ = (struct __hc__bfs_iter_async2__frame_t__ *)(hc_init_frame(sizeof(struct __hc__bfs_iter_async2__frame_t__ ),_bfs_iter_async2_));
  p_await_list_t __hc__bfs_iter_async2_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__bfs_iter_async2_awaitList_size = 0 + 1;
  __hc__bfs_iter_async2_awaitList -> array = hc_malloc((1 + __hc__bfs_iter_async2_awaitList_size) * sizeof(p_DDF_t ));
  __hc__bfs_iter_async2_awaitList -> array[__hc__bfs_iter_async2_awaitList_size] = NULL;
  unsigned int __hc__bfs_iter_async2_awaitList_curr_index = 0;
  __hc__bfs_iter_async2_awaitList -> array[__hc__bfs_iter_async2_awaitList_curr_index++] = verify_bfs_ddf;
  __hc__bfs_iter_async2_awaitList -> waitingFrontier = &__hc__bfs_iter_async2_awaitList -> array[0];
  __hc__bfs_iter_async2__frame__ -> header.awaitList = __hc__bfs_iter_async2_awaitList;
  __hc__bfs_iter_async2__frame__ -> scope0.m = ((int )m);
  __hc__bfs_iter_async2__frame__ -> scope0.bfs_tree = ((int64_t *)bfs_tree);
  __hc__bfs_iter_async2__frame__ -> scope0.bfs_iter_ddf = ((DDF_t *)bfs_iter_ddf);
  __hc__bfs_iter_async2__frame__ -> scope0.err = ((volatile int *)err);
  __hc__bfs_iter_async2__frame__ -> scope0.verify_bfs_ddf = ((DDF_t *)verify_bfs_ddf);
  ((hc_frameHeader *)__hc__bfs_iter_async2__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__bfs_iter_async2__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__bfs_iter_async2__frame__ -> header)) 
      deque_push_default(__hc__bfs_iter_async2__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope1.verify_bfs_ddf = verify_bfs_ddf;
    ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope1.err = err;
    ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope0.m = m;
    ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope0.bfs_tree = bfs_tree;
    ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope0.bfs_iter_ddf = bfs_iter_ddf;
    __hc_frame__ -> pc = 1;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _bfs_iter_async1_, and execute async: %p in _bfs_iter_async2_\n", __hc_frame__, __hc_frame__->pc, __hc__bfs_iter_async2__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__bfs_iter_async2__frame__ -> header)) 
      _bfs_iter_async2_(ws,((hc_frameHeader *)__hc__bfs_iter_async2__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _bfs_iter_async1_\n", printEntry(__hc_frame__, buffer));
      verify_bfs_ddf = ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope1.verify_bfs_ddf;
      err = ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope1.err;
      m = ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope0.m;
      bfs_tree = ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
      bfs_iter_ddf = ((struct __hc__bfs_iter_async1__frame_t__ *)__hc_frame__) -> scope0.bfs_iter_ddf;
    }
  }
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
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

static int dcmp(const void *a,const void *b)
{
  const double da =  *((const double *)a);
  const double db =  *((const double *)b);
  if (da > db) 
    return 1;
  if (db > da) 
    return -1;
  if (da == db) 
    return 0;
  fprintf(__stderrp,"No NaNs permitted in output.\n");
  abort();
  return 0;
}

void statistics(double *out,double *data,int64_t n)
{
  long double s;
  long double mean;
  double t;
  int k;
/* Quartiles */
  qsort(data,n,(sizeof(( *data))),dcmp);
  out[0] = data[0];
  t = ((n + 1) / 4.0);
  k = ((int )t);
  if (t == k) 
    out[1] = data[k];
  else 
    out[1] = ((3 * (data[k] / 4.0)) + (data[k + 1] / 4.0));
  t = ((n + 1) / 2.0);
  k = ((int )t);
  if (t == k) 
    out[2] = data[k];
  else 
    out[2] = ((data[k] / 2.0) + (data[k + 1] / 2.0));
  t = (3 * ((n + 1) / 4.0));
  k = ((int )t);
  if (t == k) 
    out[3] = data[k];
  else 
    out[3] = ((data[k] / 4.0) + (3 * (data[k + 1] / 4.0)));
  out[4] = data[n - 1];
  s = data[n - 1];
  for (k = (n - 1); k > 0; --k) 
    s += data[k - 1];
  mean = (s / n);
  out[5] = mean;
  s = (data[n - 1] - mean);
  s *= s;
  for (k = (n - 1); k > 0; --k) {
    long double tmp = (data[k - 1] - mean);
    s += (tmp * tmp);
  }
  out[6] = sqrt((s / (n - 1)));
  s = ((data[0] != 0.00000)?(1.0L / data[0]) : 0);
  for (k = 1; k < n; ++k) 
    s += ((data[k] != 0.00000)?(1.0L / data[k]) : 0);
  out[7] = (n / s);
  mean = (s / n);
/*
    Nilan Norris, The Standard Errors of the Geometric and Harmonic
    Means and Their Application to Index Numbers, 1940.
    http://www.jstor.org/stable/2235723
  */
  s = ((((data[0] != 0.00000)?(1.0L / data[0]) : 0)) - mean);
  s *= s;
  for (k = 1; k < n; ++k) {
    long double tmp = ((((data[k] != 0.00000)?(1.0L / data[k]) : 0)) - mean);
    s += (tmp * tmp);
  }
  s = (((sqrt(s) / (n - 1)) * out[7]) * out[7]);
  out[8] = s;
}

static void output_results(const int64_t SCALE,int64_t nvtx_scale,int64_t edgefactor,const double A,const double B,const double C,const double D,const double generation_time,const double construction_time,const int NBFS,const double *bfs_time,const int64_t *bfs_nedge)
{
  int k;
  int64_t sz;
  double *tm;
  double *stats;
  tm = (hc_mm_malloc(((hc_workerState *)(pthread_getspecific(wskey))),(NBFS * sizeof(( *tm)))));
  stats = (hc_mm_malloc(((hc_workerState *)(pthread_getspecific(wskey))),(9 * sizeof(( *stats)))));
  if (!(tm != 0) || !(stats != 0)) {
    perror("Error allocating within final statistics calculation.");
    abort();
  }
  sz = ((((1L << SCALE) * edgefactor) * 2) * sizeof(int64_t ));
  printf("SCALE: %lld\nnvtx: %lld\nedgefactor: %lld\nterasize: %20.17e\n",SCALE,nvtx_scale,edgefactor,(sz / 1.0e12));
  printf("A: %20.17e\nB: %20.17e\nC: %20.17e\nD: %20.17e\n",A,B,C,D);
  printf("generation_time: %20.17e\n",generation_time);
  printf("construction_time: %20.17e\n",construction_time);
  printf("nbfs: %d\n",NBFS);
  (__builtin_object_size(tm,0) != ((size_t )(-1)))?__builtin___memcpy_chk(tm,bfs_time,(NBFS * sizeof(tm[0])),__builtin_object_size(tm,0)) : __inline_memcpy_chk(tm,bfs_time,(NBFS * sizeof(tm[0])));
  statistics(stats,tm,NBFS);
  do {
    printf("min_%s: %20.17e\n","time",stats[0]);
    printf("firstquartile_%s: %20.17e\n","time",stats[1]);
    printf("median_%s: %20.17e\n","time",stats[2]);
    printf("thirdquartile_%s: %20.17e\n","time",stats[3]);
    printf("max_%s: %20.17e\n","time",stats[4]);
    if (1) {
      printf("mean_%s: %20.17e\n","time",stats[5]);
      printf("stddev_%s: %20.17e\n","time",stats[6]);
    }
    else {
      printf("harmonic_mean_%s: %20.17e\n","time",stats[7]);
      printf("harmonic_stddev_%s: %20.17e\n","time",stats[8]);
    }
  }while (0);
  for (k = 0; k < NBFS; ++k) 
    tm[k] = bfs_nedge[k];
  statistics(stats,tm,NBFS);
  do {
    printf("min_%s: %20.17e\n","nedge",stats[0]);
    printf("firstquartile_%s: %20.17e\n","nedge",stats[1]);
    printf("median_%s: %20.17e\n","nedge",stats[2]);
    printf("thirdquartile_%s: %20.17e\n","nedge",stats[3]);
    printf("max_%s: %20.17e\n","nedge",stats[4]);
    if (1) {
      printf("mean_%s: %20.17e\n","nedge",stats[5]);
      printf("stddev_%s: %20.17e\n","nedge",stats[6]);
    }
    else {
      printf("harmonic_mean_%s: %20.17e\n","nedge",stats[7]);
      printf("harmonic_stddev_%s: %20.17e\n","nedge",stats[8]);
    }
  }while (0);
  for (k = 0; k < NBFS; ++k) 
    tm[k] = (bfs_nedge[k] / bfs_time[k]);
  statistics(stats,tm,NBFS);
  do {
    printf("min_%s: %20.17e\n","TEPS",stats[0]);
    printf("firstquartile_%s: %20.17e\n","TEPS",stats[1]);
    printf("median_%s: %20.17e\n","TEPS",stats[2]);
    printf("thirdquartile_%s: %20.17e\n","TEPS",stats[3]);
    printf("max_%s: %20.17e\n","TEPS",stats[4]);
    if (0) {
      printf("mean_%s: %20.17e\n","TEPS",stats[5]);
      printf("stddev_%s: %20.17e\n","TEPS",stats[6]);
    }
    else {
      printf("harmonic_mean_%s: %20.17e\n","TEPS",stats[7]);
      printf("harmonic_stddev_%s: %20.17e\n","TEPS",stats[8]);
    }
  }while (0);
  hc_mm_free(((hc_workerState *)(pthread_getspecific(wskey))),tm);
  hc_mm_free(((hc_workerState *)(pthread_getspecific(wskey))),stats);
}
typedef struct __hc_find_nv_frame_t__ {
hc_frameHeader header;
struct __hc_find_nv_scope0_frame_t__ {
struct packed_edge *__restrict__ IJ;
int64_t nedge;
int64_t p;
DDF_t *find_nv_ddf;}scope0;
struct __hc_find_nv_scope1_frame_t__ {
p_DDF_list_t listOfFindNv;
int64_t *localMax;
int64_t i;
int64_t c;
int64_t k;
int64_t tmaxvtx;
int64_t lb;}scope1;
struct __hc_find_nv_scope2_frame_t__ {
int64_t lb;
int64_t ub;
DDF_t *d;}scope2;
struct __hc_find_nv_scope3_frame_t__ {
int64_t v0;
int64_t v1;}scope3;}__hc_find_nv_frame_t__;
typedef struct __hc__find_nv_async1__frame_t__ {
hc_frameHeader header;
struct __hc__find_nv_async1__scope0_frame_t__ {
int64_t i;
int64_t lb;
int64_t ub;
struct packed_edge *__restrict__ IJ;
int64_t *localMax;
DDF_t *d;}scope0;}__hc__find_nv_async1__frame_t__;
typedef struct __hc__find_nv_async2__frame_t__ {
hc_frameHeader header;
struct __hc__find_nv_async2__scope0_frame_t__ {
int64_t *localMax;
int64_t p;
DDF_t *find_nv_ddf;}scope0;}__hc__find_nv_async2__frame_t__;
void __hc_find_nv__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _find_nv_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _find_nv_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);

static void find_nv(struct packed_edge *__restrict__ IJ,int64_t nedge,int64_t p,DDF_t *find_nv_ddf)
{
  hc_workerState *ws = current_ws();
  ws -> costatus =  HC_FUNC_COMPLETE;
  struct __hc_find_nv_frame_t__ *__hc_frame__ = (struct __hc_find_nv_frame_t__ *)(hc_init_frame_ws(ws,sizeof(struct __hc_find_nv_frame_t__ ),__hc_find_nv__));
  __hc_frame__ -> scope0.IJ = IJ;
  __hc_frame__ -> scope0.nedge = nedge;
  __hc_frame__ -> scope0.p = p;
  __hc_frame__ -> scope0.find_nv_ddf = find_nv_ddf;
			LOG_INFO(ws, "execute frame: %p, parent: %p of function %s\n", __hc_frame__,((hc_frameHeader*) __hc_frame__)->parent, "__hc_find_nv__" );
  __hc_find_nv__(ws,((hc_frameHeader *)__hc_frame__),0);
  if (ws -> costatus ==  HC_FUNC_COMPLETE) 
    hc_free_unwind_frame(ws);
  return ;
}

void __hc_find_nv__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  struct packed_edge *__restrict__ IJ;
  int64_t nedge;
  int64_t p;
  DDF_t *find_nv_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_async2;
  }
  IJ = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope0.IJ;
  nedge = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope0.nedge;
  p = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope0.p;
  find_nv_ddf = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope0.find_nv_ddf;
  p_DDF_list_t listOfFindNv = __ddfListCreate();
  int64_t *localMax = (int64_t *)(hc_mm_malloc(((hc_workerState *)(pthread_getspecific(wskey))),(((int )p) * sizeof(int64_t ))));
  int64_t i;
  int64_t c = (nedge / p);
  for (i = 0; i < (p - 1); i++) {
    int64_t lb = (i * c);
    int64_t ub = (lb + c);
    DDF_t *d = __ddfCreate();
    __enlist(d,listOfFindNv);
    struct __hc__find_nv_async1__frame_t__ *__hc__find_nv_async1__frame__ = (struct __hc__find_nv_async1__frame_t__ *)(hc_init_frame(sizeof(struct __hc__find_nv_async1__frame_t__ ),_find_nv_async1_));
    __hc__find_nv_async1__frame__ -> scope0.i = ((int64_t )i);
    __hc__find_nv_async1__frame__ -> scope0.lb = ((int64_t )lb);
    __hc__find_nv_async1__frame__ -> scope0.ub = ((int64_t )ub);
    __hc__find_nv_async1__frame__ -> scope0.IJ = ((struct packed_edge *__restrict__ )IJ);
    __hc__find_nv_async1__frame__ -> scope0.localMax = ((int64_t *)localMax);
    __hc__find_nv_async1__frame__ -> scope0.d = ((DDF_t *)d);
    ((hc_frameHeader *)__hc__find_nv_async1__frame__) -> sched_policy = hc_sched_policy(ws);
    increment_counter();
    if (((hc_frameHeader *)__hc__find_nv_async1__frame__) -> sched_policy ==  HELP_FIRST) {
      deque_push_default(__hc__find_nv_async1__frame__);
    }
    else {
      ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope2.lb = lb;
      ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope2.ub = ub;
      ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope2.d = d;
      ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.listOfFindNv = listOfFindNv;
      ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.localMax = localMax;
      ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.i = i;
      ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.c = c;
      ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope0.IJ = IJ;
      ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope0.nedge = nedge;
      ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope0.p = p;
      ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope0.find_nv_ddf = find_nv_ddf;
      __hc_frame__ -> pc = 1;
      deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_find_nv__, and execute async: %p in _find_nv_async1_\n", __hc_frame__, __hc_frame__->pc, __hc__find_nv_async1__frame__);
      _find_nv_async1_(ws,((hc_frameHeader *)__hc__find_nv_async1__frame__),0);
      if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
        return ;
      }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
      if (0) {
        __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_find_nv__\n", printEntry(__hc_frame__, buffer));
        lb = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope2.lb;
        ub = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope2.ub;
        d = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope2.d;
        listOfFindNv = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.listOfFindNv;
        localMax = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.localMax;
        i = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.i;
        c = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.c;
        IJ = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope0.IJ;
        nedge = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope0.nedge;
        p = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope0.p;
        find_nv_ddf = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope0.find_nv_ddf;
      }
    }
  }
  int64_t k;
  int64_t tmaxvtx = (-1);
  int64_t lb = (i * c);
  for (k = lb; k < nedge; k++) {
    int64_t v0 = get_v0_from_edge((IJ + k));
    int64_t v1 = get_v1_from_edge((IJ + k));
    if (v0 > tmaxvtx) 
      tmaxvtx = v0;
    if (v1 > tmaxvtx) 
      tmaxvtx = v1;
  }
  localMax[i] = tmaxvtx;
  struct __hc__find_nv_async2__frame_t__ *__hc__find_nv_async2__frame__ = (struct __hc__find_nv_async2__frame_t__ *)(hc_init_frame(sizeof(struct __hc__find_nv_async2__frame_t__ ),_find_nv_async2_));
  p_await_list_t __hc__find_nv_async2_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__find_nv_async2_awaitList_size = 0 + listOfFindNv -> size + 0;
  __hc__find_nv_async2_awaitList -> array = hc_malloc((1 + __hc__find_nv_async2_awaitList_size) * sizeof(p_DDF_t ));
  __hc__find_nv_async2_awaitList -> array[__hc__find_nv_async2_awaitList_size] = NULL;
  unsigned int __hc__find_nv_async2_awaitList_curr_index = 0;
  __copyInto(__hc__find_nv_async2_awaitList,__hc__find_nv_async2_awaitList_curr_index,listOfFindNv);
  __hc__find_nv_async2_awaitList_curr_index += listOfFindNv -> size;
  __hc__find_nv_async2_awaitList -> waitingFrontier = &__hc__find_nv_async2_awaitList -> array[0];
  __hc__find_nv_async2__frame__ -> header.awaitList = __hc__find_nv_async2_awaitList;
  __hc__find_nv_async2__frame__ -> scope0.localMax = ((int64_t *)localMax);
  __hc__find_nv_async2__frame__ -> scope0.p = ((int64_t )p);
  __hc__find_nv_async2__frame__ -> scope0.find_nv_ddf = ((DDF_t *)find_nv_ddf);
  ((hc_frameHeader *)__hc__find_nv_async2__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__find_nv_async2__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__find_nv_async2__frame__ -> header)) 
      deque_push_default(__hc__find_nv_async2__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.listOfFindNv = listOfFindNv;
    ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.localMax = localMax;
    ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.i = i;
    ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.c = c;
    ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.k = k;
    ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.tmaxvtx = tmaxvtx;
    ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.lb = lb;
    ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope0.IJ = IJ;
    ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope0.nedge = nedge;
    ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope0.p = p;
    ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope0.find_nv_ddf = find_nv_ddf;
    __hc_frame__ -> pc = 2;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_find_nv__, and execute async: %p in _find_nv_async2_\n", __hc_frame__, __hc_frame__->pc, __hc__find_nv_async2__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__find_nv_async2__frame__ -> header)) 
      _find_nv_async2_(ws,((hc_frameHeader *)__hc__find_nv_async2__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async2:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_find_nv__\n", printEntry(__hc_frame__, buffer));
      listOfFindNv = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.listOfFindNv;
      localMax = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.localMax;
      i = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.i;
      c = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.c;
      k = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.k;
      tmaxvtx = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.tmaxvtx;
      lb = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope1.lb;
      IJ = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope0.IJ;
      nedge = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope0.nedge;
      p = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope0.p;
      find_nv_ddf = ((struct __hc_find_nv_frame_t__ *)__hc_frame__) -> scope0.find_nv_ddf;
    }
  }
  ws -> costatus =  HC_FUNC_COMPLETE;
  return ;
}

void _find_nv_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t *localMax;
  int64_t p;
  DDF_t *find_nv_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  localMax = ((struct __hc__find_nv_async2__frame_t__ *)__hc_frame__) -> scope0.localMax;
  p = ((struct __hc__find_nv_async2__frame_t__ *)__hc_frame__) -> scope0.p;
  find_nv_ddf = ((struct __hc__find_nv_async2__frame_t__ *)__hc_frame__) -> scope0.find_nv_ddf;
  maxvtx = (-1);
  int64_t i;
  for (i = 0; i < p; i++) 
    if (localMax[i] > maxvtx) 
      maxvtx = localMax[i];
  hc_mm_free(((hc_workerState *)(pthread_getspecific(wskey))),localMax);
  nv = (1 + maxvtx);
  __ddfPut(find_nv_ddf,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _find_nv_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t i;
  int64_t lb;
  int64_t ub;
  struct packed_edge *__restrict__ IJ;
  int64_t *localMax;
  DDF_t *d;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i = ((struct __hc__find_nv_async1__frame_t__ *)__hc_frame__) -> scope0.i;
  lb = ((struct __hc__find_nv_async1__frame_t__ *)__hc_frame__) -> scope0.lb;
  ub = ((struct __hc__find_nv_async1__frame_t__ *)__hc_frame__) -> scope0.ub;
  IJ = ((struct __hc__find_nv_async1__frame_t__ *)__hc_frame__) -> scope0.IJ;
  localMax = ((struct __hc__find_nv_async1__frame_t__ *)__hc_frame__) -> scope0.localMax;
  d = ((struct __hc__find_nv_async1__frame_t__ *)__hc_frame__) -> scope0.d;
  int64_t k;
  int64_t tmaxvtx = (-1);
  for (k = lb; k < ub; k++) {
    int64_t v0 = get_v0_from_edge((IJ + k));
    int64_t v1 = get_v1_from_edge((IJ + k));
    if (v0 > tmaxvtx) 
      tmaxvtx = v0;
    if (v1 > tmaxvtx) 
      tmaxvtx = v1;
  }
  localMax[i] = tmaxvtx;
  __ddfPut(d,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

static int alloc_graph(int64_t nedge)
{
  sz = (((2 * nv) + 2) * sizeof(int64_t ));
  xoff = (xmalloc_large_ext(sz));
  if (!(xoff != 0)) 
    return -1;
  return 0;
}

static void free_graph()
{
  xfree_large(xadjstore);
  xfree_large(xoff);
}
#define XOFF(k) (xoff[2*(k)])
#define XENDOFF(k) (xoff[1+2*(k)])
typedef struct __hc_prefix_sum_frame_t__ {
hc_frameHeader header;
struct __hc_prefix_sum_scope0_frame_t__ {
int64_t *rt;
int64_t p;
DDF_t *prefix_sum_ddf;}scope0;
struct __hc_prefix_sum_scope1_frame_t__ {
p_DDF_list_t listOfPrefixLocal;
int64_t *buf;
int64_t i;
int64_t c;
int64_t k;
int64_t lb;
int64_t temp;}scope1;
struct __hc_prefix_sum_scope2_frame_t__ {
int64_t lb;
int64_t ub;
DDF_t *d;}scope2;}__hc_prefix_sum_frame_t__;
typedef struct __hc__prefix_sum_async1__frame_t__ {
hc_frameHeader header;
struct __hc__prefix_sum_async1__scope0_frame_t__ {
int64_t lb;
int64_t ub;
int64_t i;
int64_t *buf;
DDF_t *d;}scope0;}__hc__prefix_sum_async1__frame_t__;
typedef struct __hc__prefix_sum_async2__frame_t__ {
hc_frameHeader header;
struct __hc__prefix_sum_async2__scope0_frame_t__ {
int64_t *buf;
int64_t p;
int64_t *rt;
DDF_t *prefix_sum_ddf;}scope0;
struct __hc__prefix_sum_async2__scope1_frame_t__ {
int64_t k;
p_DDF_list_t listOfPrefixGlobal;
int64_t i;
int64_t c;
int64_t lb;
int64_t t1;}scope1;
struct __hc__prefix_sum_async2__scope2_frame_t__ {
int64_t lb;
int64_t ub;
int64_t t1;
DDF_t *d;}scope2;
struct __hc__prefix_sum_async2__scope3_frame_t__ {
int64_t tmp;}scope3;}__hc__prefix_sum_async2__frame_t__;
typedef struct __hc__prefix_sum_async3__frame_t__ {
hc_frameHeader header;
struct __hc__prefix_sum_async3__scope0_frame_t__ {
int64_t lb;
int64_t ub;
int64_t i;
int64_t t1;
DDF_t *d;}scope0;}__hc__prefix_sum_async3__frame_t__;
typedef struct __hc__prefix_sum_async4__frame_t__ {
hc_frameHeader header;
struct __hc__prefix_sum_async4__scope0_frame_t__ {
int64_t *buf;
int64_t p;
int64_t *rt;
DDF_t *prefix_sum_ddf;}scope0;}__hc__prefix_sum_async4__frame_t__;
void __hc_prefix_sum__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _prefix_sum_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _prefix_sum_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _prefix_sum_async3_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _prefix_sum_async4_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);

static void prefix_sum(int64_t *rt,int64_t p,DDF_t *prefix_sum_ddf)
{
  hc_workerState *ws = current_ws();
  ws -> costatus =  HC_FUNC_COMPLETE;
  struct __hc_prefix_sum_frame_t__ *__hc_frame__ = (struct __hc_prefix_sum_frame_t__ *)(hc_init_frame_ws(ws,sizeof(struct __hc_prefix_sum_frame_t__ ),__hc_prefix_sum__));
  __hc_frame__ -> scope0.rt = rt;
  __hc_frame__ -> scope0.p = p;
  __hc_frame__ -> scope0.prefix_sum_ddf = prefix_sum_ddf;
			LOG_INFO(ws, "execute frame: %p, parent: %p of function %s\n", __hc_frame__,((hc_frameHeader*) __hc_frame__)->parent, "__hc_prefix_sum__" );
  __hc_prefix_sum__(ws,((hc_frameHeader *)__hc_frame__),0);
  if (ws -> costatus ==  HC_FUNC_COMPLETE) 
    hc_free_unwind_frame(ws);
  return ;
}

void __hc_prefix_sum__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t *rt;
  int64_t p;
  DDF_t *prefix_sum_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_async2;
  }
  rt = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope0.rt;
  p = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope0.p;
  prefix_sum_ddf = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope0.prefix_sum_ddf;
  p_DDF_list_t listOfPrefixLocal = __ddfListCreate();
  int64_t *buf = (int64_t *)(hc_mm_malloc(((hc_workerState *)(pthread_getspecific(wskey))),(((int )p) * sizeof(int64_t ))));
  if (!(buf != 0)) {
    perror("hc_malloc for prefix-sum hosed");
    abort();
  }
  int64_t i;
  int64_t c = (nv / p);
  for (i = 0; i < (p - 1); i++) {
    int64_t lb = (i * c);
    int64_t ub = (lb + c);
    DDF_t *d = __ddfCreate();
    __enlist(d,listOfPrefixLocal);
    struct __hc__prefix_sum_async1__frame_t__ *__hc__prefix_sum_async1__frame__ = (struct __hc__prefix_sum_async1__frame_t__ *)(hc_init_frame(sizeof(struct __hc__prefix_sum_async1__frame_t__ ),_prefix_sum_async1_));
    __hc__prefix_sum_async1__frame__ -> scope0.lb = ((int64_t )lb);
    __hc__prefix_sum_async1__frame__ -> scope0.ub = ((int64_t )ub);
    __hc__prefix_sum_async1__frame__ -> scope0.i = ((int64_t )i);
    __hc__prefix_sum_async1__frame__ -> scope0.buf = ((int64_t *)buf);
    __hc__prefix_sum_async1__frame__ -> scope0.d = ((DDF_t *)d);
    ((hc_frameHeader *)__hc__prefix_sum_async1__frame__) -> sched_policy = hc_sched_policy(ws);
    increment_counter();
    if (((hc_frameHeader *)__hc__prefix_sum_async1__frame__) -> sched_policy ==  HELP_FIRST) {
      deque_push_default(__hc__prefix_sum_async1__frame__);
    }
    else {
      ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope2.lb = lb;
      ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope2.ub = ub;
      ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope2.d = d;
      ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.listOfPrefixLocal = listOfPrefixLocal;
      ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.buf = buf;
      ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.i = i;
      ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.c = c;
      ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope0.rt = rt;
      ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope0.p = p;
      ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope0.prefix_sum_ddf = prefix_sum_ddf;
      __hc_frame__ -> pc = 1;
      deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_prefix_sum__, and execute async: %p in _prefix_sum_async1_\n", __hc_frame__, __hc_frame__->pc, __hc__prefix_sum_async1__frame__);
      _prefix_sum_async1_(ws,((hc_frameHeader *)__hc__prefix_sum_async1__frame__),0);
      if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
        return ;
      }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
      if (0) {
        __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_prefix_sum__\n", printEntry(__hc_frame__, buffer));
        lb = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope2.lb;
        ub = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope2.ub;
        d = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope2.d;
        listOfPrefixLocal = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.listOfPrefixLocal;
        buf = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.buf;
        i = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.i;
        c = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.c;
        rt = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope0.rt;
        p = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope0.p;
        prefix_sum_ddf = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope0.prefix_sum_ddf;
      }
    }
  }
  int64_t k;
  int64_t lb = (i * c);
  int64_t temp = 0;
  for (k = lb; k < nv; k++) {
    temp += xoff[2 * k];
  }
  buf[i] = temp;
  struct __hc__prefix_sum_async2__frame_t__ *__hc__prefix_sum_async2__frame__ = (struct __hc__prefix_sum_async2__frame_t__ *)(hc_init_frame(sizeof(struct __hc__prefix_sum_async2__frame_t__ ),_prefix_sum_async2_));
  p_await_list_t __hc__prefix_sum_async2_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__prefix_sum_async2_awaitList_size = 0 + listOfPrefixLocal -> size + 0;
  __hc__prefix_sum_async2_awaitList -> array = hc_malloc((1 + __hc__prefix_sum_async2_awaitList_size) * sizeof(p_DDF_t ));
  __hc__prefix_sum_async2_awaitList -> array[__hc__prefix_sum_async2_awaitList_size] = NULL;
  unsigned int __hc__prefix_sum_async2_awaitList_curr_index = 0;
  __copyInto(__hc__prefix_sum_async2_awaitList,__hc__prefix_sum_async2_awaitList_curr_index,listOfPrefixLocal);
  __hc__prefix_sum_async2_awaitList_curr_index += listOfPrefixLocal -> size;
  __hc__prefix_sum_async2_awaitList -> waitingFrontier = &__hc__prefix_sum_async2_awaitList -> array[0];
  __hc__prefix_sum_async2__frame__ -> header.awaitList = __hc__prefix_sum_async2_awaitList;
  __hc__prefix_sum_async2__frame__ -> scope0.buf = ((int64_t *)buf);
  __hc__prefix_sum_async2__frame__ -> scope0.p = ((int64_t )p);
  __hc__prefix_sum_async2__frame__ -> scope0.rt = ((int64_t *)rt);
  __hc__prefix_sum_async2__frame__ -> scope0.prefix_sum_ddf = ((DDF_t *)prefix_sum_ddf);
  ((hc_frameHeader *)__hc__prefix_sum_async2__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__prefix_sum_async2__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__prefix_sum_async2__frame__ -> header)) 
      deque_push_default(__hc__prefix_sum_async2__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.listOfPrefixLocal = listOfPrefixLocal;
    ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.buf = buf;
    ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.i = i;
    ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.c = c;
    ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.k = k;
    ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.lb = lb;
    ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.temp = temp;
    ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope0.rt = rt;
    ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope0.p = p;
    ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope0.prefix_sum_ddf = prefix_sum_ddf;
    __hc_frame__ -> pc = 2;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_prefix_sum__, and execute async: %p in _prefix_sum_async2_\n", __hc_frame__, __hc_frame__->pc, __hc__prefix_sum_async2__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__prefix_sum_async2__frame__ -> header)) 
      _prefix_sum_async2_(ws,((hc_frameHeader *)__hc__prefix_sum_async2__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async2:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_prefix_sum__\n", printEntry(__hc_frame__, buffer));
      listOfPrefixLocal = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.listOfPrefixLocal;
      buf = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.buf;
      i = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.i;
      c = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.c;
      k = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.k;
      lb = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.lb;
      temp = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope1.temp;
      rt = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope0.rt;
      p = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope0.p;
      prefix_sum_ddf = ((struct __hc_prefix_sum_frame_t__ *)__hc_frame__) -> scope0.prefix_sum_ddf;
    }
  }
  ws -> costatus =  HC_FUNC_COMPLETE;
  return ;
}

void _prefix_sum_async4_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t *buf;
  int64_t p;
  int64_t *rt;
  DDF_t *prefix_sum_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  buf = ((struct __hc__prefix_sum_async4__frame_t__ *)__hc_frame__) -> scope0.buf;
  p = ((struct __hc__prefix_sum_async4__frame_t__ *)__hc_frame__) -> scope0.p;
  rt = ((struct __hc__prefix_sum_async4__frame_t__ *)__hc_frame__) -> scope0.rt;
  prefix_sum_ddf = ((struct __hc__prefix_sum_async4__frame_t__ *)__hc_frame__) -> scope0.prefix_sum_ddf;
   *rt = buf[p - 1];
  hc_mm_free(((hc_workerState *)(pthread_getspecific(wskey))),buf);
  __ddfPut(prefix_sum_ddf,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _prefix_sum_async3_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t lb;
  int64_t ub;
  int64_t i;
  int64_t t1;
  DDF_t *d;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  lb = ((struct __hc__prefix_sum_async3__frame_t__ *)__hc_frame__) -> scope0.lb;
  ub = ((struct __hc__prefix_sum_async3__frame_t__ *)__hc_frame__) -> scope0.ub;
  i = ((struct __hc__prefix_sum_async3__frame_t__ *)__hc_frame__) -> scope0.i;
  t1 = ((struct __hc__prefix_sum_async3__frame_t__ *)__hc_frame__) -> scope0.t1;
  d = ((struct __hc__prefix_sum_async3__frame_t__ *)__hc_frame__) -> scope0.d;
  int64_t k;
  for (k = lb; k < ub; k++) {
    int64_t tmp = xoff[2 * k];
    xoff[2 * k] = t1;
    t1 += tmp;
  }
  __ddfPut(d,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _prefix_sum_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t *buf;
  int64_t p;
  int64_t *rt;
  DDF_t *prefix_sum_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_async2;
  }
  buf = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope0.buf;
  p = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope0.p;
  rt = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope0.rt;
  prefix_sum_ddf = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope0.prefix_sum_ddf;
  int64_t k;
  for (k = 1; k < p; ++k) {
    buf[k] += buf[k - 1];
  }
  p_DDF_list_t listOfPrefixGlobal = __ddfListCreate();
  int64_t i;
  int64_t c = (nv / p);
  for (i = 0; i < (p - 1); i++) {
    int64_t lb = (i * c);
    int64_t ub = (lb + c);
    int64_t t1 = (i > 0)?buf[i - 1] : 0;
    DDF_t *d = __ddfCreate();
    __enlist(d,listOfPrefixGlobal);
    struct __hc__prefix_sum_async3__frame_t__ *__hc__prefix_sum_async3__frame__ = (struct __hc__prefix_sum_async3__frame_t__ *)(hc_init_frame(sizeof(struct __hc__prefix_sum_async3__frame_t__ ),_prefix_sum_async3_));
    __hc__prefix_sum_async3__frame__ -> scope0.lb = ((int64_t )lb);
    __hc__prefix_sum_async3__frame__ -> scope0.ub = ((int64_t )ub);
    __hc__prefix_sum_async3__frame__ -> scope0.i = ((int64_t )i);
    __hc__prefix_sum_async3__frame__ -> scope0.t1 = ((int64_t )t1);
    __hc__prefix_sum_async3__frame__ -> scope0.d = ((DDF_t *)d);
    ((hc_frameHeader *)__hc__prefix_sum_async3__frame__) -> sched_policy = hc_sched_policy(ws);
    increment_counter();
    if (((hc_frameHeader *)__hc__prefix_sum_async3__frame__) -> sched_policy ==  HELP_FIRST) {
      deque_push_default(__hc__prefix_sum_async3__frame__);
    }
    else {
      ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope2.lb = lb;
      ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope2.ub = ub;
      ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope2.t1 = t1;
      ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope2.d = d;
      ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope1.k = k;
      ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope1.listOfPrefixGlobal = listOfPrefixGlobal;
      ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope1.i = i;
      ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope1.c = c;
      ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope0.buf = buf;
      ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope0.p = p;
      ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope0.rt = rt;
      ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope0.prefix_sum_ddf = prefix_sum_ddf;
      __hc_frame__ -> pc = 1;
      deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _prefix_sum_async2_, and execute async: %p in _prefix_sum_async3_\n", __hc_frame__, __hc_frame__->pc, __hc__prefix_sum_async3__frame__);
      _prefix_sum_async3_(ws,((hc_frameHeader *)__hc__prefix_sum_async3__frame__),0);
      if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
        return ;
      }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
      if (0) {
        __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _prefix_sum_async2_\n", printEntry(__hc_frame__, buffer));
        lb = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope2.lb;
        ub = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope2.ub;
        t1 = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope2.t1;
        d = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope2.d;
        k = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope1.k;
        listOfPrefixGlobal = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope1.listOfPrefixGlobal;
        i = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope1.i;
        c = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope1.c;
        buf = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope0.buf;
        p = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope0.p;
        rt = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope0.rt;
        prefix_sum_ddf = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope0.prefix_sum_ddf;
      }
    }
  }
  int64_t lb = (i * c);
  int64_t t1 = buf[i - 1];
  for (k = lb; k < nv; k++) {
    int64_t tmp = xoff[2 * k];
    xoff[2 * k] = t1;
    t1 += tmp;
  }
/* Write back value for INOUT/OUT arguments */
  struct __hc__prefix_sum_async4__frame_t__ *__hc__prefix_sum_async4__frame__ = (struct __hc__prefix_sum_async4__frame_t__ *)(hc_init_frame(sizeof(struct __hc__prefix_sum_async4__frame_t__ ),_prefix_sum_async4_));
  p_await_list_t __hc__prefix_sum_async4_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__prefix_sum_async4_awaitList_size = 0 + listOfPrefixGlobal -> size + 0;
  __hc__prefix_sum_async4_awaitList -> array = hc_malloc((1 + __hc__prefix_sum_async4_awaitList_size) * sizeof(p_DDF_t ));
  __hc__prefix_sum_async4_awaitList -> array[__hc__prefix_sum_async4_awaitList_size] = NULL;
  unsigned int __hc__prefix_sum_async4_awaitList_curr_index = 0;
  __copyInto(__hc__prefix_sum_async4_awaitList,__hc__prefix_sum_async4_awaitList_curr_index,listOfPrefixGlobal);
  __hc__prefix_sum_async4_awaitList_curr_index += listOfPrefixGlobal -> size;
  __hc__prefix_sum_async4_awaitList -> waitingFrontier = &__hc__prefix_sum_async4_awaitList -> array[0];
  __hc__prefix_sum_async4__frame__ -> header.awaitList = __hc__prefix_sum_async4_awaitList;
  __hc__prefix_sum_async4__frame__ -> scope0.buf = ((int64_t *)buf);
  __hc__prefix_sum_async4__frame__ -> scope0.p = ((int64_t )p);
  __hc__prefix_sum_async4__frame__ -> scope0.rt = ((int64_t *)rt);
  __hc__prefix_sum_async4__frame__ -> scope0.prefix_sum_ddf = ((DDF_t *)prefix_sum_ddf);
  ((hc_frameHeader *)__hc__prefix_sum_async4__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__prefix_sum_async4__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__prefix_sum_async4__frame__ -> header)) 
      deque_push_default(__hc__prefix_sum_async4__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope1.k = k;
    ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope1.listOfPrefixGlobal = listOfPrefixGlobal;
    ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope1.i = i;
    ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope1.c = c;
    ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope1.lb = lb;
    ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope1.t1 = t1;
    ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope0.buf = buf;
    ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope0.p = p;
    ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope0.rt = rt;
    ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope0.prefix_sum_ddf = prefix_sum_ddf;
    __hc_frame__ -> pc = 2;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _prefix_sum_async2_, and execute async: %p in _prefix_sum_async4_\n", __hc_frame__, __hc_frame__->pc, __hc__prefix_sum_async4__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__prefix_sum_async4__frame__ -> header)) 
      _prefix_sum_async4_(ws,((hc_frameHeader *)__hc__prefix_sum_async4__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async2:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _prefix_sum_async2_\n", printEntry(__hc_frame__, buffer));
      k = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope1.k;
      listOfPrefixGlobal = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope1.listOfPrefixGlobal;
      i = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope1.i;
      c = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope1.c;
      lb = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope1.lb;
      t1 = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope1.t1;
      buf = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope0.buf;
      p = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope0.p;
      rt = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope0.rt;
      prefix_sum_ddf = ((struct __hc__prefix_sum_async2__frame_t__ *)__hc_frame__) -> scope0.prefix_sum_ddf;
    }
  }
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _prefix_sum_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t lb;
  int64_t ub;
  int64_t i;
  int64_t *buf;
  DDF_t *d;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  lb = ((struct __hc__prefix_sum_async1__frame_t__ *)__hc_frame__) -> scope0.lb;
  ub = ((struct __hc__prefix_sum_async1__frame_t__ *)__hc_frame__) -> scope0.ub;
  i = ((struct __hc__prefix_sum_async1__frame_t__ *)__hc_frame__) -> scope0.i;
  buf = ((struct __hc__prefix_sum_async1__frame_t__ *)__hc_frame__) -> scope0.buf;
  d = ((struct __hc__prefix_sum_async1__frame_t__ *)__hc_frame__) -> scope0.d;
  int64_t k;
  int64_t temp = 0;
  for (k = lb; k < ub; k++) {
    temp += xoff[2 * k];
  }
  buf[i] = temp;
  __ddfPut(d,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}
typedef struct __hc_setup_deg_off_frame_t__ {
hc_frameHeader header;
struct __hc_setup_deg_off_scope0_frame_t__ {
struct packed_edge *__restrict__ IJ;
int64_t nedge;
int64_t p;
int *rt;
DDF_t *setup_deg_ddf;}scope0;
struct __hc_setup_deg_off_scope1_frame_t__ {
int64_t *accum;
p_DDF_list_t listOfXoffClear;
int64_t i;
int64_t c;
int64_t k;
int64_t lb;}scope1;
struct __hc_setup_deg_off_scope2_frame_t__ {
int64_t lb;
int64_t ub;
DDF_t *d;}scope2;}__hc_setup_deg_off_frame_t__;
typedef struct __hc__setup_deg_off_async1__frame_t__ {
hc_frameHeader header;
struct __hc__setup_deg_off_async1__scope0_frame_t__ {
int64_t lb;
int64_t ub;
DDF_t *d;}scope0;}__hc__setup_deg_off_async1__frame_t__;
typedef struct __hc__setup_deg_off_async2__frame_t__ {
hc_frameHeader header;
struct __hc__setup_deg_off_async2__scope0_frame_t__ {
struct packed_edge *__restrict__ IJ;
int64_t nedge;
int64_t p;
int64_t *accum;
int *rt;
DDF_t *setup_deg_ddf;}scope0;
struct __hc__setup_deg_off_async2__scope1_frame_t__ {
p_DDF_list_t listOfXoffSet;
int64_t i;
int64_t c;
int64_t k;
int64_t lb;}scope1;
struct __hc__setup_deg_off_async2__scope2_frame_t__ {
int64_t lb;
int64_t ub;
DDF_t *d;}scope2;
struct __hc__setup_deg_off_async2__scope3_frame_t__ {
int64_t v0;
int64_t v1;}scope3;}__hc__setup_deg_off_async2__frame_t__;
typedef struct __hc__setup_deg_off_async3__frame_t__ {
hc_frameHeader header;
struct __hc__setup_deg_off_async3__scope0_frame_t__ {
int64_t lb;
int64_t ub;
struct packed_edge *__restrict__ IJ;
DDF_t *d;}scope0;}__hc__setup_deg_off_async3__frame_t__;
typedef struct __hc__setup_deg_off_async4__frame_t__ {
hc_frameHeader header;
struct __hc__setup_deg_off_async4__scope0_frame_t__ {
int64_t p;
int64_t *accum;
int *rt;
DDF_t *setup_deg_ddf;}scope0;
struct __hc__setup_deg_off_async4__scope1_frame_t__ {
p_DDF_list_t listOfXoffUpdate;
int64_t i;
int64_t c;
int64_t k;
int64_t lb;}scope1;
struct __hc__setup_deg_off_async4__scope2_frame_t__ {
int64_t lb;
int64_t ub;
DDF_t *d;}scope2;}__hc__setup_deg_off_async4__frame_t__;
typedef struct __hc__setup_deg_off_async5__frame_t__ {
hc_frameHeader header;
struct __hc__setup_deg_off_async5__scope0_frame_t__ {
int64_t lb;
int64_t ub;
DDF_t *d;}scope0;}__hc__setup_deg_off_async5__frame_t__;
typedef struct __hc__setup_deg_off_async6__frame_t__ {
hc_frameHeader header;
struct __hc__setup_deg_off_async6__scope0_frame_t__ {
int64_t p;
int64_t *accum;
int *rt;
DDF_t *setup_deg_ddf;}scope0;
struct __hc__setup_deg_off_async6__scope1_frame_t__ {
DDF_t *prefix_sum_ddf;}scope1;}__hc__setup_deg_off_async6__frame_t__;
typedef struct __hc__setup_deg_off_async7__frame_t__ {
hc_frameHeader header;
struct __hc__setup_deg_off_async7__scope0_frame_t__ {
int64_t p;
int64_t *accum;
int *rt;
DDF_t *setup_deg_ddf;}scope0;
struct __hc__setup_deg_off_async7__scope1_frame_t__ {
p_DDF_list_t listOfXendoffSet;
int64_t i;
int64_t c;
int64_t k;
int64_t lb;}scope1;
struct __hc__setup_deg_off_async7__scope2_frame_t__ {
int64_t lb;
int64_t ub;
DDF_t *d;}scope2;}__hc__setup_deg_off_async7__frame_t__;
typedef struct __hc__setup_deg_off_async8__frame_t__ {
hc_frameHeader header;
struct __hc__setup_deg_off_async8__scope0_frame_t__ {
int64_t lb;
int64_t ub;
DDF_t *d;}scope0;}__hc__setup_deg_off_async8__frame_t__;
typedef struct __hc__setup_deg_off_async9__frame_t__ {
hc_frameHeader header;
struct __hc__setup_deg_off_async9__scope0_frame_t__ {
int *rt;
int64_t *accum;
DDF_t *setup_deg_ddf;}scope0;}__hc__setup_deg_off_async9__frame_t__;
void __hc_setup_deg_off__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _setup_deg_off_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _setup_deg_off_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _setup_deg_off_async3_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _setup_deg_off_async4_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _setup_deg_off_async5_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _setup_deg_off_async6_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _setup_deg_off_async7_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _setup_deg_off_async8_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _setup_deg_off_async9_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);

static void setup_deg_off(struct packed_edge *__restrict__ IJ,int64_t nedge,int64_t p,int *rt,DDF_t *setup_deg_ddf)
{
  hc_workerState *ws = current_ws();
  ws -> costatus =  HC_FUNC_COMPLETE;
  struct __hc_setup_deg_off_frame_t__ *__hc_frame__ = (struct __hc_setup_deg_off_frame_t__ *)(hc_init_frame_ws(ws,sizeof(struct __hc_setup_deg_off_frame_t__ ),__hc_setup_deg_off__));
  __hc_frame__ -> scope0.IJ = IJ;
  __hc_frame__ -> scope0.nedge = nedge;
  __hc_frame__ -> scope0.p = p;
  __hc_frame__ -> scope0.rt = rt;
  __hc_frame__ -> scope0.setup_deg_ddf = setup_deg_ddf;
			LOG_INFO(ws, "execute frame: %p, parent: %p of function %s\n", __hc_frame__,((hc_frameHeader*) __hc_frame__)->parent, "__hc_setup_deg_off__" );
  __hc_setup_deg_off__(ws,((hc_frameHeader *)__hc_frame__),0);
  if (ws -> costatus ==  HC_FUNC_COMPLETE) 
    hc_free_unwind_frame(ws);
  return ;
}

void __hc_setup_deg_off__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  struct packed_edge *__restrict__ IJ;
  int64_t nedge;
  int64_t p;
  int *rt;
  DDF_t *setup_deg_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_async2;
  }
  IJ = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.IJ;
  nedge = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.nedge;
  p = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.p;
  rt = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.rt;
  setup_deg_ddf = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf;
  xadj = ((int64_t *)((void *)0));
  int64_t *accum = (int64_t *)(hc_mm_malloc(((hc_workerState *)(pthread_getspecific(wskey))),(sizeof(int64_t ))));
   *accum = 0;
  p_DDF_list_t listOfXoffClear = __ddfListCreate();
  int64_t i = 0;
  int64_t c = (((2 * nv) + 2) / p);
  for (i = 0; i < (p - 1); i++) {
    int64_t lb = (i * c);
    int64_t ub = (lb + c);
    DDF_t *d = __ddfCreate();
    __enlist(d,listOfXoffClear);
    struct __hc__setup_deg_off_async1__frame_t__ *__hc__setup_deg_off_async1__frame__ = (struct __hc__setup_deg_off_async1__frame_t__ *)(hc_init_frame(sizeof(struct __hc__setup_deg_off_async1__frame_t__ ),_setup_deg_off_async1_));
    __hc__setup_deg_off_async1__frame__ -> scope0.lb = ((int64_t )lb);
    __hc__setup_deg_off_async1__frame__ -> scope0.ub = ((int64_t )ub);
    __hc__setup_deg_off_async1__frame__ -> scope0.d = ((DDF_t *)d);
    ((hc_frameHeader *)__hc__setup_deg_off_async1__frame__) -> sched_policy = hc_sched_policy(ws);
    increment_counter();
    if (((hc_frameHeader *)__hc__setup_deg_off_async1__frame__) -> sched_policy ==  HELP_FIRST) {
      deque_push_default(__hc__setup_deg_off_async1__frame__);
    }
    else {
      ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope2.lb = lb;
      ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope2.ub = ub;
      ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope2.d = d;
      ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope1.accum = accum;
      ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope1.listOfXoffClear = listOfXoffClear;
      ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope1.i = i;
      ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope1.c = c;
      ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.IJ = IJ;
      ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.nedge = nedge;
      ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.p = p;
      ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.rt = rt;
      ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf = setup_deg_ddf;
      __hc_frame__ -> pc = 1;
      deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_setup_deg_off__, and execute async: %p in _setup_deg_off_async1_\n", __hc_frame__, __hc_frame__->pc, __hc__setup_deg_off_async1__frame__);
      _setup_deg_off_async1_(ws,((hc_frameHeader *)__hc__setup_deg_off_async1__frame__),0);
      if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
        return ;
      }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
      if (0) {
        __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_setup_deg_off__\n", printEntry(__hc_frame__, buffer));
        lb = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope2.lb;
        ub = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope2.ub;
        d = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope2.d;
        accum = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope1.accum;
        listOfXoffClear = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope1.listOfXoffClear;
        i = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope1.i;
        c = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope1.c;
        IJ = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.IJ;
        nedge = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.nedge;
        p = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.p;
        rt = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.rt;
        setup_deg_ddf = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf;
      }
    }
  }
  int64_t k;
  int64_t lb = (i * c);
  for (k = lb; k < ((2 * nv) + 2); k++) {
    xoff[k] = 0;
  }
  struct __hc__setup_deg_off_async2__frame_t__ *__hc__setup_deg_off_async2__frame__ = (struct __hc__setup_deg_off_async2__frame_t__ *)(hc_init_frame(sizeof(struct __hc__setup_deg_off_async2__frame_t__ ),_setup_deg_off_async2_));
  p_await_list_t __hc__setup_deg_off_async2_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__setup_deg_off_async2_awaitList_size = 0 + listOfXoffClear -> size + 0;
  __hc__setup_deg_off_async2_awaitList -> array = hc_malloc((1 + __hc__setup_deg_off_async2_awaitList_size) * sizeof(p_DDF_t ));
  __hc__setup_deg_off_async2_awaitList -> array[__hc__setup_deg_off_async2_awaitList_size] = NULL;
  unsigned int __hc__setup_deg_off_async2_awaitList_curr_index = 0;
  __copyInto(__hc__setup_deg_off_async2_awaitList,__hc__setup_deg_off_async2_awaitList_curr_index,listOfXoffClear);
  __hc__setup_deg_off_async2_awaitList_curr_index += listOfXoffClear -> size;
  __hc__setup_deg_off_async2_awaitList -> waitingFrontier = &__hc__setup_deg_off_async2_awaitList -> array[0];
  __hc__setup_deg_off_async2__frame__ -> header.awaitList = __hc__setup_deg_off_async2_awaitList;
  __hc__setup_deg_off_async2__frame__ -> scope0.IJ = ((struct packed_edge *__restrict__ )IJ);
  __hc__setup_deg_off_async2__frame__ -> scope0.nedge = ((int64_t )nedge);
  __hc__setup_deg_off_async2__frame__ -> scope0.p = ((int64_t )p);
  __hc__setup_deg_off_async2__frame__ -> scope0.accum = ((int64_t *)accum);
  __hc__setup_deg_off_async2__frame__ -> scope0.rt = ((int *)rt);
  __hc__setup_deg_off_async2__frame__ -> scope0.setup_deg_ddf = ((DDF_t *)setup_deg_ddf);
  ((hc_frameHeader *)__hc__setup_deg_off_async2__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__setup_deg_off_async2__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__setup_deg_off_async2__frame__ -> header)) 
      deque_push_default(__hc__setup_deg_off_async2__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope1.accum = accum;
    ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope1.listOfXoffClear = listOfXoffClear;
    ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope1.i = i;
    ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope1.c = c;
    ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope1.k = k;
    ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope1.lb = lb;
    ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.IJ = IJ;
    ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.nedge = nedge;
    ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.p = p;
    ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.rt = rt;
    ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf = setup_deg_ddf;
    __hc_frame__ -> pc = 2;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_setup_deg_off__, and execute async: %p in _setup_deg_off_async2_\n", __hc_frame__, __hc_frame__->pc, __hc__setup_deg_off_async2__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__setup_deg_off_async2__frame__ -> header)) 
      _setup_deg_off_async2_(ws,((hc_frameHeader *)__hc__setup_deg_off_async2__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async2:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_setup_deg_off__\n", printEntry(__hc_frame__, buffer));
      accum = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope1.accum;
      listOfXoffClear = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope1.listOfXoffClear;
      i = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope1.i;
      c = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope1.c;
      k = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope1.k;
      lb = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope1.lb;
      IJ = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.IJ;
      nedge = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.nedge;
      p = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.p;
      rt = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.rt;
      setup_deg_ddf = ((struct __hc_setup_deg_off_frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf;
    }
  }
  ws -> costatus =  HC_FUNC_COMPLETE;
  return ;
}

void _setup_deg_off_async9_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int *rt;
  int64_t *accum;
  DDF_t *setup_deg_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  rt = ((struct __hc__setup_deg_off_async9__frame_t__ *)__hc_frame__) -> scope0.rt;
  accum = ((struct __hc__setup_deg_off_async9__frame_t__ *)__hc_frame__) -> scope0.accum;
  setup_deg_ddf = ((struct __hc__setup_deg_off_async9__frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf;
  int err = 0;
  xoff[2 * nv] =  *accum;
  if (!((xadjstore = (xmalloc_large_ext(((xoff[2 * nv] + 2) * sizeof(( *xadjstore)))))) != 0)) 
    err = -1;
  if (!(err != 0)) {
/* Cheat and permit xadj[-1] to work. */
    xadj = (xadjstore + 2);
    int64_t k;
    for (k = 0; k < (xoff[2 * nv] + 2); ++k) 
      xadjstore[k] = (-1);
  }
   *rt = !(xadj != 0);
  __ddfPut(setup_deg_ddf,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _setup_deg_off_async8_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t lb;
  int64_t ub;
  DDF_t *d;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  lb = ((struct __hc__setup_deg_off_async8__frame_t__ *)__hc_frame__) -> scope0.lb;
  ub = ((struct __hc__setup_deg_off_async8__frame_t__ *)__hc_frame__) -> scope0.ub;
  d = ((struct __hc__setup_deg_off_async8__frame_t__ *)__hc_frame__) -> scope0.d;
  int64_t k;
  for (k = lb; k < ub; k++) {
    xoff[1 + (2 * k)] = xoff[2 * k];
  }
  __ddfPut(d,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _setup_deg_off_async7_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t p;
  int64_t *accum;
  int *rt;
  DDF_t *setup_deg_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_async2;
  }
  p = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope0.p;
  accum = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope0.accum;
  rt = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope0.rt;
  setup_deg_ddf = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf;
  p_DDF_list_t listOfXendoffSet = __ddfListCreate();
  int64_t i = 0;
  int64_t c = (nv / p);
  for (i = 0; i < (p - 1); i++) {
    int64_t lb = (i * c);
    int64_t ub = (lb + c);
    DDF_t *d = __ddfCreate();
    __enlist(d,listOfXendoffSet);
    struct __hc__setup_deg_off_async8__frame_t__ *__hc__setup_deg_off_async8__frame__ = (struct __hc__setup_deg_off_async8__frame_t__ *)(hc_init_frame(sizeof(struct __hc__setup_deg_off_async8__frame_t__ ),_setup_deg_off_async8_));
    __hc__setup_deg_off_async8__frame__ -> scope0.lb = ((int64_t )lb);
    __hc__setup_deg_off_async8__frame__ -> scope0.ub = ((int64_t )ub);
    __hc__setup_deg_off_async8__frame__ -> scope0.d = ((DDF_t *)d);
    ((hc_frameHeader *)__hc__setup_deg_off_async8__frame__) -> sched_policy = hc_sched_policy(ws);
    increment_counter();
    if (((hc_frameHeader *)__hc__setup_deg_off_async8__frame__) -> sched_policy ==  HELP_FIRST) {
      deque_push_default(__hc__setup_deg_off_async8__frame__);
    }
    else {
      ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope2.lb = lb;
      ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope2.ub = ub;
      ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope2.d = d;
      ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope1.listOfXendoffSet = listOfXendoffSet;
      ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope1.i = i;
      ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope1.c = c;
      ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope0.p = p;
      ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope0.accum = accum;
      ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope0.rt = rt;
      ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf = setup_deg_ddf;
      __hc_frame__ -> pc = 1;
      deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _setup_deg_off_async7_, and execute async: %p in _setup_deg_off_async8_\n", __hc_frame__, __hc_frame__->pc, __hc__setup_deg_off_async8__frame__);
      _setup_deg_off_async8_(ws,((hc_frameHeader *)__hc__setup_deg_off_async8__frame__),0);
      if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
        return ;
      }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
      if (0) {
        __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _setup_deg_off_async7_\n", printEntry(__hc_frame__, buffer));
        lb = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope2.lb;
        ub = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope2.ub;
        d = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope2.d;
        listOfXendoffSet = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope1.listOfXendoffSet;
        i = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope1.i;
        c = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope1.c;
        p = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope0.p;
        accum = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope0.accum;
        rt = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope0.rt;
        setup_deg_ddf = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf;
      }
    }
  }
  int64_t k;
  int64_t lb = (i * c);
  for (k = lb; k < nv; k++) {
    xoff[1 + (2 * k)] = xoff[2 * k];
  }
/* Write back value for INOUT/OUT arguments */
  struct __hc__setup_deg_off_async9__frame_t__ *__hc__setup_deg_off_async9__frame__ = (struct __hc__setup_deg_off_async9__frame_t__ *)(hc_init_frame(sizeof(struct __hc__setup_deg_off_async9__frame_t__ ),_setup_deg_off_async9_));
  p_await_list_t __hc__setup_deg_off_async9_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__setup_deg_off_async9_awaitList_size = 0 + listOfXendoffSet -> size + 0;
  __hc__setup_deg_off_async9_awaitList -> array = hc_malloc((1 + __hc__setup_deg_off_async9_awaitList_size) * sizeof(p_DDF_t ));
  __hc__setup_deg_off_async9_awaitList -> array[__hc__setup_deg_off_async9_awaitList_size] = NULL;
  unsigned int __hc__setup_deg_off_async9_awaitList_curr_index = 0;
  __copyInto(__hc__setup_deg_off_async9_awaitList,__hc__setup_deg_off_async9_awaitList_curr_index,listOfXendoffSet);
  __hc__setup_deg_off_async9_awaitList_curr_index += listOfXendoffSet -> size;
  __hc__setup_deg_off_async9_awaitList -> waitingFrontier = &__hc__setup_deg_off_async9_awaitList -> array[0];
  __hc__setup_deg_off_async9__frame__ -> header.awaitList = __hc__setup_deg_off_async9_awaitList;
  __hc__setup_deg_off_async9__frame__ -> scope0.rt = ((int *)rt);
  __hc__setup_deg_off_async9__frame__ -> scope0.accum = ((int64_t *)accum);
  __hc__setup_deg_off_async9__frame__ -> scope0.setup_deg_ddf = ((DDF_t *)setup_deg_ddf);
  ((hc_frameHeader *)__hc__setup_deg_off_async9__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__setup_deg_off_async9__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__setup_deg_off_async9__frame__ -> header)) 
      deque_push_default(__hc__setup_deg_off_async9__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope1.listOfXendoffSet = listOfXendoffSet;
    ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope1.i = i;
    ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope1.c = c;
    ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope1.k = k;
    ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope1.lb = lb;
    ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope0.p = p;
    ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope0.accum = accum;
    ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope0.rt = rt;
    ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf = setup_deg_ddf;
    __hc_frame__ -> pc = 2;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _setup_deg_off_async7_, and execute async: %p in _setup_deg_off_async9_\n", __hc_frame__, __hc_frame__->pc, __hc__setup_deg_off_async9__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__setup_deg_off_async9__frame__ -> header)) 
      _setup_deg_off_async9_(ws,((hc_frameHeader *)__hc__setup_deg_off_async9__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async2:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _setup_deg_off_async7_\n", printEntry(__hc_frame__, buffer));
      listOfXendoffSet = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope1.listOfXendoffSet;
      i = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope1.i;
      c = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope1.c;
      k = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope1.k;
      lb = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope1.lb;
      p = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope0.p;
      accum = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope0.accum;
      rt = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope0.rt;
      setup_deg_ddf = ((struct __hc__setup_deg_off_async7__frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf;
    }
  }
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _setup_deg_off_async6_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t p;
  int64_t *accum;
  int *rt;
  DDF_t *setup_deg_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_func2;
  }
  p = ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope0.p;
  accum = ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope0.accum;
  rt = ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope0.rt;
  setup_deg_ddf = ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf;
  DDF_t *prefix_sum_ddf = __ddfCreate();
  ws -> costatus =  HC_FUNC_COMPLETE;
  __hc_frame__ -> pc = 2;
  ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope1.prefix_sum_ddf = prefix_sum_ddf;
  ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope0.p = p;
  ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope0.accum = accum;
  ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope0.rt = rt;
  ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf = setup_deg_ddf;
  prefix_sum(accum,p,prefix_sum_ddf);
  if (ws -> costatus !=  HC_FUNC_COMPLETE) 
    return ;
  if (0) {
    __post_func2:
    hc_free_unwind_frame(ws);
    __hc_frame__ = ws -> frame;
    prefix_sum_ddf = ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope1.prefix_sum_ddf;
    p = ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope0.p;
    accum = ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope0.accum;
    rt = ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope0.rt;
    setup_deg_ddf = ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf;
  }
/* Write back value for INOUT/OUT arguments */
  struct __hc__setup_deg_off_async7__frame_t__ *__hc__setup_deg_off_async7__frame__ = (struct __hc__setup_deg_off_async7__frame_t__ *)(hc_init_frame(sizeof(struct __hc__setup_deg_off_async7__frame_t__ ),_setup_deg_off_async7_));
  p_await_list_t __hc__setup_deg_off_async7_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__setup_deg_off_async7_awaitList_size = 0 + 1;
  __hc__setup_deg_off_async7_awaitList -> array = hc_malloc((1 + __hc__setup_deg_off_async7_awaitList_size) * sizeof(p_DDF_t ));
  __hc__setup_deg_off_async7_awaitList -> array[__hc__setup_deg_off_async7_awaitList_size] = NULL;
  unsigned int __hc__setup_deg_off_async7_awaitList_curr_index = 0;
  __hc__setup_deg_off_async7_awaitList -> array[__hc__setup_deg_off_async7_awaitList_curr_index++] = prefix_sum_ddf;
  __hc__setup_deg_off_async7_awaitList -> waitingFrontier = &__hc__setup_deg_off_async7_awaitList -> array[0];
  __hc__setup_deg_off_async7__frame__ -> header.awaitList = __hc__setup_deg_off_async7_awaitList;
  __hc__setup_deg_off_async7__frame__ -> scope0.p = ((int64_t )p);
  __hc__setup_deg_off_async7__frame__ -> scope0.accum = ((int64_t *)accum);
  __hc__setup_deg_off_async7__frame__ -> scope0.rt = ((int *)rt);
  __hc__setup_deg_off_async7__frame__ -> scope0.setup_deg_ddf = ((DDF_t *)setup_deg_ddf);
  ((hc_frameHeader *)__hc__setup_deg_off_async7__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__setup_deg_off_async7__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__setup_deg_off_async7__frame__ -> header)) 
      deque_push_default(__hc__setup_deg_off_async7__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope1.prefix_sum_ddf = prefix_sum_ddf;
    ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope0.p = p;
    ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope0.accum = accum;
    ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope0.rt = rt;
    ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf = setup_deg_ddf;
    __hc_frame__ -> pc = 1;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _setup_deg_off_async6_, and execute async: %p in _setup_deg_off_async7_\n", __hc_frame__, __hc_frame__->pc, __hc__setup_deg_off_async7__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__setup_deg_off_async7__frame__ -> header)) 
      _setup_deg_off_async7_(ws,((hc_frameHeader *)__hc__setup_deg_off_async7__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _setup_deg_off_async6_\n", printEntry(__hc_frame__, buffer));
      prefix_sum_ddf = ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope1.prefix_sum_ddf;
      p = ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope0.p;
      accum = ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope0.accum;
      rt = ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope0.rt;
      setup_deg_ddf = ((struct __hc__setup_deg_off_async6__frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf;
    }
  }
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _setup_deg_off_async5_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t lb;
  int64_t ub;
  DDF_t *d;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  lb = ((struct __hc__setup_deg_off_async5__frame_t__ *)__hc_frame__) -> scope0.lb;
  ub = ((struct __hc__setup_deg_off_async5__frame_t__ *)__hc_frame__) -> scope0.ub;
  d = ((struct __hc__setup_deg_off_async5__frame_t__ *)__hc_frame__) -> scope0.d;
  int64_t k;
  for (k = lb; k < ub; k++) {
    if (xoff[2 * k] < 2) 
      xoff[2 * k] = 2;
  }
  __ddfPut(d,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _setup_deg_off_async4_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t p;
  int64_t *accum;
  int *rt;
  DDF_t *setup_deg_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_async2;
  }
  p = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope0.p;
  accum = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope0.accum;
  rt = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope0.rt;
  setup_deg_ddf = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf;
  p_DDF_list_t listOfXoffUpdate = __ddfListCreate();
  int64_t i = 0;
  int64_t c = (nv / p);
  for (i = 0; i < (p - 1); i++) {
    int64_t lb = (i * c);
    int64_t ub = (lb + c);
    DDF_t *d = __ddfCreate();
    __enlist(d,listOfXoffUpdate);
    struct __hc__setup_deg_off_async5__frame_t__ *__hc__setup_deg_off_async5__frame__ = (struct __hc__setup_deg_off_async5__frame_t__ *)(hc_init_frame(sizeof(struct __hc__setup_deg_off_async5__frame_t__ ),_setup_deg_off_async5_));
    __hc__setup_deg_off_async5__frame__ -> scope0.lb = ((int64_t )lb);
    __hc__setup_deg_off_async5__frame__ -> scope0.ub = ((int64_t )ub);
    __hc__setup_deg_off_async5__frame__ -> scope0.d = ((DDF_t *)d);
    ((hc_frameHeader *)__hc__setup_deg_off_async5__frame__) -> sched_policy = hc_sched_policy(ws);
    increment_counter();
    if (((hc_frameHeader *)__hc__setup_deg_off_async5__frame__) -> sched_policy ==  HELP_FIRST) {
      deque_push_default(__hc__setup_deg_off_async5__frame__);
    }
    else {
      ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope2.lb = lb;
      ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope2.ub = ub;
      ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope2.d = d;
      ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope1.listOfXoffUpdate = listOfXoffUpdate;
      ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope1.i = i;
      ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope1.c = c;
      ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope0.p = p;
      ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope0.accum = accum;
      ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope0.rt = rt;
      ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf = setup_deg_ddf;
      __hc_frame__ -> pc = 1;
      deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _setup_deg_off_async4_, and execute async: %p in _setup_deg_off_async5_\n", __hc_frame__, __hc_frame__->pc, __hc__setup_deg_off_async5__frame__);
      _setup_deg_off_async5_(ws,((hc_frameHeader *)__hc__setup_deg_off_async5__frame__),0);
      if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
        return ;
      }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
      if (0) {
        __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _setup_deg_off_async4_\n", printEntry(__hc_frame__, buffer));
        lb = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope2.lb;
        ub = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope2.ub;
        d = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope2.d;
        listOfXoffUpdate = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope1.listOfXoffUpdate;
        i = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope1.i;
        c = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope1.c;
        p = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope0.p;
        accum = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope0.accum;
        rt = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope0.rt;
        setup_deg_ddf = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf;
      }
    }
  }
  int64_t k;
  int64_t lb = (i * c);
  for (k = lb; k < nv; k++) {
    if (xoff[2 * k] < 2) 
      xoff[2 * k] = 2;
  }
/* Write back value for INOUT/OUT arguments */
  struct __hc__setup_deg_off_async6__frame_t__ *__hc__setup_deg_off_async6__frame__ = (struct __hc__setup_deg_off_async6__frame_t__ *)(hc_init_frame(sizeof(struct __hc__setup_deg_off_async6__frame_t__ ),_setup_deg_off_async6_));
  p_await_list_t __hc__setup_deg_off_async6_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__setup_deg_off_async6_awaitList_size = 0 + listOfXoffUpdate -> size + 0;
  __hc__setup_deg_off_async6_awaitList -> array = hc_malloc((1 + __hc__setup_deg_off_async6_awaitList_size) * sizeof(p_DDF_t ));
  __hc__setup_deg_off_async6_awaitList -> array[__hc__setup_deg_off_async6_awaitList_size] = NULL;
  unsigned int __hc__setup_deg_off_async6_awaitList_curr_index = 0;
  __copyInto(__hc__setup_deg_off_async6_awaitList,__hc__setup_deg_off_async6_awaitList_curr_index,listOfXoffUpdate);
  __hc__setup_deg_off_async6_awaitList_curr_index += listOfXoffUpdate -> size;
  __hc__setup_deg_off_async6_awaitList -> waitingFrontier = &__hc__setup_deg_off_async6_awaitList -> array[0];
  __hc__setup_deg_off_async6__frame__ -> header.awaitList = __hc__setup_deg_off_async6_awaitList;
  __hc__setup_deg_off_async6__frame__ -> scope0.p = ((int64_t )p);
  __hc__setup_deg_off_async6__frame__ -> scope0.accum = ((int64_t *)accum);
  __hc__setup_deg_off_async6__frame__ -> scope0.rt = ((int *)rt);
  __hc__setup_deg_off_async6__frame__ -> scope0.setup_deg_ddf = ((DDF_t *)setup_deg_ddf);
  ((hc_frameHeader *)__hc__setup_deg_off_async6__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__setup_deg_off_async6__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__setup_deg_off_async6__frame__ -> header)) 
      deque_push_default(__hc__setup_deg_off_async6__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope1.listOfXoffUpdate = listOfXoffUpdate;
    ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope1.i = i;
    ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope1.c = c;
    ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope1.k = k;
    ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope1.lb = lb;
    ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope0.p = p;
    ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope0.accum = accum;
    ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope0.rt = rt;
    ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf = setup_deg_ddf;
    __hc_frame__ -> pc = 2;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _setup_deg_off_async4_, and execute async: %p in _setup_deg_off_async6_\n", __hc_frame__, __hc_frame__->pc, __hc__setup_deg_off_async6__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__setup_deg_off_async6__frame__ -> header)) 
      _setup_deg_off_async6_(ws,((hc_frameHeader *)__hc__setup_deg_off_async6__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async2:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _setup_deg_off_async4_\n", printEntry(__hc_frame__, buffer));
      listOfXoffUpdate = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope1.listOfXoffUpdate;
      i = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope1.i;
      c = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope1.c;
      k = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope1.k;
      lb = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope1.lb;
      p = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope0.p;
      accum = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope0.accum;
      rt = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope0.rt;
      setup_deg_ddf = ((struct __hc__setup_deg_off_async4__frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf;
    }
  }
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _setup_deg_off_async3_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t lb;
  int64_t ub;
  struct packed_edge *__restrict__ IJ;
  DDF_t *d;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  lb = ((struct __hc__setup_deg_off_async3__frame_t__ *)__hc_frame__) -> scope0.lb;
  ub = ((struct __hc__setup_deg_off_async3__frame_t__ *)__hc_frame__) -> scope0.ub;
  IJ = ((struct __hc__setup_deg_off_async3__frame_t__ *)__hc_frame__) -> scope0.IJ;
  d = ((struct __hc__setup_deg_off_async3__frame_t__ *)__hc_frame__) -> scope0.d;
  int64_t k;
  for (k = lb; k < ub; k++) {
    int64_t v0 = get_v0_from_edge((IJ + k));
    int64_t v1 = get_v1_from_edge((IJ + k));
/* Skip self-edges. */
    if (v0 != v1) {
      if (v0 >= 0) 
        hc_atomic_inc(((volatile int *)(xoff + (2 * v0))));
      if (v1 >= 0) 
        hc_atomic_inc(((volatile int *)(xoff + (2 * v1))));
    }
  }
  __ddfPut(d,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _setup_deg_off_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  struct packed_edge *__restrict__ IJ;
  int64_t nedge;
  int64_t p;
  int64_t *accum;
  int *rt;
  DDF_t *setup_deg_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_async2;
  }
  IJ = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.IJ;
  nedge = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.nedge;
  p = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.p;
  accum = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.accum;
  rt = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.rt;
  setup_deg_ddf = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf;
  p_DDF_list_t listOfXoffSet = __ddfListCreate();
  int64_t i;
  int64_t c = (nedge / p);
  for (i = 0; i < (p - 1); i++) {
    int64_t lb = (i * c);
    int64_t ub = (lb + c);
    DDF_t *d = __ddfCreate();
    __enlist(d,listOfXoffSet);
    struct __hc__setup_deg_off_async3__frame_t__ *__hc__setup_deg_off_async3__frame__ = (struct __hc__setup_deg_off_async3__frame_t__ *)(hc_init_frame(sizeof(struct __hc__setup_deg_off_async3__frame_t__ ),_setup_deg_off_async3_));
    __hc__setup_deg_off_async3__frame__ -> scope0.lb = ((int64_t )lb);
    __hc__setup_deg_off_async3__frame__ -> scope0.ub = ((int64_t )ub);
    __hc__setup_deg_off_async3__frame__ -> scope0.IJ = ((struct packed_edge *__restrict__ )IJ);
    __hc__setup_deg_off_async3__frame__ -> scope0.d = ((DDF_t *)d);
    ((hc_frameHeader *)__hc__setup_deg_off_async3__frame__) -> sched_policy = hc_sched_policy(ws);
    increment_counter();
    if (((hc_frameHeader *)__hc__setup_deg_off_async3__frame__) -> sched_policy ==  HELP_FIRST) {
      deque_push_default(__hc__setup_deg_off_async3__frame__);
    }
    else {
      ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope2.lb = lb;
      ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope2.ub = ub;
      ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope2.d = d;
      ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope1.listOfXoffSet = listOfXoffSet;
      ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope1.i = i;
      ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope1.c = c;
      ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.IJ = IJ;
      ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.nedge = nedge;
      ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.p = p;
      ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.accum = accum;
      ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.rt = rt;
      ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf = setup_deg_ddf;
      __hc_frame__ -> pc = 1;
      deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _setup_deg_off_async2_, and execute async: %p in _setup_deg_off_async3_\n", __hc_frame__, __hc_frame__->pc, __hc__setup_deg_off_async3__frame__);
      _setup_deg_off_async3_(ws,((hc_frameHeader *)__hc__setup_deg_off_async3__frame__),0);
      if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
        return ;
      }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
      if (0) {
        __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _setup_deg_off_async2_\n", printEntry(__hc_frame__, buffer));
        lb = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope2.lb;
        ub = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope2.ub;
        d = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope2.d;
        listOfXoffSet = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope1.listOfXoffSet;
        i = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope1.i;
        c = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope1.c;
        IJ = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.IJ;
        nedge = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.nedge;
        p = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.p;
        accum = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.accum;
        rt = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.rt;
        setup_deg_ddf = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf;
      }
    }
  }
  int64_t k;
  int64_t lb = (i * c);
  for (k = lb; k < nedge; k++) {
    int64_t v0 = get_v0_from_edge((IJ + k));
    int64_t v1 = get_v1_from_edge((IJ + k));
/* Skip self-edges. */
    if (v0 != v1) {
      if (v0 >= 0) 
        hc_atomic_inc(((volatile int *)(xoff + (2 * v0))));
      if (v1 >= 0) 
        hc_atomic_inc(((volatile int *)(xoff + (2 * v1))));
    }
  }
/* Write back value for INOUT/OUT arguments */
  struct __hc__setup_deg_off_async4__frame_t__ *__hc__setup_deg_off_async4__frame__ = (struct __hc__setup_deg_off_async4__frame_t__ *)(hc_init_frame(sizeof(struct __hc__setup_deg_off_async4__frame_t__ ),_setup_deg_off_async4_));
  p_await_list_t __hc__setup_deg_off_async4_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__setup_deg_off_async4_awaitList_size = 0 + listOfXoffSet -> size + 0;
  __hc__setup_deg_off_async4_awaitList -> array = hc_malloc((1 + __hc__setup_deg_off_async4_awaitList_size) * sizeof(p_DDF_t ));
  __hc__setup_deg_off_async4_awaitList -> array[__hc__setup_deg_off_async4_awaitList_size] = NULL;
  unsigned int __hc__setup_deg_off_async4_awaitList_curr_index = 0;
  __copyInto(__hc__setup_deg_off_async4_awaitList,__hc__setup_deg_off_async4_awaitList_curr_index,listOfXoffSet);
  __hc__setup_deg_off_async4_awaitList_curr_index += listOfXoffSet -> size;
  __hc__setup_deg_off_async4_awaitList -> waitingFrontier = &__hc__setup_deg_off_async4_awaitList -> array[0];
  __hc__setup_deg_off_async4__frame__ -> header.awaitList = __hc__setup_deg_off_async4_awaitList;
  __hc__setup_deg_off_async4__frame__ -> scope0.p = ((int64_t )p);
  __hc__setup_deg_off_async4__frame__ -> scope0.accum = ((int64_t *)accum);
  __hc__setup_deg_off_async4__frame__ -> scope0.rt = ((int *)rt);
  __hc__setup_deg_off_async4__frame__ -> scope0.setup_deg_ddf = ((DDF_t *)setup_deg_ddf);
  ((hc_frameHeader *)__hc__setup_deg_off_async4__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__setup_deg_off_async4__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__setup_deg_off_async4__frame__ -> header)) 
      deque_push_default(__hc__setup_deg_off_async4__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope1.listOfXoffSet = listOfXoffSet;
    ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope1.i = i;
    ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope1.c = c;
    ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope1.k = k;
    ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope1.lb = lb;
    ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.IJ = IJ;
    ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.nedge = nedge;
    ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.p = p;
    ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.accum = accum;
    ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.rt = rt;
    ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf = setup_deg_ddf;
    __hc_frame__ -> pc = 2;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _setup_deg_off_async2_, and execute async: %p in _setup_deg_off_async4_\n", __hc_frame__, __hc_frame__->pc, __hc__setup_deg_off_async4__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__setup_deg_off_async4__frame__ -> header)) 
      _setup_deg_off_async4_(ws,((hc_frameHeader *)__hc__setup_deg_off_async4__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async2:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _setup_deg_off_async2_\n", printEntry(__hc_frame__, buffer));
      listOfXoffSet = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope1.listOfXoffSet;
      i = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope1.i;
      c = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope1.c;
      k = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope1.k;
      lb = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope1.lb;
      IJ = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.IJ;
      nedge = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.nedge;
      p = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.p;
      accum = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.accum;
      rt = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.rt;
      setup_deg_ddf = ((struct __hc__setup_deg_off_async2__frame_t__ *)__hc_frame__) -> scope0.setup_deg_ddf;
    }
  }
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _setup_deg_off_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t lb;
  int64_t ub;
  DDF_t *d;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  lb = ((struct __hc__setup_deg_off_async1__frame_t__ *)__hc_frame__) -> scope0.lb;
  ub = ((struct __hc__setup_deg_off_async1__frame_t__ *)__hc_frame__) -> scope0.ub;
  d = ((struct __hc__setup_deg_off_async1__frame_t__ *)__hc_frame__) -> scope0.d;
  int64_t k;
  for (k = lb; k < ub; k++) {
    xoff[k] = 0;
  }
  __ddfPut(d,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

static void scatter_edge(int64_t i,int64_t j)
{
  int64_t where;
  where = (hc_fetch_and_add(((int *)(xoff + (1 + (2 * i)))),1));
  xadj[where] = j;
}

static int i64cmp(const void *a,const void *b)
{
  int64_t ia =  *((int64_t *)a);
  int64_t ib =  *((int64_t *)b);
  if (ia < ib) 
    return -1;
  if (ia > ib) 
    return 1;
  return 0;
}

static void pack_vtx_edges(int64_t i)
{
  int64_t kcur;
  int64_t k;
  if ((xoff[2 * i] + 1) >= xoff[1 + (2 * i)]) 
    return ;
  qsort((xadj + xoff[2 * i]),(xoff[1 + (2 * i)] - xoff[2 * i]),(sizeof(( *xadj))),i64cmp);
  kcur = xoff[2 * i];
  for (k = (xoff[2 * i] + 1); k < xoff[1 + (2 * i)]; ++k) 
    if (xadj[k] != xadj[kcur]) 
      xadj[++kcur] = xadj[k];
  ++kcur;
  for (k = kcur; k < xoff[1 + (2 * i)]; ++k) 
    xadj[k] = (-1);
  xoff[1 + (2 * i)] = kcur;
}
typedef struct __hc_pack_edges_frame_t__ {
hc_frameHeader header;
struct __hc_pack_edges_scope0_frame_t__ {
int64_t p;
DDF_t *create_graph_ddf;}scope0;
struct __hc_pack_edges_scope1_frame_t__ {
p_DDF_list_t listOfPackEdges;
int64_t i;
int64_t c;
int64_t k;
int64_t lb;}scope1;
struct __hc_pack_edges_scope2_frame_t__ {
int64_t lb;
int64_t ub;
DDF_t *d;}scope2;}__hc_pack_edges_frame_t__;
typedef struct __hc__pack_edges_async1__frame_t__ {
hc_frameHeader header;
struct __hc__pack_edges_async1__scope0_frame_t__ {
int64_t lb;
int64_t ub;
DDF_t *d;}scope0;}__hc__pack_edges_async1__frame_t__;
typedef struct __hc__pack_edges_async2__frame_t__ {
hc_frameHeader header;
struct __hc__pack_edges_async2__scope0_frame_t__ {
DDF_t *create_graph_ddf;}scope0;}__hc__pack_edges_async2__frame_t__;
void __hc_pack_edges__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _pack_edges_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _pack_edges_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);

static void pack_edges(int64_t p,DDF_t *create_graph_ddf)
{
  hc_workerState *ws = current_ws();
  ws -> costatus =  HC_FUNC_COMPLETE;
  struct __hc_pack_edges_frame_t__ *__hc_frame__ = (struct __hc_pack_edges_frame_t__ *)(hc_init_frame_ws(ws,sizeof(struct __hc_pack_edges_frame_t__ ),__hc_pack_edges__));
  __hc_frame__ -> scope0.p = p;
  __hc_frame__ -> scope0.create_graph_ddf = create_graph_ddf;
			LOG_INFO(ws, "execute frame: %p, parent: %p of function %s\n", __hc_frame__,((hc_frameHeader*) __hc_frame__)->parent, "__hc_pack_edges__" );
  __hc_pack_edges__(ws,((hc_frameHeader *)__hc_frame__),0);
  if (ws -> costatus ==  HC_FUNC_COMPLETE) 
    hc_free_unwind_frame(ws);
  return ;
}

void __hc_pack_edges__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t p;
  DDF_t *create_graph_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_async2;
  }
  p = ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope0.p;
  create_graph_ddf = ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf;
  p_DDF_list_t listOfPackEdges = __ddfListCreate();
  int64_t i;
  int64_t c = (nv / p);
  for (i = 0; i < (p - 1); i++) {
    int64_t lb = (i * c);
    int64_t ub = (lb + c);
    DDF_t *d = __ddfCreate();
    __enlist(d,listOfPackEdges);
    struct __hc__pack_edges_async1__frame_t__ *__hc__pack_edges_async1__frame__ = (struct __hc__pack_edges_async1__frame_t__ *)(hc_init_frame(sizeof(struct __hc__pack_edges_async1__frame_t__ ),_pack_edges_async1_));
    __hc__pack_edges_async1__frame__ -> scope0.lb = ((int64_t )lb);
    __hc__pack_edges_async1__frame__ -> scope0.ub = ((int64_t )ub);
    __hc__pack_edges_async1__frame__ -> scope0.d = ((DDF_t *)d);
    ((hc_frameHeader *)__hc__pack_edges_async1__frame__) -> sched_policy = hc_sched_policy(ws);
    increment_counter();
    if (((hc_frameHeader *)__hc__pack_edges_async1__frame__) -> sched_policy ==  HELP_FIRST) {
      deque_push_default(__hc__pack_edges_async1__frame__);
    }
    else {
      ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope2.lb = lb;
      ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope2.ub = ub;
      ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope2.d = d;
      ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope1.listOfPackEdges = listOfPackEdges;
      ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope1.i = i;
      ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope1.c = c;
      ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope0.p = p;
      ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf = create_graph_ddf;
      __hc_frame__ -> pc = 1;
      deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_pack_edges__, and execute async: %p in _pack_edges_async1_\n", __hc_frame__, __hc_frame__->pc, __hc__pack_edges_async1__frame__);
      _pack_edges_async1_(ws,((hc_frameHeader *)__hc__pack_edges_async1__frame__),0);
      if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
        return ;
      }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
      if (0) {
        __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_pack_edges__\n", printEntry(__hc_frame__, buffer));
        lb = ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope2.lb;
        ub = ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope2.ub;
        d = ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope2.d;
        listOfPackEdges = ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope1.listOfPackEdges;
        i = ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope1.i;
        c = ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope1.c;
        p = ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope0.p;
        create_graph_ddf = ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf;
      }
    }
  }
  int64_t k;
  int64_t lb = (i * c);
  for (k = lb; k < nv; k++) {
    pack_vtx_edges(k);
  }
  struct __hc__pack_edges_async2__frame_t__ *__hc__pack_edges_async2__frame__ = (struct __hc__pack_edges_async2__frame_t__ *)(hc_init_frame(sizeof(struct __hc__pack_edges_async2__frame_t__ ),_pack_edges_async2_));
  p_await_list_t __hc__pack_edges_async2_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__pack_edges_async2_awaitList_size = 0 + listOfPackEdges -> size + 0;
  __hc__pack_edges_async2_awaitList -> array = hc_malloc((1 + __hc__pack_edges_async2_awaitList_size) * sizeof(p_DDF_t ));
  __hc__pack_edges_async2_awaitList -> array[__hc__pack_edges_async2_awaitList_size] = NULL;
  unsigned int __hc__pack_edges_async2_awaitList_curr_index = 0;
  __copyInto(__hc__pack_edges_async2_awaitList,__hc__pack_edges_async2_awaitList_curr_index,listOfPackEdges);
  __hc__pack_edges_async2_awaitList_curr_index += listOfPackEdges -> size;
  __hc__pack_edges_async2_awaitList -> waitingFrontier = &__hc__pack_edges_async2_awaitList -> array[0];
  __hc__pack_edges_async2__frame__ -> header.awaitList = __hc__pack_edges_async2_awaitList;
  __hc__pack_edges_async2__frame__ -> scope0.create_graph_ddf = ((DDF_t *)create_graph_ddf);
  ((hc_frameHeader *)__hc__pack_edges_async2__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__pack_edges_async2__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__pack_edges_async2__frame__ -> header)) 
      deque_push_default(__hc__pack_edges_async2__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope1.listOfPackEdges = listOfPackEdges;
    ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope1.i = i;
    ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope1.c = c;
    ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope1.k = k;
    ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope1.lb = lb;
    ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope0.p = p;
    ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf = create_graph_ddf;
    __hc_frame__ -> pc = 2;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_pack_edges__, and execute async: %p in _pack_edges_async2_\n", __hc_frame__, __hc_frame__->pc, __hc__pack_edges_async2__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__pack_edges_async2__frame__ -> header)) 
      _pack_edges_async2_(ws,((hc_frameHeader *)__hc__pack_edges_async2__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async2:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_pack_edges__\n", printEntry(__hc_frame__, buffer));
      listOfPackEdges = ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope1.listOfPackEdges;
      i = ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope1.i;
      c = ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope1.c;
      k = ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope1.k;
      lb = ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope1.lb;
      p = ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope0.p;
      create_graph_ddf = ((struct __hc_pack_edges_frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf;
    }
  }
  ws -> costatus =  HC_FUNC_COMPLETE;
  return ;
}

void _pack_edges_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  DDF_t *create_graph_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  create_graph_ddf = ((struct __hc__pack_edges_async2__frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf;
  __ddfPut(create_graph_ddf,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _pack_edges_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t lb;
  int64_t ub;
  DDF_t *d;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  lb = ((struct __hc__pack_edges_async1__frame_t__ *)__hc_frame__) -> scope0.lb;
  ub = ((struct __hc__pack_edges_async1__frame_t__ *)__hc_frame__) -> scope0.ub;
  d = ((struct __hc__pack_edges_async1__frame_t__ *)__hc_frame__) -> scope0.d;
  int64_t k;
  for (k = lb; k < ub; k++) {
    pack_vtx_edges(k);
  }
  __ddfPut(d,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}
typedef struct __hc_gather_edges_frame_t__ {
hc_frameHeader header;
struct __hc_gather_edges_scope0_frame_t__ {
struct packed_edge *__restrict__ IJ;
int64_t nedge;
int64_t p;
DDF_t *create_graph_ddf;}scope0;
struct __hc_gather_edges_scope1_frame_t__ {
p_DDF_list_t listOfGatherEdges;
int64_t i;
int64_t c;
int64_t k;
int64_t lb;}scope1;
struct __hc_gather_edges_scope2_frame_t__ {
int64_t lb;
int64_t ub;
DDF_t *d;}scope2;
struct __hc_gather_edges_scope3_frame_t__ {
int64_t i;
int64_t j;}scope3;}__hc_gather_edges_frame_t__;
typedef struct __hc__gather_edges_async1__frame_t__ {
hc_frameHeader header;
struct __hc__gather_edges_async1__scope0_frame_t__ {
int64_t lb;
int64_t ub;
struct packed_edge *__restrict__ IJ;
DDF_t *d;}scope0;}__hc__gather_edges_async1__frame_t__;
typedef struct __hc__gather_edges_async2__frame_t__ {
hc_frameHeader header;
struct __hc__gather_edges_async2__scope0_frame_t__ {
int64_t p;
DDF_t *create_graph_ddf;}scope0;}__hc__gather_edges_async2__frame_t__;
void __hc_gather_edges__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _gather_edges_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _gather_edges_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);

static void gather_edges(struct packed_edge *__restrict__ IJ,int64_t nedge,int64_t p,DDF_t *create_graph_ddf)
{
  hc_workerState *ws = current_ws();
  ws -> costatus =  HC_FUNC_COMPLETE;
  struct __hc_gather_edges_frame_t__ *__hc_frame__ = (struct __hc_gather_edges_frame_t__ *)(hc_init_frame_ws(ws,sizeof(struct __hc_gather_edges_frame_t__ ),__hc_gather_edges__));
  __hc_frame__ -> scope0.IJ = IJ;
  __hc_frame__ -> scope0.nedge = nedge;
  __hc_frame__ -> scope0.p = p;
  __hc_frame__ -> scope0.create_graph_ddf = create_graph_ddf;
			LOG_INFO(ws, "execute frame: %p, parent: %p of function %s\n", __hc_frame__,((hc_frameHeader*) __hc_frame__)->parent, "__hc_gather_edges__" );
  __hc_gather_edges__(ws,((hc_frameHeader *)__hc_frame__),0);
  if (ws -> costatus ==  HC_FUNC_COMPLETE) 
    hc_free_unwind_frame(ws);
  return ;
}

void __hc_gather_edges__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  struct packed_edge *__restrict__ IJ;
  int64_t nedge;
  int64_t p;
  DDF_t *create_graph_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_async2;
  }
  IJ = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope0.IJ;
  nedge = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope0.nedge;
  p = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope0.p;
  create_graph_ddf = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf;
  p_DDF_list_t listOfGatherEdges = __ddfListCreate();
  int64_t i;
  int64_t c = (nedge / p);
  for (i = 0; i < (p - 1); i++) {
    int64_t lb = (i * c);
    int64_t ub = (lb + c);
    DDF_t *d = __ddfCreate();
    __enlist(d,listOfGatherEdges);
    struct __hc__gather_edges_async1__frame_t__ *__hc__gather_edges_async1__frame__ = (struct __hc__gather_edges_async1__frame_t__ *)(hc_init_frame(sizeof(struct __hc__gather_edges_async1__frame_t__ ),_gather_edges_async1_));
    __hc__gather_edges_async1__frame__ -> scope0.lb = ((int64_t )lb);
    __hc__gather_edges_async1__frame__ -> scope0.ub = ((int64_t )ub);
    __hc__gather_edges_async1__frame__ -> scope0.IJ = ((struct packed_edge *__restrict__ )IJ);
    __hc__gather_edges_async1__frame__ -> scope0.d = ((DDF_t *)d);
    ((hc_frameHeader *)__hc__gather_edges_async1__frame__) -> sched_policy = hc_sched_policy(ws);
    increment_counter();
    if (((hc_frameHeader *)__hc__gather_edges_async1__frame__) -> sched_policy ==  HELP_FIRST) {
      deque_push_default(__hc__gather_edges_async1__frame__);
    }
    else {
      ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope2.lb = lb;
      ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope2.ub = ub;
      ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope2.d = d;
      ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope1.listOfGatherEdges = listOfGatherEdges;
      ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope1.i = i;
      ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope1.c = c;
      ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope0.IJ = IJ;
      ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope0.nedge = nedge;
      ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope0.p = p;
      ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf = create_graph_ddf;
      __hc_frame__ -> pc = 1;
      deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_gather_edges__, and execute async: %p in _gather_edges_async1_\n", __hc_frame__, __hc_frame__->pc, __hc__gather_edges_async1__frame__);
      _gather_edges_async1_(ws,((hc_frameHeader *)__hc__gather_edges_async1__frame__),0);
      if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
        return ;
      }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
      if (0) {
        __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_gather_edges__\n", printEntry(__hc_frame__, buffer));
        lb = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope2.lb;
        ub = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope2.ub;
        d = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope2.d;
        listOfGatherEdges = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope1.listOfGatherEdges;
        i = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope1.i;
        c = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope1.c;
        IJ = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope0.IJ;
        nedge = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope0.nedge;
        p = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope0.p;
        create_graph_ddf = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf;
      }
    }
  }
  int64_t k;
  int64_t lb = (i * c);
  for (k = lb; k < nedge; k++) {
    int64_t i = get_v0_from_edge((IJ + k));
    int64_t j = get_v1_from_edge((IJ + k));
    if (((i >= 0) && (j >= 0)) && (i != j)) {
      scatter_edge(i,j);
      scatter_edge(j,i);
    }
  }
  struct __hc__gather_edges_async2__frame_t__ *__hc__gather_edges_async2__frame__ = (struct __hc__gather_edges_async2__frame_t__ *)(hc_init_frame(sizeof(struct __hc__gather_edges_async2__frame_t__ ),_gather_edges_async2_));
  p_await_list_t __hc__gather_edges_async2_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__gather_edges_async2_awaitList_size = 0 + listOfGatherEdges -> size + 0;
  __hc__gather_edges_async2_awaitList -> array = hc_malloc((1 + __hc__gather_edges_async2_awaitList_size) * sizeof(p_DDF_t ));
  __hc__gather_edges_async2_awaitList -> array[__hc__gather_edges_async2_awaitList_size] = NULL;
  unsigned int __hc__gather_edges_async2_awaitList_curr_index = 0;
  __copyInto(__hc__gather_edges_async2_awaitList,__hc__gather_edges_async2_awaitList_curr_index,listOfGatherEdges);
  __hc__gather_edges_async2_awaitList_curr_index += listOfGatherEdges -> size;
  __hc__gather_edges_async2_awaitList -> waitingFrontier = &__hc__gather_edges_async2_awaitList -> array[0];
  __hc__gather_edges_async2__frame__ -> header.awaitList = __hc__gather_edges_async2_awaitList;
  __hc__gather_edges_async2__frame__ -> scope0.p = ((int64_t )p);
  __hc__gather_edges_async2__frame__ -> scope0.create_graph_ddf = ((DDF_t *)create_graph_ddf);
  ((hc_frameHeader *)__hc__gather_edges_async2__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__gather_edges_async2__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__gather_edges_async2__frame__ -> header)) 
      deque_push_default(__hc__gather_edges_async2__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope1.listOfGatherEdges = listOfGatherEdges;
    ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope1.i = i;
    ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope1.c = c;
    ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope1.k = k;
    ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope1.lb = lb;
    ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope0.IJ = IJ;
    ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope0.nedge = nedge;
    ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope0.p = p;
    ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf = create_graph_ddf;
    __hc_frame__ -> pc = 2;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_gather_edges__, and execute async: %p in _gather_edges_async2_\n", __hc_frame__, __hc_frame__->pc, __hc__gather_edges_async2__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__gather_edges_async2__frame__ -> header)) 
      _gather_edges_async2_(ws,((hc_frameHeader *)__hc__gather_edges_async2__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async2:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_gather_edges__\n", printEntry(__hc_frame__, buffer));
      listOfGatherEdges = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope1.listOfGatherEdges;
      i = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope1.i;
      c = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope1.c;
      k = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope1.k;
      lb = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope1.lb;
      IJ = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope0.IJ;
      nedge = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope0.nedge;
      p = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope0.p;
      create_graph_ddf = ((struct __hc_gather_edges_frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf;
    }
  }
  ws -> costatus =  HC_FUNC_COMPLETE;
  return ;
}

void _gather_edges_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t p;
  DDF_t *create_graph_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_func1;
  }
  p = ((struct __hc__gather_edges_async2__frame_t__ *)__hc_frame__) -> scope0.p;
  create_graph_ddf = ((struct __hc__gather_edges_async2__frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf;
  ws -> costatus =  HC_FUNC_COMPLETE;
  __hc_frame__ -> pc = 1;
  ((struct __hc__gather_edges_async2__frame_t__ *)__hc_frame__) -> scope0.p = p;
  ((struct __hc__gather_edges_async2__frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf = create_graph_ddf;
  pack_edges(p,create_graph_ddf);
  if (ws -> costatus !=  HC_FUNC_COMPLETE) 
    return ;
  if (0) {
    __post_func1:
    hc_free_unwind_frame(ws);
    __hc_frame__ = ws -> frame;
    p = ((struct __hc__gather_edges_async2__frame_t__ *)__hc_frame__) -> scope0.p;
    create_graph_ddf = ((struct __hc__gather_edges_async2__frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf;
  }
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _gather_edges_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t lb;
  int64_t ub;
  struct packed_edge *__restrict__ IJ;
  DDF_t *d;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  lb = ((struct __hc__gather_edges_async1__frame_t__ *)__hc_frame__) -> scope0.lb;
  ub = ((struct __hc__gather_edges_async1__frame_t__ *)__hc_frame__) -> scope0.ub;
  IJ = ((struct __hc__gather_edges_async1__frame_t__ *)__hc_frame__) -> scope0.IJ;
  d = ((struct __hc__gather_edges_async1__frame_t__ *)__hc_frame__) -> scope0.d;
  int64_t k;
  for (k = lb; k < ub; k++) {
    int64_t i = get_v0_from_edge((IJ + k));
    int64_t j = get_v1_from_edge((IJ + k));
    if (((i >= 0) && (j >= 0)) && (i != j)) {
      scatter_edge(i,j);
      scatter_edge(j,i);
    }
  }
  __ddfPut(d,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}
typedef struct __hc_create_graph_from_edgelist_frame_t__ {
hc_frameHeader header;
struct __hc_create_graph_from_edgelist_scope0_frame_t__ {
struct packed_edge *IJ;
int64_t nedge;
DDF_t *create_graph_ddf;}scope0;
struct __hc_create_graph_from_edgelist_scope1_frame_t__ {
int64_t p;
DDF_t *find_nv_ddf;}scope1;}__hc_create_graph_from_edgelist_frame_t__;
typedef struct __hc__create_graph_from_edgelist_async1__frame_t__ {
hc_frameHeader header;
struct __hc__create_graph_from_edgelist_async1__scope0_frame_t__ {
struct packed_edge *IJ;
int64_t nedge;
int64_t p;
DDF_t *create_graph_ddf;}scope0;
struct __hc__create_graph_from_edgelist_async1__scope1_frame_t__ {
DDF_t *setup_deg_ddf;
int *rt;}scope1;}__hc__create_graph_from_edgelist_async1__frame_t__;
typedef struct __hc__create_graph_from_edgelist_async2__frame_t__ {
hc_frameHeader header;
struct __hc__create_graph_from_edgelist_async2__scope0_frame_t__ {
struct packed_edge *IJ;
int64_t nedge;
int *rt;
int64_t p;
DDF_t *create_graph_ddf;}scope0;}__hc__create_graph_from_edgelist_async2__frame_t__;
void __hc_create_graph_from_edgelist__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _create_graph_from_edgelist_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _create_graph_from_edgelist_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);

static void create_graph_from_edgelist(struct packed_edge *IJ,int64_t nedge,DDF_t *create_graph_ddf)
{
  hc_workerState *ws = current_ws();
  ws -> costatus =  HC_FUNC_COMPLETE;
  struct __hc_create_graph_from_edgelist_frame_t__ *__hc_frame__ = (struct __hc_create_graph_from_edgelist_frame_t__ *)(hc_init_frame_ws(ws,sizeof(struct __hc_create_graph_from_edgelist_frame_t__ ),__hc_create_graph_from_edgelist__));
  __hc_frame__ -> scope0.IJ = IJ;
  __hc_frame__ -> scope0.nedge = nedge;
  __hc_frame__ -> scope0.create_graph_ddf = create_graph_ddf;
			LOG_INFO(ws, "execute frame: %p, parent: %p of function %s\n", __hc_frame__,((hc_frameHeader*) __hc_frame__)->parent, "__hc_create_graph_from_edgelist__" );
  __hc_create_graph_from_edgelist__(ws,((hc_frameHeader *)__hc_frame__),0);
  if (ws -> costatus ==  HC_FUNC_COMPLETE) 
    hc_free_unwind_frame(ws);
  return ;
}

void __hc_create_graph_from_edgelist__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  struct packed_edge *IJ;
  int64_t nedge;
  DDF_t *create_graph_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_func2;
  }
  IJ = ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope0.IJ;
  nedge = ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope0.nedge;
  create_graph_ddf = ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf;
  int64_t p = ( *( *((hc_workerState *)(pthread_getspecific(wskey)))).context).nproc;
  printf("Running HC on %d threads\n",((int )p));
  DDF_t *find_nv_ddf = __ddfCreate();
  ws -> costatus =  HC_FUNC_COMPLETE;
  __hc_frame__ -> pc = 2;
  ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope1.p = p;
  ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope1.find_nv_ddf = find_nv_ddf;
  ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope0.IJ = IJ;
  ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope0.nedge = nedge;
  ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf = create_graph_ddf;
  find_nv(IJ,nedge,p,find_nv_ddf);
  if (ws -> costatus !=  HC_FUNC_COMPLETE) 
    return ;
  if (0) {
    __post_func2:
    hc_free_unwind_frame(ws);
    __hc_frame__ = ws -> frame;
    p = ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope1.p;
    find_nv_ddf = ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope1.find_nv_ddf;
    IJ = ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope0.IJ;
    nedge = ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope0.nedge;
    create_graph_ddf = ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf;
  }
  struct __hc__create_graph_from_edgelist_async1__frame_t__ *__hc__create_graph_from_edgelist_async1__frame__ = (struct __hc__create_graph_from_edgelist_async1__frame_t__ *)(hc_init_frame(sizeof(struct __hc__create_graph_from_edgelist_async1__frame_t__ ),_create_graph_from_edgelist_async1_));
  p_await_list_t __hc__create_graph_from_edgelist_async1_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__create_graph_from_edgelist_async1_awaitList_size = 0 + 1;
  __hc__create_graph_from_edgelist_async1_awaitList -> array = hc_malloc((1 + __hc__create_graph_from_edgelist_async1_awaitList_size) * sizeof(p_DDF_t ));
  __hc__create_graph_from_edgelist_async1_awaitList -> array[__hc__create_graph_from_edgelist_async1_awaitList_size] = NULL;
  unsigned int __hc__create_graph_from_edgelist_async1_awaitList_curr_index = 0;
  __hc__create_graph_from_edgelist_async1_awaitList -> array[__hc__create_graph_from_edgelist_async1_awaitList_curr_index++] = find_nv_ddf;
  __hc__create_graph_from_edgelist_async1_awaitList -> waitingFrontier = &__hc__create_graph_from_edgelist_async1_awaitList -> array[0];
  __hc__create_graph_from_edgelist_async1__frame__ -> header.awaitList = __hc__create_graph_from_edgelist_async1_awaitList;
  __hc__create_graph_from_edgelist_async1__frame__ -> scope0.IJ = ((struct packed_edge *)IJ);
  __hc__create_graph_from_edgelist_async1__frame__ -> scope0.nedge = ((int64_t )nedge);
  __hc__create_graph_from_edgelist_async1__frame__ -> scope0.p = ((int64_t )p);
  __hc__create_graph_from_edgelist_async1__frame__ -> scope0.create_graph_ddf = ((DDF_t *)create_graph_ddf);
  ((hc_frameHeader *)__hc__create_graph_from_edgelist_async1__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__create_graph_from_edgelist_async1__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__create_graph_from_edgelist_async1__frame__ -> header)) 
      deque_push_default(__hc__create_graph_from_edgelist_async1__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope1.p = p;
    ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope1.find_nv_ddf = find_nv_ddf;
    ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope0.IJ = IJ;
    ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope0.nedge = nedge;
    ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf = create_graph_ddf;
    __hc_frame__ -> pc = 1;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_create_graph_from_edgelist__, and execute async: %p in _create_graph_from_edgelist_async1_\n", __hc_frame__, __hc_frame__->pc, __hc__create_graph_from_edgelist_async1__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__create_graph_from_edgelist_async1__frame__ -> header)) 
      _create_graph_from_edgelist_async1_(ws,((hc_frameHeader *)__hc__create_graph_from_edgelist_async1__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_create_graph_from_edgelist__\n", printEntry(__hc_frame__, buffer));
      p = ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope1.p;
      find_nv_ddf = ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope1.find_nv_ddf;
      IJ = ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope0.IJ;
      nedge = ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope0.nedge;
      create_graph_ddf = ((struct __hc_create_graph_from_edgelist_frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf;
    }
  }
  ws -> costatus =  HC_FUNC_COMPLETE;
  return ;
}

void _create_graph_from_edgelist_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  struct packed_edge *IJ;
  int64_t nedge;
  int *rt;
  int64_t p;
  DDF_t *create_graph_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_func1;
  }
  IJ = ((struct __hc__create_graph_from_edgelist_async2__frame_t__ *)__hc_frame__) -> scope0.IJ;
  nedge = ((struct __hc__create_graph_from_edgelist_async2__frame_t__ *)__hc_frame__) -> scope0.nedge;
  rt = ((struct __hc__create_graph_from_edgelist_async2__frame_t__ *)__hc_frame__) -> scope0.rt;
  p = ((struct __hc__create_graph_from_edgelist_async2__frame_t__ *)__hc_frame__) -> scope0.p;
  create_graph_ddf = ((struct __hc__create_graph_from_edgelist_async2__frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf;
  if ( *rt != 0) {
    xfree_large(xoff);
    fprintf(__stderrp,"Failure creating graph.\n");
    exit(1);
  }
  ws -> costatus =  HC_FUNC_COMPLETE;
  __hc_frame__ -> pc = 1;
  ((struct __hc__create_graph_from_edgelist_async2__frame_t__ *)__hc_frame__) -> scope0.IJ = IJ;
  ((struct __hc__create_graph_from_edgelist_async2__frame_t__ *)__hc_frame__) -> scope0.nedge = nedge;
  ((struct __hc__create_graph_from_edgelist_async2__frame_t__ *)__hc_frame__) -> scope0.rt = rt;
  ((struct __hc__create_graph_from_edgelist_async2__frame_t__ *)__hc_frame__) -> scope0.p = p;
  ((struct __hc__create_graph_from_edgelist_async2__frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf = create_graph_ddf;
  gather_edges(IJ,nedge,p,create_graph_ddf);
  if (ws -> costatus !=  HC_FUNC_COMPLETE) 
    return ;
  if (0) {
    __post_func1:
    hc_free_unwind_frame(ws);
    __hc_frame__ = ws -> frame;
    IJ = ((struct __hc__create_graph_from_edgelist_async2__frame_t__ *)__hc_frame__) -> scope0.IJ;
    nedge = ((struct __hc__create_graph_from_edgelist_async2__frame_t__ *)__hc_frame__) -> scope0.nedge;
    rt = ((struct __hc__create_graph_from_edgelist_async2__frame_t__ *)__hc_frame__) -> scope0.rt;
    p = ((struct __hc__create_graph_from_edgelist_async2__frame_t__ *)__hc_frame__) -> scope0.p;
    create_graph_ddf = ((struct __hc__create_graph_from_edgelist_async2__frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf;
  }
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _create_graph_from_edgelist_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  struct packed_edge *IJ;
  int64_t nedge;
  int64_t p;
  DDF_t *create_graph_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_func2;
  }
  IJ = ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope0.IJ;
  nedge = ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope0.nedge;
  p = ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope0.p;
  create_graph_ddf = ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf;
  if (alloc_graph(nedge) != 0) {
    fprintf(__stderrp,"Failure creating graph.\n");
    exit(1);
  }
  DDF_t *setup_deg_ddf = __ddfCreate();
  int *rt = (int *)(hc_mm_malloc(((hc_workerState *)(pthread_getspecific(wskey))),(sizeof(int ))));
  ws -> costatus =  HC_FUNC_COMPLETE;
  __hc_frame__ -> pc = 2;
  ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope1.setup_deg_ddf = setup_deg_ddf;
  ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope1.rt = rt;
  ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope0.IJ = IJ;
  ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope0.nedge = nedge;
  ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope0.p = p;
  ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf = create_graph_ddf;
  setup_deg_off(IJ,nedge,p,rt,setup_deg_ddf);
  if (ws -> costatus !=  HC_FUNC_COMPLETE) 
    return ;
  if (0) {
    __post_func2:
    hc_free_unwind_frame(ws);
    __hc_frame__ = ws -> frame;
    setup_deg_ddf = ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope1.setup_deg_ddf;
    rt = ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope1.rt;
    IJ = ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope0.IJ;
    nedge = ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope0.nedge;
    p = ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope0.p;
    create_graph_ddf = ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf;
  }
/* Write back value for INOUT/OUT arguments */
  struct __hc__create_graph_from_edgelist_async2__frame_t__ *__hc__create_graph_from_edgelist_async2__frame__ = (struct __hc__create_graph_from_edgelist_async2__frame_t__ *)(hc_init_frame(sizeof(struct __hc__create_graph_from_edgelist_async2__frame_t__ ),_create_graph_from_edgelist_async2_));
  p_await_list_t __hc__create_graph_from_edgelist_async2_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__create_graph_from_edgelist_async2_awaitList_size = 0 + 1;
  __hc__create_graph_from_edgelist_async2_awaitList -> array = hc_malloc((1 + __hc__create_graph_from_edgelist_async2_awaitList_size) * sizeof(p_DDF_t ));
  __hc__create_graph_from_edgelist_async2_awaitList -> array[__hc__create_graph_from_edgelist_async2_awaitList_size] = NULL;
  unsigned int __hc__create_graph_from_edgelist_async2_awaitList_curr_index = 0;
  __hc__create_graph_from_edgelist_async2_awaitList -> array[__hc__create_graph_from_edgelist_async2_awaitList_curr_index++] = setup_deg_ddf;
  __hc__create_graph_from_edgelist_async2_awaitList -> waitingFrontier = &__hc__create_graph_from_edgelist_async2_awaitList -> array[0];
  __hc__create_graph_from_edgelist_async2__frame__ -> header.awaitList = __hc__create_graph_from_edgelist_async2_awaitList;
  __hc__create_graph_from_edgelist_async2__frame__ -> scope0.IJ = ((struct packed_edge *)IJ);
  __hc__create_graph_from_edgelist_async2__frame__ -> scope0.nedge = ((int64_t )nedge);
  __hc__create_graph_from_edgelist_async2__frame__ -> scope0.rt = ((int *)rt);
  __hc__create_graph_from_edgelist_async2__frame__ -> scope0.p = ((int64_t )p);
  __hc__create_graph_from_edgelist_async2__frame__ -> scope0.create_graph_ddf = ((DDF_t *)create_graph_ddf);
  ((hc_frameHeader *)__hc__create_graph_from_edgelist_async2__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__create_graph_from_edgelist_async2__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__create_graph_from_edgelist_async2__frame__ -> header)) 
      deque_push_default(__hc__create_graph_from_edgelist_async2__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope1.setup_deg_ddf = setup_deg_ddf;
    ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope1.rt = rt;
    ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope0.IJ = IJ;
    ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope0.nedge = nedge;
    ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope0.p = p;
    ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf = create_graph_ddf;
    __hc_frame__ -> pc = 1;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _create_graph_from_edgelist_async1_, and execute async: %p in _create_graph_from_edgelist_async2_\n", __hc_frame__, __hc_frame__->pc, __hc__create_graph_from_edgelist_async2__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__create_graph_from_edgelist_async2__frame__ -> header)) 
      _create_graph_from_edgelist_async2_(ws,((hc_frameHeader *)__hc__create_graph_from_edgelist_async2__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _create_graph_from_edgelist_async1_\n", printEntry(__hc_frame__, buffer));
      setup_deg_ddf = ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope1.setup_deg_ddf;
      rt = ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope1.rt;
      IJ = ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope0.IJ;
      nedge = ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope0.nedge;
      p = ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope0.p;
      create_graph_ddf = ((struct __hc__create_graph_from_edgelist_async1__frame_t__ *)__hc_frame__) -> scope0.create_graph_ddf;
    }
  }
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}
typedef struct bfs_arg {
volatile int64_t k1;
volatile int64_t k2;}bfs_arg;

static void make_bfs_kernel(int64_t lb,int64_t ub,bfs_arg *arg,int64_t *__restrict__ vlist,int64_t *__restrict__ bfs_tree,DDF_t *level_ddf)
{
  int64_t kbuf = 0;
  int64_t nbuf[256UL];
  int64_t k;
  for (k = lb; k < ub; k++) {
    int64_t v = vlist[k];
    int64_t veo = xoff[1 + (2 * v)];
    int64_t vo;
    for (vo = xoff[2 * v]; vo < veo; ++vo) {
      int64_t j = xadj[vo];
      if (bfs_tree[j] == (-1)) {
        if (hc_cas(((volatile int *)(bfs_tree + j)),-1,v) != 0) {
          if (kbuf < 256) {
            nbuf[kbuf++] = j;
          }
          else {
            int64_t voff = (hc_fetch_and_add(((int *)(&arg -> k2)),256));
            int64_t vk;
            (__builtin_expect((!((voff + 256) <= nv)),0) != 0L)?__assert_rtn(__func__,"/Users/dm14/Documents/Habanero/hc-trunk/examples/graph500-2.1.4/graph500-ddf.hc",927,"voff + CHUNK_SIZE <= nv") : ((void )0);
            for (vk = 0; vk < 256; ++vk) 
              vlist[voff + vk] = nbuf[vk];
            nbuf[0] = j;
            kbuf = 1;
          }
        }
      }
    }
  }
  if (kbuf != 0LL) {
    int64_t voff = (hc_fetch_and_add(((int *)(&arg -> k2)),kbuf));
    int64_t vk;
    (__builtin_expect((!((voff + kbuf) <= nv)),0) != 0L)?__assert_rtn(__func__,"/Users/dm14/Documents/Habanero/hc-trunk/examples/graph500-2.1.4/graph500-ddf.hc",940,"voff + kbuf <= nv") : ((void )0);
    for (vk = 0; vk < kbuf; ++vk) 
      vlist[voff + vk] = nbuf[vk];
  }
  __ddfPut(level_ddf,dummyDataVoidPtr);
}
typedef struct __hc_bfs_level_sync_frame_t__ {
hc_frameHeader header;
struct __hc_bfs_level_sync_scope0_frame_t__ {
bfs_arg *arg;
int64_t *__restrict__ vlist;
int64_t *__restrict__ bfs_tree;
int64_t oldk2;
DDF_t *bfs_fin;}scope0;
struct __hc_bfs_level_sync_scope1_frame_t__ {
int64_t range;
int64_t nc;
int64_t nr;
p_DDF_list_t listOfCurrentLevelDDFs;
int64_t c;}scope1;
struct __hc_bfs_level_sync_scope2_frame_t__ {
DDF_t *new_ddf;}scope2;
struct __hc_bfs_level_sync_scope3_frame_t__ {
DDF_t *new_ddf;
int64_t lb;
int64_t ub;}scope3;}__hc_bfs_level_sync_frame_t__;
typedef struct __hc__bfs_level_sync_async1__frame_t__ {
hc_frameHeader header;
struct __hc__bfs_level_sync_async1__scope0_frame_t__ {
int64_t c;
bfs_arg *arg;
int64_t *__restrict__ vlist;
int64_t *__restrict__ bfs_tree;
DDF_t *new_ddf;}scope0;}__hc__bfs_level_sync_async1__frame_t__;
typedef struct __hc__bfs_level_sync_async2__frame_t__ {
hc_frameHeader header;
struct __hc__bfs_level_sync_async2__scope0_frame_t__ {
int64_t lb;
int64_t ub;
bfs_arg *arg;
int64_t *__restrict__ vlist;
int64_t *__restrict__ bfs_tree;
DDF_t *new_ddf;}scope0;}__hc__bfs_level_sync_async2__frame_t__;
typedef struct __hc__bfs_level_sync_async3__frame_t__ {
hc_frameHeader header;
struct __hc__bfs_level_sync_async3__scope0_frame_t__ {
bfs_arg *arg;
int64_t *__restrict__ vlist;
int64_t *__restrict__ bfs_tree;
int64_t oldk2;
DDF_t *bfs_fin;}scope0;}__hc__bfs_level_sync_async3__frame_t__;
void __hc_bfs_level_sync__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _bfs_level_sync_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _bfs_level_sync_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _bfs_level_sync_async3_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);

static void bfs_level_sync(bfs_arg *arg,int64_t *__restrict__ vlist,int64_t *__restrict__ bfs_tree,int64_t oldk2,DDF_t *bfs_fin)
{
  hc_workerState *ws = current_ws();
  ws -> costatus =  HC_FUNC_COMPLETE;
  struct __hc_bfs_level_sync_frame_t__ *__hc_frame__ = (struct __hc_bfs_level_sync_frame_t__ *)(hc_init_frame_ws(ws,sizeof(struct __hc_bfs_level_sync_frame_t__ ),__hc_bfs_level_sync__));
  __hc_frame__ -> scope0.arg = arg;
  __hc_frame__ -> scope0.vlist = vlist;
  __hc_frame__ -> scope0.bfs_tree = bfs_tree;
  __hc_frame__ -> scope0.oldk2 = oldk2;
  __hc_frame__ -> scope0.bfs_fin = bfs_fin;
			LOG_INFO(ws, "execute frame: %p, parent: %p of function %s\n", __hc_frame__,((hc_frameHeader*) __hc_frame__)->parent, "__hc_bfs_level_sync__" );
  __hc_bfs_level_sync__(ws,((hc_frameHeader *)__hc_frame__),0);
  if (ws -> costatus ==  HC_FUNC_COMPLETE) 
    hc_free_unwind_frame(ws);
  return ;
}

void __hc_bfs_level_sync__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  bfs_arg *arg;
  int64_t *__restrict__ vlist;
  int64_t *__restrict__ bfs_tree;
  int64_t oldk2;
  DDF_t *bfs_fin;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_async2;
    case 3:
    goto __post_async3;
  }
  arg = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.arg;
  vlist = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.vlist;
  bfs_tree = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
  oldk2 = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.oldk2;
  bfs_fin = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.bfs_fin;
  arg -> k1 = oldk2;
  if ((arg -> k1) == (arg -> k2)) {
    __ddfPut(bfs_fin,dummyDataVoidPtr);
    ws -> costatus =  HC_FUNC_COMPLETE;
    return ;
  }
  oldk2 = (arg -> k2);
  int64_t range = (oldk2 - (arg -> k1));
  int64_t nc = (range / 256);
  int64_t nr = (range % 256);
  p_DDF_list_t listOfCurrentLevelDDFs = __ddfListCreate();
  int64_t c;
  for (c = 0; c < nc; c++) {
    DDF_t *new_ddf = __ddfCreate();
    __enlist(new_ddf,listOfCurrentLevelDDFs);
    struct __hc__bfs_level_sync_async1__frame_t__ *__hc__bfs_level_sync_async1__frame__ = (struct __hc__bfs_level_sync_async1__frame_t__ *)(hc_init_frame(sizeof(struct __hc__bfs_level_sync_async1__frame_t__ ),_bfs_level_sync_async1_));
    __hc__bfs_level_sync_async1__frame__ -> scope0.c = ((int64_t )c);
    __hc__bfs_level_sync_async1__frame__ -> scope0.arg = ((bfs_arg *)arg);
    __hc__bfs_level_sync_async1__frame__ -> scope0.vlist = ((int64_t *__restrict__ )vlist);
    __hc__bfs_level_sync_async1__frame__ -> scope0.bfs_tree = ((int64_t *__restrict__ )bfs_tree);
    __hc__bfs_level_sync_async1__frame__ -> scope0.new_ddf = ((DDF_t *)new_ddf);
    ((hc_frameHeader *)__hc__bfs_level_sync_async1__frame__) -> sched_policy = hc_sched_policy(ws);
    increment_counter();
    if (((hc_frameHeader *)__hc__bfs_level_sync_async1__frame__) -> sched_policy ==  HELP_FIRST) {
      deque_push_default(__hc__bfs_level_sync_async1__frame__);
    }
    else {
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope2.new_ddf = new_ddf;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.range = range;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.nc = nc;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.nr = nr;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.listOfCurrentLevelDDFs = listOfCurrentLevelDDFs;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.c = c;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.arg = arg;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.vlist = vlist;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.bfs_tree = bfs_tree;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.oldk2 = oldk2;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.bfs_fin = bfs_fin;
      __hc_frame__ -> pc = 1;
      deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_bfs_level_sync__, and execute async: %p in _bfs_level_sync_async1_\n", __hc_frame__, __hc_frame__->pc, __hc__bfs_level_sync_async1__frame__);
      _bfs_level_sync_async1_(ws,((hc_frameHeader *)__hc__bfs_level_sync_async1__frame__),0);
      if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
        return ;
      }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
      if (0) {
        __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_bfs_level_sync__\n", printEntry(__hc_frame__, buffer));
        new_ddf = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope2.new_ddf;
        range = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.range;
        nc = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.nc;
        nr = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.nr;
        listOfCurrentLevelDDFs = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.listOfCurrentLevelDDFs;
        c = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.c;
        arg = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.arg;
        vlist = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.vlist;
        bfs_tree = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
        oldk2 = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.oldk2;
        bfs_fin = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.bfs_fin;
      }
    }
  }
  if (nr != 0) {
    DDF_t *new_ddf = __ddfCreate();
    __enlist(new_ddf,listOfCurrentLevelDDFs);
    int64_t lb = ((arg -> k1) + (nc * 256));
    int64_t ub = (lb + nr);
    struct __hc__bfs_level_sync_async2__frame_t__ *__hc__bfs_level_sync_async2__frame__ = (struct __hc__bfs_level_sync_async2__frame_t__ *)(hc_init_frame(sizeof(struct __hc__bfs_level_sync_async2__frame_t__ ),_bfs_level_sync_async2_));
    __hc__bfs_level_sync_async2__frame__ -> scope0.lb = ((int64_t )lb);
    __hc__bfs_level_sync_async2__frame__ -> scope0.ub = ((int64_t )ub);
    __hc__bfs_level_sync_async2__frame__ -> scope0.arg = ((bfs_arg *)arg);
    __hc__bfs_level_sync_async2__frame__ -> scope0.vlist = ((int64_t *__restrict__ )vlist);
    __hc__bfs_level_sync_async2__frame__ -> scope0.bfs_tree = ((int64_t *__restrict__ )bfs_tree);
    __hc__bfs_level_sync_async2__frame__ -> scope0.new_ddf = ((DDF_t *)new_ddf);
    ((hc_frameHeader *)__hc__bfs_level_sync_async2__frame__) -> sched_policy = hc_sched_policy(ws);
    increment_counter();
    if (((hc_frameHeader *)__hc__bfs_level_sync_async2__frame__) -> sched_policy ==  HELP_FIRST) {
      deque_push_default(__hc__bfs_level_sync_async2__frame__);
    }
    else {
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope3.new_ddf = new_ddf;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope3.lb = lb;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope3.ub = ub;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.range = range;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.nc = nc;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.nr = nr;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.listOfCurrentLevelDDFs = listOfCurrentLevelDDFs;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.c = c;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.arg = arg;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.vlist = vlist;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.bfs_tree = bfs_tree;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.oldk2 = oldk2;
      ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.bfs_fin = bfs_fin;
      __hc_frame__ -> pc = 2;
      deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_bfs_level_sync__, and execute async: %p in _bfs_level_sync_async2_\n", __hc_frame__, __hc_frame__->pc, __hc__bfs_level_sync_async2__frame__);
      _bfs_level_sync_async2_(ws,((hc_frameHeader *)__hc__bfs_level_sync_async2__frame__),0);
      if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
        return ;
      }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
      if (0) {
        __post_async2:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_bfs_level_sync__\n", printEntry(__hc_frame__, buffer));
        new_ddf = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope3.new_ddf;
        lb = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope3.lb;
        ub = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope3.ub;
        range = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.range;
        nc = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.nc;
        nr = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.nr;
        listOfCurrentLevelDDFs = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.listOfCurrentLevelDDFs;
        c = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.c;
        arg = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.arg;
        vlist = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.vlist;
        bfs_tree = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
        oldk2 = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.oldk2;
        bfs_fin = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.bfs_fin;
      }
    }
  }
  struct __hc__bfs_level_sync_async3__frame_t__ *__hc__bfs_level_sync_async3__frame__ = (struct __hc__bfs_level_sync_async3__frame_t__ *)(hc_init_frame(sizeof(struct __hc__bfs_level_sync_async3__frame_t__ ),_bfs_level_sync_async3_));
  p_await_list_t __hc__bfs_level_sync_async3_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__bfs_level_sync_async3_awaitList_size = 0 + listOfCurrentLevelDDFs -> size + 0;
  __hc__bfs_level_sync_async3_awaitList -> array = hc_malloc((1 + __hc__bfs_level_sync_async3_awaitList_size) * sizeof(p_DDF_t ));
  __hc__bfs_level_sync_async3_awaitList -> array[__hc__bfs_level_sync_async3_awaitList_size] = NULL;
  unsigned int __hc__bfs_level_sync_async3_awaitList_curr_index = 0;
  __copyInto(__hc__bfs_level_sync_async3_awaitList,__hc__bfs_level_sync_async3_awaitList_curr_index,listOfCurrentLevelDDFs);
  __hc__bfs_level_sync_async3_awaitList_curr_index += listOfCurrentLevelDDFs -> size;
  __hc__bfs_level_sync_async3_awaitList -> waitingFrontier = &__hc__bfs_level_sync_async3_awaitList -> array[0];
  __hc__bfs_level_sync_async3__frame__ -> header.awaitList = __hc__bfs_level_sync_async3_awaitList;
  __hc__bfs_level_sync_async3__frame__ -> scope0.arg = ((bfs_arg *)arg);
  __hc__bfs_level_sync_async3__frame__ -> scope0.vlist = ((int64_t *__restrict__ )vlist);
  __hc__bfs_level_sync_async3__frame__ -> scope0.bfs_tree = ((int64_t *__restrict__ )bfs_tree);
  __hc__bfs_level_sync_async3__frame__ -> scope0.oldk2 = ((int64_t )oldk2);
  __hc__bfs_level_sync_async3__frame__ -> scope0.bfs_fin = ((DDF_t *)bfs_fin);
  ((hc_frameHeader *)__hc__bfs_level_sync_async3__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__bfs_level_sync_async3__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__bfs_level_sync_async3__frame__ -> header)) 
      deque_push_default(__hc__bfs_level_sync_async3__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.range = range;
    ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.nc = nc;
    ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.nr = nr;
    ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.listOfCurrentLevelDDFs = listOfCurrentLevelDDFs;
    ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.c = c;
    ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.arg = arg;
    ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.vlist = vlist;
    ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.bfs_tree = bfs_tree;
    ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.oldk2 = oldk2;
    ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.bfs_fin = bfs_fin;
    __hc_frame__ -> pc = 3;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_bfs_level_sync__, and execute async: %p in _bfs_level_sync_async3_\n", __hc_frame__, __hc_frame__->pc, __hc__bfs_level_sync_async3__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__bfs_level_sync_async3__frame__ -> header)) 
      _bfs_level_sync_async3_(ws,((hc_frameHeader *)__hc__bfs_level_sync_async3__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async3:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_bfs_level_sync__\n", printEntry(__hc_frame__, buffer));
      range = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.range;
      nc = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.nc;
      nr = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.nr;
      listOfCurrentLevelDDFs = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.listOfCurrentLevelDDFs;
      c = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope1.c;
      arg = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.arg;
      vlist = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.vlist;
      bfs_tree = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
      oldk2 = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.oldk2;
      bfs_fin = ((struct __hc_bfs_level_sync_frame_t__ *)__hc_frame__) -> scope0.bfs_fin;
    }
  }
  ws -> costatus =  HC_FUNC_COMPLETE;
  return ;
}

void _bfs_level_sync_async3_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  bfs_arg *arg;
  int64_t *__restrict__ vlist;
  int64_t *__restrict__ bfs_tree;
  int64_t oldk2;
  DDF_t *bfs_fin;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_func1;
  }
  arg = ((struct __hc__bfs_level_sync_async3__frame_t__ *)__hc_frame__) -> scope0.arg;
  vlist = ((struct __hc__bfs_level_sync_async3__frame_t__ *)__hc_frame__) -> scope0.vlist;
  bfs_tree = ((struct __hc__bfs_level_sync_async3__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
  oldk2 = ((struct __hc__bfs_level_sync_async3__frame_t__ *)__hc_frame__) -> scope0.oldk2;
  bfs_fin = ((struct __hc__bfs_level_sync_async3__frame_t__ *)__hc_frame__) -> scope0.bfs_fin;
  ws -> costatus =  HC_FUNC_COMPLETE;
  __hc_frame__ -> pc = 1;
  ((struct __hc__bfs_level_sync_async3__frame_t__ *)__hc_frame__) -> scope0.arg = arg;
  ((struct __hc__bfs_level_sync_async3__frame_t__ *)__hc_frame__) -> scope0.vlist = vlist;
  ((struct __hc__bfs_level_sync_async3__frame_t__ *)__hc_frame__) -> scope0.bfs_tree = bfs_tree;
  ((struct __hc__bfs_level_sync_async3__frame_t__ *)__hc_frame__) -> scope0.oldk2 = oldk2;
  ((struct __hc__bfs_level_sync_async3__frame_t__ *)__hc_frame__) -> scope0.bfs_fin = bfs_fin;
  bfs_level_sync(arg,vlist,bfs_tree,oldk2,bfs_fin);
  if (ws -> costatus !=  HC_FUNC_COMPLETE) 
    return ;
  if (0) {
    __post_func1:
    hc_free_unwind_frame(ws);
    __hc_frame__ = ws -> frame;
    arg = ((struct __hc__bfs_level_sync_async3__frame_t__ *)__hc_frame__) -> scope0.arg;
    vlist = ((struct __hc__bfs_level_sync_async3__frame_t__ *)__hc_frame__) -> scope0.vlist;
    bfs_tree = ((struct __hc__bfs_level_sync_async3__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
    oldk2 = ((struct __hc__bfs_level_sync_async3__frame_t__ *)__hc_frame__) -> scope0.oldk2;
    bfs_fin = ((struct __hc__bfs_level_sync_async3__frame_t__ *)__hc_frame__) -> scope0.bfs_fin;
  }
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _bfs_level_sync_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t lb;
  int64_t ub;
  bfs_arg *arg;
  int64_t *__restrict__ vlist;
  int64_t *__restrict__ bfs_tree;
  DDF_t *new_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  lb = ((struct __hc__bfs_level_sync_async2__frame_t__ *)__hc_frame__) -> scope0.lb;
  ub = ((struct __hc__bfs_level_sync_async2__frame_t__ *)__hc_frame__) -> scope0.ub;
  arg = ((struct __hc__bfs_level_sync_async2__frame_t__ *)__hc_frame__) -> scope0.arg;
  vlist = ((struct __hc__bfs_level_sync_async2__frame_t__ *)__hc_frame__) -> scope0.vlist;
  bfs_tree = ((struct __hc__bfs_level_sync_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
  new_ddf = ((struct __hc__bfs_level_sync_async2__frame_t__ *)__hc_frame__) -> scope0.new_ddf;
  make_bfs_kernel(lb,ub,arg,vlist,bfs_tree,new_ddf);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _bfs_level_sync_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t c;
  bfs_arg *arg;
  int64_t *__restrict__ vlist;
  int64_t *__restrict__ bfs_tree;
  DDF_t *new_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  c = ((struct __hc__bfs_level_sync_async1__frame_t__ *)__hc_frame__) -> scope0.c;
  arg = ((struct __hc__bfs_level_sync_async1__frame_t__ *)__hc_frame__) -> scope0.arg;
  vlist = ((struct __hc__bfs_level_sync_async1__frame_t__ *)__hc_frame__) -> scope0.vlist;
  bfs_tree = ((struct __hc__bfs_level_sync_async1__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
  new_ddf = ((struct __hc__bfs_level_sync_async1__frame_t__ *)__hc_frame__) -> scope0.new_ddf;
  int64_t lb = ((arg -> k1) + (c * 256));
  int64_t ub = (lb + 256);
  make_bfs_kernel(lb,ub,arg,vlist,bfs_tree,new_ddf);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}
typedef struct __hc_make_bfs_tree_frame_t__ {
hc_frameHeader header;
struct __hc_make_bfs_tree_scope0_frame_t__ {
int64_t *bfs_tree_out;
int64_t srcvtx;
DDF_t *bfs_tree_ddf;}scope0;
struct __hc_make_bfs_tree_scope1_frame_t__ {
p_DDF_list_t listOfBfsInit;
int p;
int64_t *__restrict__ bfs_tree;
int64_t *__restrict__ vlist;
bfs_arg *arg;
int64_t i;
int64_t c;
int64_t k;
int64_t lb;}scope1;
struct __hc_make_bfs_tree_scope2_frame_t__ {
int64_t lb;
int64_t ub;
DDF_t *d;}scope2;}__hc_make_bfs_tree_frame_t__;
typedef struct __hc__make_bfs_tree_async1__frame_t__ {
hc_frameHeader header;
struct __hc__make_bfs_tree_async1__scope0_frame_t__ {
int64_t lb;
int64_t ub;
int64_t *__restrict__ bfs_tree;
DDF_t *d;}scope0;}__hc__make_bfs_tree_async1__frame_t__;
typedef struct __hc__make_bfs_tree_async2__frame_t__ {
hc_frameHeader header;
struct __hc__make_bfs_tree_async2__scope0_frame_t__ {
int64_t *__restrict__ bfs_tree;
int64_t srcvtx;
bfs_arg *arg;
int64_t *__restrict__ vlist;
DDF_t *bfs_tree_ddf;}scope0;
struct __hc__make_bfs_tree_async2__scope1_frame_t__ {
DDF_t *bfs_fin;}scope1;}__hc__make_bfs_tree_async2__frame_t__;
typedef struct __hc__make_bfs_tree_async3__frame_t__ {
hc_frameHeader header;
struct __hc__make_bfs_tree_async3__scope0_frame_t__ {
bfs_arg *arg;
int64_t *__restrict__ vlist;
DDF_t *bfs_tree_ddf;}scope0;}__hc__make_bfs_tree_async3__frame_t__;
void __hc_make_bfs_tree__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _make_bfs_tree_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _make_bfs_tree_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _make_bfs_tree_async3_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);

static void make_bfs_tree(int64_t *bfs_tree_out,int64_t srcvtx,DDF_t *bfs_tree_ddf)
{
  hc_workerState *ws = current_ws();
  ws -> costatus =  HC_FUNC_COMPLETE;
  struct __hc_make_bfs_tree_frame_t__ *__hc_frame__ = (struct __hc_make_bfs_tree_frame_t__ *)(hc_init_frame_ws(ws,sizeof(struct __hc_make_bfs_tree_frame_t__ ),__hc_make_bfs_tree__));
  __hc_frame__ -> scope0.bfs_tree_out = bfs_tree_out;
  __hc_frame__ -> scope0.srcvtx = srcvtx;
  __hc_frame__ -> scope0.bfs_tree_ddf = bfs_tree_ddf;
			LOG_INFO(ws, "execute frame: %p, parent: %p of function %s\n", __hc_frame__,((hc_frameHeader*) __hc_frame__)->parent, "__hc_make_bfs_tree__" );
  __hc_make_bfs_tree__(ws,((hc_frameHeader *)__hc_frame__),0);
  if (ws -> costatus ==  HC_FUNC_COMPLETE) 
    hc_free_unwind_frame(ws);
  return ;
}

void __hc_make_bfs_tree__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t *bfs_tree_out;
  int64_t srcvtx;
  DDF_t *bfs_tree_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_async2;
  }
  bfs_tree_out = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.bfs_tree_out;
  srcvtx = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.srcvtx;
  bfs_tree_ddf = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.bfs_tree_ddf;
  p_DDF_list_t listOfBfsInit = __ddfListCreate();
  int p = ( *( *((hc_workerState *)(pthread_getspecific(wskey)))).context).nproc;
  int64_t *__restrict__ bfs_tree = bfs_tree_out;
  int64_t *__restrict__ vlist = (int64_t *)((void *)0);
  vlist = (xmalloc_large((nv * sizeof(( *vlist)))));
  if (!(vlist != 0)) {
    perror("make_bfs_tree failed");
    abort();
  }
  vlist[0] = srcvtx;
  bfs_arg *arg = (bfs_arg *)(hc_mm_malloc(((hc_workerState *)(pthread_getspecific(wskey))),(sizeof(struct bfs_arg ))));
  arg -> k1 = 0;
  arg -> k2 = 1;
  int64_t i;
  int64_t c = (nv / p);
  for (i = 0; i < (p - 1); i++) {
    int64_t lb = (i * c);
    int64_t ub = (lb + c);
    DDF_t *d = __ddfCreate();
    __enlist(d,listOfBfsInit);
    struct __hc__make_bfs_tree_async1__frame_t__ *__hc__make_bfs_tree_async1__frame__ = (struct __hc__make_bfs_tree_async1__frame_t__ *)(hc_init_frame(sizeof(struct __hc__make_bfs_tree_async1__frame_t__ ),_make_bfs_tree_async1_));
    __hc__make_bfs_tree_async1__frame__ -> scope0.lb = ((int64_t )lb);
    __hc__make_bfs_tree_async1__frame__ -> scope0.ub = ((int64_t )ub);
    __hc__make_bfs_tree_async1__frame__ -> scope0.bfs_tree = ((int64_t *__restrict__ )bfs_tree);
    __hc__make_bfs_tree_async1__frame__ -> scope0.d = ((DDF_t *)d);
    ((hc_frameHeader *)__hc__make_bfs_tree_async1__frame__) -> sched_policy = hc_sched_policy(ws);
    increment_counter();
    if (((hc_frameHeader *)__hc__make_bfs_tree_async1__frame__) -> sched_policy ==  HELP_FIRST) {
      deque_push_default(__hc__make_bfs_tree_async1__frame__);
    }
    else {
      ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope2.lb = lb;
      ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope2.ub = ub;
      ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope2.d = d;
      ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.listOfBfsInit = listOfBfsInit;
      ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.p = p;
      ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.bfs_tree = bfs_tree;
      ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.vlist = vlist;
      ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.arg = arg;
      ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.i = i;
      ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.c = c;
      ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.bfs_tree_out = bfs_tree_out;
      ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.srcvtx = srcvtx;
      ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.bfs_tree_ddf = bfs_tree_ddf;
      __hc_frame__ -> pc = 1;
      deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_make_bfs_tree__, and execute async: %p in _make_bfs_tree_async1_\n", __hc_frame__, __hc_frame__->pc, __hc__make_bfs_tree_async1__frame__);
      _make_bfs_tree_async1_(ws,((hc_frameHeader *)__hc__make_bfs_tree_async1__frame__),0);
      if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
        return ;
      }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
      if (0) {
        __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_make_bfs_tree__\n", printEntry(__hc_frame__, buffer));
        lb = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope2.lb;
        ub = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope2.ub;
        d = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope2.d;
        listOfBfsInit = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.listOfBfsInit;
        p = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.p;
        bfs_tree = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.bfs_tree;
        vlist = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.vlist;
        arg = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.arg;
        i = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.i;
        c = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.c;
        bfs_tree_out = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.bfs_tree_out;
        srcvtx = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.srcvtx;
        bfs_tree_ddf = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.bfs_tree_ddf;
      }
    }
  }
  int64_t k;
  int64_t lb = (i * c);
  for (k = lb; k < nv; k++) {
    bfs_tree[k] = (-1);
  }
  bfs_tree[srcvtx] = srcvtx;
  struct __hc__make_bfs_tree_async2__frame_t__ *__hc__make_bfs_tree_async2__frame__ = (struct __hc__make_bfs_tree_async2__frame_t__ *)(hc_init_frame(sizeof(struct __hc__make_bfs_tree_async2__frame_t__ ),_make_bfs_tree_async2_));
  p_await_list_t __hc__make_bfs_tree_async2_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__make_bfs_tree_async2_awaitList_size = 0 + listOfBfsInit -> size + 0;
  __hc__make_bfs_tree_async2_awaitList -> array = hc_malloc((1 + __hc__make_bfs_tree_async2_awaitList_size) * sizeof(p_DDF_t ));
  __hc__make_bfs_tree_async2_awaitList -> array[__hc__make_bfs_tree_async2_awaitList_size] = NULL;
  unsigned int __hc__make_bfs_tree_async2_awaitList_curr_index = 0;
  __copyInto(__hc__make_bfs_tree_async2_awaitList,__hc__make_bfs_tree_async2_awaitList_curr_index,listOfBfsInit);
  __hc__make_bfs_tree_async2_awaitList_curr_index += listOfBfsInit -> size;
  __hc__make_bfs_tree_async2_awaitList -> waitingFrontier = &__hc__make_bfs_tree_async2_awaitList -> array[0];
  __hc__make_bfs_tree_async2__frame__ -> header.awaitList = __hc__make_bfs_tree_async2_awaitList;
  __hc__make_bfs_tree_async2__frame__ -> scope0.bfs_tree = ((int64_t *__restrict__ )bfs_tree);
  __hc__make_bfs_tree_async2__frame__ -> scope0.srcvtx = ((int64_t )srcvtx);
  __hc__make_bfs_tree_async2__frame__ -> scope0.arg = ((bfs_arg *)arg);
  __hc__make_bfs_tree_async2__frame__ -> scope0.vlist = ((int64_t *__restrict__ )vlist);
  __hc__make_bfs_tree_async2__frame__ -> scope0.bfs_tree_ddf = ((DDF_t *)bfs_tree_ddf);
  ((hc_frameHeader *)__hc__make_bfs_tree_async2__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__make_bfs_tree_async2__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__make_bfs_tree_async2__frame__ -> header)) 
      deque_push_default(__hc__make_bfs_tree_async2__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.listOfBfsInit = listOfBfsInit;
    ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.p = p;
    ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.bfs_tree = bfs_tree;
    ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.vlist = vlist;
    ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.arg = arg;
    ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.i = i;
    ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.c = c;
    ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.k = k;
    ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.lb = lb;
    ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.bfs_tree_out = bfs_tree_out;
    ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.srcvtx = srcvtx;
    ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.bfs_tree_ddf = bfs_tree_ddf;
    __hc_frame__ -> pc = 2;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_make_bfs_tree__, and execute async: %p in _make_bfs_tree_async2_\n", __hc_frame__, __hc_frame__->pc, __hc__make_bfs_tree_async2__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__make_bfs_tree_async2__frame__ -> header)) 
      _make_bfs_tree_async2_(ws,((hc_frameHeader *)__hc__make_bfs_tree_async2__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async2:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_make_bfs_tree__\n", printEntry(__hc_frame__, buffer));
      listOfBfsInit = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.listOfBfsInit;
      p = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.p;
      bfs_tree = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.bfs_tree;
      vlist = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.vlist;
      arg = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.arg;
      i = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.i;
      c = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.c;
      k = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.k;
      lb = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.lb;
      bfs_tree_out = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.bfs_tree_out;
      srcvtx = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.srcvtx;
      bfs_tree_ddf = ((struct __hc_make_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.bfs_tree_ddf;
    }
  }
  ws -> costatus =  HC_FUNC_COMPLETE;
  return ;
}

void _make_bfs_tree_async3_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  bfs_arg *arg;
  int64_t *__restrict__ vlist;
  DDF_t *bfs_tree_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  arg = ((struct __hc__make_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.arg;
  vlist = ((struct __hc__make_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.vlist;
  bfs_tree_ddf = ((struct __hc__make_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.bfs_tree_ddf;
  hc_mm_free(((hc_workerState *)(pthread_getspecific(wskey))),arg);
  xfree_large(vlist);
  __ddfPut(bfs_tree_ddf,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _make_bfs_tree_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t *__restrict__ bfs_tree;
  int64_t srcvtx;
  bfs_arg *arg;
  int64_t *__restrict__ vlist;
  DDF_t *bfs_tree_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_func2;
  }
  bfs_tree = ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
  srcvtx = ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.srcvtx;
  arg = ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.arg;
  vlist = ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.vlist;
  bfs_tree_ddf = ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_tree_ddf;
  DDF_t *bfs_fin = __ddfCreate();
  ws -> costatus =  HC_FUNC_COMPLETE;
  __hc_frame__ -> pc = 2;
  ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope1.bfs_fin = bfs_fin;
  ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_tree = bfs_tree;
  ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.srcvtx = srcvtx;
  ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.arg = arg;
  ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.vlist = vlist;
  ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_tree_ddf = bfs_tree_ddf;
  bfs_level_sync(arg,vlist,bfs_tree,0,bfs_fin);
  if (ws -> costatus !=  HC_FUNC_COMPLETE) 
    return ;
  if (0) {
    __post_func2:
    hc_free_unwind_frame(ws);
    __hc_frame__ = ws -> frame;
    bfs_fin = ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope1.bfs_fin;
    bfs_tree = ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
    srcvtx = ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.srcvtx;
    arg = ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.arg;
    vlist = ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.vlist;
    bfs_tree_ddf = ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_tree_ddf;
  }
/* Write back value for INOUT/OUT arguments */
  struct __hc__make_bfs_tree_async3__frame_t__ *__hc__make_bfs_tree_async3__frame__ = (struct __hc__make_bfs_tree_async3__frame_t__ *)(hc_init_frame(sizeof(struct __hc__make_bfs_tree_async3__frame_t__ ),_make_bfs_tree_async3_));
  p_await_list_t __hc__make_bfs_tree_async3_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__make_bfs_tree_async3_awaitList_size = 0 + 1;
  __hc__make_bfs_tree_async3_awaitList -> array = hc_malloc((1 + __hc__make_bfs_tree_async3_awaitList_size) * sizeof(p_DDF_t ));
  __hc__make_bfs_tree_async3_awaitList -> array[__hc__make_bfs_tree_async3_awaitList_size] = NULL;
  unsigned int __hc__make_bfs_tree_async3_awaitList_curr_index = 0;
  __hc__make_bfs_tree_async3_awaitList -> array[__hc__make_bfs_tree_async3_awaitList_curr_index++] = bfs_fin;
  __hc__make_bfs_tree_async3_awaitList -> waitingFrontier = &__hc__make_bfs_tree_async3_awaitList -> array[0];
  __hc__make_bfs_tree_async3__frame__ -> header.awaitList = __hc__make_bfs_tree_async3_awaitList;
  __hc__make_bfs_tree_async3__frame__ -> scope0.arg = ((bfs_arg *)arg);
  __hc__make_bfs_tree_async3__frame__ -> scope0.vlist = ((int64_t *__restrict__ )vlist);
  __hc__make_bfs_tree_async3__frame__ -> scope0.bfs_tree_ddf = ((DDF_t *)bfs_tree_ddf);
  ((hc_frameHeader *)__hc__make_bfs_tree_async3__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__make_bfs_tree_async3__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__make_bfs_tree_async3__frame__ -> header)) 
      deque_push_default(__hc__make_bfs_tree_async3__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope1.bfs_fin = bfs_fin;
    ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_tree = bfs_tree;
    ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.srcvtx = srcvtx;
    ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.arg = arg;
    ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.vlist = vlist;
    ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_tree_ddf = bfs_tree_ddf;
    __hc_frame__ -> pc = 1;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _make_bfs_tree_async2_, and execute async: %p in _make_bfs_tree_async3_\n", __hc_frame__, __hc_frame__->pc, __hc__make_bfs_tree_async3__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__make_bfs_tree_async3__frame__ -> header)) 
      _make_bfs_tree_async3_(ws,((hc_frameHeader *)__hc__make_bfs_tree_async3__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _make_bfs_tree_async2_\n", printEntry(__hc_frame__, buffer));
      bfs_fin = ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope1.bfs_fin;
      bfs_tree = ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
      srcvtx = ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.srcvtx;
      arg = ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.arg;
      vlist = ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.vlist;
      bfs_tree_ddf = ((struct __hc__make_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_tree_ddf;
    }
  }
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _make_bfs_tree_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t lb;
  int64_t ub;
  int64_t *__restrict__ bfs_tree;
  DDF_t *d;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  lb = ((struct __hc__make_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.lb;
  ub = ((struct __hc__make_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.ub;
  bfs_tree = ((struct __hc__make_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
  d = ((struct __hc__make_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.d;
  int64_t k;
  for (k = lb; k < ub; k++) {
    bfs_tree[k] = (-1);
  }
  __ddfPut(d,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

static void compute_levels_kernel(int64_t lb,int64_t ub,int64_t *level,int64_t nv,int64_t *__restrict__ bfs_tree,int64_t root,volatile int *err)
{
  int terr;
  int64_t k;
{
    for (k = lb; k < ub; k++) {
      if (level[k] >= 0) 
        continue; 
      terr =  *err;
      if (terr != 0) 
        break; 
      if ((!(terr != 0) && (bfs_tree[k] >= 0)) && (k != root)) {
        int64_t parent = k;
        int64_t nhop = 0;
/* Run up the tree until we encounter an already-leveled vertex. */
        while(((parent >= 0) && (level[parent] < 0)) && (nhop < nv)){
          (__builtin_expect((!(parent != bfs_tree[parent])),0) != 0L)?__assert_rtn(__func__,"/Users/dm14/Documents/Habanero/hc-trunk/examples/graph500-2.1.4/graph500-ddf.hc",1057,"parent != bfs_tree[parent]") : ((void )0);
          parent = bfs_tree[parent];
          ++nhop;
        }
/* Cycle. */
        if (nhop >= nv) 
          terr = -1;
/* Ran off the end. */
        if (parent < 0) 
          terr = -2;
        if (!(terr != 0)) {
/* Now assign levels until we meet an already-leveled vertex */
/* NOTE: This permits benign races if parallelized. */
          nhop += level[parent];
          parent = k;
          while(level[parent] < 0){
            (__builtin_expect((!(nhop > 0)),0) != 0L)?__assert_rtn(__func__,"/Users/dm14/Documents/Habanero/hc-trunk/examples/graph500-2.1.4/graph500-ddf.hc",1070,"nhop > 0") : ((void )0);
            level[parent] = nhop--;
            parent = bfs_tree[parent];
          }
          (__builtin_expect((!(nhop == level[parent])),0) != 0L)?__assert_rtn(__func__,"/Users/dm14/Documents/Habanero/hc-trunk/examples/graph500-2.1.4/graph500-ddf.hc",1074,"nhop == level[parent]") : ((void )0);
/* Internal check to catch mistakes in races... */
#if !defined(NDEBUG)
          nhop = 0;
          parent = k;
          int64_t lastlvl = (level[k] + 1);
          while(level[parent] > 0){
            (__builtin_expect((!(lastlvl == (1 + level[parent]))),0) != 0L)?__assert_rtn(__func__,"/Users/dm14/Documents/Habanero/hc-trunk/examples/graph500-2.1.4/graph500-ddf.hc",1082,"lastlvl == 1 + level[parent]") : ((void )0);
            lastlvl = level[parent];
            parent = bfs_tree[parent];
            ++nhop;
          }
#endif
        }
      }
      if (terr != 0) {
         *err = terr;
        hc_mfence();
      }
    }
  }
}
typedef struct __hc_compute_levels_frame_t__ {
hc_frameHeader header;
struct __hc_compute_levels_scope0_frame_t__ {
int64_t *level;
int64_t nv;
int64_t *__restrict__ bfs_tree;
int64_t root;
int64_t p;
volatile int *err;
DDF_t *compute_levels_ddf;}scope0;
struct __hc_compute_levels_scope1_frame_t__ {
p_DDF_list_t listOfLevelInit;
int64_t i;
int64_t c;
int64_t k;
int64_t lb;}scope1;
struct __hc_compute_levels_scope2_frame_t__ {
int64_t lb;
int64_t ub;
DDF_t *d;}scope2;}__hc_compute_levels_frame_t__;
typedef struct __hc__compute_levels_async1__frame_t__ {
hc_frameHeader header;
struct __hc__compute_levels_async1__scope0_frame_t__ {
int64_t lb;
int64_t ub;
int64_t *level;
DDF_t *d;}scope0;}__hc__compute_levels_async1__frame_t__;
typedef struct __hc__compute_levels_async2__frame_t__ {
hc_frameHeader header;
struct __hc__compute_levels_async2__scope0_frame_t__ {
int64_t *level;
int64_t nv;
int64_t *__restrict__ bfs_tree;
int64_t root;
int64_t p;
volatile int *err;
DDF_t *compute_levels_ddf;}scope0;
struct __hc__compute_levels_async2__scope1_frame_t__ {
p_DDF_list_t listOfComputeLevel;
int64_t i;
int64_t c;
int64_t lb;}scope1;
struct __hc__compute_levels_async2__scope2_frame_t__ {
int64_t lb;
int64_t ub;
DDF_t *d;}scope2;}__hc__compute_levels_async2__frame_t__;
typedef struct __hc__compute_levels_async3__frame_t__ {
hc_frameHeader header;
struct __hc__compute_levels_async3__scope0_frame_t__ {
int64_t lb;
int64_t ub;
int64_t *level;
int64_t nv;
int64_t *__restrict__ bfs_tree;
int64_t root;
volatile int *err;
DDF_t *d;}scope0;}__hc__compute_levels_async3__frame_t__;
typedef struct __hc__compute_levels_async4__frame_t__ {
hc_frameHeader header;
struct __hc__compute_levels_async4__scope0_frame_t__ {
volatile int *err;
DDF_t *compute_levels_ddf;}scope0;}__hc__compute_levels_async4__frame_t__;
void __hc_compute_levels__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _compute_levels_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _compute_levels_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _compute_levels_async3_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _compute_levels_async4_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);

static void compute_levels(int64_t *level,int64_t nv,int64_t *__restrict__ bfs_tree,int64_t root,int64_t p,volatile int *err,DDF_t *compute_levels_ddf)
{
  hc_workerState *ws = current_ws();
  ws -> costatus =  HC_FUNC_COMPLETE;
  struct __hc_compute_levels_frame_t__ *__hc_frame__ = (struct __hc_compute_levels_frame_t__ *)(hc_init_frame_ws(ws,sizeof(struct __hc_compute_levels_frame_t__ ),__hc_compute_levels__));
  __hc_frame__ -> scope0.level = level;
  __hc_frame__ -> scope0.nv = nv;
  __hc_frame__ -> scope0.bfs_tree = bfs_tree;
  __hc_frame__ -> scope0.root = root;
  __hc_frame__ -> scope0.p = p;
  __hc_frame__ -> scope0.err = err;
  __hc_frame__ -> scope0.compute_levels_ddf = compute_levels_ddf;
			LOG_INFO(ws, "execute frame: %p, parent: %p of function %s\n", __hc_frame__,((hc_frameHeader*) __hc_frame__)->parent, "__hc_compute_levels__" );
  __hc_compute_levels__(ws,((hc_frameHeader *)__hc_frame__),0);
  if (ws -> costatus ==  HC_FUNC_COMPLETE) 
    hc_free_unwind_frame(ws);
  return ;
}

void __hc_compute_levels__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t *level;
  int64_t nv;
  int64_t *__restrict__ bfs_tree;
  int64_t root;
  int64_t p;
  volatile int *err;
  DDF_t *compute_levels_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_async2;
  }
  level = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.level;
  nv = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.nv;
  bfs_tree = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
  root = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.root;
  p = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.p;
  err = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.err;
  compute_levels_ddf = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.compute_levels_ddf;
  p_DDF_list_t listOfLevelInit = __ddfListCreate();
   *err = 0;
  int64_t i;
  int64_t c = (nv / p);
  for (i = 0; i < (p - 1); i++) {
    int64_t lb = (i * c);
    int64_t ub = (lb + c);
    DDF_t *d = __ddfCreate();
    __enlist(d,listOfLevelInit);
    struct __hc__compute_levels_async1__frame_t__ *__hc__compute_levels_async1__frame__ = (struct __hc__compute_levels_async1__frame_t__ *)(hc_init_frame(sizeof(struct __hc__compute_levels_async1__frame_t__ ),_compute_levels_async1_));
    __hc__compute_levels_async1__frame__ -> scope0.lb = ((int64_t )lb);
    __hc__compute_levels_async1__frame__ -> scope0.ub = ((int64_t )ub);
    __hc__compute_levels_async1__frame__ -> scope0.level = ((int64_t *)level);
    __hc__compute_levels_async1__frame__ -> scope0.d = ((DDF_t *)d);
    ((hc_frameHeader *)__hc__compute_levels_async1__frame__) -> sched_policy = hc_sched_policy(ws);
    increment_counter();
    if (((hc_frameHeader *)__hc__compute_levels_async1__frame__) -> sched_policy ==  HELP_FIRST) {
      deque_push_default(__hc__compute_levels_async1__frame__);
    }
    else {
      ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope2.lb = lb;
      ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope2.ub = ub;
      ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope2.d = d;
      ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope1.listOfLevelInit = listOfLevelInit;
      ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope1.i = i;
      ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope1.c = c;
      ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.level = level;
      ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.nv = nv;
      ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.bfs_tree = bfs_tree;
      ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.root = root;
      ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.p = p;
      ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.err = err;
      ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.compute_levels_ddf = compute_levels_ddf;
      __hc_frame__ -> pc = 1;
      deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_compute_levels__, and execute async: %p in _compute_levels_async1_\n", __hc_frame__, __hc_frame__->pc, __hc__compute_levels_async1__frame__);
      _compute_levels_async1_(ws,((hc_frameHeader *)__hc__compute_levels_async1__frame__),0);
      if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
        return ;
      }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
      if (0) {
        __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_compute_levels__\n", printEntry(__hc_frame__, buffer));
        lb = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope2.lb;
        ub = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope2.ub;
        d = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope2.d;
        listOfLevelInit = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope1.listOfLevelInit;
        i = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope1.i;
        c = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope1.c;
        level = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.level;
        nv = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.nv;
        bfs_tree = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
        root = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.root;
        p = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.p;
        err = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.err;
        compute_levels_ddf = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.compute_levels_ddf;
      }
    }
  }
  int64_t k;
  int64_t lb = (i * c);
  for (k = lb; k < nv; k++) {
    level[k] = (-1);
  }
  struct __hc__compute_levels_async2__frame_t__ *__hc__compute_levels_async2__frame__ = (struct __hc__compute_levels_async2__frame_t__ *)(hc_init_frame(sizeof(struct __hc__compute_levels_async2__frame_t__ ),_compute_levels_async2_));
  p_await_list_t __hc__compute_levels_async2_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__compute_levels_async2_awaitList_size = 0 + listOfLevelInit -> size + 0;
  __hc__compute_levels_async2_awaitList -> array = hc_malloc((1 + __hc__compute_levels_async2_awaitList_size) * sizeof(p_DDF_t ));
  __hc__compute_levels_async2_awaitList -> array[__hc__compute_levels_async2_awaitList_size] = NULL;
  unsigned int __hc__compute_levels_async2_awaitList_curr_index = 0;
  __copyInto(__hc__compute_levels_async2_awaitList,__hc__compute_levels_async2_awaitList_curr_index,listOfLevelInit);
  __hc__compute_levels_async2_awaitList_curr_index += listOfLevelInit -> size;
  __hc__compute_levels_async2_awaitList -> waitingFrontier = &__hc__compute_levels_async2_awaitList -> array[0];
  __hc__compute_levels_async2__frame__ -> header.awaitList = __hc__compute_levels_async2_awaitList;
  __hc__compute_levels_async2__frame__ -> scope0.level = ((int64_t *)level);
  __hc__compute_levels_async2__frame__ -> scope0.nv = ((int64_t )nv);
  __hc__compute_levels_async2__frame__ -> scope0.bfs_tree = ((int64_t *__restrict__ )bfs_tree);
  __hc__compute_levels_async2__frame__ -> scope0.root = ((int64_t )root);
  __hc__compute_levels_async2__frame__ -> scope0.p = ((int64_t )p);
  __hc__compute_levels_async2__frame__ -> scope0.err = ((volatile int *)err);
  __hc__compute_levels_async2__frame__ -> scope0.compute_levels_ddf = ((DDF_t *)compute_levels_ddf);
  ((hc_frameHeader *)__hc__compute_levels_async2__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__compute_levels_async2__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__compute_levels_async2__frame__ -> header)) 
      deque_push_default(__hc__compute_levels_async2__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope1.listOfLevelInit = listOfLevelInit;
    ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope1.i = i;
    ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope1.c = c;
    ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope1.k = k;
    ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope1.lb = lb;
    ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.level = level;
    ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.nv = nv;
    ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.bfs_tree = bfs_tree;
    ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.root = root;
    ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.p = p;
    ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.err = err;
    ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.compute_levels_ddf = compute_levels_ddf;
    __hc_frame__ -> pc = 2;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_compute_levels__, and execute async: %p in _compute_levels_async2_\n", __hc_frame__, __hc_frame__->pc, __hc__compute_levels_async2__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__compute_levels_async2__frame__ -> header)) 
      _compute_levels_async2_(ws,((hc_frameHeader *)__hc__compute_levels_async2__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async2:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_compute_levels__\n", printEntry(__hc_frame__, buffer));
      listOfLevelInit = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope1.listOfLevelInit;
      i = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope1.i;
      c = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope1.c;
      k = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope1.k;
      lb = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope1.lb;
      level = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.level;
      nv = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.nv;
      bfs_tree = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
      root = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.root;
      p = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.p;
      err = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.err;
      compute_levels_ddf = ((struct __hc_compute_levels_frame_t__ *)__hc_frame__) -> scope0.compute_levels_ddf;
    }
  }
  ws -> costatus =  HC_FUNC_COMPLETE;
  return ;
}

void _compute_levels_async4_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  volatile int *err;
  DDF_t *compute_levels_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  err = ((struct __hc__compute_levels_async4__frame_t__ *)__hc_frame__) -> scope0.err;
  compute_levels_ddf = ((struct __hc__compute_levels_async4__frame_t__ *)__hc_frame__) -> scope0.compute_levels_ddf;
  __ddfPut(compute_levels_ddf,((int *)err));
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _compute_levels_async3_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t lb;
  int64_t ub;
  int64_t *level;
  int64_t nv;
  int64_t *__restrict__ bfs_tree;
  int64_t root;
  volatile int *err;
  DDF_t *d;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  lb = ((struct __hc__compute_levels_async3__frame_t__ *)__hc_frame__) -> scope0.lb;
  ub = ((struct __hc__compute_levels_async3__frame_t__ *)__hc_frame__) -> scope0.ub;
  level = ((struct __hc__compute_levels_async3__frame_t__ *)__hc_frame__) -> scope0.level;
  nv = ((struct __hc__compute_levels_async3__frame_t__ *)__hc_frame__) -> scope0.nv;
  bfs_tree = ((struct __hc__compute_levels_async3__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
  root = ((struct __hc__compute_levels_async3__frame_t__ *)__hc_frame__) -> scope0.root;
  err = ((struct __hc__compute_levels_async3__frame_t__ *)__hc_frame__) -> scope0.err;
  d = ((struct __hc__compute_levels_async3__frame_t__ *)__hc_frame__) -> scope0.d;
  compute_levels_kernel(lb,ub,level,nv,bfs_tree,root,err);
  __ddfPut(d,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _compute_levels_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t *level;
  int64_t nv;
  int64_t *__restrict__ bfs_tree;
  int64_t root;
  int64_t p;
  volatile int *err;
  DDF_t *compute_levels_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_async2;
  }
  level = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.level;
  nv = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.nv;
  bfs_tree = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
  root = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.root;
  p = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.p;
  err = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.err;
  compute_levels_ddf = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.compute_levels_ddf;
  level[root] = 0;
  p_DDF_list_t listOfComputeLevel = __ddfListCreate();
  int64_t i;
  int64_t c = (nv / p);
  for (i = 0; i < (p - 1); i++) {
    int64_t lb = (i * c);
    int64_t ub = (lb + c);
    DDF_t *d = __ddfCreate();
    __enlist(d,listOfComputeLevel);
    struct __hc__compute_levels_async3__frame_t__ *__hc__compute_levels_async3__frame__ = (struct __hc__compute_levels_async3__frame_t__ *)(hc_init_frame(sizeof(struct __hc__compute_levels_async3__frame_t__ ),_compute_levels_async3_));
    __hc__compute_levels_async3__frame__ -> scope0.lb = ((int64_t )lb);
    __hc__compute_levels_async3__frame__ -> scope0.ub = ((int64_t )ub);
    __hc__compute_levels_async3__frame__ -> scope0.level = ((int64_t *)level);
    __hc__compute_levels_async3__frame__ -> scope0.nv = ((int64_t )nv);
    __hc__compute_levels_async3__frame__ -> scope0.bfs_tree = ((int64_t *__restrict__ )bfs_tree);
    __hc__compute_levels_async3__frame__ -> scope0.root = ((int64_t )root);
    __hc__compute_levels_async3__frame__ -> scope0.err = ((volatile int *)err);
    __hc__compute_levels_async3__frame__ -> scope0.d = ((DDF_t *)d);
    ((hc_frameHeader *)__hc__compute_levels_async3__frame__) -> sched_policy = hc_sched_policy(ws);
    increment_counter();
    if (((hc_frameHeader *)__hc__compute_levels_async3__frame__) -> sched_policy ==  HELP_FIRST) {
      deque_push_default(__hc__compute_levels_async3__frame__);
    }
    else {
      ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope2.lb = lb;
      ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope2.ub = ub;
      ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope2.d = d;
      ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope1.listOfComputeLevel = listOfComputeLevel;
      ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope1.i = i;
      ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope1.c = c;
      ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.level = level;
      ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.nv = nv;
      ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_tree = bfs_tree;
      ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.root = root;
      ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.p = p;
      ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.err = err;
      ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.compute_levels_ddf = compute_levels_ddf;
      __hc_frame__ -> pc = 1;
      deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _compute_levels_async2_, and execute async: %p in _compute_levels_async3_\n", __hc_frame__, __hc_frame__->pc, __hc__compute_levels_async3__frame__);
      _compute_levels_async3_(ws,((hc_frameHeader *)__hc__compute_levels_async3__frame__),0);
      if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
        return ;
      }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
      if (0) {
        __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _compute_levels_async2_\n", printEntry(__hc_frame__, buffer));
        lb = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope2.lb;
        ub = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope2.ub;
        d = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope2.d;
        listOfComputeLevel = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope1.listOfComputeLevel;
        i = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope1.i;
        c = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope1.c;
        level = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.level;
        nv = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.nv;
        bfs_tree = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
        root = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.root;
        p = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.p;
        err = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.err;
        compute_levels_ddf = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.compute_levels_ddf;
      }
    }
  }
  int64_t lb = (i * c);
  compute_levels_kernel(lb,nv,level,nv,bfs_tree,root,err);
/* Write back value for INOUT/OUT arguments */
  struct __hc__compute_levels_async4__frame_t__ *__hc__compute_levels_async4__frame__ = (struct __hc__compute_levels_async4__frame_t__ *)(hc_init_frame(sizeof(struct __hc__compute_levels_async4__frame_t__ ),_compute_levels_async4_));
  p_await_list_t __hc__compute_levels_async4_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__compute_levels_async4_awaitList_size = 0 + listOfComputeLevel -> size + 0;
  __hc__compute_levels_async4_awaitList -> array = hc_malloc((1 + __hc__compute_levels_async4_awaitList_size) * sizeof(p_DDF_t ));
  __hc__compute_levels_async4_awaitList -> array[__hc__compute_levels_async4_awaitList_size] = NULL;
  unsigned int __hc__compute_levels_async4_awaitList_curr_index = 0;
  __copyInto(__hc__compute_levels_async4_awaitList,__hc__compute_levels_async4_awaitList_curr_index,listOfComputeLevel);
  __hc__compute_levels_async4_awaitList_curr_index += listOfComputeLevel -> size;
  __hc__compute_levels_async4_awaitList -> waitingFrontier = &__hc__compute_levels_async4_awaitList -> array[0];
  __hc__compute_levels_async4__frame__ -> header.awaitList = __hc__compute_levels_async4_awaitList;
  __hc__compute_levels_async4__frame__ -> scope0.err = ((volatile int *)err);
  __hc__compute_levels_async4__frame__ -> scope0.compute_levels_ddf = ((DDF_t *)compute_levels_ddf);
  ((hc_frameHeader *)__hc__compute_levels_async4__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__compute_levels_async4__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__compute_levels_async4__frame__ -> header)) 
      deque_push_default(__hc__compute_levels_async4__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope1.listOfComputeLevel = listOfComputeLevel;
    ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope1.i = i;
    ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope1.c = c;
    ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope1.lb = lb;
    ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.level = level;
    ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.nv = nv;
    ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_tree = bfs_tree;
    ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.root = root;
    ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.p = p;
    ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.err = err;
    ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.compute_levels_ddf = compute_levels_ddf;
    __hc_frame__ -> pc = 2;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _compute_levels_async2_, and execute async: %p in _compute_levels_async4_\n", __hc_frame__, __hc_frame__->pc, __hc__compute_levels_async4__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__compute_levels_async4__frame__ -> header)) 
      _compute_levels_async4_(ws,((hc_frameHeader *)__hc__compute_levels_async4__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async2:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _compute_levels_async2_\n", printEntry(__hc_frame__, buffer));
      listOfComputeLevel = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope1.listOfComputeLevel;
      i = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope1.i;
      c = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope1.c;
      lb = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope1.lb;
      level = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.level;
      nv = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.nv;
      bfs_tree = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
      root = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.root;
      p = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.p;
      err = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.err;
      compute_levels_ddf = ((struct __hc__compute_levels_async2__frame_t__ *)__hc_frame__) -> scope0.compute_levels_ddf;
    }
  }
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _compute_levels_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t lb;
  int64_t ub;
  int64_t *level;
  DDF_t *d;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  lb = ((struct __hc__compute_levels_async1__frame_t__ *)__hc_frame__) -> scope0.lb;
  ub = ((struct __hc__compute_levels_async1__frame_t__ *)__hc_frame__) -> scope0.ub;
  level = ((struct __hc__compute_levels_async1__frame_t__ *)__hc_frame__) -> scope0.level;
  d = ((struct __hc__compute_levels_async1__frame_t__ *)__hc_frame__) -> scope0.d;
  int64_t k;
  for (k = lb; k < ub; k++) {
    level[k] = (-1);
  }
  __ddfPut(d,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

static void verify_kernel(int id,int64_t lb,int64_t ub,int64_t *__restrict__ bfs_tree,int64_t max_bfsvtx,int64_t *IJ,int64_t *__restrict__ seen_edge,int64_t *__restrict__ level,volatile int *nedgeArray,volatile int *err)
{
  int terr;
  int64_t k;
  int nedge_traversed = 0;
{
    for (k = lb; k < ub; k++) {{
        int64_t i = IJ[2 * k];
        int64_t j = IJ[(2 * k) + 1];
        int64_t lvldiff;
        terr =  *err;
        if (terr != 0) 
          break; 
        if ((i < 0) || (j < 0)) 
          continue; 
        if ((i > max_bfsvtx) && (j <= max_bfsvtx)) 
          terr = -'\n';
        if ((j > max_bfsvtx) && (i <= max_bfsvtx)) 
          terr = -11;
        if (terr != 0) {
           *err = terr;
          hc_mfence();
        }
/* both i & j are on the same side of max_bfsvtx */
        if ((terr != 0) || (i > max_bfsvtx)) 
          continue; 
/* All neighbors must be in the tree. */
        if ((((int )bfs_tree[i]) >= 0) && (((int )bfs_tree[j]) < 0)) 
          terr = -12;
        if ((((int )bfs_tree[j]) >= 0) && (((int )bfs_tree[i]) < 0)) 
          terr = -13;
        if (terr != 0) {
           *err = terr;
          hc_mfence();
        }
/* both i & j have the same sign */
        if ((terr != 0) || (((int )bfs_tree[i]) < 0)) 
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
        lvldiff = (level[i] - level[j]);
/* Check that the levels differ by no more than one. */
        if ((lvldiff > 1) || (lvldiff < (-1))) 
          terr = -14;
        if (terr != 0) {
           *err = terr;
          hc_mfence();
        }
      }
    }
  }
  nedgeArray[id] = nedge_traversed;
}
typedef struct __hc_verify_bfs_tree_frame_t__ {
hc_frameHeader header;
struct __hc_verify_bfs_tree_scope0_frame_t__ {
int64_t *bfs_tree_in;
int64_t max_bfsvtx;
int64_t root;
struct packed_edge *IJ_in;
int64_t nedge;
volatile int *err;
DDF_t *verify_bfs_ddf;}scope0;
struct __hc_verify_bfs_tree_scope1_frame_t__ {
int64_t *__restrict__ bfs_tree;
struct packed_edge *__restrict__ IJ;
int64_t *__restrict__ seen_edge;
int64_t *__restrict__ level;
int64_t nv;
int p;
DDF_t *compute_levels_ddf;}scope1;}__hc_verify_bfs_tree_frame_t__;
typedef struct __hc__verify_bfs_tree_async1__frame_t__ {
hc_frameHeader header;
struct __hc__verify_bfs_tree_async1__scope0_frame_t__ {
int64_t *__restrict__ bfs_tree;
int64_t max_bfsvtx;
int64_t root;
struct packed_edge *__restrict__ IJ;
int64_t nedge;
int64_t *__restrict__ seen_edge;
int64_t *__restrict__ level;
int64_t nv;
int p;
volatile int *err;
DDF_t *verify_bfs_ddf;}scope0;
struct __hc__verify_bfs_tree_async1__scope1_frame_t__ {
p_DDF_list_t listOfSeenInit;
int64_t i;
int64_t c;
int64_t k;
int64_t lb;}scope1;
struct __hc__verify_bfs_tree_async1__scope2_frame_t__ {
int64_t lb;
int64_t ub;
DDF_t *d;}scope2;}__hc__verify_bfs_tree_async1__frame_t__;
typedef struct __hc__verify_bfs_tree_async2__frame_t__ {
hc_frameHeader header;
struct __hc__verify_bfs_tree_async2__scope0_frame_t__ {
int64_t lb;
int64_t ub;
int64_t *__restrict__ seen_edge;
DDF_t *d;}scope0;}__hc__verify_bfs_tree_async2__frame_t__;
typedef struct __hc__verify_bfs_tree_async3__frame_t__ {
hc_frameHeader header;
struct __hc__verify_bfs_tree_async3__scope0_frame_t__ {
int64_t *__restrict__ bfs_tree;
int64_t max_bfsvtx;
int64_t root;
struct packed_edge *__restrict__ IJ;
int64_t nedge;
int64_t *__restrict__ seen_edge;
int64_t *__restrict__ level;
int64_t nv;
int p;
volatile int *err;
DDF_t *verify_bfs_ddf;}scope0;
struct __hc__verify_bfs_tree_async3__scope1_frame_t__ {
p_DDF_list_t listOfVerifyKernel;
volatile int *nedgeArray;
int64_t i;
int64_t c;
int64_t lb;}scope1;
struct __hc__verify_bfs_tree_async3__scope2_frame_t__ {
int64_t lb;
int64_t ub;
DDF_t *d;}scope2;}__hc__verify_bfs_tree_async3__frame_t__;
typedef struct __hc__verify_bfs_tree_async4__frame_t__ {
hc_frameHeader header;
struct __hc__verify_bfs_tree_async4__scope0_frame_t__ {
int64_t i;
int64_t lb;
int64_t ub;
int64_t *__restrict__ bfs_tree;
int64_t max_bfsvtx;
struct packed_edge *__restrict__ IJ;
int64_t *__restrict__ seen_edge;
int64_t *__restrict__ level;
volatile int *nedgeArray;
volatile int *err;
DDF_t *d;}scope0;}__hc__verify_bfs_tree_async4__frame_t__;
typedef struct __hc__verify_bfs_tree_async5__frame_t__ {
hc_frameHeader header;
struct __hc__verify_bfs_tree_async5__scope0_frame_t__ {
int64_t *__restrict__ bfs_tree;
int64_t root;
int64_t *__restrict__ seen_edge;
volatile int *nedgeArray;
int64_t nv;
int p;
volatile int *err;
DDF_t *verify_bfs_ddf;}scope0;
struct __hc__verify_bfs_tree_async5__scope1_frame_t__ {
int nedge_traversed;
int k;
p_DDF_list_t listOfErrorCheck;
int64_t i;
int64_t c;
int64_t lb;}scope1;
struct __hc__verify_bfs_tree_async5__scope2_frame_t__ {
int64_t lb;
int64_t ub;
DDF_t *d;}scope2;
struct __hc__verify_bfs_tree_async5__scope3_frame_t__ {
int terr;}scope3;}__hc__verify_bfs_tree_async5__frame_t__;
typedef struct __hc__verify_bfs_tree_async6__frame_t__ {
hc_frameHeader header;
struct __hc__verify_bfs_tree_async6__scope0_frame_t__ {
int64_t lb;
int64_t ub;
int64_t *__restrict__ bfs_tree;
int64_t *__restrict__ seen_edge;
int64_t root;
volatile int *err;
DDF_t *d;}scope0;}__hc__verify_bfs_tree_async6__frame_t__;
typedef struct __hc__verify_bfs_tree_async7__frame_t__ {
hc_frameHeader header;
struct __hc__verify_bfs_tree_async7__scope0_frame_t__ {
int64_t *__restrict__ seen_edge;
int nedge_traversed;
volatile int *err;
DDF_t *verify_bfs_ddf;}scope0;}__hc__verify_bfs_tree_async7__frame_t__;
void __hc_verify_bfs_tree__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _verify_bfs_tree_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _verify_bfs_tree_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _verify_bfs_tree_async3_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _verify_bfs_tree_async4_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _verify_bfs_tree_async5_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _verify_bfs_tree_async6_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _verify_bfs_tree_async7_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);

static void verify_bfs_tree(int64_t *bfs_tree_in,int64_t max_bfsvtx,int64_t root,struct packed_edge *IJ_in,int64_t nedge,volatile int *err,DDF_t *verify_bfs_ddf)
{
  hc_workerState *ws = current_ws();
  ws -> costatus =  HC_FUNC_COMPLETE;
  struct __hc_verify_bfs_tree_frame_t__ *__hc_frame__ = (struct __hc_verify_bfs_tree_frame_t__ *)(hc_init_frame_ws(ws,sizeof(struct __hc_verify_bfs_tree_frame_t__ ),__hc_verify_bfs_tree__));
  __hc_frame__ -> scope0.bfs_tree_in = bfs_tree_in;
  __hc_frame__ -> scope0.max_bfsvtx = max_bfsvtx;
  __hc_frame__ -> scope0.root = root;
  __hc_frame__ -> scope0.IJ_in = IJ_in;
  __hc_frame__ -> scope0.nedge = nedge;
  __hc_frame__ -> scope0.err = err;
  __hc_frame__ -> scope0.verify_bfs_ddf = verify_bfs_ddf;
			LOG_INFO(ws, "execute frame: %p, parent: %p of function %s\n", __hc_frame__,((hc_frameHeader*) __hc_frame__)->parent, "__hc_verify_bfs_tree__" );
  __hc_verify_bfs_tree__(ws,((hc_frameHeader *)__hc_frame__),0);
  if (ws -> costatus ==  HC_FUNC_COMPLETE) 
    hc_free_unwind_frame(ws);
  return ;
}

void __hc_verify_bfs_tree__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t *bfs_tree_in;
  int64_t max_bfsvtx;
  int64_t root;
  struct packed_edge *IJ_in;
  int64_t nedge;
  volatile int *err;
  DDF_t *verify_bfs_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_func2;
  }
  bfs_tree_in = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.bfs_tree_in;
  max_bfsvtx = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.max_bfsvtx;
  root = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.root;
  IJ_in = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.IJ_in;
  nedge = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.nedge;
  err = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.err;
  verify_bfs_ddf = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf;
  int64_t *__restrict__ bfs_tree = bfs_tree_in;
  struct packed_edge *__restrict__ IJ = IJ_in;
  int64_t *__restrict__ seen_edge;
  int64_t *__restrict__ level;
  int64_t nv = (max_bfsvtx + 1);
  int p = ( *( *((hc_workerState *)(pthread_getspecific(wskey)))).context).nproc;
/*
	This code is horrifically contorted because many compilers
	complain about continue, return, etc. in parallel sections.
  */
   *err = 0;
  if ((root > max_bfsvtx) || (bfs_tree[root] != root)) {
     *err = -999;
    __ddfPut(verify_bfs_ddf,((int *)err));
    ws -> costatus =  HC_FUNC_COMPLETE;
    return ;
  }
  seen_edge = (xmalloc_large(((2 * nv) * sizeof(( *seen_edge)))));
  level = (seen_edge + nv);
  DDF_t *compute_levels_ddf = __ddfCreate();
  ws -> costatus =  HC_FUNC_COMPLETE;
  __hc_frame__ -> pc = 2;
  ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.bfs_tree = bfs_tree;
  ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.IJ = IJ;
  ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.seen_edge = seen_edge;
  ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.level = level;
  ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.nv = nv;
  ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.p = p;
  ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.compute_levels_ddf = compute_levels_ddf;
  ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.bfs_tree_in = bfs_tree_in;
  ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.max_bfsvtx = max_bfsvtx;
  ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.root = root;
  ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.IJ_in = IJ_in;
  ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.nedge = nedge;
  ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.err = err;
  ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf = verify_bfs_ddf;
  compute_levels(level,nv,bfs_tree,root,p,err,compute_levels_ddf);
  if (ws -> costatus !=  HC_FUNC_COMPLETE) 
    return ;
  if (0) {
    __post_func2:
    hc_free_unwind_frame(ws);
    __hc_frame__ = ws -> frame;
    bfs_tree = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.bfs_tree;
    IJ = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.IJ;
    seen_edge = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.seen_edge;
    level = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.level;
    nv = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.nv;
    p = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.p;
    compute_levels_ddf = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.compute_levels_ddf;
    bfs_tree_in = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.bfs_tree_in;
    max_bfsvtx = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.max_bfsvtx;
    root = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.root;
    IJ_in = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.IJ_in;
    nedge = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.nedge;
    err = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.err;
    verify_bfs_ddf = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf;
  }
  struct __hc__verify_bfs_tree_async1__frame_t__ *__hc__verify_bfs_tree_async1__frame__ = (struct __hc__verify_bfs_tree_async1__frame_t__ *)(hc_init_frame(sizeof(struct __hc__verify_bfs_tree_async1__frame_t__ ),_verify_bfs_tree_async1_));
  p_await_list_t __hc__verify_bfs_tree_async1_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__verify_bfs_tree_async1_awaitList_size = 0 + 1;
  __hc__verify_bfs_tree_async1_awaitList -> array = hc_malloc((1 + __hc__verify_bfs_tree_async1_awaitList_size) * sizeof(p_DDF_t ));
  __hc__verify_bfs_tree_async1_awaitList -> array[__hc__verify_bfs_tree_async1_awaitList_size] = NULL;
  unsigned int __hc__verify_bfs_tree_async1_awaitList_curr_index = 0;
  __hc__verify_bfs_tree_async1_awaitList -> array[__hc__verify_bfs_tree_async1_awaitList_curr_index++] = compute_levels_ddf;
  __hc__verify_bfs_tree_async1_awaitList -> waitingFrontier = &__hc__verify_bfs_tree_async1_awaitList -> array[0];
  __hc__verify_bfs_tree_async1__frame__ -> header.awaitList = __hc__verify_bfs_tree_async1_awaitList;
  __hc__verify_bfs_tree_async1__frame__ -> scope0.bfs_tree = ((int64_t *__restrict__ )bfs_tree);
  __hc__verify_bfs_tree_async1__frame__ -> scope0.max_bfsvtx = ((int64_t )max_bfsvtx);
  __hc__verify_bfs_tree_async1__frame__ -> scope0.root = ((int64_t )root);
  __hc__verify_bfs_tree_async1__frame__ -> scope0.IJ = ((struct packed_edge *__restrict__ )IJ);
  __hc__verify_bfs_tree_async1__frame__ -> scope0.nedge = ((int64_t )nedge);
  __hc__verify_bfs_tree_async1__frame__ -> scope0.seen_edge = ((int64_t *__restrict__ )seen_edge);
  __hc__verify_bfs_tree_async1__frame__ -> scope0.level = ((int64_t *__restrict__ )level);
  __hc__verify_bfs_tree_async1__frame__ -> scope0.nv = ((int64_t )nv);
  __hc__verify_bfs_tree_async1__frame__ -> scope0.p = ((int )p);
  __hc__verify_bfs_tree_async1__frame__ -> scope0.err = ((volatile int *)err);
  __hc__verify_bfs_tree_async1__frame__ -> scope0.verify_bfs_ddf = ((DDF_t *)verify_bfs_ddf);
  ((hc_frameHeader *)__hc__verify_bfs_tree_async1__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__verify_bfs_tree_async1__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__verify_bfs_tree_async1__frame__ -> header)) 
      deque_push_default(__hc__verify_bfs_tree_async1__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.bfs_tree = bfs_tree;
    ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.IJ = IJ;
    ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.seen_edge = seen_edge;
    ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.level = level;
    ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.nv = nv;
    ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.p = p;
    ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.compute_levels_ddf = compute_levels_ddf;
    ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.bfs_tree_in = bfs_tree_in;
    ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.max_bfsvtx = max_bfsvtx;
    ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.root = root;
    ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.IJ_in = IJ_in;
    ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.nedge = nedge;
    ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.err = err;
    ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf = verify_bfs_ddf;
    __hc_frame__ -> pc = 1;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_verify_bfs_tree__, and execute async: %p in _verify_bfs_tree_async1_\n", __hc_frame__, __hc_frame__->pc, __hc__verify_bfs_tree_async1__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__verify_bfs_tree_async1__frame__ -> header)) 
      _verify_bfs_tree_async1_(ws,((hc_frameHeader *)__hc__verify_bfs_tree_async1__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_verify_bfs_tree__\n", printEntry(__hc_frame__, buffer));
      bfs_tree = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.bfs_tree;
      IJ = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.IJ;
      seen_edge = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.seen_edge;
      level = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.level;
      nv = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.nv;
      p = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.p;
      compute_levels_ddf = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope1.compute_levels_ddf;
      bfs_tree_in = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.bfs_tree_in;
      max_bfsvtx = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.max_bfsvtx;
      root = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.root;
      IJ_in = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.IJ_in;
      nedge = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.nedge;
      err = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.err;
      verify_bfs_ddf = ((struct __hc_verify_bfs_tree_frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf;
    }
  }
  ws -> costatus =  HC_FUNC_COMPLETE;
  return ;
}

void _verify_bfs_tree_async7_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t *__restrict__ seen_edge;
  int nedge_traversed;
  volatile int *err;
  DDF_t *verify_bfs_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  seen_edge = ((struct __hc__verify_bfs_tree_async7__frame_t__ *)__hc_frame__) -> scope0.seen_edge;
  nedge_traversed = ((struct __hc__verify_bfs_tree_async7__frame_t__ *)__hc_frame__) -> scope0.nedge_traversed;
  err = ((struct __hc__verify_bfs_tree_async7__frame_t__ *)__hc_frame__) -> scope0.err;
  verify_bfs_ddf = ((struct __hc__verify_bfs_tree_async7__frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf;
  xfree_large(seen_edge);
  if ( *err != 0) {
    __ddfPut(verify_bfs_ddf,((int *)err));
  }
  else {
     *err = nedge_traversed;
    __ddfPut(verify_bfs_ddf,((int *)err));
  }
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _verify_bfs_tree_async6_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t lb;
  int64_t ub;
  int64_t *__restrict__ bfs_tree;
  int64_t *__restrict__ seen_edge;
  int64_t root;
  volatile int *err;
  DDF_t *d;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  lb = ((struct __hc__verify_bfs_tree_async6__frame_t__ *)__hc_frame__) -> scope0.lb;
  ub = ((struct __hc__verify_bfs_tree_async6__frame_t__ *)__hc_frame__) -> scope0.ub;
  bfs_tree = ((struct __hc__verify_bfs_tree_async6__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
  seen_edge = ((struct __hc__verify_bfs_tree_async6__frame_t__ *)__hc_frame__) -> scope0.seen_edge;
  root = ((struct __hc__verify_bfs_tree_async6__frame_t__ *)__hc_frame__) -> scope0.root;
  err = ((struct __hc__verify_bfs_tree_async6__frame_t__ *)__hc_frame__) -> scope0.err;
  d = ((struct __hc__verify_bfs_tree_async6__frame_t__ *)__hc_frame__) -> scope0.d;
  int64_t k;
{
    for (k = lb; k < ub; k++) {
      int terr =  *err;
      if (terr != 0) 
        break; 
      if (k != root) {
        if ((bfs_tree[k] >= 0) && !(seen_edge[k] != 0LL)) 
          terr = -15;
        if (bfs_tree[k] == k) 
          terr = -16;
        if (terr != 0) {
           *err = terr;
          hc_mfence();
        }
      }
    }
  }
  __ddfPut(d,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _verify_bfs_tree_async5_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t *__restrict__ bfs_tree;
  int64_t root;
  int64_t *__restrict__ seen_edge;
  volatile int *nedgeArray;
  int64_t nv;
  int p;
  volatile int *err;
  DDF_t *verify_bfs_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_async2;
  }
  bfs_tree = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
  root = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.root;
  seen_edge = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.seen_edge;
  nedgeArray = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.nedgeArray;
  nv = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.nv;
  p = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.p;
  err = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.err;
  verify_bfs_ddf = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf;
  if ( *err != 0) {
    __ddfPut(verify_bfs_ddf,((int *)err));
/* Write back value for INOUT/OUT arguments */
    ws -> costatus =  HC_ASYNC_COMPLETE ;
    return ;
  }
  int nedge_traversed = 0;
  int k;
  for (k = 0; k < p; k++) 
    nedge_traversed += nedgeArray[k];
  hc_mm_free(((hc_workerState *)(pthread_getspecific(wskey))),((void *)nedgeArray));
  p_DDF_list_t listOfErrorCheck = __ddfListCreate();
  int64_t i;
  int64_t c = (nv / p);
  for (i = 0; i < (p - 1); i++) {
    int64_t lb = (i * c);
    int64_t ub = (lb + c);
    DDF_t *d = __ddfCreate();
    __enlist(d,listOfErrorCheck);
    struct __hc__verify_bfs_tree_async6__frame_t__ *__hc__verify_bfs_tree_async6__frame__ = (struct __hc__verify_bfs_tree_async6__frame_t__ *)(hc_init_frame(sizeof(struct __hc__verify_bfs_tree_async6__frame_t__ ),_verify_bfs_tree_async6_));
    __hc__verify_bfs_tree_async6__frame__ -> scope0.lb = ((int64_t )lb);
    __hc__verify_bfs_tree_async6__frame__ -> scope0.ub = ((int64_t )ub);
    __hc__verify_bfs_tree_async6__frame__ -> scope0.bfs_tree = ((int64_t *__restrict__ )bfs_tree);
    __hc__verify_bfs_tree_async6__frame__ -> scope0.seen_edge = ((int64_t *__restrict__ )seen_edge);
    __hc__verify_bfs_tree_async6__frame__ -> scope0.root = ((int64_t )root);
    __hc__verify_bfs_tree_async6__frame__ -> scope0.err = ((volatile int *)err);
    __hc__verify_bfs_tree_async6__frame__ -> scope0.d = ((DDF_t *)d);
    ((hc_frameHeader *)__hc__verify_bfs_tree_async6__frame__) -> sched_policy = hc_sched_policy(ws);
    increment_counter();
    if (((hc_frameHeader *)__hc__verify_bfs_tree_async6__frame__) -> sched_policy ==  HELP_FIRST) {
      deque_push_default(__hc__verify_bfs_tree_async6__frame__);
    }
    else {
      ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope2.lb = lb;
      ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope2.ub = ub;
      ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope2.d = d;
      ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.nedge_traversed = nedge_traversed;
      ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.k = k;
      ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.listOfErrorCheck = listOfErrorCheck;
      ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.i = i;
      ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.c = c;
      ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.bfs_tree = bfs_tree;
      ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.root = root;
      ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.seen_edge = seen_edge;
      ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.nedgeArray = nedgeArray;
      ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.nv = nv;
      ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.p = p;
      ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.err = err;
      ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf = verify_bfs_ddf;
      __hc_frame__ -> pc = 1;
      deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _verify_bfs_tree_async5_, and execute async: %p in _verify_bfs_tree_async6_\n", __hc_frame__, __hc_frame__->pc, __hc__verify_bfs_tree_async6__frame__);
      _verify_bfs_tree_async6_(ws,((hc_frameHeader *)__hc__verify_bfs_tree_async6__frame__),0);
      if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
        return ;
      }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
      if (0) {
        __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _verify_bfs_tree_async5_\n", printEntry(__hc_frame__, buffer));
        lb = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope2.lb;
        ub = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope2.ub;
        d = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope2.d;
        nedge_traversed = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.nedge_traversed;
        k = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.k;
        listOfErrorCheck = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.listOfErrorCheck;
        i = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.i;
        c = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.c;
        bfs_tree = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
        root = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.root;
        seen_edge = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.seen_edge;
        nedgeArray = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.nedgeArray;
        nv = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.nv;
        p = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.p;
        err = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.err;
        verify_bfs_ddf = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf;
      }
    }
  }
  int64_t lb = (i * c);
{
    for (k = lb; k < nv; k++) {
      int terr =  *err;
      if (terr != 0) 
        break; 
      if (k != root) {
        if ((bfs_tree[k] >= 0) && !(seen_edge[k] != 0LL)) 
          terr = -15;
        if (bfs_tree[k] == k) 
          terr = -16;
        if (terr != 0) {
           *err = terr;
          hc_mfence();
        }
      }
    }
  }
  struct __hc__verify_bfs_tree_async7__frame_t__ *__hc__verify_bfs_tree_async7__frame__ = (struct __hc__verify_bfs_tree_async7__frame_t__ *)(hc_init_frame(sizeof(struct __hc__verify_bfs_tree_async7__frame_t__ ),_verify_bfs_tree_async7_));
  p_await_list_t __hc__verify_bfs_tree_async7_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__verify_bfs_tree_async7_awaitList_size = 0 + listOfErrorCheck -> size + 0;
  __hc__verify_bfs_tree_async7_awaitList -> array = hc_malloc((1 + __hc__verify_bfs_tree_async7_awaitList_size) * sizeof(p_DDF_t ));
  __hc__verify_bfs_tree_async7_awaitList -> array[__hc__verify_bfs_tree_async7_awaitList_size] = NULL;
  unsigned int __hc__verify_bfs_tree_async7_awaitList_curr_index = 0;
  __copyInto(__hc__verify_bfs_tree_async7_awaitList,__hc__verify_bfs_tree_async7_awaitList_curr_index,listOfErrorCheck);
  __hc__verify_bfs_tree_async7_awaitList_curr_index += listOfErrorCheck -> size;
  __hc__verify_bfs_tree_async7_awaitList -> waitingFrontier = &__hc__verify_bfs_tree_async7_awaitList -> array[0];
  __hc__verify_bfs_tree_async7__frame__ -> header.awaitList = __hc__verify_bfs_tree_async7_awaitList;
  __hc__verify_bfs_tree_async7__frame__ -> scope0.seen_edge = ((int64_t *__restrict__ )seen_edge);
  __hc__verify_bfs_tree_async7__frame__ -> scope0.nedge_traversed = ((int )nedge_traversed);
  __hc__verify_bfs_tree_async7__frame__ -> scope0.err = ((volatile int *)err);
  __hc__verify_bfs_tree_async7__frame__ -> scope0.verify_bfs_ddf = ((DDF_t *)verify_bfs_ddf);
  ((hc_frameHeader *)__hc__verify_bfs_tree_async7__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__verify_bfs_tree_async7__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__verify_bfs_tree_async7__frame__ -> header)) 
      deque_push_default(__hc__verify_bfs_tree_async7__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.nedge_traversed = nedge_traversed;
    ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.k = k;
    ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.listOfErrorCheck = listOfErrorCheck;
    ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.i = i;
    ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.c = c;
    ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.lb = lb;
    ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.bfs_tree = bfs_tree;
    ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.root = root;
    ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.seen_edge = seen_edge;
    ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.nedgeArray = nedgeArray;
    ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.nv = nv;
    ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.p = p;
    ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.err = err;
    ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf = verify_bfs_ddf;
    __hc_frame__ -> pc = 2;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _verify_bfs_tree_async5_, and execute async: %p in _verify_bfs_tree_async7_\n", __hc_frame__, __hc_frame__->pc, __hc__verify_bfs_tree_async7__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__verify_bfs_tree_async7__frame__ -> header)) 
      _verify_bfs_tree_async7_(ws,((hc_frameHeader *)__hc__verify_bfs_tree_async7__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async2:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _verify_bfs_tree_async5_\n", printEntry(__hc_frame__, buffer));
      nedge_traversed = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.nedge_traversed;
      k = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.k;
      listOfErrorCheck = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.listOfErrorCheck;
      i = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.i;
      c = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.c;
      lb = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope1.lb;
      bfs_tree = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
      root = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.root;
      seen_edge = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.seen_edge;
      nedgeArray = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.nedgeArray;
      nv = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.nv;
      p = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.p;
      err = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.err;
      verify_bfs_ddf = ((struct __hc__verify_bfs_tree_async5__frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf;
    }
  }
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}
/* Write back value for INOUT/OUT arguments */

void _verify_bfs_tree_async4_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t i;
  int64_t lb;
  int64_t ub;
  int64_t *__restrict__ bfs_tree;
  int64_t max_bfsvtx;
  struct packed_edge *__restrict__ IJ;
  int64_t *__restrict__ seen_edge;
  int64_t *__restrict__ level;
  volatile int *nedgeArray;
  volatile int *err;
  DDF_t *d;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  i = ((struct __hc__verify_bfs_tree_async4__frame_t__ *)__hc_frame__) -> scope0.i;
  lb = ((struct __hc__verify_bfs_tree_async4__frame_t__ *)__hc_frame__) -> scope0.lb;
  ub = ((struct __hc__verify_bfs_tree_async4__frame_t__ *)__hc_frame__) -> scope0.ub;
  bfs_tree = ((struct __hc__verify_bfs_tree_async4__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
  max_bfsvtx = ((struct __hc__verify_bfs_tree_async4__frame_t__ *)__hc_frame__) -> scope0.max_bfsvtx;
  IJ = ((struct __hc__verify_bfs_tree_async4__frame_t__ *)__hc_frame__) -> scope0.IJ;
  seen_edge = ((struct __hc__verify_bfs_tree_async4__frame_t__ *)__hc_frame__) -> scope0.seen_edge;
  level = ((struct __hc__verify_bfs_tree_async4__frame_t__ *)__hc_frame__) -> scope0.level;
  nedgeArray = ((struct __hc__verify_bfs_tree_async4__frame_t__ *)__hc_frame__) -> scope0.nedgeArray;
  err = ((struct __hc__verify_bfs_tree_async4__frame_t__ *)__hc_frame__) -> scope0.err;
  d = ((struct __hc__verify_bfs_tree_async4__frame_t__ *)__hc_frame__) -> scope0.d;
  verify_kernel(i,lb,ub,bfs_tree,max_bfsvtx,((int64_t *)IJ),seen_edge,level,nedgeArray,err);
  __ddfPut(d,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _verify_bfs_tree_async3_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t *__restrict__ bfs_tree;
  int64_t max_bfsvtx;
  int64_t root;
  struct packed_edge *__restrict__ IJ;
  int64_t nedge;
  int64_t *__restrict__ seen_edge;
  int64_t *__restrict__ level;
  int64_t nv;
  int p;
  volatile int *err;
  DDF_t *verify_bfs_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_async2;
  }
  bfs_tree = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
  max_bfsvtx = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.max_bfsvtx;
  root = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.root;
  IJ = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.IJ;
  nedge = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.nedge;
  seen_edge = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.seen_edge;
  level = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.level;
  nv = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.nv;
  p = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.p;
  err = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.err;
  verify_bfs_ddf = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf;
  p_DDF_list_t listOfVerifyKernel = __ddfListCreate();
  volatile int *nedgeArray = ((int *)(hc_mm_malloc(((hc_workerState *)(pthread_getspecific(wskey))),(p * sizeof(int )))));
  int64_t i;
  int64_t c = (nedge / p);
  for (i = 0; i < (p - 1); i++) {
    int64_t lb = (i * c);
    int64_t ub = (lb + c);
    DDF_t *d = __ddfCreate();
    __enlist(d,listOfVerifyKernel);
    struct __hc__verify_bfs_tree_async4__frame_t__ *__hc__verify_bfs_tree_async4__frame__ = (struct __hc__verify_bfs_tree_async4__frame_t__ *)(hc_init_frame(sizeof(struct __hc__verify_bfs_tree_async4__frame_t__ ),_verify_bfs_tree_async4_));
    __hc__verify_bfs_tree_async4__frame__ -> scope0.i = ((int64_t )i);
    __hc__verify_bfs_tree_async4__frame__ -> scope0.lb = ((int64_t )lb);
    __hc__verify_bfs_tree_async4__frame__ -> scope0.ub = ((int64_t )ub);
    __hc__verify_bfs_tree_async4__frame__ -> scope0.bfs_tree = ((int64_t *__restrict__ )bfs_tree);
    __hc__verify_bfs_tree_async4__frame__ -> scope0.max_bfsvtx = ((int64_t )max_bfsvtx);
    __hc__verify_bfs_tree_async4__frame__ -> scope0.IJ = ((struct packed_edge *__restrict__ )IJ);
    __hc__verify_bfs_tree_async4__frame__ -> scope0.seen_edge = ((int64_t *__restrict__ )seen_edge);
    __hc__verify_bfs_tree_async4__frame__ -> scope0.level = ((int64_t *__restrict__ )level);
    __hc__verify_bfs_tree_async4__frame__ -> scope0.nedgeArray = ((volatile int *)nedgeArray);
    __hc__verify_bfs_tree_async4__frame__ -> scope0.err = ((volatile int *)err);
    __hc__verify_bfs_tree_async4__frame__ -> scope0.d = ((DDF_t *)d);
    ((hc_frameHeader *)__hc__verify_bfs_tree_async4__frame__) -> sched_policy = hc_sched_policy(ws);
    increment_counter();
    if (((hc_frameHeader *)__hc__verify_bfs_tree_async4__frame__) -> sched_policy ==  HELP_FIRST) {
      deque_push_default(__hc__verify_bfs_tree_async4__frame__);
    }
    else {
      ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope2.lb = lb;
      ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope2.ub = ub;
      ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope2.d = d;
      ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope1.listOfVerifyKernel = listOfVerifyKernel;
      ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope1.nedgeArray = nedgeArray;
      ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope1.i = i;
      ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope1.c = c;
      ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.bfs_tree = bfs_tree;
      ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.max_bfsvtx = max_bfsvtx;
      ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.root = root;
      ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.IJ = IJ;
      ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.nedge = nedge;
      ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.seen_edge = seen_edge;
      ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.level = level;
      ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.nv = nv;
      ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.p = p;
      ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.err = err;
      ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf = verify_bfs_ddf;
      __hc_frame__ -> pc = 1;
      deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _verify_bfs_tree_async3_, and execute async: %p in _verify_bfs_tree_async4_\n", __hc_frame__, __hc_frame__->pc, __hc__verify_bfs_tree_async4__frame__);
      _verify_bfs_tree_async4_(ws,((hc_frameHeader *)__hc__verify_bfs_tree_async4__frame__),0);
      if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
        return ;
      }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
      if (0) {
        __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _verify_bfs_tree_async3_\n", printEntry(__hc_frame__, buffer));
        lb = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope2.lb;
        ub = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope2.ub;
        d = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope2.d;
        listOfVerifyKernel = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope1.listOfVerifyKernel;
        nedgeArray = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope1.nedgeArray;
        i = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope1.i;
        c = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope1.c;
        bfs_tree = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
        max_bfsvtx = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.max_bfsvtx;
        root = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.root;
        IJ = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.IJ;
        nedge = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.nedge;
        seen_edge = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.seen_edge;
        level = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.level;
        nv = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.nv;
        p = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.p;
        err = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.err;
        verify_bfs_ddf = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf;
      }
    }
  }
  int64_t lb = (i * c);
  verify_kernel(i,lb,nedge,bfs_tree,max_bfsvtx,((int64_t *)IJ),seen_edge,level,nedgeArray,err);
/* Write back value for INOUT/OUT arguments */
  struct __hc__verify_bfs_tree_async5__frame_t__ *__hc__verify_bfs_tree_async5__frame__ = (struct __hc__verify_bfs_tree_async5__frame_t__ *)(hc_init_frame(sizeof(struct __hc__verify_bfs_tree_async5__frame_t__ ),_verify_bfs_tree_async5_));
  p_await_list_t __hc__verify_bfs_tree_async5_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__verify_bfs_tree_async5_awaitList_size = 0 + listOfVerifyKernel -> size + 0;
  __hc__verify_bfs_tree_async5_awaitList -> array = hc_malloc((1 + __hc__verify_bfs_tree_async5_awaitList_size) * sizeof(p_DDF_t ));
  __hc__verify_bfs_tree_async5_awaitList -> array[__hc__verify_bfs_tree_async5_awaitList_size] = NULL;
  unsigned int __hc__verify_bfs_tree_async5_awaitList_curr_index = 0;
  __copyInto(__hc__verify_bfs_tree_async5_awaitList,__hc__verify_bfs_tree_async5_awaitList_curr_index,listOfVerifyKernel);
  __hc__verify_bfs_tree_async5_awaitList_curr_index += listOfVerifyKernel -> size;
  __hc__verify_bfs_tree_async5_awaitList -> waitingFrontier = &__hc__verify_bfs_tree_async5_awaitList -> array[0];
  __hc__verify_bfs_tree_async5__frame__ -> header.awaitList = __hc__verify_bfs_tree_async5_awaitList;
  __hc__verify_bfs_tree_async5__frame__ -> scope0.bfs_tree = ((int64_t *__restrict__ )bfs_tree);
  __hc__verify_bfs_tree_async5__frame__ -> scope0.root = ((int64_t )root);
  __hc__verify_bfs_tree_async5__frame__ -> scope0.seen_edge = ((int64_t *__restrict__ )seen_edge);
  __hc__verify_bfs_tree_async5__frame__ -> scope0.nedgeArray = ((volatile int *)nedgeArray);
  __hc__verify_bfs_tree_async5__frame__ -> scope0.nv = ((int64_t )nv);
  __hc__verify_bfs_tree_async5__frame__ -> scope0.p = ((int )p);
  __hc__verify_bfs_tree_async5__frame__ -> scope0.err = ((volatile int *)err);
  __hc__verify_bfs_tree_async5__frame__ -> scope0.verify_bfs_ddf = ((DDF_t *)verify_bfs_ddf);
  ((hc_frameHeader *)__hc__verify_bfs_tree_async5__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__verify_bfs_tree_async5__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__verify_bfs_tree_async5__frame__ -> header)) 
      deque_push_default(__hc__verify_bfs_tree_async5__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope1.listOfVerifyKernel = listOfVerifyKernel;
    ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope1.nedgeArray = nedgeArray;
    ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope1.i = i;
    ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope1.c = c;
    ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope1.lb = lb;
    ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.bfs_tree = bfs_tree;
    ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.max_bfsvtx = max_bfsvtx;
    ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.root = root;
    ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.IJ = IJ;
    ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.nedge = nedge;
    ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.seen_edge = seen_edge;
    ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.level = level;
    ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.nv = nv;
    ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.p = p;
    ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.err = err;
    ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf = verify_bfs_ddf;
    __hc_frame__ -> pc = 2;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _verify_bfs_tree_async3_, and execute async: %p in _verify_bfs_tree_async5_\n", __hc_frame__, __hc_frame__->pc, __hc__verify_bfs_tree_async5__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__verify_bfs_tree_async5__frame__ -> header)) 
      _verify_bfs_tree_async5_(ws,((hc_frameHeader *)__hc__verify_bfs_tree_async5__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async2:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _verify_bfs_tree_async3_\n", printEntry(__hc_frame__, buffer));
      listOfVerifyKernel = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope1.listOfVerifyKernel;
      nedgeArray = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope1.nedgeArray;
      i = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope1.i;
      c = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope1.c;
      lb = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope1.lb;
      bfs_tree = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
      max_bfsvtx = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.max_bfsvtx;
      root = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.root;
      IJ = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.IJ;
      nedge = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.nedge;
      seen_edge = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.seen_edge;
      level = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.level;
      nv = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.nv;
      p = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.p;
      err = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.err;
      verify_bfs_ddf = ((struct __hc__verify_bfs_tree_async3__frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf;
    }
  }
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _verify_bfs_tree_async2_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t lb;
  int64_t ub;
  int64_t *__restrict__ seen_edge;
  DDF_t *d;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  lb = ((struct __hc__verify_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.lb;
  ub = ((struct __hc__verify_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.ub;
  seen_edge = ((struct __hc__verify_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.seen_edge;
  d = ((struct __hc__verify_bfs_tree_async2__frame_t__ *)__hc_frame__) -> scope0.d;
  int64_t k;
  for (k = lb; k < ub; k++) {
    seen_edge[k] = 0;
  }
  __ddfPut(d,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

void _verify_bfs_tree_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t *__restrict__ bfs_tree;
  int64_t max_bfsvtx;
  int64_t root;
  struct packed_edge *__restrict__ IJ;
  int64_t nedge;
  int64_t *__restrict__ seen_edge;
  int64_t *__restrict__ level;
  int64_t nv;
  int p;
  volatile int *err;
  DDF_t *verify_bfs_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_async2;
  }
  bfs_tree = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
  max_bfsvtx = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.max_bfsvtx;
  root = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.root;
  IJ = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.IJ;
  nedge = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.nedge;
  seen_edge = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.seen_edge;
  level = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.level;
  nv = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.nv;
  p = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.p;
  err = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.err;
  verify_bfs_ddf = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf;
  if ( *err != 0) {
    __ddfPut(verify_bfs_ddf,((int *)err));
/* Write back value for INOUT/OUT arguments */
    ws -> costatus =  HC_ASYNC_COMPLETE ;
    return ;
  }
  p_DDF_list_t listOfSeenInit = __ddfListCreate();
  int64_t i;
  int64_t c = (nv / p);
  for (i = 0; i < (p - 1); i++) {
    int64_t lb = (i * c);
    int64_t ub = (lb + c);
    DDF_t *d = __ddfCreate();
    __enlist(d,listOfSeenInit);
    struct __hc__verify_bfs_tree_async2__frame_t__ *__hc__verify_bfs_tree_async2__frame__ = (struct __hc__verify_bfs_tree_async2__frame_t__ *)(hc_init_frame(sizeof(struct __hc__verify_bfs_tree_async2__frame_t__ ),_verify_bfs_tree_async2_));
    __hc__verify_bfs_tree_async2__frame__ -> scope0.lb = ((int64_t )lb);
    __hc__verify_bfs_tree_async2__frame__ -> scope0.ub = ((int64_t )ub);
    __hc__verify_bfs_tree_async2__frame__ -> scope0.seen_edge = ((int64_t *__restrict__ )seen_edge);
    __hc__verify_bfs_tree_async2__frame__ -> scope0.d = ((DDF_t *)d);
    ((hc_frameHeader *)__hc__verify_bfs_tree_async2__frame__) -> sched_policy = hc_sched_policy(ws);
    increment_counter();
    if (((hc_frameHeader *)__hc__verify_bfs_tree_async2__frame__) -> sched_policy ==  HELP_FIRST) {
      deque_push_default(__hc__verify_bfs_tree_async2__frame__);
    }
    else {
      ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope2.lb = lb;
      ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope2.ub = ub;
      ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope2.d = d;
      ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope1.listOfSeenInit = listOfSeenInit;
      ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope1.i = i;
      ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope1.c = c;
      ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.bfs_tree = bfs_tree;
      ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.max_bfsvtx = max_bfsvtx;
      ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.root = root;
      ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.IJ = IJ;
      ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.nedge = nedge;
      ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.seen_edge = seen_edge;
      ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.level = level;
      ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.nv = nv;
      ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.p = p;
      ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.err = err;
      ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf = verify_bfs_ddf;
      __hc_frame__ -> pc = 1;
      deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _verify_bfs_tree_async1_, and execute async: %p in _verify_bfs_tree_async2_\n", __hc_frame__, __hc_frame__->pc, __hc__verify_bfs_tree_async2__frame__);
      _verify_bfs_tree_async2_(ws,((hc_frameHeader *)__hc__verify_bfs_tree_async2__frame__),0);
      if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
        return ;
      }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
      if (0) {
        __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _verify_bfs_tree_async1_\n", printEntry(__hc_frame__, buffer));
        lb = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope2.lb;
        ub = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope2.ub;
        d = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope2.d;
        listOfSeenInit = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope1.listOfSeenInit;
        i = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope1.i;
        c = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope1.c;
        bfs_tree = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
        max_bfsvtx = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.max_bfsvtx;
        root = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.root;
        IJ = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.IJ;
        nedge = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.nedge;
        seen_edge = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.seen_edge;
        level = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.level;
        nv = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.nv;
        p = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.p;
        err = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.err;
        verify_bfs_ddf = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf;
      }
    }
  }
  int64_t k;
  int64_t lb = (i * c);
  for (k = lb; k < nv; k++) {
    seen_edge[k] = 0;
  }
/* Write back value for INOUT/OUT arguments */
  struct __hc__verify_bfs_tree_async3__frame_t__ *__hc__verify_bfs_tree_async3__frame__ = (struct __hc__verify_bfs_tree_async3__frame_t__ *)(hc_init_frame(sizeof(struct __hc__verify_bfs_tree_async3__frame_t__ ),_verify_bfs_tree_async3_));
  p_await_list_t __hc__verify_bfs_tree_async3_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__verify_bfs_tree_async3_awaitList_size = 0 + listOfSeenInit -> size + 0;
  __hc__verify_bfs_tree_async3_awaitList -> array = hc_malloc((1 + __hc__verify_bfs_tree_async3_awaitList_size) * sizeof(p_DDF_t ));
  __hc__verify_bfs_tree_async3_awaitList -> array[__hc__verify_bfs_tree_async3_awaitList_size] = NULL;
  unsigned int __hc__verify_bfs_tree_async3_awaitList_curr_index = 0;
  __copyInto(__hc__verify_bfs_tree_async3_awaitList,__hc__verify_bfs_tree_async3_awaitList_curr_index,listOfSeenInit);
  __hc__verify_bfs_tree_async3_awaitList_curr_index += listOfSeenInit -> size;
  __hc__verify_bfs_tree_async3_awaitList -> waitingFrontier = &__hc__verify_bfs_tree_async3_awaitList -> array[0];
  __hc__verify_bfs_tree_async3__frame__ -> header.awaitList = __hc__verify_bfs_tree_async3_awaitList;
  __hc__verify_bfs_tree_async3__frame__ -> scope0.bfs_tree = ((int64_t *__restrict__ )bfs_tree);
  __hc__verify_bfs_tree_async3__frame__ -> scope0.max_bfsvtx = ((int64_t )max_bfsvtx);
  __hc__verify_bfs_tree_async3__frame__ -> scope0.root = ((int64_t )root);
  __hc__verify_bfs_tree_async3__frame__ -> scope0.IJ = ((struct packed_edge *__restrict__ )IJ);
  __hc__verify_bfs_tree_async3__frame__ -> scope0.nedge = ((int64_t )nedge);
  __hc__verify_bfs_tree_async3__frame__ -> scope0.seen_edge = ((int64_t *__restrict__ )seen_edge);
  __hc__verify_bfs_tree_async3__frame__ -> scope0.level = ((int64_t *__restrict__ )level);
  __hc__verify_bfs_tree_async3__frame__ -> scope0.nv = ((int64_t )nv);
  __hc__verify_bfs_tree_async3__frame__ -> scope0.p = ((int )p);
  __hc__verify_bfs_tree_async3__frame__ -> scope0.err = ((volatile int *)err);
  __hc__verify_bfs_tree_async3__frame__ -> scope0.verify_bfs_ddf = ((DDF_t *)verify_bfs_ddf);
  ((hc_frameHeader *)__hc__verify_bfs_tree_async3__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__verify_bfs_tree_async3__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__verify_bfs_tree_async3__frame__ -> header)) 
      deque_push_default(__hc__verify_bfs_tree_async3__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope1.listOfSeenInit = listOfSeenInit;
    ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope1.i = i;
    ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope1.c = c;
    ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope1.k = k;
    ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope1.lb = lb;
    ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.bfs_tree = bfs_tree;
    ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.max_bfsvtx = max_bfsvtx;
    ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.root = root;
    ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.IJ = IJ;
    ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.nedge = nedge;
    ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.seen_edge = seen_edge;
    ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.level = level;
    ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.nv = nv;
    ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.p = p;
    ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.err = err;
    ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf = verify_bfs_ddf;
    __hc_frame__ -> pc = 2;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of _verify_bfs_tree_async1_, and execute async: %p in _verify_bfs_tree_async3_\n", __hc_frame__, __hc_frame__->pc, __hc__verify_bfs_tree_async3__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__verify_bfs_tree_async3__frame__ -> header)) 
      _verify_bfs_tree_async3_(ws,((hc_frameHeader *)__hc__verify_bfs_tree_async3__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async2:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of _verify_bfs_tree_async1_\n", printEntry(__hc_frame__, buffer));
      listOfSeenInit = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope1.listOfSeenInit;
      i = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope1.i;
      c = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope1.c;
      k = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope1.k;
      lb = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope1.lb;
      bfs_tree = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.bfs_tree;
      max_bfsvtx = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.max_bfsvtx;
      root = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.root;
      IJ = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.IJ;
      nedge = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.nedge;
      seen_edge = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.seen_edge;
      level = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.level;
      nv = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.nv;
      p = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.p;
      err = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.err;
      verify_bfs_ddf = ((struct __hc__verify_bfs_tree_async1__frame_t__ *)__hc_frame__) -> scope0.verify_bfs_ddf;
    }
  }
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}

static void destroy_graph()
{
  free_graph();
}
/* Generate a range of edges (from start_edge to end_edge of the total graph),
 * writing into elements [0, end_edge - start_edge) of the edges array.  This
 * code is parallel on OpenMP and XMT; it must be used with
 * separately-implemented SPMD parallelism for MPI. */
typedef struct __hc_generate_kronecker_range_frame_t__ {
hc_frameHeader header;
struct __hc_generate_kronecker_range_scope0_frame_t__ {
unsigned int *seed;
int logN;
int64_t start_edge;
int64_t end_edge;
packed_edge *edges;
p_DDF_list_t listOfGraphGen;}scope0;
struct __hc_generate_kronecker_range_scope1_frame_t__ {
mrg_state state;
int64_t nverts;
uint64_t val0;
uint64_t val1;
int64_t p;
int64_t i;
int64_t n;
int64_t c;
int64_t ei;
int64_t lb;}scope1;
struct __hc_generate_kronecker_range_scope2_frame_t__ {
mrg_state new_state;}scope2;
struct __hc_generate_kronecker_range_scope3_frame_t__ {
int64_t lb;
int64_t ub;
DDF_t *d;}scope3;
struct __hc_generate_kronecker_range_scope4_frame_t__ {
mrg_state new_state;}scope4;}__hc_generate_kronecker_range_frame_t__;
typedef struct __hc__generate_kronecker_range_async1__frame_t__ {
hc_frameHeader header;
struct __hc__generate_kronecker_range_async1__scope0_frame_t__ {
int64_t lb;
int64_t ub;
mrg_state state;
int64_t nverts;
int logN;
packed_edge *edges;
int64_t start_edge;
uint64_t val0;
uint64_t val1;
DDF_t *d;}scope0;}__hc__generate_kronecker_range_async1__frame_t__;
void __hc_generate_kronecker_range__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _generate_kronecker_range_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);

static void generate_kronecker_range(
/* All values in [0, 2^31 - 1), not all zero */
unsigned int *seed,
/* In base 2 */
int logN,int64_t start_edge,int64_t end_edge,packed_edge *edges,p_DDF_list_t listOfGraphGen)
{
  hc_workerState *ws = current_ws();
  ws -> costatus =  HC_FUNC_COMPLETE;
  struct __hc_generate_kronecker_range_frame_t__ *__hc_frame__ = (struct __hc_generate_kronecker_range_frame_t__ *)(hc_init_frame_ws(ws,sizeof(struct __hc_generate_kronecker_range_frame_t__ ),__hc_generate_kronecker_range__));
  __hc_frame__ -> scope0.seed = seed;
  __hc_frame__ -> scope0.logN = logN;
  __hc_frame__ -> scope0.start_edge = start_edge;
  __hc_frame__ -> scope0.end_edge = end_edge;
  __hc_frame__ -> scope0.edges = edges;
  __hc_frame__ -> scope0.listOfGraphGen = listOfGraphGen;
			LOG_INFO(ws, "execute frame: %p, parent: %p of function %s\n", __hc_frame__,((hc_frameHeader*) __hc_frame__)->parent, "__hc_generate_kronecker_range__" );
  __hc_generate_kronecker_range__(ws,((hc_frameHeader *)__hc_frame__),0);
  if (ws -> costatus ==  HC_FUNC_COMPLETE) 
    hc_free_unwind_frame(ws);
  return ;
}

void __hc_generate_kronecker_range__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  
/* All values in [0, 2^31 - 1), not all zero */
unsigned int *seed;
  
/* In base 2 */
int logN;
  int64_t start_edge;
  int64_t end_edge;
  packed_edge *edges;
  p_DDF_list_t listOfGraphGen;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
  }
  seed = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope0.seed;
  logN = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope0.logN;
  start_edge = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope0.start_edge;
  end_edge = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope0.end_edge;
  edges = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope0.edges;
  listOfGraphGen = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope0.listOfGraphGen;
  mrg_state state;
  int64_t nverts = (((int64_t )1) << logN);
  mrg_seed(&state,seed);
/* Values for scrambling */
  uint64_t val0;
  uint64_t val1;
{
    mrg_state new_state = state;
    mrg_skip(&new_state,50,7,0);
    val0 = (mrg_get_uint_orig(&new_state));
    val0 *= 0xFFFFFFFFULL;
    val0 += (mrg_get_uint_orig(&new_state));
    val1 = (mrg_get_uint_orig(&new_state));
    val1 *= 0xFFFFFFFFULL;
    val1 += (mrg_get_uint_orig(&new_state));
  }
  int64_t p = ( *( *((hc_workerState *)(pthread_getspecific(wskey)))).context).nproc;
  int64_t i;
  int64_t n = (end_edge - start_edge);
  int64_t c = (n / p);
  for (i = 0; i < (p - 1); i++) {
    int64_t lb = (start_edge + (i * c));
    int64_t ub = (lb + c);
    DDF_t *d = __ddfCreate();
    __enlist(d,listOfGraphGen);
    struct __hc__generate_kronecker_range_async1__frame_t__ *__hc__generate_kronecker_range_async1__frame__ = (struct __hc__generate_kronecker_range_async1__frame_t__ *)(hc_init_frame(sizeof(struct __hc__generate_kronecker_range_async1__frame_t__ ),_generate_kronecker_range_async1_));
    __hc__generate_kronecker_range_async1__frame__ -> scope0.lb = ((int64_t )lb);
    __hc__generate_kronecker_range_async1__frame__ -> scope0.ub = ((int64_t )ub);
    __hc__generate_kronecker_range_async1__frame__ -> scope0.state = ((mrg_state )state);
    __hc__generate_kronecker_range_async1__frame__ -> scope0.nverts = ((int64_t )nverts);
    __hc__generate_kronecker_range_async1__frame__ -> scope0.logN = ((int )logN);
    __hc__generate_kronecker_range_async1__frame__ -> scope0.edges = ((packed_edge *)edges);
    __hc__generate_kronecker_range_async1__frame__ -> scope0.start_edge = ((int64_t )start_edge);
    __hc__generate_kronecker_range_async1__frame__ -> scope0.val0 = ((uint64_t )val0);
    __hc__generate_kronecker_range_async1__frame__ -> scope0.val1 = ((uint64_t )val1);
    __hc__generate_kronecker_range_async1__frame__ -> scope0.d = ((DDF_t *)d);
    ((hc_frameHeader *)__hc__generate_kronecker_range_async1__frame__) -> sched_policy = hc_sched_policy(ws);
    increment_counter();
    if (((hc_frameHeader *)__hc__generate_kronecker_range_async1__frame__) -> sched_policy ==  HELP_FIRST) {
      deque_push_default(__hc__generate_kronecker_range_async1__frame__);
    }
    else {
      ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope3.lb = lb;
      ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope3.ub = ub;
      ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope3.d = d;
      ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope1.state = state;
      ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope1.nverts = nverts;
      ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope1.val0 = val0;
      ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope1.val1 = val1;
      ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope1.p = p;
      ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope1.i = i;
      ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope1.n = n;
      ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope1.c = c;
      ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope0.seed = seed;
      ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope0.logN = logN;
      ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope0.start_edge = start_edge;
      ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope0.end_edge = end_edge;
      ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope0.edges = edges;
      ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope0.listOfGraphGen = listOfGraphGen;
      __hc_frame__ -> pc = 1;
      deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_generate_kronecker_range__, and execute async: %p in _generate_kronecker_range_async1_\n", __hc_frame__, __hc_frame__->pc, __hc__generate_kronecker_range_async1__frame__);
      _generate_kronecker_range_async1_(ws,((hc_frameHeader *)__hc__generate_kronecker_range_async1__frame__),0);
      if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
        return ;
      }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
      if (0) {
        __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_generate_kronecker_range__\n", printEntry(__hc_frame__, buffer));
        lb = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope3.lb;
        ub = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope3.ub;
        d = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope3.d;
        state = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope1.state;
        nverts = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope1.nverts;
        val0 = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope1.val0;
        val1 = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope1.val1;
        p = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope1.p;
        i = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope1.i;
        n = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope1.n;
        c = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope1.c;
        seed = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope0.seed;
        logN = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope0.logN;
        start_edge = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope0.start_edge;
        end_edge = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope0.end_edge;
        edges = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope0.edges;
        listOfGraphGen = ((struct __hc_generate_kronecker_range_frame_t__ *)__hc_frame__) -> scope0.listOfGraphGen;
      }
    }
  }
  int64_t ei;
  int64_t lb = (start_edge + (i * c));
  for (ei = lb; ei < end_edge; ei++) {
    mrg_state new_state = state;
    mrg_skip(&new_state,0,ei,0);
    make_one_edge(nverts,0,logN,&new_state,(edges + (ei - start_edge)),val0,val1);
  }
  ws -> costatus =  HC_FUNC_COMPLETE;
  return ;
}

void _generate_kronecker_range_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int64_t lb;
  int64_t ub;
  mrg_state state;
  int64_t nverts;
  int logN;
  packed_edge *edges;
  int64_t start_edge;
  uint64_t val0;
  uint64_t val1;
  DDF_t *d;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  lb = ((struct __hc__generate_kronecker_range_async1__frame_t__ *)__hc_frame__) -> scope0.lb;
  ub = ((struct __hc__generate_kronecker_range_async1__frame_t__ *)__hc_frame__) -> scope0.ub;
  state = ((struct __hc__generate_kronecker_range_async1__frame_t__ *)__hc_frame__) -> scope0.state;
  nverts = ((struct __hc__generate_kronecker_range_async1__frame_t__ *)__hc_frame__) -> scope0.nverts;
  logN = ((struct __hc__generate_kronecker_range_async1__frame_t__ *)__hc_frame__) -> scope0.logN;
  edges = ((struct __hc__generate_kronecker_range_async1__frame_t__ *)__hc_frame__) -> scope0.edges;
  start_edge = ((struct __hc__generate_kronecker_range_async1__frame_t__ *)__hc_frame__) -> scope0.start_edge;
  val0 = ((struct __hc__generate_kronecker_range_async1__frame_t__ *)__hc_frame__) -> scope0.val0;
  val1 = ((struct __hc__generate_kronecker_range_async1__frame_t__ *)__hc_frame__) -> scope0.val1;
  d = ((struct __hc__generate_kronecker_range_async1__frame_t__ *)__hc_frame__) -> scope0.d;
  int64_t ei;
  for (ei = lb; ei < ub; ei++) {
    mrg_state new_state = state;
    mrg_skip(&new_state,0,ei,0);
    make_one_edge(nverts,0,logN,&new_state,(edges + (ei - start_edge)),val0,val1);
  }
  __ddfPut(d,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}
/* Simplified interface to build graphs with scrambled vertices. */
typedef struct __hc_make_graph_frame_t__ {
hc_frameHeader header;
struct __hc_make_graph_scope0_frame_t__ {
int log_numverts;
int64_t M;
uint64_t userseed1;
uint64_t userseed2;
int64_t *nedges_ptr_in;
packed_edge **result_ptr_in;
DDF_t *make_graph_ddf;}scope0;
struct __hc_make_graph_scope1_frame_t__ {
p_DDF_list_t listOfGraphGen;
int64_t *__restrict__ nedges_ptr;
packed_edge *__restrict__ *__restrict__ result_ptr;
uint_fast32_t *seed;
packed_edge *edges;}scope1;}__hc_make_graph_frame_t__;
typedef struct __hc__make_graph_async1__frame_t__ {
hc_frameHeader header;
struct __hc__make_graph_async1__scope0_frame_t__ {
uint_fast32_t *seed;
DDF_t *make_graph_ddf;}scope0;}__hc__make_graph_async1__frame_t__;
void __hc_make_graph__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);
void _make_graph_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__);

static void make_graph(int log_numverts,int64_t M,uint64_t userseed1,uint64_t userseed2,int64_t *nedges_ptr_in,packed_edge **result_ptr_in,DDF_t *make_graph_ddf)
{
  hc_workerState *ws = current_ws();
  ws -> costatus =  HC_FUNC_COMPLETE;
  struct __hc_make_graph_frame_t__ *__hc_frame__ = (struct __hc_make_graph_frame_t__ *)(hc_init_frame_ws(ws,sizeof(struct __hc_make_graph_frame_t__ ),__hc_make_graph__));
  __hc_frame__ -> scope0.log_numverts = log_numverts;
  __hc_frame__ -> scope0.M = M;
  __hc_frame__ -> scope0.userseed1 = userseed1;
  __hc_frame__ -> scope0.userseed2 = userseed2;
  __hc_frame__ -> scope0.nedges_ptr_in = nedges_ptr_in;
  __hc_frame__ -> scope0.result_ptr_in = result_ptr_in;
  __hc_frame__ -> scope0.make_graph_ddf = make_graph_ddf;
			LOG_INFO(ws, "execute frame: %p, parent: %p of function %s\n", __hc_frame__,((hc_frameHeader*) __hc_frame__)->parent, "__hc_make_graph__" );
  __hc_make_graph__(ws,((hc_frameHeader *)__hc_frame__),0);
  if (ws -> costatus ==  HC_FUNC_COMPLETE) 
    hc_free_unwind_frame(ws);
  return ;
}

void __hc_make_graph__(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  int log_numverts;
  int64_t M;
  uint64_t userseed1;
  uint64_t userseed2;
  int64_t *nedges_ptr_in;
  packed_edge **result_ptr_in;
  DDF_t *make_graph_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
    case 1:
    goto __post_async1;
    case 2:
    goto __post_func2;
  }
  log_numverts = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.log_numverts;
  M = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.M;
  userseed1 = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.userseed1;
  userseed2 = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.userseed2;
  nedges_ptr_in = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.nedges_ptr_in;
  result_ptr_in = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.result_ptr_in;
  make_graph_ddf = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.make_graph_ddf;
  p_DDF_list_t listOfGraphGen = __ddfListCreate();
/* Add restrict to input pointers. */
  int64_t *__restrict__ nedges_ptr = nedges_ptr_in;
  packed_edge *__restrict__ *__restrict__ result_ptr = result_ptr_in;
/* Spread the two 64-bit numbers into five nonzero values in the correct
   * range. */
  uint_fast32_t *seed = (uint_fast32_t *)(hc_mm_malloc(((hc_workerState *)(pthread_getspecific(wskey))),(sizeof(uint_fast32_t ) * 5)));
  make_mrg_seed(userseed1,userseed2,seed);
   *nedges_ptr = M;
  packed_edge *edges = (packed_edge *)(xmalloc((M * sizeof(packed_edge ))));
   *result_ptr = edges;
/* In OpenMP and XMT versions, the inner loop in generate_kronecker_range is
   * parallel.  */
  ws -> costatus =  HC_FUNC_COMPLETE;
  __hc_frame__ -> pc = 2;
  ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope1.listOfGraphGen = listOfGraphGen;
  ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope1.nedges_ptr = nedges_ptr;
  ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope1.result_ptr = result_ptr;
  ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope1.seed = seed;
  ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope1.edges = edges;
  ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.log_numverts = log_numverts;
  ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.M = M;
  ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.userseed1 = userseed1;
  ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.userseed2 = userseed2;
  ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.nedges_ptr_in = nedges_ptr_in;
  ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.result_ptr_in = result_ptr_in;
  ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.make_graph_ddf = make_graph_ddf;
  generate_kronecker_range(seed,log_numverts,0,M,edges,listOfGraphGen);
  if (ws -> costatus !=  HC_FUNC_COMPLETE) 
    return ;
  if (0) {
    __post_func2:
    hc_free_unwind_frame(ws);
    __hc_frame__ = ws -> frame;
    listOfGraphGen = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope1.listOfGraphGen;
    nedges_ptr = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope1.nedges_ptr;
    result_ptr = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope1.result_ptr;
    seed = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope1.seed;
    edges = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope1.edges;
    log_numverts = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.log_numverts;
    M = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.M;
    userseed1 = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.userseed1;
    userseed2 = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.userseed2;
    nedges_ptr_in = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.nedges_ptr_in;
    result_ptr_in = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.result_ptr_in;
    make_graph_ddf = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.make_graph_ddf;
  }
  struct __hc__make_graph_async1__frame_t__ *__hc__make_graph_async1__frame__ = (struct __hc__make_graph_async1__frame_t__ *)(hc_init_frame(sizeof(struct __hc__make_graph_async1__frame_t__ ),_make_graph_async1_));
  p_await_list_t __hc__make_graph_async1_awaitList = hc_malloc(sizeof(await_list_t ));
  unsigned int __hc__make_graph_async1_awaitList_size = 0 + listOfGraphGen -> size + 0;
  __hc__make_graph_async1_awaitList -> array = hc_malloc((1 + __hc__make_graph_async1_awaitList_size) * sizeof(p_DDF_t ));
  __hc__make_graph_async1_awaitList -> array[__hc__make_graph_async1_awaitList_size] = NULL;
  unsigned int __hc__make_graph_async1_awaitList_curr_index = 0;
  __copyInto(__hc__make_graph_async1_awaitList,__hc__make_graph_async1_awaitList_curr_index,listOfGraphGen);
  __hc__make_graph_async1_awaitList_curr_index += listOfGraphGen -> size;
  __hc__make_graph_async1_awaitList -> waitingFrontier = &__hc__make_graph_async1_awaitList -> array[0];
  __hc__make_graph_async1__frame__ -> header.awaitList = __hc__make_graph_async1_awaitList;
  __hc__make_graph_async1__frame__ -> scope0.seed = ((uint_fast32_t *)seed);
  __hc__make_graph_async1__frame__ -> scope0.make_graph_ddf = ((DDF_t *)make_graph_ddf);
  ((hc_frameHeader *)__hc__make_graph_async1__frame__) -> sched_policy = hc_sched_policy(ws);
  increment_counter();
  if (((hc_frameHeader *)__hc__make_graph_async1__frame__) -> sched_policy ==  HELP_FIRST) {
    if (__iterateDDFWaitingFrontier(&__hc__make_graph_async1__frame__ -> header)) 
      deque_push_default(__hc__make_graph_async1__frame__);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
  }
  else {
    ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope1.listOfGraphGen = listOfGraphGen;
    ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope1.nedges_ptr = nedges_ptr;
    ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope1.result_ptr = result_ptr;
    ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope1.seed = seed;
    ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope1.edges = edges;
    ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.log_numverts = log_numverts;
    ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.M = M;
    ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.userseed1 = userseed1;
    ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.userseed2 = userseed2;
    ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.nedges_ptr_in = nedges_ptr_in;
    ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.result_ptr_in = result_ptr_in;
    ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.make_graph_ddf = make_graph_ddf;
    __hc_frame__ -> pc = 1;
    deque_push_default(__hc_frame__);
			LOG_INFO(ws, "push continuation: %p: PC: %d of __hc_make_graph__, and execute async: %p in _make_graph_async1_\n", __hc_frame__, __hc_frame__->pc, __hc__make_graph_async1__frame__);
    if (__iterateDDFWaitingFrontier(&__hc__make_graph_async1__frame__ -> header)) 
      _make_graph_async1_(ws,((hc_frameHeader *)__hc__make_graph_async1__frame__),0);
    else 
      ws -> costatus =  HC_FINISH_SUSPENDED;
    if (0 || ws -> costatus !=  HC_ASYNC_COMPLETE) {
			LOG_INFO(ws, "1: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
      return ;
    }
 				{
				void * __tmp_frame__ = hc_deque_pop(ws, ws->current);
				if (__tmp_frame__ == __hc_frame__) {
				LOG_INFO(ws, "Frame is not stolen, executing the continuation: %p, PC: %d\n", __hc_frame__, __hc_frame__->pc);
				decrement_counter_ws(ws);
				hc_free_unwind_frame(ws);
				if (ws->frame != __hc_frame__) LOG_FATAL(ws, "Frame inconsistently unwinded: ws->frame: %p != %p\n", ws->frame, __hc_frame__);
				} else {
				if (__tmp_frame__ != NULL) {
				LOG_INFO(ws, "Frame is stolen, the top frame (%p) is not the current (%p), push it back!\n", __tmp_frame__, __hc_frame__);
				hc_deque_push(ws, ws->current, __tmp_frame__);
				}
				LOG_INFO(ws, "2: Frame is stolen, return with current frame: %p, PC: %d, costatus: %d\n", __hc_frame__, __hc_frame__->pc, ws->costatus);
				return;
				}
				}
				
    if (0) {
      __post_async1:
			;char buffer[36];
			LOG_INFO(ws, "continue the stolen frame: %s of __hc_make_graph__\n", printEntry(__hc_frame__, buffer));
      listOfGraphGen = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope1.listOfGraphGen;
      nedges_ptr = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope1.nedges_ptr;
      result_ptr = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope1.result_ptr;
      seed = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope1.seed;
      edges = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope1.edges;
      log_numverts = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.log_numverts;
      M = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.M;
      userseed1 = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.userseed1;
      userseed2 = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.userseed2;
      nedges_ptr_in = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.nedges_ptr_in;
      result_ptr_in = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.result_ptr_in;
      make_graph_ddf = ((struct __hc_make_graph_frame_t__ *)__hc_frame__) -> scope0.make_graph_ddf;
    }
  }
  ws -> costatus =  HC_FUNC_COMPLETE;
  return ;
}

void _make_graph_async1_(hc_workerState *ws,hc_frameHeader *__hc_frame__,int __hc_pc__)
{
  uint_fast32_t *seed;
  DDF_t *make_graph_ddf;
  switch(__hc_pc__){
    case 0:
{
      ws -> frame = __hc_frame__;
      break; 
    }
  }
  seed = ((struct __hc__make_graph_async1__frame_t__ *)__hc_frame__) -> scope0.seed;
  make_graph_ddf = ((struct __hc__make_graph_async1__frame_t__ *)__hc_frame__) -> scope0.make_graph_ddf;
  hc_mm_free(((hc_workerState *)(pthread_getspecific(wskey))),seed);
/* Stop Time: generation_time */
  generation_time = toc();
  __ddfPut(make_graph_ddf,dummyDataVoidPtr);
/* Write back value for INOUT/OUT arguments */
  ws -> costatus =  HC_ASYNC_COMPLETE ;
  return ;
}
