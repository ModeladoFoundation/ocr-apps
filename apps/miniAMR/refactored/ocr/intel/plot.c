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

#include "block.h"
#include "proto.h"
#include "plot.h"

#ifdef NANNY_FUNC_NAMES
#line __LINE__ "plot   "
#endif

// Write block information (level and center) to plot file.  This is done by sending this block's contribution up the line all the way to the root, which aggregates all contributions and writes file.
void plot_BlockContribution(blockClone_Params_t * myParams, ocrEdtDep_t depv[], int timeStep) {

   typedef struct {
      Frame_Header_t myFrame;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__plot_BlockContribution_t;

   blockClone_Deps_t * myDeps  = (blockClone_Deps_t *)   depv;
   BlockMeta_t       * meta    = myDeps->meta_Dep.ptr;

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__plot_BlockContribution_t)

//printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);

#ifdef NANNY_ON_STEROIDS
#define NANNYLEN 200
   char nanny[NANNYLEN];
   sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif

   // Get a datablock for writing this block's contribution toward the plot information.

   gasket__ocrDbCreate(&myDeps->upboundRequest_Dep.guid, (void **) &myDeps->upboundRequest_Dep.ptr, sizeof_Plot_t(1),   __FILE__, __func__, __LINE__, nanny, "upboundRequest");
   meta->cloningState.cloningOpcode = SeasoningOneOrMoreDbCreates;
   SUSPEND__RESUME_IN_CLONE_EDT(;)
   Plot_t * plot = myDeps->upboundRequest_Dep.ptr;

   // Fill in plot contribution HERE!!!

   plot->numBlocks      = 1;
   plot->ts             = timeStep;
   plot->annex[0].level = meta->refinementLevel;
   plot->annex[0].cenx  = meta->cen[0];
   plot->annex[0].ceny  = meta->cen[1];
   plot->annex[0].cenz  = meta->cen[2];

   // This block's contribution to the plot will be passed to the parent as operand, requesting the AccumulatePlot service.
   // In like manner, the partial accumulations will progagate all they way up to the root.

   ocrGuid_t conveyServiceRequestToParent_Event  = meta->conveyServiceRequestToParent_Event;                                  // Jot down the At Bat Event.
   plot->dbCommHeader.serviceOpcode     = Operation_Plot;
   plot->dbCommHeader.timeStep          = timeStep;
   plot->dbCommHeader.atBat_Event       = conveyServiceRequestToParent_Event;                                                 // Convey At Bat Event to parent so that he can destroy the event.
#ifdef NANNY_ON_STEROIDS
   sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
   gasket__ocrEventCreate(&meta->conveyServiceRequestToParent_Event, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG, __FILE__, __func__, __LINE__, nanny, "conveyServiceRequestToParent(On Deck after Plot)");  // Create the On Deck Event; record it in our meta.
   plot->dbCommHeader.onDeck_Event      = meta->conveyServiceRequestToParent_Event;                                           // Convey On Deck Event to parent so that he can make his clone depend upon it.
   gasket__ocrDbRelease(myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "conveyServiceRequestToParent(Plot)");
   gasket__ocrEventSatisfy(conveyServiceRequestToParent_Event, myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "upboundRequest(Plot) via conveyServiceRequestToParent");  // Satisfy the parent's serviceRequest datablock dependence.
   myDeps->upboundRequest_Dep.guid      = NULL_GUID;

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE

} // plot_BlockContribution


