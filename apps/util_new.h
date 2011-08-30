

size_t global_large_memory_alloted = 0;
size_t global_large_memory_alloted_0 = 0;
size_t global_large_memory_alloted_1 = 0;

#ifdef USE_LARGE_PAGE

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

//#define MAX_PAGES_REQUESTED 2048
//#define MAX_PAGES_REQUESTED 5000
//#define MAX_PAGES_REQUESTED_MMAP 1250
//#define MAX_PAGES_REQUESTED_MMAP 6000

size_t MAX_LARGE_MEMORY = (size_t)(MAX_PAGES_REQUESTED)*(size_t)(2048*1024);
size_t MAX_LARGE_MEMORY_MMAP_SOCKET0 = (size_t)(MAX_PAGES_REQUESTED_MMAP_SOCKET0)*(size_t)(2048*1024);
size_t MAX_LARGE_MEMORY_MMAP_SOCKET1 = (size_t)(MAX_PAGES_REQUESTED_MMAP_SOCKET1)*(size_t)(2048*1024);

// version without NUMA
int shmid1; 
unsigned char *Large_Memory;


void *my_malloc_large(long long int size, int align)
{

      if (size%(1<<21)) {
              size = (1<<21)* (size/(1<<21) + 1);
                }
        printf("size: %lld\n", size);
          shmid1 = shmget(2, size, SHM_HUGETLB
                           | IPC_CREAT | SHM_R 
                                    | SHM_W); 
            if ( shmid1 < 0 ) { 
                    perror("shmget");
                        exit(1);
                          }
              printf("HugeTLB shmid: 0x%x\n", shmid1);
                unsigned int *a = (unsigned int *) shmat(shmid1, 0, 0);
                  if (a == (unsigned int *)-1) {
                          perror("Shared memory attach failure");
                              shmctl(shmid1, IPC_RMID, NULL);
                                  exit(2);
                                    }
                    return a;
}

void my_free_large() {
      shmctl(shmid1, IPC_RMID, NULL);
}

// user interfaces
void *my_malloc(size_t size0)
{
    size_t size = 0;
    if(size0 % 16) size = (size0+16) & 0xFFFFFFFFFFFFFFF0; else size = size0;
    if ( (global_large_memory_alloted + size) > MAX_LARGE_MEMORY) ERROR_PRINT();

    size_t curr_global_large_memory_alloted = global_large_memory_alloted;
    global_large_memory_alloted += size;
    return (Large_Memory + curr_global_large_memory_alloted);
//    return (my_malloc_large(size, 64) );
}

void Init_Large_Memory(void)
{
    Large_Memory = (unsigned char *)my_malloc_large(MAX_LARGE_MEMORY, 64); //Align the large memory to 64 bytes...

    for(size_t k=0; k<MAX_LARGE_MEMORY; k++) Large_Memory[k] = 123;
    global_large_memory_alloted = 0;
}


// version with NUMA
int fd0, fd1;
unsigned char *Large_Memory0;
unsigned char *Large_Memory1;


void *Init_malloc_large_mmap(long long int size, int align, int socketid)
{
      if (size%(1<<21)) {
              size = (1<<21)* (size/(1<<21) + 1);
      }
      printf("size: %lld\n", size);

      if(socketid){
        Large_Memory1 = (unsigned char *) mmap( 0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0);      
        printf("Large_Memory1 : %p\n",Large_Memory1);
      }
      else{
        Large_Memory0 = (unsigned char *) mmap( 0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd0, 0);      
        printf("Large_Memory0 : %p\n",Large_Memory0);
      }

      return 0;
}

