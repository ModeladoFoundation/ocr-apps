// TODO: FIXME:  Adjust copyright notice
// ************************************************************************
//
// miniAMR: stencil computations with boundary exchange and AMR.
//
// Copyright (2014) Sandia Corporation. Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
// retains certain rights in this software.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
// Questions? Contact Courtenay T. Vaughan (ctvaugh@sandia.gov)
//                    Richard F. Barrett (rfbarre@sandia.gov)
//
// ************************************************************************

//#define PRINT_GASKET_DEBUG_MESSAGES   // TODO

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ocr.h>
#include "continuationcloner.h"


void CastPointerToDbIndexAndOffset(ContinuationCloning_t * cloningState, void * ptrToCast, int * dbIndex, int * offset, char * filename, const char * funcname, int linenum, int ptrnum) {

   if (ptrToCast == NULL) {
      *dbIndex = -1;
      *offset = 0xDEADBEEF;
      return;
   }

   genericDbCatalogEntry_t * dbCatalog = ((genericDbCatalogEntry_t *) (((unsigned long long) cloningState) + cloningState->offsetToCatalogOfDatablocks));

   for (*dbIndex = 0; *dbIndex < cloningState->numberOfDatablocks; (*dbIndex)++) {
      if (dbCatalog[*dbIndex].dblk != NULL_GUID) {
         long long ptrOffset = ((long long) ptrToCast) - ((long long) dbCatalog[*dbIndex].base);
         if (ptrOffset >= 0 && ptrOffset < dbCatalog[*dbIndex].size) {
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

void CapturePointerBaseAndOffset(ContinuationCloning_t * cloningState, void * ptrToCapture, char * filename, const char * funcname, int linenum, int ptrnum) {
   cloningState->topPtrAdjRec++;
   CastPointerToDbIndexAndOffset(cloningState, ptrToCapture, &(cloningState->topPtrAdjRec->serialNum), &(cloningState->topPtrAdjRec->ptrOffset), filename, funcname, linenum, ptrnum);
} // CapturePointerBaseAndOffset


void RestorePointerBaseAndOffset(ContinuationCloning_t * cloningState, void ** ptrToRestore, char * filename, const char * funcname, int linenum, int ptrnum) {
   genericDbCatalogEntry_t * dbCatalog = ((genericDbCatalogEntry_t *) (((unsigned long long) cloningState) + cloningState->offsetToCatalogOfDatablocks));
   if (cloningState->topPtrAdjRec->serialNum == -1) {    // NULL pointer:
      *ptrToRestore = NULL;
   } else if (cloningState->topPtrAdjRec->serialNum >= 0 &&
              cloningState->topPtrAdjRec->serialNum < cloningState->numberOfDatablocks) {  // A viable datablock.
      *ptrToRestore = ((void *) (((unsigned long long) dbCatalog[cloningState->topPtrAdjRec->serialNum].base) + cloningState->topPtrAdjRec->ptrOffset));
   } else {
       printf ("Corrupt serialNum 0x%x taken from 0x%p for pointer in need of adjustment\n", cloningState->topPtrAdjRec->serialNum, cloningState->topPtrAdjRec);fflush(stdout);
       *((int *) 0) = 123;
       ocrShutdown();
   }
   cloningState->topPtrAdjRec--;
} // RestorePointerBaseAndOffset
