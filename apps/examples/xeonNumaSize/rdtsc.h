/** This file originally from:
 *    http://www.mcs.anl.gov/~kazutomo/rdtsc.html
 *  then modified locally...
 */

#ifndef __RDTSC_H_DEFINED__
#define __RDTSC_H_DEFINED__


#if defined(__x86_64__)

static __inline__ unsigned long long rdtsc(void)
{
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

#else

#error "No tick counter is available!"

#endif


/*  $RCSfile:  $   $Author: kazutomo $
 *  $Revision: 1.6 $  $Date: 2005/04/13 18:49:58 $
 */

#endif  //  __RDTSC_H_DEFINED__
