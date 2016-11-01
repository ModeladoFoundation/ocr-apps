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

// Event/UserEvent/Barrier implementations for OCR Realm

#ifndef OCR_REALM_EVENT_IMPL_H
#define OCR_REALM_EVENT_IMPL_H

#if USE_OCR_LAYER

#include "event.h"
#include "id.h"

#include "event_impl.h"

namespace Realm {
    class OCREventImpl{

    public:
      static const ID::ID_Types ID_TYPE = ID::ID_EVENT;
      //EDT template for the event waiter function
      static ocrGuid_t event_waiter_edt_t;
      //an  object of type ID which is used to call the convert function
      static const ID ocr_event_impl; //= ID(OCREventImpl::ID_TYPE, gasnet_mynode(), 0);

      static void static_init(void);
      static void static_destroy(void);

      //create an OCR event. Almost equivalent of create_genevent()
      static Event create_ocrevent(void);

      static bool has_triggered(ocrGuid_t);

      static void wait(ocrGuid_t);

      static void external_wait(ocrGuid_t);

      static bool add_waiter(Event needed, EventWaiter *waiter);

      static Event merge_events(const std::set<Event>& wait_for);
      static Event merge_events(Event ev1, Event ev2,
                                Event ev3 = Event::NO_EVENT, Event ev4 = Event::NO_EVENT,
                                Event ev5 = Event::NO_EVENT, Event ev6 = Event::NO_EVENT);

      static void trigger(ocrGuid_t, Event wait_on = Event::NO_EVENT);

      //private:
      //  ocrGuid_t evt_guid;
    };
}; // namespace Realm

#endif // USE_OCR_LAYER

#endif // OCR_REALM_EVENT_IMPL_H

