/*
* This file is subject to the license agreement located in the file LICENSE
* and cannot be distributed without it. This notice cannot be
* removed or modified.
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "messaging.h"

//#define MESSAGING_DEBUG_MESSAGE 1


#include <stdint.h>
__inline__ uint64_t rdtsc() {
	uint32_t lo, hi;
	/* We cannot use "=A", since this would use %rax on x86_64 */
	asm ("\n\tcpuid\n\t");
	__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
	return (uint64_t)hi << 32 | lo;
}

void ffwd_dump_message(struct ffwd_message * p)
{
    printf("Message: base addr=%p\n", p);
    printf("         size of=%d\n", sizeof(struct ffwd_message));
    if (p == 0x0) {
        printf("         None\n");
        return;
    }
    printf("         count=%d\n", p->count);
    printf("         datatype=%d\n", p->datatype);
    printf("         tag=%d\n", p->tag);
    printf("         gpid=%d\n", p->gpid);
    printf("         flag=%d\n", p->flag);
    printf("         status=%d\n", p->status);
    printf("         buf=%p\n", p->buf);
    printf("         prev=%p\n", p->prev);
    printf("         next=%p\n", p->next);
    return;
}

struct ffwd_queue {
	struct ffwd_message *head;	// global address
	struct ffwd_message *tail;	// global address
	int count;
};

#ifndef MPILITE_SHARED_HEAP

struct ffwd_message *ffwd_mq_alloc(int size)
{
	return malloc(sizeof(struct ffwd_message)+size);
}
void ffwd_mq_free(struct ffwd_message *p)
{
	free(p);
}
#endif

#ifdef MPILITE_REQUEST_SHADOWING
struct ffwd_message *ffwd_message_null = NULL;
#else
struct ffwd_message ffwd_message_null = {0, 0, 0, 0, 0, FFWD_MQ_NULL, (void *)0, (void *)0, (void *)0};
#endif
#define MQUEUE(DEST)		(&mqueue_matrix[MAX_MQUEUE*mygpid+DEST])
#define MQUEUE_REMOTE(DEST,R)	(&mqueue_matrix[MAX_MQUEUE*(R)+DEST])

struct ffwd_mq {
	struct ffwd_lock_t lock;
	struct ffwd_queue send;
	struct ffwd_queue recv;
};


#ifdef MPILITE_ANYSOURCE
static struct ffwd_mq
#endif

#define MAX_MQUEUE	mpilite_rank
static struct ffwd_mq *mqueue_matrix;
int ffwd_mq_matrix_size(void) {
	return sizeof(struct ffwd_mq)*MAX_MQUEUE*MAX_MQUEUE;
}
void ffwd_mq_init(void *p) {
	mqueue_matrix = p;
}

static struct ffwd_message *ffwd_mq_deq(struct ffwd_queue *q, int tag, int gpid)
{
#ifdef MESSAGING_DEBUG_MESSAGE
	printf("ffwd_mq_deq: from mq %p (tail:%p, head:%p)\n", q , q->tail, q->head);
#endif
	struct ffwd_message *p = q->tail;
#ifdef MESSAGING_DEBUG_MESSAGE
        printf("ffwd_mq_deq: before ffwd_message p=%p\n", p);
        ffwd_dump_message(p);
#endif
	assert(!p || (p->status < FFWD_MQ_MAX && p->status > FFWD_MQ_MIN));
	while (p && ((tag != FFWD_MQ_ANY_TAG && p->tag != FFWD_MQ_ANY_TAG && p->tag != tag) || (gpid != FFWD_MQ_ANY_SOURCE && p->gpid != FFWD_MQ_ANY_SOURCE && p->gpid != gpid))) {	// tag matching
		p = p->prev;
		assert(!p || (p->status < FFWD_MQ_MAX && p->status > FFWD_MQ_MIN));
	}
	if (!p)
        {
#ifdef MESSAGING_DEBUG_MESSAGE
            printf("ffwd_mq_deq:tag not found\n");
#endif
            return NULL;
        }
#ifdef MESSAGING_DEBUG_MESSAGE
        printf("ffwd_mq_deq: after ffwd_message p=%p\n", p);
        ffwd_dump_message(p);
#endif
	if (p == q->tail)
		q->tail = p->prev;
	if (p == q->head)
		q->head = p->next;
	if (p->next)
		(p->next)->prev = p->prev;
	if (p->prev)
		(p->prev)->next = p->next;
	q->count--;
#ifdef MESSAGING_DEBUG_MESSAGE
        printf("ffwd_mq_deq: q-deq:%p, now count=%d\n", q, q->count);
#endif
	return p;
}


