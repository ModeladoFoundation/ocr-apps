/* Copyright 2017 Stanford University, NVIDIA Corporation
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

// events for Realm

// nop, but helps IDEs
#include "event.h"

#include "serialize.h"
TYPE_IS_SERIALIZABLE(Realm::Event);
TYPE_IS_SERIALIZABLE(Realm::UserEvent);
TYPE_IS_SERIALIZABLE(Realm::Barrier);

namespace Realm {

  ////////////////////////////////////////////////////////////////////////
  //
  // class Event

  inline bool Event::operator<(const Event& rhs) const
  {
#if USE_OCR_LAYER
    return ocrGuidIsLt(evt_guid, rhs.evt_guid);
#else
    return id < rhs.id;
#endif // USE_OCR_LAYER
  }

  inline bool Event::operator==(const Event& rhs) const
  {
#if USE_OCR_LAYER
    return ocrGuidIsEq(evt_guid, rhs.evt_guid);
#else
    return id == rhs.id;
#endif // USE_OCR_LAYER
  }

  inline bool Event::operator!=(const Event& rhs) const
  {
#if USE_OCR_LAYER
    return ! ocrGuidIsEq(evt_guid, rhs.evt_guid);
#else
    return id != rhs.id;
#endif // USE_OCR_LAYER
  }

  inline bool Event::exists(void) const
  {
    return id != 0;
  }

  inline std::ostream& operator<<(std::ostream& os, Event e)
  {
    return os << std::hex << e.id << std::dec;
  }


}; // namespace Realm
