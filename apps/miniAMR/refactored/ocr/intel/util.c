// ************************************************************************
//
// miniAMR: stencil computations with boundary exchange and AMR.
//
// Copyright (2014) Sandia Corporation. Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
// retains certain rights in this software.
//
// Portions Copyright (2016) Intel Corporation.
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <proto.h>
#include <ocr.h>
#include <ocr-guid-functions.h>
#include <extensions/ocr-labeling.h>
//#include <mpi.h>

#ifdef NANNY_FUNC_NAMES
#line __LINE__ "util   "
#endif

double wtime();

double timer(void)
{
   return(wtime());
}


// Gaskets for OCR service calls


u8 gasket__ocrEdtCreate(ocrGuid_t * guid, int whoAmI_slot, ocrGuid_t templateGuid,
                        u32 paramc, u64* paramv, u32 depc, ocrGuid_t *depv,
                        u16 properties, ocrHint_t *affinity, ocrGuid_t *outputEvent,
                        char       * file,     // File of calling site.
                        const char * func,     // Function of calling site.
                        int          line,     // Line number of calling site.
                        const char * detail,   // Context of the calling site.
                        const char * name) {   // Name of the datablock being created.
   u8 returnVal = ocrEdtCreate(guid, templateGuid, paramc, paramv, depc, depv, properties, affinity, outputEvent);
   void * whoAmI;
   ocrGuid_t whoAmI_guid;
   ocrDbCreate(&whoAmI_guid, &whoAmI, sizeof(ocrGuid_t), 0, NULL_HINT, NO_ALLOC);
   *((ocrGuid_t *) whoAmI) = *guid;
   ocrDbRelease(whoAmI_guid);
   ocrAddDependence(whoAmI_guid, *guid, whoAmI_slot, DB_MODE_RO);

#ifdef NANNY_ON_STEROIDS
   printf ("grep -E \"^NOS:EDT:"GUIDF"\" df > XXEDT_"GUIDF"; grep -E -v \"^NOS:EDT:"GUIDF"\" df > dfTemp; mv dfTemp df\n", GUIDA(*guid), GUIDA(*guid), GUIDA(*guid)); fflush(stdout);
   printf ("NOS:EDT:"GUIDF" created   EDT of %s  at %20s line %d in %s context: %s\n", GUIDA(*guid), name, func, line, file, detail); fflush(stdout);
#endif
   return returnVal;
} // gasket__ocrEdtCreate



void gasket__ocrDbCreate (ocrGuid_t  * guid,     // Guid of created datablock.
                          void      ** addr,     // Address of created datablock.  (Set to NULL if RELAX_DATABLOCK_SEASONING is NOT set, forcing caller to "season" by cloning the EDT.
                          int          size,     // Size of requested datablock in bytes.
                          char       * file,     // File of calling site.
                          const char * func,     // Function of calling site.
                          int          line,     // Line number of calling site.
                          const char * detail,   // Context of the calling site.
                          const char * name) {   // Name of the datablock being created.

   ocrDbCreate (guid, addr, size, 0, NULL_HINT, NO_ALLOC);

#ifdef NANNY_ON_STEROIDS
   printf ("grep -E \"^NOS:DBK:"GUIDF"\" df > XXDBK_"GUIDF"; grep -E -v \"^NOS:DBK:"GUIDF"\" df > dfTemp; mv dfTemp df\n", GUIDA(*guid), GUIDA(*guid), GUIDA(*guid)); fflush(stdout);
   printf ("NOS:DBK:"GUIDF" created   DBK of %s  at %20s line %d in %s, addr = %p, size = %d / 0x%x, context: %s\n", GUIDA(*guid), name, func, line, file, *addr, size, size, detail); fflush(stdout);
#endif

//printf ("In %s, called from %s in %s line %d, Allocation size = %d, address is at %p, guid = 0x%lx\n", __func__, func, file, line, size, *block_base, (unsigned long) (*block_dblk));
#ifndef RELAX_DATABLOCK_SEASONING
   *addr = NULL;              // We have to "season" datablocks by passing them to a continuation clone.  NULLify the pointer.  It will become valid in the clone.
#endif
} // gasket__ocrDbCreate


