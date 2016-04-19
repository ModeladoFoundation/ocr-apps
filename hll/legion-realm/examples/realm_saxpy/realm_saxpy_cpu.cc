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

#include "realm_saxpy.h"

// Other experiments:
// - Run computation out of Zero-Copy memory
// - Use partitioning API
// - Add profiling

enum {
  TOP_LEVEL_TASK = Processor::TASK_ID_FIRST_AVAILABLE+0,
  INIT_VECTOR_TASK = Processor::TASK_ID_FIRST_AVAILABLE+1,
  CPU_SAXPY_TASK = Processor::TASK_ID_FIRST_AVAILABLE+2,
  GPU_SAXPY_TASK = Processor::TASK_ID_FIRST_AVAILABLE+3,
  CHECK_RESULT_TASK = Processor::TASK_ID_FIRST_AVAILABLE+4,
};

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
                   " has capacity %ld MB\n", it->id, cpu.id, 
                   (it->capacity() >> 20));
            break;
          }
        case Memory::Z_COPY_MEM:
          {
            printf("Zero-Copy Memory " IDFMT " for GPU Processor " IDFMT 
                   " has capacity %ld MB\n", it->id, cpu.id, 
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
  printf("HELLO WORLD from Processor " IDFMT "!\n\n", p.id);
  Processor first_cpu = Processor::NO_PROC;
  Processor first_gpu = Processor::NO_PROC;
  find_processors(first_cpu, first_gpu);

  Memory system_mem = Memory::NO_MEMORY;
  Memory framebuffer_mem = Memory::NO_MEMORY;;
  find_memories(first_cpu, first_gpu, system_mem, framebuffer_mem);

  Rect<1> bounds(Point<1>(0),Point<1>(16383));
  Domain dom = Domain::from_rect<1>(bounds);  

  RegionInstance cpu_inst_x = dom.create_instance(system_mem, sizeof(float));
  RegionInstance cpu_inst_y = dom.create_instance(system_mem, sizeof(float));
  RegionInstance cpu_inst_z = dom.create_instance(system_mem, sizeof(float));
  printf("Created System Memory Instances: " IDFMT ", " IDFMT ", and " IDFMT "\n\n", 
      cpu_inst_x.id, cpu_inst_y.id, cpu_inst_z.id);

  Domain::CopySrcDstField cpu_x_field(cpu_inst_x, 0/*offset*/, sizeof(float));
  Domain::CopySrcDstField cpu_y_field(cpu_inst_y, 0/*offset*/, sizeof(float));
  Domain::CopySrcDstField cpu_z_field(cpu_inst_z, 0/*offset*/, sizeof(float));

  drand48();
  float init_x_value = drand48();
  float init_y_value = drand48();

  Event fill_x;
  {
    std::vector<Domain::CopySrcDstField> fill_vec;
    fill_vec.push_back(cpu_x_field);
    fill_x = dom.fill(fill_vec, &init_x_value, sizeof(init_x_value));
  }
  Event fill_y;
  {
    std::vector<Domain::CopySrcDstField> fill_vec;
    fill_vec.push_back(cpu_y_field);
    fill_y = dom.fill(fill_vec, &init_y_value, sizeof(init_y_value));
  }

  Event z_ready = Event::NO_EVENT;
  SaxpyArgs saxpy_args;
  saxpy_args.x_inst = cpu_inst_x;
  saxpy_args.y_inst = cpu_inst_y;
  saxpy_args.z_inst = cpu_inst_z;
  saxpy_args.alpha = drand48();
  saxpy_args.bounds = bounds;
  if (first_gpu.exists())
  {
    // Run the computation on the GPU
    // Make instances on the GPU
    RegionInstance gpu_inst_x = dom.create_instance(framebuffer_mem, sizeof(float));
    RegionInstance gpu_inst_y = dom.create_instance(framebuffer_mem, sizeof(float));
    RegionInstance gpu_inst_z = dom.create_instance(framebuffer_mem, sizeof(float));

    printf("Created Framebuffer Memory Instances: " IDFMT ", " IDFMT ", and " IDFMT "\n\n", 
      gpu_inst_x.id, gpu_inst_y.id, gpu_inst_z.id);

    Domain::CopySrcDstField gpu_x_field(gpu_inst_x, 0/*offset*/, sizeof(float));
    Domain::CopySrcDstField gpu_y_field(gpu_inst_y, 0/*offset*/, sizeof(float));
    Domain::CopySrcDstField gpu_z_field(gpu_inst_z, 0/*offset*/, sizeof(float));

    // Copy down
    Event copy_x;
    {
      std::vector<Domain::CopySrcDstField> srcs, dsts;
      srcs.push_back(cpu_x_field);
      dsts.push_back(gpu_x_field);
      copy_x = dom.copy(srcs, dsts, fill_x);
    }
    Event copy_y;
    {
      std::vector<Domain::CopySrcDstField> srcs, dsts;
      srcs.push_back(cpu_y_field);
      dsts.push_back(gpu_y_field);
      copy_y = dom.copy(srcs, dsts, fill_y);
    }

    SaxpyArgs gpu_args;
    gpu_args.x_inst = gpu_inst_x;
    gpu_args.y_inst = gpu_inst_y;
    gpu_args.z_inst = gpu_inst_z;
    gpu_args.alpha  = saxpy_args.alpha;
    gpu_args.bounds = bounds;

    Event precondition = Event::merge_events(copy_x, copy_y);
    Event gpu_done = first_gpu.spawn(GPU_SAXPY_TASK, &gpu_args,
                                     sizeof(gpu_args), precondition);
    // Copy back
    {
      std::vector<Domain::CopySrcDstField> srcs, dsts;
      srcs.push_back(gpu_z_field);
      dsts.push_back(cpu_z_field);
      z_ready = dom.copy(srcs, dsts, gpu_done);
    }
  }
  else
  {
    // Run the computation on the CPU
    Event precondition = Event::merge_events(fill_x, fill_y);
    z_ready = first_cpu.spawn(CPU_SAXPY_TASK, &saxpy_args, 
                              sizeof(saxpy_args), precondition);
  }

  // Run our checker task
  Event done = first_cpu.spawn(CHECK_RESULT_TASK, &saxpy_args, 
                                sizeof(saxpy_args), z_ready); 
  printf("Done Event is (" IDFMT ",%d)\n\n", done.id, done.gen);
  done.wait();
}

void cpu_saxpy_task(const void *args, size_t arglen,
                    const void *userdata, size_t userlen, Processor p)
{
  assert(arglen == sizeof(SaxpyArgs));
  const SaxpyArgs *saxpy_args = (const SaxpyArgs*)args;
  printf("Running CPU Saxpy Task\n\n");
  Rect<1> actual_bounds;
  ByteOffset offsets;
  const float *x_ptr = (const float*)saxpy_args->x_inst.get_accessor().
          raw_dense_ptr<1>(saxpy_args->bounds, actual_bounds, offsets);
  assert(actual_bounds == saxpy_args->bounds);
  const float *y_ptr = (const float*)saxpy_args->y_inst.get_accessor().
          raw_dense_ptr<1>(saxpy_args->bounds, actual_bounds, offsets);
  assert(actual_bounds == saxpy_args->bounds);
  float *z_ptr = (float*)saxpy_args->z_inst.get_accessor().
          raw_dense_ptr<1>(saxpy_args->bounds, actual_bounds, offsets);
  size_t num_elements = actual_bounds.volume();
  // Here is the actual saxpy code
  for (unsigned idx = 0; idx < num_elements; idx++)
    z_ptr[idx] = saxpy_args->alpha * x_ptr[idx] + y_ptr[idx];
}

void check_result_task(const void *args, size_t arglen,
                       const void *userdata, size_t userlen, Processor)
{
  assert(arglen == sizeof(SaxpyArgs));
  const SaxpyArgs *saxpy_args = (const SaxpyArgs*)args;
  printf("Running Checking Task...");
  Rect<1> actual_bounds;
  ByteOffset offsets;
  const float *x_ptr = (const float*)saxpy_args->x_inst.get_accessor().
          raw_dense_ptr<1>(saxpy_args->bounds, actual_bounds, offsets);
  assert(actual_bounds == saxpy_args->bounds);
  const float *y_ptr = (const float*)saxpy_args->y_inst.get_accessor().
          raw_dense_ptr<1>(saxpy_args->bounds, actual_bounds, offsets);
  assert(actual_bounds == saxpy_args->bounds);
  const float *z_ptr = (const float*)saxpy_args->z_inst.get_accessor().
          raw_dense_ptr<1>(saxpy_args->bounds, actual_bounds, offsets);
  size_t num_elements = actual_bounds.volume();
  bool success = true;
  for (unsigned idx = 0; idx < num_elements; idx++)
  {
    float expected = saxpy_args->alpha * x_ptr[idx] + y_ptr[idx];
    //printf("verify %f %f %f %f\n", x_ptr[idx], y_ptr[idx], z_ptr[idx], expected); 
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

#ifdef USE_CUDA
extern void gpu_saxpy_task(const void *args, size_t arglen,
                           const void *userdata, size_t userlen, Processor p);
#endif

int legion_ocr_main(int argc, char **argv)
{
  Runtime rt;

  rt.init(&argc, &argv);

  rt.register_task(TOP_LEVEL_TASK, top_level_task);
  rt.register_task(CPU_SAXPY_TASK, cpu_saxpy_task);
#ifdef USE_CUDA
  rt.register_task(GPU_SAXPY_TASK, gpu_saxpy_task);
#endif
  rt.register_task(CHECK_RESULT_TASK, check_result_task);
  
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

