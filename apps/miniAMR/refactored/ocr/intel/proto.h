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

#ifndef __PROTO_H__
#define __PROTO_H__

#include "control.h"
#include "root.h"
#include "parent.h"
#include "block.h"
#include "refine.h"
//PROFILE:#include "profile.h"
#include <stdio.h>
//#include "block.h"


// comm.c
void comm                          (blockClone_Params_t * myParams, ocrEdtDep_t depv[], int const start, int const num_comm, int const stage);

// init.c
void init                          (blockClone_Params_t * myParams, ocrEdtDep_t depv[]);

// checksum.c
void checksum_BlockContribution    (blockClone_Params_t * myParams, ocrEdtDep_t depv[], int timeStep);
void checksum_ParentContribution   (u32 depc, ocrEdtDep_t depv[]);
void checksum_RootFinalAggregation (u32 depc, ocrEdtDep_t depv[], rootClone_Params_t * myParams);

// move.c
void move                          (ocrEdtDep_t depv[]);
RefinementDecision_t check_objects (ocrEdtDep_t depv[]);
bool check_block                   (ocrEdtDep_t depv[], double cor[3][2]);

// profile.c
//PROFILE:void propagateProfileResultsUpwardFromParent (ParentMeta_t * meta);
//PROFILE:void reportProfileResults (RootMeta_t * meta);
//PROFILE:void profile_report_perf_2and4 (FILE * fp, Control_t * control, Profile_t * profile, char * version);
//PROFILE:void calculate_results(Profile_t * profile);
//PROFILE:void init_profile(BlockMeta_t * const meta);

// refine.c
void refine                           (blockClone_Params_t * myParams, ocrEdtDep_t depv[], int const ts);
void adjustBlockAndMetaRefineOperation(ocrEdtDep_t depv[], blockClone_Params_t * myParams);
//int refine_level(Globals_t * const glbl);
//void reset_all(Globals_t * const glbl);
//void reset_neighbors(Globals_t * const glbl);
//void redistribute_blocks(Globals_t * const glbl, double * const tp, double * const tm, double * const tu, double * const time, int * const num_moved, int const num_split);


// plot.c
void plot_BlockContribution        (blockClone_Params_t * myParams, ocrEdtDep_t depv[], int timeStep);
void plot_ParentContribution       (u32 depc, ocrEdtDep_t depv[]);
void plot_RootFinalAggregation     (u32 depc, ocrEdtDep_t depv[]);

// stencil.c
void stencil_calc                  (u32 depc, ocrEdtDep_t depv[], int var);

// util.c
double timer(void);
u8 gasket__ocrEdtCreate(ocrGuid_t * guid, int whoAmI_slot, ocrGuid_t templateGuid,
                        u32 paramc, u64* paramv, u32 depc, ocrGuid_t *depv,
                        u16 properties, ocrHint_t *affinity, ocrGuid_t *outputEvent,
                        char       * file,        // File of calling site.
                        const char * func,        // Function of calling site.
                        int          line,        // Line number of calling site.
                        const char * detail,      // Context of the calling site.
                        const char * name);       // Name of the datablock being destroyed.

void gasket__ocrDbCreate ( ocrGuid_t  * guid,     // Guid of created datablock.
                           void      ** addr,     // Address of created datablock.  (Set to NULL if RELAX_DATABLOCK_SEASONING is NOT set, forcing caller to "season" by cloning the EDT.
                           int          size,     // Size of requested datablock in bytes.
                           char       * file,     // File of calling site.
                           const char * func,     // Function of calling site.
                           int          line,     // Line number of calling site.
                           const char * detail,   // Context of the calling site.
                           const char * name);    // Name of the datablock being created.

u8 gasket__ocrEventCreate(ocrGuid_t *guid, ocrEventTypes_t eventType, u16 properties,
                        char       * file,        // File of calling site.
                        const char * func,        // Function of calling site.
                        int          line,        // Line number of calling site.
                        const char * detail,      // Context of the calling site.
                        const char * name);       // Name of the datablock being destroyed.