static void ffwd_mq_enq(struct ffwd_message *pp, struct ffwd_queue *q)
{
#ifdef MESSAGING_DEBUG_MESSAGE
	printf("ffwd_mq_enq: %p to mq %p (tail:%p, head:%p)\n", pp, q , q->tail, q->head);
#endif
	pp->prev = NULL;
	pp->next = q->head;
	assert(pp->status < FFWD_MQ_MAX && pp->status > FFWD_MQ_MIN);
	if (q->head) {
		(q->head)->prev = pp;
		q->head = pp;
	} else {
		q->head = q->tail = pp;
	}
	q->count++;
#ifdef MESSAGING_DEBUG_MESSAGE
        printf("ffwd_mq_enq:%p, now count=%d\n", q, q->count);
	ffwd_dump_message(pp);
#endif
}



int ffwd_mq_wait(struct ffwd_message *p)
{
#ifdef MPILITE_REQUEST_SHADOWING
	if (p == ffwd_message_null) {
		return 1;
	}
#endif
	if (p->status >= FFWD_MQ_MAX || p->status <= FFWD_MQ_MIN) {
		assert(0 && "ffwd_mq_wait: BUG! MPI_Request is not initialized?\n");
	}
#ifdef MESSAGING_DEBUG_MESSAGE
	printf("ffwd_mq_wait: %p\n", p);
	ffwd_dump_message(p);
#endif
	if (p->status == FFWD_MQ_NULL)
		return 1;
#ifdef MESSAGING_DEBUG_MESSAGE
        printf("ffwd_mq_wait: waiting...  message status = %d\n", p->status);
#endif
	while (p->status != FFWD_MQ_DONE
#ifdef MPILITE_INTERMEDIATE_BUFFER
		 && p->status != FFWD_MQ_RECV_MATCHED
#endif
		)
		__sync_synchronize();	// full memory barrier
#ifdef MESSAGING_DEBUG_MESSAGE
        printf("ffwd_mq_wait: done, or recv_matched\n");
#endif
	return 0;
}

static struct ffwd_message *ffwd_peek(struct ffwd_mq *mq, int gpid /* source */, int tag) {
	struct ffwd_queue *q = &mq->send;
	struct ffwd_message *p = q->tail;
	assert(!p || (p->status < FFWD_MQ_MAX && p->status > FFWD_MQ_MIN));
	while (p && ((tag != FFWD_MQ_ANY_TAG && p->tag != FFWD_MQ_ANY_TAG && p->tag != tag) || (gpid != FFWD_MQ_ANY_SOURCE && p->gpid != FFWD_MQ_ANY_SOURCE && p->gpid != gpid))) {	// tag matching
		p = p->prev;
		assert(!p || (p->status < FFWD_MQ_MAX && p->status > FFWD_MQ_MIN));
	}
	return p;
}


struct ffwd_message *ffwd_mq_probe(int source, int tag, int *count, int *datatype, int *source_gpid, int blocking)
{
	int dest = FFWD_GLOBAL_PID;
	struct ffwd_mq *mq = MQUEUE(source);
	assert(dest < MAX_MQUEUE);
	assert(source < MAX_MQUEUE);

