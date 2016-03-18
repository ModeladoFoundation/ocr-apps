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
//#include <mpi.h>

#include "block.h"
// TODO:  #include "comm.h"
#include "proto.h"
#include "plot.h"

// Write block information (level and center) to plot file.  This is done by sending this block's contribution up the line all the way to the rootProgenitor, which aggregates all contributions and writes file.
void transmitBlockContributionForPlot(BlockMeta_t * meta, int const ts) {

   // TODO OBTAIN_ACCESS_TO_blocks

   typedef struct {
      Frame_Header_t myFrame;
      struct {
         Plot_t * plot;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__plot_t;

#define plot         (lcl->pointers.plot)

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__plot_t)

   // Get a datablock for writing this block's contribution toward the plot information.
   gasket__ocrDbCreate(&meta->upboundOperandDb.dblk, (void **) &meta->upboundOperandDb.base, sizeof_Plot_t(1),   __FILE__, __func__, __LINE__);
   meta->upboundOperandDb.size = sizeof_Plot_t(1);
   meta->upboundOperandDb.acMd = DB_MODE_RW;                          // Continuation clone will write to plot datablock.
   meta->cloningState.continuationOpcode = SeasoningOneOrMoreDbCreates;
   SUSPEND__RESUME_IN_CONTINUATION_EDT(;)
   plot = meta->upboundOperandDb.base;

   // Fill in plot contribution HERE!!!

   plot->numBlocks      = 1;
   plot->ts             = ts;
   plot->annex[0].level = meta->refinementLevel;
   plot->annex[0].cenx  = meta->cen[0];
   plot->annex[0].ceny  = meta->cen[1];
   plot->annex[0].cenz  = meta->cen[2];

   // This block's contribution to the plot will be passed to the parent as operand, requesting the AccumulatePlot service.
   // In like manner, the partial accumulations will progagate all they way up to the rootProgenitor.

   void * dummy;

   ocrGuid_t conveyOperandUpward_Event = meta->conveyOperandUpward_Event;                                           // Jot down the At Bat Event.
   plot->dbCommHeader.serviceOpcode    = Operation_Plot;
   plot->dbCommHeader.timeStep         = ts;
   plot->dbCommHeader.atBat_Event      = conveyOperandUpward_Event;                                                 // Convey At Bat Event to parent so that he can destroy the event.
   ocrEventCreate(                      &meta->conveyOperandUpward_Event, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);  // Create the On Deck Event; record it in our meta.
   plot->dbCommHeader.onDeck_Event     = meta->conveyOperandUpward_Event;                                           // Convey On Deck Event to parent so that he can make his clone depend upon it.
   ocrDbRelease(meta->upboundOperandDb.dblk);
   ocrEventSatisfy(conveyOperandUpward_Event, meta->upboundOperandDb.dblk);                                         // Satisfy the parent's operand1 dependence.
   meta->upboundOperandDb.base = plot = NULL;
   meta->upboundOperandDb.dblk = NULL_GUID;
   meta->upboundOperandDb.size = -9999;
   meta->upboundOperandDb.acMd = DB_MODE_NULL;

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE

#undef  plot
} // transmitBlockContributionForPlot

void doFinalPlotFileGeneration (RootProgenitorMeta_t * meta) {

   Control_t * control = meta->controlDb.base;
   int total_num_blocks = 0;
   int idep;
   int i, j, n;
   char fname[20];
   Plot_t * operand;
   FILE * fp;

   // Perform the final aggregation of partial contributions to the plot file report.

   // First, count how many blocks there are in the mesh
   for (idep = 0; idep < countof_rootProgenitorContinuation_AnnexDeps_t; idep++) {
      Plot_t * operand = ((Plot_t *) (meta->annexDb[idep].serviceRequestOperand.base));
      if (operand->dbCommHeader.serviceOpcode != Operation_Plot) {
         printf ("Error!  Inconsistency in opcodes provided to continuationRootProgenitor\n"); fflush(stdout);
         *((int *) 123) = 456;
         ocrShutdown();
      }
      total_num_blocks += operand->numBlocks;
   }

   // Open output file.

   operand = ((Plot_t *) (meta->annexDb[0].serviceRequestOperand.base));
   fname[0] = 'p';
   fname[1] = 'l';
   fname[2] = 'o';
   fname[3] = 't';
   fname[4] = '.';
   for (n = 1, j = 0; n < control->num_tsteps; j++, n *= 10) ;
   for (n = 1, i = 0; i <= j; i++, n *= 10)
      fname[5+j-i] = (char) ('0' + (operand->ts/n)%10);
   fname[6+j] = '\0';
   fp = fopen(fname, "w");

   // Write first line of file.  Reference code prints out total_num_blocks, num_refine, npx*init_block_x, npy*init_block_y, and npz*init_z.
   // Analagous elements for OCR native version:
   // total_num_blocks:       We need to count up the total_num_blocks from the plot contributions that were handed up to us by our npx*npy*npz children
   // num_refine:             We have this in our control datablock.
   // npx*init_block_x, etc:  The first term is in our control datablock.  The second term is NOT applicable for the OCR native version.

   fprintf(fp, "%d %d %d %d %d\n", total_num_blocks, control->num_refine, control->npx, control->npy, control->npz);

   for (idep = 0; idep < countof_rootProgenitorContinuation_AnnexDeps_t; idep++) {
      operand = ((Plot_t *) (meta->annexDb[idep].serviceRequestOperand.base));

      // The next line is a single number that, in the reference version, indicates how many active blocks are being processed by an MPI rank.  The OCR
      // version doesn't have any MPI ranks; or another way of looking at it is that every single block is its own "rank".  Lacking anything more sensible
      // to write here, we will simply fill in how many "leaf" blocks exist at the topology below each unrefined block.  This happens to be the very value
      // that is in the operand datablock we are receiving from each child of this rootProgenitor EDT.

      fprintf(fp, "%d\n", operand->numBlocks);

      // Now comes one line per block, consisting of the refinementLevel of the block and its cenx, ceny, and cenz positions.

      for (i = 0; i < operand->numBlocks; i++) {
         fprintf(fp, "%d %d %d %d\n", operand->annex[i].level, operand->annex[i].cenx, operand->annex[i].ceny, operand->annex[i].cenz);
      }
   }

   fclose(fp);

} // doFinalChecksumAggregationAtRootProgenitor
#if 0

   TRACE
   fp = NULL;
   bp = NULL;

// Economize on datablocks.  Alias plot_buf onto tmpDb1.  Rework code to destroy it upon completion.
// Also, file handles (pointers) are opaque, and I cannot make the crawl-out--continuation-edt--crawl-back-in
// logic work correctly if a file is open;  I cannot assure that the file pointer is accurate.  Therefore
// I close the file at every continuation-edt crawl, and reopen it for append access.
#define dbmeta__plot_buf dbmeta__tmpDb1
#define         plot_buf         tmpDb1

   if (!my_pe) {
      fname[0] = 'p';
      fname[1] = 'l';
      fname[2] = 'o';
      fname[3] = 't';
      fname[4] = '.';
      for (n = 1, j = 0; n < num_tsteps; j++, n *= 10) ;
      for (n = 1, i = 0; i <= j; i++, n *= 10)
         fname[5+j-i] = (char) ('0' + (ts/n)%10);
      fname[6+j] = '\0';
      fp = fopen(fname, "w");

      total_num_blocks = 0;
      for (i = 0; i <= num_refine; i++)
         total_num_blocks += num_blocks[i];
      fprintf(fp, "%d %d %d %d %d\n", total_num_blocks, num_refine,
                                      npx*init_block_x, npy*init_block_y,
                                      npz*init_block_z);
      plot_buf_size = 0;
      fprintf(fp, "%d\n", num_active);
      for (n = 0; n < max_active_block; n++)
         if ((bp = &blocks[n])->number >= 0)
            fprintf(fp, "%d %d %d %d\n", bp->level, bp->cen[0],
                                         bp->cen[1], bp->cen[2]);
         bp = NULL;
      fclose(fp);
      fp = NULL;
      for (i = 1; i < num_pes; i++) {
         CALL_SUSPENDABLE_CALLEE(1)
         gasket__mpi_Send(glbl, &size, 1, MPI_INTEGER, i, 1, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
         CALL_SUSPENDABLE_CALLEE(1)
         gasket__mpi_Recv(glbl, &size, 1, MPI_INTEGER, i, 2, MPI_COMM_WORLD, -9999, &status, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
         //if (size > plot_buf_size) {
         //   if (i != 1)
         //      gasket__free(&dbmeta__plot_buf, (void **) &plot_buf, __FILE__, __LINE__);
            plot_buf_size = size;
            gasket__ma_malloc(glbl, &dbmeta__plot_buf, (void *) &plot_buf, 4*plot_buf_size*sizeof(int), __FILE__, __LINE__);
            SUSPEND__RESUME_IN_CONTINUATION_EDT(;) // For the OCR version, this causes the above malloc(s) to be "seasoned", by passing control to a continuation EDT.
         //}
         gasket__mpi_Irecv(glbl, plot_buf, 4*size, MPI_INTEGER, i, 3, MPI_COMM_WORLD, -9999, &request[0], __FILE__, __LINE__);
         CALL_SUSPENDABLE_CALLEE(1)
         gasket__mpi_Send(glbl, &size, 1, MPI_INTEGER, i, 4, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
         CALL_SUSPENDABLE_CALLEE(1)
         gasket__mpi_Wait__for_Irecv(glbl, plot_buf, 4*size, MPI_INTEGER, i, MPI_COMM_WORLD, -9999, &request[0], &status, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
         fp = fopen(fname, "a");
         fprintf(fp, "%d\n", size);
         for (n = j = 0; j < size; j++, n += 4)
            fprintf(fp, "%d %d %d %d\n", plot_buf[n], plot_buf[n+1], plot_buf[n+2], plot_buf[n+3]);
         fclose(fp);
         fp = NULL;
         gasket__free(&dbmeta__plot_buf, (void **) &plot_buf, __FILE__, __LINE__);
      }
   } else {
      total_num_blocks = 0;
      for (i = 0; i <= num_refine; i++)
         total_num_blocks += local_num_blocks[i];
      gasket__ma_malloc(glbl, &dbmeta__plot_buf, (void *) &plot_buf, 4*total_num_blocks*sizeof(int), __FILE__, __LINE__);
      SUSPEND__RESUME_IN_CONTINUATION_EDT(;) // For the OCR version, this causes the above malloc(s) to be "seasoned", by passing control to a continuation EDT.
      for (i = n = 0; n < max_active_block; n++)
         if ((bp = &blocks[n])->number >= 0) {
            plot_buf[i++] = bp->level;
            plot_buf[i++] = bp->cen[0];
            plot_buf[i++] = bp->cen[1];
            plot_buf[i++] = bp->cen[2];
         }
         bp = NULL;
      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Recv(glbl, &size, 1, MPI_INTEGER, 0, 1, MPI_COMM_WORLD, -9999, &status, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Send(glbl, &total_num_blocks, 1, MPI_INTEGER, 0, 2, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Recv(glbl, &size, 1, MPI_INTEGER, 0, 4, MPI_COMM_WORLD, -9999, &status, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Send(glbl, plot_buf, 4*total_num_blocks, MPI_INTEGER, 0, 3, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      /*TODO*/gasket__free(&dbmeta__plot_buf, (void **) &plot_buf, __FILE__, __LINE__);
   }
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  i
#undef  j
#undef  n
#undef  total_num_blocks
#undef  plot_buf_size
#undef  size
#undef  fname
#undef  bp
#undef  fp
#undef  status
}
#endif