u8 gasket__ocrDbRelease(ocrGuid_t guid,
                        char       * file,        // File of calling site.
                        const char * func,        // Function of calling site.
                        int          line,        // Line number of calling site.
                        const char * detail,      // Context of the calling site.
                        const char * name);       // Name of the datablock being destroyed.

u8 gasket__ocrAddDependence(ocrGuid_t source, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode,
                        char       * file,        // File of calling site.
                        const char * func,        // Function of calling site.
                        int          line,        // Line number of calling site.
                        const char * detail,      // Context of the calling site.
                        const char * name);       // Name of the datablock being sent to recipient.

u8 gasket__ocrEvtDependence(ocrGuid_t source, ocrGuid_t destination, u32 slot, ocrDbAccessMode_t mode,    // Like AddDependence, but known to be an event rather than a datablock.
                        char       * file,        // File of calling site.                                // Distinguishing allows us to better build the audit.
                        const char * func,        // Function of calling site.
                        int          line,        // Line number of calling site.
                        const char * detail,      // Context of the calling site.
                        const char * name);       // Name of the datablock being sent to recipient.

u8 gasket__ocrEventSatisfy(ocrGuid_t event, ocrGuid_t source,
                        char       * file,        // File of calling site.
                        const char * func,        // Function of calling site.
                        int          line,        // Line number of calling site.
                        const char * detail,      // Context of the calling site.
                        const char * name);       // Name of the datablock being sent to recipient.

void gasket__InductDependence(ocrEdtDep_t whoAmI, ocrEdtDep_t source, u32 slot,
                        char       * file,        // File of calling site.
                        const char * func,        // Function of calling site.
                        int          line,        // Line number of calling site.
                        const char * detail,      // Context of the calling site.
                        const char * name);       // Name of the datablock being sent to recipient.

void gasket__reportEdtDemise(ocrEdtDep_t whoAmI);

u8 gasket__ocrDbDestroy (ocrGuid_t  * guid,     // Guid of datablock to be destroyed.  (We clobber the guid!)
                         void      ** addr,     // Address of datablock.               (We nullify the address!)
                         char       * file,     // File of calling site.
                         const char * func,     // Function of calling site.
                         int          line,     // Line number of calling site.
                         const char * detail,   // Context of the calling site.
                         const char * name);    // Name of the datablock being created.

u8 gasket__ocrEventDestroy (ocrGuid_t  * guid,     // Guid of the event being destroyed.
                            char       * file,     // File of calling site.
                            const char * func,     // Function of calling site.
                            int          line,     // Line number of calling site.
                            const char * detail,   // Context of the calling site.
                            const char * name);    // Name of the datablock brought in by the event being destroyed.

u8 gasket__ocrEventAbandon (ocrGuid_t  * guid,     // Guid of the event being destroyed.   // Like ocrEventDestroy, but this is abandoning (destroying) an event that was never used.
                            char       * file,     // File of calling site.
                            const char * func,     // Function of calling site.
                            int          line,     // Line number of calling site.
                            const char * detail,   // Context of the calling site.
                            const char * name);    // Name of the datablock brought in by the event being destroyed.

void gasket__ocrEventFlush (ocrGuid_t  * guid,     // Guid of the event being destroyed.   // Like ocrEventAbandon, but does NOT destroy the event.  Only ONE side of the comm should abandon; other should flush.
                            char       * file,     // File of calling site.
                            const char * func,     // Function of calling site.
                            int          line,     // Line number of calling site.
                            const char * detail,   // Context of the calling site.
                            const char * name);    // Name of the datablock brought in by the event being destroyed.

void gasket__ocrGuidFromIndex (ocrGuid_t  * guid,     // Guid of the event returned.
                               ocrGuid_t    range,    // Guid of the range.
                               unsigned long index,   // Index into the range
                               char       * file,     // File of calling site.
                               const char * func,     // Function of calling site.
                               int          line,     // Line number of calling site.
                               const char * detail,   // Context of the calling site.
                               const char * name);    // Name of the datablock brought in by the event being destroyed.
#endif // __PROTO_H__