void plot_ParentContribution (u32 depc, ocrEdtDep_t depv[]) {
   typedef struct {
      Frame_Header_t myFrame;
      int totalNumBlocks;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__plot_ParentContribution_t;

#define totalNumBlocks (lcl->totalNumBlocks)

   parentClone_Deps_t * myDeps  = (parentClone_Deps_t *) depv;
   ParentMeta_t       * meta    = myDeps->meta_Dep.ptr;

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__plot_ParentContribution_t)

//printf ("Function %36s, File %30s, line %4d, clone=%4d)\n", __func__, __FILE__, __LINE__, meta->cloningState.cloneNum); fflush(stdout);

   int i, j;
   totalNumBlocks = 0;
   for (i = 0; i < 8; i++) {
      Plot_t * contribution = myDeps->serviceRequest_Dep[i].ptr;
      totalNumBlocks += contribution->numBlocks;
   }

#ifdef NANNY_ON_STEROIDS
   char nanny[200];
   sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
   gasket__ocrDbCreate(&myDeps->upboundRequest_Dep.guid, (void **) &myDeps->upboundRequest_Dep.ptr, sizeof_Plot_t(totalNumBlocks),   __FILE__, __func__, __LINE__, nanny, "upboundRequest");
   meta->cloningState.cloningOpcode = SeasoningOneOrMoreDbCreates;
   SUSPEND__RESUME_IN_CLONE_EDT(;)
   Plot_t * plot = myDeps->upboundRequest_Dep.ptr;
   plot->numBlocks = totalNumBlocks;
   Plot_Annex_t * pPlot = &plot->annex[0];

   for (i = 0; i < 8; i++) {
      Plot_t * contribution = myDeps->serviceRequest_Dep[i].ptr;
      plot->ts = contribution->ts;
      for (j = 0; j < contribution->numBlocks; j++) {
          *pPlot++ = contribution->annex[j];
      }
   }

   // This parent's contribution was aggregated from its 2x2x2 children (and grand-children, etc.), and in like manner we now need to propagate the aggregation upward, through parent levels, up to the root.

   ocrGuid_t conveyServiceRequestToParent_Event  = meta->conveyServiceRequestToParent_Event;                                  // Jot down the At Bat Event.
   plot->dbCommHeader.serviceOpcode     = Operation_Plot;
   plot->dbCommHeader.timeStep          = plot->ts;
   plot->dbCommHeader.atBat_Event       = conveyServiceRequestToParent_Event;                                                 // Convey At Bat Event to parent so that he can destroy the event.
#ifdef NANNY_ON_STEROIDS
   sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
   gasket__ocrEventCreate(&meta->conveyServiceRequestToParent_Event, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG, __FILE__, __func__, __LINE__, nanny, "conveyServiceRequestToParent(On Deck after Plot)");  // Create the On Deck Event; record it in our meta.
   plot->dbCommHeader.onDeck_Event      = meta->conveyServiceRequestToParent_Event;                                           // Convey On Deck Event to parent so that he can make his clone depend upon it.
   gasket__ocrDbRelease(myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "conveyServiceRequestToParent(Plot)");
   gasket__ocrEventSatisfy(conveyServiceRequestToParent_Event, myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "upboundRequest(Plot) via conveyServiceRequestToParent");                                    // Satisfy the parent's service request datablock dependence.
   myDeps->upboundRequest_Dep.guid      = NULL_GUID;

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE

} // plot_ParentContribution


void plot_RootFinalAggregation (u32 depc, ocrEdtDep_t depv[]) {

   int total_num_blocks = 0;
   int idep;
   int i, j, n;
   char fname[20];
   Plot_t * operand;
   FILE * fp;

   // Perform the final aggregation of partial contributions to the plot file report.

   rootClone_Deps_t   * myDeps  = (rootClone_Deps_t *)   depv;
   Control_t * control = myDeps->control_Dep.ptr;

//printf ("Function %36s, File %30s, line %4d, clone=%4d)\n", __func__, __FILE__, __LINE__, meta->cloningState.cloneNum); fflush(stdout);

   // First, count how many blocks there are in the mesh
   for (idep = 0; idep < countof_rootClone_AnnexDeps_t; idep++) {
      Plot_t * operand = ((Plot_t *) (myDeps->serviceRequest_Dep[idep].ptr));
      if (operand->dbCommHeader.serviceOpcode != Operation_Plot) {
         printf ("Error!  Inconsistency in opcodes provided to rootClone\n"); fflush(stdout);
         *((int *) 123) = 456;
         ocrShutdown();
      }
      total_num_blocks += operand->numBlocks;
   }

   // Open output file.

   operand = ((Plot_t *) (myDeps->serviceRequest_Dep[0].ptr));

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

   for (idep = 0; idep < countof_rootClone_AnnexDeps_t; idep++) {
      operand = ((Plot_t *) (myDeps->serviceRequest_Dep[idep].ptr));

      // The next line is a single number that, in the reference version, indicates how many active blocks are being processed by an MPI rank.  The OCR
      // version doesn't have any MPI ranks; or another way of looking at it is that every single block is its own "rank".  Lacking anything more sensible
      // to write here, we will simply fill in how many "leaf" blocks exist at the topology below each unrefined block.  This happens to be the very value
      // that is in the operand datablock we are receiving from each child of this root EDT.

      fprintf(fp, "%d\n", operand->numBlocks);

      // Now comes one line per block, consisting of the refinementLevel of the block and its cenx, ceny, and cenz positions.

      for (i = 0; i < operand->numBlocks; i++) {
         fprintf(fp, "%d %d %d %d\n", operand->annex[i].level, operand->annex[i].cenx, operand->annex[i].ceny, operand->annex[i].cenz);
      }
   }

   fclose(fp);

} // plot_RootFinalAggregation