u8 gasket__ocrEventCreate(ocrGuid_t *guid, ocrEventTypes_t eventType, u16 properties,
                        char       * file,        // File of calling site.
                        const char * func,        // Function of calling site.
                        int          line,        // Line number of calling site.
                        const char * detail,      // Context of the calling site.
                        const char * name) {      // Name of the datablock being destroyed.
   u8 returnVal = ocrEventCreate(guid, eventType, properties);
#ifdef NANNY_ON_STEROIDS
   printf ("if [ ! -e \"XXEVT_"GUIDF"\" ]  # grep -E NOS:EVT:"GUIDF"\nthen # grep -E NOS:EVT:"GUIDF"\ngrep -E \"^NOS:EVT:"GUIDF"\" df > XXEVT_"GUIDF"; grep -E -v \"^NOS:EVT:"GUIDF"\" df > dfTemp; mv dfTemp df\nfi  # grep -E NOS:EVT:"GUIDF"\n", GUIDA(*guid), GUIDA(*guid), GUIDA(*guid), GUIDA(*guid), GUIDA(*guid), GUIDA(*guid), GUIDA(*guid)); fflush(stdout);
   printf ("NOS:EVT:"GUIDF" created   EVT of %s  at %20s line %d in %s, context: %s\n", GUIDA(*guid), name, func, line, file, detail); fflush(stdout);
#endif
   return returnVal;
} // gasket__ocrEventCreate


void gasket__ocrGuidFromIndex (ocrGuid_t  * guid,     // Guid of the event returned.
                               ocrGuid_t    range,    // Guid of the range.
                               unsigned long index,   // Index into the range
                               char       * file,     // File of calling site.
                               const char * func,     // Function of calling site.
                               int          line,     // Line number of calling site.
                               const char * detail,   // Context of the calling site.
                               const char * name) {   // Name of the datablock brought in by the event being destroyed.
   ocrGuidFromIndex(guid, range, index);
#ifdef NANNY_ON_STEROIDS
   printf ("if [ ! -e \"XXEVT_"GUIDF"\" ]  # grep -E NOS:EVT:"GUIDF"\nthen # grep -E NOS:EVT:"GUIDF"\ngrep -E \"^NOS:EVT:"GUIDF"\" df > XXEVT_"GUIDF"; grep -E -v \"^NOS:EVT:"GUIDF"\" df > dfTemp; mv dfTemp df\nfi  # grep -E NOS:EVT:"GUIDF"\n", GUIDA(*guid), GUIDA(*guid), GUIDA(*guid), GUIDA(*guid), GUIDA(*guid), GUIDA(*guid), GUIDA(*guid)); fflush(stdout);
   printf ("NOS:EVT:"GUIDF" looked-up from range "GUIDF" index %ld (0x%lx)   EVT of %s  at %20s line %d in %s, context: %s\n", GUIDA(*guid), GUIDA(range), index, index, name, func, line, file, detail); fflush(stdout);
#endif

} // gasket__ocrGuidFromIndex

u8 gasket__ocrDbRelease(ocrGuid_t guid,
                        char       * file,        // File of calling site.
                        const char * func,        // Function of calling site.
                        int          line,        // Line number of calling site.
                        const char * detail,      // Context of the calling site.
                        const char * name) {      // Name of the datablock being destroyed.

#ifdef NANNY_ON_STEROIDS
   printf ("NOS:DBK:"GUIDF" released  DBK of %s  at %20s line %d in %s, context: %s\n", GUIDA(guid), name, func, line, file, detail); fflush(stdout);
#endif

   u8 returnVal = ocrDbRelease(guid);
   return returnVal;

} // gasket__ocrDbRelease


