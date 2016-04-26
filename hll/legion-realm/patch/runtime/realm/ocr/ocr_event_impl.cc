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

    ocrGuid_t OCREventImpl::merge_events_edt_t = NULL_GUID;
    const ID OCREventImpl::ocr_event_impl = ID(OCREventImpl::ID_TYPE, gasnet_mynode(), 0);

    //This function is used to create a dummy EDT so that it waits till all dependencies are met.
    //This is used to merge events i.e create an event that waits for all the input event to be met
    ocrGuid_t ocr_realm_merge_events_func(u32 argc, u64 *argv, u32 depc, ocrEdtDep_t depv[])
    {
        assert(argc==0 && argv==NULL);
        assert(depc >= 1);
        return NULL_GUID;
    }

    /*static*/ void OCREventImpl::static_init(void)
    {
      //create the merge events edt template
      ocrEdtTemplateCreate(&OCREventImpl::merge_events_edt_t, ocr_realm_merge_events_func, 0, EDT_PARAM_UNK);
    }

    /*static*/ void OCREventImpl::static_destroy(void)
    {
      //delete the merge events edt template
      ocrEdtTemplateDestroy(OCREventImpl::merge_events_edt_t);
    }

    /*static*/ Event OCREventImpl::create_ocrevent(void)
    {
      Event e = OCREventImpl::ocr_event_impl.convert<Event>();
      ocrEventCreate(&e.evt_guid, OCR_EVENT_STICKY_T, EVT_PROP_NONE);
      return e;
    }

    /*static*/ bool OCREventImpl::guid_less(const ocrGuid_t& lhs, const ocrGuid_t& rhs)
    {
      return lhs < rhs;
    }

    /*static*/ bool OCREventImpl::guid_eq(const ocrGuid_t& lhs, const ocrGuid_t& rhs)
    {
      return lhs == rhs;
    }

    /*static*/ bool OCREventImpl::guid_neq(const ocrGuid_t& lhs, const ocrGuid_t& rhs)
    {
      return lhs != rhs;
    }

    /*static*/ bool OCREventImpl::has_triggered(ocrGuid_t evt)
    {
      //inefficient but works for now
      return false;
    }

    /*static*/ void OCREventImpl::wait(ocrGuid_t evt)
    {
      ocrLegacyBlockProgress(evt, NULL, NULL, NULL, LEGACY_PROP_NONE);
    }

    /*static*/ void OCREventImpl::external_wait(ocrGuid_t evt)
    {
      OCREventImpl::wait(evt);
    }

    //create an EDT with an array of wait_for events as dependency
    /*static*/Event OCREventImpl::merge_events(const std::set<Event>& wait_for)
    {
      if (wait_for.empty())
        return Event::NO_EVENT;
      else if(wait_for.size() == 1)
        return *(wait_for.begin());
      else
      {
        const unsigned size = wait_for.size();
        ocrGuid_t wait_for_arr[size], merge_events_edt, out_merge_events_edt;

        Event ret_evt = OCREventImpl::create_ocrevent();

        //convert Event set to an array of guids and pass it to EDT as dependency
        std::set<Event>::iterator it; int i;
        for(it = wait_for.begin(), i=0; it!=wait_for.end(); it++,i++)
        {
          wait_for_arr[i] = it->evt_guid;
        }
        wait_for_arr[0] = UNINITIALIZED_GUID;

        ocrEdtCreate(&merge_events_edt, OCREventImpl::merge_events_edt_t, EDT_PARAM_DEF, NULL, size,
                     wait_for_arr, EDT_PROP_NONE, NULL_GUID, &out_merge_events_edt);

        //attach the sticky finish event to the edt since legacy_block_progress needs persistent event
        ocrAddDependence(out_merge_events_edt, ret_evt.evt_guid, 0, DB_MODE_RO);

        //start the EDT by statisfying dependency only after linking to the return event
        ocrAddDependence(wait_for.begin()->evt_guid, merge_events_edt, 0, DB_MODE_RO);

        return ret_evt;
      }
    }

    //create an EDT with an array of ev* events as dependency
    /*static*/ Event OCREventImpl::merge_events(Event ev1, Event ev2,
                                                Event ev3 /*= NO_EVENT*/, Event ev4 /*= NO_EVENT*/,
                                                Event ev5 /*= NO_EVENT*/, Event ev6 /*= NO_EVENT*/)
    {
      const unsigned size=6;
      ocrGuid_t wait_for_arr[size], merge_events_edt, out_merge_events_edt, tmp_first_event;
      int count=0;

      Event ret_evt = OCREventImpl::create_ocrevent();

      //convert Event parameters to an array of Guid and pass it to EDT as dependency
      if(ev1.exists()) {
        log_event.info() << "event merging: event=" << ret_evt << " wait_on=" << ev1 << " index=" << 1;
         wait_for_arr[count++] =  ev1.evt_guid;
      }
      if(ev2.exists()) {
        log_event.info() << "event merging: event=" << ret_evt << " wait_on=" << ev2 << " index=" << 2;
         wait_for_arr[count++] =  ev2.evt_guid;
      }
      if(ev3.exists()) {
        log_event.info() << "event merging: event=" << ret_evt << " wait_on=" << ev3 << " index=" << 3;
         wait_for_arr[count++] =  ev3.evt_guid;
      }
      if(ev4.exists()) {
        log_event.info() << "event merging: event=" << ret_evt << " wait_on=" << ev4 << " index=" << 4;
         wait_for_arr[count++] =  ev4.evt_guid;
      }
      if(ev5.exists()) {
        log_event.info() << "event merging: event=" << ret_evt << " wait_on=" << ev5 << " index=" << 5;
         wait_for_arr[count++] =  ev5.evt_guid;
      }
      if(ev6.exists()) {
        log_event.info() << "event merging: event=" << ret_evt << " wait_on=" << ev6 << " index=" << 6;
         wait_for_arr[count++] =  ev6.evt_guid;
      }

      if(count == 0)
        return Event::NO_EVENT;

      tmp_first_event = wait_for_arr[0];
      wait_for_arr[0] = UNINITIALIZED_GUID;

      ocrEdtCreate(&merge_events_edt, OCREventImpl::merge_events_edt_t, EDT_PARAM_DEF, NULL, count,
                   wait_for_arr, EDT_PROP_NONE, NULL_GUID, &out_merge_events_edt);

      //attach the sticky finish event to the edt since legacy_block_progress needs persistent event
      ocrAddDependence(out_merge_events_edt, ret_evt.evt_guid, 0, DB_MODE_RO);

      //start the EDT by statisfying dependency only after linking to the return event
      ocrAddDependence(tmp_first_event, merge_events_edt, 0, DB_MODE_RO);

      return ret_evt;
    }
}; // namespace Realm

#endif // USE_OCR_LAYER

