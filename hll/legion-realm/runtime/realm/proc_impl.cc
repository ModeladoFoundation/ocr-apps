/* Copyright 2016 Stanford University, NVIDIA Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "proc_impl.h"

#include "timers.h"
#include "runtime_impl.h"
#include "logging.h"
#include "serialize.h"
#include "profiling.h"
#include "utils.h"

#include <sys/types.h>
#include <dirent.h>

GASNETT_THREADKEY_DEFINE(cur_preemptable_thread);

#define CHECK_PTHREAD(cmd) do { \
  int ret = (cmd); \
  if(ret != 0) { \
    fprintf(stderr, "PTHREAD: %s = %d (%s)\n", #cmd, ret, strerror(ret)); \
    exit(1); \
  } \
} while(0)

namespace Realm {

  extern Logger log_task;  // defined in tasks.cc
  extern Logger log_util;  // defined in tasks.cc
  Logger log_taskreg("taskreg");

  ////////////////////////////////////////////////////////////////////////
  //
  // class Processor
  //

    /*static*/ const Processor Processor::NO_PROC = { 0 }; 

  namespace ThreadLocal {
    __thread Processor current_processor;
  };

    Processor::Kind Processor::kind(void) const
    {
      return get_runtime()->get_processor_impl(*this)->kind;
    }

    /*static*/ Processor Processor::create_group(const std::vector<Processor>& members)
    {
      // are we creating a local group?
      if((members.size() == 0) || (ID(members[0]).node() == gasnet_mynode())) {
	ProcessorGroup *grp = get_runtime()->local_proc_group_free_list->alloc_entry();
	grp->set_group_members(members);
#ifdef EVENT_GRAPH_TRACE
        {
          const int base_size = 1024;
          char base_buffer[base_size];
          char *buffer;
          int buffer_size = (members.size() * 20);
          if (buffer_size >= base_size)
            buffer = (char*)malloc(buffer_size+1);
          else
            buffer = base_buffer;
          buffer[0] = '\0';
          int offset = 0;
          for (std::vector<Processor>::const_iterator it = members.begin();
                it != members.end(); it++)
          {
            int written = snprintf(buffer+offset,buffer_size-offset,
                                   " " IDFMT, it->id);
            assert(written < (buffer_size-offset));
            offset += written;
          }
          log_event_graph.info("Group: " IDFMT " %ld%s",
                                grp->me.id, members.size(), buffer); 
          if (buffer_size >= base_size)
            free(buffer);
        }
#endif
	return grp->me;
      }

      assert(0);
    }

    void Processor::get_group_members(std::vector<Processor>& members)
    {
      // if we're a plain old processor, the only member of our "group" is ourself
      if(ID(*this).type() == ID::ID_PROCESSOR) {
	members.push_back(*this);
	return;
      }

      assert(ID(*this).type() == ID::ID_PROCGROUP);

      ProcessorGroup *grp = get_runtime()->get_procgroup_impl(*this);
      grp->get_group_members(members);
    }

    Event Processor::spawn(TaskFuncID func_id, const void *args, size_t arglen,
			   //std::set<RegionInstance> instances_needed,
			   Event wait_on, int priority) const
    {
      DetailedTimer::ScopedPush sp(TIME_LOW_LEVEL);
#ifdef USE_OCR_LAYER
      ProcessorImpl *p = get_runtime()->get_processor_impl(*this);
      Event e = ((OCRProcessor*)p)->spawn_task_ret_event(func_id, args, arglen,
                   ProfilingRequestSet(), wait_on, priority);
      return e;
#else
      ProcessorImpl *p = get_runtime()->get_processor_impl(*this);

      GenEventImpl *finish_event = GenEventImpl::create_genevent();
      Event e = finish_event->current_event();
#ifdef EVENT_GRAPH_TRACE
      Event enclosing = find_enclosing_termination_event();
      log_event_graph.info("Task Request: %d " IDFMT 
                            " (" IDFMT ",%d) (" IDFMT ",%d)"
                            " (" IDFMT ",%d) %d %p %ld",
                            func_id, id, e.id, e.gen,
                            wait_on.id, wait_on.gen,
                            enclosing.id, enclosing.gen,
                            priority, args, arglen);
#endif

      p->spawn_task(func_id, args, arglen, ProfilingRequestSet(),
		    wait_on, e, priority);
      return e;
#endif
    }

    Event Processor::spawn(TaskFuncID func_id, const void *args, size_t arglen,
                           const ProfilingRequestSet &reqs,
			   Event wait_on, int priority) const
    {
      DetailedTimer::ScopedPush sp(TIME_LOW_LEVEL);
#ifdef USE_OCR_LAYER
      ProcessorImpl *p = get_runtime()->get_processor_impl(*this);
      //GenEventImpl *finish_event = GenEventImpl::create_genevent();
      //Event e = finish_event->current_event();
      Event e = Event::NO_EVENT;
      p->spawn_task(func_id, args, arglen, reqs,
                   wait_on, e, priority);
      return e;
#else
      ProcessorImpl *p = get_runtime()->get_processor_impl(*this);

      GenEventImpl *finish_event = GenEventImpl::create_genevent();
      Event e = finish_event->current_event();
#ifdef EVENT_GRAPH_TRACE
      Event enclosing = find_enclosing_termination_event();
      log_event_graph.info("Task Request: %d " IDFMT 
                            " (" IDFMT ",%d) (" IDFMT ",%d)"
                            " (" IDFMT ",%d) %d %p %ld",
                            func_id, id, e.id, e.gen,
                            wait_on.id, wait_on.gen,
                            enclosing.id, enclosing.gen,
                            priority, args, arglen);
#endif

      p->spawn_task(func_id, args, arglen, reqs,
		    wait_on, e, priority);
      return e;
#endif
    }

    AddressSpace Processor::address_space(void) const
    {
      return ID(id).node();
    }

    ID::IDType Processor::local_id(void) const
    {
      return ID(id).index();
    }

    Event Processor::register_task(TaskFuncID func_id,
				   const CodeDescriptor& codedesc,
				   const ProfilingRequestSet& prs,
				   const void *user_data /*= 0*/,
				   size_t user_data_len /*= 0*/) const
    {
      // some sanity checks first
      if(codedesc.type() != TypeConv::from_cpp_type<TaskFuncPtr>()) {
	log_taskreg.fatal() << "attempt to register a task function of improper type: " << codedesc.type();
	assert(0);
      }

#ifdef USE_OCR_LAYER //ignores prs parameter
      ProcessorImpl *p = get_runtime()->get_processor_impl(*this);
      p->register_task(func_id, const_cast<CodeDescriptor&>(codedesc), ByteArrayRef(user_data, user_data_len)); 
      return Event::NO_EVENT;
#else
      // TODO: special case - registration on a local processor with a raw function pointer and no
      //  profiling requests - can be done immediately and return NO_EVENT

      Event finish_event = GenEventImpl::create_genevent()->current_event();

      TaskRegistration *tro = new TaskRegistration(codedesc, 
						   ByteArrayRef(user_data, user_data_len),
						   finish_event, prs);
      tro->mark_ready();
      tro->mark_started();

      std::vector<Processor> local_procs;
      std::map<gasnet_node_t, std::vector<Processor> > remote_procs;
      // is the target a single processor or a group?
      if(ID(*this).type() == ID::ID_PROCESSOR) {
	gasnet_node_t n = ID(*this).node();
	if(n == gasnet_mynode())
	  local_procs.push_back(*this);
	else
	  remote_procs[n].push_back(*this);
      } else {
	// assume we're a group
	ProcessorGroup *grp = get_runtime()->get_procgroup_impl(*this);
	std::vector<Processor> members;
	grp->get_group_members(members);
	for(std::vector<Processor>::const_iterator it = members.begin();
	    it != members.end();
	    it++) {
	  Processor p = *it;
	  gasnet_node_t n = ID(p).node();
	  if(n == gasnet_mynode())
	    local_procs.push_back(p);
	  else
	    remote_procs[n].push_back(p);
	}
      }

      // remote processors need a portable implementation available
      if(!remote_procs.empty()) {
	if(!tro->codedesc.has_portable_implementations()) {
	  log_taskreg.fatal() << "cannot remotely register a task with no portable implementations";
	  assert(0);
	}
      }
	 
      // local processor(s) can be called directly
      if(!local_procs.empty()) {
	for(std::vector<Processor>::const_iterator it = local_procs.begin();
	    it != local_procs.end();
	    it++) {
	  ProcessorImpl *p = get_runtime()->get_processor_impl(*it);
	  p->register_task(func_id, tro->codedesc, tro->userdata);
	}
      }

      for(std::map<gasnet_node_t, std::vector<Processor> >::const_iterator it = remote_procs.begin();
	  it != remote_procs.end();
	  it++) {
	gasnet_node_t target = it->first;
	RemoteTaskRegistration *reg_op = new RemoteTaskRegistration(tro, target);
	tro->add_async_work_item(reg_op);
	RegisterTaskMessage::send_request(target, func_id, NO_KIND, it->second,
					  tro->codedesc,
					  tro->userdata.base(), tro->userdata.size(),
					  reg_op);
      }

      tro->mark_finished();
      return finish_event;
#endif
    }

    /*static*/ Event Processor::register_task_by_kind(Kind target_kind, bool global,
						      TaskFuncID func_id,
						      const CodeDescriptor& codedesc,
						      const ProfilingRequestSet& prs,
						      const void *user_data /*= 0*/,
						      size_t user_data_len /*= 0*/)
    {
      // some sanity checks first
      if(codedesc.type() != TypeConv::from_cpp_type<TaskFuncPtr>()) {
	log_taskreg.fatal() << "attempt to register a task function of improper type: " << codedesc.type();
	assert(0);
      }

#ifdef USE_OCR_LAYER  //ignores the target_kind, global and prs parameters
      //ProcessorImpl *p = get_runtime()->get_processor_impl(*this);
      //p->register_task(func_id, codedesc, ByteArrayRef(user_data, user_data_len)); 
      std::set<Processor> local_procs;
      get_runtime()->machine->get_local_processors_by_kind(local_procs, target_kind);
      for(std::set<Processor>::const_iterator it = local_procs.begin();
          it != local_procs.end();
          it++) {
        ProcessorImpl *p = get_runtime()->get_processor_impl(*it);
        p->register_task(func_id, const_cast<CodeDescriptor&>(codedesc), ByteArrayRef(user_data, user_data_len));
      }
      return Event::NO_EVENT;
#else
      // TODO: special case - registration on local processord with a raw function pointer and no
      //  profiling requests - can be done immediately and return NO_EVENT

      Event finish_event = GenEventImpl::create_genevent()->current_event();

      TaskRegistration *tro = new TaskRegistration(codedesc, 
						   ByteArrayRef(user_data, user_data_len),
						   finish_event, prs);
      tro->mark_ready();
      tro->mark_started();

      // do local processors first
      std::set<Processor> local_procs;
      get_runtime()->machine->get_local_processors_by_kind(local_procs, target_kind);
      if(!local_procs.empty()) {
	for(std::set<Processor>::const_iterator it = local_procs.begin();
	    it != local_procs.end();
	    it++) {
	  ProcessorImpl *p = get_runtime()->get_processor_impl(*it);
	  p->register_task(func_id, tro->codedesc, tro->userdata);
	}
      }

      if(global) {
	// remote processors need a portable implementation available
	if(!tro->codedesc.has_portable_implementations()) {
	  log_taskreg.fatal() << "cannot remotely register a task with no portable implementations";
	  assert(0);
	}

	for(gasnet_node_t target = 0; target < gasnet_nodes(); target++) {
	  // skip ourselves
	  if(target == gasnet_mynode())
	    continue;

	  RemoteTaskRegistration *reg_op = new RemoteTaskRegistration(tro, target);
	  tro->add_async_work_item(reg_op);
	  RegisterTaskMessage::send_request(target, func_id, target_kind, std::vector<Processor>(),
					    tro->codedesc,
					    tro->userdata.base(), tro->userdata.size(),
					    reg_op);
	}
      }

      tro->mark_finished();
      return finish_event;
#endif
    }


  ////////////////////////////////////////////////////////////////////////
  //
  // class ProcessorImpl
  //

    ProcessorImpl::ProcessorImpl(Processor _me, Processor::Kind _kind)
      : me(_me), kind(_kind)
    {
    }

    ProcessorImpl::~ProcessorImpl(void)
    {
    }

    void ProcessorImpl::shutdown(void)
    {
    }

    void ProcessorImpl::execute_task(Processor::TaskFuncID func_id,
				     const ByteArrayRef& task_args)
    {
      // should never be called
      assert(0);
    }

    void ProcessorImpl::register_task(Processor::TaskFuncID func_id,
				      CodeDescriptor& codedesc,
				      const ByteArrayRef& user_data)
    {
      // should never be called
      assert(0);
    }


  ////////////////////////////////////////////////////////////////////////
  //
  // class ProcessorGroup
  //

    ProcessorGroup::ProcessorGroup(void)
      : ProcessorImpl(Processor::NO_PROC, Processor::PROC_GROUP),
	members_valid(false), members_requested(false), next_free(0)
    {
    }

    ProcessorGroup::~ProcessorGroup(void)
    {
    }

    void ProcessorGroup::init(Processor _me, int _owner)
    {
      assert(ID(_me).node() == (unsigned)_owner);

      me = _me;
      lock.init(ID(me).convert<Reservation>(), ID(me).node());
    }

    void ProcessorGroup::set_group_members(const std::vector<Processor>& member_list)
    {
      // can only be perform on owner node
      assert(ID(me).node() == gasnet_mynode());
      
      // can only be done once
      assert(!members_valid);

      for(std::vector<Processor>::const_iterator it = member_list.begin();
	  it != member_list.end();
	  it++) {
	ProcessorImpl *m_impl = get_runtime()->get_processor_impl(*it);
	members.push_back(m_impl);
	m_impl->add_to_group(this);
      }

      members_requested = true;
      members_valid = true;
    }

    void ProcessorGroup::get_group_members(std::vector<Processor>& member_list)
    {
      assert(members_valid);

      for(std::vector<ProcessorImpl *>::const_iterator it = members.begin();
	  it != members.end();
	  it++)
	member_list.push_back((*it)->me);
    }

    void ProcessorGroup::enqueue_task(Task *task)
    {
      // put it into the task queue - one of the member procs will eventually grab it
      task->mark_ready();
      task_queue.put(task, task->priority);
    }

    void ProcessorGroup::add_to_group(ProcessorGroup *group)
    {
      // recursively add all of our members
      assert(members_valid);

      for(std::vector<ProcessorImpl *>::const_iterator it = members.begin();
	  it != members.end();
	  it++)
	(*it)->add_to_group(group);
    }

    /*virtual*/ void ProcessorGroup::spawn_task(Processor::TaskFuncID func_id,
						const void *args, size_t arglen,
                                                const ProfilingRequestSet &reqs,
						Event start_event, Event finish_event,
						int priority)
    {
      // create a task object and insert it into the queue
      Task *task = new Task(me, func_id, args, arglen, reqs,
                            start_event, finish_event, priority);

      if (start_event.has_triggered())
        enqueue_task(task);
      else
	EventImpl::add_waiter(start_event, new DeferredTaskSpawn(this, task));
    }


  ////////////////////////////////////////////////////////////////////////
  //
  // class DeferredTaskSpawn
  //

    bool DeferredTaskSpawn::event_triggered(void)
    {
      proc->enqueue_task(task);
      return true;
    }

    void DeferredTaskSpawn::print_info(FILE *f)
    {
      fprintf(f,"deferred task: func=%d proc=" IDFMT " finish=" IDFMT "/%d\n",
             task->func_id, task->proc.id, task->get_finish_event().id, task->get_finish_event().gen);
    }


  ////////////////////////////////////////////////////////////////////////
  //
  // class SpawnTaskMessage
  //

  /*static*/ void SpawnTaskMessage::handle_request(RequestArgs args,
						   const void *data,
						   size_t datalen)
  {
    DetailedTimer::ScopedPush sp(TIME_LOW_LEVEL);
    ProcessorImpl *p = get_runtime()->get_processor_impl(args.proc);

    Event start_event, finish_event;
    start_event.id = args.start_id;
    start_event.gen = args.start_gen;
    finish_event.id = args.finish_id;
    finish_event.gen = args.finish_gen;

    log_task.debug() << "received remote spawn request:"
		     << " func=" << args.func_id
		     << " proc=" << args.proc
		     << " finish=" << finish_event;

    Serialization::FixedBufferDeserializer fbd(data, datalen);
    fbd.extract_bytes(0, args.user_arglen);  // skip over task args - we'll access those directly

    // profiling requests are optional - extract only if there's data
    ProfilingRequestSet prs;
    if(fbd.bytes_left() > 0)
      fbd >> prs;
      
    p->spawn_task(args.func_id, data, args.user_arglen, prs,
		  start_event, finish_event, args.priority);
  }

  /*static*/ void SpawnTaskMessage::send_request(gasnet_node_t target, Processor proc,
						 Processor::TaskFuncID func_id,
						 const void *args, size_t arglen,
						 const ProfilingRequestSet *prs,
						 Event start_event, Event finish_event,
						 int priority)
  {
    RequestArgs r_args;

    r_args.proc = proc;
    r_args.func_id = func_id;
    r_args.start_id = start_event.id;
    r_args.start_gen = start_event.gen;
    r_args.finish_id = finish_event.id;
    r_args.finish_gen = finish_event.gen;
    r_args.priority = priority;
    r_args.user_arglen = arglen;
    
    if(!prs) {
      // no profiling, so task args are the only payload
      Message::request(target, r_args, args, arglen, PAYLOAD_COPY);
    } else {
      // need to serialize both the task args and the profiling request
      //  into a single payload
      Serialization::DynamicBufferSerializer dbs(arglen + 4096);  // assume profiling requests are < 4K

      dbs.append_bytes(args, arglen);
      dbs << *prs;

      size_t datalen = dbs.bytes_used();
      void *data = dbs.detach_buffer(-1);  // don't trim - this buffer has a short life
      Message::request(target, r_args, data, datalen, PAYLOAD_FREE);
    }
  }


  ////////////////////////////////////////////////////////////////////////
  //
  // class RegisterTaskMessage
  //

  /*static*/ void RegisterTaskMessage::handle_request(RequestArgs args, const void *data, size_t datalen)
  {
    std::vector<Processor> procs;
    CodeDescriptor codedesc;
    ByteArray userdata;

    Serialization::FixedBufferDeserializer fbd(data, datalen);
    bool ok = ((fbd >> procs) &&
	       (fbd >> codedesc) &&
	       (fbd >> userdata));
    assert(ok && (fbd.bytes_left() == 0));

    if(procs.empty()) {
      // use the supplied kind and find all procs of that kind
      std::set<Processor> local_procs;
      get_runtime()->machine->get_local_processors_by_kind(local_procs, args.kind);
    
      for(std::set<Processor>::const_iterator it = local_procs.begin();
	  it != local_procs.end();
	  it++) {
	ProcessorImpl *p = get_runtime()->get_processor_impl(*it);
	p->register_task(args.func_id, codedesc, userdata);
      }
    } else {
      for(std::vector<Processor>::const_iterator it = procs.begin();
	  it != procs.end();
	  it++) {
	ProcessorImpl *p = get_runtime()->get_processor_impl(*it);
	p->register_task(args.func_id, codedesc, userdata);
      }
    }

    // TODO: include status/profiling eventually
    RegisterTaskCompleteMessage::send_request(args.sender, args.reg_op);
  }

  /*static*/ void RegisterTaskMessage::send_request(gasnet_node_t target,
						    Processor::TaskFuncID func_id,
						    Processor::Kind kind,
						    const std::vector<Processor>& procs,
						    const CodeDescriptor& codedesc,
						    const void *userdata, size_t userlen,
						    RemoteTaskRegistration *reg_op)
  {
    RequestArgs args;

    args.sender = gasnet_mynode();
    args.func_id = func_id;
    args.kind = kind;
    args.reg_op = reg_op;

    Serialization::DynamicBufferSerializer dbs(1024);
    dbs << procs;
    dbs << codedesc;
    dbs << ByteArrayRef(userdata, userlen);

    size_t datalen = dbs.bytes_used();
    void *data = dbs.detach_buffer(-1 /*no trim*/);
    Message::request(target, args, data, datalen, PAYLOAD_FREE);
  }


  ////////////////////////////////////////////////////////////////////////
  //
  // class RegisterTaskCompleteMessage
  //

  /*static*/ void RegisterTaskCompleteMessage::handle_request(RequestArgs args)
  {
    args.reg_op->mark_finished();
  }

  /*static*/ void RegisterTaskCompleteMessage::send_request(gasnet_node_t target,
							    RemoteTaskRegistration *reg_op)
  {
    RequestArgs args;

    args.sender = gasnet_mynode();
    args.reg_op = reg_op;

    Message::request(target, args);
  }


  ////////////////////////////////////////////////////////////////////////
  //
  // class RemoteProcessor
  //

    RemoteProcessor::RemoteProcessor(Processor _me, Processor::Kind _kind)
      : ProcessorImpl(_me, _kind)
    {
    }

    RemoteProcessor::~RemoteProcessor(void)
    {
    }

    void RemoteProcessor::enqueue_task(Task *task)
    {
      // should never be called
      assert(0);
    }

    void RemoteProcessor::add_to_group(ProcessorGroup *group)
    {
      // not currently supported
      assert(0);
    }

    void RemoteProcessor::spawn_task(Processor::TaskFuncID func_id,
				     const void *args, size_t arglen,
				     const ProfilingRequestSet &reqs,
				     Event start_event, Event finish_event,
				     int priority)
    {
      log_task.debug() << "sending remote spawn request:"
		       << " func=" << func_id
		       << " proc=" << me
		       << " finish=" << finish_event;

      SpawnTaskMessage::send_request(ID(me).node(), me, func_id,
				     args, arglen, &reqs,
				     start_event, finish_event, priority);
    }

  
  ////////////////////////////////////////////////////////////////////////
  //
  // class LocalTaskProcessor
  //

  LocalTaskProcessor::LocalTaskProcessor(Processor _me, Processor::Kind _kind)
    : ProcessorImpl(_me, _kind), sched(0)
  {
    // nothing really happens until we get a scheduler
  }

  LocalTaskProcessor::~LocalTaskProcessor(void)
  {
    delete sched;
  }

  void LocalTaskProcessor::set_scheduler(ThreadedTaskScheduler *_sched)
  {
    sched = _sched;

    // add our task queue to the scheduler
    sched->add_task_queue(&task_queue);

    // this should be requested from outside now
#if 0
    // if we have an init task, queue that up (with highest priority)
    Processor::TaskIDTable::iterator it = 
      get_runtime()->task_table.find(Processor::TASK_ID_PROCESSOR_INIT);
    if(it != get_runtime()->task_table.end()) {
      Task *t = new Task(me, Processor::TASK_ID_PROCESSOR_INIT,
			 0, 0,
			 Event::NOEVENT, Event::NO_EVENT, 0);
      task_queue.put(t, task_queue.PRI_MAX_FINITE);
    } else {
      log_proc.info("no processor init task: proc=" IDFMT "", me.id);
    }
#endif

    // finally, fire up the scheduler
    sched->start();
  }

  void LocalTaskProcessor::add_to_group(ProcessorGroup *group)
  {
    // add the group's task queue to our scheduler too
    sched->add_task_queue(&group->task_queue);
  }

  void LocalTaskProcessor::enqueue_task(Task *task)
  {
    // just jam it into the task queue
    task->mark_ready();
    task_queue.put(task, task->priority);
  }

  void LocalTaskProcessor::spawn_task(Processor::TaskFuncID func_id,
				     const void *args, size_t arglen,
				     const ProfilingRequestSet &reqs,
				     Event start_event, Event finish_event,
				     int priority)
  {
    assert(func_id != 0);
    // create a task object for this
    Task *task = new Task(me, func_id, args, arglen, reqs,
			  start_event, finish_event, priority);

    // if the start event has already triggered, we can enqueue right away
    if(start_event.has_triggered()) {
      enqueue_task(task);
    } else {
      EventImpl::add_waiter(start_event, new DeferredTaskSpawn(this, task));
    }
  }

  void LocalTaskProcessor::register_task(Processor::TaskFuncID func_id,
					 CodeDescriptor& codedesc,
					 const ByteArrayRef& user_data)
  {
    // first, make sure we haven't seen this task id before
    if(task_table.count(func_id) > 0) {
      log_taskreg.fatal() << "duplicate task registration: proc=" << me << " func=" << func_id;
      assert(0);
    }

    // next, get see if we have a function pointer to register
    Processor::TaskFuncPtr fnptr;
    const FunctionPointerImplementation *fpi = codedesc.find_impl<FunctionPointerImplementation>();

    while(!fpi) {
#ifdef REALM_USE_DLFCN
      // can we make it from a DSO reference?
      const DSOReferenceImplementation *dsoref = codedesc.find_impl<DSOReferenceImplementation>();
      if(dsoref) {
        FunctionPointerImplementation *newfpi = cvt_dsoref_to_fnptr(dsoref);
	if(newfpi) {
	  codedesc.add_implementation(newfpi);
	  fpi = newfpi;
	  continue;
        }
      }
#endif

      // no other options?  give up
      assert(0);
    }

    fnptr = (Processor::TaskFuncPtr)(fpi->fnptr);

    log_taskreg.info() << "task " << func_id << " registered on " << me << ": " << codedesc;

    TaskTableEntry &tte = task_table[func_id];
    tte.fnptr = fnptr;
    tte.user_data = user_data;
  }

  void LocalTaskProcessor::execute_task(Processor::TaskFuncID func_id,
					const ByteArrayRef& task_args)
  {
    std::map<Processor::TaskFuncID, TaskTableEntry>::const_iterator it = task_table.find(func_id);
    if(it == task_table.end()) {
      // TODO: remove this hack once the tools are available to the HLR to call these directly
      if(func_id < Processor::TASK_ID_FIRST_AVAILABLE) {
	log_taskreg.warning() << "task " << func_id << " not registered on " << me << ": ignoring missing legacy setup/shutdown task";
	return;
      }
      log_taskreg.fatal() << "task " << func_id << " not registered on " << me;
      assert(0);
    }

    const TaskTableEntry& tte = it->second;

    log_taskreg.debug() << "task " << func_id << " executing on " << me << ": " << tte.fnptr;

    (tte.fnptr)(task_args.base(), task_args.size(),
		tte.user_data.base(), tte.user_data.size(),
		me);
  }

  // blocks until things are cleaned up
  void LocalTaskProcessor::shutdown(void)
  {
    // this should be requested from outside now
#if 0
    // enqueue a shutdown task, if it exists
    Processor::TaskIDTable::iterator it = 
      get_runtime()->task_table.find(Processor::TASK_ID_PROCESSOR_SHUTDOWN);
    if(it != get_runtime()->task_table.end()) {
      Task *t = new Task(me, Processor::TASK_ID_PROCESSOR_SHUTDOWN,
			 0, 0,
			 Event::NO_EVENT, Event::NO_EVENT, 0);
      task_queue.put(t, task_queue.PRI_MIN_FINITE);
    } else {
      log_proc.info("no processor shutdown task: proc=" IDFMT "", me.id);
    }
#endif

    sched->shutdown();
  }
  

  ////////////////////////////////////////////////////////////////////////
  //
  // class LocalCPUProcessor
  //

  LocalCPUProcessor::LocalCPUProcessor(Processor _me, CoreReservationSet& crs,
				       size_t _stack_size)
    : LocalTaskProcessor(_me, Processor::LOC_PROC)
  {
    CoreReservationParameters params;
    params.set_num_cores(1);
    params.set_alu_usage(params.CORE_USAGE_EXCLUSIVE);
    params.set_fpu_usage(params.CORE_USAGE_EXCLUSIVE);
    params.set_ldst_usage(params.CORE_USAGE_SHARED);
    params.set_max_stack_size(_stack_size);

    std::string name = stringbuilder() << "CPU proc " << _me;

    core_rsrv = new CoreReservation(name, crs, params);

#ifdef REALM_USE_USER_THREADS
    UserThreadTaskScheduler *sched = new UserThreadTaskScheduler(me, *core_rsrv);
    // no config settings we want to tweak yet
#else
    KernelThreadTaskScheduler *sched = new KernelThreadTaskScheduler(me, *core_rsrv);
    sched->cfg_max_idle_workers = 3; // keep a few idle threads around
#endif
    set_scheduler(sched);
  }

  LocalCPUProcessor::~LocalCPUProcessor(void)
  {
    delete core_rsrv;
  }


  ////////////////////////////////////////////////////////////////////////
  //
  // class LocalUtilityProcessor
  //

  LocalUtilityProcessor::LocalUtilityProcessor(Processor _me, CoreReservationSet& crs,
					       size_t _stack_size)
    : LocalTaskProcessor(_me, Processor::UTIL_PROC)
  {
    CoreReservationParameters params;
    params.set_num_cores(1);
    params.set_alu_usage(params.CORE_USAGE_SHARED);
    params.set_fpu_usage(params.CORE_USAGE_MINIMAL);
    params.set_ldst_usage(params.CORE_USAGE_SHARED);
    params.set_max_stack_size(_stack_size);

    std::string name = stringbuilder() << "utility proc " << _me;

    core_rsrv = new CoreReservation(name, crs, params);

#ifdef REALM_USE_USER_THREADS
    UserThreadTaskScheduler *sched = new UserThreadTaskScheduler(me, *core_rsrv);
    // no config settings we want to tweak yet
#else
    KernelThreadTaskScheduler *sched = new KernelThreadTaskScheduler(me, *core_rsrv);
    // no config settings we want to tweak yet
#endif
    set_scheduler(sched);
  }

  LocalUtilityProcessor::~LocalUtilityProcessor(void)
  {
    delete core_rsrv;
  }


  ////////////////////////////////////////////////////////////////////////
  //
  // class LocalIOProcessor
  //

  LocalIOProcessor::LocalIOProcessor(Processor _me, CoreReservationSet& crs,
				     size_t _stack_size, int _concurrent_io_threads)
    : LocalTaskProcessor(_me, Processor::IO_PROC)
  {
    CoreReservationParameters params;
    params.set_alu_usage(params.CORE_USAGE_SHARED);
    params.set_fpu_usage(params.CORE_USAGE_MINIMAL);
    params.set_ldst_usage(params.CORE_USAGE_SHARED);
    params.set_max_stack_size(_stack_size);

    std::string name = stringbuilder() << "IO proc " << _me;

    core_rsrv = new CoreReservation(name, crs, params);

    // IO processors always use kernel threads
    ThreadedTaskScheduler *sched = new KernelThreadTaskScheduler(me, *core_rsrv);

    // allow concurrent IO threads
    sched->cfg_max_active_workers = _concurrent_io_threads;

    set_scheduler(sched);
  }

  LocalIOProcessor::~LocalIOProcessor(void)
  {
    delete core_rsrv;
  }

