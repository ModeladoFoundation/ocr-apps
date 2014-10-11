

int max_levels = 20;
int num_explorers = 1;
pthread_barrier_t level_sync_barrier;

/* typedef volatile uint32_t*  head_t; */
/* typedef volatile head_t* arr_head_t; */

/* arr_head_t queue1; */
/* arr_head_t queue2; */


typedef std::vector< std::vector<uint32_t> > wq_t;
uint32_t bucket_sz;

/* explorer j creates work for i in all_wq1[i][j] */
std::vector< wq_t > all_wq1;
std::vector< wq_t > all_wq2;

std::vector<uint32_t> num_visited;
graph_t* Gptr;

void explorer_init(graph_t* g, uint32_t sv)
{
  if(pthread_barrier_init(&level_sync_barrier, NULL, num_explorers))
    {
      fprintf(stderr, "explorer:init:failed\n");
      exit(-1);
    }
  /* queue1 = (arr_head_t) malloc(G->n * sizeof(head_t)); */
  /* queue2 = (arr_head_t) malloc(G->n * sizeof(head_t)); */
  num_visited.resize(num_explorers,0);
  all_wq1.resize(num_explorers);
  all_wq2.resize(num_explorers);


  for(int i = 0; i < num_explorers; ++i)
    {
      all_wq1[i].resize(num_explorers);
      all_wq2[i].resize(num_explorers);
    }
  Gptr = g;
  bucket_sz = Gptr->n/num_explorers;
  assert(Gptr->n%num_explorers == 0);

  uint32_t sv_owner=sv/bucket_sz;
  all_wq1[sv_owner][0].push_back(sv);

}

void explorer_reset_and_seed(uint32_t sv)
{
   for(int i = 0; i < num_explorers; ++i)
    {

      for(int j = 0; j < num_explorers; ++j)
	{
	  all_wq1[i][j].clear();
	  all_wq2[i][j].clear();
	}
      num_visited[i] = 0;
    }

   uint32_t sv_owner= sv/bucket_sz;
   all_wq1[sv_owner][0].push_back(sv);
}

void relax(int explorer_id, uint32_t nid, std::vector< wq_t >*  discover_queue_ptr)
{
  IntVec* adj_i = Gptr->adj_store.arr[nid];
  for(int j = 0; j < adj_i->num_elements; j++)
    {
      uint32_t des = adj_i->arr[j];
      uint32_t des_owner = des/bucket_sz;
      //printf("tid=%d visiting=%d des_owner=%d\n", explorer_id, des, des_owner);
      (*discover_queue_ptr)[des_owner][explorer_id].push_back(des);
    }
}

void* explorer_thread(void* arg)
{
  int id = (int64_t) arg;
  std::vector< wq_t >* visit_queue_ptr = &all_wq1;
  std::vector< wq_t >* discover_queue_ptr = &all_wq2;
  uint32_t rng_begin = id* bucket_sz;
  uint32_t rng_end = (id+1)* bucket_sz;

  std::vector<bool> visited_bitmap;
  visited_bitmap.resize(bucket_sz, false);


  for(int i =0 ; i < max_levels; ++i)
    {
      //      if(id==0)printf("-------------------level=%d-----------------\n",i);
      wq_t& my_visit_queue = (*visit_queue_ptr)[id];
       for(int ne = 0; ne < num_explorers; ++ne)
	{
	  //	  printf("tid=%d wid=%d vq_sz=%d\n", id, ne, my_visit_queue[ne].size());
	  for(int k = 0; k < my_visit_queue[ne].size(); ++k)
	    {
	      uint32_t vn = my_visit_queue[ne][k]; /*relax this node*/
	      uint32_t vn_lid = vn - rng_begin;
	      assert(vn >= rng_begin);
	      assert(vn < rng_end);
	      if(visited_bitmap[vn_lid] == false)
		{
		  /* printf("visit %d %d\n", id, vn); */
		  relax(id, vn, discover_queue_ptr);
		  ++num_visited[id];
		  visited_bitmap[vn_lid] = true;
		}
	    }
	  my_visit_queue[ne].clear();
	}


      pthread_barrier_wait(&level_sync_barrier);
      std::vector< wq_t >* tmp = visit_queue_ptr;
      visit_queue_ptr = discover_queue_ptr;
      discover_queue_ptr = tmp;
      //      printf("tid=%d visit_queue=%p discover_queue=%p\n", id, visit_queue_ptr, discover_queue_ptr);

      pthread_barrier_wait(&level_sync_barrier);
      //      if(id==0)printf("------------------------------------\n");
    }
  printf("tid=%d num_visited = %d\n", id, num_visited[id]);

  return 0;
}
