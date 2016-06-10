/*
Author: Brian R Nickerson
Copyright Intel Corporation 2016

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/

//#define PRINT_GASKET_DEBUG_MESSAGES   // TODO

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ocr.h>
#include <ocr-guid-functions.h>
#include "clone.h"

#ifdef NANNY_FUNC_NAMES
#line __LINE__ "clone  "
#endif

void CastPointerToDbIndexAndOffset(Clone_t * cloningState, ocrEdtDep_t * depv, DbSize_t * dbSize, void * ptrToCast, int * dbIndex, int * offset, char * filename, const char * funcname, int linenum, int ptrnum) {

   if (ptrToCast == NULL) {
      *dbIndex = -1;
      *offset = 0xDEADBEEF;
      return;
   }

   for (*dbIndex = 0; *dbIndex < cloningState->numberOfDatablocks; (*dbIndex)++) {
      if (!ocrGuidIsNull(depv[*dbIndex].guid)) {
         long long ptrOffset = ((long long) ptrToCast) - ((long long) depv[*dbIndex].ptr);
         if (ptrOffset >= 0 && ptrOffset < DbSize(depv[*dbIndex])) {
            *offset = (int) ptrOffset;
            return;
         }
      }
   }

   printf ("ERROR!!!!!  CastPointerToDbIndexAndOffset given a pointer that is NOT to a known datablock.\n");
   printf ("            Note that there's a good chance this is an occurance of an undefined pointer variable.  Initialize it to NULL to fix the problem.\n");
   printf ("file %s, function %s, line %d, ptrnum = %d\n", filename, funcname, linenum, ptrnum);fflush(stdout);
   printf ("Pointer address is %p\n", ptrToCast);
   *((int *) 123) = 456;
} // CastPointerToGuidAndOffset

void CapturePointerBaseAndOffset(Clone_t * cloningState, ocrEdtDep_t * depv, DbSize_t * dbSize, void * ptrToCapture, char * filename, const char * funcname, int linenum, int ptrnum) {
   cloningState->topPtrAdjRec++;
   CastPointerToDbIndexAndOffset(cloningState, depv, dbSize, ptrToCapture, &(cloningState->topPtrAdjRec->serialNum), &(cloningState->topPtrAdjRec->ptrOffset), filename, funcname, linenum, ptrnum);
} // CapturePointerBaseAndOffset


void RestorePointerBaseAndOffset(Clone_t * cloningState, ocrEdtDep_t * depv, void ** ptrToRestore, char * filename, const char * funcname, int linenum, int ptrnum) {
   if (cloningState->topPtrAdjRec->serialNum == -1) {    // NULL pointer:
      *ptrToRestore = NULL;
   } else if (cloningState->topPtrAdjRec->serialNum >= 0 &&
              cloningState->topPtrAdjRec->serialNum < cloningState->numberOfDatablocks) {  // A viable datablock.
      *ptrToRestore = ((void *) (((unsigned long long) depv[cloningState->topPtrAdjRec->serialNum].ptr) + cloningState->topPtrAdjRec->ptrOffset));
   } else {
       printf ("Corrupt serialNum 0x%x taken from 0x%p for pointer in need of adjustment\n", cloningState->topPtrAdjRec->serialNum, cloningState->topPtrAdjRec);fflush(stdout);
       *((int *) 0) = 123;
       ocrShutdown();
   }
   cloningState->topPtrAdjRec--;
} // RestorePointerBaseAndOffset
