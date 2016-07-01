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

/* Reservation implementation wrapper for OCR Realm
 *
 * Limitations
 * -----------
 * 1. Only Exclusive mode is supported. Exclusive mode can be used in place of Shared mode without change in semantics.
 * 2. Acquire returns a Sticky event which causes a leak since they are not explicitly destroyed.
 */

#ifndef OCR_REALM_RESERVATION_IMPL_H
#define OCR_REALM_RESERVATION_IMPL_H

#if USE_OCR_LAYER

#include "reservation.h"
#include "id.h"

namespace Realm {
    class OCRReservationImpl{

    public:
      static const ID::ID_Types ID_TYPE = ID::ID_LOCK;

      //an object of type ID which is used to call the convert function
      static const ID ocr_rsrv_impl;

      static Reservation create_reservation(size_t _data_size = 0);

      static Event acquire(ocrGuid_t res, unsigned mode = 0, bool exclusive = true, Event wait_on = Event::NO_EVENT);

      static void release(ocrGuid_t res, Event wait_on = Event::NO_EVENT);

      static void destroy_reservation(ocrGuid_t res);
   };
}; // namespace Realm

#endif // USE_OCR_LAYER

#endif // OCR_REALM_RESERVATION_IMPL_H

