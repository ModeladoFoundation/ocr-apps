/*
Author: Brian R Nickerson
Copyright Intel Corporation 2016

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/


// These are macros that Brian Nickerson uses for productivity when writing OCR application code.

#define ADD_DEPENDENCE(source,dest,depList,depSlotName,mode) \
   if (mode == DB_MODE_NULL && source != NULL_GUID) { \
      printf ("ERROR at %s line %d:  A call to ADD_DEPENDENCE is providing a real dependence (i.e. NOT NULL_GUID) to an EDT but identifying its access mode as DB_MODE_NULL\n", __FILE__, __LINE__); \
      *((int *) 123) = 456; \
   } \
   if (mode != DB_MODE_NULL && source == NULL_GUID) { \
      printf ("ERROR at %s line %d:  A call to ADD_DEPENDENCE is providing a NULL_GUID dependence but identifying its access mode as something other than DB_MODE_NULL\n", __FILE__, __LINE__); \
      *((int *) 123) = 456; \
   } \
   ocrAddDependence(source,dest,offsetof(depList,depSlotName)/sizeof(ocrEdtDep_t),mode);
#define depsCountDivisor   (sizeof(ocrEdtDep_t))
#define paramsCountDivisor (sizeof(u64))
