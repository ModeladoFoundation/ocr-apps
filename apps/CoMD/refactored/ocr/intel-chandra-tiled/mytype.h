/// \file
/// Frequently needed typedefs.

#ifndef __MYTYPE_H_
#define __MYTYPE_H_

#include "ocr.h"

#define _OCR_TASK_FNC_(X) ocrGuid_t X( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
#define EDT_ARGS u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]

#ifdef DEBUG_APP
    #define DEBUG_PRINTF(X) PRINTF X
#else
    #define DEBUG_PRINTF(X) do {} while(0)
#endif

/// \def SINGLE determines whether single or double precision is built
#ifdef SINGLE
typedef float real_t;  //!< define native type for CoMD as single precision
  #define FMT1 "%g"    //!< /def format argument for floats
  #define EMT1 "%e"    //!< /def format argument for eng floats
#else
typedef double real_t; //!< define native type for CoMD as double precision
  #define FMT1 "%f"   //!< \def format argument for doubles
  #define EMT1 "%le"   //!< \def format argument for eng doubles
#endif

typedef real_t real3[3]; //!< a convenience vector with three real_t

static void printSeparator(void)
{
    PRINTF("\n");
}

static void zeroReal3(real3 a)
{
   a[0] = 0.0;
   a[1] = 0.0;
   a[2] = 0.0;
}

typedef ocrGuid_t ocrDBK_t;
typedef ocrGuid_t ocrEVT_t;
typedef ocrGuid_t ocrEDT_t;
typedef ocrGuid_t ocrTML_t;

static void createEventHelper(ocrGuid_t * evtGuid, u32 nbDeps) {
#ifdef WITH_COUNTED_EVT
    ocrEventParams_t params;
    params.EVENT_COUNTED.nbDeps = nbDeps;
    ocrEventCreateParams(evtGuid, OCR_EVENT_COUNTED_T, false, &params);
#else
    ocrEventCreate(evtGuid, OCR_EVENT_STICKY_T, false);
#endif
}

#endif
