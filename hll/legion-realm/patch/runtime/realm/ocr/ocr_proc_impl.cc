/* Copyright 2016 Rice University, Intel Corporation
 *
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

#if USE_OCR_LAYER

#include "ocr_proc_impl.h"

namespace Realm {

  extern Logger log_taskreg; // defined in proc_impl.cc

  ////////////////////////////////////////////////////////////////////////
  //
  // class OCRProcessor
  //

  ocrGuid_t OCRProcessor::ocr_realm_conversion_edt_t = NULL_GUID;

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
  //argv is the ArgsEDT struct
  //depv[0] is the event dependency that the edt wait on
  //depv[1] is the args parameter of realm function call
  //ideally these parameters should come inside argv and not data blocks
  ocrGuid_t ocr_realm_conversion_func(u32 argc, u64 *argv, u32 depc, ocrEdtDep_t depv[])
  {
    assert(argc > 0 && depc == 2);

    OCRProcessor::ArgsEDT *argv_ptr = (OCRProcessor::ArgsEDT *)argv;
    //make sure the current processor is set during execution of the task
    //This might have to move to ELS
    ThreadLocal::current_processor = argv_ptr->p;
    //extract the function pointer to be executed
    Processor::TaskFuncPtr task_func = (argv_ptr->task_entry).fnptr;
    //extract args and arglen and pass it to the function
    void *args = depv[1].ptr;
    task_func(args, argv_ptr->arglen, NULL, 0, argv_ptr->p);

    ocrDbDestroy(depv[1].guid);
    return NULL_GUID;
  }

  /*static*/ void OCRProcessor::static_init(void)
  {
    //create the function conversion edt template
    ocrEdtTemplateCreate(&OCRProcessor::ocr_realm_conversion_edt_t, ocr_realm_conversion_func, EDT_PARAM_UNK, EDT_PARAM_UNK);
  }

  /*static*/ void OCRProcessor::static_destroy(void)
  {
    //delete the function conversion edt template
    ocrEdtTemplateDestroy(OCRProcessor::ocr_realm_conversion_edt_t);
  }

  //creates a OCR edt that calls the realm function
  void OCRProcessor::spawn_task(Processor::TaskFuncID func_id,
                                const void *args, size_t arglen,
                               const ProfilingRequestSet &reqs,
                               Event start_event, Event finish_event,
                               int priority)
  {
    //ignores reqs, priority

    //create three dependencies : start_event, args and arglen
    const int num_dep = 2; //start_event and args
    ocrGuid_t db_guid[num_dep];

    db_guid[0] = UNINITIALIZED_GUID; //start_event.evt_guid;

    void *args_copy;
    ocrDbCreate(&db_guid[1], (void **)(&args_copy), arglen, DB_PROP_NONE, NULL_HINT, NO_ALLOC);
    memcpy(args_copy, args, arglen);

    ArgsEDT argv = {task_table[func_id], arglen, me};
    u32 argc = (sizeof(argv)+7) / sizeof(u64);

    //create and call the EDT
    ocrGuid_t ocr_realm_conversion_edt, out_ocr_realm_conversion_edt, persistent_evt_guid;
    ocrEdtCreate(&ocr_realm_conversion_edt, OCRProcessor::ocr_realm_conversion_edt_t, argc,
      (u64*)&argv, num_dep, db_guid, EDT_PROP_NONE, NULL_HINT, &out_ocr_realm_conversion_edt);

    //attach the output of EDT to the finish_event
    ocrAddDependence(out_ocr_realm_conversion_edt, finish_event.evt_guid, 0, DB_MODE_RO);

    //satsify the dependency after setting the sticky event to prevent the
    //EDT from starting before adding dependence to the sticky event
    ocrAddDependence(start_event.evt_guid, ocr_realm_conversion_edt, 0, DB_MODE_RO);
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

}; // namespace Realm

#endif // USE_OCR_LAYER

