/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

#ifndef _INTERNALS_H
#define _INTERNALS_H	1

/* Internal data structures */

/* Includes */

#include <limits.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <stackinfo.h>
#include <sys/types.h>
#include <reent.h>
#include "pthread.h"
#include <bits/libc-tsd.h> /* for _LIBC_TSD_KEY_N */
#include <bits/local_lim.h>

#include <sys/tgr.h>

#include "libc-symbols.h"
#include "pt-machine.h"
#include "semaphore.h"
#include "thread_dbP.h"
#include <hp-timing.h>

#ifndef THREAD_GETMEM
# define THREAD_GETMEM(descr, member) descr->member
#endif
#ifndef THREAD_GETMEM_NC
# define THREAD_GETMEM_NC(descr, member) descr->member
#endif
#ifndef THREAD_SETMEM
# define THREAD_SETMEM(descr, member, value) descr->member = (value)
#endif
#ifndef THREAD_SETMEM_NC
# define THREAD_SETMEM_NC(descr, member, value) descr->member = (value)
#endif

/* Arguments passed to thread creation routine */

struct pthread_start_args {
  void * (*start_routine)(void *); /* function to run */
  void * arg;                   /* its argument */
};


/* We keep thread specific data in a special data structure, a two-level
   array.  The top-level array contains pointers to dynamically allocated
   arrays of a certain number of data pointers.  So we can implement a
   sparse array.  Each dynamic second-level array has
	PTHREAD_KEY_2NDLEVEL_SIZE
   entries.  This value shouldn't be too large.  */
#define PTHREAD_KEY_2NDLEVEL_SIZE	32

/* We need to address PTHREAD_KEYS_MAX key with PTHREAD_KEY_2NDLEVEL_SIZE
   keys in each subarray.  */
#define PTHREAD_KEY_1STLEVEL_SIZE \
  ((PTHREAD_KEYS_MAX + PTHREAD_KEY_2NDLEVEL_SIZE - 1) \
   / PTHREAD_KEY_2NDLEVEL_SIZE)

typedef void (*destr_function)(void *);

struct pthread_key_struct {
  int in_use;                   /* already allocated? */
  destr_function destr;         /* destruction routine */
};


#define PTHREAD_START_ARGS_INITIALIZER(fct) \
  { (void *(*) (void *)) fct, NULL }

/* The type of thread descriptors */

typedef struct _pthread_descr_struct * pthread_descr;

/* Callback interface for removing the thread from waiting on an
   object if it is cancelled while waiting or about to wait.
   This hold a pointer to the object, and a pointer to a function
   which ``extricates'' the thread from its enqueued state.
   The function takes two arguments: pointer to the wait object,
   and a pointer to the thread. It returns 1 if an extrication
   actually occured, and hence the thread must also be signalled.
   It returns 0 if the thread had already been extricated. */

typedef struct _pthread_extricate_struct {
    void *pu_object;
    int (*pu_extricate_func)(void *, pthread_descr);
} pthread_extricate_if;

/* Atomic counter made possible by compare_and_swap */

struct pthread_atomic {
  long p_count;
  int p_spinlock;
};

/* Context info for read write locks. The pthread_rwlock_info structure
   is information about a lock that has been read-locked by the thread
   in whose list this structure appears. The pthread_rwlock_context
   is embedded in the thread context and contains a pointer to the
   head of the list of lock info structures, as well as a count of
   read locks that are untracked, because no info structure could be
   allocated for them. */

struct _pthread_rwlock_t;

typedef struct _pthread_rwlock_info {
  struct _pthread_rwlock_info *pr_next;
  struct _pthread_rwlock_t *pr_lock;
  int pr_lock_count;
} pthread_readlock_info;

