/* Copyright 2016 Stanford University, NVIDIA Corporation
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

// clocks, timers for Realm

// nop, but helps IDEs
#include "timers.h"

#include "logging.h"

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#else
#include <time.h>
#endif

namespace Realm {

  ////////////////////////////////////////////////////////////////////////
  //
  // class Clock

  inline /*static*/ double Clock::current_time(bool absolute /*= false*/)
  {
#ifdef __MACH__
    mach_timespec_t ts;
    clock_serv_t cclock;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &ts);
    mach_port_deallocate(mach_task_self(), cclock);
#else
    struct timespec ts;
    clock_gettime(absolute ? CLOCK_REALTIME : CLOCK_MONOTONIC, &ts);
#endif
    double t = ts.tv_sec + (1e-9 * ts.tv_nsec);
    if(!absolute)
      t -= 1e-9 * zero_time;
    return t;
  }
  
  inline /*static*/ long long Clock::current_time_in_microseconds(bool absolute /*= false*/)
  {
#ifdef __MACH__
    mach_timespec_t ts;
    clock_serv_t cclock;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &ts);
    mach_port_deallocate(mach_task_self(), cclock);
#else
    struct timespec ts;
    clock_gettime(absolute ? CLOCK_REALTIME : CLOCK_MONOTONIC, &ts);
#endif
    long long t = (1000000LL * ts.tv_sec) + (ts.tv_nsec / 1000);
    if(!absolute)
      t -= zero_time / 1000;
    return t;
  }
  
  inline /*static*/ long long Clock::current_time_in_nanoseconds(bool absolute /*= false*/)
  {
#ifdef __MACH__
    mach_timespec_t ts;
    clock_serv_t cclock;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &ts);
    mach_port_deallocate(mach_task_self(), cclock);
#else
    struct timespec ts;
    clock_gettime(absolute ? CLOCK_REALTIME : CLOCK_MONOTONIC, &ts);
#endif
    long long t = (1000000000LL * ts.tv_sec) + ts.tv_nsec;
    if(!absolute)
      t -= zero_time;
    return t;
  }

  inline /*static*/ long long Clock::get_zero_time(void)
  {
    return zero_time;
  }

  inline /*static*/ void Clock::set_zero_time(void)
  {
    // this looks weird, but we can't use the absolute time because it uses
    //  a different system clock in POSIX-land, so we ask for the current
    //  relative time (based on whatever zero_time currently is) and add
    //  that in
    zero_time += current_time_in_nanoseconds(false);
  }

  
  ////////////////////////////////////////////////////////////////////////
  //
  // class Clock

  inline TimeStamp::TimeStamp(const char *_message, bool _difference, Logger *_logger /*= 0*/)
    : message(_message), difference(_difference), logger(_logger)
  {
    start_time = Clock::current_time();

    if(!difference) {
      if(logger)
	logger->info("%s %7.6f", message, start_time);
      else
	printf("%s %7.6f\n", message, start_time);
    }
  }

  inline TimeStamp::~TimeStamp(void)
  {
    if(difference) {
      double interval = Clock::current_time() - start_time;

      if(logger)
	logger->info("%s %7.6f", message, interval);
      else
	printf("%s %7.6f\n", message, interval);
    }
  }

}; // namespace Realm

