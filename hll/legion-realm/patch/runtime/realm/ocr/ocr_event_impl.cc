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

#include "ocr_event_impl.h"

#include "logging.h"
#include "activemsg.h"

#include "extensions/ocr-legacy.h"

#include <assert.h>

namespace Realm {

  ////////////////////////////////////////////////////////////////////////
  //
  // class OCREvents implementation
  //

    extern Logger log_event; // in event_impl.cc

    ocrGuid_t OCREventImpl::event_waiter_edt_t = NULL_GUID;

    const ID OCREventImpl::ocr_event_impl = ID(OCREventImpl::ID_TYPE, gasnet_mynode(), 0);

    //EDT function that calls event_triggered
    //argv is the object whose event_triggered() needs to be invoked
    ocrGuid_t event_waiter_func(u32 argc, u64 *argv, u32 depc, ocrEdtDep_t depv[])
    {
      assert(depc == 1);
      ((EventWaiter *)argv)->event_triggered();
      return NULL_GUID;
    }

    /*static*/ void OCREventImpl::static_init(void)
    {
      //create the function conversion edt template
      ocrEdtTemplateCreate(&OCREventImpl::event_waiter_edt_t, event_waiter_func, EDT_PARAM_UNK, 1);
    }

    /*static*/ void OCREventImpl::static_destroy(void)
    {
      //delete the function conversion edt template
      ocrEdtTemplateDestroy(OCREventImpl::event_waiter_edt_t);
    }

    /*static*/ Event OCREventImpl::create_ocrevent(void)
    {
      Event e = OCREventImpl::ocr_event_impl.convert<Event>();
      e.gen = 0;
      ocrEventCreate(&e.evt_guid, OCR_EVENT_STICKY_T, EVT_PROP_NONE);
      e.id = GUIDA(e.evt_guid);
      return e;
    }

    /*static*/ bool OCREventImpl::has_triggered(ocrGuid_t evt)
    {
      u8 ret = ocrLegacyBlockProgress(evt, NULL, NULL, NULL, LEGACY_PROP_CHECK);
      return OCR_EINVAL != ret;
    }

    /*static*/ void OCREventImpl::wait(ocrGuid_t evt)
    {
      ocrLegacyBlockProgress(evt, NULL, NULL, NULL, LEGACY_PROP_NONE);
    }

    /*static*/ void OCREventImpl::external_wait(ocrGuid_t evt)
    {
      OCREventImpl::wait(evt);
    }

    /*static*/ bool OCREventImpl::add_waiter(Event needed, EventWaiter *waiter)
    {
      const size_t waiter_size = waiter->get_size();

      //invoke the EDT that calls event_triggered
      ocrGuid_t event_waiter_edt;
      ocrEdtCreate(&event_waiter_edt, OCREventImpl::event_waiter_edt_t,
        U64_COUNT(waiter_size), (u64*)waiter, 1, & needed.evt_guid,
        EDT_PROP_NONE, NULL_HINT, NULL);
      return true;
    }

    //create a latch event and add the set of events as its decr slot dependencies
    /*static*/Event OCREventImpl::merge_events(const std::set<Event>& wait_for)
    {
      if (wait_for.empty())
        return Event::NO_EVENT;
      else if(wait_for.size() == 1)
        return *(wait_for.begin());
      else
      {
        ocrEventParams_t params;
        params.EVENT_LATCH.counter = wait_for.size();
        ocrGuid_t latchguid;
        u8 ret = ocrEventCreateParams(&latchguid, OCR_EVENT_LATCH_T, false, &params);
        Event ret_evt = OCREventImpl::create_ocrevent();
        //attach the sticky finish event to the latch since legacy_block_progress needs persistent event
        ocrAddDependence(latchguid, ret_evt.evt_guid, 0, DB_MODE_RO);
        std::set<Event>::iterator it;
        for(it = wait_for.begin(); it!=wait_for.end(); it++)
          ocrAddDependence(it->evt_guid, latchguid, OCR_EVENT_LATCH_DECR_SLOT, DB_MODE_RO);
        return ret_evt;
      }
    }

    //create a latch event and add ev* as its decr slot dependencies
    /*static*/ Event OCREventImpl::merge_events(Event ev1, Event ev2,
                                                Event ev3 /*= NO_EVENT*/, Event ev4 /*= NO_EVENT*/,
                                                Event ev5 /*= NO_EVENT*/, Event ev6 /*= NO_EVENT*/)
    {
      const unsigned size=6;
      ocrGuid_t wait_for_arr[size];
      int count=0;

      //convert Event parameters to an array of Guid and pass it to EDT as dependency
      if(ev1.exists()) {
        log_event.info() << "event merging: event wait_on=" << ev1 << " index=" << 1;
         wait_for_arr[count++] =  ev1.evt_guid;
      }
      if(ev2.exists()) {
        log_event.info() << "event merging: event wait_on=" << ev2 << " index=" << 2;
         wait_for_arr[count++] =  ev2.evt_guid;
      }
      if(ev3.exists()) {
        log_event.info() << "event merging: event wait_on=" << ev3 << " index=" << 3;
         wait_for_arr[count++] =  ev3.evt_guid;
      }
      if(ev4.exists()) {
        log_event.info() << "event merging: event wait_on=" << ev4 << " index=" << 4;
         wait_for_arr[count++] =  ev4.evt_guid;
      }
      if(ev5.exists()) {
        log_event.info() << "event merging: event wait_on=" << ev5 << " index=" << 5;
         wait_for_arr[count++] =  ev5.evt_guid;
      }
      if(ev6.exists()) {
        log_event.info() << "event merging: event wait_on=" << ev6 << " index=" << 6;
         wait_for_arr[count++] =  ev6.evt_guid;
      }

      if(count == 0)
        return Event::NO_EVENT;

      ocrEventParams_t params;
      params.EVENT_LATCH.counter = count;
      ocrGuid_t latchguid;
      u8 ret = ocrEventCreateParams(&latchguid, OCR_EVENT_LATCH_T, false, &params);
      Event ret_evt = OCREventImpl::create_ocrevent();
      //attach the sticky finish event to the latch since legacy_block_progress needs persistent event
      ocrAddDependence(latchguid, ret_evt.evt_guid, 0, DB_MODE_RO);
      for(int i=0; i<count; i++)
        ocrAddDependence(wait_for_arr[i], latchguid, OCR_EVENT_LATCH_DECR_SLOT, DB_MODE_RO);
      return ret_evt;
    }

    /*static*/ void OCREventImpl::trigger(ocrGuid_t evt, Event wait_on /*= Event::NO_EVENT*/)
    {
      ocrAddDependence(wait_on.evt_guid, evt, 0, DB_MODE_RO);
    }

}; // namespace Realm

#endif // USE_OCR_LAYER

