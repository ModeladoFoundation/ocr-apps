/* Copyright 2017 Rice University, Intel Corporation
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
  //ideally these parameters should come inside argv and not data blocks
  ocrGuid_t ocr_realm_conversion_func(u32 argc, u64 *argv, u32 depc, ocrEdtDep_t depv[])
  {
    assert(argc > 0 && depc == 1);

    OCRProcessor::ArgsEDT *argv_ptr = (OCRProcessor::ArgsEDT *)argv;
    //make sure the current processor is set during execution of the task
    //This might have to move to ELS
    ThreadLocal::current_processor = argv_ptr->p;
    //extract the function pointer to be executed
    Processor::TaskFuncPtr task_func = argv_ptr->fnptr;
    //extract args and arglen and pass it to the function
    assert(task_func!=NULL);
    task_func(argv_ptr->args, argv_ptr->arglen, NULL, 0, argv_ptr->p);

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

    const u64 dest = me.address_space();
    assert(dest == OCRUtil::ocrCurrentPolicyDomain());

    int size = sizeof(ArgsEDT) + arglen;
    int argc = U64_COUNT(size);
    u64 argv[argc];
    ArgsEDT *argv_ptr = (ArgsEDT *) argv;
    argv_ptr->p = me;
    argv_ptr->fnptr = task_table[func_id].fnptr;
    assert(argv_ptr->fnptr!=NULL);
    argv_ptr->arglen = arglen;
    memcpy(argv_ptr->args, args, arglen);

    //create and call the EDT
    ocrGuid_t ocr_realm_conversion_edt, out_ocr_realm_conversion_edt, persistent_evt_guid;
    ocrEdtCreate(&ocr_realm_conversion_edt, OCRProcessor::ocr_realm_conversion_edt_t,
      argc, argv, 1, &start_event.evt_guid, EDT_PROP_OEVT_VALID,
      &(OCRUtil::ocrHintArr[dest]), &finish_event.evt_guid);
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

#if 0
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
#endif

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

