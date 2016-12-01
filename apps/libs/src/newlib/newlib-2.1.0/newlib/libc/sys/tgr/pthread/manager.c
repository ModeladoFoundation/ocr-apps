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

/* This code was previously run by the "thread manager" thread.
 * It was used to manage the creation and termination of threads.
 * Now threads themselves call these functions directly. Race conditions
 * which could occure when these functions are not only called from one
 * thread (the manager) are prevented by wrapping much of this code in
 * __MANAGER_CRIT_START() and __MANAGER_CRIT_END().
 *
 * Any references in comments seen which refer to the manager thread now
 * refer to the methods in this file.
 */

#include <errno.h>
#define __USE_MISC
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/time.h>

#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "restart.h"
#include "semaphore.h"

#include <machine/lea.h>

#include <sys/tgr.h>

/* Array of active threads. Entry 0 is reserved for the initial thread. */
struct pthread_handle_struct __pthread_handles[PTHREAD_THREADS_MAX] =
{ { __LOCK_INITIALIZER, &__pthread_initial_thread, 0} };

/* For debugging purposes put the maximum number of threads in a variable.  */
const int __linuxthreads_pthread_threads_max = PTHREAD_THREADS_MAX;

/* Number of active entries in __pthread_handles (used by gdb) */
volatile int __pthread_handles_num = 1;

/* Whether to use debugger additional actions for thread creation
   (set to 1 by gdb) */
volatile int __pthread_threads_debug;

/* Globally enabled events.  */
volatile td_thr_events_t __pthread_threads_events;

/* Pointer to thread descriptor with last event.  */
volatile pthread_descr __pthread_last_event;

/* Counter used to generate unique thread identifier.
   Thread identifier is pthread_threads_counter + segment. */

static pthread_t pthread_threads_counter;

/* Lock for critical sections of manager code */
struct _pthread_fastlock __manager_crit_lock;
static pthread_descr manager_thread;
#define __MANAGER_CRIT_START() do {__pthread_lock(&__manager_crit_lock, thread_self()); manager_thread = thread_self(); } while (0)
#define __MANAGER_CRIT_END() do {__pthread_unlock(&__manager_crit_lock); manager_thread = NULL; } while (0)


/* This acts like the start() function in crt0.c for a new thread. It makes
 * a copy of the pthread_descr sent to it, sets it up, and registers it with
 * the __pthread_handles and active thread lists. Then it calls the function for
 * the child thread with the supplied argument. It also handles the cleanup
 * the child thread is responcible fore after the function completes.
 */
static s64
__attribute__ ((noreturn))
pthread_start_thread(void *arg)
{
  pthread_descr p_self_img = (pthread_descr) arg;
  void * outcome;
#if HP_TIMING_AVAIL
  hp_timing_t tmpclock;
#endif
  /* Initialize special thread_self processing, if any.  */
#ifdef INIT_THREAD_SELF
  INIT_THREAD_SELF(p_self_img, p_self_img->p_nr);
#endif
#if HP_TIMING_AVAIL
  HP_TIMING_NOW (tmpclock);
  THREAD_SETMEM (p_self_img, p_cpuclock_offset, tmpclock);
#endif

  /* Make our own copy of self.
   *
   * Note that this lives in this stack frame, but is going to be made available
   * to all the threads via __pthread_handles. It is OK that this struct lives in
   * this frame because this function *never* returns. This is important!
   */
  struct _pthread_descr_struct self;
  memcpy(&self, p_self_img, sizeof (struct _pthread_descr_struct));

  /* Set up internal pointers */
  self.p_header.data.self = xstg_lea(&self);
  self.p_reentp = xstg_lea(&self.p_reent);
  self.p_h_errnop = xstg_lea(&self.p_h_errno);
  _REENT_INIT_PTR(self.p_reentp);

  __pthread_handles[self.p_nr].h_descr = xstg_lea(&self);

  /* Insert new thread in doubly linked list of active threads */
  self.p_prevlive = __pthread_main_thread;
  self.p_nextlive = __pthread_main_thread->p_nextlive;
  __pthread_main_thread->p_nextlive->p_prevlive = xstg_lea(&self);
  __pthread_main_thread->p_nextlive = xstg_lea(&self);

  /* Set the new thread's pid */
  self.p_pid = tgr_getpid();

  /* Generate the xstg reent */
  if (tgr_sizeof_reent()) {
    self.p_reent.__tgr_reent = malloc(tgr_sizeof_reent());
    memset(self.p_reent.__tgr_reent, 0, tgr_sizeof_reent()); /*zero the tgr reent*/
  }
  /* else tgr doesn't require a reent / sets it up itself. */

  /* Test if we were to be created detached */
  if (self.p_detached) {
    tgr_detach(self.p_pid);
  }

  /* Set the atexit to clean up this thread */
  atexit(__pthread_exited);

  /* Set up the TLS */
  __pthread_init_tls(&self);

  /* Set this to NULL to inform creating thread that we have finished copying */
  p_self_img->p_header.data.self = NULL;
  tgr_resume(self.p_ppid);

  /* Run the thread code */
  outcome = self.p_start_args.start_routine(self.p_start_args.arg);
  /* Exit with the given return value */
  __pthread_do_exit(outcome, CURRENT_STACK_FRAME);
}

