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
  CALCULATE_TASK = Processor::TASK_ID_FIRST_AVAILABLE+3,
  VERIFY_TASK = Processor::TASK_ID_FIRST_AVAILABLE+4,
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

void find_memories(Processor cpu,
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
        case Memory::OCR_MEM:
        {
          system = *it;
          printf("OCR Memory " IDFMT " for CPU Processor " IDFMT
                 " has capacity %ld MB\n", it->id, cpu.id,
                 (it->capacity() >> 20));
          break;
        }

      case Memory::SYSTEM_MEM:
        {
          system = *it;
          printf("System Memory " IDFMT " for CPU Processor " IDFMT
                 " has capacity %ld MB\n", it->id, cpu.id,
                 (it->capacity() >> 20));
          break;
        }
      case Memory::Z_COPY_MEM:
        {
          printf("Zero-Copy Memory " IDFMT " for CPU Processor " IDFMT
                 " has capacity %ld MB\n", it->id, cpu.id,
                 (it->capacity() >> 20));
          break;
        }
      default:
        printf("Unknown Memory Kind for CPU: %d\n", kind);
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

void calculate_task(const void *args, size_t arglen,
                    const void *userdata, size_t userlen, Processor p)
{
  assert(arglen == sizeof(HelloArgs));
  const HelloArgs *hello_args = (const HelloArgs*)args;
  printf("Running calculate Task\n\n");
  Rect<1> actual_bounds;
  ByteOffset offsets;
  float *x_ptr = (float*)hello_args->x_inst.get_accessor().
          raw_dense_ptr<1>(hello_args->bounds, actual_bounds, offsets);
  assert(actual_bounds == hello_args->bounds);
  float *y_ptr = (float*)hello_args->y_inst.get_accessor().
          raw_dense_ptr<1>(hello_args->bounds, actual_bounds, offsets);
  assert(actual_bounds == hello_args->bounds);
  float *z_ptr = (float*)hello_args->z_inst.get_accessor().
          raw_dense_ptr<1>(hello_args->bounds, actual_bounds, offsets);
  size_t num_elements = actual_bounds.volume();
  drand48();
  float init_x_value = drand48();
  float init_y_value = drand48();

  // Here is the actual calulation code
  for (unsigned idx = 0; idx < num_elements; idx++)
  {
    x_ptr[idx] = init_x_value; y_ptr[idx] = init_y_value;
    z_ptr[idx] = 2*x_ptr[idx] + 3*y_ptr[idx];
    //printf("calc %d, %f %f %f\n", idx, x_ptr[idx], y_ptr[idx], z_ptr[idx]);
  }
}

void verify_task(const void *args, size_t arglen,
                    const void *userdata, size_t userlen, Processor p)
{
  assert(arglen == sizeof(HelloArgs));
  const HelloArgs *hello_args = (const HelloArgs*)args;
  printf("Running verify Task\n\n");
  Rect<1> actual_bounds;
  ByteOffset offsets;
  const float *x_ptr = (const float*)hello_args->x_inst.get_accessor().
          raw_dense_ptr<1>(hello_args->bounds, actual_bounds, offsets);
  assert(actual_bounds == hello_args->bounds);
  const float *y_ptr = (const float*)hello_args->y_inst.get_accessor().
          raw_dense_ptr<1>(hello_args->bounds, actual_bounds, offsets);
  assert(actual_bounds == hello_args->bounds);
  float *z_ptr = (float*)hello_args->z_inst.get_accessor().
          raw_dense_ptr<1>(hello_args->bounds, actual_bounds, offsets);
  size_t num_elements = actual_bounds.volume();

  bool success = true;
  for (unsigned idx = 0; idx < num_elements; idx++)
  {
    float expected = 2*x_ptr[idx] + 3*y_ptr[idx];
    //printf("verify %d, %f %f %f %f\n", idx, x_ptr[idx], y_ptr[idx], z_ptr[idx], expected);
    float actual = z_ptr[idx];
    // FMAs are too acurate
    float diff = (actual >= expected) ? actual - expected : expected - actual;
    float relative = diff / expected;
    if (relative > 1e-6)
    {
      printf("Expected: %.8g Actual: %.8g\n", expected, actual);
      success = false;
      break;
    }
  }
  if (success)
    printf("SUCCESS!\n\n");
  else
    printf("FAILURE!\n\n");
}

void top_level_task(const void *args, size_t arglen,
                    const void *userdata, size_t userlen, Processor p)
{ 
  printf("top level task from Processor " IDFMT "\n len = %ld, val = %d\n", p.id, arglen, *(int*)args);
  int i=1, j=2, k=3;
  Processor first_cpu = Processor::NO_PROC;
  find_processors(first_cpu);

  Memory system_mem = Memory::NO_MEMORY;
  Memory framebuffer_mem = Memory::NO_MEMORY;;
  find_memories(first_cpu, system_mem, framebuffer_mem);

  Rect<1> bounds(Point<1>(0),Point<1>(16));
  Domain dom = Domain::from_rect<1>(bounds);

  RegionInstance cpu_inst_x = dom.create_instance(system_mem, sizeof(float));
  RegionInstance cpu_inst_y = dom.create_instance(system_mem, sizeof(float));
  RegionInstance cpu_inst_z = dom.create_instance(system_mem, sizeof(float));

  HelloArgs ha;
  ha.x_inst = cpu_inst_x;
  ha.y_inst = cpu_inst_y;
  ha.z_inst = cpu_inst_z;
  ha.bounds = bounds;

  Event done_calc = first_cpu.spawn(CALCULATE_TASK, &ha, sizeof(ha), Event::NO_EVENT);

  Event done_verify = first_cpu.spawn(VERIFY_TASK, &ha, sizeof(ha), done_calc);

  Event done1 = first_cpu.spawn(FIRST_WORKER_TASK, &i, sizeof(int), Event::NO_EVENT);

  Event done2 = first_cpu.spawn(FIRST_WORKER_TASK, &j, sizeof(int), Event::NO_EVENT);

  Processor second_cpu = Processor::NO_PROC;
  find_processors(second_cpu);
  Event done3 = first_cpu.spawn(SECOND_WORKER_TASK, &k, sizeof(int), Event::NO_EVENT);

  done1.wait();
  done2.wait();
  done3.wait();
  done_verify.wait();
}

int legion_ocr_main(int argc, char **argv)
{
  Runtime rt;

  int ret = rt.init(&argc, &argv);
  printf("init return %d\n", ret);
  
  rt.register_task(TOP_LEVEL_TASK, top_level_task);
  rt.register_task(FIRST_WORKER_TASK, first_worker_task);
  rt.register_task(SECOND_WORKER_TASK, second_worker_task);
  rt.register_task(CALCULATE_TASK, calculate_task);
  rt.register_task(VERIFY_TASK, verify_task);
 
  //deprecated
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

  // request shutdown once that task is complete
  rt.shutdown(e);
  
  // sleep this thread until shutdown actually happens
  rt.wait_for_shutdown();
  
  return 0;
}

