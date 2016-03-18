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

#include <stdio.h>
#include <math.h>
//#include <mpi.h>
#include "continuationcloner.h"
#include "control.h"
#include "meta.h"
#include "checksum.h"
#include "proto.h"

void transmitBlockContributionForChecksum (BlockMeta_t * meta, int timeStep) {
   typedef struct {
      Frame_Header_t myFrame;
      struct {
         Checksum_t * checksum;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__transmitBlockContributionForChecksum_t;

#define checksum (lcl->pointers.checksum)

   Control_t * control = meta->controlDb.base;
   double (* pCells) /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2] =
           (double(*)/*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2]) (meta->blockDb.base->cells);

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__transmitBlockContributionForChecksum_t)

   checksum = meta->upboundOperandDb.base;

printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);

   if (checksum == NULL) {
      gasket__ocrDbCreate(&meta->upboundOperandDb.dblk, (void **) &meta->upboundOperandDb.base, sizeof_Checksum_t,   __FILE__, __func__, __LINE__);
      meta->upboundOperandDb.size = sizeof_Checksum_t;
      meta->upboundOperandDb.acMd = DB_MODE_RW;                          // Continuation clone will write to checksum datablock.
      meta->cloningState.continuationOpcode = SeasoningOneOrMoreDbCreates;
      SUSPEND__RESUME_IN_CONTINUATION_EDT(;)
      checksum = meta->upboundOperandDb.base;
   }

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
   // In like manner, the partial accumulations will progagate all they way up to the rootProgenitor.

   ocrGuid_t conveyOperandUpward_Event  = meta->conveyOperandUpward_Event;                                           // Jot down the At Bat Event.
   checksum->dbCommHeader.serviceOpcode = Operation_Checksum;
   checksum->dbCommHeader.timeStep      = timeStep;
   checksum->dbCommHeader.atBat_Event   = conveyOperandUpward_Event;                                                 // Convey At Bat Event to parent so that he can destroy the event.
   ocrEventCreate(                       &meta->conveyOperandUpward_Event, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);  // Create the On Deck Event; record it in our meta.
   checksum->dbCommHeader.onDeck_Event  = meta->conveyOperandUpward_Event;                                           // Convey On Deck Event to parent so that he can make his clone depend upon it.
   ocrDbRelease(meta->upboundOperandDb.dblk);
   ocrEventSatisfy(conveyOperandUpward_Event,   meta->upboundOperandDb.dblk);                                        // Satisfy the parent's operand1 dependence.
   meta->upboundOperandDb.base = checksum = NULL;
   meta->upboundOperandDb.dblk = NULL_GUID;
   meta->upboundOperandDb.size = -9999;
   meta->upboundOperandDb.acMd = DB_MODE_NULL;

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE

#undef  checksum
} // transmitBlockContributionForChecksum


void doFinalChecksumAggregationAtRootProgenitor (RootProgenitorMeta_t * meta, rootProgenitorContinuation_Params_t * myParams) {

   typedef struct {
      Frame_Header_t myFrame;
      struct {
         Control_t  * control;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__doFinalChecksumAggregationAtRootProgenitor_t;

#define control (lcl->pointers.control)

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__doFinalChecksumAggregationAtRootProgenitor_t)

   control = meta->controlDb.base;

   // Perform the final accumulation of partial contributions to the checksum.  If this is the first time, we record this as the "golden" checksum;  otherwise we compare it against the "golden"
   // values and complain if they differ.  Note that this particular service does NOT return anything to the children, nor do they expect anything back.  If the checksum is bad, this EDT will
   // simply report the mismatch and shutdown the run.

   int isFirstChecksum = myParams->isFirstChecksum;     // Note whether this is the first time we are calculating a checksum (i.e. the "golden" one) ...
   myParams->isFirstChecksum = 0;                       // ... but tell the continuation EDT that the "golden" one is now in hand.

   // If this is the golden one, we want to accumulate into the golden checksum.  Otherwise, we accumulate into child[0][0][0]'s contribution.

   Checksum_t * accumulator = meta->goldenChecksumDb.base;  // Speculate this being the first time.
   if (!isFirstChecksum) {
      gasket__ocrDbCreate(&meta->scratchChecksumDb.dblk,  (void **) &meta->scratchChecksumDb.base, sizeof_Checksum_t, __FILE__, __func__, __LINE__);
      meta->scratchChecksumDb.acMd = DB_MODE_RW;
      meta->cloningState.continuationOpcode = SeasoningOneOrMoreDbCreates;
      SUSPEND__RESUME_IN_CONTINUATION_EDT(;)
      accumulator = meta->scratchChecksumDb.base;
   }

   int i;
   for (i = 0; i <control->num_vars; i++) {
       accumulator->sum[i] = 0.0;
   }

   int idep;
   for (idep = 0; idep < countof_rootProgenitorContinuation_AnnexDeps_t; idep++) {

      // The operand to Operation_Checksum is the child's contribution to the overall checksum.  Add it into our accumulator.

      Checksum_t * operand      = ((Checksum_t *) (meta->annexDb[idep].serviceRequestOperand.base));

      if (operand->dbCommHeader.serviceOpcode != Operation_Checksum) {
         printf ("Error!  Inconsistency in opcodes provided to continuationRootProgenitor\n"); fflush(stdout);
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
      Checksum_t * operand      = ((Checksum_t *) (meta->annexDb[0].serviceRequestOperand.base));
      if (control->report_diffusion) {
         printf ("%d var %d sum %lf old %lf diff %lf tol %lf\n",
            operand->dbCommHeader.timeStep,
            i,
            accumulator->sum[i],
            meta->goldenChecksumDb.base->sum[i],
            fabs(accumulator->sum[i] - meta->goldenChecksumDb.base->sum[i]) / meta->goldenChecksumDb.base->sum[i],
            control->tol);
      }
      if (fabs(accumulator->sum[i] - meta->goldenChecksumDb.base->sum[i]) / meta->goldenChecksumDb.base->sum[i] > control->tol) {
         printf ("Time step %d sum %lf (old %lf) variable %d difference too large\n",
            operand->dbCommHeader.timeStep,
            accumulator->sum[i],
            meta->goldenChecksumDb.base->sum[i],
            i); fflush(stdout);
         * ((int *) 123) = 456;
         ocrShutdown();
      }
   }
printf ("Grand Total Checksum == %lf\n", total); fflush(stdout);

   if (meta->scratchChecksumDb.dblk != NULL_GUID) {
      ocrDbDestroy(meta->scratchChecksumDb.dblk);  meta->scratchChecksumDb.dblk = NULL_GUID; meta->scratchChecksumDb.base = NULL;
      meta->scratchChecksumDb.acMd = DB_MODE_NULL;
   }

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE

#undef control
} // doFinalChecksumAggregationAtRootProgenitor
