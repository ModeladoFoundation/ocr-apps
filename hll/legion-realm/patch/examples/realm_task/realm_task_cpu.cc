/* Copyright 2017 Rice University, Intel Corporation
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
  HELLO_TASK = Processor::TASK_ID_FIRST_AVAILABLE+1,
};

void find_processors(Processor &first_cpu, Processor &first_gpu, int n)
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
#if USE_OCR_LAYER
       case OCR_PROC:
        {
          if (!first_cpu.exists() && n==0) {
            first_cpu = *it;
	    break;
	  }
          n--;
          printf("OCR Processor " IDFMT "\n", it->id);
          continue;
        }
#endif // USE_OCR_LAYER
      case LOC_PROC:
        {
          if (!first_cpu.exists() && n==0) {
            first_cpu = *it;
	    break;
	  }
          n--;
          printf("CPU Processor " IDFMT "\n", it->id);
          continue;
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

void find_memories(Processor cpu, Processor gpu,
                   Memory &system, Memory &framebuffer)
{
  Machine machine = Machine::get_machine();
  std::set<Memory> visible_mems;
  machine.get_visible_memories(cpu, visible_mems);
  for (std::set<Memory>::const_iterator it = visible_mems.begin();
        it != visible_mems.end(); it++)
  {
    Memory::Kind kind = it->kind();
    switch (kind)
    {
#if USE_OCR_LAYER
      case Memory::OCR_MEM:
        {
          system = *it;
          printf("OCR Memory " IDFMT " for CPU Processor " IDFMT
                 " has capacity %ld MB\n", it->id, cpu.id,
                 (it->capacity() >> 20));
          break;
        }
#endif // USE_OCR_LAYER
      case Memory::SYSTEM_MEM:
        {
          system = *it;
          printf("System Memory " IDFMT " for CPU Processor " IDFMT
                 " has capacity %zd MB\n", it->id, cpu.id,
                 (it->capacity() >> 20));
          break;
        }
      case Memory::Z_COPY_MEM:
        {
          printf("Zero-Copy Memory " IDFMT " for CPU Processor " IDFMT
                 " has capacity %zd MB\n", it->id, cpu.id,
                 (it->capacity() >> 20));
          break;
        }
      default:
        printf("Unknown Memory Kind for CPU: %d\n", kind);
    }
  }
  printf("\n");
  if (gpu.exists())
  {
    visible_mems.clear();
    machine.get_visible_memories(gpu, visible_mems);
    for (std::set<Memory>::const_iterator it = visible_mems.begin();
          it != visible_mems.end(); it++)
    {
      Memory::Kind kind = it->kind();
      switch (kind)
      {
        case Memory::GPU_FB_MEM:
          {
            framebuffer = *it;
            printf("Framebuffer Memory " IDFMT " for GPU Processor " IDFMT
                   " has capacity %zd MB\n", it->id, cpu.id,
                   (it->capacity() >> 20));
            break;
          }
        case Memory::Z_COPY_MEM:
          {
            printf("Zero-Copy Memory " IDFMT " for GPU Processor " IDFMT
                   " has capacity %zd MB\n", it->id, cpu.id,
                   (it->capacity() >> 20));
            break;
          }
        default:
          printf("Unknown Memory Kind for GPU: %d\n", kind);
      }
    }
    printf("\n");
  }
}

void top_level_task(const void *args, size_t arglen,
                    const void *userdata, size_t userlen, Processor p)
{
  printf("TOP LEVEL TASK from Processor " IDFMT "!\n\n", p.id);
  Processor first_cpu = Processor::NO_PROC;
  Processor first_gpu = Processor::NO_PROC;
  //find_processors(first_cpu, first_gpu);

  Memory system_mem = Memory::NO_MEMORY;
  Memory framebuffer_mem = Memory::NO_MEMORY;;
  find_memories(first_cpu, first_gpu, system_mem, framebuffer_mem);

#ifdef USE_GASNET
  int n = gasnet_nodes();
#else
  int n = 1;
#endif

  std::set<Event> ret_events;
  for(int i=0;i<n*2;i++)
  {
    first_cpu = Processor::NO_PROC;
    find_processors(first_cpu, first_gpu, i%n);
    Event ret = first_cpu.spawn(HELLO_TASK, &i, sizeof(i), Event::NO_EVENT);
    ret_events.insert(ret);
  }
  Event done = Event::merge_events(ret_events);
  printf("Done Event is (" IDFMT ")\n\n", done.id);
  done.wait();
}

void hello_task(const void *args, size_t arglen,
                    const void *userdata, size_t userlen, Processor p)
{
  printf("HELLO from Processor " IDFMT " i = %d !\n\n", p.id, *(int*)args);
}

#if USE_OCR_LAYER
int legion_ocr_main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif // USE_OCR_LAYER
{
  Runtime rt;

  rt.init(&argc, &argv);

  rt.register_task(TOP_LEVEL_TASK, top_level_task);
  rt.register_task(HELLO_TASK, hello_task);

  // select a processor to run the top level task on
  Processor p = Processor::NO_PROC;
  {
    std::set<Processor> all_procs;
    Machine::get_machine().get_all_processors(all_procs);
    for(std::set<Processor>::const_iterator it = all_procs.begin();
	it != all_procs.end();
	it++)
#if USE_OCR_LAYER
      if(it->kind() == Processor::OCR_PROC) {
#else
      if(it->kind() == Processor::LOC_PROC) {
#endif // USE_OCR_LAYER
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
