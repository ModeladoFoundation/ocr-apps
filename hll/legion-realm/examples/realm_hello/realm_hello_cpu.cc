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

#include "realm_hello.h"

enum {
  TOP_LEVEL_TASK = Processor::TASK_ID_FIRST_AVAILABLE+0,
  FIRST_WORKER_TASK = Processor::TASK_ID_FIRST_AVAILABLE+1,
  SECOND_WORKER_TASK = Processor::TASK_ID_FIRST_AVAILABLE+2,
};

void find_processors(Processor &first_cpu)
{
  // Print out our processors and their kinds
  // Remember the first  OCR CPU
  Machine machine = Machine::get_machine();
  std::set<Processor> all_procs;
  machine.get_all_processors(all_procs);
  for (std::set<Processor>::const_iterator it = all_procs.begin();
        it != all_procs.end(); it++)
  {
    Processor::Kind kind = it->kind();  
    switch (kind)
    {
       case OCR_PROC:
        {
          if (!first_cpu.exists())
            first_cpu = *it;
          printf("OCR Processor " IDFMT "\n", it->id);
          break;
        }

      case LOC_PROC:
        {
          printf("CPU Processor " IDFMT "\n", it->id);
          break;
        }
      case TOC_PROC:
        {
          printf("GPU Processor " IDFMT "\n", it->id);
          break;
        }
      case UTIL_PROC:
        {
          printf("Utility Processor " IDFMT "\n", it->id);
          break;
        }
      case IO_PROC:
        {
          printf("I/O Processor " IDFMT "\n", it->id);
          break;
        }
      case PROC_GROUP:
        {
          printf("Processor Group " IDFMT "\n", it->id);
          break;
        }
      default:
        assert(false);
    }
  }
  printf("\n");
}


void first_worker_task(const void *args, size_t arglen,
                    const void *userdata, size_t userlen, Processor p)
{
  printf("first worker from Processor " IDFMT "\n len = %ld, val = %d\n", p.id, arglen, *(int*)args);
  Processor second_cpu = Processor::NO_PROC;
  find_processors(second_cpu);
  int new_val = *(int*)args+10;
  second_cpu.spawn(SECOND_WORKER_TASK, &new_val, sizeof(int), Event::NO_EVENT);
}

void second_worker_task(const void *args, size_t arglen,
                    const void *userdata, size_t userlen, Processor p)
{
  printf("second worker from Processor " IDFMT "\n len = %ld, val = %d\n", p.id, arglen, *(int*)args);
}

void top_level_task(const void *args, size_t arglen,
                    const void *userdata, size_t userlen, Processor p)
{ 
  printf("top level task from Processor " IDFMT "\n len = %ld, val = %d\n", p.id, arglen, *(int*)args);
  int i=1, j=2, k=3;
  Processor first_cpu = Processor::NO_PROC;
  find_processors(first_cpu);
  Event done1 = first_cpu.spawn(FIRST_WORKER_TASK, &i, sizeof(int), Event::NO_EVENT);

  Event done2 = first_cpu.spawn(FIRST_WORKER_TASK, &j, sizeof(int), Event::NO_EVENT);

  Processor second_cpu = Processor::NO_PROC;
  find_processors(second_cpu);
  Event done3 = first_cpu.spawn(SECOND_WORKER_TASK, &k, sizeof(int), Event::NO_EVENT);

  done1.wait();
  done2.wait();
  done3.wait();

  Runtime rt = Runtime::get_runtime();
  rt.shutdown();
}

int main(int argc, char **argv)
{
  Runtime rt;

  int ret = rt.init(&argc, &argv);
  printf("init return %d\n", ret);

  rt.register_task(TOP_LEVEL_TASK, top_level_task);
  rt.register_task(FIRST_WORKER_TASK, first_worker_task);
  rt.register_task(SECOND_WORKER_TASK, second_worker_task);
 
  //rt.run(TOP_LEVEL_TASK, Runtime::ONE_TASK_ONLY, &argc, sizeof(int));
  
  // select a processor to run the top level task on
  Processor p = Processor::NO_PROC;
  {
    std::set<Processor> all_procs;
    Machine::get_machine().get_all_processors(all_procs);
    for(std::set<Processor>::const_iterator it = all_procs.begin();
	it != all_procs.end();
	it++)
      if(it->kind() == Processor::OCR_PROC) {
	p = *it;
	break;
      }
  }
  assert(p.exists());

  Event e = rt.collective_spawn(p, TOP_LEVEL_TASK, &argc, sizeof(int));
  
  // sleep this thread until shutdown actually happens
  rt.wait_for_shutdown();

  return 0;
}
