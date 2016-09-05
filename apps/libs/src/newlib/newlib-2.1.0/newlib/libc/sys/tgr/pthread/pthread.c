
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

/* Thread creation, initialization, and basic low-level routines */

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <poll.h>
#include <shlib-compat.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "restart.h"

/* These variables are used by the setup code.  */

/* Descriptor of the initial thread */

struct _pthread_descr_struct __pthread_initial_thread = {
  {
    {
      &__pthread_initial_thread /* pthread_descr self */
    }
  },
  &__pthread_initial_thread,  /* pthread_descr p_nextlive */
  &__pthread_initial_thread,  /* pthread_descr p_prevlive */
  NULL,                       /* pthread_descr p_nextwaiting */
  NULL,			      /* pthread_descr p_nextlock */
  PTHREAD_THREADS_MAX,        /* pthread_t p_tid */
  0,                          /* int p_pid */
  0,                          /* int p_ppid */
  0,                          /* int p_priority */
  &__pthread_handles[0].h_lock, /* struct _pthread_fastlock * p_lock */
  0,                          /* char p_terminated */
  0,                          /* char p_detached */
  0,                          /* char p_exited */
  NULL,                       /* void * p_retval */
  0,                          /* int p_retval */
  NULL,                       /* pthread_descr p_joining */
  NULL,                       /* struct _pthread_cleanup_buffer * p_cleanup */
  0,                          /* char p_cancelstate */
  0,                          /* char p_canceltype */
  0,                          /* char p_canceled */
  &__pthread_initial_thread.p_reent, /* struct _reent *p_reentp */
  _REENT_INIT(__pthread_initial_thread.p_reent),  /* struct _reent p_reent */
  NULL,                       /* int *p_h_errnop */
  0,                          /* int p_h_errno */
  PTHREAD_START_ARGS_INITIALIZER(NULL),
                              /* struct pthread_start_args p_start_args */
  {NULL},                     /* void ** p_specific[PTHREAD_KEY_1STLEVEL_SIZE] */
  {NULL},                     /* void * p_libc_specific[_LIBC_TSD_KEY_N] */
  0,                          /* Always index 0 */
  0,                          /* int p_report_events */
  {{{0, }}, 0, NULL},         /* td_eventbuf_t p_eventbuf */
  __ATOMIC_INITIALIZER,       /* struct pthread_atomic p_resume_count */
  0,                          /* char p_woken_by_cancel */
  0,                          /* char p_condvar_avail */
  0,                          /* char p_sem_avail */
  NULL,                       /* struct pthread_extricate_if *p_extricate */
  NULL,	                      /* pthread_readlock_info *p_readlock_list; */
  NULL,                       /* pthread_readlock_info *p_readlock_free; */
  0                           /* int p_untracked_readlock_count; */
};


/* Pointer to the main thread (the father of the thread manager thread) */
/* Originally, this is the initial thread, but this changes after fork() */

pthread_descr __pthread_main_thread = &__pthread_initial_thread;

/* For process-wide exit() */

int __pthread_exit_requested;
int __pthread_exit_code;

/* Maximum stack size.  */
size_t __pthread_max_stacksize;

/* Nozero if the machine has more than one processor.  */
const int __pthread_smp_kernel = 1;

/* Communicate relevant LinuxThreads constants to gdb */

const int __pthread_threads_max = PTHREAD_THREADS_MAX;
const int __pthread_sizeof_handle = sizeof(struct pthread_handle_struct);
const int __pthread_offsetof_descr = offsetof(struct pthread_handle_struct,
                                              h_descr);
const int __pthread_offsetof_pid = offsetof(struct _pthread_descr_struct,
                                            p_pid);
const int __linuxthreads_pthread_sizeof_descr
  = sizeof(struct _pthread_descr_struct);

/* Forward declarations */

static void pthread_onexit_process(int retcode, void *arg);
#ifndef HAVE_Z_NODELETE
static void pthread_atexit_process(void *arg, int retcode);
static void pthread_atexit_retcode(void *arg, int retcode);
#endif
static void pthread_handle_sigdebug(int sig);

/* CPU clock handling.  */
#if HP_TIMING_AVAIL
extern hp_timing_t _dl_cpuclock_offset;
#endif

/* Initialize the pthread library.
   Initialization is split in two functions:
   - a constructor function that blocks the __pthread_sig_restart signal
     (must do this very early, since the program could capture the signal
      mask with e.g. sigsetjmp before creating the first thread);
   - a regular function called from pthread_create when needed. */

