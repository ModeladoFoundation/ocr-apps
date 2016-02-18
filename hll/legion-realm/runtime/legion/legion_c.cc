/* Copyright 2016 Stanford University
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

#include "legion.h"
#include "legion_c.h"
#include "legion_c_util.h"
#include "utilities.h"
#include "default_mapper.h"

#ifndef USE_LEGION_PARTAPI_SHIM
#ifdef SHARED_LOWLEVEL
#define USE_LEGION_PARTAPI_SHIM 0
#else
// General LLR can't handle new partion API yet. Use a shim instead.
#define USE_LEGION_PARTAPI_SHIM 1
#endif
#endif

using namespace LegionRuntime;
using namespace LegionRuntime::HighLevel;
using namespace LegionRuntime::HighLevel::MappingUtilities;
typedef CObjectWrapper::Generic Generic;
typedef CObjectWrapper::SOA SOA;
typedef CObjectWrapper::AccessorGeneric AccessorGeneric;
typedef CObjectWrapper::AccessorArray AccessorArray;

// -----------------------------------------------------------------------
// Pointer Operations
// -----------------------------------------------------------------------

legion_ptr_t
legion_ptr_safe_cast(legion_runtime_t runtime_,
                     legion_context_t ctx_,
                     legion_ptr_t pointer_,
                     legion_logical_region_t region_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  ptr_t pointer = CObjectWrapper::unwrap(pointer_);
  LogicalRegion region = CObjectWrapper::unwrap(region_);

  ptr_t result = runtime->safe_cast(ctx, pointer, region);
  return CObjectWrapper::wrap(result);
}

// -----------------------------------------------------------------------
// Domain Operations
// -----------------------------------------------------------------------

legion_domain_t
legion_domain_from_rect_1d(legion_rect_1d_t r_)
{
  Rect<1> r = CObjectWrapper::unwrap(r_);

  return CObjectWrapper::wrap(Domain::from_rect<1>(r));
}

legion_domain_t
legion_domain_from_rect_2d(legion_rect_2d_t r_)
{
  Rect<2> r = CObjectWrapper::unwrap(r_);

  return CObjectWrapper::wrap(Domain::from_rect<2>(r));
}

legion_domain_t
legion_domain_from_rect_3d(legion_rect_3d_t r_)
{
  Rect<3> r = CObjectWrapper::unwrap(r_);

  return CObjectWrapper::wrap(Domain::from_rect<3>(r));
}

legion_rect_1d_t
legion_domain_get_rect_1d(legion_domain_t d_)
{
  Domain d = CObjectWrapper::unwrap(d_);

  return CObjectWrapper::wrap(d.get_rect<1>());
}

legion_rect_2d_t
legion_domain_get_rect_2d(legion_domain_t d_)
{
  Domain d = CObjectWrapper::unwrap(d_);

  return CObjectWrapper::wrap(d.get_rect<2>());
}

legion_rect_3d_t
legion_domain_get_rect_3d(legion_domain_t d_)
{
  Domain d = CObjectWrapper::unwrap(d_);

  return CObjectWrapper::wrap(d.get_rect<3>());
}

#if 0
size_t
legion_domain_get_volume(legion_domain_t d_)
{
  Domain d = CObjectWrapper::unwrap(d_);

  return d.get_volume();
}

legion_domain_t
legion_domain_from_index_space(legion_index_space_t is_)
{
  IndexSpace is = CObjectWrapper::unwrap(is_);

  return CObjectWrapper::wrap(Domain(is));
}
#endif

// -----------------------------------------------------------------------
// Domain Point Operations
// -----------------------------------------------------------------------

legion_domain_point_t
legion_domain_point_from_point_1d(legion_point_1d_t p_)
{
  Point<1> p = CObjectWrapper::unwrap(p_);

  return CObjectWrapper::wrap(DomainPoint::from_point<1>(p));
}

legion_domain_point_t
legion_domain_point_from_point_2d(legion_point_2d_t p_)
{
  Point<2> p = CObjectWrapper::unwrap(p_);

  return CObjectWrapper::wrap(DomainPoint::from_point<2>(p));
}

legion_domain_point_t
legion_domain_point_from_point_3d(legion_point_3d_t p_)
{
  Point<3> p = CObjectWrapper::unwrap(p_);

  return CObjectWrapper::wrap(DomainPoint::from_point<3>(p));
}

legion_domain_point_t
legion_domain_point_safe_cast(legion_runtime_t runtime_,
                              legion_context_t ctx_,
                              legion_domain_point_t point_,
                              legion_logical_region_t region_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  DomainPoint point = CObjectWrapper::unwrap(point_);
  LogicalRegion region = CObjectWrapper::unwrap(region_);

  DomainPoint result = runtime->safe_cast(ctx, point, region);
  return CObjectWrapper::wrap(result);
}

// -------------------------------------------------------
// Coloring Operations
// -------------------------------------------------------

legion_coloring_t
legion_coloring_create(void)
{
  Coloring *coloring = new Coloring();

  return CObjectWrapper::wrap(coloring);
}

void
legion_coloring_destroy(legion_coloring_t handle_)
{
  Coloring *handle = CObjectWrapper::unwrap(handle_);

  delete handle;
}

void
legion_coloring_ensure_color(legion_coloring_t handle_,
                             legion_color_t color)
{
  Coloring *handle = CObjectWrapper::unwrap(handle_);

  (*handle)[color];
}

void
legion_coloring_add_point(legion_coloring_t handle_,
                          legion_color_t color,
                          legion_ptr_t point_)
{
  Coloring *handle = CObjectWrapper::unwrap(handle_);
  ptr_t point = CObjectWrapper::unwrap(point_);

  (*handle)[color].points.insert(point);
}

void
legion_coloring_delete_point(legion_coloring_t handle_,
                             legion_color_t color,
                             legion_ptr_t point_)
{
  Coloring *handle = CObjectWrapper::unwrap(handle_);
  ptr_t point = CObjectWrapper::unwrap(point_);

  (*handle)[color].points.erase(point);
}

bool
legion_coloring_has_point(legion_coloring_t handle_,
                          legion_color_t color,
                          legion_ptr_t point_)
{
  Coloring *handle = CObjectWrapper::unwrap(handle_);
  ptr_t point = CObjectWrapper::unwrap(point_);
  std::set<ptr_t>& points = (*handle)[color].points;

  return points.find(point) != points.end();
}

void
legion_coloring_add_range(legion_coloring_t handle_,
                          legion_color_t color,
                          legion_ptr_t start_,
                          legion_ptr_t end_)
{
  Coloring *handle = CObjectWrapper::unwrap(handle_);
  ptr_t start = CObjectWrapper::unwrap(start_);
  ptr_t end = CObjectWrapper::unwrap(end_);

  (*handle)[color].ranges.insert(std::pair<ptr_t, ptr_t>(start, end));
}

// -----------------------------------------------------------------------
// Domain Coloring Operations
// -----------------------------------------------------------------------

legion_domain_coloring_t
legion_domain_coloring_create(void)
{
  return CObjectWrapper::wrap(new DomainColoring());
}

void
legion_domain_coloring_destroy(legion_domain_coloring_t handle)
{
  delete CObjectWrapper::unwrap(handle);
}

void
legion_domain_coloring_color_domain(legion_domain_coloring_t dc_,
                                    legion_color_t color,
                                    legion_domain_t domain_)
{
  DomainColoring *dc = CObjectWrapper::unwrap(dc_);
  Domain domain = CObjectWrapper::unwrap(domain_);
  (*dc)[color] = domain;
}

legion_domain_t
legion_domain_coloring_get_color_space(legion_domain_coloring_t handle_)
{
  DomainColoring *handle = CObjectWrapper::unwrap(handle_);
  Color color_min = (Color)-1, color_max = 0;
  for(std::map<Color,Domain>::iterator it = handle->begin(),
        ie = handle->end(); it != ie; it++) {
    color_min = std::min(color_min, it->first);
    color_max = std::max(color_max, it->first);
  }
  Domain domain = Domain::from_rect<1>(
    Rect<1>(Point<1>(color_min), Point<1>(color_max)));
  return CObjectWrapper::wrap(domain);
}

// -------------------------------------------------------
// Index Space Operations
// -------------------------------------------------------

legion_index_space_t
legion_index_space_create(legion_runtime_t runtime_,
                          legion_context_t ctx_,
                          size_t max_num_elmts)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);

  IndexSpace is = runtime->create_index_space(ctx, max_num_elmts);
  return CObjectWrapper::wrap(is);
}

legion_index_space_t
legion_index_space_create_domain(legion_runtime_t runtime_,
                                 legion_context_t ctx_,
                                 legion_domain_t domain_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  Domain domain = CObjectWrapper::unwrap(domain_);

  IndexSpace is = runtime->create_index_space(ctx, domain);
  return CObjectWrapper::wrap(is);
}

void
legion_index_space_destroy(legion_runtime_t runtime_,
                           legion_context_t ctx_,
                           legion_index_space_t handle_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexSpace handle = CObjectWrapper::unwrap(handle_);

  runtime->destroy_index_space(ctx, handle);
}

legion_domain_t
legion_index_space_get_domain(legion_runtime_t runtime_,
                              legion_context_t ctx_,
                              legion_index_space_t handle_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexSpace handle = CObjectWrapper::unwrap(handle_);

  return CObjectWrapper::wrap(runtime->get_index_space_domain(ctx, handle));
}

void
legion_index_space_attach_name(legion_runtime_t runtime_,
                               legion_index_space_t handle_,
                               const char *name)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  IndexSpace handle = CObjectWrapper::unwrap(handle_);

  runtime->attach_name(handle, name);
}

void
legion_index_space_retrieve_name(legion_runtime_t runtime_,
                                 legion_index_space_t handle_,
                                 const char **result)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  IndexSpace handle = CObjectWrapper::unwrap(handle_);

  runtime->retrieve_name(handle, *result);
}

//------------------------------------------------------------------------
// Index Partition Operations
//------------------------------------------------------------------------

legion_index_partition_t
legion_index_partition_create_coloring(
  legion_runtime_t runtime_,
  legion_context_t ctx_,
  legion_index_space_t parent_,
  legion_coloring_t coloring_,
  bool disjoint,
  int part_color /* = AUTO_GENERATE_ID */)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexSpace parent = CObjectWrapper::unwrap(parent_);
  Coloring *coloring = CObjectWrapper::unwrap(coloring_);

  IndexPartition ip =
    runtime->create_index_partition(ctx, parent, *coloring, disjoint,
                                    part_color);
  return CObjectWrapper::wrap(ip);
}

legion_index_partition_t
legion_index_partition_create_domain_coloring(
  legion_runtime_t runtime_,
  legion_context_t ctx_,
  legion_index_space_t parent_,
  legion_domain_t color_space_,
  legion_domain_coloring_t coloring_,
  bool disjoint,
  int part_color /* = AUTO_GENERATE_ID */)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexSpace parent = CObjectWrapper::unwrap(parent_);
  Domain color_space = CObjectWrapper::unwrap(color_space_);
  DomainColoring *coloring = CObjectWrapper::unwrap(coloring_);

  IndexPartition ip =
    runtime->create_index_partition(ctx, parent, color_space, *coloring,
                                    disjoint, part_color);
  return CObjectWrapper::wrap(ip);
}

legion_index_partition_t
legion_index_partition_create_blockify_1d(
  legion_runtime_t runtime_,
  legion_context_t ctx_,
  legion_index_space_t parent_,
  legion_blockify_1d_t blockify_,
  int part_color /* = AUTO_GENERATE_ID */)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexSpace parent = CObjectWrapper::unwrap(parent_);
  Blockify<1> blockify = CObjectWrapper::unwrap(blockify_);

  IndexPartition ip =
    runtime->create_index_partition(ctx, parent, blockify, part_color);
  return CObjectWrapper::wrap(ip);
}

legion_index_partition_t
legion_index_partition_create_blockify_2d(
  legion_runtime_t runtime_,
  legion_context_t ctx_,
  legion_index_space_t parent_,
  legion_blockify_2d_t blockify_,
  int part_color /* = AUTO_GENERATE_ID */)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexSpace parent = CObjectWrapper::unwrap(parent_);
  Blockify<2> blockify = CObjectWrapper::unwrap(blockify_);

  IndexPartition ip =
    runtime->create_index_partition(ctx, parent, blockify, part_color);
  return CObjectWrapper::wrap(ip);
}

legion_index_partition_t
legion_index_partition_create_blockify_3d(
  legion_runtime_t runtime_,
  legion_context_t ctx_,
  legion_index_space_t parent_,
  legion_blockify_3d_t blockify_,
  int part_color /* = AUTO_GENERATE_ID */)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexSpace parent = CObjectWrapper::unwrap(parent_);
  Blockify<3> blockify = CObjectWrapper::unwrap(blockify_);

  IndexPartition ip =
    runtime->create_index_partition(ctx, parent, blockify, part_color);
  return CObjectWrapper::wrap(ip);
}

#if USE_LEGION_PARTAPI_SHIM
// The Legion runtime can't handle colorings with 0D (unstructured)
// points, so upgrade them to 1D.
static DomainPoint upgrade_point(DomainPoint p)
{
  if (p.get_dim() == 0) {
    return DomainPoint::from_point<1>(Point<1>(p[0]));
  }
  return p;
}
#endif

// Shim for Legion Dependent Partition API

#if USE_LEGION_PARTAPI_SHIM
class PartitionEqualShim {
public:
  static TaskID register_task();
  static IndexPartition launch(HighLevelRuntime *runtime,
                               Context ctx,
                               IndexSpace handle,
                               const Domain &color_space,
                               size_t granularity,
                               int color = AUTO_GENERATE_ID,
                               bool allocable = false);
  static IndexPartition task(const Task *task,
                             const std::vector<PhysicalRegion> &regions,
                             Context ctx, HighLevelRuntime *runtime);
private:
  static const TaskID task_id = 586659; // a "unique" number
  struct Args {
    IndexSpace handle;
    Domain color_space;
    size_t granularity;
    int color;
    bool allocable;
  };
};

Processor::TaskFuncID
PartitionEqualShim::register_task()
{
  return HighLevelRuntime::register_legion_task<IndexPartition, task>(
    task_id, Processor::LOC_PROC, true, false,
    AUTO_GENERATE_ID, TaskConfigOptions(),
    "PartitionEqualShim::task");
}

IndexPartition
PartitionEqualShim::launch(HighLevelRuntime *runtime,
                           Context ctx,
                           IndexSpace handle,
                           const Domain &color_space,
                           size_t granularity,
                           int color,
                           bool allocable)
{
  Args args;
  args.handle = handle;
  args.color_space = color_space;
  args.granularity = granularity;
  args.color = color;
  args.allocable = allocable;
  TaskArgument targs(&args, sizeof(args));
  TaskLauncher task(task_id, targs);
  Future f = runtime->execute_task(ctx, task);
  return f.get_result<IndexPartition>();
}