#ifdef USE_OCR_LAYER
  ////////////////////////////////////////////////////////////////////////
  //
  // class OCRProcessor
  //

  ocrGuid_t OCRProcessor::ocr_realm_conversion_edt_t = NULL_GUID;
  const int num_dep = 3;

  OCRProcessor::OCRProcessor(Processor _me)
    : ProcessorImpl(_me, Processor::OCR_PROC)
  {
  }

  OCRProcessor::~OCRProcessor(void)
  {
  }

  void OCRProcessor::enqueue_task(Task *task)
  {
    assert(0);
  }

  //convert from OCR function call to realm function call provided by user
  //argv[0] is the function pointer of the realm function call
  //depv[0] is the event dependency that the edt wait on
  //depv[1] is the args parameter of realm function call
  //depv[2] is the arglen paramter of realm function call
  //ideally these parameters should come inside argv and not data blocks
  ocrGuid_t ocr_realm_conversion_func(u32 argc, u64 *argv, u32 depc, ocrEdtDep_t depv[])
  {
    assert(argc == 1 && depc == 3);
    //extract the function pointer to be executed
    Processor::TaskFuncPtr task_func = ((OCRProcessor::TaskTableEntry*)argv)->fnptr;
    //extract args and arglen and pass it to the function
    void *args = depv[1].ptr;
    size_t *arglen = (size_t*)depv[2].ptr;
    task_func(args, *arglen, NULL, 0, Processor::NO_PROC);

    ocrDbDestroy(depv[1].guid);
    ocrDbDestroy(depv[2].guid);
    return NULL_GUID;
  }

  /*static*/ void OCRProcessor::static_init(void)
  {
    //create the function conversion edt template
    ocrEdtTemplateCreate(&OCRProcessor::ocr_realm_conversion_edt_t, ocr_realm_conversion_func, 1, 3);
  }

  /*static*/ void OCRProcessor::static_destroy(void)
  {
    //delete the function conversion edt template
    ocrEdtTemplateDestroy(OCRProcessor::ocr_realm_conversion_edt_t);
  }

  //creates a OCR edt that calls the realm user function
  //function pointer of the realm function call is put in argv[0] parameter
  //start_event is put in first dependency data block
  //args parameter of realm function call is put in second dependency data block
  //arglen paramter of realm function call is put in third dependency data block
  Event OCRProcessor::spawn_task_ret_event(Processor::TaskFuncID func_id,
                                const void *args, size_t arglen,
                               const ProfilingRequestSet &reqs,
                               Event start_event, int priority)
  {
    //ignores reqs, priority
   
    //create three dependencies : start_event, args and arglen
    ocrGuid_t db_guid[num_dep];
  
    db_guid[0] = UNINITIALIZED_GUID; //start_event.evt_guid;
  
    void *args_copy;
    ocrDbCreate(&db_guid[1], (void **)(&args_copy), arglen, DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    memcpy(args_copy, args, arglen);
  
    size_t *arglen_copy;
    ocrDbCreate(&db_guid[2], (void **)(&arglen_copy), sizeof(size_t), DB_PROP_NONE, NULL_GUID, NO_ALLOC);
    arglen_copy[0] = arglen;
  
    //create and call the Edt (move out the template to a static variable)
    ocrGuid_t ocr_realm_conversion_edt, out_ocr_realm_conversion_edt, persistent_evt_guid;
    ocrEdtCreate(&ocr_realm_conversion_edt, OCRProcessor::ocr_realm_conversion_edt_t, EDT_PARAM_DEF, 
      (u64*)(&task_table[func_id]), EDT_PARAM_DEF, db_guid, 
      EDT_PROP_NONE, NULL_GUID, &out_ocr_realm_conversion_edt);
     
    //create another persistent event and attach it to the edt since legacy_block_progress needs persistent event
    ocrEventCreate(&persistent_evt_guid, OCR_EVENT_STICKY_T, EVT_PROP_NONE);
    ocrAddDependence(out_ocr_realm_conversion_edt, persistent_evt_guid, 0, DB_MODE_RO);

    //satsify the dependency after setting the sticky event to prevent the edt from starting before adding dependence to the sticky event
    ocrAddDependence(start_event.evt_guid, ocr_realm_conversion_edt, 0, DB_MODE_RO);

    Event finish_event = ID(OCREventImpl::ID_TYPE, gasnet_mynode(), 0).convert<Event>();
    finish_event.evt_guid = persistent_evt_guid;
    return finish_event;
  }

  //creates a OCR edt that calls the realm function
  //not used since the finish event was requried in return
  //because no table is maintained to map event and its implementation
  void OCRProcessor::spawn_task(Processor::TaskFuncID func_id,
                                const void *args, size_t arglen,
                               const ProfilingRequestSet &reqs,
                               Event start_event, Event finish_event,
                               int priority)
  {
    assert(0);
  }

  void OCRProcessor::shutdown()
  {
    assert(0);
  }


  void OCRProcessor::add_to_group(ProcessorGroup *group)
  {
    assert(0);
  }

  //register_task creates a mapping from func_id to the function pointer using the tak_table map
  void OCRProcessor::register_task(Processor::TaskFuncID func_id,
                                         CodeDescriptor& codedesc,
                                         const ByteArrayRef& user_data)
  {
    // first, make sure we haven't seen this task id before
    if(task_table.count(func_id) > 0) {
      log_taskreg.fatal() << "duplicate task registration: proc=" << me << " func=" << func_id;
      assert(0);
    }

    // next, get see if we have a function pointer to register
    Processor::TaskFuncPtr fnptr;
    const FunctionPointerImplementation *fpi = codedesc.find_impl<FunctionPointerImplementation>();

    while(!fpi) {
#ifdef REALM_USE_DLFCN
      // can we make it from a DSO reference?
      const DSOReferenceImplementation *dsoref = codedesc.find_impl<DSOReferenceImplementation>();
      if(dsoref) {
        FunctionPointerImplementation *newfpi = cvt_dsoref_to_fnptr(dsoref);
	if(newfpi) {
	  codedesc.add_implementation(newfpi);
	  fpi = newfpi;
	  continue;
        }
      }
#endif

      // no other options?  give up
      assert(0);
    }

    fnptr = (Processor::TaskFuncPtr)(fpi->fnptr);

    log_taskreg.info() << "task " << func_id << " registered on " << me << ": " << codedesc;

    TaskTableEntry &tte = task_table[func_id];
    tte.fnptr = fnptr;
    tte.user_data = user_data;
  }

  void OCRProcessor::execute_task(Processor::TaskFuncID func_id,
                                const ByteArrayRef& task_args)
  {
    assert(0);
  }
  
#endif

  ////////////////////////////////////////////////////////////////////////
  //
  // class TaskRegistration
  //

  TaskRegistration::TaskRegistration(const CodeDescriptor& _codedesc,
				     const ByteArrayRef& _userdata,
				     Event _finish_event, const ProfilingRequestSet &_requests)
    : Operation(_finish_event, _requests)
    , codedesc(_codedesc), userdata(_userdata)
  {}


  ////////////////////////////////////////////////////////////////////////
  //
  // class RemoteTaskRegistration
  //

  RemoteTaskRegistration::RemoteTaskRegistration(TaskRegistration *reg_op, int _target_node)
    : Operation::AsyncWorkItem(reg_op)
    , target_node(_target_node)
  {}

  void RemoteTaskRegistration::request_cancellation(void)
  {
    // ignored
  }


}; // namespace Realm
