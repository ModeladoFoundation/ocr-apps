int max_levels = 20;
int num_explorers = -1;
pthread_barrier_t level_sync_barrier;

uint32_t bucket_sz;
uint32_t sp_extent = 4096; /* Reserved scratchpad space reserved  for BFS queue managment */
/* Divide into  num_explores blocks */

uint32_t sp_blk; /* scratchpad block size; sp_extent is divided into num_explorer block; each of size sp_blk */
uint32_t sp_blk_capacity; /* number of uint32_t per sp_block */


graph_t* Gptr;
uint32_t volatile *  all_ws1; /* a work-stack for each explorer */
uint32_t volatile*  all_ws2;

uint32_t start_vertex;
void explorer_init(graph_t* g, uint32_t sv)
{
  if(pthread_barrier_init(&level_sync_barrier, NULL, num_explorers))
    {
      fprintf(stderr, "explorer:init:failed\n");
      exit(-1);
    }
  assert(sp_extent%num_explorers ==0);
  sp_blk = sp_extent/num_explorers;
  assert(sp_blk%sizeof(uint32_t) == 0);
  sp_blk_capacity = sp_blk/sizeof(uint32_t);
  Gptr = g;
  bucket_sz = Gptr->n/num_explorers;
  assert(Gptr->n%num_explorers == 0);

  uint32_t sv_owner=sv/bucket_sz;
  start_vertex = sv;

  all_ws1 = new uint32_t[num_explorers];
  all_ws2 = new uint32_t[num_explorers];
  for(int i = 0; i < num_explorers; ++i)
    {
      all_ws1[i] = -1;
      all_ws2[i] = -1;
    }
}

void explorer_reset_and_seed(uint32_t sv)
{
  start_vertex = sv;
  for(int eidx = 0; eidx < num_explorers; ++eidx)
    {
      assert(all_ws1[eidx] == (uint32_t)-1);
      assert(all_ws2[eidx] == (uint32_t)-1);
    }
}

void relax(int explorer_id, uint32_t nid,  uint32_t** sp_discover_queue, volatile uint32_t* sp_discover_stack, FILE*fp)
{
  IntVec* adj_i = Gptr->adj_store.arr[nid];
  for(int j = 0; j < adj_i->num_elements; j++)
    {
      uint32_t des = adj_i->arr[j];
      uint32_t des_owner = des/bucket_sz;
      if(sp_discover_queue[des_owner][0]  < sp_blk_capacity)
	{
	  sp_discover_queue[des_owner][sp_discover_queue[des_owner][0]] =  des;
	  ++sp_discover_queue[des_owner][0];
	}
      else{
	  /* ---flush */
	/* allocate and copy sp_discover_queue[des_owner]  to main memory */
	uint32_t* blk_mm = new uint32_t[sp_blk_capacity];
	/* copy from sp_mem --> global memory */
	memcpy(blk_mm, sp_discover_queue[des_owner], sp_blk);
	push(&sp_discover_stack[des_owner],  blk_mm);
	sp_discover_queue[des_owner][0] = 1;
	sp_discover_queue[des_owner][sp_discover_queue[des_owner][0]] =  des;
	++sp_discover_queue[des_owner][0];
      }
    }
}

void flush_discovered(int explorer_id,  uint32_t** sp_discover_queue, volatile uint32_t* sp_discover_stack)
{
  for(int j = 0; j < num_explorers; ++j)
    {
      if(sp_discover_queue[j][0] > 1)
	{
	  uint32_t* blk_mm = new uint32_t[sp_blk_capacity];
	  /* copy from sp_mem --> global memory */
	  memcpy(blk_mm, sp_discover_queue[j], sp_blk);
	  push(&sp_discover_stack[j],  blk_mm);
	  sp_discover_queue[j][0] = 1;
	}
    }
}
void* explorer_thread(void* arg)
{
  int64_t id = (int64_t) arg;
  char fn[500];
  sprintf(fn, "pid_%d.log", id);
  FILE* fp = fopen(fn, "w");
  uint32_t** all_discover_wq = new uint32_t*[num_explorers];
  for(int i = 0; i < num_explorers; ++i)
    {
      all_discover_wq[i] = new uint32_t[sp_blk_capacity];
      all_discover_wq[i][0] = 1;
    }
  uint32_t num_visited = 0;

  volatile uint32_t* all_discover_ws = all_ws1;
  volatile uint32_t* all_visit_ws = all_ws2;
  uint32_t rng_begin = id* bucket_sz;
  uint32_t rng_end = (id+1)* bucket_sz;

  std::vector<bool> visited_bitmap;
  visited_bitmap.resize(bucket_sz, false);


  for(int i =0 ; i < max_levels; ++i)
    {

      pthread_barrier_wait(&level_sync_barrier);

      if(i == 0)
	{
	  uint32_t sv_owner  = start_vertex/bucket_sz;
	  if(sv_owner == id)
	    {
	      ++num_visited;
	      relax(id, start_vertex,  all_discover_wq, all_discover_ws, fp);
	    }
	}
      while(all_visit_ws[id] != (uint32_t)-1)
	{

	  payload_t visit_wb = pop(&all_visit_ws[id]); //wb--> work block
	  for(int i = 1; i < visit_wb[0]; ++i)
	     {
	      uint32_t vn = visit_wb[i];

	      assert(vn >= rng_begin);
	      assert(vn < rng_end);
	      uint32_t vn_lid = vn - rng_begin;
	      if(visited_bitmap[vn_lid] == false)
		{
		  relax(id, vn,  all_discover_wq, all_discover_ws,fp);
		  ++num_visited;
		  visited_bitmap[vn_lid] = true;
		}
	    }
	  delete visit_wb;
	}
      flush_discovered(id, all_discover_wq, all_discover_ws);
      pthread_barrier_wait(&level_sync_barrier);
      volatile uint32_t* tmps = all_visit_ws;
      all_visit_ws = all_discover_ws;
      all_discover_ws = tmps;
      pthread_barrier_wait(&level_sync_barrier);

    }
  //  printf("tid=%d num_visited = %d\n", id, num_visited);
  fclose(fp);
  return 0;
}
