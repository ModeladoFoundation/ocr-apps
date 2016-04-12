/*
* This file is subject to the license agreement located in the file LICENSE
* and cannot be distributed without it. This notice cannot be
* removed or modified.
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/mman.h>
#include "mpi.h"
//#define DEBUG_MPI 1

__THREAD int mygpid = -1;
int mpilite_rank;
int mpilite_commbuff;
void *mpilite_pool;
void *mpilite_init_struct;
void *mpilite_barrier;


#ifdef MPILITE_SHARED_HEAP
#include "common.c"
static void quickPrint();

//#define ENABLE_VALGRIND
#include "quick-allocator.c"

void *ffwd_alloc(int size)
{
	void *ret = quickMalloc((poolHdr_t *)mpilite_pool, size, NULL);
	assert(ret);
	return ret;
}

void *ffwd_free(void *p)
{
	quickFree(p);
}

struct ffwd_message *ffwd_mq_alloc(int size)
{
	void *ret = quickMalloc((poolHdr_t *)mpilite_pool, sizeof(struct ffwd_message)+size, NULL);
	assert(ret);
	return ret;
}
void ffwd_mq_free(struct ffwd_message *p)
{
	quickFree(p);
}

#endif


struct ffwd_barrier_t {
	volatile long count;
	volatile long round;
	char pad[64-sizeof(long)*2];	// safe to pad, because this struct is accessed thru uncacheable rsn
};

void ffwd_barrier(int MAX, struct ffwd_barrier_t *bar)
{
	long next_round = (bar)->round+1;
	__sync_synchronize();	//_fence_load();
	long ret = __sync_fetch_and_add(&bar->count, 1);	// _xadd64(&((bar)->count), 1);
	if (ret == (MAX)-1) {
		(bar)->count = 0;
		__sync_synchronize();	//_fence_store();
		(bar)->round = next_round;
	}
	while ((bar)->round != next_round) {
		//_local_ffwd_yield();
		__sync_synchronize();	//_fence_load();
	}
}


static void set_env_var(void)
{
	char *s;

	mpilite_rank = MPILITE_RANK;
	s = getenv("MPILITE_RANK");
	if (s)
		mpilite_rank = atoi(s);
	assert(mpilite_rank > 0);

	mpilite_commbuff = MPILITE_COMMBUFF;
	s = getenv("MPILITE_COMMBUFF");
	if (s)
		mpilite_commbuff = atoi(s);
	assert(mpilite_commbuff > 0);
	mpilite_commbuff *= 1024*1024;
}

#ifdef MPILITE_THREAD_MODEL
// Use align(64) so it is alone in a cache line.
static struct ffwd_barrier_t mpi_bar __attribute__((aligned(64)));	// TODO move to scratch pad??

void *mpilite_main(void *arg)
{
	mygpid = (long) arg;
	int ret = main(0, NULL);
	return NULL;
}

void mpilite_entry(void *a,void *b,void *c,void *d)
{
	set_env_var();
	mpilite_pool = calloc(1, mpilite_commbuff);
	assert(mpilite_pool && "Internal buffer allocation failed. Check MPILITE_POOL_SIZE env var, please.");

	int i, err;
	pthread_t tcb[mpilite_rank];
	for(i=0;i<mpilite_rank;i++) {
		err = pthread_create(&tcb[i], NULL, mpilite_main, (long)i);
		assert(err == 0);
	}
	void *status;
	for(i=0;i<mpilite_rank;i++) {
		err = pthread_join(tcb[i], &status);
		assert(err == 0);
	}
	exit(0);
}
#endif




struct element {
	short int off;
	short int count;
	MPI_Datatype type;
};

struct datatype {
	unsigned char flag;
	unsigned char count;
	short int size;		// may include short padding
	short int net_size;	// sum of len
	short int extent;	// extent
	struct element e[0];
};


#define MPI_TYPEFLAG_PACKED	0x01
#define MPI_TYPEFLAG_VECTOR	0x02	// a vector consists of same packed types with strides
#define MPI_TYPEFLAG_BASIC	0x80
//#define SCRATCHPAD_COMM_BUFFER

#if 0

long time_mpi_start;
long time_mpi_end;
// MPI_[Barrier,Bcast,Reduce,Allreduce,Scatter,Gather,Allgather,Alltoall,Alltoallv]
long count_barrier, time_barrier, time_wait_barrier, time_ceng_barrier;
long count_bcast, time_bcast, time_wait_bcast, time_ceng_bcast;
long count_reduce, time_reduce, time_wait_reduce, time_ceng_reduce;
long count_allreduce, time_allreduce, time_wait_allreduce, time_ceng_allreduce;
long count_scatter, time_scatter, time_wait_scatter;
long count_gather, time_gather, time_wait_gather;
long count_allgather, time_allgather, time_wait_allgather, time_ceng_allgather;
long count_alltoall, time_alltoall, time_wait_alltoall, time_ceng_alltoall;	// includes alltoallv
// MPI_[Sendrecv, Isend, Send, Irecv, Recv]
long count_sendrecv, time_sendrecv, time_wait_sendrecv;
long count_isend, time_isend;	// no time_wait_isend;
long count_send, time_send, time_wait_send;
long count_irecv, time_irecv;	// no time_wait_irecv;
long count_recv, time_recv, time_wait_recv;
// MPI_[Wait,Waitall,Waitall_fortran]
long count_wait, time_wait, time_wait_wait;
// MPI_[Test,Testall]
long count_test, time_test;
// MPI_[Probe,Probeall]
long count_probe, time_probe, time_wait_probe;


long size_p2p_sum;
long size_p2p_64;
long size_p2p_256;
long size_p2p_1k;
long size_p2p_4k;
long size_p2p_16k;
long size_p2p_64k;
long size_p2p_big;
long count_p2p_sum;
long count_p2p_64;
long count_p2p_256;
long count_p2p_1k;
long count_p2p_4k;
long count_p2p_16k;
long count_p2p_64k;
long count_p2p_big;
long size_collective_sum;
long size_collective_64;
long size_collective_256;
long size_collective_1k;
long size_collective_4k;
long size_collective_16k;
long size_collective_64k;
long size_collective_big;
long count_collective_sum;
long count_collective_64;
long count_collective_256;
long count_collective_1k;
long count_collective_4k;
long count_collective_16k;
long count_collective_64k;
long count_collective_big;
#endif

// assumption : each rank calls exactly same sequence of datatype calls. This ensures same datatypes are maintained independently.
#define MAX_DATATYPE	128
static struct datatype *datatype_table[MAX_DATATYPE];
static int datatype_max;

static inline int datatype_size(MPI_Datatype type)
{
	return datatype_table[type]->size;
}

static MPI_Datatype init_basic_datatype(int len)
{
	struct datatype *ret = malloc(sizeof(struct datatype));
	if (!ret)
		assert(0 && "init_basic_datatype failed?\n");
	ret->flag = MPI_TYPEFLAG_PACKED | MPI_TYPEFLAG_BASIC;
	ret->count = 0;	// 0 for basic types
	ret->size = ret->net_size = ret->extent = len;

	int i = datatype_max;
	assert(i < MAX_DATATYPE);
	datatype_table[datatype_max++] = ret;
	return i;
}

static void fini_basic_datatype(void)
{
	int i;
	for(i=0;i<datatype_max;i++) {
		free(datatype_table[i]);
	}
}

MPI_Datatype MPI_CHAR, MPI_SIGNED_CHAR;
MPI_Datatype MPI_SHORT;
MPI_Datatype MPI_INT;
MPI_Datatype MPI_LONG;
MPI_Datatype MPI_LONG_LONG, MPI_LONG_LONG_INT;
MPI_Datatype MPI_BYTE, MPI_UNSIGNED_CHAR;
MPI_Datatype MPI_UNSIGNED_SHORT;
MPI_Datatype MPI_UNSIGNED;
MPI_Datatype MPI_UNSIGNED_LONG;
MPI_Datatype MPI_UNSIGNED_LONG_LONG;
MPI_Datatype MPI_UINT64_T;
MPI_Datatype MPI_FLOAT;
MPI_Datatype MPI_DOUBLE;
MPI_Datatype MPI_LONG_DOUBLE;
//MPI_Datatype MPI_UB;
MPI_Datatype MPI_DOUBLE_INT;

typedef struct {
	double a;
	int b;
} double_int;

#define PRV_EVENT(X, Y)	\
	do {	\
		printf("[PRV]2:%d:1:%d:%d:%ld:%d:%d\n", (FFWD_GLOBAL_PID+1), (FFWD_GLOBAL_PID+1), FlixCurrentThread()->index_in_process+1 , _cycle_count, X, Y);	\
	} while(0)


#if 0
static int mpmode4, mpmode_info;
#define mpi_depth	(FlixCurrentThread()->mpi_depth)
#define mpi_waittime	(FlixCurrentThread()->mpi_waittime)
#define mpi_cengtime	(FlixCurrentThread()->mpi_cengtime)

#endif

int MPI_Init_thread( int *argc, char ***argv, int required, int *provided )
{
#if 0
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Init_thread\n", mpi_depth);
		PRV_EVENT(50000003, 0);
	}
	mpi_depth++;
#endif
	*provided = MPI_THREAD_MULTIPLE;
	if (required > *provided) {
		printf("INFO MPI_Init_thread required higher threading model.\n");
	}
	MPI_Init(argc, argv);
#if 0
	//A threaded MPI program that does not call MPI_Init_thread is an incorrect program
	mpi_depth--;
	if (depth) {
		PRV_EVENT(50000003, 1);
	}
#endif
	return MPI_SUCCESS;
}

static int mpi_inited;

_MPI_Group _mpi_group_world;
_MPI_Group _mpi_group_self;
MPI_Group mpi_group_world = &_mpi_group_world;
MPI_Group mpi_group_self  = &_mpi_group_self;


_MPI_Comm _mpi_comm_world;
_MPI_Comm _mpi_comm_self;
MPI_Comm mpi_comm_world = &_mpi_comm_world;
MPI_Comm mpi_comm_self = &_mpi_comm_self;

void init_group_and_comm(void)
{
	int i;
	if (MPI_GROUP_MAX < FFWD_GLOBAL_MAX)
		assert(0 && "BUG! MPI_GROUP_MAX must be >= FFWD_GLOBAL_MAX\n");
	for(i=0;i<FFWD_GLOBAL_MAX;i++)
		mpi_group_world->gpid[i] = i;
	mpi_group_world->size = FFWD_GLOBAL_MAX;
#ifndef MPILITE_THREAD_MODEL
	mpi_group_world->rank = FFWD_GLOBAL_PID;
#endif
	mpi_group_self->size = 1;
	mpi_group_self->gpid[0] = FFWD_GLOBAL_PID;
#ifndef MPILITE_THREAD_MODEL
	mpi_group_self->rank = 0;
#endif
	mpi_comm_world->group = mpi_group_world;
	mpi_comm_self->group = mpi_group_self;
}

struct init_struct {
	struct ffwd_lock_t lock;
	int count;
};

int MPI_Init(int *argc, char ***argv)	// argc,argv is not used.
{
#if 0
	time_mpi_start = _cycle_count;
	mpmode4 = (FlixGetParam()->mpmode / 1000 ) % 10;
	mpmode_info = 1;	// on by default
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		int numProc = FlixGetParam()->islands_per_socket*FlixGetParam()->processes_per_island;
		int numThread = FlixGetParam()->pes_per_island/FlixGetParam()->processes_per_island;
		printf("[MPI] %d Init\n", mpi_depth);
		printf("[PRV]#Paraver (xx/xx/xxxx at xx:xx):0:0:1:%d", numProc);
		int i;
		for(i=0;i<numProc;i++) {
			if (i==0)
				printf("(");
			else
				printf(",");
			printf(	"%d:%d", numThread, ((i*numThread)/FlixGetParam()->pes_per_island)+1);
		}
		printf(")\n");
		PRV_EVENT(50000003, 2);
	}
	mpi_depth++;
#endif
#ifdef MPILITE_THREAD_MODEL
	struct init_struct *t = mpilite_pool;
	void *p;
	if (mygpid == 0) {
		t->count = mpilite_rank;
		p = (void *)((unsigned long)mpilite_pool + sizeof(struct init_struct));
		assert(sizeof(struct init_struct) + ffwd_mq_matrix_size() <= mpilite_commbuff);
	}
	mpilite_barrier = &mpi_bar;
#else
	set_env_var();

	void *p = mmap(NULL, mpilite_commbuff, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
	assert(p && "Internal buffer setup failed (mmap). Check MPILITE_POOL_SIZE env var, please.");
	mpilite_pool = p;

#ifdef MPILITE_SHARED_HEAP
        quickInit((poolHdr_t *)mpilite_pool, mpilite_commbuff);
	struct init_struct *t = quickMalloc((poolHdr_t *)mpilite_pool, sizeof(struct init_struct), NULL);
	assert(t);

	p = quickMalloc((poolHdr_t *)mpilite_pool, 64*2, NULL );
	assert(p);
	memset(p, 0, 64*2 );
	mpilite_barrier = (void *)(((unsigned long)p + 63) & ~63);

	p = quickMalloc((poolHdr_t *)mpilite_pool, ffwd_mq_matrix_size(), NULL );
	assert(p);
	memset(p, 0, ffwd_mq_matrix_size());
#endif
	ffwd_lock_init(t->lock);
	t->count = 0;

	int i;
	for(i=1;i<mpilite_rank;i++) {
		pid_t pid = fork();
		if (pid == 0) {
			break;
		} else if (pid > 0) {
		} else
			assert(0 && "fork() failed\n");
	}
	ffwd_lock_acquire(t->lock);
	mygpid = t->count;
	t->count++;
	ffwd_lock_release(t->lock);
	// wait until others. Without this, one goes to MPI_Finalize(), and mess up rank assignment.
	while(t->count != mpilite_rank);
#endif
	mpilite_init_struct = t;

#ifdef MPILITE_THREAD_MODEL
	if (mygpid == 0)
#else
	if ( 1 )	// TODO: only master thread of rank 0 ?
#endif
	{	// only master PE does init
		ffwd_mq_init(p);	// this is only place ffwd_mq_init() is called.
		init_group_and_comm();
		// signed
		MPI_CHAR = MPI_SIGNED_CHAR = init_basic_datatype( sizeof(char) );	// 0
		MPI_SHORT = init_basic_datatype( sizeof(short) );			// 1
		MPI_INT = init_basic_datatype( sizeof(int) );				// 2
		MPI_LONG = init_basic_datatype( sizeof(long) );				// 3
		MPI_LONG_LONG = MPI_LONG_LONG_INT = init_basic_datatype( sizeof(long long) );	// 4
		// unsigned
		MPI_BYTE = MPI_UNSIGNED_CHAR = init_basic_datatype( sizeof(unsigned char) );	// 5
		MPI_UNSIGNED_SHORT = init_basic_datatype( sizeof(unsigned short) );		// 6
		MPI_UNSIGNED = init_basic_datatype( sizeof(unsigned) );				// 7
		MPI_UNSIGNED_LONG = init_basic_datatype( sizeof(unsigned long) );		// 8
		MPI_UNSIGNED_LONG_LONG = init_basic_datatype( sizeof(unsigned long long) );	// 9
		assert(sizeof(uint64_t) == 8);
		MPI_UINT64_T = init_basic_datatype( sizeof(uint64_t) ); // fixed size.
		// float
		MPI_FLOAT = init_basic_datatype( sizeof(float) );
		MPI_DOUBLE = init_basic_datatype( sizeof(double) );
		MPI_LONG_DOUBLE = init_basic_datatype( sizeof(long double) );
		// special
//		MPI_UB = 1;
		// struct
		MPI_DOUBLE_INT = init_basic_datatype( sizeof(double_int) );
		mpi_inited = 1;
	}
#ifdef MPILITE_THREAD_MODEL
	else while(mpi_inited==0);	// wait for other ranks. without this, comm_world->group may be null
#endif

#if 0
	mpi_depth--;
	if (depth) {
		PRV_EVENT(50000003, 3);
	}
#endif
	return MPI_SUCCESS;
}

int MPI_Initialized(int *flag)
{
	*flag = mpi_inited;
	return MPI_SUCCESS;
}


void mpi_lite_info(void)
{
#if 0
	printf("=== MPI-lite stats (sum across threads within current rank. see other .console files for other ranks) ===\n");
	time_mpi_end  = _cycle_count;
	long count_collectives = count_barrier+count_bcast+count_reduce+count_allreduce+count_scatter+count_gather+count_allgather+count_alltoall;
	long time_collectives = time_barrier+time_bcast+time_reduce+time_allreduce+time_gather+time_allgather+time_alltoall;
	long time_wait_collectives = time_wait_barrier+time_wait_bcast+time_wait_reduce+time_wait_allreduce+time_wait_gather+time_wait_allgather+time_wait_alltoall;
	long time_ceng_collectives = time_ceng_barrier+time_ceng_bcast+time_ceng_reduce+time_ceng_allreduce+time_ceng_allgather+time_ceng_alltoall;
	printf("                  count :       time          waittime              cengtime\n");
	printf("Collectives %9ld : %9ld ns %9ld ns (%.2lf%%)  %9ld ns (%.2lf%%)\n", count_collectives, time_collectives,
			time_wait_collectives, (double)time_wait_collectives*100.0/(double)time_collectives,
			time_ceng_collectives, (double)time_ceng_collectives*100.0/(double)time_collectives
		);
	printf("  barrier   %9ld : %9ld ns %9ld ns         %9ld ns\n", count_barrier, time_barrier, time_wait_barrier, time_ceng_barrier);
	printf("  bcast     %9ld : %9ld ns %9ld ns         %9ld ns\n", count_bcast,  time_bcast, time_wait_bcast, time_ceng_bcast);
	printf("  reduce    %9ld : %9ld ns %9ld ns         %9ld ns\n", count_reduce, time_reduce, time_wait_reduce, time_ceng_reduce);
	printf("  allreduce %9ld : %9ld ns %9ld ns         %9ld ns\n", count_allreduce, time_allreduce, time_wait_allreduce, time_ceng_allreduce);
	printf("  scatter   %9ld : %9ld ns %9ld ns\n", count_scatter, time_scatter, time_wait_scatter);
	printf("  gather    %9ld : %9ld ns %9ld ns\n", count_gather, time_gather, time_wait_gather);
	printf("  allgather %9ld : %9ld ns %9ld ns         %9ld ns\n", count_allgather, time_allgather, time_wait_allgather, time_ceng_allgather);
	printf("  alltoall  %9ld : %9ld ns %9ld ns         %9ld ns\n", count_alltoall, time_alltoall, time_wait_alltoall, time_ceng_alltoall);
	printf("  sizes       sum = (<64) + (<256) + (<1k) + (<4k) + (<16k) + (<64k) + (>=64k)\n");
	printf("  sizes(col)  %ld = %ld + %ld + %ld + %ld + %ld + %ld + %ld\n", size_collective_sum, size_collective_64, size_collective_256, size_collective_1k, size_collective_4k, size_collective_16k, size_collective_64k, size_collective_big);
	printf("  sizes(c-c)  %ld = %ld + %ld + %ld + %ld + %ld + %ld + %ld\n", count_collective_sum, count_collective_64, count_collective_256, count_collective_1k, count_collective_4k, count_collective_16k, count_collective_64k, count_collective_big);
	long count_p2p = count_sendrecv+count_isend+count_send+count_irecv+count_recv;
	long time_p2p = time_sendrecv+time_isend+time_send+time_irecv+time_recv;
	long time_wait_p2p = time_wait_sendrecv+time_wait_send+time_wait_recv;
	printf("P-to-P      %9ld : %9ld ns %9ld ns, %.2lf%%\n", count_p2p, time_p2p, time_wait_p2p,
			(double)time_wait_p2p*100.0/(double)time_p2p
		);
	printf("  sendrecv  %9ld : %9ld ns %9ld ns\n", count_sendrecv, time_sendrecv, time_wait_sendrecv);
	printf("  isend     %9ld : %9ld ns\n", count_isend, time_isend);
	printf("  irecv     %9ld : %9ld ns\n", count_irecv, time_irecv);
	printf("  send      %9ld : %9ld ns %9ld ns\n", count_send, time_send, time_wait_send);
	printf("  recv      %9ld : %9ld ns %9ld ns\n", count_recv, time_recv, time_wait_recv);
	printf("  sizes       sum = (<64) + (<256) + (<1k) + (<4k) + (<16k) + (<64k) (>=64k)\n");
	printf("  sizes       * This excludes received messages. only sent messages are counted *\n");
	printf("  sizes(p2p)  %ld = %ld + %ld + %ld + %ld + %ld + %ld + %ld\n", size_p2p_sum, size_p2p_64, size_p2p_256, size_p2p_1k, size_p2p_4k, size_p2p_16k, size_p2p_64k, size_p2p_big);
	printf("  sizes(p-c)  %ld = %ld + %ld + %ld + %ld + %ld + %ld + %ld\n", count_p2p_sum, count_p2p_64, count_p2p_256, count_p2p_1k, count_p2p_4k, count_p2p_16k, count_p2p_64k, count_p2p_big);

	printf("Wait        %9ld : %9ld ns %9ld ns\n", count_wait, time_wait, time_wait_wait);
	printf("Probe       %9ld : %9ld ns %9ld ns\n", count_probe, time_probe, time_wait_probe);
	printf("Test        %9ld : %9ld ns %9ld ns\n", count_test, time_test);
	printf("Total comm  %9ld : %9ld ns, %.2lf%% (out of total exec)\n",
		count_collectives + count_p2p,
		time_collectives+time_p2p+time_wait+time_test+time_probe,
		(double)(time_collectives+time_p2p+time_wait+time_test+time_probe)*100.0/(double)(time_mpi_end-time_mpi_start)
	);

	int pes_per_rank = FlixGetParam()->pes_per_island / FlixGetParam()->processes_per_island;
	printf("comm-per-PE %9.1lf : %9.1lf ns, %.2lf%% (out of total exec)\n",
		(count_collectives + count_p2p)/(double)pes_per_rank,
		(time_collectives+time_p2p+time_wait+time_test+time_probe)/(double)pes_per_rank,
		(double)(time_collectives+time_p2p+time_wait+time_test+time_probe)*100.0/(double)(time_mpi_end-time_mpi_start)/(double)pes_per_rank
	);
	printf("comm-per-PE is total-comm divided by #PEs ( %d PEs per rank )\n", pes_per_rank);
	printf("Choose one of two depending on your workload( e.g., use comm-per-PE for SNAP )\n");

	printf("Total exec              %9ld ns (elap.time, Init to Finalize, for current rank)\n", time_mpi_end - time_mpi_start);
#endif
}


int MPI_Finalize(void)
{
#if 0
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Finalize\n", mpi_depth);
		PRV_EVENT(50000003, 4);
	}
#endif
	mpi_lite_info();

	struct init_struct *t = mpilite_init_struct;
	int num;

	ffwd_lock_acquire(t->lock);
	t->count--;
	num = t->count;
	ffwd_lock_release(t->lock);
#ifdef MPILITE_THREAD_MODEL
	if (num == 0)	// last one does cleanups
#else
	if ( 1 )	// TODO: only master thread of rank 0 ?
#endif
	{
		fini_basic_datatype();
#ifdef MPILITE_THREAD_MODEL
		free(mpilite_pool);
		mpilite_pool = NULL;
#endif
	}
	// TODO: rever operation of mpi_init
//        mpi_inited = 0;	// what if multithreaded?
#if 0
	if (depth) {
		PRV_EVENT(50000003, 5);
	}
#endif
	return MPI_SUCCESS;
}
int MPI_Abort(MPI_Comm comm, int errorcode)
{
	printf("MPI_Abort() is called, errorcode %d\n", errorcode);
	assert(0 && "Look at errorcode and use core file to debug your mpi code.");
	return 0;
}
double MPI_Wtime( void )	// Time in seconds since an arbitrary time in the past.
{
	struct timeval now;
	int rc;
	double ret;
	rc = gettimeofday(&now, NULL);	// rc always 0 for now
	ret = (double) now.tv_sec + (double) now.tv_usec * 1.0e-6;
	return ret;
}

int MPI_Error_class(int errorcode, int *errorclass)
{
	*errorclass = errorcode;	// hacky
	return MPI_SUCCESS;
}

int MPI_Error_string(int errorcode, char *string, int *resultlen)
{
	*resultlen = sprintf(string, "MPI_ERR %d (See mpi.h)", errorcode);
	assert(*resultlen < MPI_MAX_ERROR_STRING);
	return MPI_SUCCESS;
}

int MPI_Comm_create_errhandler(MPI_Comm_errhandler_fn *function, MPI_Errhandler *errhandler)
{
	*errhandler = function;
	return MPI_SUCCESS;
}

int MPI_Comm_set_errhandler(MPI_Comm comm, MPI_Errhandler errhandler)
{
	comm->handler = errhandler;
	return MPI_SUCCESS;
}

// Users should note that the default error handler is MPI_ERRORS_ARE_FATAL. Thus, calling MPI_COMM_CALL_ERRHANDLER will abort the comm processes if the default error handler has not been changed for this communicator or on the parent before the communicator was created.
int MPI_Comm_call_errhandler(MPI_Comm comm, int errorcode)
{
	// TODO use 'comm'
	error_handling(errorcode);
	return MPI_SUCCESS;
}


int MPI_Errhandler_free(MPI_Errhandler *errhandler)
{
	*errhandler = NULL;
}

// hacky
int MPI_Attr_get(MPI_Comm comm, int keyval,void *attribute_val, int *flag )
{
	*flag = 1; // true;
	if (keyval == MPI_TAG_UB) {
		*(int *)attribute_val = MPILITE_TAG_MAX;
		return MPI_SUCCESS;
	}
	assert(0 && "TODO: not-yet-implemeted keyval in MPI_Attr_get()");
	*flag = 0; //false;
}


#if 1
static inline int gpid_to_rank(MPI_Comm comm, int gpid)
{
	int i;
	for(i=0;i<comm->group->size;i++) {
		if (comm->group->gpid[i] == gpid)
			return i;
	}
	assert(0 && "BUG? no such gpid in this comm\n");
}

static inline int _rank_to_gpid(MPI_Group group, int rank)
{
	if (rank == MPI_ANY_SOURCE)
		return FFWD_MQ_ANY_SOURCE;
	assert(rank >= 0);
	assert(group->size > rank);
	return group->gpid[rank];
}

static inline int rank_to_gpid(MPI_Comm comm, int rank)
{
	if (rank == MPI_ANY_SOURCE)
		return FFWD_MQ_ANY_SOURCE;
	assert(rank >= 0);
	assert(comm->group->size > rank);
	return comm->group->gpid[rank];
}

int MPI_Group_size(MPI_Group group, int *size)
{
	*size = group->size;
	return MPI_SUCCESS;
}

int MPI_Group_rank(MPI_Group group, int *rank)
{
#ifdef MPILITE_THREAD_MODEL
	int i;
	for(i=0;i<group->size;i++) {
		if (group->gpid[i] == mygpid) {
			*rank = i;
			return MPI_SUCCESS;
		}
	}
	return MPI_UNDEFINED;
#else
	if (group->rank == -1)
		return MPI_UNDEFINED;
	*rank = group->rank;
	return MPI_SUCCESS;
#endif
}

int MPI_Comm_group(MPI_Comm comm, MPI_Group *group)
{
	*group = comm->group;
	return MPI_SUCCESS;
}


int error_handling(int err)
{
	// TODO just use comm_world for now.
	MPI_Comm comm = mpi_comm_world;

	if (comm->handler == MPI_ERRORS_ARE_FATAL)
		MPI_Abort(comm, err);
	else if (comm->handler == MPI_ERRORS_RETURN)
		return err;
	// use user-defined error handler
	MPI_Errhandler f = comm->handler;
	f(&mpi_comm_world/*TODO*/, &err);
	return err;
}