IndexPartition
PartitionEqualShim::task(const Task *task,
                           const std::vector<PhysicalRegion> &regions,
                           Context ctx, HighLevelRuntime *runtime)
{
  assert(task->arglen == sizeof(Args));
  Args &args = *(Args *)task->args;
  assert(args.granularity == 1);

  size_t total = 0;
  for (IndexIterator it(runtime, ctx, args.handle); it.has_next();) {
    size_t count = 0;
    it.next_span(count);
    total += count;
  }

  PointColoring coloring;
  size_t chunks = args.color_space.get_volume();
  size_t chunksize = total / chunks;
  size_t leftover = total % chunks;
  size_t elt = 0;
  Domain::DomainPointIterator c(args.color_space);
  for (IndexIterator it(runtime, ctx, args.handle); it.has_next();) {
    size_t count = 0;
    ptr_t start = it.next_span(count);
    size_t chunk = chunksize + (leftover > 0 ? 1 : 0);

    while (elt + count >= chunk) {
      size_t rest = chunk - elt;
      assert(c);
      coloring[upgrade_point(c.p)].ranges.insert(std::pair<ptr_t, ptr_t>(start, start.value + rest - 1));
      start.value += rest;
      count -= rest;
      elt = 0;
      c++;
      if (chunk > chunksize) {
        leftover--;
      }
      chunk = chunksize + (leftover > 0 ? 1 : 0);
    }

    if (count > 0) {
      assert(c);
      coloring[upgrade_point(c.p)].ranges.insert(std::pair<ptr_t, ptr_t>(start, start.value + count - 1));
      elt += count;
    }
  }

  IndexPartition ip =
    runtime->create_index_partition(
      ctx, args.handle, args.color_space, coloring,
      DISJOINT_KIND, args.color, args.allocable);
  return ip;
}

static TaskID force_PartitionEqualShim_static_initialize =
  PartitionEqualShim::register_task();
#endif

legion_index_partition_t
legion_index_partition_create_equal(legion_runtime_t runtime_,
                                    legion_context_t ctx_,
                                    legion_index_space_t parent_,
                                    legion_domain_t color_space_,
                                    size_t granularity,
                                    int color /* = AUTO_GENERATE_ID */,
                                    bool allocable /* = false */)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexSpace parent = CObjectWrapper::unwrap(parent_);
  Domain color_space = CObjectWrapper::unwrap(color_space_);

  IndexPartition ip =
#if USE_LEGION_PARTAPI_SHIM
    PartitionEqualShim::launch(runtime, ctx, parent, color_space, granularity,
                               color, allocable);
#else
    runtime->create_equal_partition(ctx, parent, color_space, granularity,
                                    color, allocable);
#endif
  return CObjectWrapper::wrap(ip);
}

// Shim for Legion Dependent Partition API

#if USE_LEGION_PARTAPI_SHIM
class PartitionByUnionShim {
public:
  static TaskID register_task();
  static IndexPartition launch(HighLevelRuntime *runtime,
                               Context ctx,
                               IndexSpace parent,
                               IndexPartition handle1,
                               IndexPartition handle2,
                               PartitionKind part_kind = COMPUTE_KIND,
                               int color = AUTO_GENERATE_ID,
                               bool allocable = false);
  static IndexPartition task(const Task *task,
                             const std::vector<PhysicalRegion> &regions,
                             Context ctx, HighLevelRuntime *runtime);
private:
  static const TaskID task_id = 555831; // a "unique" number
  struct Args {
    IndexSpace parent;
    IndexPartition handle1;
    IndexPartition handle2;
    PartitionKind part_kind;
    int color;
    bool allocable;
  };
};

Processor::TaskFuncID
PartitionByUnionShim::register_task()
{
  return HighLevelRuntime::register_legion_task<IndexPartition, task>(
    task_id, Processor::LOC_PROC, true, false,
    AUTO_GENERATE_ID, TaskConfigOptions(),
    "PartitionByUnionShim::task");
}

IndexPartition
PartitionByUnionShim::launch(HighLevelRuntime *runtime,
                                  Context ctx,
                                  IndexSpace parent,
                                  IndexPartition handle1,
                                  IndexPartition handle2,
                                  PartitionKind part_kind,
                                  int color,
                                  bool allocable)
{
  Args args;
  args.parent = parent;
  args.handle1 = handle1;
  args.handle2 = handle2;
  args.part_kind = part_kind;
  args.color = color;
  args.allocable = allocable;
  TaskArgument targs(&args, sizeof(args));
  TaskLauncher task(task_id, targs);
  Future f = runtime->execute_task(ctx, task);
  return f.get_result<IndexPartition>();
}

IndexPartition
PartitionByUnionShim::task(const Task *task,
                                const std::vector<PhysicalRegion> &regions,
                                Context ctx, HighLevelRuntime *runtime)
{
  assert(task->arglen == sizeof(Args));
  Args &args = *(Args *)task->args;

  Domain color_space = runtime->get_index_partition_color_space(ctx, args.handle1);
  PointColoring coloring;
  for(Domain::DomainPointIterator c(color_space); c; c++) {
    IndexSpace lhs = runtime->get_index_subspace(ctx, args.handle1, c.p);
    IndexSpace rhs = runtime->get_index_subspace(ctx, args.handle2, c.p);

    for (IndexIterator it(runtime, ctx, lhs); it.has_next();) {
      size_t count = 0;
      ptr_t start = it.next_span(count);
      coloring[c.p].ranges.insert(
        std::pair<ptr_t, ptr_t>(start, start.value+count-1));
    }

    for (IndexIterator it(runtime, ctx, rhs); it.has_next();) {
      size_t count = 0;
      ptr_t start = it.next_span(count);
      coloring[c.p].ranges.insert(
        std::pair<ptr_t, ptr_t>(start, start.value+count-1));
    }
  }

  IndexPartition ip =
    runtime->create_index_partition(
      ctx, args.parent, color_space, coloring,
      args.part_kind, args.color, args.allocable);
  return ip;
}

static TaskID force_PartitionByUnionShim_static_initialize =
  PartitionByUnionShim::register_task();
#endif

legion_index_partition_t
legion_index_partition_create_by_union(
  legion_runtime_t runtime_,
  legion_context_t ctx_,
  legion_index_space_t parent_,
  legion_index_partition_t handle1_,
  legion_index_partition_t handle2_,
  legion_partition_kind_t part_kind /* = COMPUTE_KIND */,
  int color /* = AUTO_GENERATE_ID */,
  bool allocable /* = false */)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexSpace parent = CObjectWrapper::unwrap(parent_);
  IndexPartition handle1 = CObjectWrapper::unwrap(handle1_);
  IndexPartition handle2 = CObjectWrapper::unwrap(handle2_);

  IndexPartition ip =
#if USE_LEGION_PARTAPI_SHIM
    PartitionByUnionShim::launch(runtime, ctx, parent, handle1, handle2,
                                      part_kind, color, allocable);
#else
    runtime->create_partition_by_union(ctx, parent, handle1, handle2,
                                            part_kind, color, allocable);
#endif
  return CObjectWrapper::wrap(ip);
}

// Shim for Legion Dependent Partition API

#if USE_LEGION_PARTAPI_SHIM
class PartitionByIntersectionShim {
public:
  static TaskID register_task();
  static IndexPartition launch(HighLevelRuntime *runtime,
                               Context ctx,
                               IndexSpace parent,
                               IndexPartition handle1,
                               IndexPartition handle2,
                               PartitionKind part_kind = COMPUTE_KIND,
                               int color = AUTO_GENERATE_ID,
                               bool allocable = false);
  static IndexPartition task(const Task *task,
                             const std::vector<PhysicalRegion> &regions,
                             Context ctx, HighLevelRuntime *runtime);
private:
  static const TaskID task_id = 582347; // a "unique" number
  struct Args {
    IndexSpace parent;
    IndexPartition handle1;
    IndexPartition handle2;
    PartitionKind part_kind;
    int color;
    bool allocable;
  };
};

Processor::TaskFuncID
PartitionByIntersectionShim::register_task()
{
  return HighLevelRuntime::register_legion_task<IndexPartition, task>(
    task_id, Processor::LOC_PROC, true, false,
    AUTO_GENERATE_ID, TaskConfigOptions(),
    "PartitionByIntersectionShim::task");
}

IndexPartition
PartitionByIntersectionShim::launch(HighLevelRuntime *runtime,
                                  Context ctx,
                                  IndexSpace parent,
                                  IndexPartition handle1,
                                  IndexPartition handle2,
                                  PartitionKind part_kind,
                                  int color,
                                  bool allocable)
{
  Args args;
  args.parent = parent;
  args.handle1 = handle1;
  args.handle2 = handle2;
  args.part_kind = part_kind;
  args.color = color;
  args.allocable = allocable;
  TaskArgument targs(&args, sizeof(args));
  TaskLauncher task(task_id, targs);
  Future f = runtime->execute_task(ctx, task);
  return f.get_result<IndexPartition>();
}

IndexPartition
PartitionByIntersectionShim::task(const Task *task,
                                const std::vector<PhysicalRegion> &regions,
                                Context ctx, HighLevelRuntime *runtime)
{
  assert(task->arglen == sizeof(Args));
  Args &args = *(Args *)task->args;

  Domain color_space = runtime->get_index_partition_color_space(ctx, args.handle1);
  PointColoring coloring;
  for(Domain::DomainPointIterator c(color_space); c; c++) {
    IndexSpace lhs = runtime->get_index_subspace(ctx, args.handle1, c.p);
    IndexSpace rhs = runtime->get_index_subspace(ctx, args.handle2, c.p);

    std::set<ptr_t> rhs_points;
    for (IndexIterator it(runtime, ctx, rhs); it.has_next();) {
      size_t count = 0;
      ptr_t start = it.next_span(count);
      for (ptr_t p(start); p.value - start.value < count; p++) {
        rhs_points.insert(p);
      }
    }

    for (IndexIterator it(runtime, ctx, lhs); it.has_next();) {
      size_t count = 0;
      ptr_t start = it.next_span(count);
      for (ptr_t p(start); p.value - start.value < count; p++) {
        if (rhs_points.count(p)) {
          coloring[c.p].points.insert(p);
        }
      }
    }
  }

  IndexPartition ip =
    runtime->create_index_partition(
      ctx, args.parent, color_space, coloring,
      args.part_kind, args.color, args.allocable);
  return ip;
}

static TaskID force_PartitionByIntersectionShim_static_initialize =
  PartitionByIntersectionShim::register_task();
#endif

legion_index_partition_t
legion_index_partition_create_by_intersection(
  legion_runtime_t runtime_,
  legion_context_t ctx_,
  legion_index_space_t parent_,
  legion_index_partition_t handle1_,
  legion_index_partition_t handle2_,
  legion_partition_kind_t part_kind /* = COMPUTE_KIND */,
  int color /* = AUTO_GENERATE_ID */,
  bool allocable /* = false */)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexSpace parent = CObjectWrapper::unwrap(parent_);
  IndexPartition handle1 = CObjectWrapper::unwrap(handle1_);
  IndexPartition handle2 = CObjectWrapper::unwrap(handle2_);

  IndexPartition ip =
#if USE_LEGION_PARTAPI_SHIM
    PartitionByIntersectionShim::launch(runtime, ctx, parent, handle1, handle2,
                                        part_kind, color, allocable);
#else
    runtime->create_partition_by_intersection(ctx, parent, handle1, handle2,
                                              part_kind, color, allocable);
#endif
  return CObjectWrapper::wrap(ip);
}

// Shim for Legion Dependent Partition API

#if USE_LEGION_PARTAPI_SHIM
class PartitionByDifferenceShim {
public:
  static TaskID register_task();
  static IndexPartition launch(HighLevelRuntime *runtime,
                               Context ctx,
                               IndexSpace parent,
                               IndexPartition handle1,
                               IndexPartition handle2,
                               PartitionKind part_kind = COMPUTE_KIND,
                               int color = AUTO_GENERATE_ID,
                               bool allocable = false);
  static IndexPartition task(const Task *task,
                             const std::vector<PhysicalRegion> &regions,
                             Context ctx, HighLevelRuntime *runtime);
private:
  static const TaskID task_id = 577802; // a "unique" number
  struct Args {
    IndexSpace parent;
    IndexPartition handle1;
    IndexPartition handle2;
    PartitionKind part_kind;
    int color;
    bool allocable;
  };
};

Processor::TaskFuncID
PartitionByDifferenceShim::register_task()
{
  return HighLevelRuntime::register_legion_task<IndexPartition, task>(
    task_id, Processor::LOC_PROC, true, false,
    AUTO_GENERATE_ID, TaskConfigOptions(),
    "PartitionByDifferenceShim::task");
}

IndexPartition
PartitionByDifferenceShim::launch(HighLevelRuntime *runtime,
                                  Context ctx,
                                  IndexSpace parent,
                                  IndexPartition handle1,
                                  IndexPartition handle2,
                                  PartitionKind part_kind,
                                  int color,
                                  bool allocable)
{
  Args args;
  args.parent = parent;
  args.handle1 = handle1;
  args.handle2 = handle2;
  args.part_kind = part_kind;
  args.color = color;
  args.allocable = allocable;
  TaskArgument targs(&args, sizeof(args));
  TaskLauncher task(task_id, targs);
  Future f = runtime->execute_task(ctx, task);
  return f.get_result<IndexPartition>();
}

IndexPartition
PartitionByDifferenceShim::task(const Task *task,
                                const std::vector<PhysicalRegion> &regions,
                                Context ctx, HighLevelRuntime *runtime)
{
  assert(task->arglen == sizeof(Args));
  Args &args = *(Args *)task->args;

  Domain color_space = runtime->get_index_partition_color_space(ctx, args.handle1);
  PointColoring coloring;
  for(Domain::DomainPointIterator c(color_space); c; c++) {
    IndexSpace lhs = runtime->get_index_subspace(ctx, args.handle1, c.p);
    IndexSpace rhs = runtime->get_index_subspace(ctx, args.handle2, c.p);

    std::set<ptr_t> rhs_points;
    for (IndexIterator it(runtime, ctx, rhs); it.has_next();) {
      size_t count = 0;
      ptr_t start = it.next_span(count);
      for (ptr_t p(start); p.value - start.value < count; p++) {
        rhs_points.insert(p);
      }
    }

    for (IndexIterator it(runtime, ctx, lhs); it.has_next();) {
      size_t count = 0;
      ptr_t start = it.next_span(count);
      for (ptr_t p(start); p.value - start.value < count; p++) {
        if (!rhs_points.count(p)) {
          coloring[c.p].points.insert(p);
        }
      }
    }
  }

  IndexPartition ip =
    runtime->create_index_partition(
      ctx, args.parent, color_space, coloring,
      args.part_kind, args.color, args.allocable);
  return ip;
}

static TaskID force_PartitionByDifferenceShim_static_initialize =
  PartitionByDifferenceShim::register_task();
#endif