struct _pthread_descr_struct {
  union {
    struct {
      pthread_descr self;	/* Pointer to this structure */
    } data;
    void *__padding[16];
  } p_header;
  pthread_descr p_nextlive, p_prevlive;
                                /* Double chaining of active threads */
  pthread_descr p_nextwaiting;  /* Next element in the queue holding the thr */
  pthread_descr p_nextlock;	/* can be on a queue and waiting on a lock */
  pthread_t p_tid;              /* Thread identifier */
  pid_t p_pid;                    /* PID of the running XE */
  pid_t p_ppid;                   /* PID of the thread which created us */
  int p_priority;               /* Thread priority (== 0 if not realtime) */
  struct _pthread_fastlock * p_lock; /* Spinlock for synchronized accesses */
  char p_terminated;            /* true if terminated e.g. by pthread_exit */
  char p_detached;              /* true if detached */
  char p_exited;                /* true if the assoc. process terminated */
  void * p_retval;              /* placeholder for return value */
  int p_retcode;                /* placeholder for return code */
  pthread_descr p_joining;      /* thread joining on that thread or NULL */
  struct _pthread_cleanup_buffer * p_cleanup; /* cleanup functions */
  char p_cancelstate;           /* cancellation state */
  char p_canceltype;            /* cancellation type (deferred/async) */
  char p_canceled;              /* cancellation request pending */
  struct _reent * p_reentp;     /* pointer to reent struct */
  struct _reent p_reent;        /* reentrant structure for newlib */
  int * p_h_errnop;             /* pointer to used h_errno variable */
  int p_h_errno;                /* error returned by last netdb function */
  struct pthread_start_args p_start_args; /* arguments for thread creation */
  void ** p_specific[PTHREAD_KEY_1STLEVEL_SIZE]; /* thread-specific data */
  void * p_libc_specific[_LIBC_TSD_KEY_N]; /* thread-specific data for libc */
  int p_nr;                     /* Index of descriptor in __pthread_handles */
  int p_report_events;		/* Nonzero if events must be reported.  */
  td_eventbuf_t p_eventbuf;     /* Data for event.  */
  struct pthread_atomic p_resume_count; /* number of times restart() was
					   called on thread */
  char p_woken_by_cancel;       /* cancellation performed wakeup */
  char p_condvar_avail;		/* flag if conditional variable became avail */
  char p_sem_avail;             /* flag if semaphore became available */
  pthread_extricate_if *p_extricate; /* See above */
  pthread_readlock_info *p_readlock_list;  /* List of readlock info structs */
  pthread_readlock_info *p_readlock_free;  /* Free list of structs */
  int p_untracked_readlock_count;	/* Readlocks not tracked by list */
#if HP_TIMING_AVAIL
  hp_timing_t p_cpuclock_offset; /* Initial CPU clock for thread.  */
#endif
  /* New elements must be added at the end.  */
} __attribute__ ((aligned(32))); /* We need to align the structure so that
				    doubles are aligned properly.  This is 8
				    bytes on MIPS and 16 bytes on MIPS64.
				    32 bytes might give better cache
				    utilization.  */


/* The type of thread handles. */

typedef struct pthread_handle_struct * pthread_handle;

struct pthread_handle_struct {
  struct _pthread_fastlock h_lock; /* Fast lock for sychronized access */
  pthread_descr h_descr;        /* Thread descriptor or NULL if invalid */
  char * h_bottom;              /* Lowest address in the stack thread */
};

//
// Functions which were called by the manager thread, which are now open for use by all threads
//
// Note that some of them contains a critical section of code. No two critical sections may run
// at the same time, so they may block.
//
void __pthread_for_each_thread(void *arg, void (*fn)(void *, pthread_descr));
int __pthread_handle_create(pthread_t *thread, const pthread_attr_t *attr,
                            void * (*start_routine)(void *), void *arg,
                            int report_events, td_thr_events_t *event_maskp);
/* Process-wide exit() */
void __pthread_handle_exit(pthread_descr issuing_thread, int exitcode) __attribute__ ((noreturn));
/* Try to free the resources of a thread when requested by pthread_join
   or pthread_detach on a terminated thread.

   If detatched, then this is the responsibility of the detached thread, else it is the
   responsibility of the joining thread.
   */
void __pthread_handle_free(pthread_t th_id);
void __pthread_do_cancel(void);
/* Handle threads that have exited (responsibility of exiting thread) */
void __pthread_exited();
/* Lock for critical sections of manager code */
extern struct _pthread_fastlock __manager_crit_lock;

/* Signal used for interfacing with gdb */

extern int __pthread_sig_debug;

/* Global array of thread handles, used for validating a thread id
   and retrieving the corresponding thread descriptor. Also used for
   mapping the available stack segments. */

extern struct pthread_handle_struct __pthread_handles[PTHREAD_THREADS_MAX];

/* Descriptor of the initial thread */

extern struct _pthread_descr_struct __pthread_initial_thread;

