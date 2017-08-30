/* Copyright 2017 Stanford University
 * Portions Copyright 2017 Rice University, Intel Corporation
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


#include <cstdio>

#include "legion.h"

using namespace LegionRuntime::HighLevel;

enum TaskIDs {
  TOP_LEVEL_TASK_ID,
};

// GLOBAL VARIABLES IN LEGION ARE ILLEGAL!
// There is no way for the runtime to know how
// that this variable exists and how to keep it
// up to date across multiple processes running
// on different nodes across the system.
int global_var = 0;

// The one exception to this is constant values.
// This is ok since the value doesn't change
// and is therefore guaranteed to be the same
// across all processes on all machines in the system.
const int global_constant = 4;

// Another thing to be careful of is that function
// pointers are another kind of global variable that
// may differ between multiple processes in the machine.
// Therefore passing function pointers around Legion
// programs is strongly discouraged.
void foo(void)
{

}

void top_level_task(const Task *task,
                    const std::vector<PhysicalRegion> &regions,
                    Context ctx, HighLevelRuntime *runtime)
{
  printf("The value of global_var %d is undefined\n", global_var);

  printf("The value of global_constant %d will always be the same\n", global_constant);

  printf("The function pointer to foo %p may be different on different processors\n", foo);

  // In general, Legion tasks should not allocate
  // memory directly but should instead create
  // logical regions and use them for storing data.
  // The one exception to this is that tasks can
  // use standard C and C++ memory allocation routines
  // as long as the lifetime of the allocation does
  // not exceed the lifetime of the task. Furthermore
  // any pointers referencing the allocation should
  // not be passed to sub-tasks or escape the task's
  // context. Violating either of these conditions
  // will result in a Legion application with
  // undefined behavior.
  void *some_memory = malloc(16*sizeof(int));
  free(some_memory);
}

#if USE_OCR_LAYER
int legion_ocr_main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif // USE_OCR_LAYER
{
  HighLevelRuntime::set_top_level_task_id(TOP_LEVEL_TASK_ID);
  HighLevelRuntime::register_legion_task<top_level_task>(TOP_LEVEL_TASK_ID,
#if USE_OCR_LAYER
      Processor::OCR_PROC, true/*single*/, false/*index*/);
#else
      Processor::LOC_PROC, true/*single*/, false/*index*/);
#endif // USE_OCR_LAYER
  return HighLevelRuntime::start(argc, argv);
}
