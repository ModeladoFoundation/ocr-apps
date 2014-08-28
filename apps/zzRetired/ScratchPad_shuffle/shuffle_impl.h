pthread_barrier_t workers_station; //all workers stop sync-up here
namespace ds = shuffle_ds;

void scatter_nugget(int tid, int sender_id, int receiver_id)
{
  printf("sg-begin: tid = %d, sender=%d, reciever = %d\n", tid, sender_id, receiver_id);

  /* look at bin with id == receiver_id */
  int64_t recv_off = receiver_id * ds::bin_extant + sender_id * ds::nugget_extant;

  int recvout;
  int sendout;
  int64_t* rst;


  /*--------------MMAP Section---------------*/
  /* map the nugget sender_id portion */
  if ((recvout = open64(ds::bucket_fn, O_RDWR, S_IRWXU)) < 0)
    perror(ds::bucket_fn);
  //  int64_t pos = sender_id * ds::nugget_extant; /* pos has to be multiple of 4096 */

  if (lseek64(recvout, 0, SEEK_SET) == -1)
    perror("lseek error");
  if ((rst = (int64_t*) mmap64(0, ds::nugget_extant, PROT_READ|PROT_WRITE, MAP_SHARED, recvout, recv_off)) == (int64_t*) -1)
    perror("mmap error for output");

  /*-----------------------------------------*/


  /* map the  receiver_id portion in bin sender_id */

  int64_t sender_off = sender_id * ds::bin_extant + receiver_id * ds::nugget_extant;
  printf("sender_off=%d\n", sender_off);
  /*----------------MMAP Section------------------*/
  int64_t* sst;
  if ((sendout = open64(ds::cbucket_fn, O_RDWR, S_IRWXU)) < 0)
    perror(ds::cbucket_fn);
  //  pos = receiver_id * ds::nugget_extant;
  if (lseek64(sendout, 0, SEEK_SET) == -1)
    perror("lseek error");
  if ((sst = (int64_t*) mmap64(0, ds::nugget_extant, PROT_READ|PROT_WRITE, MAP_SHARED, sendout,sender_off)) == (int64_t*) -1)
    {
      perror("xx:mmap error for output");
      printf("%d\n", sender_off);
    }
  /*--------------------------------------------------*/

  memcpy(rst, sst, ds::nugget_extant);

  /*-----------------MMAP Section---------------------*/

  // msync(rst, ds::nugget_extant, MS_SYNC);

  if (munmap(sst, ds::nugget_extant) == -1) {
    perror("Error un-mmapping the file");
  }
  if (munmap(rst, ds::nugget_extant) == -1) {
    perror("Error un-mmapping the file");
  }

  close(sendout);
  close(recvout);
  /*--------------------------------------------------*/
}





int alloc_file(char *fn, int64_t sz)
{
  int fdout;
  if ((fdout = open64(fn, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU)) < 0)
    perror(fn);
  if (lseek64(fdout, sz, SEEK_SET) == -1)
    perror("lseek error");
  if (write (fdout, "", 1) != 1)
    perror("write error");
  if (lseek64(fdout, 0, SEEK_SET) == -1)
    perror("lseek error");

  return fdout;

}




void random_shuffle_bin(int bin_id)
{
  int64_t pos = bin_id * ds::bin_extant;
  int fdout;
  int64_t* rst;
  if ((fdout = open64(ds::bucket_fn, O_RDWR, S_IRWXU)) < 0)
    perror(ds::bucket_fn);
  if (lseek64(fdout, 0, SEEK_SET) == -1)
    perror("lseek error");
  if ((rst = (int64_t*) mmap64(0, ds::bin_extant, PROT_READ|PROT_WRITE, MAP_SHARED, fdout, pos)) == (int64_t*) -1)
    perror("mmap error for output");
  // std::random_shuffle(rst, rst+ds::bin_sz);
  //  msync(rst, ds::bin_extant, MS_SYNC);
  if (munmap(rst, ds::bin_extant) == -1) {
    perror("Error un-mmapping the file");
  }
}
void* shuffle_worker(void* arg)
{

  int tid = (int64_t)arg;
  //int tid = (int)arg;
  /* -------------  initialization phase----------------- */
  int start_bin= ds::num_bins_per_core * tid;
  int end_bin = ds::num_bins_per_core * (tid+1);


  for(int r = 0; r<ds::rounds; ++r)
    {
  /* -------------  random_shuffle phase----------------- */
  for(int bin_id = start_bin; bin_id < end_bin; ++bin_id)
    {
      random_shuffle_bin(bin_id);
    }
  int rc = pthread_barrier_wait(&workers_station);

  if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
    {
        printf("Could not wait on barrier\n");
        exit(-1);
    }
  if(tid ==0)
    {
      //TODO: swap the bucket and cbucket pointers
    }
  rc = pthread_barrier_wait(&workers_station);

  if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
    {
        printf("Could not wait on barrier\n");
        exit(-1);
    }


  /* -------------  scatter_gather phase----------------- */

  for(int scatter_bin_id = start_bin; scatter_bin_id < end_bin; ++scatter_bin_id)
    {
      for(int gather_bin_id = 0; gather_bin_id < ds::num_bins; ++gather_bin_id)
	{

	  scatter_nugget(tid,scatter_bin_id, gather_bin_id);
	}
    }
  rc = pthread_barrier_wait(&workers_station);
  if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
    {
        printf("Could not wait on barrier\n");
        exit(-1);
    }
    }
  /* ------------- -------------------------------- */
}



void initialize_bucket()
{
  using shuffle_ds::bucket_extant;
  using shuffle_ds::bucket_sz;
  int fdout = alloc_file(ds::bucket_fn, bucket_extant);
  int64_t* dst;

  /*---------------------MMAP section ------------------*/
  if ((dst = (int64_t*) mmap64(0, bucket_extant, PROT_READ | PROT_WRITE, MAP_SHARED, fdout, 0)) == (int64_t*)-1)
    perror("mmap error for output");
  int64_t start_idx = 0;
  int64_t end_idx = bucket_sz;
  //  std::cout<<bin_id<<" "<<start_idx<<" "<<end_idx<<std::endl;
  assert(bucket_sz * sizeof(int64_t) == bucket_extant);
  for(int64_t j = start_idx; j < end_idx; ++j)
    {
      dst[j-start_idx] =  j;
    }
  if (munmap(dst, bucket_extant) == -1) {
    perror("Error un-mmapping the file");
  }
  close(fdout);

  char bbin_fn[500];

  fdout = alloc_file(ds::cbucket_fn, ds::bucket_extant);

  if ((dst = (int64_t*) mmap64(0, bucket_extant, PROT_READ | PROT_WRITE, MAP_SHARED, fdout, 0)) == (int64_t*)-1)
    perror("mmap error for output");
  for(int64_t j = start_idx; j < end_idx; ++j)
    {
      dst[j-start_idx] =  j;
    }
  if (munmap(dst, ds::bucket_extant) == -1) {
    perror("Error un-mmapping the file");
  }
  close(fdout);
  /*---------------------MMAP section ------------------*/
}
