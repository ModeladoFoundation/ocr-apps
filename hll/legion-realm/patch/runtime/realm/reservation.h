/* Copyright 2016 Stanford University, NVIDIA Corporation
 * Portions Copyright 2016 Rice University, Intel Corporation
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

// reservations for Realm

#ifndef REALM_RESERVATION_H
#define REALM_RESERVATION_H

#include "lowlevel_config.h"

#include "event.h"

namespace Realm {

    class Reservation {
    public:
      typedef ::legion_lowlevel_id_t id_t;
      id_t id;

#if USE_OCR_LAYER
      ocrGuid_t res_guid;
#endif // USE_OCR_LAYER

      bool operator<(const Reservation& rhs) const;
      bool operator==(const Reservation& rhs) const;
      bool operator!=(const Reservation& rhs) const;

      static const Reservation NO_RESERVATION;

      bool exists(void) const { return id != 0; }

      // requests ownership (either exclusive or shared) of the reservation with a
      //   specified mode - returns an event that will trigger when the reservation
      //   is granted
      Event acquire(unsigned mode = 0, bool exclusive = true, Event wait_on = Event::NO_EVENT) const;
      // releases a held reservation - release can be deferred until an event triggers
      void release(Event wait_on = Event::NO_EVENT) const;

      // Create a new reservation, destroy an existing reservation
      static Reservation create_reservation(size_t _data_size = 0);
      void destroy_reservation();

      size_t data_size(void) const;
      void *data_ptr(void) const;
    };

    inline std::ostream& operator<<(std::ostream& os, Reservation r) { return os << std::hex << r.id << std::dec; }

}; // namespace Realm

//include "reservation.inl"

#endif // ifndef REALM_RESERVATION_H

