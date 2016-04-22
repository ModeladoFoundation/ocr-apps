/* Copyright 2015 Stanford University, NVIDIA Corporation
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

// Included from legion.h - do not include this directly

// Useful for IDEs 
#include "legion.h"

namespace LegionRuntime {
  namespace HighLevel {

    /**
     * \struct SerdezRedopFns
     * Small helper class for storing instantiated templates
     */
    struct SerdezRedopFns {
    public:
      SerdezInitFnptr init_fn;
      SerdezFoldFnptr fold_fn;
    };

    /**
     * \class LegionSerialization
     * The Legion serialization class provides template meta-programming
     * help for returning complex data types from task calls.  If the 
     * types have three special methods defined on them then we know
     * how to serialize the type for the runtime rather than just doing
     * a dumb bit copy.  This is especially useful for types which 
     * require deep copies instead of shallow bit copies.  The three
     * methods which must be defined are:
     * size_t legion_buffer_size(void)
     * void legion_serialize(void *buffer)
     * void legion_deserialize(const void *buffer)
     */
    class LegionSerialization {
    public:
      // A helper method for getting access to the runtime's
      // end_task method with private access
      static inline void end_helper(Runtime *rt, Context ctx,
          const void *result, size_t result_size, bool owned)
      {
        rt->end_task(ctx, result, result_size, owned);
      }
      static inline Future from_value_helper(Runtime *rt, 
          const void *value, size_t value_size, bool owned)
      {
        return rt->from_value(value, value_size, owned);
      }

      // WARNING: There are two levels of SFINAE (substitution failure is 
      // not an error) here.  Proceed at your own risk. First we have to 
      // check to see if the type is a struct.  If it is then we check to 
      // see if it has a 'legion_serialize' method.  We assume if there is 
      // a 'legion_serialize' method there are also 'legion_buffer_size'
      // and 'legion_deserialize' methods.
      
      template<typename T, bool HAS_SERIALIZE>
      struct NonPODSerializer {
        static inline void end_task(Runtime *rt, Context ctx, 
                                    T *result)
        {
          size_t buffer_size = result->legion_buffer_size();
          void *buffer = malloc(buffer_size);
          result->legion_serialize(buffer);
          end_helper(rt, ctx, buffer, buffer_size, true/*owned*/);
          // No need to free the buffer, the Legion runtime owns it now
        }
        static inline Future from_value(Runtime *rt, const T *value)
        {
          size_t buffer_size = value->legion_buffer_size();
          void *buffer = malloc(buffer_size);
          value->legion_serialize(buffer);
          return from_value_helper(rt, buffer, buffer_size, true/*owned*/);
        }
        static inline T unpack(const void *result)
        {
          T derez;
          derez.legion_deserialize(result);
          return derez;
        }
      };

      template<typename T>
      struct NonPODSerializer<T,false> {
        static inline void end_task(Runtime *rt, Context ctx, 
                                    T *result)
        {
          end_helper(rt, ctx, (void*)result, sizeof(T), false/*owned*/);
        }
        static inline Future from_value(Runtime *rt, const T *value)
        {
          return from_value_helper(rt, (const void*)value,
                                   sizeof(T), false/*owned*/);
        }
        static inline T unpack(const void *result)
        {
          return (*((const T*)result));
        }
      };

      template<typename T>
      struct HasSerialize {
        typedef char no[1];
        typedef char yes[2];

        struct Fallback { void legion_serialize(void *); };
        struct Derived : T, Fallback { };

        template<typename U, U> struct Check;

        template<typename U>
        static no& test_for_serialize(
                  Check<void (Fallback::*)(void*), &U::legion_serialize> *);

        template<typename U>
        static yes& test_for_serialize(...);

        static const bool value = 
          (sizeof(test_for_serialize<Derived>(0)) == sizeof(yes));
      };

      template<typename T, bool IS_STRUCT>
      struct StructHandler {
        static inline void end_task(Runtime *rt, 
                                    Context ctx, T *result)
        {
          // Otherwise this is a struct, so see if it has serialization methods 
          NonPODSerializer<T,HasSerialize<T>::value>::end_task(rt, ctx, 
                                                               result);
        }
        static inline Future from_value(Runtime *rt, const T *value)
        {
          return NonPODSerializer<T,HasSerialize<T>::value>::from_value(
                                                                  rt, value);
        }
        static inline T unpack(const void *result)
        {
          return NonPODSerializer<T,HasSerialize<T>::value>::unpack(result); 
        }
      };
      // False case of template specialization
      template<typename T>
      struct StructHandler<T,false> {
        static inline void end_task(Runtime *rt, Context ctx, 
                                    T *result)
        {
          end_helper(rt, ctx, (void*)result, sizeof(T), false/*owned*/);
        }
        static inline Future from_value(Runtime *rt, const T *value)
        {
          return from_value_helper(rt, (const void*)value, 
                                   sizeof(T), false/*owned*/);
        }
        static inline T unpack(const void *result)
        {
          return (*((const T*)result));
        }
      };

      template<typename T>
      struct IsAStruct {
        typedef char no[1];
        typedef char yes[2];
        
        template <typename U> static yes& test_for_struct(int U:: *x);
        template <typename U> static no& test_for_struct(...);

        static const bool value = 
                        (sizeof(test_for_struct<T>(0)) == sizeof(yes));
      };

      // Figure out whether this is a struct or not 
      // and call the appropriate Finisher
      template<typename T>
      static inline void end_task(Runtime *rt, Context ctx, T *result)
      {
        StructHandler<T,IsAStruct<T>::value>::end_task(rt, ctx, 
                                                       result); 
      }

      template<typename T>
      static inline Future from_value(Runtime *rt, const T *value)
      {
        return StructHandler<T,IsAStruct<T>::value>::from_value(rt, value);
      }

      template<typename T>
      static inline T unpack(const void *result)
      {
        return StructHandler<T,IsAStruct<T>::value>::unpack(result);
      }

      // Some more help for reduction operations with RHS types
      // that have serialize and deserialize methods

      template<typename REDOP_RHS>
      static void serdez_redop_init(const ReductionOp *reduction_op,
                              void *&ptr, size_t &size)
      {
        REDOP_RHS init_serdez;
        reduction_op->init(&init_serdez, 1);
        size_t new_size = init_serdez.legion_buffer_size();
        if (new_size > size)
        {
          size = new_size;
          ptr = realloc(ptr, size);
        }
        init_serdez.legion_serialize(ptr);
      }

      template<typename REDOP_RHS>
      static void serdez_redop_fold(const ReductionOp *reduction_op,
                                    void *&lhs_ptr, size_t &lhs_size,
                                    const void *rhs_ptr, bool exclusive)
      {
        REDOP_RHS lhs_serdez, rhs_serdez;
        lhs_serdez.legion_deserialize(lhs_ptr);
        rhs_serdez.legion_deserialize(rhs_ptr);
        reduction_op->fold(&lhs_serdez, &rhs_serdez, 1, exclusive); 
        size_t new_size = lhs_serdez.legion_buffer_size();
        // Reallocate the buffer if it has grown
        if (new_size > lhs_size)
        {
          lhs_size = new_size;
          lhs_ptr = realloc(lhs_ptr, lhs_size);
        }
        // Now save the value
        lhs_serdez.legion_serialize(lhs_ptr);
      }

      template<typename REDOP_RHS, bool HAS_SERDEZ>
      struct SerdezRedopHandler {
        static inline void register_reduction(SerdezRedopTable &table,
                                              ReductionOpID redop_id)
        {
          // Do nothing in the case where there are no serdez functions
        }
      };
      // True case of template specialization
      template<typename REDOP_RHS>
      struct SerdezRedopHandler<REDOP_RHS,true> {
        static inline void register_reduction(SerdezRedopTable &table,
                                              ReductionOpID redop_id)
        {
          // Now we can do the registration
          SerdezRedopFns &fns = table[redop_id];
          fns.init_fn = serdez_redop_init<REDOP_RHS>;
          fns.fold_fn = serdez_redop_fold<REDOP_RHS>;
        }
      };

      template<typename REDOP_RHS, bool IS_STRUCT>
      struct StructRedopHandler {
        static inline void register_reduction(SerdezRedopTable &table,
                                              ReductionOpID redop_id)
        {
          // Do nothing in the case where this isn't a struct
        }
      };
      // True case of template specialization
      template<typename REDOP_RHS>
      struct StructRedopHandler<REDOP_RHS,true> {
        static inline void register_reduction(SerdezRedopTable &table,
                                              ReductionOpID redop_id)
        {
          SerdezRedopHandler<REDOP_RHS,HasSerialize<REDOP_RHS>::value>::
            register_reduction(table, redop_id);
        }
      };

      // Register reduction functions if necessary
      template<typename REDOP>
      static inline void register_reduction(SerdezRedopTable &table,
                                            ReductionOpID redop_id)
      {
        StructRedopHandler<typename REDOP::RHS, 
          IsAStruct<typename REDOP::RHS>::value>::register_reduction(table, 
                                                                     redop_id);
      }

    };

    //--------------------------------------------------------------------------
    inline IndexSpace& IndexSpace::operator=(const IndexSpace &rhs)
    //--------------------------------------------------------------------------
    {
      id = rhs.id;
      tid = rhs.tid;
      return *this;
    }

    //--------------------------------------------------------------------------
    inline bool IndexSpace::operator==(const IndexSpace &rhs) const
    //--------------------------------------------------------------------------
    {
      if (id != rhs.id)
        return false;
      if (tid != rhs.tid)
        return false;
      return true;
    }

    //--------------------------------------------------------------------------
    inline bool IndexSpace::operator!=(const IndexSpace &rhs) const
    //--------------------------------------------------------------------------
    {
      if ((id == rhs.id) && (tid == rhs.tid))
        return false;
      return true;
    }

    //--------------------------------------------------------------------------
    inline bool IndexSpace::operator<(const IndexSpace &rhs) const
    //--------------------------------------------------------------------------
    {
      if (id < rhs.id)
        return true;
      if (id > rhs.id)
        return false;
      return (tid < rhs.tid);
    }

    //--------------------------------------------------------------------------
    inline bool IndexSpace::operator>(const IndexSpace &rhs) const
    //--------------------------------------------------------------------------
    {
      if (id > rhs.id)
        return true;
      if (id < rhs.id)
        return false;
      return (tid > rhs.tid);
    }

    //--------------------------------------------------------------------------
    inline IndexPartition& IndexPartition::operator=(const IndexPartition &rhs)
    //--------------------------------------------------------------------------
    {
      id = rhs.id;
      tid = rhs.tid;
      return *this;
    }
    
    //--------------------------------------------------------------------------
    inline bool IndexPartition::operator==(const IndexPartition &rhs) const
    //--------------------------------------------------------------------------
    {
      if (id != rhs.id)
        return false;
      if (tid != rhs.tid)
        return false;
      return true;
    }

    //--------------------------------------------------------------------------
    inline bool IndexPartition::operator!=(const IndexPartition &rhs) const
    //--------------------------------------------------------------------------
    {
      if ((id == rhs.id) && (tid == rhs.tid))
        return false;
      return true;
    }

    //--------------------------------------------------------------------------
    inline bool IndexPartition::operator<(const IndexPartition &rhs) const
    //--------------------------------------------------------------------------
    {
      if (id < rhs.id)
        return true;
      if (id > rhs.id)
        return false;
      return (tid < rhs.tid);
    }

    //--------------------------------------------------------------------------
    inline bool IndexPartition::operator>(const IndexPartition &rhs) const
    //--------------------------------------------------------------------------
    {
      if (id > rhs.id)
        return true;
      if (id < rhs.id)
        return false;
      return (tid > rhs.tid);
    }
    
    //--------------------------------------------------------------------------
    inline FieldSpace& FieldSpace::operator=(const FieldSpace &rhs)
    //--------------------------------------------------------------------------
    {
      id = rhs.id;
      return *this;
    }

    //--------------------------------------------------------------------------
    inline bool FieldSpace::operator==(const FieldSpace &rhs) const
    //--------------------------------------------------------------------------
    {
      return (id == rhs.id);
    }

    //--------------------------------------------------------------------------
    inline bool FieldSpace::operator!=(const FieldSpace &rhs) const
    //--------------------------------------------------------------------------
    {
      return (id != rhs.id);
    }

    //--------------------------------------------------------------------------
    inline bool FieldSpace::operator<(const FieldSpace &rhs) const
    //--------------------------------------------------------------------------
    {
      return (id < rhs.id);
    }

    //--------------------------------------------------------------------------
    inline bool FieldSpace::operator>(const FieldSpace &rhs) const
    //--------------------------------------------------------------------------
    {
      return (id > rhs.id);
    }

    //--------------------------------------------------------------------------
    inline LogicalRegion& LogicalRegion::operator=(const LogicalRegion &rhs) 
    //--------------------------------------------------------------------------
    {
      tree_id = rhs.tree_id;
      index_space = rhs.index_space;
      field_space = rhs.field_space;
      return *this;
    }
    
    //--------------------------------------------------------------------------
    inline bool LogicalRegion::operator==(const LogicalRegion &rhs) const
    //--------------------------------------------------------------------------
    {
      return ((tree_id == rhs.tree_id) && (index_space == rhs.index_space) 
              && (field_space == rhs.field_space));
    }

    //--------------------------------------------------------------------------
    inline bool LogicalRegion::operator!=(const LogicalRegion &rhs) const
    //--------------------------------------------------------------------------
    {
      return (!((*this) == rhs));
    }

    //--------------------------------------------------------------------------
    inline bool LogicalRegion::operator<(const LogicalRegion &rhs) const
    //--------------------------------------------------------------------------
    {
      if (tree_id < rhs.tree_id)
        return true;
      else if (tree_id > rhs.tree_id)
        return false;
      else
      {
        if (index_space < rhs.index_space)
          return true;
        else if (index_space != rhs.index_space) // therefore greater than
          return false;
        else
          return field_space < rhs.field_space;
      }
    }

    //--------------------------------------------------------------------------
    inline LogicalPartition& LogicalPartition::operator=(
                                                    const LogicalPartition &rhs)
    //--------------------------------------------------------------------------
    {
      tree_id = rhs.tree_id;
      index_partition = rhs.index_partition;
      field_space = rhs.field_space;
      return *this;
    }

    //--------------------------------------------------------------------------
    inline bool LogicalPartition::operator==(const LogicalPartition &rhs) const
    //--------------------------------------------------------------------------
    {
      return ((tree_id == rhs.tree_id) && 
              (index_partition == rhs.index_partition) && 
              (field_space == rhs.field_space));
    }

    //--------------------------------------------------------------------------
    inline bool LogicalPartition::operator!=(const LogicalPartition &rhs) const
    //--------------------------------------------------------------------------
    {
      return (!((*this) == rhs));
    }

    //--------------------------------------------------------------------------
    inline bool LogicalPartition::operator<(const LogicalPartition &rhs) const
    //--------------------------------------------------------------------------
    {
      if (tree_id < rhs.tree_id)
        return true;
      else if (tree_id > rhs.tree_id)
        return false;
      else
      {
        if (index_partition < rhs.index_partition)
          return true;
        else if (index_partition > rhs.index_partition)
          return false;
        else
          return (field_space < rhs.field_space);
      }
    }

    //--------------------------------------------------------------------------
    inline bool IndexAllocator::operator==(const IndexAllocator &rhs) const
    //--------------------------------------------------------------------------
    {
      return ((index_space == rhs.index_space) && (allocator == rhs.allocator));
    }

    //--------------------------------------------------------------------------
    inline bool IndexAllocator::operator<(const IndexAllocator &rhs) const
    //--------------------------------------------------------------------------
    {
      if (allocator < rhs.allocator)
        return true;
      else if (allocator > rhs.allocator)
        return false;
      else
        return (index_space < rhs.index_space);
    }

    //--------------------------------------------------------------------------
    inline ptr_t IndexAllocator::alloc(unsigned num_elements /*= 1*/)
    //--------------------------------------------------------------------------
    {
      ptr_t result(allocator->alloc(num_elements));
      return result;
    }

    //--------------------------------------------------------------------------
    inline void IndexAllocator::free(ptr_t ptr, unsigned num_elements /*= 1*/)
    //--------------------------------------------------------------------------
    {
      allocator->free(ptr.value,num_elements);
    }

    //--------------------------------------------------------------------------
    inline bool FieldAllocator::operator==(const FieldAllocator &rhs) const
    //--------------------------------------------------------------------------
    {
      return ((field_space == rhs.field_space) && (runtime == rhs.runtime));
    }

    //--------------------------------------------------------------------------
    inline bool FieldAllocator::operator<(const FieldAllocator &rhs) const
    //--------------------------------------------------------------------------
    {
      if (runtime < rhs.runtime)
        return true;
      else if (runtime > rhs.runtime)
        return false;
      else
        return (field_space < rhs.field_space);
    }

    //--------------------------------------------------------------------------
    inline FieldID FieldAllocator::allocate_field(size_t field_size, 
                                FieldID desired_fieldid /*= AUTO_GENERATE_ID*/)
    //--------------------------------------------------------------------------
    {
      return runtime->allocate_field(parent, field_space, 
                                 field_size, desired_fieldid, false/*local*/); 
    }

    //--------------------------------------------------------------------------
    inline void FieldAllocator::free_field(FieldID id)
    //--------------------------------------------------------------------------
    {
      runtime->free_field(parent, field_space, id);
    }

    //--------------------------------------------------------------------------
    inline FieldID FieldAllocator::allocate_local_field(size_t field_size,
                                FieldID desired_fieldid /*= AUTO_GENERATE_ID*/)
    //--------------------------------------------------------------------------
    {
      return runtime->allocate_field(parent, field_space,
                                field_size, desired_fieldid, true/*local*/);
    }

    //--------------------------------------------------------------------------
    inline void FieldAllocator::allocate_fields(
        const std::vector<size_t> &field_sizes,
        std::vector<FieldID> &resulting_fields)
    //--------------------------------------------------------------------------
    {
      runtime->allocate_fields(parent, field_space, 
                               field_sizes, resulting_fields, false/*local*/);
    }

    //--------------------------------------------------------------------------
    inline void FieldAllocator::free_fields(const std::set<FieldID> &to_free)
    //--------------------------------------------------------------------------
    {
      runtime->free_fields(parent, field_space, to_free);
    }

    //--------------------------------------------------------------------------
    inline void FieldAllocator::allocate_local_fields(
        const std::vector<size_t> &field_sizes,
        std::vector<FieldID> &resulting_fields)
    //--------------------------------------------------------------------------
    {
      runtime->allocate_fields(parent, field_space, 
                               field_sizes, resulting_fields, true/*local*/);
    }

    //--------------------------------------------------------------------------
    template<typename PT, unsigned DIM>
    inline void ArgumentMap::set_point_arg(const PT point[DIM], 
                                           const TaskArgument &arg, 
                                           bool replace/*= false*/)
    //--------------------------------------------------------------------------
    {
      LEGION_STATIC_ASSERT(DIM <= DomainPoint::MAX_POINT_DIM);  
      DomainPoint dp;
      dp.dim = DIM;
      for (unsigned idx = 0; idx < DIM; idx++)
        dp.point_data[idx] = point[idx];
      set_point(dp, arg, replace);
    }

    //--------------------------------------------------------------------------
    template<typename PT, unsigned DIM>
    inline bool ArgumentMap::remove_point(const PT point[DIM])
    //--------------------------------------------------------------------------
    {
      LEGION_STATIC_ASSERT(DIM <= DomainPoint::MAX_POINT_DIM);
      DomainPoint dp;
      dp.dim = DIM;
      for (unsigned idx = 0; idx < DIM; idx++)
        dp.point_data[idx] = point[idx];
      return remove_point(dp);
    }

    //--------------------------------------------------------------------------
    inline bool Predicate::operator==(const Predicate &p) const
    //--------------------------------------------------------------------------
    {
      if (impl == NULL)
      {
        if (p.impl == NULL)
          return (const_value == p.const_value);
        else
          return false;
      }
      else
        return (impl == p.impl);
    }

    //--------------------------------------------------------------------------
    inline bool Predicate::operator<(const Predicate &p) const
    //--------------------------------------------------------------------------
    {
      if (impl == NULL)
      {
        if (p.impl == NULL)
          return (const_value < p.const_value);
        else
          return true;
      }
      else
        return (impl < p.impl);
    }

    //--------------------------------------------------------------------------
    inline bool Predicate::operator!=(const Predicate &p) const
    //--------------------------------------------------------------------------
    {
      return !(*this == p);
    }

    //--------------------------------------------------------------------------
    inline RegionFlags operator~(RegionFlags f)
    //--------------------------------------------------------------------------
    {
      return static_cast<RegionFlags>(~unsigned(f));
    }

    //--------------------------------------------------------------------------
    inline RegionFlags operator|(RegionFlags left, RegionFlags right)
    //--------------------------------------------------------------------------
    {
      return static_cast<RegionFlags>(unsigned(left) | unsigned(right));
    }

    //--------------------------------------------------------------------------
    inline RegionFlags operator&(RegionFlags left, RegionFlags right)
    //--------------------------------------------------------------------------
    {
      return static_cast<RegionFlags>(unsigned(left) & unsigned(right));
    }

    //--------------------------------------------------------------------------
    inline RegionFlags operator^(RegionFlags left, RegionFlags right)
    //--------------------------------------------------------------------------
    {
      return static_cast<RegionFlags>(unsigned(left) ^ unsigned(right));
    }

    //--------------------------------------------------------------------------
    inline RegionFlags operator|=(RegionFlags &left, RegionFlags right)
    //--------------------------------------------------------------------------
    {
      unsigned l = static_cast<unsigned>(left);
      unsigned r = static_cast<unsigned>(right);
      l |= r;
      return left = static_cast<RegionFlags>(l);
    }

    //--------------------------------------------------------------------------
    inline RegionFlags operator&=(RegionFlags &left, RegionFlags right)
    //--------------------------------------------------------------------------
    {
      unsigned l = static_cast<unsigned>(left);
      unsigned r = static_cast<unsigned>(right);
      l &= r;
      return left = static_cast<RegionFlags>(l);
    }

    //--------------------------------------------------------------------------
    inline RegionFlags operator^=(RegionFlags &left, RegionFlags right)
    //--------------------------------------------------------------------------
    {
      unsigned l = static_cast<unsigned>(left);
      unsigned r = static_cast<unsigned>(right);
      l ^= r;
      return left = static_cast<RegionFlags>(l);
    }

    //--------------------------------------------------------------------------
    inline RegionRequirement& RegionRequirement::add_field(FieldID fid, 
                                             bool instance/*= true*/)
    //--------------------------------------------------------------------------
    {
      privilege_fields.insert(fid);
      if (instance)
        instance_fields.push_back(fid);
      return *this;
    }

    //--------------------------------------------------------------------------
    inline RegionRequirement& RegionRequirement::add_fields(
                      const std::vector<FieldID>& fids, bool instance/*= true*/)
    //--------------------------------------------------------------------------
    {
      privilege_fields.insert(fids.begin(), fids.end());
      if (instance)
        instance_fields.insert(instance_fields.end(), fids.begin(), fids.end());
      return *this;
    }

    //--------------------------------------------------------------------------
    inline RegionRequirement& RegionRequirement::add_flags(RegionFlags new_flags)
    //--------------------------------------------------------------------------
    {
      flags |= new_flags;
      return *this;
    }

    //--------------------------------------------------------------------------
    inline IndexSpaceRequirement& TaskLauncher::add_index_requirement(
                                              const IndexSpaceRequirement &req)
    //--------------------------------------------------------------------------
    {
      index_requirements.push_back(req);
      return index_requirements.back();
    }

    //--------------------------------------------------------------------------
    inline RegionRequirement& TaskLauncher::add_region_requirement(
                                                  const RegionRequirement &req)
    //--------------------------------------------------------------------------
    {
      region_requirements.push_back(req);
      return region_requirements.back();
    }

    //--------------------------------------------------------------------------
    inline void TaskLauncher::add_field(unsigned idx, FieldID fid, bool inst)
    //--------------------------------------------------------------------------
    {
#ifdef DEBUG_HIGH_LEVEL
      assert(idx < region_requirements.size());
#endif
      region_requirements[idx].add_field(fid, inst);
    }

    //--------------------------------------------------------------------------
    inline void TaskLauncher::add_future(Future f)
    //--------------------------------------------------------------------------
    {
      futures.push_back(f);
    }

    //--------------------------------------------------------------------------
    inline void TaskLauncher::add_grant(Grant g)
    //--------------------------------------------------------------------------
    {
      grants.push_back(g);
    }

    //--------------------------------------------------------------------------
    inline void TaskLauncher::add_wait_barrier(PhaseBarrier bar)
    //--------------------------------------------------------------------------
    {
      wait_barriers.push_back(bar);
    }

    //--------------------------------------------------------------------------
    inline void TaskLauncher::add_arrival_barrier(PhaseBarrier bar)
    //--------------------------------------------------------------------------
    {
      arrive_barriers.push_back(bar);
    }

    //--------------------------------------------------------------------------
    inline void TaskLauncher::set_predicate_false_future(Future f)
    //--------------------------------------------------------------------------
    {
      predicate_false_future = f;
    }

    //--------------------------------------------------------------------------
    inline void TaskLauncher::set_predicate_false_result(TaskArgument arg)
    //--------------------------------------------------------------------------
    {
      predicate_false_result = arg;
    }

    //--------------------------------------------------------------------------
    inline IndexSpaceRequirement& IndexLauncher::add_index_requirement(
                                              const IndexSpaceRequirement &req)
    //--------------------------------------------------------------------------
    {
      index_requirements.push_back(req);
      return index_requirements.back();
    }

    //--------------------------------------------------------------------------
    inline RegionRequirement& IndexLauncher::add_region_requirement(
                                                  const RegionRequirement &req)
    //--------------------------------------------------------------------------
    {
      region_requirements.push_back(req);
      return region_requirements.back();
    }

    //--------------------------------------------------------------------------
    inline void IndexLauncher::add_field(unsigned idx, FieldID fid, bool inst)
    //--------------------------------------------------------------------------
    {
#ifdef DEBUG_HIGH_LEVEL
      assert(idx < region_requirements.size());
#endif
      region_requirements[idx].add_field(fid, inst);
    }

    //--------------------------------------------------------------------------
    inline void IndexLauncher::add_future(Future f)
    //--------------------------------------------------------------------------
    {
      futures.push_back(f);
    }

    //--------------------------------------------------------------------------
    inline void IndexLauncher::add_grant(Grant g)
    //--------------------------------------------------------------------------
    {
      grants.push_back(g);
    }

    //--------------------------------------------------------------------------
    inline void IndexLauncher::add_wait_barrier(PhaseBarrier bar)
    //--------------------------------------------------------------------------
    {
      wait_barriers.push_back(bar);
    }

    //--------------------------------------------------------------------------
    inline void IndexLauncher::add_arrival_barrier(PhaseBarrier bar)
    //--------------------------------------------------------------------------
    {
      arrive_barriers.push_back(bar);
    }

    //--------------------------------------------------------------------------
    inline void IndexLauncher::set_predicate_false_future(Future f)
    //--------------------------------------------------------------------------
    {
      predicate_false_future = f;
    }

    //--------------------------------------------------------------------------
    inline void IndexLauncher::set_predicate_false_result(TaskArgument arg)
    //--------------------------------------------------------------------------
    {
      predicate_false_result = arg;
    }

    //--------------------------------------------------------------------------
    inline void InlineLauncher::add_field(FieldID fid, bool inst)
    //--------------------------------------------------------------------------
    {
      requirement.add_field(fid, inst);
    }

    //--------------------------------------------------------------------------
    inline unsigned CopyLauncher::add_copy_requirements(
                     const RegionRequirement &src, const RegionRequirement &dst)
    //--------------------------------------------------------------------------
    {
      unsigned result = src_requirements.size();
#ifdef DEBUG_HIGH_LEVEL
      assert(result == dst_requirements.size());
#endif
      src_requirements.push_back(src);
      dst_requirements.push_back(dst);
      return result;
    }

    //--------------------------------------------------------------------------
    inline void CopyLauncher::add_src_field(unsigned idx,FieldID fid,bool inst)
    //--------------------------------------------------------------------------
    {
#ifdef DEBUG_HIGH_LEVEL
      assert(idx < src_requirements.size());
#endif
      src_requirements[idx].add_field(fid, inst);
    }

    //--------------------------------------------------------------------------
    inline void CopyLauncher::add_dst_field(unsigned idx,FieldID fid,bool inst)
    //--------------------------------------------------------------------------
    {
#ifdef DEBUG_HIGH_LEVEL
      assert(idx < dst_requirements.size());
#endif
      dst_requirements[idx].add_field(fid, inst);
    }

    //--------------------------------------------------------------------------
    inline void CopyLauncher::add_grant(Grant g)
    //--------------------------------------------------------------------------
    {
      grants.push_back(g);
    }

    //--------------------------------------------------------------------------
    inline void CopyLauncher::add_wait_barrier(PhaseBarrier bar)
    //--------------------------------------------------------------------------
    {
      wait_barriers.push_back(bar);
    }

    //--------------------------------------------------------------------------
    inline void CopyLauncher::add_arrival_barrier(PhaseBarrier bar)
    //--------------------------------------------------------------------------
    {
      arrive_barriers.push_back(bar);
    }

    //--------------------------------------------------------------------------
    inline void AcquireLauncher::add_field(FieldID f)
    //--------------------------------------------------------------------------
    {
      fields.insert(f);
    }

    //--------------------------------------------------------------------------
    inline void AcquireLauncher::add_grant(Grant g)
    //--------------------------------------------------------------------------
    {
      grants.push_back(g);
    }

    //--------------------------------------------------------------------------
    inline void AcquireLauncher::add_wait_barrier(PhaseBarrier bar)
    //--------------------------------------------------------------------------
    {
      wait_barriers.push_back(bar);
    }

    //--------------------------------------------------------------------------
    inline void AcquireLauncher::add_arrival_barrier(PhaseBarrier bar)
    //--------------------------------------------------------------------------
    {
      arrive_barriers.push_back(bar);
    }

    //--------------------------------------------------------------------------
    inline void ReleaseLauncher::add_field(FieldID f)
    //--------------------------------------------------------------------------
    {
      fields.insert(f);
    }

    //--------------------------------------------------------------------------
    inline void ReleaseLauncher::add_grant(Grant g)
    //--------------------------------------------------------------------------
    {
      grants.push_back(g);
    }

    //--------------------------------------------------------------------------
    inline void ReleaseLauncher::add_wait_barrier(PhaseBarrier bar)
    //--------------------------------------------------------------------------
    {
      wait_barriers.push_back(bar);
    }

    //--------------------------------------------------------------------------
    inline void ReleaseLauncher::add_arrival_barrier(PhaseBarrier bar)
    //--------------------------------------------------------------------------
    {
      arrive_barriers.push_back(bar);
    }

    //--------------------------------------------------------------------------
    inline void MustEpochLauncher::add_single_task(const DomainPoint &point,
                                                   const TaskLauncher &launcher)
    //--------------------------------------------------------------------------
    {
      single_tasks.push_back(launcher);
      single_tasks.back().point = point;
    }

    //--------------------------------------------------------------------------
    inline void MustEpochLauncher::add_index_task(const IndexLauncher &launcher)
    //--------------------------------------------------------------------------
    {
      index_tasks.push_back(launcher);
    }

    //--------------------------------------------------------------------------
    template<typename T>
    inline T Future::get_result(void)
    //--------------------------------------------------------------------------
    {
      // Unpack the value using LegionSerialization in case
      // the type has an alternative method of unpacking
      return LegionSerialization::unpack<T>(get_untyped_result());
    }

    //--------------------------------------------------------------------------
    template<typename T>
    inline const T& Future::get_reference(void)
    //--------------------------------------------------------------------------
    {
      return *((const T*)get_untyped_result());
    }

    //--------------------------------------------------------------------------
    inline const void* Future::get_untyped_pointer(void)
    //--------------------------------------------------------------------------
    {
      return get_untyped_result();
    }

    //--------------------------------------------------------------------------
    template<typename T>
    /*static*/ inline Future Future::from_value(Runtime *rt, const T &value)
    //--------------------------------------------------------------------------
    {
      return LegionSerialization::from_value(rt, &value);
    }

    //--------------------------------------------------------------------------
    template<typename T>
    inline T FutureMap::get_result(const DomainPoint &dp)
    //--------------------------------------------------------------------------
    {
      Future f = get_future(dp);
      return f.get_result<T>();
    }

    //--------------------------------------------------------------------------
    template<typename RT, typename PT, unsigned DIM>
    inline RT FutureMap::get_result(const PT point[DIM])
    //--------------------------------------------------------------------------
    {
      LEGION_STATIC_ASSERT(DIM <= DomainPoint::MAX_POINT_DIM);
      DomainPoint dp;
      dp.dim = DIM;
      for (unsigned idx = 0; idx < DIM; idx++)
        dp.point_data[idx] = point[idx];
      Future f = get_future(dp);
      return f.get_result<RT>();
    }

    //--------------------------------------------------------------------------
    template<typename PT, unsigned DIM>
    inline Future FutureMap::get_future(const PT point[DIM])
    //--------------------------------------------------------------------------
    {
      LEGION_STATIC_ASSERT(DIM <= DomainPoint::MAX_POINT_DIM);
      DomainPoint dp;
      dp.dim = DIM;
      for (unsigned idx = 0; idx < DIM; idx++)
        dp.point_data[idx] = point[idx];
      return get_future(dp);
    }

    //--------------------------------------------------------------------------
    template<typename PT, unsigned DIM>
    inline void FutureMap::get_void_result(const PT point[DIM])
    //--------------------------------------------------------------------------
    {
      LEGION_STATIC_ASSERT(DIM <= DomainPoint::MAX_POINT_DIM);
      DomainPoint dp;
      dp.dim = DIM;
      for (unsigned idx = 0; idx < DIM; idx++)
        dp.point_data[idx] = point[idx];
      Future f = get_future(dp);
      return f.get_void_result();
    }

    //--------------------------------------------------------------------------
    inline bool PhysicalRegion::is_mapped(void) const
    //--------------------------------------------------------------------------
    {
      return (impl != NULL);
    }

    //--------------------------------------------------------------------------
    inline bool IndexIterator::has_next(void) const
    //--------------------------------------------------------------------------
    {
      return (!finished);
    }
    
    //--------------------------------------------------------------------------
    inline ptr_t IndexIterator::next(void)
    //--------------------------------------------------------------------------
    {
#ifdef DEBUG_HIGH_LEVEL
      assert(!finished);
#endif
      ptr_t result = current_pointer;
      remaining_elmts--;
      if (remaining_elmts > 0)
      {
        current_pointer++;
      }
      else
      {
        finished = !(enumerator->get_next(current_pointer, remaining_elmts));
      }
      return result;
    }

    //--------------------------------------------------------------------------
    inline ptr_t IndexIterator::next_span(size_t& act_count, size_t req_count)
    //--------------------------------------------------------------------------
    {
#ifdef DEBUG_HIGH_LEVEL
      assert(!finished);
#endif
      ptr_t result = current_pointer;
      // did we consume the entire span from the enumerator?
      if ((size_t)remaining_elmts <= req_count)
      {
	// yes, limit the actual count to what we had, and get the next span
	act_count = remaining_elmts;
	current_pointer += remaining_elmts;
        finished = !(enumerator->get_next(current_pointer, remaining_elmts));
      }
      else
      {
	// no, just return what was requested
	act_count = req_count;
	current_pointer += req_count;
      }
      return result;
    }

    //--------------------------------------------------------------------------
    inline UniqueID Task::get_unique_task_id(void) const
    //--------------------------------------------------------------------------
    {
      return get_unique_mappable_id();
    }

    //--------------------------------------------------------------------------
    inline UniqueID Copy::get_unique_copy_id(void) const
    //--------------------------------------------------------------------------
    {
      return get_unique_mappable_id();
    }

    //--------------------------------------------------------------------------
    inline UniqueID Inline::get_unique_inline_id(void) const
    //--------------------------------------------------------------------------
    {
      return get_unique_mappable_id();
    }

    //--------------------------------------------------------------------------
    inline UniqueID Acquire::get_unique_acquire_id(void) const
    //--------------------------------------------------------------------------
    {
      return get_unique_mappable_id();
    }

    //--------------------------------------------------------------------------
    inline UniqueID Release::get_unique_release_id(void) const
    //--------------------------------------------------------------------------
    {
      return get_unique_mappable_id();
    }

    //--------------------------------------------------------------------------
    template<typename T>
    IndexPartition Runtime::create_index_partition(Context ctx,
        IndexSpace parent, const T& mapping, int part_color /*= AUTO_GENERATE*/)
    //--------------------------------------------------------------------------
    {
      Arrays::Rect<T::IDIM> parent_rect = 
        get_index_space_domain(ctx, parent).get_rect<T::IDIM>();
      Arrays::Rect<T::ODIM> color_space = mapping.image_convex(parent_rect);
      DomainPointColoring c;
      for (typename T::PointInOutputRectIterator pir(color_space); 
          pir; pir++) 
      {
        Arrays::Rect<T::IDIM> preimage = mapping.preimage(pir.p);
#ifdef DEBUG_HIGH_LEVEL
        assert(mapping.preimage_is_dense(pir.p));
#endif
        c[DomainPoint::from_point<T::IDIM>(pir.p)] =
          Domain::from_rect<T::IDIM>(preimage);
      }
      IndexPartition result = create_index_partition(ctx, parent, 
              Domain::from_rect<T::ODIM>(color_space), c, 
              DISJOINT_KIND, part_color);
#ifdef DEBUG_HIGH_LEVEL
      // We don't actually know if we're supposed to check disjointness
      // so if we're in debug mode then just do it.
      {
        std::set<DomainPoint> current_colors;  
        for (DomainPointColoring::const_iterator it1 = c.begin();
              it1 != c.end(); it1++)
        {
          current_colors.insert(it1->first);
          for (DomainPointColoring::const_iterator it2 = c.begin();
                it2 != c.end(); it2++)
          {
            if (current_colors.find(it2->first) != current_colors.end())
              continue;
            Arrays::Rect<T::IDIM> rect1 = it1->second.get_rect<T::IDIM>();
            Arrays::Rect<T::IDIM> rect2 = it2->second.get_rect<T::IDIM>();
            if (rect1.overlaps(rect2))
            {
              switch (it1->first.dim)
              {
                case 1:
                  fprintf(stderr, "ERROR: colors %d and %d of partition %d are "
                                  "not disjoint rectangles as they should be!",
                                   (it1->first)[0], (it2->first)[0], result.id);
                  break;
                case 2:
                  fprintf(stderr, "ERROR: colors (%d, %d) and (%d, %d) of "
                                  "partition %d are not disjoint rectangles "
                                  "as they should be!",
                                  (it1->first)[0], (it1->first)[1],
                                  (it2->first)[0], (it2->first)[1],
                                  result.id);
                  break;
                case 3:
                  fprintf(stderr, "ERROR: colors (%d, %d, %d) and (%d, %d, %d) "
                                  "of partition %d are not disjoint rectangles "
                                  "as they should be!",
                                  (it1->first)[0], (it1->first)[1],
                                  (it1->first)[2], (it2->first)[0],
                                  (it2->first)[1], (it2->first)[2],
                                  result.id);
                  break;
                default:
                  assert(false);
              }
              assert(false);
              exit(ERROR_DISJOINTNESS_TEST_FAILURE);
            }
          }
        }
      }
#endif
      return result;
    }

    //--------------------------------------------------------------------------
    template<unsigned DIM>
    IndexSpace Runtime::get_index_subspace(Context ctx, 
                              IndexPartition p, Arrays::Point<DIM> color_point)
    //--------------------------------------------------------------------------
    {
      DomainPoint dom_point = DomainPoint::from_point<DIM>(color_point);
      return get_index_subspace(ctx, p, dom_point);
    }

    //--------------------------------------------------------------------------
    template<typename T>
    void Runtime::fill_field(Context ctx, LogicalRegion handle,
                                      LogicalRegion parent, FieldID fid,
                                      const T &value, Predicate pred)
    //--------------------------------------------------------------------------
    {
      fill_field(ctx, handle, parent, fid, &value, sizeof(T), pred);
    }

    //--------------------------------------------------------------------------
    template<typename T>
    void Runtime::fill_fields(Context ctx, LogicalRegion handle,
                                       LogicalRegion parent, 
                                       const std::set<FieldID> &fields,
                                       const T &value, Predicate pred)
    //--------------------------------------------------------------------------
    {
      fill_fields(ctx, handle, parent, fields, &value, sizeof(T), pred);
    }

    //--------------------------------------------------------------------------
    template<typename REDOP>
    /*static*/ void Runtime::register_reduction_op(ReductionOpID redop_id)
    //--------------------------------------------------------------------------
    {
      if (redop_id == 0)
      {
        fprintf(stderr,"ERROR: ReductionOpID zero is reserved.\n");
#ifdef DEBUG_HIGH_LEVEL
        assert(false);
#endif
        exit(ERROR_RESERVED_REDOP_ID);
      }
      ReductionOpTable &red_table = Runtime::get_reduction_table(); 
      // Check to make sure we're not overwriting a prior reduction op 
      if (red_table.find(redop_id) != red_table.end())
      {
        fprintf(stderr,"ERROR: ReductionOpID %d has already been used " 
                       "in the reduction table\n",redop_id);
#ifdef DEBUG_HIGH_LEVEL
        assert(false);
#endif
        exit(ERROR_DUPLICATE_REDOP_ID);
      }
      red_table[redop_id] = 
        LowLevel::ReductionOpUntyped::create_reduction_op<REDOP>(); 
      // We also have to check to see if there are explicit serialization
      // and deserialization methods on the RHS type for doing fold reductions
      SerdezRedopTable &serdez_red_table = Runtime::get_serdez_redop_table();
      LegionSerialization::register_reduction<REDOP>(serdez_red_table,redop_id);
    }

    //--------------------------------------------------------------------------
    template<LogicalRegion (*PROJ_PTR)(LogicalRegion, const DomainPoint&,
                                       Runtime*)>
    /*static*/ ProjectionID Runtime::register_region_function(
                                                            ProjectionID handle)
    //--------------------------------------------------------------------------
    {
      return Runtime::register_region_projection_function(
                                  handle, reinterpret_cast<void *>(PROJ_PTR));
    }

    //--------------------------------------------------------------------------
    template<LogicalRegion (*PROJ_PTR)(LogicalPartition, const DomainPoint&,
                                       Runtime*)>
    /*static*/ ProjectionID Runtime::register_partition_function(
                                                    ProjectionID handle)
    //--------------------------------------------------------------------------
    {
      return Runtime::register_partition_projection_function(
                                  handle, reinterpret_cast<void *>(PROJ_PTR));
    }

    //--------------------------------------------------------------------------
    template<unsigned DIM>
    IndexSpace Mapper::get_index_subspace(IndexPartition p,
                                          Arrays::Point<DIM> &color_point) const
    //--------------------------------------------------------------------------
    {
      DomainPoint dom_point = DomainPoint::from_point<DIM>(color_point);
      return get_index_subspace(p, dom_point);
    }
    
    //--------------------------------------------------------------------------
    // Wrapper functions for high-level tasks
    //--------------------------------------------------------------------------

    /**
     * \class LegionTaskWrapper
     * This is a helper class that has static template methods for 
     * wrapping Legion application tasks.  For all tasks we can make
     * wrappers both for normal execution and also for inline execution.
     */
    class LegionTaskWrapper {
    public: 
      // Non-void return type for new legion task types
      template<typename T,
        T (*TASK_PTR)(const Task*, const std::vector<PhysicalRegion>&,
                      Context, Runtime*)>
      static void legion_task_wrapper(const void*, size_t, 
                                      const void*, size_t, Processor);
      template<typename T, typename UDT,
        T (*TASK_PTR)(const Task*, const std::vector<PhysicalRegion>&,
                      Context, Runtime*, const UDT&)>
      static void legion_udt_task_wrapper(const void*, size_t, 
                                          const void*, size_t, Processor);
    public:
      // Void return type for new legion task types
      template<
        void (*TASK_PTR)(const Task*, const std::vector<PhysicalRegion>&,
                         Context, Runtime*)>
      static void legion_task_wrapper(const void*, size_t, 
                                      const void*, size_t, Processor);
      template<typename UDT,
        void (*TASK_PTR)(const Task*, const std::vector<PhysicalRegion>&,
                         Context, Runtime*, const UDT&)>
      static void legion_udt_task_wrapper(const void*, size_t, 
                                          const void*, size_t, Processor);
    public:
      // Non-void single task wrapper
      template<typename T,
      T (*TASK_PTR)(const void*,size_t,const std::vector<RegionRequirement>&,
                  const std::vector<PhysicalRegion>&,Context,Runtime*)>
      static void high_level_task_wrapper(const void*, size_t, 
                                          const void*, size_t, Processor);
    public:
      // Void single task wrapper
      template<
      void (*TASK_PTR)(const void*,size_t,const std::vector<RegionRequirement>&,
                 const std::vector<PhysicalRegion>&,Context,Runtime*)>
      static void high_level_task_wrapper(const void*, size_t, 
                                          const void*, size_t, Processor);
    public:
      // Non-void index task wrapper
      template<typename RT,
      RT (*TASK_PTR)(const void*,size_t,const void*,size_t,const DomainPoint&,
                 const std::vector<RegionRequirement>&,
                 const std::vector<PhysicalRegion>&,Context,Runtime*)>
      static void high_level_index_task_wrapper(const void*,size_t,
                                                const void*,size_t,Processor);
    public:
      // Void index task wrapper
      template< 
      void (*TASK_PTR)(const void*,size_t,const void*,size_t,const DomainPoint&, 
                 const std::vector<RegionRequirement>&,
                 const std::vector<PhysicalRegion>&,Context,Runtime*)>
      static void high_level_index_task_wrapper(const void*,size_t,
                                                const void*,size_t,Processor);
    public: // INLINE VERSIONS OF THE ABOVE METHODS
      template<typename T,
        T (*TASK_PTR)(const Task*, const std::vector<PhysicalRegion>&,
                      Context, Runtime*)>
      static void inline_task_wrapper(const Task*, 
          const std::vector<PhysicalRegion>&, Context, Runtime*,
          void*&, size_t&);
      template<typename T, typename UDT,
        T (*TASK_PTR)(const Task*, const std::vector<PhysicalRegion>&,
                      Context, Runtime*, const UDT&)>
      static void inline_udt_task_wrapper(const Task*,
          const std::vector<PhysicalRegion>&, Context, Runtime*,
          void*&, size_t&);
    public:
      template<
        void (*TASK_PTR)(const Task*, const std::vector<PhysicalRegion>&,
                         Context, Runtime*)>
      static void inline_task_wrapper(const Task*,
          const std::vector<PhysicalRegion>&, Context, Runtime*,
          void*&, size_t&);
      template<typename UDT,
        void (*TASK_PTR)(const Task*, const std::vector<PhysicalRegion>&,
                         Context, Runtime*, const UDT&)>
      static void inline_udt_task_wrapper(const Task*,
          const std::vector<PhysicalRegion>&, Context, Runtime*,
          void*&, size_t&);
    public:
      template<typename T,
      T (*TASK_PTR)(const void*,size_t,const std::vector<RegionRequirement>&,
                  const std::vector<PhysicalRegion>&,Context,Runtime*)>
      static void high_level_inline_task_wrapper(const Task*,
          const std::vector<PhysicalRegion>&, Context, Runtime*,
          void*&, size_t&);
    public:
      template<
      void (*TASK_PTR)(const void*,size_t,const std::vector<RegionRequirement>&,
                 const std::vector<PhysicalRegion>&,Context,Runtime*)>
      static void high_level_inline_task_wrapper(const Task*,
          const std::vector<PhysicalRegion>&, Context, Runtime*,
          void*&, size_t&);
    public:
      template<typename RT,
      RT (*TASK_PTR)(const void*,size_t,const void*,size_t,const DomainPoint&,
                 const std::vector<RegionRequirement>&,
                 const std::vector<PhysicalRegion>&,Context,Runtime*)>
      static void high_level_inline_index_task_wrapper(const Task*,
          const std::vector<PhysicalRegion>&, Context, Runtime*,
          void*&, size_t&);
    public:
      template< 
      void (*TASK_PTR)(const void*,size_t,const void*,size_t,const DomainPoint&, 
                 const std::vector<RegionRequirement>&,
                 const std::vector<PhysicalRegion>&,Context,Runtime*)>
      static void high_level_inline_index_task_wrapper(const Task*,
          const std::vector<PhysicalRegion>&, Context, Runtime*,
          void*&, size_t&);
    };
    
    //--------------------------------------------------------------------------
    template<typename T,
      T (*TASK_PTR)(const Task*, const std::vector<PhysicalRegion>&,
                    Context, Runtime*)>
    void LegionTaskWrapper::legion_task_wrapper(const void *args, 
                                                size_t arglen, 
                                                const void *userdata,
                                                size_t userlen,
                                                Processor p)
    //--------------------------------------------------------------------------
    {
      // Assert that we are returning Futures or FutureMaps
      LEGION_STATIC_ASSERT((LegionTypeInequality<T,Future>::value));
      LEGION_STATIC_ASSERT((LegionTypeInequality<T,FutureMap>::value));
      // Assert that the return type size is within the required size
      LEGION_STATIC_ASSERT(sizeof(T) <= MAX_RETURN_SIZE);
      // Get the high level runtime
      Runtime *runtime = Runtime::get_runtime(p);

      // Read the context out of the buffer
      Context ctx = *((const Context*)args);
#ifdef DEBUG_HIGH_LEVEL
      assert(arglen == sizeof(Context));
#endif
      const std::vector<PhysicalRegion> &regions = runtime->begin_task(ctx);

      // Invoke the task with the given context
      T return_value = 
        (*TASK_PTR)(reinterpret_cast<Task*>(ctx),regions,ctx,runtime);

      // Send the return value back
      LegionSerialization::end_task<T>(runtime, ctx, &return_value);
    }

    //--------------------------------------------------------------------------
    template<
      void (*TASK_PTR)(const Task*, const std::vector<PhysicalRegion>&,
                       Context, Runtime*)>
    void LegionTaskWrapper::legion_task_wrapper(const void *args, 
                                                size_t arglen, 
                                                const void *userdata,
                                                size_t userlen,
                                                Processor p)
    //--------------------------------------------------------------------------
    {
      // Get the high level runtime
      Runtime *runtime = Runtime::get_runtime(p);

      // Read the context out of the buffer
      Context ctx = *((const Context*)args);
#ifdef DEBUG_HIGH_LEVEL
      assert(arglen == sizeof(Context));
#endif
      const std::vector<PhysicalRegion> &regions = runtime->begin_task(ctx);

      (*TASK_PTR)(reinterpret_cast<Task*>(ctx), regions, ctx, runtime);

      // Send an empty return value back
      runtime->end_task(ctx, NULL, 0);
    }

    //--------------------------------------------------------------------------
    template<typename T, typename UDT,
      T (*TASK_PTR)(const Task*, const std::vector<PhysicalRegion>&,
                    Context, Runtime*, const UDT&)>
    void LegionTaskWrapper::legion_udt_task_wrapper(const void *args,
                                                    size_t arglen, 
                                                    const void *userdata,
                                                    size_t userlen,
                                                    Processor p)
    //--------------------------------------------------------------------------
    {
      // Assert that we are returning Futures or FutureMaps
      LEGION_STATIC_ASSERT((LegionTypeInequality<T,Future>::value));
      LEGION_STATIC_ASSERT((LegionTypeInequality<T,FutureMap>::value));
      // Assert that the return type size is within the required size
      LEGION_STATIC_ASSERT(sizeof(T) <= MAX_RETURN_SIZE);
      // Get the high level runtime
      Runtime *runtime = Runtime::get_runtime(p);

      // Read the context out of the buffer
      Context ctx = *((const Context*)args);
#ifdef DEBUG_HIGH_LEVEL
      assert(arglen == sizeof(Context));
#endif
      Task *task = reinterpret_cast<Task*>(ctx);
      const UDT *user_data = (const UDT *)Runtime::find_user_data(
                                     task->task_id, task->selected_variant);

      const std::vector<PhysicalRegion> &regions = runtime->begin_task(ctx);

      // Invoke the task with the given context
      T return_value = (*TASK_PTR)(task, regions, ctx, runtime, *user_data);

      // Send the return value back
      LegionSerialization::end_task<T>(runtime, ctx, &return_value);
    }

    //--------------------------------------------------------------------------
    template<typename UDT,
      void (*TASK_PTR)(const Task*, const std::vector<PhysicalRegion>&,
                       Context, Runtime*, const UDT&)>
    void LegionTaskWrapper::legion_udt_task_wrapper(const void *args,
                                                    size_t arglen, 
                                                    const void *userdata,
                                                    size_t userlen,
                                                    Processor p)
    //--------------------------------------------------------------------------
    {
      // Get the high level runtime
      Runtime *runtime = Runtime::get_runtime(p);

      // Read the context out of the buffer
      Context ctx = *((const Context*)args);
#ifdef DEBUG_HIGH_LEVEL
      assert(arglen == sizeof(Context));
#endif
      Task *task = reinterpret_cast<Task*>(ctx);
      const UDT *user_data = (const UDT *)Runtime::find_user_data(
                                     task->task_id, task->selected_variant);

      const std::vector<PhysicalRegion> &regions = runtime->begin_task(ctx);

      (*TASK_PTR)(task, regions, ctx, runtime, *user_data);

      // Send an empty return value back
      runtime->end_task(ctx, NULL, 0);
    }

    //--------------------------------------------------------------------------
    template<typename T,
      T (*TASK_PTR)(const void*,size_t,const std::vector<RegionRequirement>&,
                  const std::vector<PhysicalRegion>&,Context,Runtime*)>
    void LegionTaskWrapper::high_level_task_wrapper(const void *args, 
                                                    size_t arglen, 
                                                    const void *userdata,
                                                    size_t userlen,
                                                    Processor p)
    //--------------------------------------------------------------------------
    {
      // Assert that we aren't returning Futures or FutureMaps
      LEGION_STATIC_ASSERT((LegionTypeInequality<T,Future>::value));
      LEGION_STATIC_ASSERT((LegionTypeInequality<T,FutureMap>::value));
      // Assert that the return type size is within the required size
      LEGION_STATIC_ASSERT(sizeof(T) <= MAX_RETURN_SIZE);
      // Get the high level runtime
      Runtime *runtime = Runtime::get_runtime(p);

      // Read the context out of the buffer
      Context ctx = *((const Context*)args);
#ifdef DEBUG_HIGH_LEVEL
      assert(arglen == sizeof(Context));
#endif
      // Get the arguments associated with the context
      const std::vector<PhysicalRegion> &regions = runtime->begin_task(ctx);
      Task *task = reinterpret_cast<Task*>(ctx);

      // Invoke the task with the given context
      T return_value = (*TASK_PTR)(task->args, task->arglen, task->regions,
                                   regions, ctx, runtime);

      // Send the return value back
      LegionSerialization::end_task<T>(runtime, ctx, &return_value);
    }

    //--------------------------------------------------------------------------
    template<
      void (*TASK_PTR)(const void*,size_t,const std::vector<RegionRequirement>&,
                 const std::vector<PhysicalRegion>&,Context,Runtime*)>
    void LegionTaskWrapper::high_level_task_wrapper(const void *args, 
                                                    size_t arglen, 
                                                    const void *userdata,
                                                    size_t userlen,
                                                    Processor p)
    //--------------------------------------------------------------------------
    {
      // Get the high level runtime
      Runtime *runtime = Runtime::get_runtime(p);

      // Read the context out of the buffer
      Context ctx = *((const Context*)args);
#ifdef DEBUG_HIGH_LEVEL
      assert(arglen == sizeof(Context));
#endif
      // Get the arguments associated with the context
      const std::vector<PhysicalRegion> &regions = runtime->begin_task(ctx);
      Task *task = reinterpret_cast<Task*>(ctx);

      // Invoke the task with the given context
      (*TASK_PTR)(task->args, task->arglen, task->regions, 
                                    regions, ctx, runtime);

      // Send an empty return value back
      runtime->end_task(ctx, NULL, 0);
    }

    //--------------------------------------------------------------------------
    template<typename RT,
      RT (*TASK_PTR)(const void*,size_t,const void*,size_t,const DomainPoint&,
                 const std::vector<RegionRequirement>&,
                 const std::vector<PhysicalRegion>&,Context,Runtime*)>
    void LegionTaskWrapper::high_level_index_task_wrapper(const void *args, 
                                                          size_t arglen, 
                                                          const void *userdata,
                                                          size_t userlen,
                                                          Processor p)
    //--------------------------------------------------------------------------
    {
      // Assert that we aren't returning Futures or FutureMaps
      LEGION_STATIC_ASSERT((LegionTypeInequality<RT,Future>::value));
      LEGION_STATIC_ASSERT((LegionTypeInequality<RT,FutureMap>::value));
      // Assert that the return type size is within the required size
      LEGION_STATIC_ASSERT(sizeof(RT) <= MAX_RETURN_SIZE);
      // Get the high level runtime
      Runtime *runtime = Runtime::get_runtime(p);

      // Read the context out of the buffer
      Context ctx = *((const Context*)args);
#ifdef DEBUG_HIGH_LEVEL
      assert(arglen == sizeof(Context));
#endif
      // Get the arguments associated with the context
      const std::vector<PhysicalRegion> &regions = runtime->begin_task(ctx);
      Task *task = reinterpret_cast<Task*>(ctx);
      
      // Invoke the task with the given context
      RT return_value = (*TASK_PTR)(task->args, task->arglen, task->local_args, 
                                    task->local_arglen, task->index_point, 
                                    task->regions, regions, ctx, runtime);

      // Send the return value back
      LegionSerialization::end_task<RT>(runtime, ctx, &return_value); 
    }

    //--------------------------------------------------------------------------
    template< 
      void (*TASK_PTR)(const void*,size_t,const void*,size_t,const DomainPoint&, 
                 const std::vector<RegionRequirement>&,
                 const std::vector<PhysicalRegion>&,Context,Runtime*)>
    void LegionTaskWrapper::high_level_index_task_wrapper(const void *args, 
                                                          size_t arglen, 
                                                          const void *userdata,
                                                          size_t userlen,
                                                          Processor p)
    //--------------------------------------------------------------------------
    {
      // Get the high level runtime
      Runtime *runtime = Runtime::get_runtime(p);

      // Read the context out of the buffer
      Context ctx = *((const Context*)args);
#ifdef DEBUG_HIGH_LEVEL
      assert(arglen == sizeof(Context));
#endif
      // Get the arguments associated with the context
      const std::vector<PhysicalRegion> &regions = runtime->begin_task(ctx);
      Task *task = reinterpret_cast<Task*>(ctx);

      // Invoke the task with the given context
      (*TASK_PTR)(task->args, task->arglen, task->local_args, 
                  task->local_arglen, task->index_point, 
                  task->regions, regions, ctx, runtime);

      // Send an empty return value back
      runtime->end_task(ctx, NULL, 0); 
    }

    //--------------------------------------------------------------------------
    template<typename T,
        T (*TASK_PTR)(const Task*, const std::vector<PhysicalRegion>&,
                      Context, Runtime*)>
    void LegionTaskWrapper::inline_task_wrapper(const Task *task, 
          const std::vector<PhysicalRegion> &regions, 
          Context ctx, Runtime *runtime, 
          void *&return_addr, size_t &return_size)
    //--------------------------------------------------------------------------
    {
      // Assert that we aren't returning Futures or FutureMaps
      LEGION_STATIC_ASSERT((LegionTypeInequality<T,Future>::value));
      LEGION_STATIC_ASSERT((LegionTypeInequality<T,FutureMap>::value));
      // Assert that the return type size is within the required size
      LEGION_STATIC_ASSERT(sizeof(T) <= MAX_RETURN_SIZE);

      T return_value = (*TASK_PTR)(task, regions, ctx, runtime);

      // Send the return value back, no need to pack it
      return_size = sizeof(return_value);
      return_addr = malloc(return_size);
      memcpy(return_addr,&return_value,return_size);
    }

    //--------------------------------------------------------------------------
    template<
      void (*TASK_PTR)(const Task*, const std::vector<PhysicalRegion>&,
                       Context, Runtime*)>
    void LegionTaskWrapper::inline_task_wrapper(const Task *task,
          const std::vector<PhysicalRegion> &regions, 
          Context ctx, Runtime *runtime,
          void *&return_addr, size_t &return_size)
    //--------------------------------------------------------------------------
    {
      (*TASK_PTR)(task, regions, ctx, runtime);

      return_size = 0;
      return_addr = 0;
    }

    //--------------------------------------------------------------------------
    template<typename T, typename UDT,
      T (*TASK_PTR)(const Task*, const std::vector<PhysicalRegion>&,
                    Context, Runtime*, const UDT&)>
    void LegionTaskWrapper::inline_udt_task_wrapper(const Task *task,
          const std::vector<PhysicalRegion> &regions,
          Context ctx, Runtime *runtime,
          void *&return_addr, size_t &return_size)
    //--------------------------------------------------------------------------
    {
      // Assert that we aren't returning Futures or FutureMaps
      LEGION_STATIC_ASSERT((LegionTypeInequality<T,Future>::value));
      LEGION_STATIC_ASSERT((LegionTypeInequality<T,FutureMap>::value));
      // Assert that the return type size is within the required size
      LEGION_STATIC_ASSERT(sizeof(T) <= MAX_RETURN_SIZE);

      const UDT *user_data = (const UDT *)Runtime::find_user_data(
                                     task->task_id, task->selected_variant);

      T return_value = (*TASK_PTR)(task, regions, ctx, runtime, *user_data);

      // Send the return value back, no need to pack it
      return_size = sizeof(return_value);
      return_addr = malloc(return_size);
      memcpy(return_addr,&return_value,return_size);
    }

    //--------------------------------------------------------------------------
    template<typename UDT,
      void (*TASK_PTR)(const Task*, const std::vector<PhysicalRegion>&,
                       Context, Runtime*, const UDT&)>
    void LegionTaskWrapper::inline_udt_task_wrapper(const Task *task,
        const std::vector<PhysicalRegion> &regions,
        Context ctx, Runtime *runtime,
        void *&return_addr, size_t &return_size)
    //--------------------------------------------------------------------------
    {
      const UDT *user_data = (const UDT *)Runtime::find_user_data(
                                      task->task_id, task->selected_variant);

      (*TASK_PTR)(task, regions, ctx, runtime, *user_data);

      return_size = 0;
      return_addr = 0;
    }

    //--------------------------------------------------------------------------
    template<typename T,
      T (*TASK_PTR)(const void*,size_t,const std::vector<RegionRequirement>&,
                  const std::vector<PhysicalRegion>&,Context,Runtime*)>
    void LegionTaskWrapper::high_level_inline_task_wrapper(const Task *task,
          const std::vector<PhysicalRegion> &regions,
          Context ctx, Runtime *runtime,
          void *&return_addr, size_t &return_size)
    //--------------------------------------------------------------------------
    {
      // Assert that we aren't returning Futures or FutureMaps
      LEGION_STATIC_ASSERT((LegionTypeInequality<T,Future>::value));
      LEGION_STATIC_ASSERT((LegionTypeInequality<T,FutureMap>::value));
      // Assert that the return type size is within the required size
      LEGION_STATIC_ASSERT(sizeof(T) <= MAX_RETURN_SIZE);

      T return_value = (*TASK_PTR)(task->args, task->arglen,
                                   task->regions, regions, ctx, runtime);

      return_size = sizeof(return_value);
      return_addr = malloc(return_size);
      memcpy(return_addr,&return_value,return_size);
    }

    //--------------------------------------------------------------------------
    template<
    void (*TASK_PTR)(const void*,size_t,const std::vector<RegionRequirement>&,
                 const std::vector<PhysicalRegion>&,Context,Runtime*)>
    void LegionTaskWrapper::high_level_inline_task_wrapper(const Task *task,
        const std::vector<PhysicalRegion> &regions,
        Context ctx, Runtime *runtime,
        void *&return_addr, size_t &return_size)
    //--------------------------------------------------------------------------
    {
      (*TASK_PTR)(task->args, task->arglen, 
                  task->regions, regions, ctx, runtime);

      return_size = 0;
      return_addr = NULL;
    }

    //--------------------------------------------------------------------------
    template<typename RT,
      RT (*TASK_PTR)(const void*,size_t,const void*,size_t,const DomainPoint&,
                 const std::vector<RegionRequirement>&,
                 const std::vector<PhysicalRegion>&,Context,Runtime*)>
    void LegionTaskWrapper::high_level_inline_index_task_wrapper(
          const Task *task, const std::vector<PhysicalRegion> &regions,
          Context ctx, Runtime *runtime,
          void *&return_addr, size_t &return_size)
    //--------------------------------------------------------------------------
    {
      // Assert that we aren't returning Futures or FutureMaps
      LEGION_STATIC_ASSERT((LegionTypeInequality<RT,Future>::value));
      LEGION_STATIC_ASSERT((LegionTypeInequality<RT,FutureMap>::value));
      // Assert that the return type size is within the required size
      LEGION_STATIC_ASSERT(sizeof(RT) <= MAX_RETURN_SIZE);

      RT return_value = (*TASK_PTR)(task->args, task->arglen,
                                    task->local_args, task->local_arglen,
                                    task->index_point, task->regions,
                                    regions, ctx, runtime);

      return_size = sizeof(return_value);
      return_addr = malloc(return_size);
      memcpy(return_addr,&return_value,return_size);
    }

    //--------------------------------------------------------------------------
    template<
      void (*TASK_PTR)(const void*,size_t,const void*,size_t,const DomainPoint&, 
                 const std::vector<RegionRequirement>&,
                 const std::vector<PhysicalRegion>&,Context,Runtime*)>
    void LegionTaskWrapper::high_level_inline_index_task_wrapper(
        const Task *task, const std::vector<PhysicalRegion> &regions,
        Context ctx, Runtime *runtime,
        void *&return_addr, size_t &return_size)
    //--------------------------------------------------------------------------
    {
      (*TASK_PTR)(task->args, task->arglen, 
                  task->local_args, task->local_arglen,
                  task->index_point, task->regions,
                  regions, ctx, runtime);
                              
      return_size = 0;
      return_addr = NULL;
    }

    //--------------------------------------------------------------------------
    template<typename T,
      T (*TASK_PTR)(const Task*, const std::vector<PhysicalRegion>&,
                    Context, Runtime*)>
    /*static*/ TaskID Runtime::register_legion_task(TaskID id,
                                                    Processor::Kind proc_kind,
                                                    bool single, bool index,
                                                    VariantID vid,
                                                    TaskConfigOptions options,
                                                    const char *task_name)
    //--------------------------------------------------------------------------
    {
      if (task_name == NULL)
      {
        // Has no name, so just call it by 'unnamed_task_<uid>'
        char *buffer = (char*)malloc(32*sizeof(char));
        sprintf(buffer,"unnamed_task_%d",id);
        task_name = buffer;
      }
      return Runtime::update_collection_table(
        LegionTaskWrapper::legion_task_wrapper<T,TASK_PTR>, 
        LegionTaskWrapper::inline_task_wrapper<T,TASK_PTR>, id, proc_kind, 
                            single, index, vid, sizeof(T), options, task_name);
    }

    //--------------------------------------------------------------------------
    template<
      void (*TASK_PTR)(const Task*, const std::vector<PhysicalRegion>&,
                    Context, Runtime*)>
    /*static*/ TaskID Runtime::register_legion_task(TaskID id,
                                                    Processor::Kind proc_kind,
                                                    bool single, bool index,
                                                    VariantID vid,
                                                    TaskConfigOptions options,
                                                    const char *task_name)
    //--------------------------------------------------------------------------
    {
      if (task_name == NULL)
      {
        // Has no name, so just call it by 'unnamed_task_<uid>'
        char *buffer = (char*)malloc(32*sizeof(char));
        sprintf(buffer,"unnamed_task_%d",id);
        task_name = buffer;
      }
      else
        task_name = strdup(task_name);
      return Runtime::update_collection_table(
        LegionTaskWrapper::legion_task_wrapper<TASK_PTR>, 
        LegionTaskWrapper::inline_task_wrapper<TASK_PTR>, id, proc_kind, 
                            single, index, vid, 0/*size*/, options, task_name);
    }

    //--------------------------------------------------------------------------
    template<typename T, typename UDT,
      T (*TASK_PTR)(const Task*, const std::vector<PhysicalRegion>&,
                    Context, Runtime*, const UDT&)>
    /*static*/ TaskID Runtime::register_legion_task(TaskID id,
                                                    Processor::Kind proc_kind,
                                                    bool single, bool index,
                                                    const UDT &user_data,
                                                    VariantID vid,
                                                    TaskConfigOptions options,
                                                    const char *task_name)
    //--------------------------------------------------------------------------
    {
      if (task_name == NULL)
      {
        // Has no name, so just call it by 'unnamed_task_<uid>'
        char *buffer = (char*)malloc(32*sizeof(char));
        sprintf(buffer,"unnamed_task_%d",id);
        task_name = buffer;
      }
      else
        task_name = strdup(task_name);
      return Runtime::update_collection_table(
        LegionTaskWrapper::legion_udt_task_wrapper<T,UDT,TASK_PTR>,
        LegionTaskWrapper::inline_udt_task_wrapper<T,UDT,TASK_PTR>, id, 
                              proc_kind, single, index, vid, sizeof(T), 
                              options, task_name, &user_data, sizeof(UDT));
    }

    //--------------------------------------------------------------------------
    template<typename UDT,
      void (*TASK_PTR)(const Task*, const std::vector<PhysicalRegion>&,
                       Context, Runtime*, const UDT&)>
    /*static*/ TaskID Runtime::register_legion_task(TaskID id,
                                                    Processor::Kind proc_kind,
                                                    bool single, bool index,
                                                    const UDT &user_data,
                                                    VariantID vid,
                                                    TaskConfigOptions options,
                                                    const char *task_name)
    //--------------------------------------------------------------------------
    {
      if (task_name == NULL)
      {
        // Has no name, so just call it by 'unnamed_task_<uid>'
        char *buffer = (char*)malloc(32*sizeof(char));
        sprintf(buffer,"unnamed_task_%d",id);
        task_name = buffer;
      }
      else
        task_name = strdup(task_name);
      return Runtime::update_collection_table(
        LegionTaskWrapper::legion_udt_task_wrapper<UDT,TASK_PTR>,
        LegionTaskWrapper::inline_udt_task_wrapper<UDT,TASK_PTR>, id, proc_kind,
                                         single, index, vid, 0/*size*/, options, 
                                         task_name, &user_data, sizeof(UDT));
    }

    //--------------------------------------------------------------------------
    template<typename T,
        T (*TASK_PTR)(const void*,size_t,const std::vector<RegionRequirement>&, 
                  const std::vector<PhysicalRegion>&,Context,Runtime*)>
    /*static*/ TaskID Runtime::register_single_task(TaskID id, 
                                                    Processor::Kind proc_kind, 
                                                    bool leaf/*= false*/, 
                                                    const char *name/*= NULL*/,
                                                    VariantID vid/*= AUTO*/,
                                                    bool inner/*= false*/,
                                                    bool idempotent/*= false*/)
    //--------------------------------------------------------------------------
    {
      if (name == NULL)
      {
        // Has no name, so just call it by its number
        char *buffer = (char*)malloc(32*sizeof(char));
        sprintf(buffer,"%d",id);
        name = buffer;
      }
      else
        name = strdup(name);
      return Runtime::update_collection_table(
              LegionTaskWrapper::high_level_task_wrapper<T,TASK_PTR>,  
              LegionTaskWrapper::high_level_inline_task_wrapper<T,TASK_PTR>,
              id, proc_kind, true/*single*/, false/*index space*/, vid,
              sizeof(T), TaskConfigOptions(leaf, inner, idempotent), name);
    }

    //--------------------------------------------------------------------------
    template<
        void (*TASK_PTR)(const void*,size_t,
                         const std::vector<RegionRequirement>&,
                         const std::vector<PhysicalRegion>&,
                         Context,Runtime*)>
    /*static*/ TaskID Runtime::register_single_task(TaskID id, 
                                                     Processor::Kind proc_kind, 
                                                     bool leaf/*= false*/, 
                                                     const char *name/*= NULL*/,
                                                     VariantID vid/*= AUTO*/,
                                                     bool inner/*= false*/,
                                                     bool idempotent/*= false*/)
    //--------------------------------------------------------------------------
    {
      if (name == NULL)
      {
        // Has no name, so just call it by its number
        char *buffer = (char*)malloc(32*sizeof(char));
        sprintf(buffer,"%d",id);
        name = buffer;
      }
      else
        name = strdup(name);
      return Runtime::update_collection_table(
              LegionTaskWrapper::high_level_task_wrapper<TASK_PTR>,
              LegionTaskWrapper::high_level_inline_task_wrapper<TASK_PTR>,
              id, proc_kind, true/*single*/, false/*index space*/, vid,
              0/*size*/, TaskConfigOptions(leaf, inner, idempotent), name);
    }

    //--------------------------------------------------------------------------
    template<typename RT/*return type*/,
        RT (*TASK_PTR)(const void*,size_t,const void*,size_t,const DomainPoint&,
                       const std::vector<RegionRequirement>&,
                       const std::vector<PhysicalRegion>&,
                       Context,Runtime*)>
    /*static*/ TaskID Runtime::register_index_task(TaskID id, 
                                                    Processor::Kind proc_kind, 
                                                    bool leaf/*= false*/, 
                                                    const char *name/*= NULL*/,
                                                    VariantID vid/*= AUTO*/,
                                                    bool inner/*= false*/,
                                                    bool idempotent/*= false*/)
    //--------------------------------------------------------------------------
    {
      if (name == NULL)
      {
        // Has no name, so just call it by its number
        char *buffer = (char*)malloc(32*sizeof(char));
        sprintf(buffer,"%d",id);
        name = buffer;
      }
      else
        name = strdup(name);
      return Runtime::update_collection_table(
          LegionTaskWrapper::high_level_index_task_wrapper<RT,TASK_PTR>,  
          LegionTaskWrapper::high_level_inline_index_task_wrapper<RT,TASK_PTR>,
          id, proc_kind, false/*single*/, true/*index space*/, vid,
          sizeof(RT), TaskConfigOptions(leaf, inner, idempotent), name);
    }

    //--------------------------------------------------------------------------
    template<
        void (*TASK_PTR)(const void*,size_t,const void*,size_t,
                         const DomainPoint&,
                         const std::vector<RegionRequirement>&,
                         const std::vector<PhysicalRegion>&,
                         Context,Runtime*)>
    /*static*/ TaskID Runtime::register_index_task(TaskID id, 
                                                    Processor::Kind proc_kind, 
                                                    bool leaf/*= false*/, 
                                                    const char *name/*= NULL*/,
                                                    VariantID vid/*= AUTO*/,
                                                    bool inner/*= false*/,
                                                    bool idempotent/*= false*/)
    //--------------------------------------------------------------------------
    {
      if (name == NULL)
      {
        // Has no name, so just call it by its number
        char *buffer = (char*)malloc(32*sizeof(char));
        sprintf(buffer,"%d",id);
        name = buffer;
      }
      else
        name = strdup(name);
      return Runtime::update_collection_table(
              LegionTaskWrapper::high_level_index_task_wrapper<TASK_PTR>, 
              LegionTaskWrapper::high_level_inline_index_task_wrapper<TASK_PTR>,
              id, proc_kind, false/*single*/, true/*index space*/, vid,
              0/*size*/, TaskConfigOptions(leaf, inner, idempotent), name);
    }

    //--------------------------------------------------------------------------
    inline PrivilegeMode operator~(PrivilegeMode p)
    //--------------------------------------------------------------------------
    {
      return static_cast<PrivilegeMode>(~unsigned(p));
    }

    //--------------------------------------------------------------------------
    inline PrivilegeMode operator|(PrivilegeMode left, PrivilegeMode right)
    //--------------------------------------------------------------------------
    {
      return static_cast<PrivilegeMode>(unsigned(left) | unsigned(right));
    }

    //--------------------------------------------------------------------------
    inline PrivilegeMode operator&(PrivilegeMode left, PrivilegeMode right)
    //--------------------------------------------------------------------------
    {
      return static_cast<PrivilegeMode>(unsigned(left) & unsigned(right));
    }

    //--------------------------------------------------------------------------
    inline PrivilegeMode operator^(PrivilegeMode left, PrivilegeMode right)
    //--------------------------------------------------------------------------
    {
      return static_cast<PrivilegeMode>(unsigned(left) ^ unsigned(right));
    }

    //--------------------------------------------------------------------------
    inline PrivilegeMode operator|=(PrivilegeMode &left, PrivilegeMode right)
    //--------------------------------------------------------------------------
    {
      unsigned l = static_cast<unsigned>(left);
      unsigned r = static_cast<unsigned>(right);
      l |= r;
      return left = static_cast<PrivilegeMode>(l);
    }

    //--------------------------------------------------------------------------
    inline PrivilegeMode operator&=(PrivilegeMode &left, PrivilegeMode right)
    //--------------------------------------------------------------------------
    {
      unsigned l = static_cast<unsigned>(left);
      unsigned r = static_cast<unsigned>(right);
      l &= r;
      return left = static_cast<PrivilegeMode>(l);
    }

    //--------------------------------------------------------------------------
    inline PrivilegeMode operator^=(PrivilegeMode &left, PrivilegeMode right)
    //--------------------------------------------------------------------------
    {
      unsigned l = static_cast<unsigned>(left);
      unsigned r = static_cast<unsigned>(right);
      l ^= r;
      return left = static_cast<PrivilegeMode>(l);
    }

    //--------------------------------------------------------------------------
    inline AllocateMode operator~(AllocateMode a)
    //--------------------------------------------------------------------------
    {
      return static_cast<AllocateMode>(~unsigned(a));
    }

    //--------------------------------------------------------------------------
    inline AllocateMode operator|(AllocateMode left, AllocateMode right)
    //--------------------------------------------------------------------------
    {
      return static_cast<AllocateMode>(unsigned(left) | unsigned(right));
    }

    //--------------------------------------------------------------------------
    inline AllocateMode operator&(AllocateMode left, AllocateMode right)
    //--------------------------------------------------------------------------
    {
      return static_cast<AllocateMode>(unsigned(left) & unsigned(right));
    }

    //--------------------------------------------------------------------------
    inline AllocateMode operator^(AllocateMode left, AllocateMode right)
    //--------------------------------------------------------------------------
    {
      return static_cast<AllocateMode>(unsigned(left) ^ unsigned(right));
    }

    //--------------------------------------------------------------------------
    inline AllocateMode operator|=(AllocateMode &left, AllocateMode right)
    //--------------------------------------------------------------------------
    {
      unsigned l = static_cast<unsigned>(left);
      unsigned r = static_cast<unsigned>(right);
      l |= r;
      return left = static_cast<AllocateMode>(l);
    }

    //--------------------------------------------------------------------------
    inline AllocateMode operator&=(AllocateMode &left, AllocateMode right)
    //--------------------------------------------------------------------------
    {
      unsigned l = static_cast<unsigned>(left);
      unsigned r = static_cast<unsigned>(right);
      l &= r;
      return left = static_cast<AllocateMode>(l);
    }

    //--------------------------------------------------------------------------
    inline AllocateMode operator^=(AllocateMode &left, AllocateMode right)
    //--------------------------------------------------------------------------
    {
      unsigned l = static_cast<unsigned>(left);
      unsigned r = static_cast<unsigned>(right);
      l ^= r;
      return left = static_cast<AllocateMode>(l);
    }

  };
};