static void pthread_initialize(void) __attribute__((constructor));

#ifndef HAVE_Z_NODELETE
extern void *__dso_handle __attribute__ ((weak));
#endif


/* Do some minimal initialization which has to be done during the
   startup of the C library.  */
void
__pthread_initialize_minimal(void)
{
  /* If we have special thread_self processing, initialize that for the
     main thread now.  */
#ifdef INIT_THREAD_SELF
  INIT_THREAD_SELF(&__pthread_initial_thread, 0);
#endif
#if HP_TIMING_AVAIL
  __pthread_initial_thread.p_cpuclock_offset = _dl_cpuclock_offset;
#endif
}


void
__pthread_init_max_stacksize(void)
{
  size_t max_stack;

#ifdef FLOATING_STACKS
  struct rlimit limit;
  getrlimit(RLIMIT_STACK, &limit);
  if (limit.rlim_cur == RLIM_INFINITY)
    limit.rlim_cur = ARCH_STACK_MAX_SIZE;
# ifdef NEED_SEPARATE_REGISTER_STACK
  max_stack = limit.rlim_cur / 2;
# else
  max_stack = limit.rlim_cur;
# endif
#else
  /* Play with the stack size limit to make sure that no stack ever grows
     beyond STACK_SIZE minus one page (to act as a guard page). */
# ifdef NEED_SEPARATE_REGISTER_STACK
  /* STACK_SIZE bytes hold both the main stack and register backing
     store. The rlimit value applies to each individually.  */
  max_stack = STACK_SIZE/2 - tgr_getpagesize();
# else
  max_stack = STACK_SIZE - tgr_getpagesize();
# endif
#endif
  __pthread_max_stacksize = max_stack;
}


static void pthread_initialize(void)
{
  struct sigaction sa;
  sigset_t mask;

  /* If already done (e.g. by a constructor called earlier!), bail out */
  if (__pthread_initial_thread.p_pid == getpid()) return;
#ifdef TEST_FOR_COMPARE_AND_SWAP
  /* Test if compare-and-swap is available */
  __pthread_has_cas = compare_and_swap_is_available();
#endif
  /* Determine stack size limits .  */
  __pthread_init_max_stacksize ();
  /* Update the descriptor for the initial thread. */
  __pthread_initial_thread.p_pid = getpid();

  /* Initialize the lock for the critical sections of "manager" code */
  __pthread_init_lock(&__manager_crit_lock);

  /* Register an exit function to kill all other threads. */
  /* Do it early so that user-registered atexit functions are called
     before pthread_*exit_process. */
#ifndef HAVE_Z_NODELETE
  if (__builtin_expect (&__dso_handle != NULL, 1))
    __cxa_atexit ((void (*) (void *)) pthread_atexit_process, NULL,
		  __dso_handle);
  else
#endif
    on_exit (pthread_onexit_process, NULL);
}

void __pthread_initialize(void)
{
  pthread_initialize();
}

/* Thread creation */

int __pthread_create_2_1(pthread_t *thread, const pthread_attr_t *attr,
			 void * (*start_routine)(void *), void *arg)
{
  pthread_descr self = thread_self();
  int retval;
  retval = __pthread_handle_create((pthread_t *)&self->p_retval,
                                   attr, start_routine, arg,
                                   self->p_report_events,
                                   &self->p_eventbuf.eventmask);
  if (__builtin_expect (retval, 0) == 0)
    *thread = (pthread_t) THREAD_GETMEM(self, p_retval);
  return retval;
}

versioned_symbol (libpthread, __pthread_create_2_1, pthread_create, GLIBC_2_1);

#if SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_1)

int __pthread_create_2_0(pthread_t *thread, const pthread_attr_t *attr,
			 void * (*start_routine)(void *), void *arg)
{
  /* The ATTR attribute is not really of type `pthread_attr_t *'.  It has
     the old size and access to the new members might crash the program.
     We convert the struct now.  */
  pthread_attr_t new_attr;

  if (attr != NULL)
    {
      size_t ps = tgr_getpagesize()

      memcpy (&new_attr, attr,
	      (size_t) &(((pthread_attr_t*)NULL)->__guardsize));
      new_attr.__guardsize = ps;
      new_attr.__stackaddr_set = 0;
      new_attr.__stackaddr = NULL;
      new_attr.__stacksize = STACK_SIZE - ps;
      attr = &new_attr;
    }
  return __pthread_create_2_1 (thread, attr, start_routine, arg);
}
compat_symbol (libpthread, __pthread_create_2_0, pthread_create, GLIBC_2_0);
#endif