legion_index_partition_t
legion_index_partition_create_by_difference(
  legion_runtime_t runtime_,
  legion_context_t ctx_,
  legion_index_space_t parent_,
  legion_index_partition_t handle1_,
  legion_index_partition_t handle2_,
  legion_partition_kind_t part_kind /* = COMPUTE_KIND */,
  int color /* = AUTO_GENERATE_ID */,
  bool allocable /* = false */)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexSpace parent = CObjectWrapper::unwrap(parent_);
  IndexPartition handle1 = CObjectWrapper::unwrap(handle1_);
  IndexPartition handle2 = CObjectWrapper::unwrap(handle2_);

  IndexPartition ip =
#if USE_LEGION_PARTAPI_SHIM
    PartitionByDifferenceShim::launch(runtime, ctx, parent, handle1, handle2,
                                      part_kind, color, allocable);
#else
    runtime->create_partition_by_difference(ctx, parent, handle1, handle2,
                                            part_kind, color, allocable);
#endif
  return CObjectWrapper::wrap(ip);
}

// Shim for Legion Dependent Partition API

#if USE_LEGION_PARTAPI_SHIM
class PartitionByFieldShim {
public:
  static TaskID register_task();
  static IndexPartition launch(HighLevelRuntime *runtime,
                               Context ctx,
                               LogicalRegion handle,
                               LogicalRegion parent,
                               FieldID fid,
                               const Domain &color_space,
                               int color = AUTO_GENERATE_ID,
                               bool allocable = false);
  static IndexPartition task(const Task *task,
                             const std::vector<PhysicalRegion> &regions,
                             Context ctx, HighLevelRuntime *runtime);
private:
  static const TaskID task_id = 539418; // a "unique" number
  struct Args {
    LogicalRegion handle;
    LogicalRegion parent;
    FieldID fid;
    Domain color_space;
    int color;
    bool allocable;
  };
};

Processor::TaskFuncID
PartitionByFieldShim::register_task()
{
  return HighLevelRuntime::register_legion_task<IndexPartition, task>(
    task_id, Processor::LOC_PROC, true, false,
    AUTO_GENERATE_ID, TaskConfigOptions(),
    "PartitionByFieldShim::task");
}

IndexPartition
PartitionByFieldShim::launch(HighLevelRuntime *runtime,
                             Context ctx,
                             LogicalRegion handle,
                             LogicalRegion parent,
                             FieldID fid,
                             const Domain &color_space,
                             int color,
                             bool allocable)
{
  Args args;
  args.handle = handle;
  args.parent = parent;
  args.fid = fid;
  args.color_space = color_space;
  args.color = color;
  args.allocable = allocable;
  TaskArgument targs(&args, sizeof(args));
  TaskLauncher task(task_id, targs);
  task.add_region_requirement(
    RegionRequirement(handle, READ_ONLY, EXCLUSIVE, parent)
    .add_field(fid));
  Future f = runtime->execute_task(ctx, task);
  return f.get_result<IndexPartition>();
}

IndexPartition
PartitionByFieldShim::task(const Task *task,
                           const std::vector<PhysicalRegion> &regions,
                           Context ctx, HighLevelRuntime *runtime)
{
  assert(task->arglen == sizeof(Args));
  Args &args = *(Args *)task->args;

  PointColoring coloring;
  assert(args.color_space.get_dim() == 1);

  Accessor::RegionAccessor<SOA, Color> accessor =
    regions[0].get_field_accessor(args.fid).typeify<Color>().convert<SOA>();
  for (IndexIterator it(runtime, ctx, regions[0].get_logical_region());
       it.has_next();) {
    size_t count = 0;
    ptr_t start = it.next_span(count);
    for (ptr_t p(start); p.value - start.value < count; p++) {
      Color c = accessor.read(p);
      coloring[DomainPoint::from_point<1>(Point<1>(c))].points.insert(p);
    }
  }

  IndexPartition ip =
    runtime->create_index_partition(
      ctx, args.handle.get_index_space(), args.color_space, coloring,
      DISJOINT_KIND, args.color, args.allocable);
  return ip;
}

static TaskID force_PartitionByFieldShim_static_initialize =
  PartitionByFieldShim::register_task();
#endif

legion_index_partition_t
legion_index_partition_create_by_field(legion_runtime_t runtime_,
                                       legion_context_t ctx_,
                                       legion_logical_region_t handle_,
                                       legion_logical_region_t parent_,
                                       legion_field_id_t fid,
                                       legion_domain_t color_space_,
                                       int color /* = AUTO_GENERATE_ID */,
                                       bool allocable /* = false */)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  LogicalRegion handle = CObjectWrapper::unwrap(handle_);
  LogicalRegion parent = CObjectWrapper::unwrap(parent_);
  Domain color_space = CObjectWrapper::unwrap(color_space_);

  IndexPartition ip =
#if USE_LEGION_PARTAPI_SHIM
    PartitionByFieldShim::launch(runtime, ctx, handle, parent, fid, color_space,
                                 color, allocable);
#else
    runtime->create_partition_by_field(ctx, handle, parent, fid, color_space,
                                       color, allocable);
#endif

  return CObjectWrapper::wrap(ip);
}

// Shim for Legion Dependent Partition API

#if USE_LEGION_PARTAPI_SHIM
class PartitionByImageShim {
public:
  static TaskID register_task();
  static IndexPartition launch(HighLevelRuntime *runtime,
                               Context ctx,
                               IndexSpace handle,
                               LogicalPartition projection,
                               LogicalRegion parent,
                               FieldID fid,
                               const Domain &color_space,
                               PartitionKind part_kind = COMPUTE_KIND,
                               int color = AUTO_GENERATE_ID,
                               bool allocable = false);
  static IndexPartition task(const Task *task,
                             const std::vector<PhysicalRegion> &regions,
                             Context ctx, HighLevelRuntime *runtime);
private:
  static const TaskID task_id = 590467; // a "unique" number
  struct Args {
    IndexSpace handle;
    LogicalPartition projection;
    LogicalRegion parent;
    FieldID fid;
    Domain color_space;
    PartitionKind part_kind;
    int color;
    bool allocable;
  };
};

Processor::TaskFuncID
PartitionByImageShim::register_task()
{
  return HighLevelRuntime::register_legion_task<IndexPartition, task>(
    task_id, Processor::LOC_PROC, true, false,
    AUTO_GENERATE_ID, TaskConfigOptions(),
    "PartitionByImageShim::task");
}

IndexPartition
PartitionByImageShim::launch(HighLevelRuntime *runtime,
                             Context ctx,
                             IndexSpace handle,
                             LogicalPartition projection,
                             LogicalRegion parent,
                             FieldID fid,
                             const Domain &color_space,
                             PartitionKind part_kind,
                             int color,
                             bool allocable)
{
  Args args;
  args.handle = handle;
  args.projection = projection;
  args.parent = parent;
  args.fid = fid;
  args.color_space = color_space;
  args.part_kind = part_kind;
  args.color = color;
  args.allocable = allocable;
  TaskArgument targs(&args, sizeof(args));
  TaskLauncher task(task_id, targs);
  task.add_region_requirement(
    RegionRequirement(parent, READ_ONLY, EXCLUSIVE, parent)
    .add_field(fid));
  Future f = runtime->execute_task(ctx, task);
  return f.get_result<IndexPartition>();
}

IndexPartition
PartitionByImageShim::task(const Task *task,
                           const std::vector<PhysicalRegion> &regions,
                           Context ctx, HighLevelRuntime *runtime)
{
  assert(task->arglen == sizeof(Args));
  Args &args = *(Args *)task->args;

  PointColoring coloring;
  Accessor::RegionAccessor<SOA, ptr_t> accessor =
    regions[0].get_field_accessor(args.fid).typeify<ptr_t>().convert<SOA>();
  for(Domain::DomainPointIterator c(args.color_space); c; c++) {
    LogicalRegion r =
      runtime->get_logical_subregion_by_color(ctx, args.projection, c.p);
    for (IndexIterator it(runtime, ctx, r); it.has_next();) {
      size_t count = 0;
      ptr_t start = it.next_span(count);
      for (ptr_t p(start); p.value - start.value < count; p++) {
        coloring[upgrade_point(c.p)].points.insert(accessor.read(p));
      }
    }
  }

  IndexPartition ip =
    runtime->create_index_partition(
      ctx, args.handle, args.color_space, coloring,
      args.part_kind, args.color, args.allocable);
  return ip;
}

static TaskID force_PartitionByImageShim_static_initialize =
  PartitionByImageShim::register_task();
#endif

legion_index_partition_t
legion_index_partition_create_by_image(
  legion_runtime_t runtime_,
  legion_context_t ctx_,
  legion_index_space_t handle_,
  legion_logical_partition_t projection_,
  legion_logical_region_t parent_,
  legion_field_id_t fid,
  legion_domain_t color_space_,
  legion_partition_kind_t part_kind /* = COMPUTE_KIND */,
  int color /* = AUTO_GENERATE_ID */,
  bool allocable /* = false */)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexSpace handle = CObjectWrapper::unwrap(handle_);
  LogicalPartition projection = CObjectWrapper::unwrap(projection_);
  LogicalRegion parent = CObjectWrapper::unwrap(parent_);
  Domain color_space = CObjectWrapper::unwrap(color_space_);

  IndexPartition ip =
#if USE_LEGION_PARTAPI_SHIM
    PartitionByImageShim::launch(
      runtime, ctx, handle, projection, parent, fid, color_space, part_kind,
      color, allocable);
#else
    runtime->create_partition_by_image(
      ctx, handle, projection, parent, fid, color_space, part_kind, color,
      allocable);
#endif

  return CObjectWrapper::wrap(ip);
}

// Shim for Legion Dependent Partition API

#if USE_LEGION_PARTAPI_SHIM
class PartitionByPreimageShim {
public:
  static TaskID register_task();
  static IndexPartition launch(HighLevelRuntime *runtime,
                               Context ctx,
                               IndexPartition projection,
                               LogicalRegion handle,
                               LogicalRegion parent,
                               FieldID fid,
                               const Domain &color_space,
                               PartitionKind part_kind = COMPUTE_KIND,
                               int color = AUTO_GENERATE_ID,
                               bool allocable = false);
  static IndexPartition task(const Task *task,
                             const std::vector<PhysicalRegion> &regions,
                             Context ctx, HighLevelRuntime *runtime);
private:
  static const TaskID task_id = 509532; // a "unique" number
  struct Args {
    IndexPartition projection;
    LogicalRegion handle;
    LogicalRegion parent;
    FieldID fid;
    Domain color_space;
    PartitionKind part_kind;
    int color;
    bool allocable;
  };
};

Processor::TaskFuncID
PartitionByPreimageShim::register_task()
{
  return HighLevelRuntime::register_legion_task<IndexPartition, task>(
    task_id, Processor::LOC_PROC, true, false,
    AUTO_GENERATE_ID, TaskConfigOptions(),
    "PartitionByPreimageShim::task");
}

IndexPartition
PartitionByPreimageShim::launch(HighLevelRuntime *runtime,
                                Context ctx,
                                IndexPartition projection,
                                LogicalRegion handle,
                                LogicalRegion parent,
                                FieldID fid,
                                const Domain &color_space,
                                PartitionKind part_kind,
                                int color,
                                bool allocable)
{
  Args args;
  args.handle = handle;
  args.projection = projection;
  args.parent = parent;
  args.fid = fid;
  args.color_space = color_space;
  args.part_kind = part_kind;
  args.color = color;
  args.allocable = allocable;
  TaskArgument targs(&args, sizeof(args));
  TaskLauncher task(task_id, targs);
  task.add_region_requirement(
    RegionRequirement(parent, READ_ONLY, EXCLUSIVE, parent)
    .add_field(fid));
  Future f = runtime->execute_task(ctx, task);
  return f.get_result<IndexPartition>();
}

IndexPartition
PartitionByPreimageShim::task(const Task *task,
                              const std::vector<PhysicalRegion> &regions,
                              Context ctx, HighLevelRuntime *runtime)
{
  assert(task->arglen == sizeof(Args));
  Args &args = *(Args *)task->args;

  PointColoring coloring;
  Accessor::RegionAccessor<SOA, ptr_t> accessor =
    regions[0].get_field_accessor(args.fid).typeify<ptr_t>().convert<SOA>();
  for(Domain::DomainPointIterator c(args.color_space); c; c++) {
    IndexSpace target = runtime->get_index_subspace(ctx, args.projection, c.p);
    std::set<ptr_t> points;
    for (IndexIterator it(runtime, ctx, target); it.has_next();) {
      size_t count = 0;
      ptr_t start = it.next_span(count);
      for (ptr_t p(start); p.value - start.value < count; p++) {
        points.insert(p);
      }
    }

    for (IndexIterator it(runtime, ctx, args.handle); it.has_next();) {
      size_t count = 0;
      ptr_t start = it.next_span(count);
      for (ptr_t p(start); p.value - start.value < count; p++) {
        if (points.count(accessor.read(p))) {
          coloring[upgrade_point(c.p)].points.insert(p);
        }
      }
    }
  }

  IndexPartition ip =
    runtime->create_index_partition(
      ctx, args.handle.get_index_space(), args.color_space, coloring,
      args.part_kind, args.color, args.allocable);
  return ip;
}

static TaskID force_PartitionByPreimageShim_static_initialize =
  PartitionByPreimageShim::register_task();
#endif

legion_index_partition_t
legion_index_partition_create_by_preimage(
  legion_runtime_t runtime_,
  legion_context_t ctx_,
  legion_index_partition_t projection_,
  legion_logical_region_t handle_,
  legion_logical_region_t parent_,
  legion_field_id_t fid,
  legion_domain_t color_space_,
  legion_partition_kind_t part_kind /* = COMPUTE_KIND */,
  int color /* = AUTO_GENERATE_ID */,
  bool allocable /* = false */)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexPartition projection = CObjectWrapper::unwrap(projection_);
  LogicalRegion handle = CObjectWrapper::unwrap(handle_);
  LogicalRegion parent = CObjectWrapper::unwrap(parent_);
  Domain color_space = CObjectWrapper::unwrap(color_space_);

  IndexPartition ip =
#if USE_LEGION_PARTAPI_SHIM
    PartitionByPreimageShim::launch(
      runtime, ctx, projection, handle, parent, fid, color_space, part_kind,
      color, allocable);
#else
    runtime->create_partition_by_preimage(
      ctx, projection, handle, parent, fid, color_space, part_kind, color,
      allocable);
#endif

  return CObjectWrapper::wrap(ip);
}

legion_index_space_t
legion_index_partition_get_index_subspace(legion_runtime_t runtime_,
                                          legion_context_t ctx_,
                                          legion_index_partition_t handle_,
                                          legion_color_t color)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexPartition handle = CObjectWrapper::unwrap(handle_);

  IndexSpace is = runtime->get_index_subspace(ctx, handle, color);

  return CObjectWrapper::wrap(is);
}

legion_domain_t
legion_index_partition_get_color_space(legion_runtime_t runtime_,
                                       legion_context_t ctx_,
                                       legion_index_partition_t handle_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexPartition handle = CObjectWrapper::unwrap(handle_);

  Domain d = runtime->get_index_partition_color_space(ctx, handle);

  return CObjectWrapper::wrap(d);
}