/* The Code which uses this is not currently implemented */
#if 0
static int
__attribute__ ((noreturn))
pthread_start_thread_event(void *arg)
{
  pthread_descr self = (pthread_descr) arg;

#ifdef INIT_THREAD_SELF
  INIT_THREAD_SELF(self, self->p_nr);
#endif
  /* Make sure our pid field is initialized, just in case we get there
     before our father has initialized it. */
  THREAD_SETMEM(self, p_pid, getpid());
  /* Get the lock the manager will free once all is correctly set up.  */
  __pthread_lock (THREAD_GETMEM(self, p_lock), NULL);
  /* Free it immediately.  */
  __pthread_unlock (THREAD_GETMEM(self, p_lock));

  /* Continue with the real function.  */
  pthread_start_thread (arg);
}
#endif

/* This launches a thread and blocks until it is set up and running */
int __pthread_handle_create(pthread_t *thread, const pthread_attr_t *attr,
				 void * (*start_routine)(void *), void *arg,
				 int report_events, td_thr_events_t *event_maskp)
{
__MANAGER_CRIT_START();
  size_t sseg;
  pid_t pid;
  struct _pthread_descr_struct new_thread;
  pthread_t new_thread_id;

  // Clear the new thread structure before doing anything with it.
  memset(&new_thread, 0, sizeof(new_thread));

  /* Find a free segment for the thread, and allocate a stack if needed */
  for (sseg = 1; ; sseg++)
    {
      if (sseg >= PTHREAD_THREADS_MAX) {
        __MANAGER_CRIT_END();
	    return EAGAIN;
      }
      if (__pthread_handles[sseg].h_descr == NULL)
	break;
    }
  __pthread_handles_num++;
  /* Allocate new thread identifier */
  pthread_threads_counter += PTHREAD_THREADS_MAX;
  new_thread_id = sseg + pthread_threads_counter;
  /* Initialize the thread descriptor.  Elements which have to be
     initialized to zero already have this value.  */
  new_thread.p_header.data.self = &new_thread;
  new_thread.p_tid = new_thread_id;
  new_thread.p_lock = &(__pthread_handles[sseg].h_lock);
  new_thread.p_cancelstate = PTHREAD_CANCEL_ENABLE;
  new_thread.p_canceltype = PTHREAD_CANCEL_DEFERRED;
  new_thread.p_header.data.self = &new_thread;
  new_thread.p_nr = sseg;
  new_thread.p_ppid = tgr_getpid();
  /* Initialize the thread handle */
  __pthread_init_lock(&__pthread_handles[sseg].h_lock);
  if (attr != NULL) {
    new_thread.p_detached = attr->__detachstate;
  }
  /* Finish setting up arguments to pthread_start_thread */
  new_thread.p_start_args.start_routine = start_routine;
  new_thread.p_start_args.arg = arg;
  /* Make the new thread ID available already now.  If any of the later
     functions fail we return an error value and the caller must not use
     the stored thread ID.  */
  *thread = new_thread_id;
  /* Do the cloning.  We have to use two different functions depending
     on whether we are debugging or not.  */
  pid = 0;	/* Note that the thread never can have PID zero.  */

#if 0 //Unimplemented
  if (report_events)
    {
      /* See whether the TD_CREATE event bit is set in any of the
         masks.  */
      int idx = __td_eventword (TD_CREATE);
      uint32_t mask = __td_eventmask (TD_CREATE);

      if ((mask & (__pthread_threads_events.event_bits[idx]
		   | event_maskp->event_bits[idx])) != 0)
	{

	  /* We have to report this event.  */
	  pid = clone(pthread_start_thread, xstg_lea(&new_thread));
	  if (pid != -1)
	    {
	      /* Now fill in the information about the new thread in
		 the newly created thread's data structure.  We cannot let
		 the new thread do this since we don't know whether it was
		 already scheduled when we send the event.  */
	      new_thread.p_eventbuf.eventdata = &new_thread;
	      new_thread.p_eventbuf.eventnum = TD_CREATE;
	      __pthread_last_event = &new_thread;

	      new_thread.p_pid = pid;

	      /* Now call the function which signals the event.  */
	      __linuxthreads_create_event ();
	    }
	}
    }
#endif
  if (pid == 0)
    {
	  tgr_clone(pthread_start_thread, xstg_lea(&new_thread), &pid);
    }

  /* Check if cloning succeeded */
  if (pid == -1) {
    __pthread_handles[sseg].h_descr = NULL;
    __pthread_handles[sseg].h_bottom = NULL;
    __pthread_handles_num--;
    __MANAGER_CRIT_END();
    return errno;
  }
  /* Wait for it to consume the new_thread data */
  while (new_thread.p_header.data.self == &new_thread) {
      suspend();
  }

  __MANAGER_CRIT_END();
  return 0;
}