/* Simple operations on thread identifiers */

pthread_t pthread_self(void)
{
  pthread_descr self = thread_self();
  return THREAD_GETMEM(self, p_tid);
}

int pthread_equal(pthread_t thread1, pthread_t thread2)
{
  return thread1 == thread2;
}

/* Helper function for thread_self in the case of user-provided stacks */

pthread_descr __pthread_find_self(void)
{
  pthread_handle h;

  /* __pthread_handles[0] is the initial thread, so start at 1 */
  h = __pthread_handles + 1;
  pid_t pid = tgr_getpid();
  while (h->h_descr == NULL || h->h_descr->p_pid != pid)
      h++;
  return h->h_descr;
}

int __pthread_yield (void)
{
  /* For now this is unimplemented */
  errno = ENOSYS;
  return -1;
}
weak_alias (__pthread_yield, pthread_yield)

/* Process-wide exit() request */

static void pthread_onexit_process(int retcode, void *arg)
{
    pthread_descr self = thread_self();
    if (self == __pthread_main_thread)
      {
    tgr_killall();
      }
}

#ifndef HAVE_Z_NODELETE
static int __pthread_atexit_retcode;

static void pthread_atexit_process(void *arg, int retcode)
{
  pthread_onexit_process (retcode ?: __pthread_atexit_retcode, arg);
}

static void pthread_atexit_retcode(void *arg, int retcode)
{
  __pthread_atexit_retcode = retcode;
}
#endif

void __pthread_do_cancel(void)
{
  pthread_descr self = thread_self();

  if (__builtin_expect (__pthread_exit_requested, 0)) {
    if (self == __pthread_main_thread) {
      tgr_waitall();
    }
    _exit(__pthread_exit_code);
  }
  __pthread_do_exit(PTHREAD_CANCELED, CURRENT_STACK_FRAME);
  __builtin_unreachable();
}

/* Handler for the DEBUG signal.
   The debugging strategy is as follows:
   On reception of a REQ_DEBUG request (sent by new threads created to
   the thread manager under debugging mode), the thread manager throws
   __pthread_sig_debug to itself. The debugger (if active) intercepts
   this signal, takes into account new threads and continue execution
   of the thread manager by propagating the signal because it doesn't
   know what it is specifically done for. In the current implementation,
   the thread manager simply discards it. */

static void pthread_handle_sigdebug(int sig)
{
  /* Nothing */
}

/* Reset the state of the thread machinery after a fork().
   Close the pipe used for requests and set the main thread to the forked
   thread.
   Notice that we can't free the stack segments, as the forked thread
   may hold pointers into them. */

void __pthread_reset_main_thread(void)
{
  pthread_descr self = thread_self();

  /* Update the pid of the main thread */
  THREAD_SETMEM(self, p_pid, getpid());
  /* Make the forked thread the main thread */
  __pthread_main_thread = self;
  THREAD_SETMEM(self, p_nextlive, self);
  THREAD_SETMEM(self, p_prevlive, self);
  /* Now this thread modifies the global variables.  */
}

/* Concurrency symbol level.  */
static int current_level;

int __pthread_setconcurrency(int level)
{
  /* We don't do anything unless we have found a useful interpretation.  */
  current_level = level;
  return 0;
}
weak_alias (__pthread_setconcurrency, pthread_setconcurrency)

int __pthread_getconcurrency(void)
{
  return current_level;
}
weak_alias (__pthread_getconcurrency, pthread_getconcurrency)

/* Debugging aid */

#ifdef DEBUG
#include <stdarg.h>

void __pthread_message(char * fmt, ...)
{
  char buffer[1024];
  va_list args;
  sprintf(buffer, "%05d : ", getpid());
  va_start(args, fmt);
  vsnprintf(buffer + 8, sizeof(buffer) - 8, fmt, args);
  va_end(args);
  TEMP_FAILURE_RETRY(write(2, buffer, strlen(buffer)));
}

#endif

#ifndef SHARED
/* We need a hook to force the file locking and getreent
   to be linked in when static libpthread is used.  */
extern const int __pthread_provide_lockfile;
static const int *const __pthread_require_lockfile =
  &__pthread_provide_lockfile;
extern const int __pthread_provide_getreent;
static const int *const __pthread_require_getreent =
  &__pthread_provide_getreent;
#endif
