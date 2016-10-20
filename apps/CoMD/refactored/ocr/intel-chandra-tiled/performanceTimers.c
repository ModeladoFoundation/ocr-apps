/// \file
/// Performance timer functions.
///
/// Use the timer functionality to collect timing and number of calls
/// information for chosen computations (such as force calls) and
/// communication (such as sends, receives, reductions).  Timing results
/// are reported at the end of the run showing overall timings and
/// statistics of timings across ranks.
///
/// A new timer can be added as follows:
/// -# add new handle to the TimerHandle in performanceTimers.h
/// -# provide a corresponding name in timerName
///
/// Note that the order of the handles and names must be the
/// same. This order also determines the order in which the timers are
/// printed. Names can contain leading spaces to show a hierarchical
/// ordering.  Timers with zero calls are omitted from the report.
///
/// Raw timer data is obtained from the getTime() and getTick()
/// functions.  The supplied portable versions of these functions can be
/// replaced with platform specific versions for improved accuracy or
/// lower latency.
/// \see TimerHandle
/// \see getTime
/// \see getTick
///


#include <sys/time.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

#include "performanceTimers.h"
#include "mytype.h"

#include "CoMDTypes.h"

static uint64_t getTime(void);
static double getTick(void);
static void timerStats( Timers* perfTimer, double* recvBuf, int myRank, int nRanks );

void profileStart(Timers* perfTimer, const enum TimerHandle handle)
{
   perfTimer[handle].start = getTime();
}

void profileStop(Timers* perfTimer, const enum TimerHandle handle)
{
   perfTimer[handle].count += 1;
   uint64_t delta = getTime() - perfTimer[handle].start;
   perfTimer[handle].total += delta;
   perfTimer[handle].elapsed += delta;
}

/// \details
/// Return elapsed time (in seconds) since last call with this handle
/// and clear for next lap.
double getElapsedTime(Timers* perfTimer, const enum TimerHandle handle)
{
   double etime = getTick() * (double)perfTimer[handle].elapsed;
   perfTimer[handle].elapsed = 0;

   return etime;
}

/// \details
/// The report contains two blocks.  The upper block is performance
/// information for the printRank.  The lower block is statistical
/// information over all ranks.
void printPerformanceResults(Timers* perfTimer, double* recvBuf, TimerGlobal perfGlobal, int nGlobalAtoms, int printRate, int myRank, int nRanks)
{
/// You must add timer name in same order as enum in .h file.
/// Leading spaces can be specified to show a hierarchy of timers.
char* timerName[numberOfTimers] = {
   "total               ",
   "loop                ",
   "timestep            ",
   "  position          ",
   "  velocity          ",
   "  redistribute      ",
   "    atomHalo        ",
   "  force             ",
   "    eamHalo         ",
   "commHalo            ",
   "commReduce          "
};
   // Collect timer statistics overall and across ranks
   timerStats( perfTimer, recvBuf, myRank, nRanks);

   if (myRank != 0)
      return;

   // only print timers with non-zero values.
   double tick = getTick();
   double loopTime = perfTimer[loopTimer].total*tick;

   PRINTF("\n\nTimings for Rank %d\n", myRank);
   PRINTF("        Timer        # Calls    Avg/Call (s)   Total (s)    %% Loop\n");
   PRINTF("___________________________________________________________________\n");
   for (int ii=0; ii<numberOfTimers; ++ii)
   {
      double totalTime = perfTimer[ii].total*tick;
      if (perfTimer[ii].count > 0)
         PRINTF("%16s   %16d     %8.4f      %8.4f    %8.2f\n",
                 timerName[ii],
                 perfTimer[ii].count,
                 totalTime/(double)perfTimer[ii].count,
                 totalTime,
                 totalTime/loopTime*100.0);
   }

   PRINTF("\nTiming Statistics Across %d Ranks:\n", nRanks);
   PRINTF("        Timer        Rank: Min(s)       Rank: Max(s)      Avg(s)    Stdev(s)\n");
   PRINTF("_____________________________________________________________________________\n");

   for (int ii = 0; ii < numberOfTimers; ++ii)
   {
      if (perfTimer[ii].count > 0)
         PRINTF("%16s    %6d:%10.4f  %6d:%10.4f  %10.4f  %10.4f\n",
            timerName[ii],
            perfTimer[ii].minRank, perfTimer[ii].minValue*tick,
            perfTimer[ii].maxRank, perfTimer[ii].maxValue*tick,
            perfTimer[ii].average*tick, perfTimer[ii].stdev*tick);
   }
   double atomsPerTask = nGlobalAtoms/(real_t)nRanks;
   perfGlobal.atomRate = perfTimer[timestepTimer].average * tick * 1e6 /
      (atomsPerTask * perfTimer[timestepTimer].count * printRate);
   perfGlobal.atomAllRate = perfTimer[timestepTimer].average * tick * 1e6 /
      (nGlobalAtoms * perfTimer[timestepTimer].count * printRate);
   perfGlobal.atomsPerUSec = 1.0 / perfGlobal.atomAllRate;

   PRINTF("\n---------------------------------------------------\n");
   PRINTF(" Average atom update rate:     %6.2f us/atom/task\n", perfGlobal.atomRate);
   PRINTF("---------------------------------------------------\n\n");

   PRINTF("\n---------------------------------------------------\n");
   PRINTF(" Average all atom update rate: %6.2f us/atom\n", perfGlobal.atomAllRate);
   PRINTF("---------------------------------------------------\n\n");

   PRINTF("\n---------------------------------------------------\n");
   PRINTF(" Average atom rate:            %6.2f atoms/us\n", perfGlobal.atomsPerUSec);
   PRINTF("---------------------------------------------------\n\n");
}

