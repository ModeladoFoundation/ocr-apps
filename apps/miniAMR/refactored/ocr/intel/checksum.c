// TODO: FIXME: Adjust this copyright notice
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

#include <stdio.h>
//#include <mpi.h>
//#include "continuationcloner.h"
#include "control.h"
#include "meta.h"
#include "checksum.h"
#include "proto.h"

void checksum (BlockMeta_t * meta) {
   typedef struct {
      Frame_Header_t myFrame;
      struct {
         Control_t  * control;
         Checksum_t * checksum;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__checksum_t;

#define control (lcl->pointers.control)
#define checksum (lcl->pointers.checksum)

   typedef double BlockCells_t [meta->controlDb.base->num_vars][meta->controlDb.base->x_block_size+2][meta->controlDb.base->y_block_size+2][meta->controlDb.base->z_block_size+2];
   BlockCells_t * pCells = ((BlockCells_t *) (meta->blockDb.base->cells));

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__checksum_t)

   control  = meta->controlDb.base;
   checksum = meta->checksumDb.base;

printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);

   if (checksum == NULL) {
      gasket__ocrDbCreate(&meta->checksumDb.dblk, (void **) &meta->checksumDb.base, sizeof_Checksum_t,   __FILE__, __func__, __LINE__);
      meta->checksumDb.acMd = DB_MODE_RW;                          // Continuation clone will write to checksum datablock.
      meta->cloningState.continuationOpcode = SeasoningOneOrMoreDbCreates;
      SUSPEND__RESUME_IN_CONTINUATION_EDT(;)
      checksum = meta->checksumDb.base;
   }

   int var, i, j, k;

   for (var = 0; var < control->num_vars; var++) {
      double sum = 0.0;
      for (i = 1; i <= control->x_block_size; i++) {
         for (j = 1; j <= control->y_block_size; j++) {
            for (k = 1; k <= control->z_block_size; k++) {
               sum += (*pCells)[var][i][j][k];
            }
         }
      }
      checksum->sum[var] = sum;
   }

   // This block's contribution to the checksum will be passed to the parent as operand, requesting the AccumulateChecksum service.
   // In like manner, the partial accumulations will progagate all they way up to the rootProgenitor.

   void * dummy;

   ocrGuid_t conveyOperand_Event        = meta->conveyOperand_Event;                                           // Jot down the At Bat Event.
   checksum->dbCommHeader.serviceOpcode = Operation_Checksum;
   checksum->dbCommHeader.atBat_Event   = conveyOperand_Event;                                                 // Convey At Bat Event to parent so that he can destroy the event.
   ocrEventCreate(                       &meta->conveyOperand_Event, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);  // Create the On Deck Event; record it in our meta.
   checksum->dbCommHeader.onDeck_Event  = meta->conveyOperand_Event;                                           // Convey On Deck Event to parent so that he can make his clone depend upon it.
   ocrDbRelease(meta->checksumDb.dblk);                           meta->checksumDb.base = checksum = NULL;
   ocrEventSatisfy(conveyOperand_Event,   meta->checksumDb.dblk); meta->checksumDb.dblk = NULL_GUID; meta->checksumDb.acMd = DB_MODE_NULL;  // Satisfy the parent's operand1 dependence.

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE

#undef  control
#undef  checksum
} // checksum
