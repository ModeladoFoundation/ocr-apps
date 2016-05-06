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

#if USE_OCR_LAYER

#include "ocr_mem_impl.h"

#include "extensions/ocr-legacy.h"

namespace Realm {

  extern Logger log_malloc; // in mem_impl.cc

  ////////////////////////////////////////////////////////////////////////
  //
  // class OCRMemory
  //

  //create a datablock for the whole memory and keep the EDT alive so that the data block wont be relocated
  //argv[0] is the size of the data block to be created
  //depv[0] contains the event GUID taat needs to be satisfied for the memory constructor to unblock
  //ideally the event GUID should also be passed through argv
  ocrGuid_t ocr_realm_alloc_db_func(u32 argc, u64 *argv, u32 depc, ocrEdtDep_t depv[])
  {
    assert(argc == 1 && depc == 1);
    ocrGuid_t ret_db_guid;
    DB_Alloc_Data *data;
    //create the data block that is returned to memory constructor using EventSatisfy
    ocrDbCreate(&ret_db_guid, (void **)(& data), sizeof(DB_Alloc_Data), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
    //create the data block to represent memory buffer
    ocrDbCreate(& data->buff_db_guid, (void **)(& data->base_addr), argv[0], DB_PROP_NONE, NULL_HINT, NO_ALLOC);
    //create an event on which this EDT is blocked so as to prevent the relocation of memory buffer data block
    ocrEventCreate(& data->block_evt_guid, OCR_EVENT_STICKY_T, EVT_PROP_NONE);

    //satisfy the event on which the memory constructor is blocked
    //return the base_addr, db guid and block evt guid
    ocrDbRelease(ret_db_guid);
    ocrEventSatisfy(*((ocrGuid_t *)depv[0].ptr), ret_db_guid);

    //block the thread until memory destructor satisfies the event
    ocrLegacyBlockProgress(data->block_evt_guid, NULL, NULL, NULL, LEGACY_PROP_NONE);

    //cleanup the event
    ocrEventDestroy(data->block_evt_guid);

    return NULL_GUID;
  }


  OCRMemory::OCRMemory(Memory _me, size_t _size)
    : MemoryImpl(_me, _size, MKIND_OCR, ALIGNMENT, Memory::OCR_MEM)
  {
    // allocate our own space
    // enforce alignment on the whole memory range
    //base_orig = new char[_size + ALIGNMENT - 1];

    ocrGuid_t ret_evt_guid, ret_db_guid, ocr_realm_alloc_db_edt_t, ocr_realm_alloc_db_edt;
    ocrEdtTemplateCreate(&ocr_realm_alloc_db_edt_t, ocr_realm_alloc_db_func, 1, 1);

    //create a data block and pass ret_evt_guid using it
    size_t align_size = _size + ALIGNMENT - 1;
    ocrGuid_t db_guid, *db_buff;
    ocrDbCreate(&db_guid, (void **)(&db_buff), sizeof(ocrGuid_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
    //create the event on which this constructor should be blocked while datablock is created
    ocrEventCreate(&ret_evt_guid, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
    *db_buff = ret_evt_guid;

    //invoke the EDT to create the data block
    ocrEdtCreate(&ocr_realm_alloc_db_edt, ocr_realm_alloc_db_edt_t, EDT_PARAM_DEF,
        (u64*)(&align_size), EDT_PARAM_DEF, &db_guid,
        EDT_PROP_NONE, NULL_HINT, NULL);
    ocrEdtTemplateDestroy(ocr_realm_alloc_db_edt_t);

    //task that allocates the db returns the base address, db guid and guid of the event on which that EDT is blocked
    DB_Alloc_Data *ret_data;
    size_t ret_size;
    ocrLegacyBlockProgress(ret_evt_guid, &ret_db_guid, (void**)&ret_data, &ret_size, LEGACY_PROP_NONE);

    base_orig = ret_data->base_addr;
    ocr_db_guid = ret_data->buff_db_guid;
    ocr_evt_guid = ret_data->block_evt_guid;

    //cleanup
    ocrEventDestroy(ret_evt_guid);
    ocrDbDestroy(db_guid);
    ocrDbDestroy(ret_db_guid);

    size_t ofs = reinterpret_cast<size_t>(base_orig) % ALIGNMENT;
    if(ofs > 0) {
      base = base_orig + (ALIGNMENT - ofs);
    } else {
      base = base_orig;
    }

    log_malloc.debug("OCR memory at %p, size = %zd", base, _size);
    free_blocks[0] = _size;
  }

  OCRMemory::~OCRMemory(void)
  {
      //delete[] base_orig;
      //unblock the task that created the data block
      ocrEventSatisfy(ocr_evt_guid, NULL_GUID);
      //cleanup the memory buffer data block
      ocrDbDestroy(ocr_db_guid);
  }

  RegionInstance OCRMemory::create_instance(IndexSpace r,
                                                 const int *linearization_bits,
                                                 size_t bytes_needed,
                                                 size_t block_size,
                                                 size_t element_size,
                                                 const std::vector<size_t>& field_sizes,
                                                 ReductionOpID redopid,
                                                 off_t list_size,
                                                 const ProfilingRequestSet &reqs,
                                                 RegionInstance parent_inst)
  {
    return create_instance_local(r, linearization_bits, bytes_needed,
                                 block_size, element_size, field_sizes, redopid,
                                 list_size, reqs, parent_inst);
  }

  void OCRMemory::destroy_instance(RegionInstance i,
                                   bool local_destroy)
  {
    destroy_instance_local(i, local_destroy);
  }

  off_t OCRMemory::alloc_bytes(size_t size)
  {
    return alloc_bytes_local(size);
  }

  void OCRMemory::free_bytes(off_t offset, size_t size)
  {
    free_bytes_local(offset, size);
  }

  void OCRMemory::get_bytes(off_t offset, void *dst, size_t size)
  {
    memcpy(dst, base+offset, size);
  }

  void OCRMemory::put_bytes(off_t offset, const void *src, size_t size)
  {
    memcpy(base+offset, src, size);
  }

  void *OCRMemory::get_direct_ptr(off_t offset, size_t size)
  {
    return (base + offset);
  }

  int OCRMemory::get_home_node(off_t offset, size_t size)
  {
    return gasnet_mynode();
  }

}; // namespace Realm

#endif // USE_OCR_LAYER