/* Try to free the resources of a thread when requested by pthread_join
   or pthread_detach on a terminated thread. */

static void pthread_free(pthread_descr th)
{
  pthread_handle handle;
  pthread_readlock_info *iter, *next;

  ASSERT(th->p_exited);
  /* Make the handle invalid */
  handle =  thread_handle(th->p_tid);
  __pthread_lock(&handle->h_lock, NULL);
  handle->h_descr = NULL;
  handle->h_bottom = (char *)(-1L);
  __pthread_unlock(&handle->h_lock);
#ifdef FREE_THREAD
  FREE_THREAD(th, th->p_nr);
#endif
  /* One fewer threads in __pthread_handles */
  __pthread_handles_num--;

  /* Destroy read lock list, and list of free read lock structures.
     If the former is not empty, it means the thread exited while
     holding read locks! */

  for (iter = th->p_readlock_list; iter != NULL; iter = next)
    {
      next = iter->pr_next;
      free(iter);
    }

  for (iter = th->p_readlock_free; iter != NULL; iter = next)
    {
      next = iter->pr_next;
      free(iter);
    }
}

void __pthread_exited()
{
  __MANAGER_CRIT_START();
  pthread_descr self = thread_self();
  int detached;
      /* Remove thread from list of active threads */
      self->p_nextlive->p_prevlive = self->p_prevlive;
      self->p_prevlive->p_nextlive = self->p_nextlive;
      /* Mark thread as exited, and if detached, free its resources */
      __pthread_lock(self->p_lock, NULL);
      self->p_exited = 1;
      /* If we have to signal this event do it now.  */
      if (self->p_report_events)
	{
	  /* See whether TD_REAP is in any of the mask.  */
	  int idx = __td_eventword (TD_REAP);
	  uint32_t mask = __td_eventmask (TD_REAP);

	  if ((mask & (__pthread_threads_events.event_bits[idx]
		       | self->p_eventbuf.eventmask.event_bits[idx])) != 0)
	    {
	      /* Yep, we have to signal the reapage.  */
	      self->p_eventbuf.eventnum = TD_REAP;
	      self->p_eventbuf.eventdata = self;
	      __pthread_last_event = self;

	      /* Now call the function to signal the event.  */
	      __linuxthreads_reap_event();
	    }
	}
      detached = self->p_detached;
      __pthread_unlock(self->p_lock);
  __MANAGER_CRIT_END();

      if (detached)
        __pthread_handle_free(self->p_tid);
}