void Init_Large_Memory_mmap(void)
{
    fd0 = open("/var/lib/hugetlbfs/pagesize-2097152/mem0", O_RDWR);
    if(fd0 == -1){
        printf("Could not open /var/lib/hugetlbfs/pagesize-2097152/mem0\n");
        printf("errno: %d\n", errno);
    }
    fd1 = open("/var/lib/hugetlbfs/pagesize-2097152/mem1", O_RDWR);
    if(fd1 == -1){
        printf("Could not open /var/lib/hugetlbfs/pagesize-2097152/mem1\n");
        printf("errno: %d\n", errno);
    }
    Init_malloc_large_mmap(MAX_LARGE_MEMORY_MMAP_SOCKET0, 64, 0); //Align the large memory to 64 bytes...
    Init_malloc_large_mmap(MAX_LARGE_MEMORY_MMAP_SOCKET1, 64, 1); //Align the large memory to 64 bytes...
    global_large_memory_alloted_0 = 0;
    global_large_memory_alloted_1 = 0;
}

void Release_Large_Memory_mmap(void)
{
    close(fd0);
    close(fd1);
    munmap(Large_Memory0, MAX_LARGE_MEMORY_MMAP_SOCKET0);
    munmap(Large_Memory1, MAX_LARGE_MEMORY_MMAP_SOCKET1);
}

void *my_malloc_mmap(size_t size0, int socket)
{
    size_t size = 0;
    if(size0 % 16) size = (size0+16) & 0xFFFFFFFFFFFFFFF0; else size = size0;
    //global_size = size; 
    if(socket == 0){
        if ( (global_large_memory_alloted_0 + size) > MAX_LARGE_MEMORY_MMAP_SOCKET0) ERROR_PRINT();

        size_t curr_global_large_memory_alloted = global_large_memory_alloted_0;
        global_large_memory_alloted_0 += size;
        return (Large_Memory0 + curr_global_large_memory_alloted);
    }else{
        if ( (global_large_memory_alloted_1 + size) > MAX_LARGE_MEMORY_MMAP_SOCKET1) {
      
            ERROR_PRINT();
        }

        size_t curr_global_large_memory_alloted = global_large_memory_alloted_1;
        global_large_memory_alloted_1 += size;
        return (Large_Memory1 + curr_global_large_memory_alloted);
    }
}

#else

extern size_t global_small_memory_alloted;
extern size_t global_small_memory_alloted_0;
extern size_t global_small_memory_alloted_1;

void *my_malloc(size_t size0)
{
    size_t size = 0;
    if(size0 % 16) size = (size0+16) & 0xFFFFFFFFFFFFFFF0; else size = size0;
    global_large_memory_alloted += size;
    return (malloc(size));
}

void *my_malloc_mmap(size_t size0, int socket_id)
{
#ifdef NUMA_AWARENESS
    size_t size = 0;
    if(size0 % 16) size = (size0+16) & 0xFFFFFFFFFFFFFFF0; else size = size0;

    if (socket_id == 1) global_large_memory_alloted_1 += size;
    if (socket_id == 0) global_large_memory_alloted_0 += size;
    return(numa_alloc_onnode(size, socket_id));
#else
    printf("Are you trying to run numa code on a single-socket Machine??? 'my_malloc_mmap' should not be called... Please check\n");
    ERROR_PRINT();
#endif
}

#endif



unsigned long long int Return_Max(unsigned long long int *X)
{
    int i;
    unsigned long long int min_time;
    unsigned long long int max_time;
    unsigned long long int avg_time;
    unsigned long long int sum_time = 0;

    min_time = X[0];
    max_time = X[0];

    for(i=0; i<nthreads; i++)
    {
        printf("Time taken by (%d) thread is %lld cycles \n", i, X[i]);
        if (X[i] < min_time) min_time = X[i];
        if (X[i] > max_time) max_time = X[i];
        sum_time += X[i];
    }

    avg_time = sum_time / nthreads;

    printf("Min_Time = %lld ::: Max_Time = %lld ::: Avg. Time = %lld ::: Max/Min Ratio = %.2lfX\n", min_time, max_time, avg_time, (max_time*1.0/min_time));

    return (avg_time);
}



int Adjust(int number)
{

    if (number <= 1) number = 1;
    if (number >= 30) number = 30;
    return(pow(2, number));

    int i;
    int ii = 2;

    for(ii=2; ; ii = ii*2)
    {
        if (ii >= number) break;
    }

    return (ii);
}

