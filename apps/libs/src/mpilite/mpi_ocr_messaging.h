/*
* This file is subject to the license agreement located in the file LICENSE
* and cannot be distributed without it. This notice cannot be
* removed or modified.
*/

#ifndef MPI_OCR_MESSAGING_H
#define MPI_OCR_MESSAGING_H

#define NEW_MQ

struct ffwd_status {    // also modify mpif.h MPI_STATUS_SIZE if the size changes
        int count;
        int datatype;
        int tag;
        int source;
};

struct ffwd_message {
        int op;  // what operation should be done?: send, recv, probe
        int count;
        int datatype;
        int tag;
  //    int gpid;    // dest or source depending on context.. global pid
        int rank;    // dest or source depending on context.. global pid
        int comm;
        int flag;
        volatile int status;
        void *buf;            // global address
#if 0
//    long extra;
    short int pe_in_skt_id;        // in case of qeng
    char index_data;        // in case of qeng
    char index_flag;        // in case of qeng
    int size_qeng;            // in case of qeng
    unsigned int head;        // in case of qeng
    unsigned int tail;        // in case of qeng
    void *buf;            // global address
    struct ffwd_message *prev;    // global address
    struct ffwd_message *next;    // global address
    char *embed[0];
#endif
};

#define FFWD_MQ_MAX             (0)
#define FFWD_MQ_NULL            (-1)
#define FFWD_MQ_INIT            (-2)
#define FFWD_MQ_SEND            (-3)
#define FFWD_MQ_SEND_MATCHED    (-4)    // used for push-only qeng, but not necessarily enabled
#define FFWD_MQ_RECV            (-5)
#define FFWD_MQ_RECV_MATCHED    (-6)    // used for pull-only spad. mandatory.
#define FFWD_MQ_DONE            (-7)
#define FFWD_MQ_MIN             (-8)

#ifdef NEW_MQ
extern struct ffwd_message *ffwd_message_null;
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
void ffwd_mq_init(void);
#ifdef NEW_MQ
struct ffwd_message *ffwd_mq_alloc(int size);
void ffwd_mq_free(struct ffwd_message *p);
struct ffwd_message *ffwd_mq_del_laundry(void);
void ffwd_mq_add_laundry(struct ffwd_message *pp);
#endif

#define FFWD_MQF_INTERMEDIATE   (1UL << 0)
#define FFWD_MQF_QENG           (1UL << 1)
#define FFWD_MQF_QENG_WAIT      (1UL << 2)
#define FFWD_MQF_QENG_HEAD      (1UL << 3)
#define FFWD_MQF_QENG_TAIL      (1UL << 4)
#define FFWD_MQF_EMBEDDED       (1UL << 5)

#define FFWD_MQ_ANY_SOURCE      (-1)
#define FFWD_MQ_ANY_TAG         (-1)

#define FFWD_GLOBAL_MAX         (FlixCurrentDomain()->process_count * FlixCurrentSocket()->domain_count)
#define FFWD_GLOBAL_PID         (FlixCurrentDomain()->process_count * FlixCurrentDomain()->did + FlixCurrentProcess()->pid)

#endif