#ifndef MPILITE_THREAD_MODEL
int MPI_Group_incl(MPI_Group group, int n, const int ranks[], MPI_Group *newgroup)
{
	int i;
	MPI_Group ret = malloc(sizeof(_MPI_Group));
	if (!ret)
		return error_handling(MPI_ERR_INTERN);
	ret->rank = -1;
	for(i=0;i<n;i++) {
		ret->gpid[i] = _rank_to_gpid(group, ranks[i]);
		if (ret->gpid[i] == FFWD_GLOBAL_PID) {
			ret->rank = i;
		}
	}
	ret->size = n;
	*newgroup = ret;
	return MPI_SUCCESS;
}
#endif

static int get_my_rank(MPI_Group group)
{
	int i;
	for(i=0;i<group->size;i++)
		if (group->gpid[i] == FFWD_GLOBAL_PID) {
			return i;
	}
	return -1;
}


#ifndef MPILITE_THREAD_MODEL
static int MPI_Group_dup(MPI_Group group, MPI_Group *newgroup)
{
	MPI_Group ret = malloc(sizeof(_MPI_Group));
	if (!ret)
		return error_handling(MPI_ERR_INTERN);
	*ret = *group;
	ret->rank = get_my_rank(group);
	*newgroup = ret;
	return MPI_SUCCESS;
}
#endif

int MPI_Group_free(MPI_Group *group)
{
	free(*group);
	*group = MPI_GROUP_NULL;
	return MPI_SUCCESS;
}

int MPI_Comm_create(MPI_Comm comm, MPI_Group group, MPI_Comm *newcomm)
{
	MPI_Comm ret = malloc(sizeof(_MPI_Comm));
	if (!ret)
		return error_handling(MPI_ERR_INTERN);
	ret->group = group;
	ret->topology = NULL;
	ret->handler = NULL;
	*newcomm = ret;
	return MPI_SUCCESS;
}

#ifdef MPILITE_TEMP
int MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm)
{
	// deep-copy. first, copy group
	MPI_Group newgroup;
	if (MPI_Group_dup(comm->group, &newgroup) != MPI_SUCCESS)
		return error_handling(MPI_ERR_INTERN);
	if (MPI_Comm_create(comm, newgroup, newcomm) != MPI_SUCCESS) {
		MPI_Group_free(&newgroup);
		return error_handling(MPI_ERR_INTERN);
	}
#ifdef DEBUG_MPI
	printf("MPI_Comm_dup: success, comm: 0x%p, *newcomm:%p\n", comm, *newcomm);
#endif
	return MPI_SUCCESS;
}

// Creates new communicators based on colors and keys
int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm)
{
	int size = comm->group->size;
	int colors[size];
	int keys[size];

	MPI_Allgather(&color, 1, MPI_INT, colors, 1, MPI_INT, comm);
	MPI_Allgather(&key  , 1, MPI_INT, keys  , 1, MPI_INT, comm);

	if (color == MPI_UNDEFINED) {
		*newcomm = MPI_COMM_NULL;
		return MPI_SUCCESS;
	}
	assert(color >= 0); // The color must be non-negative or MPI_UNDEFINED.

#ifdef DEBUG_MPI
	printf("global colors:");
	for(i=0;i<size;i++) {
		printf("%d ", colors[i]);
	}
	printf("\n");
#endif
	int i,j,tmp;
	int ranks[size];
	int keys2[size];
	int count = 0;
	for(i=0;i<size;i++) {
		if (colors[i] != color)
			continue;
		keys2[count  ] = keys[i];
		ranks[count++] = i;
	}

	// sorting by key values -- yes, it's O(n^2)
	for(i=0;i<count-1;i++) {
		for(j=i+1;j<count;j++) {
			if (keys2[i]>keys2[j]) {
				tmp = ranks[i];
				ranks[i] = ranks[j];
				ranks[j] = tmp;
				tmp = keys2[i];
				keys2[i] = keys2[j];
				keys2[j] = tmp;
			}
		}
	}

	MPI_Group newgroup;
	MPI_Group_incl(comm->group, count, ranks, &newgroup);
#ifdef DEBUG_MPI
	// print group
	printf("newgroup size:%d rank:%d gpid:", newgroup->size, newgroup->rank);
	for(i=0;i<newgroup->size;i++)
		printf("%d ", newgroup->gpid[i]);
	printf("\n");
#endif
	MPI_Comm_create(comm, newgroup, newcomm);
	return MPI_SUCCESS;
}

#endif
int MPI_Comm_free(MPI_Comm *comm)
{
	MPI_Group_free(&(*comm)->group);
	MPI_Topology t = ((*comm)->topology);
        if (t)
		free(t);
	free((*comm));
	*comm = MPI_COMM_NULL;
	return MPI_SUCCESS;
}


int MPI_Comm_rank(MPI_Comm comm, int *rank)
{
	MPI_Group_rank(comm->group, rank);
	return MPI_SUCCESS;
}
int MPI_Comm_size(MPI_Comm comm, int *size)
{
	MPI_Group_size(comm->group, size);
	return MPI_SUCCESS;
}

#ifdef MPILITE_FORTRAN
MPI_Comm MPI_Comm_f2c(MPI_Fint comm)
{
#ifdef DEBUG_MPI
  printf("MPI_Comm_f2c: comm 0x%x\n", comm);
#endif
	return comm;
}
#endif
#endif



int MPI_Barrier( MPI_Comm comm )
{
#if 0
	mpi_waittime = 0;
	mpi_cengtime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
//		printf("[MPI] %d == Barrier %s==\n", mpi_depth, (mpmode3)?"(ceng) ":"");
		printf("[MPI] %d == Barrier ==\n", mpi_depth);
		PRV_EVENT(50000002, 0);
	}
#endif
	if (comm->group->size == 1)	// e.g. comm_self
		goto mpi_barrier_exit;

#ifdef DEBUG_MPI
printf("MPI_Barrier entering... \n" );
#endif
	ffwd_barrier(comm->group->size, mpilite_barrier);
#ifdef DEBUG_MPI
printf("MPI_Barrier exit\n");
#endif

mpi_barrier_exit:
#if 0
	if (depth) {
		PRV_EVENT(50000002, 1);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_barrier, _cycle_count - start);
		_xadd64(&time_wait_barrier, mpi_waittime);
		_xadd64(&time_ceng_barrier, mpi_cengtime);
		_xadd64(&count_barrier, 1);
	}
#endif
	return MPI_SUCCESS;
}




// dirty quick hack
#ifdef MPILITE_REQUEST_SHADOWING
#define NEW_REQUEST	_request
#else
#define NEW_REQUEST	request
#endif


void print_type(MPI_Datatype index)
{
#ifdef DEBUG_MPI
	int i;
	struct datatype *type = datatype_table[index];
	printf("[%d] flag:0x%x count:%d size:(%d)%d ext:%d\n", index, type->flag, type->count, type->net_size, type->size, type->extent);
	for(i=0;i<type->count;i++) {
		printf("     type %d cnt %d at off %d\n", type->e[i].type, type->e[i].count, type->e[i].off);
	}
#endif
}