legion_index_space_t
legion_index_partition_get_parent_index_space(legion_runtime_t runtime_,
                                              legion_context_t ctx_,
                                              legion_index_partition_t handle_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexPartition handle = CObjectWrapper::unwrap(handle_);

  IndexSpace is = runtime->get_parent_index_space(ctx, handle);

  return CObjectWrapper::wrap(is);
}

void
legion_index_partition_destroy(legion_runtime_t runtime_,
                               legion_context_t ctx_,
                               legion_index_partition_t handle_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexPartition handle = CObjectWrapper::unwrap(handle_);

  runtime->destroy_index_partition(ctx, handle);
}

void
legion_index_partition_attach_name(legion_runtime_t runtime_,
                                   legion_index_partition_t handle_,
                                   const char *name)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  IndexPartition handle = CObjectWrapper::unwrap(handle_);

  runtime->attach_name(handle, name);
}

void
legion_index_partition_retrieve_name(legion_runtime_t runtime_,
                                     legion_index_partition_t handle_,
                                     const char **result)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  IndexPartition handle = CObjectWrapper::unwrap(handle_);

  runtime->retrieve_name(handle, *result);
}

// -------------------------------------------------------
// Field Space Operations
// -------------------------------------------------------

legion_field_space_t
legion_field_space_create(legion_runtime_t runtime_,
                          legion_context_t ctx_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);

  FieldSpace fs = runtime->create_field_space(ctx);
  return CObjectWrapper::wrap(fs);
}

void
legion_field_space_destroy(legion_runtime_t runtime_,
                           legion_context_t ctx_,
                           legion_field_space_t handle_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  FieldSpace handle = CObjectWrapper::unwrap(handle_);

  runtime->destroy_field_space(ctx, handle);
}

void
legion_field_space_attach_name(legion_runtime_t runtime_,
                               legion_field_space_t handle_,
                               const char *name)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  FieldSpace handle = CObjectWrapper::unwrap(handle_);

  runtime->attach_name(handle, name);
}

void
legion_field_space_retrieve_name(legion_runtime_t runtime_,
                                 legion_field_space_t handle_,
                                 const char **result)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  FieldSpace handle = CObjectWrapper::unwrap(handle_);

  runtime->retrieve_name(handle, *result);
}

void
legion_field_id_attach_name(legion_runtime_t runtime_,
                            legion_field_space_t handle_,
                            legion_field_id_t id,
                            const char *name)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  FieldSpace handle = CObjectWrapper::unwrap(handle_);

  runtime->attach_name(handle, id, name);
}

void
legion_field_id_retrieve_name(legion_runtime_t runtime_,
                              legion_field_space_t handle_,
                              legion_field_id_t id,
                              const char **result)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  FieldSpace handle = CObjectWrapper::unwrap(handle_);

  runtime->retrieve_name(handle, id, *result);
}

// -------------------------------------------------------
// Logical Region Operations
// -------------------------------------------------------

legion_logical_region_t
legion_logical_region_create(legion_runtime_t runtime_,
                             legion_context_t ctx_,
                             legion_index_space_t index_,
                             legion_field_space_t fields_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexSpace index = CObjectWrapper::unwrap(index_);
  FieldSpace fields = CObjectWrapper::unwrap(fields_);

  LogicalRegion r = runtime->create_logical_region(ctx, index, fields);
  return CObjectWrapper::wrap(r);
}

void
legion_logical_region_destroy(legion_runtime_t runtime_,
                              legion_context_t ctx_,
                              legion_logical_region_t handle_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  LogicalRegion handle = CObjectWrapper::unwrap(handle_);

  runtime->destroy_logical_region(ctx, handle);
}

legion_color_t
legion_logical_region_get_color(legion_runtime_t runtime_,
                                legion_context_t ctx_,
                                legion_logical_region_t handle_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  LogicalRegion handle = CObjectWrapper::unwrap(handle_);

  return runtime->get_logical_region_color(ctx, handle);
}

bool
legion_logical_region_has_parent_logical_partition(
  legion_runtime_t runtime_,
  legion_context_t ctx_,
  legion_logical_region_t handle_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  LogicalRegion handle = CObjectWrapper::unwrap(handle_);

  return runtime->has_parent_logical_partition(ctx, handle);
}

legion_logical_partition_t
legion_logical_region_get_parent_logical_partition(
  legion_runtime_t runtime_,
  legion_context_t ctx_,
  legion_logical_region_t handle_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  LogicalRegion handle = CObjectWrapper::unwrap(handle_);

  LogicalPartition p = runtime->get_parent_logical_partition(ctx, handle);
  return CObjectWrapper::wrap(p);
}

void
legion_logical_region_attach_name(legion_runtime_t runtime_,
                                  legion_logical_region_t handle_,
                                  const char *name)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  LogicalRegion handle = CObjectWrapper::unwrap(handle_);

  runtime->attach_name(handle, name);
}

void
legion_logical_region_retrieve_name(legion_runtime_t runtime_,
                                    legion_logical_region_t handle_,
                                    const char **result)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  LogicalRegion handle = CObjectWrapper::unwrap(handle_);

  runtime->retrieve_name(handle, *result);
}

// -----------------------------------------------------------------------
// Logical Region Tree Traversal Operations
// -----------------------------------------------------------------------

legion_logical_partition_t
legion_logical_partition_create(legion_runtime_t runtime_,
                                legion_context_t ctx_,
                                legion_logical_region_t parent_,
                                legion_index_partition_t handle_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  LogicalRegion parent = CObjectWrapper::unwrap(parent_);
  IndexPartition handle = CObjectWrapper::unwrap(handle_);

  LogicalPartition r = runtime->get_logical_partition(ctx, parent, handle);
  return CObjectWrapper::wrap(r);
}

legion_logical_partition_t
legion_logical_partition_create_by_tree(legion_runtime_t runtime_,
                                        legion_context_t ctx_,
                                        legion_index_partition_t handle_,
                                        legion_field_space_t fspace_,
                                        legion_region_tree_id_t tid)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  FieldSpace fspace = CObjectWrapper::unwrap(fspace_);
  IndexPartition handle = CObjectWrapper::unwrap(handle_);

  LogicalPartition r =
    runtime->get_logical_partition_by_tree(ctx, handle, fspace, tid);
  return CObjectWrapper::wrap(r);
}

void
legion_logical_partition_destroy(legion_runtime_t runtime_,
                                 legion_context_t ctx_,
                                 legion_logical_partition_t handle_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  LogicalPartition handle = CObjectWrapper::unwrap(handle_);

  runtime->destroy_logical_partition(ctx, handle);
}

legion_logical_region_t
legion_logical_partition_get_logical_subregion(
  legion_runtime_t runtime_,
  legion_context_t ctx_,
  legion_logical_partition_t parent_,
  legion_index_space_t handle_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  LogicalPartition parent = CObjectWrapper::unwrap(parent_);
  IndexSpace handle = CObjectWrapper::unwrap(handle_);

  LogicalRegion r = runtime->get_logical_subregion(ctx, parent, handle);
  return CObjectWrapper::wrap(r);
}

legion_logical_region_t
legion_logical_partition_get_logical_subregion_by_color(
  legion_runtime_t runtime_,
  legion_context_t ctx_,
  legion_logical_partition_t parent_,
  legion_color_t c)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  LogicalPartition parent = CObjectWrapper::unwrap(parent_);

  LogicalRegion r = runtime->get_logical_subregion_by_color(ctx, parent, c);
  return CObjectWrapper::wrap(r);
}

legion_logical_region_t
legion_logical_partition_get_parent_logical_region(
  legion_runtime_t runtime_,
  legion_context_t ctx_,
  legion_logical_partition_t handle_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  LogicalPartition handle = CObjectWrapper::unwrap(handle_);

  LogicalRegion r = runtime->get_parent_logical_region(ctx, handle);
  return CObjectWrapper::wrap(r);
}

void
legion_logical_partition_attach_name(legion_runtime_t runtime_,
                                     legion_logical_partition_t handle_,
                                     const char *name)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  LogicalPartition handle = CObjectWrapper::unwrap(handle_);

  runtime->attach_name(handle, name);
}

void
legion_logical_partition_retrieve_name(legion_runtime_t runtime_,
                                       legion_logical_partition_t handle_,
                                       const char **result)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  LogicalPartition handle = CObjectWrapper::unwrap(handle_);

  runtime->retrieve_name(handle, *result);
}

// -----------------------------------------------------------------------
// Region Requirement Operations
// -----------------------------------------------------------------------

legion_logical_region_t
legion_region_requirement_get_region(legion_region_requirement_t req_)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  return CObjectWrapper::wrap(req->region);
}

legion_logical_region_t
legion_region_requirement_get_parent(legion_region_requirement_t req_)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  return CObjectWrapper::wrap(req->parent);
}

legion_logical_partition_t
legion_region_requirement_get_partition(legion_region_requirement_t req_)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  return CObjectWrapper::wrap(req->partition);
}

unsigned
legion_region_requirement_get_privilege_fields_size(
    legion_region_requirement_t req_)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  return req->privilege_fields.size();
}

template<typename DST, typename SRC>
static void copy_n(DST dst, SRC src, size_t n)
{
  for(size_t i = 0; i < n; ++i)
    *dst++ = *src++;
}

void
legion_region_requirement_get_privilege_fields(
    legion_region_requirement_t req_,
    legion_field_id_t* fields,
    unsigned fields_size)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  copy_n(fields, req->privilege_fields.begin(),
         std::min(req->privilege_fields.size(),
                  static_cast<size_t>(fields_size)));
}


legion_field_id_t
legion_region_requirement_get_privilege_field(
    legion_region_requirement_t req_,
    unsigned idx)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);
  assert(idx >= 0 && idx < req->instance_fields.size());

  std::set<FieldID>::iterator itr = req->privilege_fields.begin();
  for (unsigned i = 0; i < idx; ++i, ++itr);
  return *itr;
}

unsigned
legion_region_requirement_get_instance_fields_size(
    legion_region_requirement_t req_)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  return req->instance_fields.size();
}

void
legion_region_requirement_get_instance_fields(
    legion_region_requirement_t req_,
    legion_field_id_t* fields,
    unsigned fields_size)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  copy_n(fields, req->instance_fields.begin(),
         std::min(req->instance_fields.size(),
                  static_cast<size_t>(fields_size)));
}

legion_field_id_t
legion_region_requirement_get_instance_field(
    legion_region_requirement_t req_,
    unsigned idx)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  assert(idx >= 0 && idx < req->instance_fields.size());
  return req->instance_fields[idx];
}

legion_privilege_mode_t
legion_region_requirement_get_privilege(legion_region_requirement_t req_)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  return req->privilege;
}

legion_coherence_property_t
legion_region_requirement_get_prop(legion_region_requirement_t req_)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  return req->prop;
}

legion_reduction_op_id_t
legion_region_requirement_get_redop(legion_region_requirement_t req_)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  return req->redop;
}

legion_mapping_tag_id_t
legion_region_requirement_get_tag(legion_region_requirement_t req_)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  return req->tag;
}

legion_handle_type_t
legion_region_requirement_get_handle_type(legion_region_requirement_t req_)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  return req->handle_type;
}

legion_projection_id_t
legion_region_requirement_get_projection(legion_region_requirement_t req_)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  return req->projection;
}

bool
legion_region_requirement_get_virtual_map(legion_region_requirement_t req_)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  return req->virtual_map;
}

void
legion_region_requirement_set_virtual_map(
  legion_region_requirement_t req_,
  bool value)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  req->virtual_map = value;
}

bool
legion_region_requirement_get_early_map(legion_region_requirement_t req_)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  return req->early_map;
}

void
legion_region_requirement_set_early_map(
  legion_region_requirement_t req_,
  bool value)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  req->early_map = value;
}

bool
legion_region_requirement_get_enable_WAR_optimization(
  legion_region_requirement_t req_)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  return req->enable_WAR_optimization;
}

void
legion_region_requirement_set_enable_WAR_optimization(
  legion_region_requirement_t req_,
  bool value)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  req->enable_WAR_optimization = value;
}

bool
legion_region_requirement_get_reduction_list(
  legion_region_requirement_t req_)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  return req->reduction_list;
}

void
legion_region_requirement_set_reduction_list(
  legion_region_requirement_t req_,
  bool value)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  req->reduction_list = value;
}

unsigned
legion_region_requirement_get_make_persistent(
  legion_region_requirement_t req_)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  return req->make_persistent;
}

void
legion_region_requirement_set_make_persistent(
  legion_region_requirement_t req_,
  unsigned value)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  req->make_persistent = value;
}

unsigned
legion_region_requirement_get_blocking_factor(
  legion_region_requirement_t req_)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  return req->blocking_factor;
}

void
legion_region_requirement_set_blocking_factor(
  legion_region_requirement_t req_,
  unsigned value)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  req->blocking_factor = value;
}

unsigned
legion_region_requirement_get_max_blocking_factor(
  legion_region_requirement_t req_)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  return req->max_blocking_factor;
}

void
legion_region_requirement_add_target_ranking(
  legion_region_requirement_t req_,
  legion_memory_t memory)
{
  RegionRequirement *req = CObjectWrapper::unwrap(req_);

  req->target_ranking.push_back(CObjectWrapper::unwrap(memory));
}

// -------------------------------------------------------
// Allocator and Argument Map Operations
// -------------------------------------------------------

legion_index_allocator_t
legion_index_allocator_create(legion_runtime_t runtime_,
                              legion_context_t ctx_,
                              legion_index_space_t handle_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexSpace handle = CObjectWrapper::unwrap(handle_);

  IndexAllocator isa = runtime->create_index_allocator(ctx, handle);
  return CObjectWrapper::wrap(isa);
}

void
legion_index_allocator_destroy(legion_index_allocator_t handle_)
{
  IndexAllocator handle = CObjectWrapper::unwrap(handle_);
  // Destructor is a nop anyway.
}

legion_ptr_t
legion_index_allocator_alloc(legion_index_allocator_t allocator_,
                             unsigned num_elements)
{
  IndexAllocator allocator = CObjectWrapper::unwrap(allocator_);
  ptr_t ptr = allocator.alloc(num_elements);
  return CObjectWrapper::wrap(ptr);
}

void
legion_index_allocator_free(legion_index_allocator_t allocator_,
                            legion_ptr_t ptr_,
                            unsigned num_elements)
{
  IndexAllocator allocator = CObjectWrapper::unwrap(allocator_);
  ptr_t ptr = CObjectWrapper::unwrap(ptr_);
  allocator.free(ptr, num_elements);
}

legion_field_allocator_t
legion_field_allocator_create(legion_runtime_t runtime_,
                              legion_context_t ctx_,
                              legion_field_space_t handle_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  FieldSpace handle = CObjectWrapper::unwrap(handle_);

  FieldAllocator fsa = runtime->create_field_allocator(ctx, handle);
  return CObjectWrapper::wrap(fsa);
}

