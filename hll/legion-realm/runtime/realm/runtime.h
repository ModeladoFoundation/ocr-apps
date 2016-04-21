/* Copyright 2016 Stanford University, NVIDIA Corporation
 * Portions Copyright 2016 Rice University, Intel Corporation
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

// Realm runtime object

#ifndef REALM_RUNTIME_H
#define REALM_RUNTIME_H

#include "processor.h"
#include "redop.h"
#include "custom_serdez.h"

#include "lowlevel_config.h"

#if USE_OCR_LAYER
#include "ocr.h"
#endif // USE_OCR_LAYER

namespace Realm {

    class Runtime {
    protected:
      void *impl;  // hidden internal implementation - this is NOT a transferrable handle

    public:
      Runtime(void);
      Runtime(const Runtime& r) : impl(r.impl) {}
      Runtime& operator=(const Runtime& r) { impl = r.impl; return *this; }

      ~Runtime(void) {}

      static Runtime get_runtime(void);

      bool init(int *argc, char ***argv);

      // this is now just a wrapper around Processor::register_task - consider switching to
      //  that
      bool register_task(Processor::TaskFuncID taskid, Processor::TaskFuncPtr taskptr);

      bool register_reduction(ReductionOpID redop_id, const ReductionOpUntyped *redop);
      template <typename REDOP>
      bool register_reduction(ReductionOpID redop_id)
      {
	return register_reduction(redop_id, ReductionOpUntyped::create_reduction_op<REDOP>());
      }

      bool register_custom_serdez(CustomSerdezID serdez_id, const CustomSerdezUntyped *serdez);
      template <typename SERDEZ>
      bool register_custom_serdez(CustomSerdezID serdez_id)
      {
	return register_custom_serdez(serdez_id, CustomSerdezUntyped::create_custom_serdez<SERDEZ>());
      }

      Event collective_spawn(Processor target_proc, Processor::TaskFuncID task_id, 
			     const void *args, size_t arglen,
			     Event wait_on = Event::NO_EVENT, int priority = 0);

      Event collective_spawn_by_kind(Processor::Kind target_kind, Processor::TaskFuncID task_id, 
				     const void *args, size_t arglen,
				     bool one_per_node = false,
				     Event wait_on = Event::NO_EVENT, int priority = 0);

      // there are three potentially interesting ways to start the initial
      // tasks:
      enum RunStyle {
	ONE_TASK_ONLY,  // a single task on a single node of the machine
	ONE_TASK_PER_NODE, // one task running on one proc of each node
	ONE_TASK_PER_PROC, // a task for every processor in the machine
      };

      void run(Processor::TaskFuncID task_id = 0, RunStyle style = ONE_TASK_ONLY,
	       const void *args = 0, size_t arglen = 0, bool background = false)
	__attribute__((deprecated/*("use collect_spawn calls instead")*/));

      // requests a shutdown of the runtime
      void shutdown(Event wait_on = Event::NO_EVENT);

      void wait_for_shutdown(void);
    };
	
}; // namespace Realm

//include "runtime.inl"

#endif // ifndef REALM_RUNTIME_H