static void cleanup_refcnt(struct ffwd_message *p)
{
#ifdef MPILITE_REFCNT_MESSAGE
	int *extra = p->extra;
	int ret = __sync_fetch_and_add(extra, -1);
	if (ret == 1) {
		ffwd_free(extra);
		ffwd_free(p->buf);
	}
#else
	assert(0 && "extra must be NULL\n");
#endif
}


static void do_copy(struct ffwd_message *send, struct ffwd_message *recv, char *s)
{
#if 0
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		PRV_EVENT(50000003, 6);
	}
	mpi_depth++;
#endif
	if (send->count > recv->count) {
		assert(0 && "Isend: too small receive buffer, probably app bug -- want to just truncate?\n");
	}
	if (send->tag != MPI_ANY_TAG && recv->tag != MPI_ANY_TAG && send->tag != recv->tag)
		printf("do_copy: WARN: tag different %d != %d\n", send->tag, recv->tag);
	if (send->datatype != recv->datatype) {
		printf("do_copy: WARN: datatype different %d != %d\n", send->datatype, recv->datatype);
		print_type(send->datatype);
		print_type(recv->datatype);
	}
	assert(send->datatype < MAX_DATATYPE);
	assert(recv->datatype < MAX_DATATYPE);

	char *sendbuf, *recvbuf;
	sendbuf = send->buf;
	recvbuf = recv->buf;
	struct datatype *type = datatype_table[send->datatype];
	if (type->flag & MPI_TYPEFLAG_VECTOR) {
		int extent = type->extent;
		struct datatype *type2 = datatype_table[type->e[0].type];
		int size = (type2->size * type->e[0].count);
		int count = type->count;
		//printf("str:%d size:%d count:%d\n", extent, size, count);
#if 0
		if (depth) {
			printf("[MPI] %d do_copy(vector) size %d from %p to %p, %s\n", mpi_depth-1, count*size, sendbuf, recvbuf, s);
		}
#endif
		int i;
		for(i=0;i<count;i++) {
			memcpy(recvbuf , sendbuf, size);
			recvbuf += extent;
			sendbuf += extent;
		}
	} else {
		if (!(type->flag & MPI_TYPEFLAG_PACKED)) {
			printf("WARN: operation on non-contig datatype\n");
			assert(0 && "TODO: operation on non-contig datatype copy");
		}
		// use sender's count and datatype
		// one of buf would be local, while other one would be global address
		int size = send->count*datatype_size(send->datatype);
#if 0
		if (depth) {
			printf("[MPI] %d do_copy size %d from %p to %p, %s\n", mpi_depth-1, size, sendbuf, recvbuf, s);
		}
#endif
		memcpy(recvbuf, sendbuf, size);
	}

	if (send->count < recv->count) {	// This is normal. Recv receives AT MOST 'count' elements
#ifdef DEBUG_MPI
		printf("INFO: received only %d count, max:%d\n", send->count, recv->count);
#endif
		recv->count = send->count;	// let the receiver know how many 'count' received.
	}

	__sync_synchronize();
//	_fence_store();	        // make sure memcpy has completed; if
				// changed to ffwd_meng_copy, need a meng fence

	ffwd_mq_done(send, recv);	// signal the receiver and sender
#if 0
	mpi_depth--;
	if (depth) {
		PRV_EVENT(50000003, 7);
	}
#endif
}

static void _works_in_wait(struct ffwd_message *request)
{
#ifdef MPILITE_INTERMEDIATE_BUFFER
	if (request->status == FFWD_MQ_RECV_MATCHED) {
//		printf("pull from wait, sender:%p,%p me:%p,%p \n", request->prev, request->prev->buf, request, request->buf);
		do_copy(request->prev /* sender's request */, request, "pull from wait");
		assert(request->prev->flag & FFWD_MQF_INTERMEDIATE);	// always it's intermediate buffer
		if (request->prev->extra) {
			cleanup_refcnt(request->prev);
		}
		ffwd_mq_free(request->prev);	// free intermediate buffer
		request->prev = NULL;		// delete reference just in case.
	}
#endif
}

static void _do_status(MPI_Status *status, struct ffwd_message *request)
{
	if (status != MPI_STATUS_IGNORE) {
		status->mq_status.count    = request->count;
		status->mq_status.datatype = request->datatype;
		status->mq_status.tag      = request->tag;
		status->mq_status.source   = request->gpid;	// TODO: what if req->rank is dest??
	}
}



int MPI_Iprobe(int source, int tag, MPI_Comm comm, int *flag, MPI_Status *status)
{
#if 0
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Iprobe\n", mpi_depth);
	}
	mpi_depth++;
#endif
	assert(tag <= MPILITE_TAG_MAX);
	int count, datatype, source_gpid;
	struct ffwd_message *p = ffwd_mq_probe(rank_to_gpid(comm, source), tag, &count, &datatype, &source_gpid, 0);
	*flag = (p != NULL);
	if (p && status != MPI_STATUS_IGNORE) {
		status->mq_status.count = count;
		status->mq_status.datatype = datatype;
		if (source == MPI_ANY_SOURCE) {
			status->mq_status.source = gpid_to_rank(comm, source_gpid);
		} else {
			status->mq_status.source = source;
		}
		status->mq_status.tag = tag;
	}
#if 0
	mpi_depth--;
	if (depth) {
		printf("[MPI] %d Iprobe done\n", mpi_depth);
		printf("[PRV]1:%d:1:%d:%d:%ld:%ld:%d\n", (FFWD_GLOBAL_PID+1), (FFWD_GLOBAL_PID+1), FlixCurrentThread()->index_in_process+1 , start, _cycle_count, 8);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_probe, _cycle_count - start);
		_xadd64(&count_probe, 1);
	}
#endif
	return MPI_SUCCESS;
}


int MPI_Probe(int source, int tag, MPI_Comm comm, MPI_Status *status)
{
#if 0
	mpi_waittime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Probe\n", mpi_depth);
	}
	mpi_depth++;
#endif
	assert(tag <= MPILITE_TAG_MAX);
	int count, datatype, source_gpid;
	struct ffwd_message *p;
#if 0
	if (mpmode_info) {
		long start_wait = _cycle_count;
		p = ffwd_mq_probe(rank_to_gpid(comm, source), tag, &count, &datatype, &source_gpid, 1);
		_xadd64(&mpi_waittime, _cycle_count - start_wait);
	} else {
		p = ffwd_mq_probe(rank_to_gpid(comm, source), tag, &count, &datatype, &source_gpid, 1);
	}
#else
	p = ffwd_mq_probe(rank_to_gpid(comm, source), tag, &count, &datatype, &source_gpid, 1);
#endif

	if (p && status != MPI_STATUS_IGNORE) {
		status->mq_status.count = count;
		status->mq_status.datatype = datatype;
		if (source == MPI_ANY_SOURCE) {
			status->mq_status.source = gpid_to_rank(comm, source_gpid);
		} else {
			status->mq_status.source = source;
		}
		status->mq_status.tag = tag;
	}
#if 0
	mpi_depth--;
	if (depth) {
		printf("[MPI] %d Probe done\n", mpi_depth);
		printf("[PRV]1:%d:1:%d:%d:%ld:%ld:%d\n", (FFWD_GLOBAL_PID+1), (FFWD_GLOBAL_PID+1), FlixCurrentThread()->index_in_process+1 , start, _cycle_count, 8);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_probe, _cycle_count - start);
		_xadd64(&time_wait_probe, mpi_waittime);
		_xadd64(&count_probe, 1);
	}
#endif
	return MPI_SUCCESS;
}

#ifdef MPILITE_TEMP
int MPI_Test(MPI_Request *NEW_REQUEST, int *flag, MPI_Status *status)	// MPI_Wait() still should be called to cleanup! MPI_Test() doesn't do cleanup
{
#if 0
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Test\n", mpi_depth);
	}
	mpi_depth++;
#endif
#ifndef MPILITE_THREAD_MODEL
	struct ffwd_message *request = *_request;
#endif
	if (request == MPI_REQUEST_NULL)
		return error_handling(MPI_ERR_REQUEST); // unlike wait, here we return err
	if (request->status >= FFWD_MQ_MAX || request->status <= FFWD_MQ_MIN) {
		printf("WARN MPI_Test invalid request status?\n");
	}
	*flag = 0;
	_works_in_wait(request);
//printf("_req = %p , request = %p , status=%d in test\n", _request, request, request->status);
	if (request->status == FFWD_MQ_DONE) {
		*flag = 1;
		_do_status(status, request);
		*_request = MPI_REQUEST_NULL;
		ffwd_mq_free(request);
	}
#if 0
	mpi_depth--;
	if (depth) {
//		printf("[MPI] %d Test done\n", mpi_depth);
		printf("[PRV]1:%d:1:%d:%d:%ld:%ld:%d\n", (FFWD_GLOBAL_PID+1), (FFWD_GLOBAL_PID+1), FlixCurrentThread()->index_in_process+1 , start, _cycle_count, 8);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_test, _cycle_count - start);
		_xadd64(&count_test, 1);
	}
#endif
	return MPI_SUCCESS;
}

int MPI_Testall(int count, MPI_Request *array_of_requests, int *flag,
    MPI_Status *array_of_statuses)
{
#if 0
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Testall cnt %d\n", mpi_depth, count);
		PRV_EVENT(50000002, 2);
	}
	mpi_depth++;
#endif
	int i;
	int temp;
#ifdef DEBUG_MPI
	printf("testall: %d aor %p aos %p\n", count, array_of_requests, array_of_statuses);
#endif
	*flag = 1;	// True if all requests have completed; false otherwise (logical)
	if (array_of_statuses == MPI_STATUSES_IGNORE)
		for(i=0;i<count;i++) {
			MPI_Test(&array_of_requests[i], &temp, MPI_STATUS_IGNORE);
			*flag = *flag && temp;
		}
	else
		for(i=0;i<count;i++) {
			MPI_Test(&array_of_requests[i], &temp, &array_of_statuses[i]);
			*flag = *flag && temp;
		}
#if 0
	mpi_depth--;
	if (depth) {
		PRV_EVENT(50000002, 3);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_test, _cycle_count - start);
	}
#endif
	return MPI_SUCCESS;
}
#endif

int MPI_Wait(MPI_Request *NEW_REQUEST, MPI_Status *status)
{
#if 0
	mpi_waittime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Wait\n", mpi_depth);
	}
	mpi_depth++;
#endif
#if 0
if (*_request == NULL) {
printf("_request == NULL\n");
} else {
printf("_request != NULL\n");
}
#endif
#ifdef MPILITE_REQUEST_SHADOWING
	struct ffwd_message *request = *_request;
#endif
#ifdef DEBUG_MPI
	printf("MPI_Wait: req:%p\n", request);
	if (request)
		printf("MPI_Wait: request->status:%d, status:%p\n", request->status, status);
        printf("MPI_Wait: Calling ffwd_mq_wait\n");
#endif
	if (request == NULL)
		return MPI_SUCCESS;
#ifdef MPILITE_ASYNC_COLLECTIVE
	if (request->flag == FFWD_MQF_REQUEST_SET) {
		MPI_Request *reqs = request->buf;
		MPI_Waitall(request->tag /* count */, reqs, MPI_STATUSES_IGNORE);
		// TODO: status?
		*_request = NULL;
		free(request);
		return MPI_SUCCESS;
	}
#endif

	int ret_wait;
#if 0
	if (mpmode_info) {
		long start_wait = _cycle_count;
		ret_wait = ffwd_mq_wait(request);
		_xadd64(&mpi_waittime, _cycle_count - start_wait);
	} else {
		ret_wait = ffwd_mq_wait(request);
	}
#else
	ret_wait = ffwd_mq_wait(request);
#endif
	if (!ret_wait) {	// if not MPI_REQUEST_NULL
		_works_in_wait(request);
		_do_status(status, request);
	}
#ifdef MPILITE_REQUEST_SHADOWING
	*_request = MPI_REQUEST_NULL;
	ffwd_mq_free(request);
#endif
#if 0
	mpi_depth--;
	if (depth) {
		printf("[MPI] %d Wait done\n", mpi_depth);
		printf("[PRV]1:%d:1:%d:%d:%ld:%ld:%d\n", (FFWD_GLOBAL_PID+1), (FFWD_GLOBAL_PID+1), FlixCurrentThread()->index_in_process+1 , start, _cycle_count, 8);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_wait, _cycle_count - start);
		_xadd64(&time_wait_wait, mpi_waittime);
		_xadd64(&count_wait, 1);
	}
#endif
	return MPI_SUCCESS;
}

int MPI_Waitall(int count, MPI_Request *array_of_requests,
    MPI_Status *array_of_statuses)
{
#if 0
	mpi_waittime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Waitall cnt %d\n", mpi_depth, count);
		PRV_EVENT(50000002, 2);
	}
	mpi_depth++;
#endif
	int i;
#ifdef DEBUG_MPI
	printf("waitall: %d aor %p aos %p\n", count, array_of_requests, array_of_statuses);
#endif
	if (array_of_statuses == MPI_STATUSES_IGNORE)
		for(i=0;i<count;i++) {
			MPI_Wait(&array_of_requests[i], MPI_STATUS_IGNORE);
		}
	else
		for(i=0;i<count;i++) {
			MPI_Wait(&array_of_requests[i], &array_of_statuses[i]);
		}
#if 0
	mpi_depth--;
	if (depth) {
		PRV_EVENT(50000002, 3);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_wait, _cycle_count - start);
		_xadd64(&time_wait_wait, mpi_waittime);
	}
#endif
	return MPI_SUCCESS;
}

#ifdef MPILITE_FORTRAN
static int MPI_Waitall_fortran(int count, int /*MPI_Request*/ *_array_of_requests,
    MPI_Status *array_of_statuses)
{
	mpi_waittime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Waitall(fort)\n", mpi_depth);
		PRV_EVENT(50000002, 4);
	}
	mpi_depth++;
	int i;
#ifdef DEBUG_MPI
	printf("waitall: %d aor(int*) %p aos %p\n", count, _array_of_requests, array_of_statuses);
#endif
	if (array_of_statuses == MPI_STATUSES_IGNORE)
		for(i=0;i<count;i++) {
			MPI_Request *req = (MPI_Request *)_array_of_requests[i];// int->pointer casting..how do I remove warning?
			MPI_Wait(req, MPI_STATUS_IGNORE);
			free(req);
		}
	else
		for(i=0;i<count;i++) {
			MPI_Request *req = (MPI_Request *)_array_of_requests[i];// int->pointer casting.. how do I remove warning?
			MPI_Wait(req, &array_of_statuses[i]);
			free(req);
		}
	mpi_depth--;
	if (depth) {
		PRV_EVENT(50000002, 5);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_wait, _cycle_count - start);
		_xadd64(&time_wait_wait, mpi_waittime);
	}
	return MPI_SUCCESS;
}
#endif

int MPI_Sendrecv(void *sendbuf, int sendcount, MPI_Datatype sendtype,
                int dest, int sendtag,
                void *recvbuf, int recvcount, MPI_Datatype recvtype,
                int source, int recvtag,
                MPI_Comm comm, MPI_Status *status)
{
#if 0
	mpi_waittime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Sendrecv\n", mpi_depth);
		PRV_EVENT(50000001, 0);
	}
	mpi_depth++;
#endif
	MPI_Request r1, r2;
	MPI_Isend(sendbuf, sendcount, sendtype, dest, sendtag, comm, &r1);
	MPI_Irecv(recvbuf, recvcount, recvtype, source, recvtag, comm, &r2);
	MPI_Wait(&r1, MPI_STATUS_IGNORE);
	MPI_Wait(&r2, status);
#if 0
	mpi_depth--;
	if (depth) {
		PRV_EVENT(50000001, 1);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_sendrecv, _cycle_count - start);
		_xadd64(&time_wait_sendrecv, mpi_waittime);
		_xadd64(&count_sendrecv, 1);
	int size = sendcount*datatype_size(sendtype);
	if (size != -1) {
		_xadd64(&size_p2p_sum , size);
		_xinc64(&count_p2p_sum , 0 /* disp */);
		if (size < 64) {
			_xadd64(&size_p2p_64 , size);
			_xinc64(&count_p2p_64 , 0 /* disp */);
		} else if (size < 256) {
			_xadd64(&size_p2p_256, size);
			_xinc64(&count_p2p_256, 0 /* disp */);
		} else if (size < 1024) {
			_xadd64(&size_p2p_1k , size);
			_xinc64(&count_p2p_1k , 0 /* disp */);
		} else if (size < 4*1024) {
			_xadd64(&size_p2p_4k , size);
			_xinc64(&count_p2p_4k , 0 /* disp */);
		} else if (size < 16*1024) {
			_xadd64(&size_p2p_16k, size);
			_xinc64(&count_p2p_16k, 0 /* disp */);
		} else if (size < 64*1024) {
			_xadd64(&size_p2p_64k, size);
			_xinc64(&count_p2p_64k, 0 /* disp */);
		} else {
			_xadd64(&size_p2p_big, size);
			_xinc64(&count_p2p_big, 0 /* disp */);
		}
	}
	}
#endif
	return MPI_SUCCESS;
}