	struct ffwd_message *p;
	do {
		ffwd_lock_acquire(mq->lock);
#ifdef MESSAGING_DEBUG_MESSAGE
	        printf("ffwd_mq_probe: calling ffwd_mq_deq\n");
#endif
		p = ffwd_peek(mq, source, tag);
		if (p && count) {
			*count = p->count;
		}
		if (p && datatype) {
			*datatype = p->datatype;
		}
		if (p && source_gpid) {
			*source_gpid = p->gpid;
		}
		ffwd_lock_release(mq->lock);
	} while (blocking && p == NULL);
	return p;
}


struct ffwd_message *ffwd_mq_send(void *buf, int count, int datatype, int dest, int tag, struct ffwd_message *newp)
{
	int source = FFWD_GLOBAL_PID;
#if 0 //
//	struct ffwd_mq *mq = *(struct ffwd_mq **)ffwd_local_to_remote(&(MQUEUE(source)), dest);
#endif
	struct ffwd_mq *mq = MQUEUE_REMOTE(source, dest);


	assert(dest < MAX_MQUEUE);
	assert(source < MAX_MQUEUE);
	ffwd_lock_acquire(mq->lock);
#ifdef MESSAGING_DEBUG_MESSAGE
        printf("ffwd_mq_send: calling ffwd_mq_deq\n");
#endif
	struct ffwd_message *p = ffwd_mq_deq(&mq->recv, tag, source);// recv already there for me 'source'?
	if (!p) {
#ifdef MESSAGING_DEBUG_MESSAGE
		printf("ffwd_mq_send: couldn't find message..post request.\n");
#endif
		newp->status = FFWD_MQ_SEND;	// send arrived first
		ffwd_mq_enq(newp, &mq->send);
		ffwd_lock_release(mq->lock);
		return NULL;
	}
	ffwd_lock_release(mq->lock);
#ifdef MESSAGING_DEBUG_MESSAGE
        printf("ffwd_mq_send: now %d msg. dest to %d, tag %d, buf:%p\n", mq->recv.count, dest, tag, buf);
#endif
	return p;
}

struct ffwd_message *ffwd_mq_recv(void *buf, int count, int datatype, int source, int tag, struct ffwd_message *newp)
{
	int dest = FFWD_GLOBAL_PID;
	struct ffwd_mq *mq = MQUEUE(source);
	assert(dest < MAX_MQUEUE);
	assert(source < MAX_MQUEUE);
	if (source == FFWD_MQ_ANY_SOURCE) {
		assert(0 && "TODO 'any source' not supported yet\n");
	}

	ffwd_lock_acquire(mq->lock);
#ifdef MESSAGING_DEBUG_MESSAGE
        printf("ffwd_mq_recv: calling ffwd_mq_deq mq %p\n", mq);
#endif
	struct ffwd_message *p = ffwd_mq_deq(&mq->send, tag, source);// send already there from 'source'?
	if (!p) {
#ifdef MESSAGING_DEBUG_MESSAGE
		printf("ffwd_mq_recv: couldn't find message..post request\n");
#endif
		newp->status = FFWD_MQ_RECV;	// recv arrived first
		ffwd_mq_enq(newp, &mq->recv);
		ffwd_lock_release(mq->lock);
		return NULL;
	}
	ffwd_lock_release(mq->lock);
#ifdef MESSAGING_DEBUG_MESSAGE
     printf("ffwd_mq_recv: now %d msg. dest to %d, tag %d, buf:%p\n", mq->send.count, dest, tag, buf);
#endif
	return p;
}

void ffwd_mq_done(struct ffwd_message *send, struct ffwd_message *recv)
{
	if (send->status >= FFWD_MQ_MAX || send->status <= FFWD_MQ_MIN) {
		assert(0 && "ffwd_mq_done: BUG! MPI_Request send is not initialized?\n");
	}
	if (recv->status >= FFWD_MQ_MAX || recv->status <= FFWD_MQ_MIN) {
		assert(0 && "ffwd_mq_done: BUG! MPI_Request recv is not initialized?\n");
	}
	__sync_synchronize();	// _fence_memory();
	send->status = recv->status = FFWD_MQ_DONE;
#ifdef MESSAGING_DEBUG_MESSAGE
        printf("ffwd_mq_done: set send & recv status to done.\n");
#endif
}
