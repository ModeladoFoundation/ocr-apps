/*
* This file is subject to the license agreement located in the file LICENSE
* and cannot be distributed without it. This notice cannot be
* removed or modified.
*/

#ifndef FFWD_MESSAGING_H
#define FFWD_MESSAGING_H

#include "mpilite.h"

#define _cycle_count	((long)rdtsc())


struct ffwd_status {	// also modify mpif.h MPI_STATUS_SIZE if the size changes
        int count;
        int datatype;
        int tag;
        int source;
};

#define ffwd_lock_init(X)	do { (X).lock = 0; } while(0)
#define ffwd_lock_acquire(X)	do {if (__sync_bool_compare_and_swap(&((X).lock), 0, 1)) break;} while(1)
#define ffwd_lock_release(X)	do {assert((X).lock == 1);__sync_bool_compare_and_swap(&((X).lock), 1, 0);} while(0)

struct ffwd_lock_t {
	int lock;
};

struct ffwd_message {
	int count;
	int datatype;
	int tag;
	int gpid;	// dest or source depending on context.. global pid
	int flag;
	volatile int status;
	void *buf;			// global address
	void *extra;
	struct ffwd_message *prev;	// global address
	struct ffwd_message *next;	// global address
#ifdef MPILITE_INTERMEDIATE_BUFFER
	char embed[0];
#endif
};
#define FFWD_MQ_MAX		(0)
#define FFWD_MQ_NULL		(-1)
#define FFWD_MQ_INIT		(-2)
#define FFWD_MQ_SEND		(-3)
#define FFWD_MQ_RECV		(-5)
#ifdef MPILITE_INTERMEDIATE_BUFFER
#define FFWD_MQ_RECV_MATCHED	(-6)	// used for intermediate buffer support in case that recv comes first.
#endif
#define FFWD_MQ_DONE		(-7)
#define FFWD_MQ_MIN		(-8)



#ifdef MPILITE_REQUEST_SHADOWING
extern struct ffwd_message *ffwd_message_null;
struct ffwd_message *ffwd_mq_alloc(int size);
void ffwd_mq_free(struct ffwd_message *p);
struct ffwd_message *ffwd_mq_del_laundry(void);
void ffwd_mq_add_laundry(struct ffwd_message *pp);
#else
extern struct ffwd_message ffwd_message_null;
#endif

struct ffwd_message *ffwd_mq_probe(int source, int tag, int *count, int *datatype, int *source_gpid, int blocking);
struct ffwd_message *ffwd_mq_test(int rank, int tag);
struct ffwd_message *ffwd_mq_send(void *buf, int count, int datatype, int dest, int tag, struct ffwd_message *newp);
struct ffwd_message *ffwd_mq_recv(void *buf, int count, int datatype, int source, int tag, struct ffwd_message *newp);
void ffwd_mq_done(struct ffwd_message *send, struct ffwd_message *recv);
int ffwd_mq_wait(struct ffwd_message *p);
void *ffwd_local_to_remote(void *ret, int gpid);
void ffwd_mq_init(void *p);
int ffwd_mq_matrix_size(void);

#define FFWD_MQF_INTERMEDIATE	(1UL << 0)
#ifdef MPILITE_ASYNC_COLLECTIVE
#define FFWD_MQF_REQUEST_SET	(1UL << 1)
#endif

#define FFWD_MQ_ANY_SOURCE	(-1)
#define FFWD_MQ_ANY_TAG		(-1)


#define FFWD_GLOBAL_MAX         mpilite_rank
#define FFWD_GLOBAL_PID         mygpid

#endif