#ifdef MPILITE_REQUEST_SHADOWING
static inline void init_request(MPI_Request p, int count, int datatype, int tag, int gpid, void *buf)
#else
static inline void init_request(MPI_Request *p, int count, int datatype, int tag, int gpid, void *buf)
#endif
{
	p->count = count;
	p->datatype = datatype;
	p->tag = tag;
	p->gpid = gpid;
	p->flag = 0;
	p->status = FFWD_MQ_INIT;
	p->buf = buf;
	p->extra = NULL;
	p->prev = p->next = NULL;
}
#if 0

static __inline long fq_deq(void* queue) {
    long val;
    __asm volatile("qeng_deq %1, %0" : "=r"(val) : "r"(queue));
    return val;
}


#endif

int get_size_if_packed(int count, int datatype)
{
	struct datatype *type = datatype_table[datatype];
	if (!(type->flag & MPI_TYPEFLAG_PACKED)) {
		printf("INFO operation on non-contig datatype? fallback.\n");
		return -1;
	}
#if 0	// this seems unnecessary..
	if (type->flag & MPI_TYPEFLAG_VECTOR) {
		printf("INFO vector..fallback..\n");
		return -1;
	}
#endif
	int size = count*datatype_size(datatype);
	return size;
}

#ifdef MPILITE_REFCNT_MESSAGE
int MPI_Isend2(const void *buf, int *extra, int count, MPI_Datatype datatype, int dest, int tag,
              MPI_Comm comm, MPI_Request *NEW_REQUEST)
{
#if 0
	mpi_waittime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Isend to rank %d tag 0x%x\n", mpi_depth, dest, tag);
	}
	mpi_depth++;
#endif
	assert(dest < comm->group->size);
	assert(datatype < MAX_DATATYPE);
	assert(tag <= MPILITE_TAG_MAX);

	struct ffwd_message *ret = NULL;
	int size = get_size_if_packed(count, datatype);
#ifdef MPILITE_REQUEST_SHADOWING
	struct ffwd_message *request = ffwd_mq_alloc(0);
	*_request = request;
#endif
	init_request(request, count, datatype, tag, FFWD_GLOBAL_PID /* source.. mark it's from me */, (void *)buf );
#ifdef MPILITE_INTERMEDIATE_BUFFER
	assert(size != -1);	// TODO
	struct ffwd_message *intermediate = ffwd_mq_alloc(0);
//printf("intermediate %p alloc\n", intermediate);
	init_request(intermediate, count, datatype, tag, FFWD_GLOBAL_PID /* source */,
		(void *)buf	// this will be used in the receiver's wait to pull data
		);
	intermediate->flag = FFWD_MQF_INTERMEDIATE;	// mark as intermediate
	intermediate->extra = extra;
//	do_copy(request, intermediate, "copy to scratchpad");	// this marks both DONE
	__sync_synchronize();	// _fence_memory();
	request->status = FFWD_MQ_DONE;
	request = intermediate;	// override 'request' var
#endif

#ifdef DEBUG_MPI
        printf("MPI_Isend: Calling ffwd_mq_send\n");
#endif
	ret = ffwd_mq_send((void *)buf, count, datatype, rank_to_gpid(comm,dest), tag, request);
	if (ret) {
		ret->prev = request;
		__sync_synchronize();	//_fence_store();
#ifndef MPILITE_INTERMEDIATE_BUFFER
		do_copy(request, ret, "push (send copies to recvbuff)");	// push doesn't make sense on ffwd
#else
		// push doesn't work in this case. Recv will pull in its wait function.
		ret->status = FFWD_MQ_RECV_MATCHED;	// signal
#ifdef DEBUG_MPI
		printf("pull_only:Isend:set status to recv_matched\n");
#endif
#endif
	}
#if 0
	mpi_depth--;
	if (depth) {
		printf("[PRV]1:%d:1:%d:%d:%ld:%ld:%d\n", (FFWD_GLOBAL_PID+1), (FFWD_GLOBAL_PID+1),FlixCurrentThread()->index_in_process+1 , start, _cycle_count, 10);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_isend, _cycle_count - start);
//		_xadd64(&time_wait_isend, mpi_waittime);
		_xadd64(&count_isend, 1);
	if (size != -1) {
		_xadd64(&size_p2p_sum , size);
		_xinc64(&count_p2p_sum , 0 /* disp */);
		if (size < 64) {
			_xadd64(&size_p2p_64 , size);
			_xinc64(&count_p2p_64 , 0 /* disp */);
		} else if (size < 256) {
			_xadd64(&size_p2p_256, size);
			_xinc64(&count_p2p_256, 0 /* disp */);
		} else if (size < 1024) {
			_xadd64(&size_p2p_1k , size);
			_xinc64(&count_p2p_1k , 0 /* disp */);
		} else if (size < 4*1024) {
			_xadd64(&size_p2p_4k , size);
			_xinc64(&count_p2p_4k , 0 /* disp */);
		} else if (size < 16*1024) {
			_xadd64(&size_p2p_16k, size);
			_xinc64(&count_p2p_16k, 0 /* disp */);
		} else if (size < 64*1024) {
			_xadd64(&size_p2p_64k, size);
			_xinc64(&count_p2p_64k, 0 /* disp */);
		} else {
			_xadd64(&size_p2p_big, size);
			_xinc64(&count_p2p_big, 0 /* disp */);
		}
	}
	}
#endif
	return MPI_SUCCESS;
}
#endif

int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
              MPI_Comm comm, MPI_Request *NEW_REQUEST)
{
#if 0
	mpi_waittime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Isend to rank %d tag 0x%x\n", mpi_depth, dest, tag);
	}
	mpi_depth++;
#endif
	assert(dest < comm->group->size);
	assert(datatype < MAX_DATATYPE);
	assert(tag <= MPILITE_TAG_MAX);

	struct ffwd_message *ret = NULL;
	int size = get_size_if_packed(count, datatype);
#ifdef MPILITE_REQUEST_SHADOWING
	struct ffwd_message *request = ffwd_mq_alloc(0);
	*_request = request;
#endif
	init_request(request, count, datatype, tag, FFWD_GLOBAL_PID /* source.. mark it's from me */, (void *)buf );
#ifdef MPILITE_INTERMEDIATE_BUFFER
	assert(size != -1);	// TODO
	struct ffwd_message *intermediate = ffwd_mq_alloc(size);
//printf("intermediate %p alloc\n", intermediate);
	init_request(intermediate, count, datatype, tag, FFWD_GLOBAL_PID /* source */,
		intermediate->embed	// this will be used in the receiver's wait to pull data
		);
	intermediate->flag = FFWD_MQF_INTERMEDIATE;	// mark as intermediate
//	do_copy(request, intermediate, "copy to scratchpad");	// this marks both DONE
	memcpy(intermediate->embed, buf, size);
	__sync_synchronize();	// _fence_memory();
	request->status = FFWD_MQ_DONE;
	request = intermediate;	// override 'request' var
#endif

#ifdef DEBUG_MPI
        printf("MPI_Isend: Calling ffwd_mq_send\n");
#endif
	ret = ffwd_mq_send((void *)buf, count, datatype, rank_to_gpid(comm,dest), tag, request);
	if (ret) {
		ret->prev = request;
		__sync_synchronize();	//_fence_store();
#ifndef MPILITE_INTERMEDIATE_BUFFER
		do_copy(request, ret, "push (send copies to recvbuff)");	// push doesn't make sense on ffwd
#else
		// push doesn't work in this case. Recv will pull in its wait function.
		ret->status = FFWD_MQ_RECV_MATCHED;	// signal
#ifdef DEBUG_MPI
		printf("pull_only:Isend:set status to recv_matched\n");
#endif
#endif
	}
#if 0
	mpi_depth--;
	if (depth) {
		printf("[PRV]1:%d:1:%d:%d:%ld:%ld:%d\n", (FFWD_GLOBAL_PID+1), (FFWD_GLOBAL_PID+1),FlixCurrentThread()->index_in_process+1 , start, _cycle_count, 10);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_isend, _cycle_count - start);
//		_xadd64(&time_wait_isend, mpi_waittime);
		_xadd64(&count_isend, 1);
	if (size != -1) {
		_xadd64(&size_p2p_sum , size);
		_xinc64(&count_p2p_sum , 0 /* disp */);
		if (size < 64) {
			_xadd64(&size_p2p_64 , size);
			_xinc64(&count_p2p_64 , 0 /* disp */);
		} else if (size < 256) {
			_xadd64(&size_p2p_256, size);
			_xinc64(&count_p2p_256, 0 /* disp */);
		} else if (size < 1024) {
			_xadd64(&size_p2p_1k , size);
			_xinc64(&count_p2p_1k , 0 /* disp */);
		} else if (size < 4*1024) {
			_xadd64(&size_p2p_4k , size);
			_xinc64(&count_p2p_4k , 0 /* disp */);
		} else if (size < 16*1024) {
			_xadd64(&size_p2p_16k, size);
			_xinc64(&count_p2p_16k, 0 /* disp */);
		} else if (size < 64*1024) {
			_xadd64(&size_p2p_64k, size);
			_xinc64(&count_p2p_64k, 0 /* disp */);
		} else {
			_xadd64(&size_p2p_big, size);
			_xinc64(&count_p2p_big, 0 /* disp */);
		}
	}
	}
#endif
	return MPI_SUCCESS;
}



int MPI_Send(const void *buf,int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
#if 0
	mpi_waittime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Send to rank %d tag 0x%x\n", mpi_depth, dest, tag);
		PRV_EVENT(50000001, 2);
	}
	mpi_depth++;
#endif
	MPI_Request req;
#ifdef DEBUG_MPI
        printf("MPI_Send: Calling MPI_Isend 0\n");
#endif
	MPI_Isend(buf, count, datatype, dest, tag, comm, &req);
#ifdef DEBUG_MPI
        printf("MPI_Send: Calling MPI_Wait 0\n");
#endif
	MPI_Wait(&req, MPI_STATUS_IGNORE);
#ifdef DEBUG_MPI
        printf("MPI_Send: Completed MPI_Wait\n");
#endif
#if 0
	mpi_depth--;
	if (depth) {
		PRV_EVENT(50000001, 3);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_send, _cycle_count - start);
		_xadd64(&time_wait_send, mpi_waittime);
		_xadd64(&count_send, 1);
	int size = count*datatype_size(datatype);
	if (size != -1) {
		_xadd64(&size_p2p_sum , size);
		_xinc64(&count_p2p_sum , 0 /* disp */);
		if (size < 64) {
			_xadd64(&size_p2p_64 , size);
			_xinc64(&count_p2p_64 , 0 /* disp */);
		} else if (size < 256) {
			_xadd64(&size_p2p_256, size);
			_xinc64(&count_p2p_256, 0 /* disp */);
		} else if (size < 1024) {
			_xadd64(&size_p2p_1k , size);
			_xinc64(&count_p2p_1k , 0 /* disp */);
		} else if (size < 4*1024) {
			_xadd64(&size_p2p_4k , size);
			_xinc64(&count_p2p_4k , 0 /* disp */);
		} else if (size < 16*1024) {
			_xadd64(&size_p2p_16k, size);
			_xinc64(&count_p2p_16k, 0 /* disp */);
		} else if (size < 64*1024) {
			_xadd64(&size_p2p_64k, size);
			_xinc64(&count_p2p_64k, 0 /* disp */);
		} else {
			_xadd64(&size_p2p_big, size);
			_xinc64(&count_p2p_big, 0 /* disp */);
		}
	}
	}
#endif
	return MPI_SUCCESS;
}


int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source,
              int tag, MPI_Comm comm, MPI_Request *NEW_REQUEST)
{
#if 0
	mpi_waittime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Irecv from rank %d tag 0x%x\n", mpi_depth, source, tag);
	}
	mpi_depth++;
#endif
	assert(source < comm->group->size);
	assert(datatype < MAX_DATATYPE);
	assert(tag <= MPILITE_TAG_MAX);
#ifdef DEBUG_MPI
        printf("MPI_Irecv: Calling ffwd_mq_recv\n");
#endif
#ifdef MPILITE_REQUEST_SHADOWING
	struct ffwd_message *request = ffwd_mq_alloc(0);
	*_request = request;
#endif
	struct ffwd_message *ret;
	init_request(request, count, datatype, tag, rank_to_gpid(comm, source), buf );
	ret = ffwd_mq_recv(buf, count, datatype, rank_to_gpid(comm, source), tag, request);
	if (ret) {
		request->gpid = ret->gpid;	// in case of MPI_ANY_SOURCE, we need source ( ret->gpid )
		request->buf = buf;		// I think this is not necessary since it's already globalized
		do_copy(ret, request, "pull (recv copies from sendbuff)");	// this is pull (receiver copies from sender's buffer because receiver came later)
		if (ret->extra) {
			cleanup_refcnt(ret);
		}
#ifdef MPILITE_INTERMEDIATE_BUFFER
		assert(ret->flag & FFWD_MQF_INTERMEDIATE);	// always true.
		ffwd_mq_free(ret);	// free intermediate buffer
#endif
	}
irecv_exit:
#if 0
	mpi_depth--;
	if (depth) {
		printf("[PRV]1:%d:1:%d:%d:%ld:%ld:%d\n", (FFWD_GLOBAL_PID+1), (FFWD_GLOBAL_PID+1),FlixCurrentThread()->index_in_process+1 , start, _cycle_count, 11);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_irecv, _cycle_count - start);
//		_xadd64(&time_wait_irecv, mpi_waittime);
		_xadd64(&count_irecv, 1);
	}
#endif
	return MPI_SUCCESS;
}

int MPI_Recv(void *buf,int count, MPI_Datatype
datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
{
#if 0
	mpi_waittime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Recv from rank %d tag 0x%x\n", mpi_depth, source, tag);
		PRV_EVENT(50000001, 4);
	}
	mpi_depth++;
#endif
	MPI_Request req;
        // it's OK to recv from self because one may use Isend+Recv...
#ifdef DEBUG_MPI
        printf("MPI_Recv: Calling MPI_Irecv\n");
#endif

	MPI_Irecv(buf, count, datatype, source, tag, comm, &req);

#ifdef DEBUG_MPI
        printf("MPI_Recv: Calling MPI_Wait\n");
#endif

	MPI_Wait(&req, status);

#ifdef DEBUG_MPI
        printf("MPI_Recv: Completed MPI_Wait\n");
#endif
#if 0
	mpi_depth--;
	if (depth) {
		PRV_EVENT(50000001, 5);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_recv, _cycle_count - start);
		_xadd64(&time_wait_recv, mpi_waittime);
		_xadd64(&count_recv, 1);
	}
#endif
	return MPI_SUCCESS;
}


int MPI_Get_count(
               MPI_Status *status,
               MPI_Datatype datatype,
               int *count )
{
	if (datatype != status->mq_status.datatype)
		assert(0 && "warn datatype differs in get_count");
	if (status == MPI_STATUS_IGNORE) {
		assert(0 && "MPI_Get_count status IGNORE?");
	}
	*count = status->mq_status.count;
	return MPI_SUCCESS;
}




MPI_User_function *user_func_table[MPI_USER_FUNC_MAX];

int MPI_Op_create(MPI_User_function *function, int commute, MPI_Op *op)
{
	int i;
	for(i=0;i<MPI_USER_FUNC_MAX;i++) {
		if (user_func_table[i] == NULL) {
			user_func_table[i] = function;
			// TODO: commute?
			*op = i + MPI_USER_FUNC_BASE;
			return MPI_SUCCESS;
		}
	}
	return error_handling(MPI_ERR_INTERN);
}
int MPI_Op_free( MPI_Op *op)
{
	user_func_table[*op - MPI_USER_FUNC_BASE] = NULL;
	return MPI_SUCCESS;
}

int MPI_Scan(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
             MPI_Op op, MPI_Comm comm)
{
	printf("MPI_SCAN\n");
	assert(0 && "TODO MPI_Scan is missing at the moment\n");
}



