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

// Memory implementation for OCR Realm

#ifndef OCR_REALM_MEMORY_IMPL_H
#define OCR_REALM_MEMORY_IMPL_H

#if USE_OCR_LAYER

#include "mem_impl.h"

namespace Realm {

    //structure used to pass information between EDT and constructor
    struct DB_Alloc_Data
    {
      char *base_addr;
      ocrGuid_t buff_db_guid, block_evt_guid;
    };

    class OCRMemory : public MemoryImpl {
    public:
      static const size_t ALIGNMENT = 256;

      OCRMemory(Memory _me, size_t _size);

      virtual ~OCRMemory(void);

      virtual RegionInstance create_instance(IndexSpace r,
                                             const int *linearization_bits,
                                             size_t bytes_needed,
                                             size_t block_size,
                                             size_t element_size,
                                             const std::vector<size_t>& field_sizes,
                                             ReductionOpID redopid,
                                             off_t list_size,
                                             const ProfilingRequestSet &reqs,
                                             RegionInstance parent_inst);
      virtual void destroy_instance(RegionInstance i,
                                    bool local_destroy);
      virtual off_t alloc_bytes(size_t size);
      virtual void free_bytes(off_t offset, size_t size);
      virtual void get_bytes(off_t offset, void *dst, size_t size);
      virtual void put_bytes(off_t offset, const void *src, size_t size);
      virtual void *get_direct_ptr(off_t offset, size_t size);
      virtual int get_home_node(off_t offset, size_t size);

    private:
      //guid of the giant data block that represents memory
      ocrGuid_t ocr_db_guid;
      //guid of the event that holds alive the EDT which created the memory data block
      ocrGuid_t ocr_evt_guid;
      char *base, *base_orig;
    };

}; // namespace Realm

#endif // USE_OCR_LAYER

#endif // OCR_REALM_MEM_IMPL_H