/* Descriptor of the main thread */

extern pthread_descr __pthread_main_thread;

/* Pending request for a process-wide exit */

extern int __pthread_exit_requested, __pthread_exit_code;

/* Set to 1 by gdb if we're debugging */

extern volatile int __pthread_threads_debug;

/* Globally enabled events.  */
extern volatile td_thr_events_t __pthread_threads_events;

/* Pointer to descriptor of thread with last event.  */
extern volatile pthread_descr __pthread_last_event;

/* Flag which tells whether we are executing on SMP kernel. */
extern const int __pthread_smp_kernel;

/* Return the handle corresponding to a thread id */

static inline pthread_handle thread_handle(pthread_t id)
{
  return &__pthread_handles[id % PTHREAD_THREADS_MAX];
}

/* Validate a thread handle. Must have acquired h->h_spinlock before. */

static inline int invalid_handle(pthread_handle h, pthread_t id)
{
  return h->h_descr == NULL || h->h_descr->p_tid != id || h->h_descr->p_terminated;
}

static inline int nonexisting_handle(pthread_handle h, pthread_t id)
{
  return h->h_descr == NULL || h->h_descr->p_tid != id;
}

/* The page size we can get from the system.  This should likely not be
   changed by the machine file but, you never know.  */
#ifndef PAGE_SIZE
#define PAGE_SIZE  (tgr_getpagesize())
#endif

/* The max size of the thread stack segments.  If the default
   THREAD_SELF implementation is used, this must be a power of two and
   a multiple of PAGE_SIZE.  */
#ifndef STACK_SIZE
#define STACK_SIZE  (PAGE_SIZE * 1024)
#endif

/* The initial size of the thread stack.  Must be a multiple of PAGE_SIZE.  */
#ifndef INITIAL_STACK_SIZE
#define INITIAL_STACK_SIZE  (4 * PAGE_SIZE)
#endif

/* Recover thread descriptor for the current thread */

extern pthread_descr __pthread_find_self (void) __attribute__ ((const));

static inline pthread_descr thread_self (void) __attribute__ ((const));
static inline pthread_descr thread_self (void)
{
  if (__pthread_initial_thread.p_pid == 0 || /* not initialized yet, so we must be the only thread */
      tgr_getpid() == __pthread_initial_thread.p_pid)
    return &__pthread_initial_thread;
  else
    return __pthread_find_self();
}

/* If MEMORY_BARRIER isn't defined in pt-machine.h, assume the architecture
   doesn't need a memory barrier instruction (e.g. Intel x86).  Still we
   need the compiler to respect the barrier and emit all outstanding
   operations which modify memory.  Some architectures distinguish between
   full, read and write barriers.  */

#ifndef MEMORY_BARRIER
#define MEMORY_BARRIER() __asm__ ("" : : : "memory")
#endif
#ifndef READ_MEMORY_BARRIER
#define READ_MEMORY_BARRIER() MEMORY_BARRIER()
#endif
#ifndef WRITE_MEMORY_BARRIER
#define WRITE_MEMORY_BARRIER() MEMORY_BARRIER()
#endif

/* Max number of times we must spin on a spinlock calling sched_yield().
   After MAX_SPIN_COUNT iterations, we put the calling thread to sleep. */

#ifndef MAX_SPIN_COUNT
#define MAX_SPIN_COUNT 50
#endif

/* Max number of times the spinlock in the adaptive mutex implementation
   spins actively on SMP systems.  */

#ifndef MAX_ADAPTIVE_SPIN_COUNT
#define MAX_ADAPTIVE_SPIN_COUNT 100
#endif

/* Duration of sleep (in nanoseconds) when we can't acquire a spinlock
   after MAX_SPIN_COUNT iterations of sched_yield().
   With the 2.0 and 2.1 kernels, this MUST BE > 2ms.
   (Otherwise the kernel does busy-waiting for realtime threads,
    giving other threads no chance to run.) */

#ifndef SPIN_SLEEP_DURATION
#define SPIN_SLEEP_DURATION 2000001
#endif

/* Debugging */

#ifdef DEBUG
#include <assert.h>
#define ASSERT assert
#define MSG __pthread_message
#else
#define ASSERT(x)
#define MSG(msg,arg...)
#endif

/* Internal global functions */