#ifdef MPILITE_TEMP
int MPI_Reduce(void *sendbuf, void *recvbuf, int count,
    MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
{
#if 0
	mpi_waittime = 0;
	mpi_cengtime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
//		printf("[MPI] %d Reduce%s\n", mpi_depth, (mpmode3)?"(ceng)","");
		printf("[MPI] %d Reduce\n", mpi_depth);
		PRV_EVENT(50000002, 6);
	}
	mpi_depth++;
#endif
	if (sendbuf == MPI_IN_PLACE) {
		sendbuf = recvbuf;
	}
	if (root >= comm->group->size || root < 0) {
		return error_handling(MPI_ERR_ARG);
	}
	if (root == comm->group->rank) {
		int i, j;
		int size = count*datatype_size(datatype);
		memcpy(recvbuf, sendbuf, size);

		int max = comm->group->size;
		void *p = malloc(count*datatype_size(datatype));	// temporary buffer for recv
		if (!p)
			assert(0 && "temporary buffer allocation for MPI_Reduce failed.");
		if (op >= MPI_USER_FUNC_BASE) {
			printf("INFO reduce using user-defined function\n");
			for(i=0;i<max;i++) {
				if (i == root) {
					user_func_table[op-MPI_USER_FUNC_BASE](sendbuf, recvbuf, &count, &datatype);
				} else {
					MPI_Recv(p, count, datatype, i, i /*tag*/, comm, MPI_STATUS_IGNORE);
					// user defined function
					user_func_table[op-MPI_USER_FUNC_BASE](p, recvbuf, &count, &datatype);
				}
			}
		} else
		for(i=0;i<max;i++) {
			if (i == root)
				continue;
			MPI_Recv(p, count, datatype, i, i /*tag*/, comm, MPI_STATUS_IGNORE);

			if (datatype == MPI_INT) {
				int *a = recvbuf;
				int *b = p;
				if (op == MPI_SUM) {
					for(j=0;j<count;j++) {
						a[j] += b[j];
					}
				} else if (op == MPI_MIN) {
					for(j=0;j<count;j++) {
						a[j] = (a[j]>b[j]) ? b[j] : a[j];
					}
				} else if (op == MPI_MAX) {
					for(j=0;j<count;j++) {
						a[j] = (a[j]<b[j]) ? b[j] : a[j];
					}
// no MINLOC/MAXLOC
				} else if (op == MPI_PROD) {
					for(j=0;j<count;j++) {
						a[j] *= b[j];
					}
				} else if (op == MPI_LAND) {
					for(j=0;j<count;j++) {
						a[j] = a[j] && b[j];
					}
				} else if (op == MPI_BAND) {
					for(j=0;j<count;j++) {
						a[j] = a[j] & b[j];
					}
				} else if (op == MPI_LOR) {
					for(j=0;j<count;j++) {
						a[j] = a[j] || b[j];
					}
				} else if (op == MPI_BOR) {
					for(j=0;j<count;j++) {
						a[j] = a[j] | b[j];
					}
				} else if (op == MPI_LXOR) {
					for(j=0;j<count;j++) {
						a[j] = (a[j] && b[j]) || (!a[j] && !b[j]);
					}
				} else if (op == MPI_BXOR) {
					for(j=0;j<count;j++) {
						a[j] = a[j] ^ b[j];
					}
				} else {
					printf("not yet supported MPI_Op: %d, type: %d\n", op, datatype);
					assert(0 && "not yet supported MPI_Op/type");
				}
			} else if (datatype == MPI_LONG) {
				long *a = recvbuf;
				long *b = p;
				if (op == MPI_SUM) {
					for(j=0;j<count;j++) {
						a[j] += b[j];
					}
				} else if (op == MPI_MIN) {
					for(j=0;j<count;j++) {
						a[j] = (a[j]>b[j]) ? b[j] : a[j];
					}
				} else if (op == MPI_MAX) {
					for(j=0;j<count;j++) {
						a[j] = (a[j]<b[j]) ? b[j] : a[j];
					}
// no MINLOC/MAXLOC
				} else if (op == MPI_PROD) {
					for(j=0;j<count;j++) {
						a[j] *= b[j];
					}
				} else if (op == MPI_LAND) {
					for(j=0;j<count;j++) {
						a[j] = a[j] && b[j];
					}
				} else if (op == MPI_BAND) {
					for(j=0;j<count;j++) {
						a[j] = a[j] & b[j];
					}
				} else if (op == MPI_LOR) {
					for(j=0;j<count;j++) {
						a[j] = a[j] || b[j];
					}
				} else if (op == MPI_BOR) {
					for(j=0;j<count;j++) {
						a[j] = a[j] | b[j];
					}
				} else if (op == MPI_LXOR) {
					for(j=0;j<count;j++) {
						a[j] = (a[j] && b[j]) || (!a[j] && !b[j]);
					}
				} else if (op == MPI_BXOR) {
					for(j=0;j<count;j++) {
						a[j] = a[j] ^ b[j];
					}
				} else {
					printf("not yet supported MPI_Op: %d, type: %d\n", op, datatype);
					assert(0 && "not yet supported MPI_Op/type");
				}
			} else if (datatype == MPI_LONG_LONG || datatype == MPI_LONG_LONG_INT) {
				long long *a = recvbuf;
				long long *b = p;
				if (op == MPI_SUM) {
					for(j=0;j<count;j++) {
						a[j] += b[j];
					}
				} else if (op == MPI_MIN) {
					for(j=0;j<count;j++) {
						a[j] = (a[j]>b[j]) ? b[j] : a[j];
					}
				} else if (op == MPI_MAX) {
					for(j=0;j<count;j++) {
						a[j] = (a[j]<b[j]) ? b[j] : a[j];
					}
// no MINLOC/MAXLOC
				} else if (op == MPI_PROD) {
					for(j=0;j<count;j++) {
						a[j] *= b[j];
					}
				} else if (op == MPI_LAND) {
					for(j=0;j<count;j++) {
						a[j] = a[j] && b[j];
					}
				} else if (op == MPI_BAND) {
					for(j=0;j<count;j++) {
						a[j] = a[j] & b[j];
					}
				} else if (op == MPI_LOR) {
					for(j=0;j<count;j++) {
						a[j] = a[j] || b[j];
					}
				} else if (op == MPI_BOR) {
					for(j=0;j<count;j++) {
						a[j] = a[j] | b[j];
					}
				} else if (op == MPI_LXOR) {
					for(j=0;j<count;j++) {
						a[j] = (a[j] && b[j]) || (!a[j] && !b[j]);
					}
				} else if (op == MPI_BXOR) {
					for(j=0;j<count;j++) {
						a[j] = a[j] ^ b[j];
					}
				} else {
					printf("not yet supported MPI_Op: %d, type: %d\n", op, datatype);
					assert(0 && "not yet supported MPI_Op/type");
				}
			} else if (datatype == MPI_UNSIGNED) {
				unsigned *a = recvbuf;
				unsigned *b = p;
				if (op == MPI_SUM) {
					for(j=0;j<count;j++) {
						a[j] += b[j];
					}
				} else if (op == MPI_MIN) {
					for(j=0;j<count;j++) {
						a[j] = (a[j]>b[j]) ? b[j] : a[j];
					}
				} else if (op == MPI_MAX) {
					for(j=0;j<count;j++) {
						a[j] = (a[j]<b[j]) ? b[j] : a[j];
					}
// no MINLOC/MAXLOC
				} else if (op == MPI_PROD) {
					for(j=0;j<count;j++) {
						a[j] *= b[j];
					}
				} else if (op == MPI_LAND) {
					for(j=0;j<count;j++) {
						a[j] = a[j] && b[j];
					}
				} else if (op == MPI_BAND) {
					for(j=0;j<count;j++) {
						a[j] = a[j] & b[j];
					}
				} else if (op == MPI_LOR) {
					for(j=0;j<count;j++) {
						a[j] = a[j] || b[j];
					}
				} else if (op == MPI_BOR) {
					for(j=0;j<count;j++) {
						a[j] = a[j] | b[j];
					}
				} else if (op == MPI_LXOR) {
					for(j=0;j<count;j++) {
						a[j] = (a[j] && b[j]) || (!a[j] && !b[j]);
					}
				} else if (op == MPI_BXOR) {
					for(j=0;j<count;j++) {
						a[j] = a[j] ^ b[j];
					}
				} else {
					printf("not yet supported MPI_Op: %d, type: %d\n", op, datatype);
					assert(0 && "not yet supported MPI_Op/type");
				}
			} else if (datatype == MPI_UNSIGNED_LONG) {
				unsigned long *a = recvbuf;
				unsigned long *b = p;
				if (op == MPI_SUM) {
					for(j=0;j<count;j++) {
						a[j] += b[j];
					}
				} else if (op == MPI_MIN) {
					for(j=0;j<count;j++) {
						a[j] = (a[j]>b[j]) ? b[j] : a[j];
					}
				} else if (op == MPI_MAX) {
					for(j=0;j<count;j++) {
						a[j] = (a[j]<b[j]) ? b[j] : a[j];
					}
// no MINLOC/MAXLOC
				} else if (op == MPI_PROD) {
					for(j=0;j<count;j++) {
						a[j] *= b[j];
					}
				} else if (op == MPI_LAND) {
					for(j=0;j<count;j++) {
						a[j] = a[j] && b[j];
					}
				} else if (op == MPI_BAND) {
					for(j=0;j<count;j++) {
						a[j] = a[j] & b[j];
					}
				} else if (op == MPI_LOR) {
					for(j=0;j<count;j++) {
						a[j] = a[j] || b[j];
					}
				} else if (op == MPI_BOR) {
					for(j=0;j<count;j++) {
						a[j] = a[j] | b[j];
					}
				} else if (op == MPI_LXOR) {
					for(j=0;j<count;j++) {
						a[j] = (a[j] && b[j]) || (!a[j] && !b[j]);
					}
				} else if (op == MPI_BXOR) {
					for(j=0;j<count;j++) {
						a[j] = a[j] ^ b[j];
					}
				} else {
					printf("not yet supported MPI_Op: %d, type: %d\n", op, datatype);
					assert(0 && "not yet supported MPI_Op/type");
				}
			} else if (datatype == MPI_UNSIGNED_LONG_LONG) {
				unsigned long long *a = recvbuf;
				unsigned long long *b = p;
				if (op == MPI_SUM) {
					for(j=0;j<count;j++) {
						a[j] += b[j];
					}
				} else if (op == MPI_MIN) {
					for(j=0;j<count;j++) {
						a[j] = (a[j]>b[j]) ? b[j] : a[j];
					}
				} else if (op == MPI_MAX) {
					for(j=0;j<count;j++) {
						a[j] = (a[j]<b[j]) ? b[j] : a[j];
					}
// no MINLOC/MAXLOC
				} else if (op == MPI_PROD) {
					for(j=0;j<count;j++) {
						a[j] *= b[j];
					}
				} else if (op == MPI_LAND) {
					for(j=0;j<count;j++) {
						a[j] = a[j] && b[j];
					}
				} else if (op == MPI_BAND) {
					for(j=0;j<count;j++) {
						a[j] = a[j] & b[j];
					}
				} else if (op == MPI_LOR) {
					for(j=0;j<count;j++) {
						a[j] = a[j] || b[j];
					}
				} else if (op == MPI_BOR) {
					for(j=0;j<count;j++) {
						a[j] = a[j] | b[j];
					}
				} else if (op == MPI_LXOR) {
					for(j=0;j<count;j++) {
						a[j] = (a[j] && b[j]) || (!a[j] && !b[j]);
					}
				} else if (op == MPI_BXOR) {
					for(j=0;j<count;j++) {
						a[j] = a[j] ^ b[j];
					}
				} else {
					printf("not yet supported MPI_Op: %d, type: %d\n", op, datatype);
					assert(0 && "not yet supported MPI_Op/type");
				}
			} else if (datatype == MPI_UINT64_T) {
				uint64_t *a = recvbuf;
				uint64_t *b = p;
				if (op == MPI_SUM) {
					for(j=0;j<count;j++) {
						a[j] += b[j];
					}
				} else if (op == MPI_MIN) {
					for(j=0;j<count;j++) {
						a[j] = (a[j]>b[j]) ? b[j] : a[j];
					}
				} else if (op == MPI_MAX) {
					for(j=0;j<count;j++) {
						a[j] = (a[j]<b[j]) ? b[j] : a[j];
					}
// no MINLOC/MAXLOC
				} else if (op == MPI_PROD) {
					for(j=0;j<count;j++) {
						a[j] *= b[j];
					}
				} else if (op == MPI_LAND) {
					for(j=0;j<count;j++) {
						a[j] = a[j] && b[j];
					}
				} else if (op == MPI_BAND) {
					for(j=0;j<count;j++) {
						a[j] = a[j] & b[j];
					}
				} else if (op == MPI_LOR) {
					for(j=0;j<count;j++) {
						a[j] = a[j] || b[j];
					}
				} else if (op == MPI_BOR) {
					for(j=0;j<count;j++) {
						a[j] = a[j] | b[j];
					}
				} else if (op == MPI_LXOR) {
					for(j=0;j<count;j++) {
						a[j] = (a[j] && b[j]) || (!a[j] && !b[j]);
					}
				} else if (op == MPI_BXOR) {
					for(j=0;j<count;j++) {
						a[j] = a[j] ^ b[j];
					}
				} else {
					printf("not yet supported MPI_Op: %d, type: %d\n", op, datatype);
					assert(0 && "not yet supported MPI_Op/type");
				}
			} else if (datatype == MPI_FLOAT) {
				float *a = recvbuf;
				float *b = p;
				if (op == MPI_SUM) {
					for(j=0;j<count;j++) {
						a[j] += b[j];
					}
				} else if (op == MPI_MIN) {
					for(j=0;j<count;j++) {
						a[j] = (a[j]>b[j]) ? b[j] : a[j];
					}
				} else if (op == MPI_MAX) {
					for(j=0;j<count;j++) {
						a[j] = (a[j]<b[j]) ? b[j] : a[j];
					}
// no MINLOC/MAXLOC
				} else if (op == MPI_PROD) {
					for(j=0;j<count;j++) {
						a[j] *= b[j];
					}
// no LAND/BAND/LOR/BOR/LXOR/BXOR
				} else {
					printf("not yet supported MPI_Op: %d, type: %d\n", op, datatype);
					assert(0 && "not yet supported MPI_Op/type");
				}
			} else if (datatype == MPI_DOUBLE) {
				double *a = recvbuf;
				double *b = p;
				if (op == MPI_SUM) {
					for(j=0;j<count;j++) {
						a[j] += b[j];
					}
				} else if (op == MPI_MIN) {
					for(j=0;j<count;j++) {
						a[j] = (a[j]>b[j]) ? b[j] : a[j];
					}
				} else if (op == MPI_MAX) {
					for(j=0;j<count;j++) {
						a[j] = (a[j]<b[j]) ? b[j] : a[j];
					}
// no MINLOC/MAXLOC
				} else if (op == MPI_PROD) {
					for(j=0;j<count;j++) {
						a[j] *= b[j];
					}
// no LAND/BAND/LOR/BOR/LXOR/BXOR
				} else {
					printf("not yet supported MPI_Op: %d, type: %d\n", op, datatype);
					assert(0 && "not yet supported MPI_Op/type");
				}
			} else if (datatype == MPI_LONG_DOUBLE) {
				long double *a = recvbuf;
				long double *b = p;
				if (op == MPI_SUM) {
					for(j=0;j<count;j++) {
						a[j] += b[j];
					}
				} else if (op == MPI_MIN) {
					for(j=0;j<count;j++) {
						a[j] = (a[j]>b[j]) ? b[j] : a[j];
					}
				} else if (op == MPI_MAX) {
					for(j=0;j<count;j++) {
						a[j] = (a[j]<b[j]) ? b[j] : a[j];
					}
// no MINLOC/MAXLOC
				} else if (op == MPI_PROD) {
					for(j=0;j<count;j++) {
						a[j] *= b[j];
					}
// no LAND/BAND/LOR/BOR/LXOR/BXOR
				} else {
					printf("not yet supported MPI_Op: %d, type: %d\n", op, datatype);
					assert(0 && "not yet supported MPI_Op/type");
				}
			} else if (datatype == MPI_DOUBLE_INT) {
				double_int *a = recvbuf;
				double_int *b = p;
				if (op == MPI_MINLOC) {
					for(j=0;j<count;j++) {
						if (a[j].a > b[j].a) {
							a[j].a = b[j].a;
							a[j].b = b[j].b;
						} else if (a[j].a == b[j].a) {	// get min index if equals
							if (a[j].b > b[j].b)
								a[j].b = b[j].b;
						}
					}
				} else if (op == MPI_MAXLOC) {
					for(j=0;j<count;j++) {
						if (a[j].a < b[j].a) {
							a[j].a = b[j].a;
							a[j].b = b[j].b;
						} else if (a[j].a == b[j].a) {	// get min index if equal
							if (a[j].b > b[j].b)
								a[j].b = b[j].b;
						}
					}
				} else {
					printf("not yet supported MPI_Op: %d, type: %d\n", op, datatype);
					assert(0 && "not yet supported MPI_Op/type");
				}
			} else {
				printf("not yet supported MPI_type: %d\n", datatype);
				assert(0 && "not yet supported type");
			}
		}
		free(p);
	} else {
		MPI_Send(sendbuf, count, datatype, root, comm->group->rank, comm);
	}

mpi_reduce_exit:
#if 0
	mpi_depth--;
	if (depth) {
		PRV_EVENT(50000002, 7);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_reduce, _cycle_count - start);
		_xadd64(&time_wait_reduce, mpi_waittime);
		_xadd64(&time_ceng_reduce, mpi_cengtime);
		_xadd64(&count_reduce, 1);
	int size = count*datatype_size(datatype);
	if (size != -1) {
		_xadd64(&size_collective_sum , size);
		_xinc64(&count_collective_sum, 0 /* disp */);
		if (size < 64) {
			_xadd64(&size_collective_64 , size);
			_xinc64(&count_collective_64 , 0 /* disp */);
		} else if (size < 256) {
			_xadd64(&size_collective_256, size);
			_xinc64(&count_collective_256, 0 /* disp */);
		} else if (size < 1024) {
			_xadd64(&size_collective_1k , size);
			_xinc64(&count_collective_1k , 0 /* disp */);
		} else if (size < 4*1024) {
			_xadd64(&size_collective_4k , size);
			_xinc64(&count_collective_4k , 0 /* disp */);
		} else if (size < 16*1024) {
			_xadd64(&size_collective_16k, size);
			_xinc64(&count_collective_16k, 0 /* disp */);
		} else if (size < 64*1024) {
			_xadd64(&size_collective_64k, size);
			_xinc64(&count_collective_64k, 0 /* disp */);
		} else {
			_xadd64(&size_collective_big, size);
			_xinc64(&count_collective_big, 0 /* disp */);
		}
	}
	}
#endif
	return MPI_SUCCESS;
}


#define ALLREDUCE_ROOT	0
int MPI_Allreduce ( void *sendbuf, void *recvbuf, int count,
                          MPI_Datatype datatype, MPI_Op op, MPI_Comm comm )
{
#if 0
	mpi_waittime = 0;
	mpi_cengtime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Allreduce\n", mpi_depth);
		PRV_EVENT(50000002, 8);
	}
	mpi_depth++;
