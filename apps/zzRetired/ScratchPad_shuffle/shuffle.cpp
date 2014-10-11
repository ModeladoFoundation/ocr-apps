#include "headers.h"

#include "shuffle_ds.h"
#include "shuffle_impl.h"


int main(int argc, char** argv)
{
  using namespace shuffle_ds;
  num_cores = 4;
  scale = 17;
  num_nodes = (int64_t) pow(2, scale);
  bucket_sz = num_nodes;
  bucket_extant = num_nodes  * sizeof(int64_t);
  bin_extant = pow(2,18); //(int64_t) 2 * pow(2,16);
  bin_sz = bin_extant/8;
  assert(bin_extant%8 == 0);
  num_bins = num_nodes/bin_sz;
  assert(num_nodes%bin_sz == 0);
  num_bins_per_core = num_bins/num_cores;
  assert(num_bins%num_cores == 0);
  nugget_sz = bin_sz/num_bins;

  assert(bin_sz%num_bins == 0);
  nugget_extant = nugget_sz * 8;
  if(nugget_extant < 4096)
    {
      printf("nugget_extant too low %d\n", nugget_extant);
      int64_t s_num_nuggets = bucket_extant/4096;
      int64_t s_num_bins = sqrt(s_num_nuggets);
      printf("suggest num bins %d\n", s_num_bins);
      exit(0);
    }
  rounds = 1;//log(num_bins);
  std::cout<<"bin_sz = "<<bin_sz<<std::endl;
  std::cout<<"num_bins = "<<num_bins<<std::endl;
  std::cout<<"nugget_sz = "<<nugget_sz<<std::endl;
  std::cout<<"bins_per_core = "<<num_bins_per_core<<std::endl;
  initialize_bucket();
  if(pthread_barrier_init(&workers_station, NULL, num_cores))
    {
      printf("Could not create a barrier\n");
      return -1;
    }
  pthread_t all_threads[num_cores];
  for(int i =0; i< num_cores; ++i)
    {
      pthread_create(&all_threads[i], 0, shuffle_worker, (void*)i);
    }
  void* pstatus;
  for(int i = 0; i < num_cores; i++)
    {
      pthread_join(all_threads[i], &pstatus);
    }
  return 0;
}