void
legion_field_allocator_destroy(legion_field_allocator_t handle_)
{
  FieldAllocator handle = CObjectWrapper::unwrap(handle_);
  // Destructor is a nop anyway.
}

legion_field_id_t
legion_field_allocator_allocate_field(legion_field_allocator_t allocator_,
                                      size_t field_size,
                                      legion_field_id_t desired_fieldid)
{
  FieldAllocator allocator = CObjectWrapper::unwrap(allocator_);
  return allocator.allocate_field(field_size, desired_fieldid);
}

void
legion_field_allocator_free_field(legion_field_allocator_t allocator_,
                                  legion_field_id_t fid)
{
  FieldAllocator allocator = CObjectWrapper::unwrap(allocator_);
  allocator.free_field(fid);
}

legion_field_id_t
legion_field_allocator_allocate_local_field(legion_field_allocator_t allocator_,
                                            size_t field_size,
                                            legion_field_id_t desired_fieldid)
{
  FieldAllocator allocator = CObjectWrapper::unwrap(allocator_);
  return allocator.allocate_local_field(field_size, desired_fieldid);
}

legion_argument_map_t
legion_argument_map_create()
{
  return CObjectWrapper::wrap(new ArgumentMap());
}

void
legion_argument_map_set_point(legion_argument_map_t map_,
                              legion_domain_point_t dp_,
                              legion_task_argument_t arg_,
                              bool replace)
{
  ArgumentMap *map = CObjectWrapper::unwrap(map_);
  DomainPoint dp = CObjectWrapper::unwrap(dp_);
  TaskArgument arg = CObjectWrapper::unwrap(arg_);

  map->set_point(dp, arg, replace);
}

void
legion_argument_map_destroy(legion_argument_map_t map_)
{
  ArgumentMap *map = CObjectWrapper::unwrap(map_);

  delete map;
}

//------------------------------------------------------------------------
// Predicate Operations
//------------------------------------------------------------------------

void
legion_predicate_destroy(legion_predicate_t handle_)
{
  Predicate *handle = CObjectWrapper::unwrap(handle_);

  delete handle;
}

const legion_predicate_t
legion_predicate_true(void)
{
  return CObjectWrapper::wrap_const(&Predicate::TRUE_PRED);
}

const legion_predicate_t
legion_predicate_false(void)
{
  return CObjectWrapper::wrap_const(&Predicate::FALSE_PRED);
}

// -----------------------------------------------------------------------
// Phase Barrier Operations
// -----------------------------------------------------------------------

legion_phase_barrier_t
legion_phase_barrier_create(legion_runtime_t runtime_,
                            legion_context_t ctx_,
                            unsigned arrivals)
{
  HighLevelRuntime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);

  PhaseBarrier result = runtime->create_phase_barrier(ctx, arrivals);
  return CObjectWrapper::wrap(result);
}

void
legion_phase_barrier_destroy(legion_runtime_t runtime_,
                             legion_context_t ctx_,
                             legion_phase_barrier_t handle_)
{
  HighLevelRuntime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  PhaseBarrier handle = CObjectWrapper::unwrap(handle_);

  runtime->destroy_phase_barrier(ctx, handle);
}

legion_phase_barrier_t
legion_phase_barrier_advance(legion_runtime_t runtime_,
                             legion_context_t ctx_,
                             legion_phase_barrier_t handle_)
{
  HighLevelRuntime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  PhaseBarrier handle = CObjectWrapper::unwrap(handle_);

  PhaseBarrier result = runtime->advance_phase_barrier(ctx, handle);
  return CObjectWrapper::wrap(result);
}

//------------------------------------------------------------------------
// Future Operations
//------------------------------------------------------------------------

legion_future_t
legion_future_from_buffer(legion_runtime_t runtime_,
                          const void *buffer,
                          size_t size)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);

  TaskResult task_result(buffer, size);
  Future *result = new Future(Future::from_value(runtime, task_result));
  return CObjectWrapper::wrap(result);
}

legion_future_t
legion_future_from_uint32(legion_runtime_t runtime_,
                          uint32_t value)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);

  Future *result = new Future(Future::from_value(runtime, value));
  return CObjectWrapper::wrap(result);
}

legion_future_t
legion_future_from_uint64(legion_runtime_t runtime_,
                          uint64_t value)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);

  Future *result = new Future(Future::from_value(runtime, value));
  return CObjectWrapper::wrap(result);
}

void
legion_future_destroy(legion_future_t handle_)
{
  Future *handle = CObjectWrapper::unwrap(handle_);

  delete handle;
}

void
legion_future_get_void_result(legion_future_t handle_)
{
  Future *handle = CObjectWrapper::unwrap(handle_);

  handle->get_void_result();
}

legion_task_result_t
legion_future_get_result(legion_future_t handle_)
{
  Future *handle = CObjectWrapper::unwrap(handle_);

  TaskResult result = handle->get_result<TaskResult>();
  return CObjectWrapper::wrap(result);
}

uint32_t
legion_future_get_result_uint32(legion_future_t handle_)
{
  Future *handle = CObjectWrapper::unwrap(handle_);

  return handle->get_result<uint32_t>();
}

uint64_t
legion_future_get_result_uint64(legion_future_t handle_)
{
  Future *handle = CObjectWrapper::unwrap(handle_);

  return handle->get_result<uint64_t>();
}

bool
legion_future_is_empty(legion_future_t handle_,
                       bool block /* = false */)
{
  Future *handle = CObjectWrapper::unwrap(handle_);

  return handle->is_empty(block);
}

const void *
legion_future_get_untyped_pointer(legion_future_t handle_)
{
  Future *handle = CObjectWrapper::unwrap(handle_);

  return handle->get_untyped_pointer();
}

legion_task_result_t
legion_task_result_create(const void *handle, size_t size)
{
  legion_task_result_t result;
  result.value = malloc(size);
  assert(result.value);
  memcpy(result.value, handle, size);
  result.value_size = size;
  return result;
}

void
legion_task_result_destroy(legion_task_result_t handle)
{
  free(handle.value);
}

// -----------------------------------------------------------------------
// Future Map Operations
// -----------------------------------------------------------------------

void
legion_future_map_destroy(legion_future_map_t fm_)
{
  FutureMap *fm = CObjectWrapper::unwrap(fm_);

  delete fm;
}

void
legion_future_map_wait_all_results(legion_future_map_t fm_)
{
  FutureMap *fm = CObjectWrapper::unwrap(fm_);

  fm->wait_all_results();
}

legion_future_t
legion_future_map_get_future(legion_future_map_t fm_,
                             legion_domain_point_t dp_)
{
  FutureMap *fm = CObjectWrapper::unwrap(fm_);
  DomainPoint dp = CObjectWrapper::unwrap(dp_);

  return CObjectWrapper::wrap(new Future(fm->get_future(dp)));
}

legion_task_result_t
legion_future_map_get_result(legion_future_map_t fm_,
                             legion_domain_point_t dp_)
{
  FutureMap *fm = CObjectWrapper::unwrap(fm_);
  DomainPoint dp = CObjectWrapper::unwrap(dp_);

  TaskResult result = fm->get_result<TaskResult>(dp);
  return CObjectWrapper::wrap(result);
}

//------------------------------------------------------------------------
// Task Launch Operations
//------------------------------------------------------------------------

legion_task_launcher_t
legion_task_launcher_create(
  legion_task_id_t tid,
  legion_task_argument_t arg_,
  legion_predicate_t pred_ /* = legion_predicate_true() */,
  legion_mapper_id_t id /* = 0 */,
  legion_mapping_tag_id_t tag /* = 0 */)
{
  TaskArgument arg = CObjectWrapper::unwrap(arg_);
  Predicate *pred = CObjectWrapper::unwrap(pred_);

  TaskLauncher *launcher = new TaskLauncher(tid, arg, *pred, id, tag);
  return CObjectWrapper::wrap(launcher);
}

void
legion_task_launcher_destroy(legion_task_launcher_t launcher_)
{
  TaskLauncher *launcher = CObjectWrapper::unwrap(launcher_);

  delete launcher;
}

legion_future_t
legion_task_launcher_execute(legion_runtime_t runtime_,
                             legion_context_t ctx_,
                             legion_task_launcher_t launcher_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  TaskLauncher *launcher = CObjectWrapper::unwrap(launcher_);

  Future f = runtime->execute_task(ctx, *launcher);
  return CObjectWrapper::wrap(new Future(f));
}

unsigned
legion_task_launcher_add_region_requirement_logical_region(
  legion_task_launcher_t launcher_,
  legion_logical_region_t handle_,
  legion_privilege_mode_t priv,
  legion_coherence_property_t prop,
  legion_logical_region_t parent_,
  legion_mapping_tag_id_t tag /* = 0 */,
  bool verified /* = false*/)
{
  TaskLauncher *launcher = CObjectWrapper::unwrap(launcher_);
  LogicalRegion handle = CObjectWrapper::unwrap(handle_);
  LogicalRegion parent = CObjectWrapper::unwrap(parent_);

  unsigned idx = launcher->region_requirements.size();
  launcher->add_region_requirement(
    RegionRequirement(handle, priv, prop, parent, tag, verified));
  return idx;
}

unsigned
legion_task_launcher_add_region_requirement_logical_region_reduction(
  legion_task_launcher_t launcher_,
  legion_logical_region_t handle_,
  legion_reduction_op_id_t redop,
  legion_coherence_property_t prop,
  legion_logical_region_t parent_,
  legion_mapping_tag_id_t tag /* = 0 */,
  bool verified /* = false*/)
{
  TaskLauncher *launcher = CObjectWrapper::unwrap(launcher_);
  LogicalRegion handle = CObjectWrapper::unwrap(handle_);
  LogicalRegion parent = CObjectWrapper::unwrap(parent_);

  unsigned idx = launcher->region_requirements.size();
  launcher->add_region_requirement(
    RegionRequirement(handle, redop, prop, parent, tag, verified));
  return idx;
}

void
legion_task_launcher_add_field(legion_task_launcher_t launcher_,
                               unsigned idx,
                               legion_field_id_t fid,
                               bool inst /* = true */)
{
  TaskLauncher *launcher = CObjectWrapper::unwrap(launcher_);

  launcher->add_field(idx, fid, inst);
}

void
legion_task_launcher_add_flags(legion_task_launcher_t launcher_,
                               unsigned idx,
                               enum legion_region_flags_t flags)
{
  TaskLauncher *launcher = CObjectWrapper::unwrap(launcher_);

  launcher->region_requirements[idx].add_flags(flags);
}

unsigned
legion_task_launcher_add_index_requirement(
  legion_task_launcher_t launcher_,
  legion_index_space_t handle_,
  legion_allocate_mode_t priv,
  legion_index_space_t parent_,
  bool verified /* = false*/)
{
  TaskLauncher *launcher = CObjectWrapper::unwrap(launcher_);
  IndexSpace handle = CObjectWrapper::unwrap(handle_);
  IndexSpace parent = CObjectWrapper::unwrap(parent_);

  unsigned idx = launcher->index_requirements.size();
  launcher->add_index_requirement(
    IndexSpaceRequirement(handle, priv, parent, verified));
  return idx;
}

void
legion_task_launcher_add_future(legion_task_launcher_t launcher_,
                                legion_future_t future_)
{
  TaskLauncher *launcher = CObjectWrapper::unwrap(launcher_);
  Future *future = CObjectWrapper::unwrap(future_);

  launcher->add_future(*future);
}

void
legion_task_launcher_add_wait_barrier(legion_task_launcher_t launcher_,
                                      legion_phase_barrier_t bar_)
{
  TaskLauncher *launcher = CObjectWrapper::unwrap(launcher_);
  PhaseBarrier bar = CObjectWrapper::unwrap(bar_);

  launcher->add_wait_barrier(bar);
}

void
legion_task_launcher_add_arrival_barrier(legion_task_launcher_t launcher_,
                                         legion_phase_barrier_t bar_)
{
  TaskLauncher *launcher = CObjectWrapper::unwrap(launcher_);
  PhaseBarrier bar = CObjectWrapper::unwrap(bar_);

  launcher->add_arrival_barrier(bar);
}

legion_index_launcher_t
legion_index_launcher_create(
  legion_task_id_t tid,
  legion_domain_t domain_,
  legion_task_argument_t global_arg_,
  legion_argument_map_t map_,
  legion_predicate_t pred_ /* = legion_predicate_true() */,
  bool must /* = false */,
  legion_mapper_id_t id /* = 0 */,
  legion_mapping_tag_id_t tag /* = 0 */)
{
  Domain domain = CObjectWrapper::unwrap(domain_);
  TaskArgument global_arg = CObjectWrapper::unwrap(global_arg_);
  ArgumentMap *map = CObjectWrapper::unwrap(map_);
  Predicate *pred = CObjectWrapper::unwrap(pred_);

  IndexLauncher *launcher =
    new IndexLauncher(tid, domain, global_arg, *map, *pred, must, id, tag);
  return CObjectWrapper::wrap(launcher);
}

void
legion_index_launcher_destroy(legion_index_launcher_t launcher_)
{
  IndexLauncher *launcher = CObjectWrapper::unwrap(launcher_);

  delete launcher;
}

legion_future_map_t
legion_index_launcher_execute(legion_runtime_t runtime_,
                             legion_context_t ctx_,
                             legion_index_launcher_t launcher_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexLauncher *launcher = CObjectWrapper::unwrap(launcher_);

  FutureMap f = runtime->execute_index_space(ctx, *launcher);
  return CObjectWrapper::wrap(new FutureMap(f));
}

legion_future_t
legion_index_launcher_execute_reduction(legion_runtime_t runtime_,
                                        legion_context_t ctx_,
                                        legion_index_launcher_t launcher_,
                                        legion_reduction_op_id_t redop)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexLauncher *launcher = CObjectWrapper::unwrap(launcher_);

  Future f = runtime->execute_index_space(ctx, *launcher, redop);
  return CObjectWrapper::wrap(new Future(f));
}

unsigned
legion_index_launcher_add_region_requirement_logical_region(
  legion_index_launcher_t launcher_,
  legion_logical_region_t handle_,
  legion_projection_id_t proj /* = 0 */,
  legion_privilege_mode_t priv,
  legion_coherence_property_t prop,
  legion_logical_region_t parent_,
  legion_mapping_tag_id_t tag /* = 0 */,
  bool verified /* = false*/)
{
  IndexLauncher *launcher = CObjectWrapper::unwrap(launcher_);
  LogicalRegion handle = CObjectWrapper::unwrap(handle_);
  LogicalRegion parent = CObjectWrapper::unwrap(parent_);

  unsigned idx = launcher->region_requirements.size();
  launcher->add_region_requirement(
    RegionRequirement(handle, proj, priv, prop, parent, tag, verified));
  return idx;
}