#endif
	if (sendbuf == MPI_IN_PLACE) {
		sendbuf = recvbuf;
	}

	MPI_Reduce(sendbuf, recvbuf, count, datatype, op, ALLREDUCE_ROOT, comm);
	MPI_Bcast(recvbuf, count, datatype, ALLREDUCE_ROOT, comm);
mpi_allreduce_exit:
#if 0
	mpi_depth--;
	if (depth) {
		PRV_EVENT(50000002, 9);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_allreduce, _cycle_count - start);
		_xadd64(&time_wait_allreduce, mpi_waittime);
		_xadd64(&time_ceng_allreduce, mpi_cengtime);
		_xadd64(&count_allreduce, 1);
	int size = count*datatype_size(datatype);
	if (size != -1) {
		_xadd64(&size_collective_sum , size);
		_xinc64(&count_collective_sum , 0 /* disp */);
		if (size < 64) {
			_xadd64(&size_collective_64 , size);
			_xinc64(&count_collective_64 , 0 /* disp */);
		} else if (size < 256) {
			_xadd64(&size_collective_256, size);
			_xinc64(&count_collective_256, 0 /* disp */);
		} else if (size < 1024) {
			_xadd64(&size_collective_1k , size);
			_xinc64(&count_collective_1k , 0 /* disp */);
		} else if (size < 4*1024) {
			_xadd64(&size_collective_4k , size);
			_xinc64(&count_collective_4k , 0 /* disp */);
		} else if (size < 16*1024) {
			_xadd64(&size_collective_16k, size);
			_xinc64(&count_collective_16k, 0 /* disp */);
		} else if (size < 64*1024) {
			_xadd64(&size_collective_64k, size);
			_xinc64(&count_collective_64k, 0 /* disp */);
		} else {
			_xadd64(&size_collective_big, size);
			_xinc64(&count_collective_big, 0 /* disp */);
		}
	}
	}
#endif
	return MPI_SUCCESS;
}


MPI_Request *alloc_request_set(MPI_Request *_request, int count, MPI_Datatype type, int max )
{
	struct ffwd_message *request = malloc( sizeof(struct ffwd_message) + sizeof(MPI_Request)*max);
	assert(request);
	*_request = request;
	MPI_Request *req = (MPI_Request *)((unsigned long)request + sizeof(struct ffwd_message));
	init_request(request, count , type, max /*tag*/ , FFWD_GLOBAL_PID /* source */ , req);
	request->flag = FFWD_MQF_REQUEST_SET;
	return req;
}

#ifdef MPILITE_REFCNT_MESSAGE
int MPI_Ibcast(void *buffer, int count, MPI_Datatype datatype,
    int root, MPI_Comm comm, MPI_Request *NEW_REQUEST)
{
#if 0
#endif
	if (comm->group->size == 1)	// e.g. comm_self
		goto mpi_bcast_exit;
	//printf("buffer %p , count %d , type %d, root %d\n", buffer, count, datatype, root);
	if (root >= comm->group->size || root < 0) {
		return error_handling(MPI_ERR_ARG);
	}
	if (root == comm->group->rank) {
		int i;
		int max = comm->group->size;
//		MPI_Request req[max];
		MPI_Request *req = alloc_request_set(_request, count, datatype, max);

int size = get_size_if_packed(count, datatype);
assert(size != -1);
void *ret = ffwd_alloc(size);
memcpy(ret, buffer, size);

int *extra = ffwd_alloc(sizeof(int));
assert(extra);
*extra = max-1;

		for(i=0;i<max;i++) {
			if (i != root) {
				MPI_Isend2(ret, extra, count, datatype, i, i /* tag */, comm, &req[i]);
			} else {
				req[i] = MPI_REQUEST_NULL;
			}
		}
//		MPI_Waitall(max, req, MPI_STATUSES_IGNORE);
	} else {
		MPI_Irecv(buffer, count, datatype, root, comm->group->rank /* tag */, comm, NEW_REQUEST);
	}
mpi_bcast_exit:
#if 0
#endif
	return MPI_SUCCESS;
}
#else

int MPI_Ibcast(void *buffer, int count, MPI_Datatype datatype,
    int root, MPI_Comm comm, MPI_Request *NEW_REQUEST)
{
#if 0
#endif
	if (comm->group->size == 1)	// e.g. comm_self
		goto mpi_bcast_exit;
	//printf("buffer %p , count %d , type %d, root %d\n", buffer, count, datatype, root);
	if (root >= comm->group->size || root < 0) {
		return error_handling(MPI_ERR_ARG);
	}
	if (root == comm->group->rank) {
		int i;
		int max = comm->group->size;
//		MPI_Request req[max];
		MPI_Request *req = alloc_request_set(_request, count, datatype, max);

		for(i=0;i<max;i++) {
			if (i != root) {
				MPI_Isend(buffer, count, datatype, i, i /* tag */, comm, &req[i]);
			} else {
				req[i] = MPI_REQUEST_NULL;
			}
		}
//		MPI_Waitall(max, req, MPI_STATUSES_IGNORE);
	} else {
		MPI_Irecv(buffer, count, datatype, root, comm->group->rank /* tag */, comm, NEW_REQUEST);
	}
mpi_bcast_exit:
#if 0
#endif
	return MPI_SUCCESS;
}
#endif


int MPI_Bcast ( void *buffer, int count, MPI_Datatype datatype, int root,
                      MPI_Comm comm )
{
#if 0
	mpi_waittime = 0;
	mpi_cengtime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Bcast\n", mpi_depth);
		PRV_EVENT(50000002, 10);
	}
	mpi_depth++;
#endif
	if (comm->group->size == 1)	// e.g. comm_self
		goto mpi_bcast_exit;
	//printf("buffer %p , count %d , type %d, root %d\n", buffer, count, datatype, root);
	if (root >= comm->group->size || root < 0) {
		return error_handling(MPI_ERR_ARG);
	}
	if (root == comm->group->rank) {
		int i;
		int max = comm->group->size;
		MPI_Request req[max];
		for(i=0;i<max;i++) {
			if (i != root) {
				MPI_Isend(buffer, count, datatype, i, i /* tag */, comm, &req[i]);
			} else {
				req[i] = MPI_REQUEST_NULL;
			}
		}
		MPI_Waitall(max, req, MPI_STATUSES_IGNORE);
	} else {
		MPI_Recv(buffer, count, datatype, root, comm->group->rank /* tag */, comm, MPI_STATUS_IGNORE);
	}
mpi_bcast_exit:
#if 0
	mpi_depth--;
	if (depth) {
		PRV_EVENT(50000002, 11);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_bcast, _cycle_count - start);
		_xadd64(&time_wait_bcast, mpi_waittime);
		_xadd64(&time_ceng_bcast, mpi_cengtime);
		_xadd64(&count_bcast, 1);

	int size = count*datatype_size(datatype);
	if (size != -1) {
		_xadd64(&size_collective_sum , size);
		_xinc64(&count_collective_sum , 0 /* disp */);
		if (size < 64) {
			_xadd64(&size_collective_64 , size);
			_xinc64(&count_collective_64 , 0 /* disp */);
		} else if (size < 256) {
			_xadd64(&size_collective_256, size);
			_xinc64(&count_collective_256, 0 /* disp */);
		} else if (size < 1024) {
			_xadd64(&size_collective_1k , size);
			_xinc64(&count_collective_1k , 0 /* disp */);
		} else if (size < 4*1024) {
			_xadd64(&size_collective_4k , size);
			_xinc64(&count_collective_4k , 0 /* disp */);
		} else if (size < 16*1024) {
			_xadd64(&size_collective_16k, size);
			_xinc64(&count_collective_16k, 0 /* disp */);
		} else if (size < 64*1024) {
			_xadd64(&size_collective_64k, size);
			_xinc64(&count_collective_64k, 0 /* disp */);
		} else {
			_xadd64(&size_collective_big, size);
			_xinc64(&count_collective_big, 0 /* disp */);
		}
	}
	}
#endif
	return MPI_SUCCESS;
}

int MPI_Scatterv(const void *sendbuf, const int *sendcounts, const int *displs,
                 MPI_Datatype sendtype, void *recvbuf, int recvcount,
                 MPI_Datatype recvtype,
                 int root, MPI_Comm comm)
{
#if 0
	mpi_waittime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Scatterv\n", mpi_depth);
		PRV_EVENT(50000002, 12);
	}
	mpi_depth++;
#endif
	int recvsize = recvcount*datatype_size(recvtype);
	int _ssize   =           datatype_size(sendtype);
/*	if (recvsize < sendsize) {
		ffwd_exit("scatter:recvbuff < sendbuff?\n");
	}*/
	if (root >= comm->group->size || root < 0) {
		return error_handling(MPI_ERR_ARG);
	}
	if (root == comm->group->rank) {
		int i, j;

		void *target = (char *)recvbuf;
		memcpy(target, sendbuf + _ssize * displs[root], _ssize * sendcounts[root]);
		int max = comm->group->size;
		for(i=0;i<max;i++) {
			if (i == root)
				continue;
			void *p = (char *)sendbuf + _ssize * displs[i];
			MPI_Send(p, sendcounts[i], sendtype, i, i /*tag*/, comm);
		}
	} else {
		MPI_Recv(recvbuf, recvcount, recvtype, root, comm->group->rank, comm, MPI_STATUS_IGNORE);
	}
#if 0
	mpi_depth--;
	if (depth) {
		PRV_EVENT(50000002, 13);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_scatter, _cycle_count - start);
		_xadd64(&time_wait_scatter, mpi_waittime);
		_xadd64(&count_scatter, 1);
#if 0	// TODO: need fix
	int size = sendcount*datatype_size(sendtype);
	if (size != -1) {
		_xadd64(&size_collective_sum , size);
		_xinc64(&count_collective_sum , 0 /* disp */);
		if (size < 64) {
			_xadd64(&size_collective_64 , size);
			_xinc64(&count_collective_64 , 0 /* disp */);
		} else if (size < 256) {
			_xadd64(&size_collective_256, size);
			_xinc64(&count_collective_256, 0 /* disp */);
		} else if (size < 1024) {
			_xadd64(&size_collective_1k , size);
			_xinc64(&count_collective_1k , 0 /* disp */);
		} else if (size < 4*1024) {
			_xadd64(&size_collective_4k , size);
			_xinc64(&count_collective_4k , 0 /* disp */);
		} else if (size < 16*1024) {
			_xadd64(&size_collective_16k, size);
			_xinc64(&count_collective_16k, 0 /* disp */);
		} else if (size < 64*1024) {
			_xadd64(&size_collective_64k, size);
			_xinc64(&count_collective_64k, 0 /* disp */);
		} else {
			_xadd64(&size_collective_big, size);
			_xinc64(&count_collective_big, 0 /* disp */);
		}
	}
#endif
	}
#endif
	return MPI_SUCCESS;
}



int MPI_Gatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                void *recvbuf, const int *recvcounts, const int *displs,
                MPI_Datatype recvtype, int root, MPI_Comm comm)
{
#if 0
	mpi_waittime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Gatherv\n", mpi_depth);
		PRV_EVENT(50000002, 12);
	}
	mpi_depth++;
#endif
	int _rsize   =           datatype_size(recvtype);
	int sendsize = sendcount*datatype_size(sendtype);
/*	if (recvsize < sendsize) {
		ffwd_exit("gather:recvbuff < sendbuff?\n");
	}*/
	if (root >= comm->group->size || root < 0) {
		return error_handling(MPI_ERR_ARG);
	}
	if (root == comm->group->rank) {
		int i, j;
		void *target = (char *)recvbuf + _rsize * displs[root];
		memcpy(target, sendbuf, sendsize);

		int max = comm->group->size;
		for(i=0;i<max;i++) {
			if (i == root)
				continue;
			void *p = (char *)recvbuf + _rsize * displs[i];
			MPI_Recv(p, recvcounts[i], recvtype, i, i /*tag*/, comm, MPI_STATUS_IGNORE);
		}
	} else {
		MPI_Send(sendbuf, sendcount, sendtype, root, comm->group->rank, comm);
	}
#if 0
	mpi_depth--;
	if (depth) {
		PRV_EVENT(50000002, 13);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_gather, _cycle_count - start);
		_xadd64(&time_wait_gather, mpi_waittime);
		_xadd64(&count_gather, 1);
	int size = sendcount*datatype_size(sendtype);
	if (size != -1) {
		_xadd64(&size_collective_sum , size);
		_xinc64(&count_collective_sum , 0 /* disp */);
		if (size < 64) {
			_xadd64(&size_collective_64 , size);
			_xinc64(&count_collective_64 , 0 /* disp */);
		} else if (size < 256) {
			_xadd64(&size_collective_256, size);
			_xinc64(&count_collective_256, 0 /* disp */);
		} else if (size < 1024) {
			_xadd64(&size_collective_1k , size);
			_xinc64(&count_collective_1k , 0 /* disp */);
		} else if (size < 4*1024) {
			_xadd64(&size_collective_4k , size);
			_xinc64(&count_collective_4k , 0 /* disp */);
		} else if (size < 16*1024) {
			_xadd64(&size_collective_16k, size);
			_xinc64(&count_collective_16k, 0 /* disp */);
		} else if (size < 64*1024) {
			_xadd64(&size_collective_64k, size);
			_xinc64(&count_collective_64k, 0 /* disp */);
		} else {
			_xadd64(&size_collective_big, size);
			_xinc64(&count_collective_big, 0 /* disp */);
		}
	}
	}
#endif
	return MPI_SUCCESS;
}



int MPI_Scatter(void *sendbuf, int sendcount, MPI_Datatype sendtype,
    void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
    MPI_Comm comm)
{
#if 0
	mpi_waittime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Scatter\n", mpi_depth);
		PRV_EVENT(50000002, 12);
	}
	mpi_depth++;
#endif
	int recvsize = recvcount*datatype_size(recvtype);
	int sendsize = sendcount*datatype_size(sendtype);
	if (recvsize < sendsize) {
		assert(0 && "scatter:recvbuff < sendbuff?\n");
	}
	if (root >= comm->group->size || root < 0) {
		return error_handling(MPI_ERR_ARG);
	}
	if (root == comm->group->rank) {
		int i, j;
		void *target = (char *)recvbuf;
		memcpy(target, sendbuf + sendsize*root, sendsize);

		int max = comm->group->size;
		for(i=0;i<max;i++) {
			if (i == root)
				continue;
			void *p = (char *)sendbuf + sendsize*i;
			MPI_Send(p, sendcount, sendtype, i, i /*tag*/, comm);
		}
	} else {
		MPI_Recv(recvbuf, recvcount, recvtype, root, comm->group->rank, comm, MPI_STATUS_IGNORE);
	}
#if 0
	mpi_depth--;
	if (depth) {
		PRV_EVENT(50000002, 13);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_scatter, _cycle_count - start);
		_xadd64(&time_wait_scatter, mpi_waittime);
		_xadd64(&count_scatter, 1);
	// TODO: need fix
	int size = sendcount*datatype_size(sendtype);
	if (size != -1) {
		_xadd64(&size_collective_sum , size);
		_xinc64(&count_collective_sum , 0 /* disp */);
		if (size < 64) {
			_xadd64(&size_collective_64 , size);
			_xinc64(&count_collective_64 , 0 /* disp */);
		} else if (size < 256) {
			_xadd64(&size_collective_256, size);
			_xinc64(&count_collective_256, 0 /* disp */);
		} else if (size < 1024) {
			_xadd64(&size_collective_1k , size);
			_xinc64(&count_collective_1k , 0 /* disp */);
		} else if (size < 4*1024) {
			_xadd64(&size_collective_4k , size);
			_xinc64(&count_collective_4k , 0 /* disp */);
		} else if (size < 16*1024) {
			_xadd64(&size_collective_16k, size);
			_xinc64(&count_collective_16k, 0 /* disp */);
		} else if (size < 64*1024) {
			_xadd64(&size_collective_64k, size);
			_xinc64(&count_collective_64k, 0 /* disp */);
		} else {
			_xadd64(&size_collective_big, size);
			_xinc64(&count_collective_big, 0 /* disp */);
		}
	}
	}
#endif
	return MPI_SUCCESS;
}



int MPI_Gather(void *sendbuf, int sendcount, MPI_Datatype sendtype,
    void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
    MPI_Comm comm)
{
#if 0
	mpi_waittime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Gather\n", mpi_depth);
		PRV_EVENT(50000002, 12);
	}
	mpi_depth++;
#endif
	int recvsize = recvcount*datatype_size(recvtype);
	int sendsize = sendcount*datatype_size(sendtype);
	if (recvsize < sendsize) {
		assert(0 && "gather:recvbuff < sendbuff?\n");
	}
	if (root >= comm->group->size || root < 0) {
		return error_handling(MPI_ERR_ARG);
	}
	if (root == comm->group->rank) {
		int i, j;
		void *target = (char *)recvbuf + recvsize*root;
		memcpy(target, sendbuf, sendsize);

		int max = comm->group->size;
		for(i=0;i<max;i++) {
			if (i == root)
				continue;
			void *p = (char *)recvbuf + recvsize*i;
			MPI_Recv(p, recvcount, recvtype, i, i /*tag*/, comm, MPI_STATUS_IGNORE);
		}
	} else {
		MPI_Send(sendbuf, sendcount, sendtype, root, comm->group->rank, comm);
	}