ocrGuid_t printPerformanceResultsEdt( EDT_ARGS )
{
    s64 iStep = paramv[0];
    s32 _idep=0;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_sim = depv[_idep++].guid;
    ocrDBK_t rpPerfTimerEVT = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    SimFlat* sim = depv[_idep++].ptr;
    double* recvBuf = depv[_idep++].ptr;

    sim->atoms = &sim->atoms_INST;
    sim->validate = &sim->validate_INST;

    sim->PTR_rankH = PTR_rankH;

    //double sendBuf[numberOfTimers], recvBuf[numberOfTimers];
    printPerformanceResults(sim->perfTimer, recvBuf, sim->perfGlobal, sim->atoms->nGlobal, sim->printRate, sim->PTR_rankH->myRank, sim->PTR_rankH->nRanks);

    return NULL_GUID;
}

/// Returns current time as a 64-bit integer.  This portable version
/// returns the number of microseconds since mindight, Jamuary 1, 1970.
/// Hence, timing data will have a resolution of 1 microsecond.
/// Platforms with access to calls with lower latency or higher
/// resolution (such as a cycle counter) may wish to replace this
/// implementation and change the conversion factor in getTick as
/// appropriate.
/// \see getTick for the conversion factor between the integer time
/// units of this function and seconds.
static uint64_t getTime(void)
{
   struct timeval ptime;
   uint64_t t = 0;
   gettimeofday(&ptime, (struct timezone *)NULL);
   t = ((uint64_t)1000000)*(uint64_t)ptime.tv_sec + (uint64_t)ptime.tv_usec;

   return t;
}

/// Returns the factor for converting the integer time reported by
/// getTime into seconds.  The portable getTime returns values in units
/// of microseconds so the conversion is simply 1e-6.
/// \see getTime
static double getTick(void)
{
   double seconds_per_cycle = 1.0e-6;
   return seconds_per_cycle;
}

/// Collect timer statistics across ranks.
void timerStats( Timers* perfTimer, double* recvBuf, int myRank, int nRanks )
{
   double sendBuf[numberOfTimers];

   for (int ii = 0; ii < numberOfTimers; ii++)
      perfTimer[ii].average = recvBuf[ii] / (double)nRanks;


   // Determine min and max across ranks and which rank
   RankReduceData reduceSendBuf[numberOfTimers], reduceRecvBuf[numberOfTimers];
   for (int ii = 0; ii < numberOfTimers; ii++)
   {
      //reduceSendBuf[ii].val = (double)perfTimer[ii].total;
      //reduceSendBuf[ii].rank = myRank();
      reduceRecvBuf[ii].val = -1.;
      reduceRecvBuf[ii].rank = -1;
   }
   //minRankDoubleParallel(reduceSendBuf, reduceRecvBuf, numberOfTimers);
   for (int ii = 0; ii < numberOfTimers; ii++)
   {
      perfTimer[ii].minValue = reduceRecvBuf[ii].val;
      perfTimer[ii].minRank = reduceRecvBuf[ii].rank;
   }

   //maxRankDoubleParallel(reduceSendBuf, reduceRecvBuf, numberOfTimers);
   for (int ii = 0; ii < numberOfTimers; ii++)
   {
      perfTimer[ii].maxValue = reduceRecvBuf[ii].val;
      perfTimer[ii].maxRank = reduceRecvBuf[ii].rank;
   }

   // Determine standard deviation
   for (int ii = 0; ii < numberOfTimers; ii++)
   {
      double temp = (double)perfTimer[ii].total - perfTimer[ii].average;
      sendBuf[ii] = temp * temp;
   }
   //addDoubleParallel(sendBuf, recvBuf, numberOfTimers);
   for (int ii = 0; ii < numberOfTimers; ii++)
   {
      perfTimer[ii].stdev = -1.; //sqrt(recvBuf[ii] / (double) getNRanks());
   }
}

void initTimers(Timers* perfTimer)
{
    int i;

    for( i=0; i < numberOfTimers; i++ )
    {
        perfTimer[i].start = 0;
        perfTimer[i].total = 0;
        perfTimer[i].count = 0;
        perfTimer[i].elapsed = 0;

        perfTimer[i].minRank = 0;
        perfTimer[i].maxRank = 0;

        perfTimer[i].minValue = 0;
        perfTimer[i].maxValue = 0;
        perfTimer[i].average = 0;
        perfTimer[i].stdev = 0;
    }
}
