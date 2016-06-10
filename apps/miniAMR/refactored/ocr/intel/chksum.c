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
#include <math.h>
#include <ocr.h>
#include <ocr-guid-functions.h>
#include "clone.h"
#include "control.h"
#include "block.h"
#include "chksum.h"
#include "proto.h"

#ifdef NANNY_FUNC_NAMES
#line __LINE__ "chksum "
#endif

void checksum_BlockContribution (blockClone_Params_t * myParams, ocrEdtDep_t depv[], int timeStep) {
   typedef struct {
      Frame_Header_t myFrame;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__checksum_BlockContribution_t;

   blockClone_Deps_t * myDeps  = (blockClone_Deps_t *) depv;
   BlockMeta_t       * meta    = myDeps->meta_Dep.ptr;
   Control_t         * control = myDeps->control_Dep.ptr;
   Block_t           * block   = myDeps->block_Dep.ptr;
   double (* pCells) /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2] =
           (double(*)/*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2]) (block->cells);

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__checksum_BlockContribution_t)

//printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);

#ifdef NANNY_ON_STEROIDS
#define NANNYLEN 200
   char nanny[NANNYLEN];
   sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif

   gasket__ocrDbCreate(&myDeps->upboundRequest_Dep.guid, (void **) &myDeps->upboundRequest_Dep.ptr, sizeof_Checksum_t,   __FILE__, __func__, __LINE__, nanny, "upboundRequest -- for checksum");
   meta->cloningState.cloningOpcode = SeasoningOneOrMoreDbCreates;
   SUSPEND__RESUME_IN_CLONE_EDT(;)
   Checksum_t * checksum = myDeps->upboundRequest_Dep.ptr;

   int var, i, j, k;

   for (var = 0; var < control->num_vars; var++) {
      double sum = 0.0;
      for (i = 1; i <= control->x_block_size; i++) {
         for (j = 1; j <= control->y_block_size; j++) {
            for (k = 1; k <= control->z_block_size; k++) {
               sum += pCells[var][i][j][k];
            }
         }
      }
      checksum->sum[var] = sum;
   }

   // This block's contribution to the checksum will be passed to the parent as operand, requesting the AccumulateChecksum service.
   // In like manner, the partial accumulations will progagate all they way up to the root.

   ocrGuid_t conveyServiceRequestToParent_Event  = meta->conveyServiceRequestToParent_Event;                                  // Jot down the At Bat Event.
   checksum->dbCommHeader.serviceOpcode = Operation_Checksum;
   checksum->dbCommHeader.timeStep      = timeStep;
   checksum->dbCommHeader.atBat_Event   = conveyServiceRequestToParent_Event;                                                 // Convey At Bat Event to parent so that he can destroy the event.
#ifdef NANNY_ON_STEROIDS
   sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
   gasket__ocrEventCreate(&meta->conveyServiceRequestToParent_Event, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG, __FILE__, __func__, __LINE__, nanny, "conveyServiceRequestToParent(On Deck after Checksum)");  // Create the On Deck Event; record it in our meta.
   checksum->dbCommHeader.onDeck_Event  = meta->conveyServiceRequestToParent_Event;                                           // Convey On Deck Event to parent so that he can make his clone depend upon it.

   static unsigned int seed = 0;
   static int first = 1;
   if (first) {
      first = 0;
      seed = (unsigned int) (meta->xPos + (meta->yPos * control->npx) + (meta->zPos * control->npx * control->npy));
   }
   checksum->dbCommHeader.squawk = rand_r(&seed);
//printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d, squawk=%d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, checksum->dbCommHeader.squawk); fflush(stdout);

   gasket__ocrDbRelease(myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "upboundRequest(Checksum)");
   gasket__ocrEventSatisfy(conveyServiceRequestToParent_Event, myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "upboundRequest(Checksum) via conveyServiceRequestToParent");  // Satisfy the parent's serviceRequest datablock dependence.
   myDeps->upboundRequest_Dep.guid      = NULL_GUID;

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE

} // checksum_BlockContribution


void checksum_ParentContribution (u32 depc, ocrEdtDep_t depv[]) {
   typedef struct {
      Frame_Header_t myFrame;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__checksum_ParentContribution_t;

   parentClone_Deps_t * myDeps  = (parentClone_Deps_t *) depv;
   ParentMeta_t       * meta    = myDeps->meta_Dep.ptr;
   Control_t          * control = myDeps->control_Dep.ptr;

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__checksum_ParentContribution_t)

#ifdef NANNY_ON_STEROIDS
   char nanny[NANNYLEN];
   sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
   gasket__ocrDbCreate(&myDeps->upboundRequest_Dep.guid, (void **) &myDeps->upboundRequest_Dep.ptr, sizeof_Checksum_t,   __FILE__, __func__, __LINE__, nanny, "upboundRequest");
   meta->cloningState.cloningOpcode = SeasoningOneOrMoreDbCreates;
   SUSPEND__RESUME_IN_CLONE_EDT(;)
   Checksum_t * checksum = myDeps->upboundRequest_Dep.ptr;

   static unsigned int seed = 0;
   static int first = 1;
   if (first) {
      first = 0;
      seed = (unsigned int) (meta->xPos + (meta->yPos * control->npx) + (meta->zPos * control->npx * control->npy));
   }
   Checksum_t * contribution = myDeps->serviceRequest_Dep[0].ptr;
   memcpy(checksum, contribution, sizeof_Checksum_t);

   checksum->dbCommHeader.squawk = rand_r(&seed);

   int i, j;
   for (i = 1; i < 8; i++) {
      contribution = myDeps->serviceRequest_Dep[i].ptr;
      for (j = 0; j < control->num_vars; j++) {
          checksum->sum[j] += contribution->sum[j];
      }
   }

   // This parent's contribution was aggregated from its 2x2x2 children (and grand-children, etc.), and in like manner we now need to propagate the aggregation upward, through parent levels, up to the root.

   ocrGuid_t conveyServiceRequestToParent_Event  = meta->conveyServiceRequestToParent_Event;                                  // Jot down the At Bat Event.
   // Already set:  checksum->dbCommHeader.serviceOpcode = Operation_Checksum;
   // Already set:  checksum->dbCommHeader.timeStep      = timeStep;
   checksum->dbCommHeader.atBat_Event   = conveyServiceRequestToParent_Event;                                                 // Convey At Bat Event to parent so that he can destroy the event.
#ifdef NANNY_ON_STEROIDS
   sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
   gasket__ocrEventCreate(&meta->conveyServiceRequestToParent_Event, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG, __FILE__, __func__, __LINE__, nanny, "conveyServiceRequestToParent(On Deck after Checksum)");  // Create the On Deck Event; record it in our meta.
   checksum->dbCommHeader.onDeck_Event  = meta->conveyServiceRequestToParent_Event;                                           // Convey On Deck Event to parent so that he can make his clone depend upon it.
   gasket__ocrDbRelease(myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "upboundRequest(Checksum)");
   gasket__ocrEventSatisfy(conveyServiceRequestToParent_Event, myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "upboundRequest(Checksum) via conveyServiceRequestToParent");  // Satisfy the parent's serviceRequest datablock dependence.
   myDeps->upboundRequest_Dep.guid      = NULL_GUID;

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE

} // checksum_ParentContribution



void checksum_RootFinalAggregation (u32 depc, ocrEdtDep_t depv[], rootClone_Params_t * myParams) {

   typedef struct {
      Frame_Header_t myFrame;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__checksum_RootFinalAggregation_t;

   rootClone_Deps_t   * myDeps  = (rootClone_Deps_t *)   depv;
   RootMeta_t         * meta    = myDeps->meta_Dep.ptr;
   Control_t          * control = myDeps->control_Dep.ptr;
   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__checksum_RootFinalAggregation_t)

   // Perform the final accumulation of partial contributions to the checksum.  If this is the first time, we record this as the "golden" checksum;  otherwise we compare it against the "golden"
   // values and complain if they differ.  Note that this particular service does NOT return anything to the children, nor do they expect anything back.  If the checksum is bad, this EDT will
   // simply report the mismatch and shutdown the run.

   int isFirstChecksum = myParams->isFirstChecksum;     // Note whether this is the first time we are calculating a checksum (i.e. the "golden" one) ...
   myParams->isFirstChecksum = 0;                       // ... but tell the continuation EDT that the "golden" one is now in hand.

   // If this is the golden one, we want to accumulate into the golden checksum.  Otherwise, we accumulate into child[0][0][0]'s contribution.

   Checksum_t * accumulator = myDeps->goldenChecksum_Dep.ptr;  // Speculate this being the first time.
   if (!isFirstChecksum) {
#ifdef NANNY_ON_STEROIDS
   char nanny[NANNYLEN];
   sprintf(nanny, "clone=%5d", meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
      gasket__ocrDbCreate(&myDeps->scratchChecksum_Dep.guid,  (void **) &myDeps->scratchChecksum_Dep.ptr, sizeof_Checksum_t, __FILE__, __func__, __LINE__, nanny, "scratchChecksum");
      meta->cloningState.cloningOpcode = SeasoningOneOrMoreDbCreates;
      SUSPEND__RESUME_IN_CLONE_EDT(;)
      accumulator = myDeps->scratchChecksum_Dep.ptr;
      static unsigned int seed = 0;
      static int first = 1;
      if (first) {
         first = 0;
         seed = (unsigned int) 9999;
      }
      accumulator->dbCommHeader.squawk = rand_r(&seed);
//printf ("Function %36s, File %30s, line %4d, squawk=%d)\n", __func__, __FILE__, __LINE__, accumulator->dbCommHeader.squawk); fflush(stdout);
   }

   int i;
   for (i = 0; i <control->num_vars; i++) {
       accumulator->sum[i] = 0.0;
   }

   int idep;
   int timeStep=-999;
   for (idep = 0; idep < countof_rootClone_AnnexDeps_t; idep++) {

      // The operand to Operation_Checksum is the child's contribution to the overall checksum.  Add it into our accumulator.

      Checksum_t * operand      = ((Checksum_t *) (myDeps->serviceRequest_Dep[idep].ptr));
//printf ("Function %36s, File %30s, line %4d, squawk=%d, %d:%d)\n", __func__, __FILE__, __LINE__, accumulator->dbCommHeader.squawk, idep, operand->dbCommHeader.squawk); fflush(stdout);

      timeStep = operand->dbCommHeader.timeStep;                            // Snag the timeStep for reporting out.

      if (operand->dbCommHeader.serviceOpcode != Operation_Checksum) {
         printf ("Error!  Inconsistency in opcodes provided to rootClone\n"); fflush(stdout);
         *((int *) 123) = 456;
         ocrShutdown();
      }

      for (i = 0; i < control->num_vars; i++) {
          accumulator->sum[i] = accumulator->sum[i] + operand->sum[i];
      }
   }

   double total = 0.0;
   for (i = 0; i < control->num_vars; i++) {
      total += accumulator->sum[i];
      Checksum_t * goldenChecksum = myDeps->goldenChecksum_Dep.ptr;
      if (control->report_diffusion) {
         printf ("%d var %d sum %lf old %lf diff %lf tol %lf\n",
            timeStep,
            i,
            accumulator->sum[i],
            goldenChecksum->sum[i],
            fabs(accumulator->sum[i] - goldenChecksum->sum[i]) / goldenChecksum->sum[i],
            control->tol);
      }
      if (fabs(accumulator->sum[i] - goldenChecksum->sum[i]) / goldenChecksum->sum[i] > control->tol) {
         printf ("Time step %d sum %lf (old %lf) variable %d difference too large\n",
            timeStep,
            accumulator->sum[i],
            goldenChecksum->sum[i],
            i); fflush(stdout);
         * ((int *) 123) = 456;
         ocrShutdown();
      }
   }
//printf ("Grand Total Checksum == %lf at timestep %d\n", total, timeStep); fflush(stdout);

   if (!ocrGuidIsNull(myDeps->scratchChecksum_Dep.guid)) {
      gasket__ocrDbDestroy(&myDeps->scratchChecksum_Dep.guid, &myDeps->scratchChecksum_Dep.ptr, __FILE__, __func__, __LINE__, "scratchChecksum", " "); // Destroy the scratch checksum
   }

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE

} // checksum_RootFinalAggregation