#if 0
	mpi_depth--;
	if (depth) {
		PRV_EVENT(50000002, 13);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_gather, _cycle_count - start);
		_xadd64(&time_wait_gather, mpi_waittime);
		_xadd64(&count_gather, 1);
	int size = sendcount*datatype_size(sendtype);
	if (size != -1) {
		_xadd64(&size_collective_sum , size);
		_xinc64(&count_collective_sum , 0 /* disp */);
		if (size < 64) {
			_xadd64(&size_collective_64 , size);
			_xinc64(&count_collective_64 , 0 /* disp */);
		} else if (size < 256) {
			_xadd64(&size_collective_256, size);
			_xinc64(&count_collective_256, 0 /* disp */);
		} else if (size < 1024) {
			_xadd64(&size_collective_1k , size);
			_xinc64(&count_collective_1k , 0 /* disp */);
		} else if (size < 4*1024) {
			_xadd64(&size_collective_4k , size);
			_xinc64(&count_collective_4k , 0 /* disp */);
		} else if (size < 16*1024) {
			_xadd64(&size_collective_16k, size);
			_xinc64(&count_collective_16k, 0 /* disp */);
		} else if (size < 64*1024) {
			_xadd64(&size_collective_64k, size);
			_xinc64(&count_collective_64k, 0 /* disp */);
		} else {
			_xadd64(&size_collective_big, size);
			_xinc64(&count_collective_big, 0 /* disp */);
		}
	}
	}
#endif
	return MPI_SUCCESS;
}



#define ALLGATHER_ROOT	0
int MPI_Allgatherv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                   void *recvbuf, const int *recvcounts, const int *displs,
                   MPI_Datatype recvtype, MPI_Comm comm)
{
#if 0
	mpi_waittime = 0;
	mpi_cengtime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Allgatherv\n", mpi_depth);
		PRV_EVENT(50000002, 14);
	}
	mpi_depth++;
#endif
	int gsize = FFWD_GLOBAL_MAX;
	MPI_Gatherv(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, ALLGATHER_ROOT, comm);
	MPI_Bcast(recvbuf, displs[gsize-1]+recvcounts[gsize-1] /* assumes..  */ , recvtype, ALLGATHER_ROOT, comm);
#if 0
	mpi_depth--;
	if (depth) {
		PRV_EVENT(50000002, 15);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_allgather, _cycle_count - start);
		_xadd64(&time_wait_allgather, mpi_waittime);
		_xadd64(&time_ceng_allgather, mpi_cengtime);
		_xadd64(&count_allgather, 1);
	int size = sendcount*datatype_size(sendtype);
	if (size != -1) {
		_xadd64(&size_collective_sum , size);
		_xinc64(&count_collective_sum , 0 /* disp */);
		if (size < 64) {
			_xadd64(&size_collective_64 , size);
			_xinc64(&count_collective_64 , 0 /* disp */);
		} else if (size < 256) {
			_xadd64(&size_collective_256, size);
			_xinc64(&count_collective_256, 0 /* disp */);
		} else if (size < 1024) {
			_xadd64(&size_collective_1k , size);
			_xinc64(&count_collective_1k , 0 /* disp */);
		} else if (size < 4*1024) {
			_xadd64(&size_collective_4k , size);
			_xinc64(&count_collective_4k , 0 /* disp */);
		} else if (size < 16*1024) {
			_xadd64(&size_collective_16k, size);
			_xinc64(&count_collective_16k, 0 /* disp */);
		} else if (size < 64*1024) {
			_xadd64(&size_collective_64k, size);
			_xinc64(&count_collective_64k, 0 /* disp */);
		} else {
			_xadd64(&size_collective_big, size);
			_xinc64(&count_collective_big, 0 /* disp */);
		}
	}
	}
#endif
	return MPI_SUCCESS;
}




int MPI_Allgather(void *sendbuf, int  sendcount,
    MPI_Datatype sendtype, void *recvbuf, int recvcount,
     MPI_Datatype recvtype, MPI_Comm comm)
{
#if 0
	mpi_waittime = 0;
	mpi_cengtime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Allgather\n", mpi_depth);
		PRV_EVENT(50000002, 14);
	}
	mpi_depth++;
#endif
	MPI_Gather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, ALLGATHER_ROOT, comm);
	MPI_Bcast(recvbuf, recvcount*FFWD_GLOBAL_MAX, recvtype, ALLGATHER_ROOT, comm);
#if 0
	mpi_depth--;
	if (depth) {
		PRV_EVENT(50000002, 15);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_allgather, _cycle_count - start);
		_xadd64(&time_wait_allgather, mpi_waittime);
		_xadd64(&time_ceng_allgather, mpi_cengtime);
		_xadd64(&count_allgather, 1);
	int size = sendcount*datatype_size(sendtype);
	if (size != -1) {
		_xadd64(&size_collective_sum , size);
		_xinc64(&count_collective_sum , 0 /* disp */);
		if (size < 64) {
			_xadd64(&size_collective_64 , size);
			_xinc64(&count_collective_64 , 0 /* disp */);
		} else if (size < 256) {
			_xadd64(&size_collective_256, size);
			_xinc64(&count_collective_256, 0 /* disp */);
		} else if (size < 1024) {
			_xadd64(&size_collective_1k , size);
			_xinc64(&count_collective_1k , 0 /* disp */);
		} else if (size < 4*1024) {
			_xadd64(&size_collective_4k , size);
			_xinc64(&count_collective_4k , 0 /* disp */);
		} else if (size < 16*1024) {
			_xadd64(&size_collective_16k, size);
			_xinc64(&count_collective_16k, 0 /* disp */);
		} else if (size < 64*1024) {
			_xadd64(&size_collective_64k, size);
			_xinc64(&count_collective_64k, 0 /* disp */);
		} else {
			_xadd64(&size_collective_big, size);
			_xinc64(&count_collective_big, 0 /* disp */);
		}
	}
	}
#endif
	return MPI_SUCCESS;
}


int MPI_Ialltoall(const void *sendbuf, int sendcount,
    MPI_Datatype sendtype, void *recvbuf, int recvcount,
    MPI_Datatype recvtype, MPI_Comm comm, MPI_Request *NEW_REQUEST)
{
	int i;
	int n = comm->group->size;
	int self = comm->group->rank;
	int sendsize, recvsize;
	recvsize = get_size_if_packed(recvcount, recvtype);
	sendsize = get_size_if_packed(sendcount, sendtype);
	if (recvsize == -1 || sendsize == -1) {
		printf("WARN alltoall : not-pacekd datatype, TODO.\n");
		return error_handling(MPI_ERR_INTERN);
	}


//	MPI_Request req[n];
	MPI_Request *req = alloc_request_set(_request, sendcount /* recvcount? */, sendtype /* recvtype? */, 2*n /*tag*/ );

	for(i=0;i<n;i++) {
		if (i == self) {
			memcpy(recvbuf, (char*)sendbuf + sendsize*i, recvsize);
			req[i] = MPI_REQUEST_NULL;
		} else {
			MPI_Irecv(recvbuf, recvcount, recvtype, i, self, comm, &req[i]);
		}
		recvbuf = (char *)recvbuf + recvsize;
	}
	for(i=0;i<n;i++) {
		if (i == self) {
			req[i+n] = MPI_REQUEST_NULL;
		} else {
			MPI_Isend(sendbuf, sendcount, sendtype, i, i, comm, &req[i+n]);
		}
		sendbuf = (char *)sendbuf + sendsize;
	}
//	MPI_Waitall(n, req, MPI_STATUSES_IGNORE);

}

int MPI_Alltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                 void *recvbuf, int recvcount, MPI_Datatype recvtype,
                 MPI_Comm comm)
{
	int i;
	int n = comm->group->size;
	int self = comm->group->rank;
	int sendsize, recvsize;
	recvsize = get_size_if_packed(recvcount, recvtype);
	sendsize = get_size_if_packed(sendcount, sendtype);
	if (recvsize == -1 || sendsize == -1) {
		printf("WARN alltoall : not-pacekd datatype, TODO.\n");
		return error_handling(MPI_ERR_INTERN);
	}
#if 0
	mpi_waittime = 0;
	mpi_cengtime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Alltoall\n", mpi_depth);
		PRV_EVENT(50000002, 14);
	}
	mpi_depth++;
#endif
	MPI_Request req[n];
	for(i=0;i<n;i++) {
		if (i == self) {
			memcpy(recvbuf, (char*)sendbuf + sendsize*i, recvsize);
			req[i] = MPI_REQUEST_NULL;
		} else {
			MPI_Irecv(recvbuf, recvcount, recvtype, i, self, comm, &req[i]);
		}
		recvbuf = (char *)recvbuf + recvsize;
	}
	for(i=0;i<n;i++) {
		if (i == self) {
		} else {
			MPI_Send(sendbuf, sendcount, sendtype, i, i, comm);
		}
		sendbuf = (char *)sendbuf + sendsize;
	}
	MPI_Waitall(n, req, MPI_STATUSES_IGNORE);
#if 0
	mpi_depth--;
	if (depth) {
		PRV_EVENT(50000002, 15);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_alltoall, _cycle_count - start);
		_xadd64(&time_wait_alltoall, mpi_waittime);
		_xadd64(&time_ceng_alltoall, mpi_cengtime);
		_xadd64(&count_alltoall, 1);
	int size = sendcount*datatype_size(sendtype);
	if (size != -1) {
		_xadd64(&size_collective_sum , size * (n-1) );
		_xadd64(&count_collective_sum , n-1 );
		if (size < 64) {
			_xadd64(&size_collective_64 , size * (n-1) );
			_xadd64(&count_collective_64 , n-1);
		} else if (size < 256) {
			_xadd64(&size_collective_256, size * (n-1) );
			_xadd64(&count_collective_256, n-1);
		} else if (size < 1024) {
			_xadd64(&size_collective_1k , size * (n-1) );
			_xadd64(&count_collective_1k , n-1);
		} else if (size < 4*1024) {
			_xadd64(&size_collective_4k , size * (n-1) );
			_xadd64(&count_collective_4k , n-1);
		} else if (size < 16*1024) {
			_xadd64(&size_collective_16k, size * (n-1) );
			_xadd64(&count_collective_16k, n-1);
		} else if (size < 64*1024) {
			_xadd64(&size_collective_64k, size * (n-1) );
			_xadd64(&count_collective_64k, n-1);
		} else {
			_xadd64(&size_collective_big, size * (n-1) );
			_xadd64(&count_collective_big, n-1);
		}
	}
	}
#endif
	return MPI_SUCCESS;
}

int MPI_Alltoallv(const void *sendbuf, const int *sendcounts,
                  const int *sdispls, MPI_Datatype sendtype, void *recvbuf,
                  const int *recvcounts, const int *rdispls, MPI_Datatype recvtype,
                  MPI_Comm comm)
{
	int i;
	int n = comm->group->size;
	int self = comm->group->rank;
	int sendsize, recvsize;
	recvsize = get_size_if_packed(1, recvtype);
	sendsize = get_size_if_packed(1, sendtype);
	if (recvsize == -1 || sendsize == -1) {
		printf("WARN alltoallv : not-pacekd datatype, TODO.\n");
		return error_handling(MPI_ERR_INTERN);
	}
#if 0
	mpi_waittime = 0;
	mpi_cengtime = 0;
	long start = _cycle_count;
	int depth = (mpmode4 > mpi_depth) ? 1 : 0;
	if (depth) {
		printf("[MPI] %d Alltoallv\n", mpi_depth);
		PRV_EVENT(50000002, 14);
	}
	mpi_depth++;
printf("sendbuf %p, scounts %p, sdispls %p, stype %d\n", sendbuf, sendcounts, sdispls, sendtype);
for(i=0;i<n;i++) {
printf("%d count %d disp %d\n", i, sendcounts[i], sdispls[i]);
}
printf("recvbuf %p, rcounts %p, rdispls %p, rtype %d\n", recvbuf, recvcounts, rdispls, recvtype);
for(i=0;i<n;i++) {
printf("%d count %d disp %d\n", i, recvcounts[i], rdispls[i]);
}
#endif
	char *buf;
	MPI_Request req[n];
	for(i=0;i<n;i++) {
		buf = (char *)recvbuf + recvsize * rdispls[i];
		if (i == self) {
			memcpy(buf, (char*)sendbuf + sendsize * sdispls[i], recvsize * recvcounts[i]);
			req[i] = MPI_REQUEST_NULL;
		} else {
			MPI_Irecv(buf, recvcounts[i], recvtype, i, self, comm, &req[i]);
		}
	}
	for(i=0;i<n;i++) {
		buf = (char *)sendbuf + sendsize * sdispls[i];
		if (i == self) {
		} else {
			MPI_Send(buf, sendcounts[i], sendtype, i, i, comm);
		}
	}
	MPI_Waitall(n, req, MPI_STATUSES_IGNORE);
#if 0
	mpi_depth--;
	if (depth) {
		PRV_EVENT(50000002, 15);
	}
	if (mpmode_info && mpi_depth == 0) {
		_xadd64(&time_alltoall, _cycle_count - start);
		_xadd64(&time_wait_alltoall, mpi_waittime);
		_xadd64(&time_ceng_alltoall, mpi_cengtime);
		_xadd64(&count_alltoall, 1);
	int size = datatype_size(sendtype);
	if (size != -1) {
	for(i=0;i<n;i++) {
		if (i == self) continue;
		size = datatype_size(sendtype) * sendcounts[i] ;
		_xadd64(&size_collective_sum , size );
		_xadd64(&count_collective_sum , 1 );
		if (size < 64) {
			_xadd64(&size_collective_64 , size );
			_xadd64(&count_collective_64 , 1);
		} else if (size < 256) {
			_xadd64(&size_collective_256, size );
			_xadd64(&count_collective_256, 1);
		} else if (size < 1024) {
			_xadd64(&size_collective_1k , size );
			_xadd64(&count_collective_1k , 1);
		} else if (size < 4*1024) {
			_xadd64(&size_collective_4k , size );
			_xadd64(&count_collective_4k , 1);
		} else if (size < 16*1024) {
			_xadd64(&size_collective_16k, size );
			_xadd64(&count_collective_16k, 1);
		} else if (size < 64*1024) {
			_xadd64(&size_collective_64k, size );
			_xadd64(&count_collective_64k, 1);
		} else {
			_xadd64(&size_collective_big, size );
			_xadd64(&count_collective_big, 1);
		}
	}
	}
	}
#endif
	return MPI_SUCCESS;
}

#endif
int MPI_Type_struct(int count,
                   int blocklens[],
                   MPI_Aint indices[],
                   MPI_Datatype old_types[],
                   MPI_Datatype *newtype)
{
	int i;
	struct datatype *ret = malloc(sizeof(struct datatype)+count*sizeof(struct element));
	ret->flag = MPI_TYPEFLAG_PACKED;
	ret->count = count;
	ret->size = ret->net_size = 0;
	for(i=0;i<count;i++) {
		ret->e[i].off = indices[i];
		ret->e[i].count = blocklens[i];
		ret->e[i].type = old_types[i];
/*		if (old_types[i] == MPI_UB) {
			// ret->size ?
			break;
		}
*/
//printf("indices:%d\n", indices[i]);
		if (!(datatype_table[old_types[i]]->flag & MPI_TYPEFLAG_PACKED)) {
			ret->flag &= ~MPI_TYPEFLAG_PACKED;
		}
		if (!(indices[i] == ret->size)) {
			assert(indices[i] > ret->size);
			if (indices[i] - ret->size < sizeof(unsigned long)) {	// short padding
				ret->size = indices[i];
			} else {
				ret->flag &= ~MPI_TYPEFLAG_PACKED;
			}
		}
		ret->size += blocklens[i]*datatype_size(old_types[i]);
		ret->net_size += blocklens[i]*datatype_size(old_types[i]);
	}
	ret->extent = ret->size;	// TODO: alignment?

	i = datatype_max;
	assert(i < MAX_DATATYPE);
	datatype_table[datatype_max++] = ret;
	print_type(i);
	*newtype = i;
	return MPI_SUCCESS;
}


int MPI_Type_free(MPI_Datatype *datatype)
{
	if (*datatype < 0 || *datatype >= datatype_max) {
		printf("WARN datatype %d out of range. max:%d (arg:%p)\n", *datatype, datatype_max, datatype);
		return error_handling(MPI_ERR_TYPE);
	}
	if (datatype_table[*datatype]->flag & MPI_TYPEFLAG_BASIC) {
		return error_handling(MPI_ERR_ARG);  // or MPI_ERR_TYPE ?
	}
	free(datatype_table[*datatype]);
	datatype_table[*datatype] = NULL;
	return MPI_SUCCESS;
}


int MPI_Type_commit(MPI_Datatype *datatype)
{
	return MPI_SUCCESS;
}

int MPI_Address(void *location, MPI_Aint *address)
{
	*address = (MPI_Aint)location;
	return MPI_SUCCESS;
}

int MPI_Get_address(void *location, MPI_Aint *address)
{
	*address = (MPI_Aint)location;
	return MPI_SUCCESS;
}