static void pthread_handle_free_self(void) {
    pthread_descr self = thread_self();
    self->p_exited = 1;
    __pthread_handle_free(self->p_tid);
}

/* Try to free the resources of a thread when requested by pthread_join
   or pthread_detach on a terminated thread. */

void __pthread_handle_free(pthread_t th_id)
{
  __MANAGER_CRIT_START();
  pthread_handle handle = thread_handle(th_id);
  pthread_descr th;

  __pthread_lock(&handle->h_lock, NULL);
  if (nonexisting_handle(handle, th_id)) {
    /* nothing needs to be done */
    __pthread_unlock(&handle->h_lock);
    return;
  }
  th = handle->h_descr;
  if (th->p_exited) {
    if (!th->p_detached && th != __pthread_main_thread) {
      /* wait for thread to fully exit and be cleaned */
      tgr_waitpid(th->p_pid, NULL, 1);
    }
    __pthread_unlock(&handle->h_lock);
    pthread_free(th);
  } else {
    /* The process of the thread is still running.
       Mark the thread as detached and set it up to clean
       itself and deallocate its resources when the process exits. */
    th->p_detached = 1;
    tgr_detach(th->p_pid);
    atexit(pthread_handle_free_self);
    __pthread_unlock(&handle->h_lock);
  }
  __MANAGER_CRIT_END();
}

void __pthread_for_each_thread(void *arg,
    void (*fn)(void *, pthread_descr))
{
  pthread_descr th;

__MANAGER_CRIT_START();
  for (th = __pthread_main_thread->p_nextlive;
       th != __pthread_main_thread;
       th = th->p_nextlive) {
    fn(arg, th);
  }
__MANAGER_CRIT_END();

  fn(arg, __pthread_main_thread);
}

/* Process-wide exit() */

void __pthread_handle_exit(pthread_descr issuing_thread, int exitcode)
{
  __MANAGER_CRIT_START();
  pthread_descr th;
  __pthread_exit_requested = 1;
  __pthread_exit_code = exitcode;
  /* A forced asynchronous cancellation follows.  Make sure we won't
     get stuck later in the main thread with a system lock being held
     by one of the cancelled threads.  Ideally one would use the same
     code as in pthread_atfork(), but we can't distinguish system and
     user handlers there.  */
  __flockfilelist();
  /* Send the CANCEL signal to all running threads, including the main
     thread, but excluding the thread from which the exit request originated
     (that thread must complete the exit, e.g. calling atexit functions
     and flushing stdio buffers). */
  for (th = issuing_thread->p_nextlive;
       th != issuing_thread;
       th = th->p_nextlive) {
      tgr_cancel_pid(th->p_pid, __pthread_do_cancel);
  }
  /* Now, wait for all these threads, so that they don't become zombies
     and their times are properly added to the thread manager's times. */
  for (th = issuing_thread->p_nextlive;
       th != issuing_thread;
       th = th->p_nextlive) {
    tgr_waitpid(th->p_pid, NULL, 1);
  }
  __fresetlockfiles();
  __MANAGER_CRIT_END();
  _exit(0);
}
