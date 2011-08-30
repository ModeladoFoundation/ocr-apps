
#define PTHREAD

#ifdef PTHREAD
#include <pthread.h> 
pthread_mutex_t barrier_mutex;
pthread_mutex_t complete_mutex;
pthread_cond_t barrier_cond;
pthread_cond_t complete_cond;

volatile static int _barrier_turn_ = 0;
volatile static int _barrier_go_1;
volatile static int _barrier_1[256];
volatile static int _barrier_go_2;
volatile static int _barrier_2[256];

static volatile __declspec(align(64)) unsigned _MY_BARRIER_TURN_ = 0;
static volatile __declspec(align(64)) unsigned _MY_BARRIER_COUNT_0 = 0;
static volatile __declspec(align(64)) unsigned _MY_BARRIER_COUNT_1 = 0;
static volatile __declspec(align(64)) unsigned _MY_BARRIER_FLAG_0 = 0;
static volatile __declspec(align(64)) unsigned _MY_BARRIER_FLAG_1 = 0;

#endif  //PTHREAD


#ifdef PTHREAD
//global value
static volatile int version = 0;
static volatile int gcount = 0;		

void old_barrier()
{
	pthread_mutex_lock(&complete_mutex);
	gcount += 1;
	if(gcount == nthreads){	
		gcount = 0;
		pthread_cond_broadcast(&complete_cond);	
	}
	else{
		pthread_cond_wait(&complete_cond,&complete_mutex);
	}
	pthread_mutex_unlock(&complete_mutex);
}

//#define NULL_BARRIER
//#define PTHREAD_BARRIER
//#define CONDITIONAL_BARRIER
//#define SPIN_BARRIER
#define DH1_BARRIER
//#define DH2_BARRIER
//#define LIGHT_BARRIER
void barrier(int threadid=0)
{

//  printf("tid: %d\n", threadid);
#ifdef NULL_BARRIER
	
#endif
#ifdef PTHREAD_BARRIER
    pthread_barrier_wait(&mybarrier);
#endif
#ifdef CONDITIONAL_BARRIER
	pthread_mutex_lock(&barrier_mutex);
	gcount += 1;
	if(gcount == nthreads){	
		gcount = 0;
		pthread_cond_broadcast(&barrier_cond);	
	}
	else{
		pthread_cond_wait(&barrier_cond,&barrier_mutex);
	}
	pthread_mutex_unlock(&barrier_mutex);
#endif
#ifdef SPIN_BARRIER
	int myversion = version+1;
	pthread_mutex_lock(&barrier_mutex);
	if(gcount < nthreads - 1){
		gcount++;
	}
	else{   //last thread sets back to zero
		gcount = 0;
		version++;
	}
	pthread_mutex_unlock(&barrier_mutex);
	do {
            __asm { pause };
	} while ( myversion != version); 
#endif
#ifdef DH1_BARRIER
  if (_barrier_turn_ == 0) {
    if (threadid == 0) {
      for (int i=1; i<nthreads; i++) {
        while(_barrier_1[i] == 0);
        _barrier_2[i] = 0;
      }
      _barrier_turn_ = 1;
      _barrier_go_2 = 0;
      _barrier_go_1 = 1;
    }
    else
    {
      _barrier_1[threadid] = 1;
      while(_barrier_go_1 == 0);
    }
  }
  else {
    if (threadid == 0) {
      for (int i=1; i<nthreads; i++) {
        while(_barrier_2[i] == 0);
        _barrier_1[i] = 0;
      }
      _barrier_turn_ = 0;
      _barrier_go_1 = 0;
      _barrier_go_2 = 1;
    }
    else {
      _barrier_2[threadid] = 1;
      while(_barrier_go_2 == 0);
    }
  }
#endif
#ifdef DH2_BARRIER
  if (_MY_BARRIER_TURN_) {
    asm __volatile__ ("lock incl %[mem]" :: [mem] "m" (*&_MY_BARRIER_COUNT_0));
    if (threadid == 0) {
      while (_MY_BARRIER_COUNT_0 < nthreads);
      _MY_BARRIER_TURN_ = 0;
      _MY_BARRIER_COUNT_1 = 0;
      _MY_BARRIER_FLAG_1 = 0;
      _MY_BARRIER_FLAG_0 = 1;
    }
    else  {
      while (!_MY_BARRIER_FLAG_0);
    }
  }
  else {
    asm __volatile__ ("lock incl %[mem]" :: [mem] "m" (*&_MY_BARRIER_COUNT_1));
    if (threadid == 0) {
      while (_MY_BARRIER_COUNT_1 < nthreads);
        _MY_BARRIER_TURN_ = 1;
        _MY_BARRIER_COUNT_0 = 0;
        _MY_BARRIER_FLAG_0 = 0;
        _MY_BARRIER_FLAG_1 = 1;
      }
      else {
        while (!_MY_BARRIER_FLAG_1);
      }
  }
#endif
#ifdef LIGHT_BARRIER
	int myversion = version+1;
	if(gcount < nthreads - 1) {
         __asm { lock inc dword ptr [gcount] };
	}
	else{   //last thread sets back to zero
		gcount = 0;
		version++;
     }
     do {
         __asm { pause };
     } while ( myversion != version); 
	
#endif	
}

extern int global_number_of_phases;



#if 0
void barrier3(int threadid, int phase, int iteration)
{
    if (nthreads == 1) return;

    // This function assumes that nthreads >= 2
    int current_phase = (phase+1) + iteration * (global_number_of_phases+1);

    global_Timestamp[threadid*16] = current_phase;

    if (threadid == 0)
    {
        //threadid == (0) so no prev neighbor...
        
        while ( (global_Timestamp[16*(threadid+1)] < current_phase))
        {
        }
    }

    else if (threadid < (nthreads-1))
    {
        while ( (global_Timestamp[16*(threadid-1)] < current_phase) || (global_Timestamp[16*(threadid+1)] < current_phase))
        {
        }
    }
    else
    {
        //threadid == (nthreads-1) so no next neighbor...
        while ( (global_Timestamp[16*(threadid-1)] < current_phase))
        {
        }
    }
}
#else

void barrier3(int threadid, int phase, int iteration)
{
    barrier(threadid);
}

#endif

#endif