unsigned
legion_index_launcher_add_region_requirement_logical_partition(
  legion_index_launcher_t launcher_,
  legion_logical_partition_t handle_,
  legion_projection_id_t proj /* = 0 */,
  legion_privilege_mode_t priv,
  legion_coherence_property_t prop,
  legion_logical_region_t parent_,
  legion_mapping_tag_id_t tag /* = 0 */,
  bool verified /* = false*/)
{
  IndexLauncher *launcher = CObjectWrapper::unwrap(launcher_);
  LogicalPartition handle = CObjectWrapper::unwrap(handle_);
  LogicalRegion parent = CObjectWrapper::unwrap(parent_);

  unsigned idx = launcher->region_requirements.size();
  launcher->add_region_requirement(
    RegionRequirement(handle, proj, priv, prop, parent, tag, verified));
  return idx;
}

unsigned
legion_index_launcher_add_region_requirement_logical_region_reduction(
  legion_index_launcher_t launcher_,
  legion_logical_region_t handle_,
  legion_projection_id_t proj /* = 0 */,
  legion_reduction_op_id_t redop,
  legion_coherence_property_t prop,
  legion_logical_region_t parent_,
  legion_mapping_tag_id_t tag /* = 0 */,
  bool verified /* = false*/)
{
  IndexLauncher *launcher = CObjectWrapper::unwrap(launcher_);
  LogicalRegion handle = CObjectWrapper::unwrap(handle_);
  LogicalRegion parent = CObjectWrapper::unwrap(parent_);

  unsigned idx = launcher->region_requirements.size();
  launcher->add_region_requirement(
    RegionRequirement(handle, proj, redop, prop, parent, tag, verified));
  return idx;
}

unsigned
legion_index_launcher_add_region_requirement_logical_partition_reduction(
  legion_index_launcher_t launcher_,
  legion_logical_partition_t handle_,
  legion_projection_id_t proj /* = 0 */,
  legion_reduction_op_id_t redop,
  legion_coherence_property_t prop,
  legion_logical_region_t parent_,
  legion_mapping_tag_id_t tag /* = 0 */,
  bool verified /* = false*/)
{
  IndexLauncher *launcher = CObjectWrapper::unwrap(launcher_);
  LogicalPartition handle = CObjectWrapper::unwrap(handle_);
  LogicalRegion parent = CObjectWrapper::unwrap(parent_);

  unsigned idx = launcher->region_requirements.size();
  launcher->add_region_requirement(
    RegionRequirement(handle, proj, redop, prop, parent, tag, verified));
  return idx;
}

void
legion_index_launcher_add_field(legion_index_launcher_t launcher_,
                               unsigned idx,
                               legion_field_id_t fid,
                               bool inst /* = true */)
{
  IndexLauncher *launcher = CObjectWrapper::unwrap(launcher_);

  launcher->add_field(idx, fid, inst);
}

void
legion_index_launcher_add_flags(legion_index_launcher_t launcher_,
                                unsigned idx,
                                enum legion_region_flags_t flags)
{
  IndexLauncher *launcher = CObjectWrapper::unwrap(launcher_);

  launcher->region_requirements[idx].add_flags(flags);
}

unsigned
legion_index_launcher_add_index_requirement(
  legion_index_launcher_t launcher_,
  legion_index_space_t handle_,
  legion_allocate_mode_t priv,
  legion_index_space_t parent_,
  bool verified /* = false*/)
{
  IndexLauncher *launcher = CObjectWrapper::unwrap(launcher_);
  IndexSpace handle = CObjectWrapper::unwrap(handle_);
  IndexSpace parent = CObjectWrapper::unwrap(parent_);

  unsigned idx = launcher->index_requirements.size();
  launcher->add_index_requirement(
    IndexSpaceRequirement(handle, priv, parent, verified));
  return idx;
}

void
legion_index_launcher_add_future(legion_index_launcher_t launcher_,
                                 legion_future_t future_)
{
  IndexLauncher *launcher = CObjectWrapper::unwrap(launcher_);
  Future *future = CObjectWrapper::unwrap(future_);

  launcher->add_future(*future);
}

void
legion_index_launcher_add_wait_barrier(legion_index_launcher_t launcher_,
                                      legion_phase_barrier_t bar_)
{
  IndexLauncher *launcher = CObjectWrapper::unwrap(launcher_);
  PhaseBarrier bar = CObjectWrapper::unwrap(bar_);

  launcher->add_wait_barrier(bar);
}

void
legion_index_launcher_add_arrival_barrier(legion_index_launcher_t launcher_,
                                         legion_phase_barrier_t bar_)
{
  IndexLauncher *launcher = CObjectWrapper::unwrap(launcher_);
  PhaseBarrier bar = CObjectWrapper::unwrap(bar_);

  launcher->add_arrival_barrier(bar);
}

// -----------------------------------------------------------------------
// Inline Mapping Operations
// -----------------------------------------------------------------------

legion_inline_launcher_t
legion_inline_launcher_create_logical_region(
  legion_logical_region_t handle_,
  legion_privilege_mode_t priv,
  legion_coherence_property_t prop,
  legion_logical_region_t parent_,
  legion_mapping_tag_id_t region_tag /* = 0 */,
  bool verified /* = false*/,
  legion_mapper_id_t id /* = 0 */,
  legion_mapping_tag_id_t launcher_tag /* = 0 */)
{
  LogicalRegion handle = CObjectWrapper::unwrap(handle_);
  LogicalRegion parent = CObjectWrapper::unwrap(parent_);

  InlineLauncher *launcher = new InlineLauncher(
    RegionRequirement(handle, priv, prop, parent, region_tag, verified),
    id,
    launcher_tag);
  return CObjectWrapper::wrap(launcher);
}

void
legion_inline_launcher_destroy(legion_inline_launcher_t handle_)
{
  InlineLauncher *handle = CObjectWrapper::unwrap(handle_);

  delete handle;
}

legion_physical_region_t
legion_inline_launcher_execute(legion_runtime_t runtime_,
                               legion_context_t ctx_,
                               legion_inline_launcher_t launcher_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  InlineLauncher *launcher = CObjectWrapper::unwrap(launcher_);

  PhysicalRegion r = runtime->map_region(ctx, *launcher);
  return CObjectWrapper::wrap(new PhysicalRegion(r));
}

void
legion_inline_launcher_add_field(legion_inline_launcher_t launcher_,
                                 legion_field_id_t fid,
                                 bool inst /* = true */)
{
  InlineLauncher *launcher = CObjectWrapper::unwrap(launcher_);

  launcher->add_field(fid, inst);
}

void
legion_runtime_remap_region(legion_runtime_t runtime_,
                            legion_context_t ctx_,
                            legion_physical_region_t region_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  PhysicalRegion *region = CObjectWrapper::unwrap(region_);

  runtime->remap_region(ctx, *region);
}

void
legion_runtime_unmap_region(legion_runtime_t runtime_,
                            legion_context_t ctx_,
                            legion_physical_region_t region_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  PhysicalRegion *region = CObjectWrapper::unwrap(region_);

  runtime->unmap_region(ctx, *region);
}

void
legion_runtime_unmap_all_regions(legion_runtime_t runtime_,
                                 legion_context_t ctx_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);

  runtime->unmap_all_regions(ctx);
}

void
legion_runtime_fill_field(
  legion_runtime_t runtime_,
  legion_context_t ctx_,
  legion_logical_region_t handle_,
  legion_logical_region_t parent_,
  legion_field_id_t fid,
  const void *value,
  size_t value_size,
  legion_predicate_t pred_ /* = legion_predicate_true() */)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  LogicalRegion handle = CObjectWrapper::unwrap(handle_);
  LogicalRegion parent = CObjectWrapper::unwrap(parent_);
  Predicate *pred = CObjectWrapper::unwrap(pred_);

  runtime->fill_field(ctx, handle, parent, fid, value, value_size, *pred);
}

void
legion_runtime_fill_field_future(
  legion_runtime_t runtime_,
  legion_context_t ctx_,
  legion_logical_region_t handle_,
  legion_logical_region_t parent_,
  legion_field_id_t fid,
  legion_future_t f_,
  legion_predicate_t pred_ /* = legion_predicate_true() */)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  LogicalRegion handle = CObjectWrapper::unwrap(handle_);
  LogicalRegion parent = CObjectWrapper::unwrap(parent_);
  Future *f = CObjectWrapper::unwrap(f_);
  Predicate *pred = CObjectWrapper::unwrap(pred_);

  runtime->fill_field(ctx, handle, parent, fid, *f, *pred);
}

// -----------------------------------------------------------------------
// Copy Operations
// -----------------------------------------------------------------------

legion_copy_launcher_t
legion_copy_launcher_create(
  legion_predicate_t pred_ /* = legion_predicate_true() */,
  legion_mapper_id_t id /* = 0 */,
  legion_mapping_tag_id_t launcher_tag /* = 0 */)
{
  Predicate *pred = CObjectWrapper::unwrap(pred_);

  CopyLauncher *launcher = new CopyLauncher(*pred, id, launcher_tag);
  return CObjectWrapper::wrap(launcher);
}

void
legion_copy_launcher_destroy(legion_copy_launcher_t handle_)
{
  CopyLauncher *handle = CObjectWrapper::unwrap(handle_);

  delete handle;
}

void
legion_copy_launcher_execute(legion_runtime_t runtime_,
                             legion_context_t ctx_,
                             legion_copy_launcher_t launcher_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  CopyLauncher *launcher = CObjectWrapper::unwrap(launcher_);

  runtime->issue_copy_operation(ctx, *launcher);
}

unsigned
legion_copy_launcher_add_src_region_requirement_logical_region(
  legion_copy_launcher_t launcher_,
  legion_logical_region_t handle_,
  legion_privilege_mode_t priv,
  legion_coherence_property_t prop,
  legion_logical_region_t parent_,
  legion_mapping_tag_id_t tag /* = 0 */,
  bool verified /* = false*/)
{
  CopyLauncher *launcher = CObjectWrapper::unwrap(launcher_);
  LogicalRegion handle = CObjectWrapper::unwrap(handle_);
  LogicalRegion parent = CObjectWrapper::unwrap(parent_);

  unsigned idx = launcher->src_requirements.size();
  launcher->src_requirements.push_back(
    RegionRequirement(handle, priv, prop, parent, tag, verified));
  return idx;
}

unsigned
legion_copy_launcher_add_dst_region_requirement_logical_region(
  legion_copy_launcher_t launcher_,
  legion_logical_region_t handle_,
  legion_privilege_mode_t priv,
  legion_coherence_property_t prop,
  legion_logical_region_t parent_,
  legion_mapping_tag_id_t tag /* = 0 */,
  bool verified /* = false*/)
{
  CopyLauncher *launcher = CObjectWrapper::unwrap(launcher_);
  LogicalRegion handle = CObjectWrapper::unwrap(handle_);
  LogicalRegion parent = CObjectWrapper::unwrap(parent_);

  unsigned idx = launcher->dst_requirements.size();
  launcher->dst_requirements.push_back(
    RegionRequirement(handle, priv, prop, parent, tag, verified));
  return idx;
}

unsigned
legion_copy_launcher_add_dst_region_requirement_logical_region_reduction(
  legion_copy_launcher_t launcher_,
  legion_logical_region_t handle_,
  legion_reduction_op_id_t redop,
  legion_coherence_property_t prop,
  legion_logical_region_t parent_,
  legion_mapping_tag_id_t tag /* = 0 */,
  bool verified /* = false*/)
{
  CopyLauncher *launcher = CObjectWrapper::unwrap(launcher_);
  LogicalRegion handle = CObjectWrapper::unwrap(handle_);
  LogicalRegion parent = CObjectWrapper::unwrap(parent_);

  unsigned idx = launcher->dst_requirements.size();
  launcher->dst_requirements.push_back(
    RegionRequirement(handle, redop, prop, parent, tag, verified));
  return idx;
}

void
legion_copy_launcher_add_src_field(legion_copy_launcher_t launcher_,
                                   unsigned idx,
                                   legion_field_id_t fid,
                                   bool inst /* = true */)
{
  CopyLauncher *launcher = CObjectWrapper::unwrap(launcher_);

  launcher->add_src_field(idx, fid, inst);
}

void
legion_copy_launcher_add_dst_field(legion_copy_launcher_t launcher_,
                                   unsigned idx,
                                   legion_field_id_t fid,
                                   bool inst /* = true */)
{
  CopyLauncher *launcher = CObjectWrapper::unwrap(launcher_);

  launcher->add_dst_field(idx, fid, inst);
}

void
legion_copy_launcher_add_wait_barrier(legion_copy_launcher_t launcher_,
                                      legion_phase_barrier_t bar_)
{
  CopyLauncher *launcher = CObjectWrapper::unwrap(launcher_);
  PhaseBarrier bar = CObjectWrapper::unwrap(bar_);

  launcher->add_wait_barrier(bar);
}

void
legion_copy_launcher_add_arrival_barrier(legion_copy_launcher_t launcher_,
                                         legion_phase_barrier_t bar_)
{
  CopyLauncher *launcher = CObjectWrapper::unwrap(launcher_);
  PhaseBarrier bar = CObjectWrapper::unwrap(bar_);

  launcher->add_arrival_barrier(bar);
}

// -----------------------------------------------------------------------
// Must Epoch Operations
// -----------------------------------------------------------------------

legion_must_epoch_launcher_t
legion_must_epoch_launcher_create(
  legion_mapper_id_t id /* = 0 */,
  legion_mapping_tag_id_t launcher_tag /* = 0 */)
{
  MustEpochLauncher *launcher = new MustEpochLauncher(id, launcher_tag);
  return CObjectWrapper::wrap(launcher);
}

void
legion_must_epoch_launcher_destroy(legion_must_epoch_launcher_t handle_)
{
  MustEpochLauncher *handle = CObjectWrapper::unwrap(handle_);

  delete handle;
}

legion_future_map_t
legion_must_epoch_launcher_execute(legion_runtime_t runtime_,
                                   legion_context_t ctx_,
                                   legion_must_epoch_launcher_t launcher_)
{
  HighLevelRuntime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  MustEpochLauncher *launcher = CObjectWrapper::unwrap(launcher_);

  FutureMap f = runtime->execute_must_epoch(ctx, *launcher);
  return CObjectWrapper::wrap(new FutureMap(f));
}

void
legion_must_epoch_launcher_add_single_task(
  legion_must_epoch_launcher_t launcher_,
  legion_domain_point_t point_,
  legion_task_launcher_t handle_)
{
  MustEpochLauncher *launcher = CObjectWrapper::unwrap(launcher_);
  DomainPoint point = CObjectWrapper::unwrap(point_);
  {
    TaskLauncher *handle = CObjectWrapper::unwrap(handle_);
    launcher->add_single_task(point, *handle);
  }

  // Destroy handle.
  legion_task_launcher_destroy(handle_);
}

void
legion_must_epoch_launcher_add_index_task(
  legion_must_epoch_launcher_t launcher_,
  legion_index_launcher_t handle_)
{
  MustEpochLauncher *launcher = CObjectWrapper::unwrap(launcher_);
  {
    IndexLauncher *handle = CObjectWrapper::unwrap(handle_);
    launcher->add_index_task(*handle);
  }

  // Destroy handle.
  legion_index_launcher_destroy(handle_);
}