int MPI_Type_vector(int count,
                   int blocklength,
                   int stride,
                   MPI_Datatype old_type,
                   MPI_Datatype *newtype_p)
{
	int i;
	int indices = 0;
	struct datatype *ret = malloc(sizeof(struct datatype)+count*sizeof(struct element));
	ret->flag = MPI_TYPEFLAG_PACKED | MPI_TYPEFLAG_VECTOR;
	ret->count = count;
	ret->size = ret->net_size = 0;
	for(i=0;i<count;i++) {
		ret->e[i].off = indices;
		ret->e[i].count = blocklength;
		ret->e[i].type = old_type;
/*		if (old_types[i] == MPI_UB) {
			// ret->size ?
			break;
		}
*/
//printf("indices:%d\n", indices[i]);
		if (!(datatype_table[old_type]->flag & MPI_TYPEFLAG_PACKED)) {
			ret->flag = 0;
			printf("WARN! MPI_Type_vector consists of non-contiguous types\n");
		}
		if (!(indices == ret->size)) {
			assert(indices > ret->size);
			if (indices - ret->size < sizeof(unsigned long)) {	// short padding
				ret->size = indices;
			} else {
				ret->flag &= ~MPI_TYPEFLAG_PACKED;
			}
		}
		ret->size += blocklength*datatype_size(old_type);
		ret->net_size += blocklength*datatype_size(old_type);
		indices += stride*datatype_size(old_type);
	}
	ret->extent = stride*datatype_size(old_type);

	i = datatype_max;
	assert(i < MAX_DATATYPE);
	datatype_table[datatype_max++] = ret;
	*newtype_p = i;
	print_type(i);
	return MPI_SUCCESS;
}

int MPI_Type_contiguous(int count,
                      MPI_Datatype old_type,
                      MPI_Datatype *new_type_p)
{
	int size = datatype_size(old_type);
	MPI_Type_vector(count, 1, 1, old_type, new_type_p);
	return MPI_SUCCESS;
}

int MPI_Type_size(MPI_Datatype datatype, int *size)
{
	*size = datatype_size(datatype);
	return MPI_SUCCESS;
}

int MPI_Type_indexed(int count, const int array_of_blocklengths[],
    const int array_of_displacements[], MPI_Datatype old_type,
    MPI_Datatype *newtype_p)
{
	int i;
	int blocklength, displ;
	struct datatype *ret = malloc(sizeof(struct datatype)+count*sizeof(struct element));
	ret->flag = 0;	//MPI_TYPEFLAG_PACKED | MPI_TYPEFLAG_VECTOR;
	ret->count = count;
	ret->size = ret->net_size = 0;
	int next_off = 0, packed = 1;	// detecting contiguous types
	int end = 0;
	for(i=0;i<count;i++) {
		blocklength = array_of_blocklengths[i];
		displ = array_of_displacements[i] * datatype_size(old_type);
		if (displ != next_off)
			packed = 0;
		ret->e[i].off = displ;
		ret->e[i].count = blocklength;
		ret->e[i].type = old_type;
		ret->size += blocklength*datatype_size(old_type);	// assumes no overlap
		ret->net_size += blocklength*datatype_size(old_type);
		if (end < ret->e[i].off + blocklength*datatype_size(old_type)) {
			end = ret->e[i].off + blocklength*datatype_size(old_type);
		}
		next_off = displ + ret->size;
	}
	ret->extent = end;

	i = datatype_max;
	assert(i < MAX_DATATYPE);
	datatype_table[datatype_max++] = ret;
	*newtype_p = i;
#ifdef DEBUG_MPI
printf("type_indexed: packed=%d, size %d,%d\n", packed, ret->size, ret->net_size);
#endif
	if (packed)
		ret->flag = MPI_TYPEFLAG_PACKED;
//	print_type(old_type);
//	print_type(i);
	return MPI_SUCCESS;
}

int MPI_Type_create_hindexed(int count, const int array_of_blocklengths[],
    const int array_of_displacements[], MPI_Datatype old_type,
    MPI_Datatype *newtype_p)
{
	int i;
	int blocklength, displ;
	struct datatype *ret = malloc(sizeof(struct datatype)+count*sizeof(struct element));
	ret->flag = 0;	//MPI_TYPEFLAG_PACKED | MPI_TYPEFLAG_VECTOR;
	ret->count = count;
	ret->size = ret->net_size = 0;
	int next_off = 0, packed = 1;	// detecting contiguous types
	int end = 0;
	for(i=0;i<count;i++) {
		blocklength = array_of_blocklengths[i];
		displ = array_of_displacements[i];	// * datatype_size(old_type);
		if (displ != next_off)
			packed = 0;
		ret->e[i].off = displ;
		ret->e[i].count = blocklength;
		ret->e[i].type = old_type;
		ret->size += blocklength*datatype_size(old_type);	// assumes no overlap
		ret->net_size += blocklength*datatype_size(old_type);
		if (end < ret->e[i].off + blocklength*datatype_size(old_type)) {
			end = ret->e[i].off + blocklength*datatype_size(old_type);
		}
		next_off = displ + ret->size;
	}
	ret->extent = end;

	i = datatype_max;
	assert(i < MAX_DATATYPE);
	datatype_table[datatype_max++] = ret;
	*newtype_p = i;
#ifdef DEBUG_MPI
printf("type_indexed: packed=%d\n", packed);
#endif
	if (packed)
		ret->flag = MPI_TYPEFLAG_PACKED;
//	print_type(old_type);
//	print_type(i);
	return MPI_SUCCESS;
}


//--------------
#ifdef MPILITE_TEMP
int MPI_Cart_create(MPI_Comm comm_old, int ndims, const int dims[],
                    const int periods[], int reorder, MPI_Comm *comm_cart)
{
	MPI_Comm_dup(comm_old, comm_cart);
	MPI_Topology t = malloc(sizeof(_MPI_Topology) + ndims*2*sizeof(int) );
	assert(t);
	t->ndims = ndims;
	t->reorder = reorder;
	int i;
	for(i=0;i<ndims;i++) {
		t->a[i].dims = dims[i];
		t->a[i].periods = periods[i];
	}
	(*comm_cart)->topology = t;
}

int MPI_Cart_get(MPI_Comm comm, int maxdims, int dims[], int periods[],
                 int coords[])
{
	MPI_Topology t = comm->topology;
	if (t == NULL) {
		return error_handling(MPI_ERR_TOPOLOGY);
	}
	int i;
	if (maxdims != t->ndims)
		printf("INFO maxdims != ndims?\n");
	int coord = comm->group->rank;	// rank
	for(i=0;i<maxdims;i++) {
		dims[i] = t->a[i].dims;
		periods[i] = t->a[i].periods;
		coords[i] = coord % dims[i];
		coord = coord / dims[i];
	}
	return MPI_SUCCESS;
}

int MPI_Cart_coords(MPI_Comm comm, int rank, int maxdims, int coords[])
{
	MPI_Topology t = comm->topology;
	if (t == NULL) {
		return error_handling(MPI_ERR_TOPOLOGY);
	}
	int i;
	int dims;
	if (maxdims != t->ndims)
		printf("INFO maxdims != ndims?\n");
	int coord = rank;
	for(i=0;i<maxdims;i++) {
		dims = t->a[i].dims;
		coords[i] = coord % dims;
		coord = coord / dims;
	}
	return MPI_SUCCESS;
}

int MPI_Cart_sub(MPI_Comm comm, int *remain_dims, MPI_Comm *newcomm)
{
	printf("TODO : not implemented yet -- mpi_cart_sub\n");
}

int MPI_Cart_rank(MPI_Comm comm, const int coords[], int *rank)
{
	printf("TODO : not implemented yet -- mpi_cart_rank\n");
}


#endif
#ifdef MPILITE_FORTRAN
//------------ support for Fortran -----------------------
// algorithm for making C wrapper for Fortran SUBroutine. Note, the Fortran
// and C API have the same argument names in the same order, except fortran
// has IERROR as the last argument, instead of being a function and having
// it as a function result.
// 1. copy corresponding C fn decl to this section thrice
//   a. once for the fn decl
//   b. once for the fn call
//   c. once for a comment showing the full args and types.
// 2. for fn decl:
//   a. replace int return type by "void"
//   b. convert name to lower case, append '_'
//   c. for each arg, if it isn't declared as a pointer, add '*' (all fortran args are pointers)
//   d. add ", int *ierr" as the last argument - fortran MPI always passes this as last arg
// 3. for fn call
//   a. replace int return type by "*ierr =" to get the error status
//   b. for each argument
//      i. remove the datatype
//      ii. if there is a '*' because the arg is supposed to be a Pointer, DELETE the '*' (so it remains a pointer!)
//      iii. if there is no '*' because the arg is supposed to be a Value, ADD a '*' to deref the pointer and get the value!
// 4. Copy the fortran decl from http://www.mpi-forum.org/docs as a comment for documentation

struct { MPI_Comm mpi_comm_world; } mpisupport_  __attribute__ ((aligned(32))) = { &_mpi_comm_world };

void mpi_comm_rank_(int *comm, int* rank, int* ierr)
{
#ifdef DEBUG_MPI
  printf("mpi_comm_rank_: comm: %p\n", comm);
  printf("mpi_comm_rank_: *comm: %p, MPI_COMM_WORLD: %p\n", *comm, MPI_COMM_WORLD);
#endif
//	printf("c: comm=%p, %d\n", comm, *comm);
	*ierr = MPI_Comm_rank(*comm, rank);
}

void mpi_comm_size_(int *comm, int* size, int* ierr)
{
#ifdef DEBUG_MPI
  printf("mpi_comm_size_: comm: %p\n", comm);
  printf("mpi_comm_size_: *comm: %p, MPI_COMM_WORLD: %p\n", *comm, MPI_COMM_WORLD);
#endif
//	printf("c: comm=%p, %d\n", comm, *comm);
	*ierr = MPI_Comm_size(*comm, size);
}

void mpi_comm_dup_(int *comm, int *newcomm, int *ierr)
{
 // MPI_COMM_DUP(COMM, NEWCOMM, IERROR)
 // INTEGER COMM, NEWCOMM, IERROR
  *ierr = MPI_Comm_dup(*comm, newcomm);
 // int MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm)
#ifdef DEBUG_MPI
  printf("mpi_comm_dup_: *comm: %p, *newcomm: %p\n", *comm, *newcomm);
#endif
}

void mpi_init_thread_(int *required, int *provided, int *ierr)
{
	int dummy_argc;
	char** dummy_argv;
	*ierr = MPI_Init_thread(&dummy_argc, &dummy_argv, *required, provided);
}

void mpi_init_(int *ierr)
{
#ifdef DEBUG_MPI
  printf("mpi_init_: \n");
#endif

	int dummy_argc;
	char** dummy_argv;
	*ierr = MPI_Init(&dummy_argc, &dummy_argv);
}

void mpi_initialized_(int *flag, int *ierr)
{
	*ierr = MPI_Initialized(flag);
}

void mpi_finalize_(int *ierr)
{
	*ierr = MPI_Finalize();
}

void mpi_send_(void *buf, int *count, int *datatype, int *dest, int *tag, int *comm, int *ierr)
{
#ifdef DEBUG_MPI
  printf("mpi_send_: *comm: %p, *tag: %d, *count: %d\n", *comm, *tag, *count);
#endif
//int MPI_Send (void *buf,int count, MPI_Datatype
//datatype, int dest, int tag, MPI_Comm comm)
	*ierr = MPI_Send(buf, *count, *datatype, *dest, *tag, *comm);
}

void mpi_recv_(void *buf, int *count, int *datatype, int *src, int *tag, int *comm, void *status, int *ierr)
{
#ifdef DEBUG_MPI
  printf("mpi_recv_: *comm: %p, *tag: %d, *count %d\n", *comm, *tag, *count);
#endif
	*ierr = MPI_Recv(buf, *count, *datatype, *src, *tag, *comm, status);
//int MPI_Recv(void *buf,int count, MPI_Datatype
//datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
}

void mpi_get_count_(void *status, MPI_Datatype *datatype, int *count, int *ierr)
{
	*ierr = MPI_Get_count(status, *datatype, count);
//int MPI_Get_count(
  //             MPI_Status *status,
    //           MPI_Datatype datatype,
      //         int *count )
}


/*void mpi_abort()
{
}
*/

void mpi_barrier_( int *comm, int *ierr )
{
#ifdef DEBUG_MPI
  printf("mpi_comm_barrier_: comm: %p\n", comm);
  printf("mpi_comm_barrier_: *comm: %p, MPI_COMM_WORLD: %p\n", *comm, MPI_COMM_WORLD);
#endif

  //MPI_BARRIER(COMM, IERROR)
  //INTEGER COMM, IERROR

  *ierr = MPI_Barrier(*comm);
  // int MPI_Barrier( MPI_Comm comm )
}

void mpi_bcast_ ( void *buffer, int *count, MPI_Datatype *datatype, int *root,
		  int *comm, int *ierr )
{
  // MPI_BCAST(BUFFER, COUNT, DATATYPE, ROOT, COMM, IERROR)
  // <type> BUFFER(*)
  // INTEGER COUNT, DATATYPE, ROOT, COMM, IERROR

  *ierr = MPI_Bcast ( buffer, *count, *datatype, *root, *comm );

  // int MPI_Bcast ( void *buffer, int count, MPI_Datatype datatype, int root,
  //                      MPI_Comm comm )
}

void mpi_allreduce_ ( void *sendbuf, void *recvbuf, int *count,
		      MPI_Datatype *datatype, MPI_Op *op, int *comm, int *ierr )
{
  //  MPI_ALLREDUCE(SENDBUF, RECVBUF, COUNT, DATATYPE, OP, COMM, IERROR)
  // <type> SENDBUF(*), RECVBUF(*)
  // INTEGER COUNT, DATATYPE, OP, COMM, IERROR

  *ierr = MPI_Allreduce ( sendbuf, recvbuf, *count,
		    *datatype, *op, *comm );

  // int MPI_Allreduce ( void *sendbuf, void *recvbuf, int count,
  //                        MPI_Datatype datatype, MPI_Op op, MPI_Comm comm )
}

double mpi_wtime_( void )	// Time in seconds since an arbitrary time in the past.
{
  //      EXTERNAL MPI_WTIME
  //       DOUBLE PRECISION MPI_WTIME

  return MPI_Wtime();
  //  double MPI_Wtime( void );
}

void mpi_allgather_(void *sendbuf, int *sendcount,
    MPI_Datatype *sendtype, void *recvbuf, int *recvcount,
     MPI_Datatype *recvtype, int *comm, int *ierr)
{
// MPI_ALLGATHER(SENDBUF, SENDCOUNT, SENDTYPE, RECVBUF, RECVCOUNT, RECVTYPE,
// COMM, IERROR)
// <type> SENDBUF(*), RECVBUF(*)
// INTEGER SENDCOUNT, SENDTYPE, RECVCOUNT, RECVTYPE, COMM, IERROR

*ierr = MPI_Allgather(sendbuf, *sendcount,
    *sendtype, recvbuf, *recvcount,
    *recvtype, *comm);

//int MPI_Allgather(void *sendbuf, int  sendcount,
//    MPI_Datatype sendtype, void *recvbuf, int recvcount,
//     MPI_Datatype recvtype, MPI_Comm comm)
}

void mpi_waitall_(int *count, int /*MPI_Request*/ *array_of_requests,
    MPI_Status *array_of_statuses, int *ierr)
{
*ierr = MPI_Waitall_fortran(*count, array_of_requests,
    array_of_statuses);

//int MPI_Waitall(int count, MPI_Request *array_of_requests,
//    MPI_Status *array_of_statuses)
}


void mpi_gather_(void *sendbuf, int *sendcount, MPI_Datatype *sendtype,
    void *recvbuf, int *recvcount, MPI_Datatype *recvtype, int *root,
    int *comm, int *ierr)
{
//  MPI_GATHER(SENDBUF, SENDCOUNT, SENDTYPE, RECVBUF, RECVCOUNT, RECVTYPE,
//  ROOT, COMM, IERROR)
//  <type> SENDBUF(*), RECVBUF(*)
//  INTEGER SENDCOUNT, SENDTYPE, RECVCOUNT, RECVTYPE, ROOT, COMM, IERROR


  *ierr = MPI_Gather(sendbuf, *sendcount, *sendtype,
    recvbuf, *recvcount, *recvtype, *root,
    *comm);

// int MPI_Gather(void *sendbuf, int sendcount, MPI_Datatype sendtype,
//     void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
//     MPI_Comm comm)
}


void mpi_irecv_(void *buf, int *count, MPI_Datatype *datatype, int *source,
              int *tag, int *comm, int /* MPI_Request*/ *request, int *ierr)
{
	// in fortran, 'request' is integer..so we need malloc/free
	MPI_Request *req = malloc(sizeof(MPI_Request));	// will be freed at MPI_Waitall_fortran()
	assert(((unsigned long)req & 0xffffffff00000000UL) == 0);	// must be 32bit address
	*request = (int)req;	// because here it's assigned to int
	*ierr = MPI_Irecv(buf, *count, *datatype, *source,
        	      *tag, *comm, req);

//int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source,
//              int tag, MPI_Comm comm, MPI_Request *request)
}


void mpi_reduce_(void *sendbuf, void *recvbuf, int *count,
    MPI_Datatype *datatype, MPI_Op *op, int *root, int *comm, int *ierr)
{
*ierr = MPI_Reduce(sendbuf, recvbuf, *count,
    *datatype, *op, *root, *comm);

//int MPI_Reduce(void *sendbuf, void *recvbuf, int count,
//    MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
}

void mpi_abort_(int *comm, int *errorcode, int *ierr)
{
*ierr = MPI_Abort(*comm, *errorcode);

//int MPI_Abort(MPI_Comm comm, int errorcode)
}

#endif
