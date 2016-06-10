/*
Author: Brian R Nickerson
Copyright Intel Corporation 2016

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/

#ifndef __OCR_MACROS_BRN_H__
#define __OCR_MACROS_BRN_H__

#include <ocr.h>
#include <ocr-guid-functions.h>
#include "clone.h"

// These are macros that Brian Nickerson uses for productivity when writing OCR application code.

#define STRINGIFY_X(a) #a
#define STRINGIFY(a) STRINGIFY_X(a)

#define SLOT(depList,depSlotName) offsetof(depList,depSlotName)/sizeof(ocrEdtDep_t)

#define ADD_DEPENDENCE(source,dest,depList,depSlotName,mode,details,name) \
   if (mode == DB_MODE_NULL && !ocrGuidIsNull(source)) { \
      printf ("ERROR at %s line %d:  A call to ADD_DEPENDENCE is providing a real dependence (i.e. NOT NULL_GUID) to an EDT but identifying its access mode as DB_MODE_NULL\n", __FILE__, __LINE__); \
      *((int *) 123) = 456; \
   } \
   gasket__ocrAddDependence(source,dest,SLOT(depList,depSlotName),mode, __FILE__, __func__, __LINE__,details,name);

#define EVT_DEPENDENCE(source,dest,depList,depSlotName,mode,details,name) \
   if (mode == DB_MODE_NULL && !ocrGuidIsNull(source)) { \
      printf ("ERROR at %s line %d:  A call to EVT_DEPENDENCE is providing a real dependence (i.e. NOT NULL_GUID) to an EDT but identifying its access mode as DB_MODE_NULL\n", __FILE__, __LINE__); \
      *((int *) 123) = 456; \
   } \
   gasket__ocrEvtDependence(source,dest,SLOT(depList,depSlotName),mode, __FILE__, __func__, __LINE__,details,name);

#if 0
#define ADD_DEPENDENCE__indexed_depv(source,dest,depNum,mode) \
   if (mode == DB_MODE_NULL && !ocrGuidIsNull(source)) { \
      printf ("ERROR at %s line %d:  A call to ADD_DEPENDENCE is providing a real dependence (i.e. NOT NULL_GUID) to an EDT but identifying its access mode as DB_MODE_NULL\n", __FILE__, __LINE__); fflush(stdout);\
      *((int *) 123) = 456; \
   } \
   if (mode != DB_MODE_NULL && ocrGuidIsNull(source)) { \
      printf ("ERROR at %s line %d:  A call to ADD_DEPENDENCE is providing a NULL_GUID dependence but identifying its access mode as something other than DB_MODE_NULL\n", __FILE__, __LINE__); fflush(stdout);\
      *((int *) 123) = 456; \
   } \
   gasket__ocrAddDependence(source,dest,depNum,mode, __FILE__, __func__, __LINE__, " ");
#endif


#define INDUCT_DEPENDENCE(whoAmI, source,depList,depSlotName,details,name) gasket__InductDependence(whoAmI, source,SLOT(depList,depSlotName),__FILE__,__func__,__LINE__,details,name);
#define REPORT_EDT_DEMISE(whoAmI) gasket__reportEdtDemise(whoAmI);

#define depsCountDivisor   (sizeof(ocrEdtDep_t))
#define paramsCountDivisor (sizeof(u64))



#endif  // __OCR_MACROS_BRN_H__
