/* Copyright 2016 Rice University, Intel Corporation
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

// Processor/ProcessorGroup implementations for OCR Realm

#ifndef OCR_REALM_PROC_IMPL_H
#define OCR_REALM_PROC_IMPL_H

#if USE_OCR_LAYER

#include "proc_impl.h"

namespace Realm {

    //OCR processor that implements the required processor functionalities
    //using OCR calls. Compared to other processor implementaions this
    //doesnt have a scheduler since scheduling is done by OCR
    class OCRProcessor : public ProcessorImpl {
    public:
      //EDT template for the EDT that converts OCR function to realm user function
      static ocrGuid_t ocr_realm_conversion_edt_t;

      OCRProcessor(Processor _me);
      virtual ~OCRProcessor(void);

      static void static_init(void);
      static void static_destroy(void);

      virtual void enqueue_task(Task *task);

      virtual void spawn_task(Processor::TaskFuncID func_id,
                             const void *args, size_t arglen,
                              const ProfilingRequestSet &reqs,
                             Event start_event, Event finish_event,
                              int priority);

      virtual void shutdown(void);

      virtual void add_to_group(ProcessorGroup *group);

      virtual void register_task(Processor::TaskFuncID func_id,
                                CodeDescriptor& codedesc,
                                const ByteArrayRef& user_data);

      struct TaskTableEntry {
        Processor::TaskFuncPtr fnptr;
        ByteArray user_data;
      };

      struct ArgsEDT{
        TaskTableEntry task_entry;
        size_t arglen;
        Processor p;
      };

    protected:
      virtual void execute_task(Processor::TaskFuncID func_id,
                               const ByteArrayRef& task_args);

      std::map<Processor::TaskFuncID, TaskTableEntry> task_table;
    };

}; // namespace Realm

#endif // USE_OCR_LAYER

#endif // OCR_REALM_PROC_IMPL_H
