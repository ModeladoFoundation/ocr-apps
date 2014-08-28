#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <pthread.h>
#include <assert.h>
#include <vector>
#include <algorithm>

//#include "rmd_afl_all.h"

#include "int_vec.h"  /* define vec_int and vec_of_vec_of_ints */
#include "graph_gen.h"
#include "explorer.h"

graph_t G;
std::vector<uint32_t> perm_vec; /* TODO: use arrays  */
void edge_visitor(uint32_t src, uint32_t des)
{
  uint32_t srcp = perm_vec[src];
  uint32_t desp = perm_vec[des];
  add_int_uniq(G.adj_store.arr[srcp], desp);
  add_int_uniq(G.adj_store.arr[desp], srcp);
}


int main(int argc, char** argv)
{
   if(argc !=3 ){
    printf("usage: %s <num_explorer_threads> <problem size>\n", argv[0]);
    return -1;
   }

  /*rmd_env_param env_param = RMD_ENV_DEFAULT;
  env_param.nchips        = 1;
  env_param.block_perchip = 1;
  env_param.ce_perblock   = 1;
  env_param.xe_perblock   = 1;
  env_param.block_size    = 16*1024*1024; //16MB
  env_param.num_block     = 1;
  env_param.dram_size     = (size_t)4*1024*1024*1024; //4GB

  create_rmd_env(&env_param);
  initmem_rmd_thread(0);*/

  srand48(1234567); /* sesc simulator does not support time  (i think) */
  num_explorers = atoi(argv[1]);
  int scale=atoi(argv[2]);

  G.n = 1; G.n=G.n<<scale;
  G.m = G.n * 16;
  //  perm_vec = (uint32_t*)malloc(G.n * sizeof(uint32_t));
  perm_vec.resize(G.n);
  for(int i = 0; i < G.n; ++i)
    {
      perm_vec[i] = i;
    }
  std::random_shuffle(perm_vec.begin(), perm_vec.end());
  init_adjlist(&G);
  gen_edges(edge_visitor, G.n, G.m);
  /* graph_stats(&G); */
  uint32_t start_v=152;
  explorer_init(&G,start_v); /* initialize barriers, etc. related to explorer */
  pthread_attr_t attr;
  void* status;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_t all_threads[num_explorers];
  int num_queries = 100;
  for(int nq=0; nq < num_queries; ++nq)
    {
      printf("Start vertex = %d\n", start_v);
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
  free_Vec_IntVec(&G.adj_store);

  //cleanup_rmd_env();
  return 0;
}