u8 gasket__ocrAddDependence(ocrGuid_t source, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode,
                        char       * file,        // File of calling site.
                        const char * func,        // Function of calling site.
                        int          line,        // Line number of calling site.
                        const char * detail,      // Context of the calling site.
                        const char * name) {      // Name of the datablock being sent to recipient.
   if (ocrGuidIsNull(source)) mode = DB_MODE_NULL;
#ifdef NANNY_ON_STEROIDS
   if (!ocrGuidIsNull(source)) {
      printf ("NOS:DBK:"GUIDF" addDepend DBK of %s  to slot %2d of EDT "GUIDF" provisioning %s access  at %20s line %d in %s, context: %s\n",
              GUIDA(source), name, slot, GUIDA(destination), mode == DB_MODE_RW ? "RW" : mode == DB_MODE_RO ? "RO" : mode == DB_MODE_NULL ? "NULL" : "????", func, line, file, detail); fflush(stdout);
   }
   printf ("NOS:EDT:"GUIDF" slot %d (%s) filled with DBK "GUIDF" for %s access, via addDepend done at %20s line %d in %s, context: %s\n",
          GUIDA(destination), slot, name, GUIDA(source), mode == DB_MODE_RW ? "RW" : mode == DB_MODE_RO ? "RO" : mode == DB_MODE_NULL ? "NULL" : "????", func, line, file, detail); fflush(stdout);
#endif
   u8 returnVal = ocrAddDependence(source, destination, slot, mode);
   return returnVal;
} // gasket__ocrAddDependence


u8 gasket__ocrEvtDependence(ocrGuid_t source, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode,
                        char       * file,        // File of calling site.
                        const char * func,        // Function of calling site.
                        int          line,        // Line number of calling site.
                        const char * detail,      // Context of the calling site.
                        const char * name) {      // Name of the datablock being sent to recipient.
   if (ocrGuidIsNull(source)) mode = DB_MODE_NULL;
#ifdef NANNY_ON_STEROIDS
   if (!ocrGuidIsNull(source)) {
      printf ("NOS:EVT:"GUIDF" addDepend via EVT of %s  to slot %2d of EDT "GUIDF" provisioning %s access  at %20s line %d in %s, context: %s\n",
              GUIDA(source), name, slot, GUIDA(destination), mode == DB_MODE_RW ? "RW" : mode == DB_MODE_RO ? "RO" : mode == DB_MODE_NULL ? "NULL" : "????", func, line, file, detail); fflush(stdout);
   }
   printf ("NOS:EDT:"GUIDF" slot %d (%s) filled with EVT "GUIDF" for %s access, via addDepend done at %20s line %d in %s, context: %s\n",
          GUIDA(destination), slot, name, GUIDA(source), mode == DB_MODE_RW ? "RW" : mode == DB_MODE_RO ? "RO" : mode == DB_MODE_NULL ? "NULL" : "????", func, line, file, detail); fflush(stdout);
#endif
   u8 returnVal = ocrAddDependence(source, destination, slot, mode);
   return returnVal;
} // gasket__ocrEvtDependence


u8 gasket__ocrEventSatisfy(ocrGuid_t event, ocrGuid_t source,
                        char       * file,        // File of calling site.
                        const char * func,        // Function of calling site.
                        int          line,        // Line number of calling site.
                        const char * detail,      // Context of the calling site.
                        const char * name) {      // Name of the datablock being sent to recipient.
#ifdef NANNY_ON_STEROIDS
   printf ("NOS:EVT:"GUIDF" is conveying DBK "GUIDF" of %s to its destination EDT  at %20s line %d in %s,  context: %s\n", GUIDA(event), GUIDA(source), name, func, line, file, detail); fflush(stdout);
   printf ("NOS:DBK:"GUIDF" of %s is being carried by EVT "GUIDF" to its destination EDT  at %20s line %d in %s,  context: %s\n", GUIDA(source), name, GUIDA(event), func, line, file, detail); fflush(stdout);
#endif
   return ocrEventSatisfy(event, source);
} // gasket__ocrEventSatisfy

void gasket__InductDependence(ocrEdtDep_t whoAmI, ocrEdtDep_t source, u32 slot,
                        char       * file,        // File of calling site.
                        const char * func,        // Function of calling site.
                        int          line,        // Line number of calling site.
                        const char * detail,      // Context of the calling site.
                        const char * name) {      // Name of the datablock being sent to recipient.
#ifdef NANNY_ON_STEROIDS
   if (!ocrGuidIsNull(source.guid)) {
      printf ("NOS:DBK:"GUIDF" %s received to slot %d of EDT "GUIDF", at %20s line %d in %s, context: %s\n", GUIDA(source.guid), name, slot, GUIDA(*((ocrGuid_t *) (whoAmI.ptr))), func, line, file, detail); fflush(stdout);
      printf ("NOS:EDT:"GUIDF" received DBK "GUIDF" %s in slot %d, at %20s line %d in %s, context: %s\n", GUIDA(*((ocrGuid_t *) (whoAmI.ptr))), GUIDA(source.guid), name, slot, func, line, file, detail); fflush(stdout);
   }
#endif
} // gasket__InductDependence

