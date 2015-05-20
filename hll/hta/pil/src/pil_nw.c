#include "pil_nw.h"
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <semaphore.h>

#ifndef NON_BLOCKING_SEND
#define NON_BLOCKING_SEND (1)
#endif // NON_BLOCKING_SEND

#ifndef PIL_PREALLOC
// turn on or off the preallocation of PIL send buffers
#define PIL_PREALLOC (0)
#endif // PIL_PREALLOC

#ifndef PIL_PREALLOC_SIZE
// size in bytes to prealloc the PIL send buffers
#define PIL_PREALLOC_SIZE (1024)
#endif // PIL_PREALLOC_SIZE

#ifndef PIL_PREALLOC_VERIFY
// turn the verification messages of PIL_PREALLOC on/off
#define PIL_PREALLOC_VERIFY (0)
#endif // PIL_PREALLOC_VERIFY

#ifndef CIRC_BUF_SIZE
#define CIRC_BUF_SIZE (3)
#endif // CIRC_BUF_SIZE

#ifndef NW_CRITICAL_SECTION
#define NW_CRITICAL_SECTION (0)
#endif // NW_CRITICAL_SECTION

#define DEBUG_BARRIER (0)

// global variables
int _pil_barrier_counter = 0;
sem_t _pil_turnstile;
sem_t _pil_turnstile2;
sem_t _pil_mutex;
#if (CIRC_BUF_SIZE == 1)
struct _pil_communication_buffer **_pil_send_buf = NULL;
#else
struct _pil_send_buffer **_pil_send_buf = NULL;
#endif // CIRC_BUF_SIZE

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#if 0
pil_nwID_t pil_get_nwID()
{
	pil_nwID_t id = 0;
#ifdef DISTSWARM
	id = nw_getNodeID();
#endif /* DISTSWARM */
	return id;
}
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void pil_barrier_shared(int rank)
{
	sem_wait(&_pil_mutex);
#if DEBUG_BARRIER
		printf("%d: increment critical section %d\n", rank, _pil_barrier_counter); fflush(stdout);
#endif // DEBUG_BARRIER
		_pil_barrier_counter += 1;
		if (_pil_barrier_counter == pil_get_nwCount()) {
#if DEBUG_BARRIER
			printf("%d: all threads have critical sectioned\n", rank); fflush(stdout);
#endif // DEBUG_BARRIER
			sem_wait(&_pil_turnstile2); // lock the second
			sem_post(&_pil_turnstile); // unlock the first
		}
	sem_post(&_pil_mutex);

#if DEBUG_BARRIER
	printf("%d: first turnstile\n", rank); fflush(stdout);
#endif // DEBUG_BARRIER
	sem_wait(&_pil_turnstile); // first turnstile
	sem_post(&_pil_turnstile);

	// critical point

#if DEBUG_BARRIER
	printf("%d: critical point\n", rank);
#endif // DEBUG_BARRIER

	sem_wait(&_pil_mutex);
#if DEBUG_BARRIER
		printf("%d: decrement critical section %d\n", rank, _pil_barrier_counter); fflush(stdout);
#endif // DEBUG_BARRIER
		_pil_barrier_counter -= 1;
		if (_pil_barrier_counter == 0) {
#if DEBUG_BARRIER
			printf("%d: all threads have critical sectioned\n", rank); fflush(stdout);
#endif // DEBUG_BARRIER
			sem_wait(&_pil_turnstile); // lock the first
			sem_post(&_pil_turnstile2); // unlock the second
		}
	sem_post(&_pil_mutex);

	sem_wait(&_pil_turnstile2); // second turnstile
	sem_post(&_pil_turnstile2);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void _pil_send_shared(int myrank, int dest, gpp_t buf, size_t size, size_t offset, int target)
{
#if (CIRC_BUF_SIZE == 1)
	struct _pil_communication_buffer *psb = &_pil_send_buf[myrank][dest];
	//printf("%d: pil_send\n", myrank);
#if NON_BLOCKING_SEND
	while (psb->full == 1) ; // wait for the recv to collect the data. blocking send.
#else
	assert(psb->full == 0);
#endif // NON_BLOCKING_SEND
	// check that the buffer is at least large enough. if not, free it and alloc a new one
	if (psb->size < size) {
		free(psb->ptr);
		psb->ptr = (void *) malloc(size);
	}
	// do the send
	psb->size = size;
	memcpy(psb->ptr, buf.ptr + offset, size);
	psb->full = 1;
#if !NON_BLOCKING_SEND
	//printf("%d: data sent waiting for it to be read\n", myrank); fflush(stdout);
	while (psb->full == 1) ; // wait for the recv to collect the data. blocking send.
#endif // NON_BLOCKING_SEND
#else // CIRC_BUF_SIZE
	struct _pil_send_buffer *psb = &_pil_send_buf[myrank][dest];
	// check that there is a free buffer by checking the read and write counts
#if NW_CRITICAL_SECTION
	while(1) {
		sem_wait(&psb->mutex);
		if (psb->writer->full == 0) break;
		else sem_post(&psb->mutex);
	}
#else // NW_CRITICAL_SECTION
	while (psb->writer->full == 1) ;
#endif // NW_CRITICAL_SECTION
	// check that the free buffer at the write (send) head is at least the right size.
	if (psb->writer->size < size) {
#if PIL_PREALLOC_VERIFY
		printf("WARNING: PIL_PREALLOC is turned on, but pil_send is reallocing a buffer of size %zu\n", psb->max_buffer_size);
#endif // PIL_PREALLOC_VERIFY
		// if not alloc a new buffer the size of the largest message sent so far.
		free(psb->writer->ptr);
		if (size > psb->max_buffer_size) psb->max_buffer_size = size;
		psb->writer->ptr = (void *) malloc(psb->max_buffer_size);
		psb->writer->size = size;
	}
	// now that we have a buffer of the right size, perform the send by populating it
	memcpy(psb->writer->ptr, buf.ptr + offset, size);
	assert(psb->writer->full == 0);
	// change the write (send) head to the next buffer in the circle
	psb->writer->full = 1;
	psb->writer = psb->writer->next;
#if NW_CRITICAL_SECTION
	sem_post(&psb->mutex);
#endif // NW_CRITICAL_SECTION
#endif // CIRC_BUF_SIZE
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void _pil_recv_shared(int myrank, int src, gpp_t *buf, size_t size, size_t offset, int target)
{
#if (CIRC_BUF_SIZE == 1)
	struct _pil_communication_buffer *psb = & _pil_send_buf[src][myrank];
	//printf("%d: pil_recv waiting for data\n", myrank); fflush(stdout);
	while(psb->full == 0) ; // wait for the buffer to be populated
	memcpy(buf->ptr + offset, psb->ptr, size);
	assert(psb->full == 1);
	psb->full = 0;
	//printf("%d: recv complete\n", myrank); fflush(stdout);
#else // CIRC_BUF_SIZE
	struct _pil_send_buffer *psb = & _pil_send_buf[src][myrank];
#if NW_CRITICAL_SECTION
	while(1) {
		sem_wait(&psb->mutex);
		if (psb->reader->full == 1) break;
		else sem_post(&psb->mutex);
	}
#else
	while(psb->reader->full == 0) ; // wait for the buffer to be populated
#endif // NW_CRITICAL_SECTION
	memcpy(buf->ptr + offset, psb->reader->ptr, size);
	assert(psb->reader->full == 1);
	psb->reader->full = 0;
	psb->reader = psb->reader->next;
#if NW_CRITICAL_SECTION
	sem_post(&psb->mutex);
#endif // NW_CRITICAL_SECTION
#endif // CIRC_BUF_SIZE
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
pil_nwID_t pil_get_nwCount()
{
	pil_nwID_t id = 0;
#ifdef DISTSWARM
	id = nw_getNodeCount();
#else
	const char *_PIL_NUM_THREADS = "PIL_NUM_THREADS";
	char *_pil_NP_str = getenv(_PIL_NUM_THREADS);
	int _pil_NP = 1;
	if (_pil_NP_str != NULL) {
		_pil_NP = atoi(_pil_NP_str);
	}
	id = _pil_NP;
#endif /* DISTSWARM */
	return id;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void pil_barrier_all(int rank)
{
#ifdef PIL2C
	//#pragma omp taskwait
	pil_barrier_shared(rank);
#endif
#ifdef PIL2SWARM
	//printf("\t%d: barrier\n", rank); fflush(stdout);
	pil_barrier_shared(rank);
#endif
#ifdef DISTSWARM
	//printf("pil_barrier_all()\n"); fflush(stdout);
	nw_barrierAll(GRAPH_BARRIER_0, swarm_cargs(bar));
#endif /* DISTSWARM */
#ifdef PIL2OCR
	pil_barrier_shared(rank);
#endif /* PIL2OCR */
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void pil_send(int myrank, int dest, gpp_t buf, size_t size, size_t offset, int target)
{
#if 0
#ifdef DISTSWARM
	nw_netBuffer_t *nb;
	nb = nw_netBuffer_new(NULL, num)
	//void * memcpy ( void * destination, const void * source, size_t num );
	memcpy(nb->p, source_ptr, num);
	nw_call(target_node, TARGET_CODELET, nw_NetBuffer_to_swarm_Trasferable(nb), NULL, NULL);
#endif /* DISTSWARM */
#endif
#ifdef PIL2C
	_pil_send_shared(myrank, dest, buf, size, offset, target);
#endif
#ifdef PIL2SWARM
	_pil_send_shared(myrank, dest, buf, size, offset, target);
#endif
#ifdef PIL2OCR
	_pil_send_shared(myrank, dest, buf, size, offset, target);
#endif
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void pil_recv(int myrank, int src, gpp_t *buf, size_t size, size_t offset, int target)
{
#ifdef PIL2C
	_pil_recv_shared(myrank, src, buf, size, offset, target);
#endif
#ifdef PIL2SWARM
	_pil_recv_shared(myrank, src, buf, size, offset, target);
#endif
#ifdef PIL2OCR
	_pil_recv_shared(myrank, src, buf, size, offset, target);
#endif
}

//-----------------------------------------------------------------------------
// return the status of the full/empty bit in the corresponding buffer. This
// is used to see if it is worthwhile to make a pil_recv call. If the buffer
// is not full, the call to recv will busy wait.
//-----------------------------------------------------------------------------
uint8_t pil_recv_get_buf_status(int myrank, int src)
{
#if (CIRC_BUF_SIZE == 1)
	return _pil_send_buf[src][myrank].full;
#else
	return _pil_send_buf[src][myrank].reader->full;
#endif // CIRC_BUF_SIZE
}

//-----------------------------------------------------------------------------
// return the status of the full/empty bit in the corresponding buffer. This
// is used to see if it is worthwhile to make a pil_send call. If the buffer
// is already full, the call to send will busy wait.
//-----------------------------------------------------------------------------
uint8_t pil_send_get_buf_status(int myrank, int dest)
{
#if (CIRC_BUF_SIZE == 1)
	return _pil_send_buf[myrank][dest].full;
#else
	return _pil_send_buf[myrank][dest].writer->full;
#endif // CIRC_BUF_SIZE
}


void pil_init_single_buffer()
{
#if (CIRC_BUF_SIZE == 1)
	int _pil_NP = pil_get_nwCount();
	// each processor gets space to hold buffers
	_pil_send_buf = (struct _pil_communication_buffer **) malloc(_pil_NP*sizeof(struct _pil_communication_buffer *));
	for (int src = 0; src < _pil_NP; src++) {
		// there is P*P buffers
		_pil_send_buf[src] = (struct _pil_communication_buffer *) malloc(_pil_NP*sizeof(struct _pil_communication_buffer));
		for (int dest = 0; dest < _pil_NP; dest++) {
#if PIL_PREALLOC
			_pil_send_buf[src][dest].ptr = (void *) malloc(PIL_PREALLOC_SIZE);
			_pil_send_buf[src][dest].size = PIL_PREALLOC_SIZE;
#else
			_pil_send_buf[src][dest].ptr = NULL;
			_pil_send_buf[src][dest].size = 0;
#endif // PIL_PREALLOC
			_pil_send_buf[src][dest].full = 0;
			_pil_send_buf[src][dest].next = NULL;
		}
	}
	for (int src = 0; src < _pil_NP; src++) {
		for (int dest = 0; dest < _pil_NP; dest++) {
#if PIL_PREALLOC
			assert(_pil_send_buf[src][dest].ptr);
			assert(_pil_send_buf[src][dest].size == PIL_PREALLOC_SIZE);
#else
			assert(_pil_send_buf[src][dest].ptr == NULL);
			assert(_pil_send_buf[src][dest].size == 0);
#endif // PIL_PREALLOC
		}
	}
#endif // CIRC_BUF_SIZE
}

void pil_nw_init_barrier()
{
	// init the semaphores for the barrier
	sem_init(&_pil_mutex, 0, 1);
	sem_init(&_pil_turnstile, 0, 0);
	sem_init(&_pil_turnstile2, 0, 1);
	_pil_barrier_counter = 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void pil_nw_init()
{
	pil_nw_init_barrier();
#if PIL_PREALLOC_VERIFY
	printf("preallocating buffers of size %d\n", PIL_PREALLOC_SIZE);
#endif

#if (CIRC_BUF_SIZE == 1)
	pil_init_single_buffer();
#else
	int _pil_NP = pil_get_nwCount();
	// each processor gets space to hold buffers
	_pil_send_buf = (struct _pil_send_buffer **) malloc(_pil_NP*sizeof(struct _pil_send_buffer *));
	for (int src = 0; src < _pil_NP; src++) {
		// there is P*P buffers
		_pil_send_buf[src] = (struct _pil_send_buffer *) malloc(_pil_NP*sizeof(struct _pil_send_buffer));
		for (int dest = 0; dest < _pil_NP; dest++) {
			_pil_send_buf[src][dest].reader = NULL;
			_pil_send_buf[src][dest].writer = NULL;
#if PIL_PREALLOC
			_pil_send_buf[src][dest].max_buffer_size = PIL_PREALLOC_SIZE;
#else
			_pil_send_buf[src][dest].max_buffer_size = 0;
#endif
			sem_init(&_pil_send_buf[src][dest].mutex, 0, 1);
			for (int bufcount = 0; bufcount < CIRC_BUF_SIZE; bufcount++) {
				struct _pil_communication_buffer *buf = (struct _pil_communication_buffer *) malloc(sizeof(struct _pil_communication_buffer));
#if PIL_PREALLOC
				buf->ptr = (void *) malloc(PIL_PREALLOC_SIZE); // prealloc the size of a pointer
				buf->size = PIL_PREALLOC_SIZE;
#else
				buf->ptr = NULL;
				buf->size = 0;
#endif
				buf->full = 0;
				buf->next = _pil_send_buf[src][dest].reader; // link the chain
				_pil_send_buf[src][dest].reader = buf; // reader is the pointer to the beginning of the chain
				if (buf->next == NULL) _pil_send_buf[src][dest].writer = buf; // make writer be the first buf allocated. the end of the chain
			}
			_pil_send_buf[src][dest].writer->next = _pil_send_buf[src][dest].reader; // complete the circle by closing the chain
			_pil_send_buf[src][dest].writer = _pil_send_buf[src][dest].reader; // set reader and writer the same since all are empty
		}
	}
#if PIL_PREALLOC_VERIFY
	printf("verifying the buffers were allocated\n");
#endif // PIL_PREALLOC
	for (int src = 0; src < _pil_NP; src++) {
		for (int dest = 0; dest < _pil_NP; dest++) {
#if PIL_PREALLOC
#if PIL_PREALLOC_VERIFY
			int count = 0;
			printf("_pil_send_buf[%d][%d].max_buffer_size: %d\n", src, dest, PIL_PREALLOC_SIZE);
#endif // PIL_PREALLOC_VERIFY
			assert(_pil_send_buf[src][dest].max_buffer_size == PIL_PREALLOC_SIZE);
#else
			assert(_pil_send_buf[src][dest].max_buffer_size == 0);
#endif // PIL_PREALLOC
			assert(_pil_send_buf[src][dest].reader == _pil_send_buf[src][dest].writer);
			for (int bufcount = 0; bufcount < CIRC_BUF_SIZE; bufcount++) {
#if PIL_PREALLOC
#if PIL_PREALLOC_VERIFY
				printf("_pil_send_buf[%d][%d].reader[%d]->size: %d\n", src, dest, count++, PIL_PREALLOC_SIZE);
#endif // PIL_PREALLOC_VERIFY
				assert(_pil_send_buf[src][dest].reader->ptr);
				assert(_pil_send_buf[src][dest].reader->size == PIL_PREALLOC_SIZE);
#else
				assert(_pil_send_buf[src][dest].reader->ptr == NULL);
				assert(_pil_send_buf[src][dest].reader->size == 0);
#endif // PIL_PREALLOC
				assert(_pil_send_buf[src][dest].reader->full == 0);
				_pil_send_buf[src][dest].reader = _pil_send_buf[src][dest].reader->next;
			}
			assert(_pil_send_buf[src][dest].reader == _pil_send_buf[src][dest].writer);
		}
	}
#endif // CIRC_BUF_SIZE
}

void pil_nw_cleanup()
{
#if (CIRC_BUF_SIZE == 1)
	for (int src = 0; src < pil_get_nwCount(); src++) {
/*
		for (int dest = 0; dest <pil_get_nwCount(); dest++) {
			free(_pil_send_buf[src][dest].ptr);
		}
*/
		free (_pil_send_buf[src]);
	}
	free(_pil_send_buf);
#else
	for (int src = 0; src < pil_get_nwCount(); src++) {
		for (int dest = 0; dest < pil_get_nwCount(); dest++) {
			for (int bufcount = 0; bufcount < CIRC_BUF_SIZE; bufcount++) {
				struct _pil_communication_buffer *buf = _pil_send_buf[src][dest].reader;
				_pil_send_buf[src][dest].reader = _pil_send_buf[src][dest].reader->next;
				free(buf);
			}
		}
		free(_pil_send_buf[src]);
	}
	free(_pil_send_buf);
#endif // CIRC_BUF_SIZE
}