// -----------------------------------------------------------------------
// Tracing Operations
// -----------------------------------------------------------------------

void
legion_runtime_begin_trace(legion_runtime_t runtime_,
                           legion_context_t ctx_,
                           legion_trace_id_t tid)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);

  runtime->begin_trace(ctx, tid);
}

void
legion_runtime_end_trace(legion_runtime_t runtime_,
                         legion_context_t ctx_,
                         legion_trace_id_t tid)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);

  runtime->end_trace(ctx, tid);
}

// -----------------------------------------------------------------------
// Fence Operations
// -----------------------------------------------------------------------

void
legion_runtime_issue_mapping_fence(legion_runtime_t runtime_,
                                   legion_context_t ctx_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);

  runtime->issue_mapping_fence(ctx);
}

void
legion_runtime_issue_execution_fence(legion_runtime_t runtime_,
                                     legion_context_t ctx_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);

  runtime->issue_execution_fence(ctx);
}

// -----------------------------------------------------------------------
// Miscellaneous Operations
// -----------------------------------------------------------------------

legion_processor_t
legion_runtime_get_executing_processor(legion_runtime_t runtime_,
                                       legion_context_t ctx_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);

  Processor proc = runtime->get_executing_processor(ctx);
  return CObjectWrapper::wrap(proc);
}

// -----------------------------------------------------------------------
// Physical Data Operations
// -----------------------------------------------------------------------

void
legion_physical_region_destroy(legion_physical_region_t handle_)
{
  PhysicalRegion *handle = CObjectWrapper::unwrap(handle_);

  delete handle;
}

bool
legion_physical_region_is_mapped(legion_physical_region_t handle_)
{
  PhysicalRegion *handle = CObjectWrapper::unwrap(handle_);

  return handle->is_mapped();
}

void
legion_physical_region_wait_until_valid(legion_physical_region_t handle_)
{
  PhysicalRegion *handle = CObjectWrapper::unwrap(handle_);

  handle->wait_until_valid();
}

bool
legion_physical_region_is_valid(legion_physical_region_t handle_)
{
  PhysicalRegion *handle = CObjectWrapper::unwrap(handle_);

  return handle->is_valid();
}

legion_logical_region_t
legion_physical_region_get_logical_region(legion_physical_region_t handle_)
{
  PhysicalRegion *handle = CObjectWrapper::unwrap(handle_);

  LogicalRegion region = handle->get_logical_region();
  return CObjectWrapper::wrap(region);
}

legion_accessor_generic_t
legion_physical_region_get_accessor_generic(legion_physical_region_t handle_)
{
  PhysicalRegion *handle = CObjectWrapper::unwrap(handle_);

  AccessorGeneric *accessor =
    new AccessorGeneric(handle->get_accessor());
  return CObjectWrapper::wrap(accessor);
}

legion_accessor_generic_t
legion_physical_region_get_field_accessor_generic(
  legion_physical_region_t handle_,
  legion_field_id_t fid)
{
  PhysicalRegion *handle = CObjectWrapper::unwrap(handle_);

  AccessorGeneric *accessor =
    new AccessorGeneric(handle->get_field_accessor(fid));
  return CObjectWrapper::wrap(accessor);
}

legion_accessor_array_t
legion_physical_region_get_accessor_array(
  legion_physical_region_t handle_)
{
  PhysicalRegion *handle = CObjectWrapper::unwrap(handle_);

  AccessorArray *accessor =
    new AccessorArray(
      handle->get_accessor().typeify<char>().convert<SOA>());
  return CObjectWrapper::wrap(accessor);
}

legion_accessor_array_t
legion_physical_region_get_field_accessor_array(
  legion_physical_region_t handle_,
  legion_field_id_t fid)
{
  PhysicalRegion *handle = CObjectWrapper::unwrap(handle_);

  AccessorArray *accessor =
    new AccessorArray(
      handle->get_field_accessor(fid).typeify<char>().convert<SOA>());
  return CObjectWrapper::wrap(accessor);
}

void
legion_accessor_generic_destroy(legion_accessor_generic_t handle_)
{
  AccessorGeneric *handle = CObjectWrapper::unwrap(handle_);

  delete handle;
}

void
legion_accessor_generic_read(legion_accessor_generic_t handle_,
                             legion_ptr_t ptr_,
                             void *dst,
                             size_t bytes)
{
  AccessorGeneric *handle = CObjectWrapper::unwrap(handle_);
  ptr_t ptr = CObjectWrapper::unwrap(ptr_);

  handle->read_untyped(ptr, dst, bytes);
}

void
legion_accessor_generic_write(legion_accessor_generic_t handle_,
                              legion_ptr_t ptr_,
                              const void *src,
                              size_t bytes)
{
  AccessorGeneric *handle = CObjectWrapper::unwrap(handle_);
  ptr_t ptr = CObjectWrapper::unwrap(ptr_);

  handle->write_untyped(ptr, src, bytes);
}

void
legion_accessor_generic_read_domain_point(legion_accessor_generic_t handle_,
                                          legion_domain_point_t dp_,
                                          void *dst,
                                          size_t bytes)
{
  AccessorGeneric *handle = CObjectWrapper::unwrap(handle_);
  DomainPoint dp = CObjectWrapper::unwrap(dp_);

  handle->read_untyped(dp, dst, bytes);
}

void
legion_accessor_generic_write_domain_point(legion_accessor_generic_t handle_,
                                           legion_domain_point_t dp_,
                                           const void *src,
                                           size_t bytes)
{
  AccessorGeneric *handle = CObjectWrapper::unwrap(handle_);
  DomainPoint dp = CObjectWrapper::unwrap(dp_);

  handle->write_untyped(dp, src, bytes);
}

void *
legion_accessor_generic_raw_span_ptr(legion_accessor_generic_t handle_,
                                     legion_ptr_t ptr_,
                                     size_t req_count,
                                     size_t *act_count,
                                     legion_byte_offset_t *stride_)
{
  AccessorGeneric *handle = CObjectWrapper::unwrap(handle_);
  ptr_t ptr = CObjectWrapper::unwrap(ptr_);

  Accessor::ByteOffset stride;
  void *data = handle->raw_span_ptr(ptr, req_count, *act_count, stride);
  *stride_ = CObjectWrapper::wrap(stride);
  return data;
}

void *
legion_accessor_generic_raw_rect_ptr_1d(legion_accessor_generic_t handle_,
                                        legion_rect_1d_t rect_,
                                        legion_rect_1d_t *subrect_,
                                        legion_byte_offset_t *offsets_)
{
  AccessorGeneric *handle = CObjectWrapper::unwrap(handle_);
  Rect<1> rect = CObjectWrapper::unwrap(rect_);

  Rect<1> subrect;
  Accessor::ByteOffset offsets[1];
  void *data = handle->raw_rect_ptr<1>(rect, subrect, &offsets[0]);
  *subrect_ = CObjectWrapper::wrap(subrect);
  offsets_[0] = CObjectWrapper::wrap(offsets[0]);
  return data;
}

void *
legion_accessor_generic_raw_rect_ptr_2d(legion_accessor_generic_t handle_,
                                        legion_rect_2d_t rect_,
                                        legion_rect_2d_t *subrect_,
                                        legion_byte_offset_t *offsets_)
{
  AccessorGeneric *handle = CObjectWrapper::unwrap(handle_);
  Rect<2> rect = CObjectWrapper::unwrap(rect_);

  Rect<2> subrect;
  Accessor::ByteOffset offsets[2];
  void *data = handle->raw_rect_ptr<2>(rect, subrect, &offsets[0]);
  *subrect_ = CObjectWrapper::wrap(subrect);
  offsets_[0] = CObjectWrapper::wrap(offsets[0]);
  offsets_[1] = CObjectWrapper::wrap(offsets[1]);
  return data;
}

void *
legion_accessor_generic_raw_rect_ptr_3d(legion_accessor_generic_t handle_,
                                        legion_rect_3d_t rect_,
                                        legion_rect_3d_t *subrect_,
                                        legion_byte_offset_t *offsets_)
{
  AccessorGeneric *handle = CObjectWrapper::unwrap(handle_);
  Rect<3> rect = CObjectWrapper::unwrap(rect_);

  Rect<3> subrect;
  Accessor::ByteOffset offsets[3];
  void *data = handle->raw_rect_ptr<3>(rect, subrect, &offsets[0]);
  *subrect_ = CObjectWrapper::wrap(subrect);
  offsets_[0] = CObjectWrapper::wrap(offsets[0]);
  offsets_[1] = CObjectWrapper::wrap(offsets[1]);
  offsets_[2] = CObjectWrapper::wrap(offsets[2]);
  return data;
}

bool
legion_accessor_generic_get_soa_parameters(legion_accessor_generic_t handle_,
                                           void **base,
                                           size_t *stride)
{
  AccessorGeneric *handle = CObjectWrapper::unwrap(handle_);

  return handle->get_soa_parameters(*base, *stride);
}

void
legion_accessor_array_destroy(legion_accessor_array_t handle_)
{
  AccessorArray *handle = CObjectWrapper::unwrap(handle_);

  delete handle;
}

void
legion_accessor_array_read(legion_accessor_array_t handle_,
                           legion_ptr_t ptr_,
                           void *dst,
                           size_t bytes)
{
  AccessorArray *handle = CObjectWrapper::unwrap(handle_);
  ptr_t ptr = CObjectWrapper::unwrap(ptr_);

  char *data = &(handle->ref(ptr));
  std::copy(data, data + bytes, static_cast<char *>(dst));
}

void
legion_accessor_array_write(legion_accessor_array_t handle_,
                            legion_ptr_t ptr_,
                            const void *src,
                            size_t bytes)
{
  AccessorArray *handle = CObjectWrapper::unwrap(handle_);
  ptr_t ptr = CObjectWrapper::unwrap(ptr_);

  char *data = &(handle->ref(ptr));
  std::copy(static_cast<const char *>(src),
            static_cast<const char *>(src) + bytes,
            data);
}

void *
legion_accessor_array_ref(legion_accessor_array_t handle_,
                          legion_ptr_t ptr_)
{
  AccessorArray *handle = CObjectWrapper::unwrap(handle_);
  ptr_t ptr = CObjectWrapper::unwrap(ptr_);

  char *data = &(handle->ref(ptr));
  return static_cast<void *>(data);
}

legion_index_iterator_t
legion_index_iterator_create(legion_runtime_t runtime_,
                             legion_context_t ctx_,
                             legion_index_space_t handle_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Context ctx = CObjectWrapper::unwrap(ctx_);
  IndexSpace handle = CObjectWrapper::unwrap(handle_);

  IndexIterator *iterator = new IndexIterator(runtime, ctx, handle);
  return CObjectWrapper::wrap(iterator);
}

void
legion_index_iterator_destroy(legion_index_iterator_t handle_)
{
  IndexIterator *handle = CObjectWrapper::unwrap(handle_);

  delete handle;
}

bool
legion_index_iterator_has_next(legion_index_iterator_t handle_)
{
  IndexIterator *handle = CObjectWrapper::unwrap(handle_);

  return handle->has_next();
}

legion_ptr_t
legion_index_iterator_next(legion_index_iterator_t handle_)
{
  IndexIterator *handle = CObjectWrapper::unwrap(handle_);

  return CObjectWrapper::wrap(handle->next());
}

legion_ptr_t
legion_index_iterator_next_span(legion_index_iterator_t handle_,
                                size_t *act_count,
                                size_t req_count)
{
  IndexIterator *handle = CObjectWrapper::unwrap(handle_);

  return CObjectWrapper::wrap(handle->next_span(*act_count, req_count));
}

//------------------------------------------------------------------------
// Task Operations
//------------------------------------------------------------------------

void *
legion_task_get_args(legion_task_t task_)
{
  Task *task = CObjectWrapper::unwrap(task_);

  return task->args;
}

size_t
legion_task_get_arglen(legion_task_t task_)
{
  Task *task = CObjectWrapper::unwrap(task_);

  return task->arglen;
}

legion_domain_t
legion_task_get_index_domain(legion_task_t task_)
{
  Task *task = CObjectWrapper::unwrap(task_);

  return CObjectWrapper::wrap(task->index_domain);
}

legion_domain_point_t
legion_task_get_index_point(legion_task_t task_)
{
  Task *task = CObjectWrapper::unwrap(task_);

  return CObjectWrapper::wrap(task->index_point);
}

bool
legion_task_get_is_index_space(legion_task_t task_)
{
  Task *task = CObjectWrapper::unwrap(task_);

  return task->is_index_space;
}

void *
legion_task_get_local_args(legion_task_t task_)
{
  Task *task = CObjectWrapper::unwrap(task_);

  return task->local_args;
}

size_t
legion_task_get_local_arglen(legion_task_t task_)
{
  Task *task = CObjectWrapper::unwrap(task_);

  return task->local_arglen;
}

unsigned
legion_task_get_regions_size(legion_task_t task_)
{
  Task *task = CObjectWrapper::unwrap(task_);

  return task->regions.size();
}

legion_region_requirement_t
legion_task_get_region(legion_task_t task_, unsigned idx)
{
  Task *task = CObjectWrapper::unwrap(task_);

  return CObjectWrapper::wrap(&task->regions[idx]);
}

unsigned
legion_task_get_futures_size(legion_task_t task_)
{
  Task *task = CObjectWrapper::unwrap(task_);

  return task->futures.size();
}

legion_future_t
legion_task_get_future(legion_task_t task_, unsigned idx)
{
  Task *task = CObjectWrapper::unwrap(task_);
  Future future = task->futures[idx];

  return CObjectWrapper::wrap(new Future(future));
}

legion_task_id_t
legion_task_get_task_id(legion_task_t task_)
{
  Task *task = CObjectWrapper::unwrap(task_);

  return task->task_id;
}

legion_processor_t
legion_task_get_target_proc(legion_task_t task_)
{
  Task *task = CObjectWrapper::unwrap(task_);

  return CObjectWrapper::wrap(task->target_proc);
}

const char *
legion_task_get_name(legion_task_t task_)
{
  Task *task = CObjectWrapper::unwrap(task_);

  return task->variants->name;
}

void
legion_task_set_target_proc(legion_task_t task_, legion_processor_t proc_)
{
  Task *task = CObjectWrapper::unwrap(task_);
  Processor proc = CObjectWrapper::unwrap(proc_);

  task->target_proc = proc;
}

void
legion_task_add_additional_proc(legion_task_t task_, legion_processor_t proc_)
{
  Task *task = CObjectWrapper::unwrap(task_);
  Processor proc = CObjectWrapper::unwrap(proc_);

  task->additional_procs.insert(proc);
}

// -----------------------------------------------------------------------
// Inline Operations
// -----------------------------------------------------------------------