void gasket__reportEdtDemise(ocrEdtDep_t whoAmI) {
#ifdef NANNY_ON_STEROIDS
   printf ("NOS:EDT:"GUIDF" done and eliminated: destroyed\n", GUIDA(*((ocrGuid_t *) (whoAmI.ptr)))); fflush(stdout);
#endif
   ocrDbDestroy(whoAmI.guid);
} // gasket__reportEdtDemise

u8 gasket__ocrDbDestroy (ocrGuid_t  * guid,     // Guid of datablock to be destroyed.  (We clobber the guid!)
                         void      ** addr,     // Address of datablock.               (We nullify the address!)
                         char       * file,     // File of calling site.
                         const char * func,     // Function of calling site.
                         int          line,     // Line number of calling site.
                         const char * detail,   // Context of the calling site.
                         const char * name) {   // Name of the datablock being destroyed.
#ifdef NANNY_ON_STEROIDS
   printf ("NOS:DBK:"GUIDF" eliminated: destroyed DBK of %s  at %20s line %d in %s, addr = %p, context: %s\n", GUIDA(*guid), name, func, line, file, *addr, detail); fflush(stdout);
#endif
   u8 returnVal = ocrDbDestroy(*guid);
   *guid = NULL_GUID;
   *addr = NULL;
   return returnVal;
} // gasket__ocrDbDestroy



u8 gasket__ocrEventDestroy (ocrGuid_t  * guid,     // Guid of the event being destroyed.
                            char       * file,     // File of calling site.
                            const char * func,     // Function of calling site.
                            int          line,     // Line number of calling site.
                            const char * detail,   // Context of the calling site.
                            const char * name) {   // Name of the datablock brought in by the event being destroyed.
#ifdef NANNY_ON_STEROIDS
   printf ("NOS:EVT:"GUIDF" eliminated: destroyed EVT of %s  at %20s line %d in %s, context: %s\n", GUIDA(*guid), name, func, line, file, detail); fflush(stdout);
#endif
   u8 returnVal = ocrEventDestroy(*guid);
   *guid = NULL_GUID;
   return returnVal;

} // gasket__ocrEventDestroy


u8 gasket__ocrEventAbandon (ocrGuid_t  * guid,     // Guid of the event being destroyed.   // Like ocrEventDestroy, but this is abandoning (destroying) an event that was never used.
                            char       * file,     // File of calling site.
                            const char * func,     // Function of calling site.
                            int          line,     // Line number of calling site.
                            const char * detail,   // Context of the calling site.
                            const char * name) {   // Name of the datablock brought in by the event being destroyed.
#ifdef NANNY_ON_STEROIDS
   printf ("NOS:EVT:"GUIDF" eliminated: abandoned EVT of %s  at %20s line %d in %s, context: %s\n", GUIDA(*guid), name, func, line, file, detail); fflush(stdout);
#endif
   u8 returnVal = ocrEventDestroy(*guid);
   *guid = NULL_GUID;
   return returnVal;

} // gasket__ocrEventAbandon

void gasket__ocrEventFlush (ocrGuid_t  * guid,     // Guid of the event being destroyed.   // Like ocrEventAbandon, but does NOT destroy the event.  Only ONE side of the comm should abandon; other should flush.
                            char       * file,     // File of calling site.
                            const char * func,     // Function of calling site.
                            int          line,     // Line number of calling site.
                            const char * detail,   // Context of the calling site.
                            const char * name) {   // Name of the datablock brought in by the event being destroyed.
#ifdef NANNY_ON_STEROIDS
   printf ("NOS:EVT:"GUIDF" eliminated: flushed   EVT of %s  at %20s line %d in %s, context: %s\n", GUIDA(*guid), name, func, line, file, detail); fflush(stdout);
#endif
   *guid = NULL_GUID;

} // gasket__ocrEventFlush