extern void __pthread_do_exit (void *retval, char *currentframe)
     __attribute__ ((__noreturn__));
extern void __pthread_destroy_specifics (void);
extern void __pthread_perform_cleanup (char *currentframe);
extern void __pthread_reset_main_thread (void);
extern void __pthread_once_fork_prepare (void);
extern void __pthread_once_fork_parent (void);
extern void __pthread_once_fork_child (void);
extern void __flockfilelist (void);
extern void __funlockfilelist (void);
extern void __fresetlockfiles (void);
extern void __pthread_initialize_minimal (void);

extern int __pthread_attr_setguardsize (pthread_attr_t *__attr,
					size_t __guardsize);
extern int __pthread_attr_getguardsize (const pthread_attr_t *__attr,
					size_t *__guardsize);
extern int __pthread_attr_setstackaddr (pthread_attr_t *__attr,
					void *__stackaddr);
extern int __pthread_attr_getstackaddr (const pthread_attr_t *__attr,
					void **__stackaddr);
extern int __pthread_attr_setstacksize (pthread_attr_t *__attr,
					size_t __stacksize);
extern int __pthread_attr_getstacksize (const pthread_attr_t *__attr,
					size_t *__stacksize);
extern int __pthread_attr_setstack (pthread_attr_t *__attr, void *__stackaddr,
				    size_t __stacksize);
extern int __pthread_attr_getstack (const pthread_attr_t *__attr, void **__stackaddr,
				    size_t *__stacksize);
extern int __pthread_getconcurrency (void);
extern int __pthread_setconcurrency (int __level);
extern int __pthread_mutex_timedlock (pthread_mutex_t *__mutex,
				      const struct timespec *__abstime);
extern int __pthread_mutexattr_getpshared (const pthread_mutexattr_t *__attr,
					   int *__pshared);
extern int __pthread_mutexattr_setpshared (pthread_mutexattr_t *__attr,
					   int __pshared);
extern int __pthread_mutexattr_gettype (const pthread_mutexattr_t *__attr,
					int *__kind);

extern int __pthread_yield (void);

extern int __pthread_rwlock_timedrdlock (pthread_rwlock_t *__restrict __rwlock,
					 __const struct timespec *__restrict
					 __abstime);
extern int __pthread_rwlock_timedwrlock (pthread_rwlock_t *__restrict __rwlock,
					 __const struct timespec *__restrict
					 __abstime);
extern int __pthread_rwlockattr_destroy (pthread_rwlockattr_t *__attr);

extern int __pthread_barrierattr_getpshared (__const pthread_barrierattr_t *
					     __restrict __attr,
					     int *__restrict __pshared);

extern int __pthread_clock_gettime (hp_timing_t freq, struct timespec *tp);
extern void __pthread_clock_settime (hp_timing_t offset);


/* Global pointers to old or new suspend functions */

extern void (*__pthread_restart)(pthread_descr);
extern void (*__pthread_suspend)(pthread_descr);
extern int (*__pthread_timedsuspend)(pthread_descr, const struct timespec *);

/* Prototypes for some of the new semaphore functions.  */
extern int __new_sem_post (sem_t * sem);
extern int __new_sem_init (sem_t *__sem, int __pshared, unsigned int __value);
extern int __new_sem_wait (sem_t *__sem);
extern int __new_sem_trywait (sem_t *__sem);
extern int __new_sem_getvalue (sem_t *__restrict __sem, int *__restrict __sval);
extern int __new_sem_destroy (sem_t *__sem);

/* Prototypes for compatibility functions.  */
extern int __pthread_attr_init_2_1 (pthread_attr_t *__attr);
extern int __pthread_attr_init_2_0 (pthread_attr_t *__attr);
extern int __pthread_create_2_1 (pthread_t *__restrict __thread1,
				 const pthread_attr_t *__attr,
				 void *(*__start_routine) (void *),
				 void *__restrict __arg);
extern int __pthread_create_2_0 (pthread_t *__restrict __thread1,
				 const pthread_attr_t *__attr,
				 void *(*__start_routine) (void *),
				 void *__restrict arg);

/* The functions called the signal events.  */
extern void __linuxthreads_create_event (void);
extern void __linuxthreads_death_event (void);
extern void __linuxthreads_reap_event (void);

/* This function is called to initialize the pthread library.  */
extern void __pthread_initialize (void);

#endif /* internals.h */
