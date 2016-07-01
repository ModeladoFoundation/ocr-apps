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

#include "realm.h"

using namespace Realm;

enum {
  TOP_LEVEL_TASK = Processor::TASK_ID_FIRST_AVAILABLE+0,
  DUMMY_TASK = Processor::TASK_ID_FIRST_AVAILABLE+1,
};

int count = 0;

void find_processors(Processor &first_cpu, Processor &first_gpu)
{
  // Print out our processors and their kinds
  // Remember the first CPU and GPU processor
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
          if (!first_cpu.exists())
            first_cpu = *it;
          printf("CPU Processor " IDFMT "\n", it->id);
          break;
        }
      case TOC_PROC:
        {
          if (!first_gpu.exists())
            first_gpu = *it;
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

void top_level_task(const void *args, size_t arglen,
                    const void *userdata, size_t userlen, Processor p)
{
  printf("HELLO WORLD from Processor " IDFMT "!\n\n", p.id);

  Processor first_cpu = Processor::NO_PROC;
  Processor first_gpu = Processor::NO_PROC;
  find_processors(first_cpu, first_gpu);

  Reservation r = Reservation::create_reservation();
  std::set<Event> e_lst;

  for(int i=0; i<10; i++)
  {
    Event e_acq = r.acquire();
    //e_acq = Event::NO_EVENT; //uncomment to remove effect of acquire
    Event e_tsk = first_cpu.spawn(DUMMY_TASK, NULL, 0, e_acq);
    r.release(e_tsk);
    e_lst.insert(e_tsk);
  }
  Event done = Event::merge_events(e_lst);
  done.wait();
}

void dummy_task(const void *args, size_t arglen,
                    const void *userdata, size_t userlen, Processor p)
{
  //assert(arglen == 0 && args == NULL);
  int i;
  int temp = count;
  for(int i=0; i<100000; i++);
  printf("%d\n", temp);
  count = temp + 1;
}

int legion_ocr_main(int argc, char **argv)
{
  Runtime rt;

  rt.init(&argc, &argv);

  rt.register_task(TOP_LEVEL_TASK, top_level_task);
  rt.register_task(DUMMY_TASK, dummy_task);

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

  // collective launch of a single task - everybody gets the same finish event
  Event e = rt.collective_spawn(p, TOP_LEVEL_TASK, 0, 0);

  // request shutdown once that task is complete
  rt.shutdown(e);

  // now sleep this thread until that shutdown actually happens
  rt.wait_for_shutdown();

  return 0;
}

