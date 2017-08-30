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

#if USE_OCR_LAYER

#include "ocr_rsrv_impl.h"
#include "ocr_event_impl.h"
#include "ocr-reservations.h"
#include "logging.h"
#include "activemsg.h"

#include <assert.h>

namespace Realm {

  ////////////////////////////////////////////////////////////////////////
  //
  // class OCRReservation implementation
  //

    extern Logger log_reservation; // in reservation_impl.cc

    //const ID OCRReservationImpl::ocr_rsrv_impl = ID(OCRReservationImpl::ID_TYPE, gasnet_mynode(), 0);
    const ID OCRReservationImpl::ocr_rsrv_impl = ID::make_reservation(gasnet_mynode(), 0);

    /*static*/ Reservation OCRReservationImpl::create_reservation(size_t _data_size /*= 0*/)
    {
      Reservation r = OCRReservationImpl::ocr_rsrv_impl.convert<Reservation>();
      ocrReservationCreate(&r.res_guid, NULL);
      return r;
    }

    /*static*/ Event OCRReservationImpl::acquire(ocrGuid_t res, unsigned mode /*0*/,
                     bool exclusive /*true*/, Event wait_on /*Event::NO_EVENT*/)
    {
      //only exclusive mode available for now

      //create a sticky event to return
      Event e = OCREventImpl::ocr_event_impl.convert<Event>();
      ocrEventCreate(&e.evt_guid, OCR_EVENT_STICKY_T, EVT_PROP_NONE);

      //add one additional unsatisfied dependency event to prevent acquire
      //until dependency is added to return sticky event
      ocrGuid_t out_evt, dep_evt[2];
      dep_evt[0] = wait_on.evt_guid;
      ocrEventCreate(&dep_evt[1], OCR_EVENT_ONCE_T, EVT_PROP_NONE);

      ocrReservationAcquireRequest(res, OCR_RES_EXCL_T, 2, dep_evt, &out_evt);

      //add dependency to return sicky event
      ocrAddDependence(out_evt, e.evt_guid, 0, DB_MODE_RO);
      //enable acquire by satisfying the event
      ocrEventSatisfy(dep_evt[1], NULL_GUID);

      return e;
    }

    /*static*/ void OCRReservationImpl::release(ocrGuid_t res, Event wait_on /*Event::NO_EVENT*/)
    {
      ocrReservationReleaseRequest(res, 1, &wait_on.evt_guid);
    }

    /*static*/ void OCRReservationImpl::destroy_reservation(ocrGuid_t res)
    {
      ocrReservationDestroy(res);
    }

}; // namespace Realm

#endif // USE_OCR_LAYER