legion_region_requirement_t
legion_inline_get_requirement(legion_inline_t inline_operation_)
{
  Inline *inline_operation = CObjectWrapper::unwrap(inline_operation_);

  return CObjectWrapper::wrap(&inline_operation->requirement);
}

//------------------------------------------------------------------------
// Start-up Operations
//------------------------------------------------------------------------

int
legion_runtime_start(int argc,
                     char **argv,
                     bool background /* = false */)
{
  return Runtime::start(argc, argv, background);
}

void
legion_runtime_wait_for_shutdown(void)
{
  Runtime::wait_for_shutdown();
}

void
legion_runtime_set_top_level_task_id(legion_task_id_t top_id)
{
  Runtime::set_top_level_task_id(top_id);
}

const legion_input_args_t
legion_runtime_get_input_args(void)
{
  return CObjectWrapper::wrap_const(Runtime::get_input_args());
}

// a pointer to the callback function that is last registered
static legion_registration_callback_pointer_t callback;

void
registration_callback_wrapper(Machine machine,
                              Runtime *rt,
                              const std::set<Processor> &local_procs)
{
  legion_machine_t machine_ = CObjectWrapper::wrap(&machine);
  legion_runtime_t rt_ = CObjectWrapper::wrap(rt);
  legion_processor_t local_procs_[local_procs.size()];

  unsigned idx = 0;
  for (std::set<Processor>::iterator itr = local_procs.begin();
      itr != local_procs.end(); ++itr)
  {
    const Processor& proc = *itr;
    local_procs_[idx++] = CObjectWrapper::wrap(proc);
  }

  callback(machine_, rt_, local_procs_, idx);
}

void
legion_runtime_set_registration_callback(
  legion_registration_callback_pointer_t callback_)
{
  callback = callback_;
  Runtime::set_registration_callback(registration_callback_wrapper);
}

void
legion_runtime_replace_default_mapper(
  legion_runtime_t runtime_,
  legion_mapper_t mapper_,
  legion_processor_t proc_)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);
  Mapper *mapper = CObjectWrapper::unwrap(mapper_);
  Processor proc = CObjectWrapper::unwrap(proc_);

  runtime->replace_default_mapper(mapper, proc);
}

void
task_wrapper_void(const Task *task,
                  const std::vector<PhysicalRegion> &regions,
                  Context ctx,
                  Runtime *runtime,
                  const legion_task_pointer_void_t &task_pointer)
{
  const legion_task_t task_ = CObjectWrapper::wrap_const(task);
  std::vector<legion_physical_region_t> regions_;
  for (size_t i = 0; i < regions.size(); i++) {
    regions_.push_back(CObjectWrapper::wrap_const(&regions[i]));
  }
  legion_physical_region_t *regions_ptr = NULL;
  if (regions.size() > 0) {
    regions_ptr = &regions_[0];
  }
  unsigned num_regions = regions_.size();
  legion_context_t ctx_ = CObjectWrapper::wrap(ctx);
  legion_runtime_t runtime_ = CObjectWrapper::wrap(runtime);

  task_pointer(task_, regions_ptr, num_regions, ctx_, runtime_);
}

legion_task_id_t
legion_runtime_register_task_void(
  legion_task_id_t id,
  legion_processor_kind_t proc_kind_,
  bool single,
  bool index,
  legion_variant_id_t vid /* = AUTO_GENERATE_ID */,
  legion_task_config_options_t options_,
  const char *task_name /* = NULL*/,
  legion_task_pointer_void_t task_pointer)
{
  Processor::Kind proc_kind = CObjectWrapper::unwrap(proc_kind_);
  TaskConfigOptions options = CObjectWrapper::unwrap(options_);

  return Runtime::register_legion_task<
    legion_task_pointer_void_t, task_wrapper_void>(
    id, proc_kind, single, index, task_pointer, vid, options, task_name);
}

TaskResult
task_wrapper(const Task *task,
             const std::vector<PhysicalRegion> &regions,
             Context ctx,
             Runtime *runtime,
             const legion_task_pointer_t &task_pointer)
{
  const legion_task_t task_ = CObjectWrapper::wrap_const(task);
  std::vector<legion_physical_region_t> regions_;
  for (size_t i = 0; i < regions.size(); i++) {
    regions_.push_back(CObjectWrapper::wrap_const(&regions[i]));
  }
  legion_physical_region_t *regions_ptr = NULL;
  if (regions_.size() > 0) {
    regions_ptr = &regions_[0];
  }
  unsigned num_regions = regions_.size();
  legion_context_t ctx_ = CObjectWrapper::wrap(ctx);
  legion_runtime_t runtime_ = CObjectWrapper::wrap(runtime);

  legion_task_result_t result_ =
    task_pointer(task_, regions_ptr, num_regions, ctx_, runtime_);

  TaskResult result = CObjectWrapper::unwrap(result_);
  legion_task_result_destroy(result_);
  return result;
}

legion_task_id_t
legion_runtime_register_task(
  legion_task_id_t id,
  legion_processor_kind_t proc_kind_,
  bool single,
  bool index,
  legion_variant_id_t vid /* = AUTO_GENERATE_ID */,
  legion_task_config_options_t options_,
  const char *task_name /* = NULL*/,
  legion_task_pointer_t task_pointer)
{
  Processor::Kind proc_kind = CObjectWrapper::unwrap(proc_kind_);
  TaskConfigOptions options = CObjectWrapper::unwrap(options_);

  return Runtime::register_legion_task<
    TaskResult, legion_task_pointer_t, task_wrapper>(
    id, proc_kind, single, index, task_pointer, vid, options, task_name);
}

uint32_t
task_wrapper_uint32(const Task *task,
                    const std::vector<PhysicalRegion> &regions,
                    Context ctx,
                    Runtime *runtime,
                    const legion_task_pointer_uint32_t &task_pointer)
{
  const legion_task_t task_ = CObjectWrapper::wrap_const(task);
  std::vector<legion_physical_region_t> regions_;
  for (size_t i = 0; i < regions.size(); i++) {
    regions_.push_back(CObjectWrapper::wrap_const(&regions[i]));
  }
  legion_physical_region_t *regions_ptr = NULL;
  if (regions_.size() > 0) {
    regions_ptr = &regions_[0];
  }
  unsigned num_regions = regions_.size();
  legion_context_t ctx_ = CObjectWrapper::wrap(ctx);
  legion_runtime_t runtime_ = CObjectWrapper::wrap(runtime);

  return task_pointer(task_, regions_ptr, num_regions, ctx_, runtime_);
}

legion_task_id_t
legion_runtime_register_task_uint32(
  legion_task_id_t id,
  legion_processor_kind_t proc_kind_,
  bool single,
  bool index,
  legion_variant_id_t vid /* = AUTO_GENERATE_ID */,
  legion_task_config_options_t options_,
  const char *task_name /* = NULL*/,
  legion_task_pointer_uint32_t task_pointer)
{
  Processor::Kind proc_kind = CObjectWrapper::unwrap(proc_kind_);
  TaskConfigOptions options = CObjectWrapper::unwrap(options_);

  return Runtime::register_legion_task<
    uint32_t, legion_task_pointer_uint32_t, task_wrapper_uint32>(
    id, proc_kind, single, index, task_pointer, vid, options, task_name);
}

uint64_t
task_wrapper_uint64(const Task *task,
                    const std::vector<PhysicalRegion> &regions,
                    Context ctx,
                    Runtime *runtime,
                    const legion_task_pointer_uint64_t &task_pointer)
{
  const legion_task_t task_ = CObjectWrapper::wrap_const(task);
  std::vector<legion_physical_region_t> regions_;
  for (size_t i = 0; i < regions.size(); i++) {
    regions_.push_back(CObjectWrapper::wrap_const(&regions[i]));
  }
  legion_physical_region_t *regions_ptr = NULL;
  if (regions_.size() > 0) {
    regions_ptr = &regions_[0];
  }
  unsigned num_regions = regions_.size();
  legion_context_t ctx_ = CObjectWrapper::wrap(ctx);
  legion_runtime_t runtime_ = CObjectWrapper::wrap(runtime);

  return task_pointer(task_, regions_ptr, num_regions, ctx_, runtime_);
}

legion_task_id_t
legion_runtime_register_task_uint64(
  legion_task_id_t id,
  legion_processor_kind_t proc_kind_,
  bool single,
  bool index,
  legion_variant_id_t vid /* = AUTO_GENERATE_ID */,
  legion_task_config_options_t options_,
  const char *task_name /* = NULL*/,
  legion_task_pointer_uint64_t task_pointer)
{
  Processor::Kind proc_kind = CObjectWrapper::unwrap(proc_kind_);
  TaskConfigOptions options = CObjectWrapper::unwrap(options_);

  return Runtime::register_legion_task<
    uint64_t, legion_task_pointer_uint64_t, task_wrapper_uint64>(
    id, proc_kind, single, index, task_pointer, vid, options, task_name);
}

class FunctorWrapper : public ProjectionFunctor {
public:
  FunctorWrapper(Runtime *rt,
                 legion_projection_functor_logical_region_t region_fn,
                 legion_projection_functor_logical_partition_t partition_fn)
    : ProjectionFunctor(rt)
    , region_functor(region_fn)
    , partition_functor(partition_fn)
  {
  }

  LogicalRegion project(Context ctx, Task *task,
                        unsigned index,
                        LogicalRegion upper_bound,
                        const DomainPoint &point)
  {
    legion_runtime_t runtime_ = CObjectWrapper::wrap(runtime);
    legion_context_t ctx_ = CObjectWrapper::wrap(ctx);
    legion_task_t task_ = CObjectWrapper::wrap(task);
    legion_logical_region_t upper_bound_ = CObjectWrapper::wrap(upper_bound);
    legion_domain_point_t point_ = CObjectWrapper::wrap(point);

    assert(region_functor);
    legion_logical_region_t result =
      region_functor(runtime_, ctx_, task_, index, upper_bound_, point_);
    return CObjectWrapper::unwrap(result);
  }

  LogicalRegion project(Context ctx, Task *task,
                        unsigned index,
                        LogicalPartition upper_bound,
                        const DomainPoint &point)
  {
    legion_runtime_t runtime_ = CObjectWrapper::wrap(runtime);
    legion_context_t ctx_ = CObjectWrapper::wrap(ctx);
    legion_task_t task_ = CObjectWrapper::wrap(task);
    legion_logical_partition_t upper_bound_ = CObjectWrapper::wrap(upper_bound);
    legion_domain_point_t point_ = CObjectWrapper::wrap(point);

    assert(partition_functor);
    legion_logical_region_t result =
      partition_functor(runtime_, ctx_, task_, index, upper_bound_, point_);
    return CObjectWrapper::unwrap(result);
  }

private:
  legion_projection_functor_logical_region_t region_functor;
  legion_projection_functor_logical_partition_t partition_functor;
};

void
legion_runtime_register_projection_functor(
  legion_runtime_t runtime_,
  legion_projection_id_t id,
  legion_projection_functor_logical_region_t region_functor,
  legion_projection_functor_logical_partition_t partition_functor)
{
  Runtime *runtime = CObjectWrapper::unwrap(runtime_);

  FunctorWrapper *functor =
    new FunctorWrapper(runtime, region_functor, partition_functor);
  runtime->register_projection_functor(id, functor);
}

// -----------------------------------------------------------------------
// Timing Operations
// -----------------------------------------------------------------------

unsigned long long
legion_get_current_time_in_micros(void)
{
  return Realm::Clock::current_time_in_microseconds();
}

// -----------------------------------------------------------------------
// Machine Operations
// -----------------------------------------------------------------------

legion_machine_t
legion_machine_create()
{
  Machine *result = new Machine(Machine::get_machine());

  return CObjectWrapper::wrap(result);
}

void
legion_machine_destroy(legion_machine_t handle_)
{
  Machine *handle = CObjectWrapper::unwrap(handle_);

  delete handle;
}

void
legion_machine_get_all_processors(
  legion_machine_t machine_,
  legion_processor_t *processors_,
  unsigned processors_size)
{
  Machine *machine = CObjectWrapper::unwrap(machine_);

  std::set<Processor> pset;
  machine->get_all_processors(pset);
  std::set<Processor>::iterator itr = pset.begin();

  unsigned num_to_copy =
    std::min((unsigned)pset.size(), processors_size);

  for (unsigned i = 0; i < num_to_copy; ++i)
    processors_[i] = CObjectWrapper::wrap(*itr++);
}

unsigned
legion_machine_get_all_processors_size(legion_machine_t machine_)
{
  Machine *machine = CObjectWrapper::unwrap(machine_);

  std::set<Processor> pset;
  machine->get_all_processors(pset);
  return pset.size();
}

// -----------------------------------------------------------------------
// Processor Operations
// -----------------------------------------------------------------------

legion_processor_kind_t
legion_processor_kind(legion_processor_t proc_)
{
  Processor proc = CObjectWrapper::unwrap(proc_);

  return CObjectWrapper::wrap(proc.kind());
}

// -----------------------------------------------------------------------
// Memory Operations
// -----------------------------------------------------------------------

legion_memory_kind_t
legion_memory_kind(legion_memory_t mem_)
{
  Memory mem = CObjectWrapper::unwrap(mem_);

  return CObjectWrapper::wrap(mem.kind());
}

// -----------------------------------------------------------------------
// Machine Query Interface Operations
// -----------------------------------------------------------------------


legion_machine_query_interface_t
legion_machine_query_interface_create(legion_machine_t machine_)
{
  Machine *machine = CObjectWrapper::unwrap(machine_);

  return CObjectWrapper::wrap(new MachineQueryInterface(*machine));
}

void
legion_machine_query_interface_destroy(
  legion_machine_query_interface_t handle_)
{
  MachineQueryInterface *handle = CObjectWrapper::unwrap(handle_);
  delete handle;
}

legion_memory_t
legion_machine_query_interface_find_memory_kind(
  legion_machine_query_interface_t handle_,
  legion_processor_t proc_,
  legion_memory_kind_t kind_)
{
  MachineQueryInterface *handle = CObjectWrapper::unwrap(handle_);
  Processor proc = CObjectWrapper::unwrap(proc_);
  Memory::Kind kind = CObjectWrapper::unwrap(kind_);

  return CObjectWrapper::wrap(handle->find_memory_kind(proc, kind));
}

// -----------------------------------------------------------------------
// Default Mapper Operations
// -----------------------------------------------------------------------

bool
legion_default_mapper_map_task(
  legion_default_mapper_t mapper_,
  legion_task_t task_)
{
  DefaultMapper *mapper = CObjectWrapper::unwrap(mapper_);
  Task *task = CObjectWrapper::unwrap(task_);

  return mapper->DefaultMapper::map_task(task);
}

bool
legion_default_mapper_map_inline(
  legion_default_mapper_t mapper_,
  legion_inline_t inline_operation_)
{
  DefaultMapper *mapper = CObjectWrapper::unwrap(mapper_);
  Inline *inline_operation = CObjectWrapper::unwrap(inline_operation_);

  return mapper->DefaultMapper::map_inline(inline_operation);
}
