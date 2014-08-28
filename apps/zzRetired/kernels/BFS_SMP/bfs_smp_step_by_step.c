#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <pthread.h>
#include <assert.h>
#include <vector>
//#include <algorithm>
#include <string.h>

#include "int_vec.h"  /* define vec_int and vec_of_vec_of_ints */
#include "graph_gen.h"
#include "AO_reroute.h"

typedef uint32_t* payload_t;
#include "memory.h"
#include "stack.h"
#include "explorer_sp.h"


graph_t G;
//std::vector<uint32_t> perm_vec; /* TODO: use arrays  */

uint32_t* perm_vec;

void edge_visitor(uint32_t src, uint32_t des)
{
  uint32_t srcp = perm_vec[src];
  uint32_t desp = perm_vec[des];
  add_int_uniq(G.adj_store.arr[srcp], desp);
  add_int_uniq(G.adj_store.arr[desp], srcp);
}

double get_seconds() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return (double) (tp.tv_sec + ((1e-6)*tp.tv_usec));
}
void shuffle(uint32_t *array, size_t n)
{
    if (n > 1) {
        size_t i;
	for (i = 0; i < n - 1; i++) {
	  size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
	  uint32_t t = array[j];
	  array[j] = array[i];
	  array[i] = t;
	}
    }
}


int main(int argc, char** argv)
{
  srand48(1234567); /* sesc simulator does not support time  (i think) */

  //  num_explorers = atoi(argv[1]);
  num_explorers = atoi(argv[1]);
  //  int scale=atoi(argv[2]);
  int scale = atoi(argv[2]);
  sp_extent  = atoi(argv[3]);

  G.n = 1; G.n=G.n<<scale;
  G.m = G.n * 16;
  uint32_t mem_sz = 4 * G.n; /* heap area size */
  init_store(mem_sz);
  //  perm_vec = (uint32_t*)malloc(G.n * sizeof(uint32_t));
  perm_vec = new uint32_t[G.n];
  for(int i = 0; i < G.n; ++i)
    {
      perm_vec[i] = i;
    }
  shuffle(perm_vec, G.n);
  //  std::random_shuffle(perm_vec.begin(), perm_vec.end());
  init_adjlist(&G);
  gen_edges(edge_visitor, G.n, G.m);
  delete perm_vec;
  /* graph_stats(&G); */
  uint32_t start_v= lrand48()%G.n;
  explorer_init(&G,start_v); /* initialize barriers, etc. related to explorer */
  pthread_attr_t attr;
  void* status;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_t all_threads[num_explorers];
  int num_queries = 10;
  double elapsed  = get_seconds();
  for(int nq=0; nq < num_queries; ++nq)
    {
      //      printf("Start vertex = %d\n", start_v);
      for(int64_t i =0; i< num_explorers; ++i)
	{
	  pthread_create(&all_threads[i], 0, explorer_thread, (void*)i);
	}
      void* pstatus;
      for(int i = 0; i < num_explorers; ++i)
	{
	  pthread_join(all_threads[i], &pstatus);
	}
      start_v = lrand48()%G.n;
      explorer_reset_and_seed(start_v);
    }
  elapsed = get_seconds() - elapsed;
  fprintf(stderr, "%d %d %d %f\n", num_explorers, scale, sp_extent, elapsed/num_queries);
  free_Vec_IntVec(&G.adj_store);
  return 0;
}
