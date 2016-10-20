/// \file
/// Performance timer functions.
#ifndef __PERFORMANCE_TIMERS_H_
#define __PERFORMANCE_TIMERS_H_

#include <stdint.h>
#include <inttypes.h>

#include "mytype.h"

/// Timer handles
enum TimerHandle{
   totalTimer,
   loopTimer,
   timestepTimer,
   positionTimer,
   velocityTimer,
   redistributeTimer,
   atomHaloTimer,
   computeForceTimer,
   eamHaloTimer,
   commHaloTimer,
   commReduceTimer,
   numberOfTimers};

/// Timer data collected.  Also facilitates computing averages and
/// statistics.
typedef struct TimersSt
{
   uint64_t start;     //!< call start time
   uint64_t total;     //!< current total time
   uint64_t count;     //!< current call count
   uint64_t elapsed;   //!< lap time

   int minRank;        //!< rank with min value
   int maxRank;        //!< rank with max value

   double minValue;    //!< min over ranks
   double maxValue;    //!< max over ranks
   double average;     //!< average over ranks
   double stdev;       //!< stdev across ranks
} Timers;

/// Global timing data collected.
typedef struct TimerGlobalSt
{
   double atomRate;       //!< average time (us) per atom per rank
   double atomAllRate;    //!< average time (us) per atom
   double atomsPerUSec;   //!< average atoms per time (us)
} TimerGlobal;

/// Structure for use with MINLOC and MAXLOC operations.
typedef struct RankReduceDataSt
{
   double val;
   int rank;
} RankReduceData;

/// Use the startTimer and stopTimer macros for timers in code regions
/// that may be performance sensitive.  These can be compiled away by
/// defining NTIMING.  If you are placing a timer anywere outside of a
/// tight loop, consider calling profileStart and profileStop instead.
///
/// Place calls as follows to collect time for code pieces.
/// Time is collected everytime this portion of code is executed.
///
///     ...
///     startTimer(computeForceTimer);
///     computeForce(sim);
///     stopTimer(computeForceTimer);
///     ...
///
#ifndef NTIMING
#define startTimer(perfTimerh,handle)    \
   do                         \
{                          \
   profileStart(perfTimerh,handle);   \
} while(0)
#define stopTimer(perfTimerh,handle)     \
   do                         \
{                          \
   profileStop(perfTimerh,handle);    \
} while(0)
#else
#define startTimer(perfTimerh,handle)
#define stopTimer(perfTimerh,handle)
#endif

void initTimers(Timers* perfTimer);

/// Use profileStart and profileStop only for timers that should *never*
/// be turned off.  Typically this means they are outside the main
/// simulation loop.  If the timer is inside the main loop use
/// startTimer and stopTimer instead.
void profileStart(Timers* perfTimer, const enum TimerHandle handle);
void profileStop(Timers* perfTimer, const enum TimerHandle handle);

/// Use to get elapsed time (lap timer).
double getElapsedTime(Timers* perfTimer, const enum TimerHandle handle);

ocrGuid_t printPerformanceResultsEdt( EDT_ARGS );

#endif
