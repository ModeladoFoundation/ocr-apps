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
#include <stdlib.h>

#include "block.h"
//#include "comm.h"
//PROFILE:#include "profile.h"
#include "proto.h"
#include <ocr.h>
#include <ocr-std.h>
#include <ocr-guid-functions.h>
#include <extensions/ocr-labeling.h>

#ifdef NANNY_FUNC_NAMES
#line __LINE__ "comm   "
#endif

// The routines in this file are used in the communication of ghost values between blocks.

// Values of the "code" command-line argument:
#define MinimalSends                   0
#define SendGhosts                     1
#define SendGhosts_ProcessOnSenderSide 2

void comm(blockClone_Params_t * myParams, ocrEdtDep_t depv[], int const start, int const num_comm, int const stage) {

   char permutations[6][3] = { {0, 1, 2}, {1, 2, 0}, {2, 0, 1},
                               {0, 2, 1}, {1, 0, 2}, {2, 1, 0} };

   typedef struct {
      Frame_Header_t myFrame;
      double t1;
      int axisIterator;
      int axis;
      int pole;
      int faceCount;
      int planeCountInBlk;
      int planePitchInBlk;
      int rowCountInPlaneOfBlk;
      int rowPitchInPlaneOfBlk;
      int rowOffsetInPlaneOfBlk;
      int colCountInPlaneOfBlk;
      int colPitchInPlaneOfBlk;
      int colOffsetInPlaneOfBlk;
      int firstRowInPlaneOfBlk;
      int firstColInPlaneOfBlk;
      int rowCountInGst[2];
      int colCountInGst[2];
      int blkPlane;
      int haloPlane;
      int var;
      int row;
      int col;
      int qrtrHaloLeftRight;
      int qrtrHaloUpDown;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__comm_t;

#define t1                       (lcl->t1)
#define axisIterator             (lcl->axisIterator)
#define axis                     (lcl->axis)
#define pole                     (lcl->pole)
#define faceCount                (lcl->faceCount)
#define planeCountInBlk          (lcl->planeCountInBlk)
#define planePitchInBlk          (lcl->planePitchInBlk)
#define rowCountInPlaneOfBlk     (lcl->rowCountInPlaneOfBlk)
#define rowPitchInPlaneOfBlk     (lcl->rowPitchInPlaneOfBlk)
#define rowOffsetInPlaneOfBlk    (lcl->rowOffsetInPlaneOfBlk)
#define colCountInPlaneOfBlk     (lcl->colCountInPlaneOfBlk)
#define colPitchInPlaneOfBlk     (lcl->colPitchInPlaneOfBlk)
#define colOffsetInPlaneOfBlk    (lcl->colOffsetInPlaneOfBlk)
#define rowCountInGst            (lcl->rowCountInGst)
#define colCountInGst            (lcl->colCountInGst)
#define firstRowInPlaneOfBlk     (lcl->firstRowInPlaneOfBlk)
#define firstColInPlaneOfBlk     (lcl->firstColInPlaneOfBlk)
#define blkPlane                 (lcl->blkPlane)
#define haloPlane                (lcl->haloPlane)
#define var                      (lcl->var)
#define row                      (lcl->row)
#define col                      (lcl->col)
#define qrtrHaloLeftRight        (lcl->qrtrHaloLeftRight)
#define qrtrHaloUpDown           (lcl->qrtrHaloUpDown)

   blockClone_Deps_t * myDeps  = (blockClone_Deps_t *) depv;
   BlockMeta_t       * meta    = myDeps->meta_Dep.ptr;
   Control_t         * control = myDeps->control_Dep.ptr;
   Block_t           * block   = myDeps->block_Dep.ptr;

   typedef double BlockCells_flat_t /*[control->num_vars]*/[(control->x_block_size+2)*(control->y_block_size+2)*(control->z_block_size+2)];
   BlockCells_flat_t * cp = ((BlockCells_flat_t *) (block->cells));

#define checkAddressing
#ifdef checkAddressing
   double (* cp4) /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2] =
        (double(*)/*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2]) (block->cells);
#endif

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__comm_t)

//printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d, clone=%4d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum); fflush(stdout);

   for (axisIterator = 0; axisIterator < 3; axisIterator++) {
      if (control->permute)
         axis = permutations[stage%6][axisIterator];
      else
         axis = axisIterator;
      t1 = timer();

      // Calculate the characteristics of the two block faces for this axis.

      switch (axis) {
      case 0:   // Doing halo exchange for East/West faces, i.e. in the x-direction, i.e. writing the planes where the first dimension of the block is 1 and x_blocks_size, and reading 0 and x_block_size+1.
         // Planes of x, comprised of rows of y by columns of z.
         planeCountInBlk      = (control->x_block_size);                                  // x_block_size, i.e. number of planes in the x-directed stack, not counting the two halo planes.
         planePitchInBlk      = (control->y_block_size+2) * (control->z_block_size+2);    // Distance from a row and column in one x-plane to its counterpart in the next.
         rowCountInPlaneOfBlk = (control->y_block_size+2);                                // Number of rows    (aka length of columns) in the plane, including halos
         colCountInPlaneOfBlk = (control->z_block_size+2);                                // Number of columns (aka length of rows)    in the plane, including halos
         rowPitchInPlaneOfBlk = (control->z_block_size+2);                                // Distance from one row    of y in the x-plane to the next row    of y in the same plane.
         colPitchInPlaneOfBlk = 1;                                                        // Distance from one column of z in the x-plane to the next column of z in the same plane.
#ifdef checkAddressing
         if (&cp4[1][3][5][7] != &cp[1][planePitchInBlk*3+rowPitchInPlaneOfBlk*5+colPitchInPlaneOfBlk*7]) {
            printf ("Addressing mismatch, %s line %d, axis = %d, planeCountInBlk = %d, planePitchInBlk = %d rowCountInPlaneOfBlk = %d, colCountInPlaneOfBlk = %d, rowPitchInPlaneOfBlk = %d, colPitchInPlaneOfBlk = %d\n",
                    __FILE__, __LINE__, axis, planeCountInBlk, planePitchInBlk, rowCountInPlaneOfBlk, rowPitchInPlaneOfBlk, colCountInPlaneOfBlk, colPitchInPlaneOfBlk); fflush(stdout);
            *((int *) 123) = 456;
         }
#endif
         break;
      case 1:   // Doing halo exchange for South/North faces, i.e. in the y-direction, i.e. writing the planes where the second dimension of the block is 1 and y_blocks_size, and reading 0 and y_block_size+1.
         // Planes of y, comprised of rows of x by columns of z.
         planeCountInBlk      = (control->y_block_size);                                  // y_block_size, i.e. number of planes in the y-directed stack, not counting the two halo planes.
         planePitchInBlk      = (control->z_block_size+2);                                // Distance from a row and column in one y-plane to its counterpart in the next.
         rowCountInPlaneOfBlk = (control->x_block_size+2);                                // Number of rows    (aka length of columns) in the plane, including halos
         colCountInPlaneOfBlk = (control->z_block_size+2);                                // Number of columns (aka length of rows)    in the plane, including halos
         rowPitchInPlaneOfBlk = (control->y_block_size+2) * (control->z_block_size+2);    // Distance from one row of the y-plane to the next.
         colPitchInPlaneOfBlk = 1;                                                        // Distance from one column of the y-plane to the next.
#ifdef checkAddressing
         if (&cp4[1][3][5][7] != &cp[1][planePitchInBlk*5+rowPitchInPlaneOfBlk*3+colPitchInPlaneOfBlk*7]) {
            printf ("Addressing mismatch, %s line %d, axis = %d, planeCountInBlk = %d, planePitchInBlk = %d rowCountInPlaneOfBlk = %d, rowPitchInPlaneOfBlk = %d, colCountInPlaneOfBlk = %d, colPitchInPlaneOfBlk = %d\n",
                    __FILE__, __LINE__, axis, planeCountInBlk, planePitchInBlk, rowCountInPlaneOfBlk, rowPitchInPlaneOfBlk, colCountInPlaneOfBlk, colPitchInPlaneOfBlk); fflush(stdout);
            *((int *) 123) = 456;
         }
#endif
         break;
      case 2:   // Doing halo exchange for Down/Up faces, i.e. in the z-direction, i.e. writing the planes where the third dimension of the block is 1 and z_blocks_size, and reading 0 and z_block_size+1.
         // Planes of z, comprised of rows of x by columns of y.
         planeCountInBlk      = (control->z_block_size);                                  // z_block_size, i.e. number of planes in the z-directed stack, not counting the two halo planes.
         planePitchInBlk      = 1;                                                        // Distance from a row and column in one z-plane to its counterpart in the next.
         rowCountInPlaneOfBlk = (control->x_block_size+2);                                // Number of rows    (aka length of columns) in the plane, including halos
         colCountInPlaneOfBlk = (control->y_block_size+2);                                // Number of columns (aka length of rows)    in the plane, including halos
         rowPitchInPlaneOfBlk = (control->y_block_size+2) * (control->z_block_size+2);    // Distance from one row of the y-plane to the next.
         colPitchInPlaneOfBlk = (control->z_block_size+2);                                // Distance from one column of the y-plane to the next.
#ifdef checkAddressing
         if (&cp4[1][3][5][7] != &cp[1][planePitchInBlk*7+rowPitchInPlaneOfBlk*3+colPitchInPlaneOfBlk*5]) {
            printf ("Addressing mismatch, %s line %d, axis = %d, planeCountInBlk = %d, planePitchInBlk = %d rowCountInPlaneOfBlk = %d, rowPitchInPlaneOfBlk = %d, colCountInPlaneOfBlk = %d, colPitchInPlaneOfBlk = %d\n",
                    __FILE__, __LINE__, axis, planeCountInBlk, planePitchInBlk, rowCountInPlaneOfBlk, rowPitchInPlaneOfBlk, colCountInPlaneOfBlk, colPitchInPlaneOfBlk); fflush(stdout);
            *((int *) 123) = 456;
         }
#endif
         break;
      }

      // Count how many halo faces and/or quarter faces need to be exchanged along the current axis.
      faceCount = 0;
      for (pole = 0; pole <= 1; pole++) {                                                 // West then East (for axis=0); South then North (for axis=1); Down then Up (for axis=2)
         if (meta->neighborRefinementLevel[axis][pole] == NEIGHBOR_IS_OFF_EDGE_OF_MESH) continue; /* neighbor non-existent */
         faceCount += (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel + 1) ?  /* neighbor finer        */ 4 : /* neighbor same or coarser*/ 1;
      }
      if (faceCount != 0) {
#ifdef NANNY_ON_STEROIDS
#define NANNYLEN 200
         char nanny[NANNYLEN];
         sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif

         gasket__ocrEdtCreate(&meta->blockClone_Edt,                     // Guid of the EDT created to continue at function blockContinuaiton_Func.
                              SLOT(blockClone_Deps_t, whoAmI_Dep),
                              myParams->template.blockClone_Template,    // Template for the EDT we are creating.
                              EDT_PARAM_DEF,
                              (u64 *) myParams,
                              countof_blockClone_fixedDeps_t + faceCount,
                              NULL,
                              EDT_PROP_NONE,
                              NULL_HINT,
                              NULL,
                              __FILE__,
                              __func__,
                              __LINE__,
                              nanny,
                              "blockClone (Continuation -- obtain datablocks for halos out)");
      } else {
         meta->blockClone_Edt = NULL_GUID;
      }

      int auxDepIdx = 0;

      // Calculate the shape of the halo faces or quarter faces, and allocate datablocks for them.

      for (pole = 0; pole <= 1; pole++) {                                                 // West then East (for axis=0); South then North (for axis=1); Down then Up (for axis=2)
         if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel) {        // Neighbor block is at same refinement level.
            if (control->code == MinimalSends) {                                          // Halo exchange will only need to cover the exact face;  no need to expand to include edges and corners.
               rowCountInGst[pole] = rowCountInPlaneOfBlk - 2;
               colCountInGst[pole] = colCountInPlaneOfBlk - 2;
               firstRowInPlaneOfBlk = 1;
               firstColInPlaneOfBlk = 1;
            } else if (control->code == SendGhosts) {   // TODO
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
            } else if (control->code == SendGhosts_ProcessOnSenderSide) {   // TODO
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
            } else {
               printf ("%s line %d, case not yet known!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
            }

            int size = sizeof_Face_BasePart_t + num_comm * rowCountInGst[pole] * colCountInGst[pole] * sizeof(double);
#ifdef NANNY_ON_STEROIDS
            char nanny[200];
            sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, index is pole=%d, axis=%c, nei=sameLvl, [auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, pole, (axis==0)?(pole==0?'W':'E'):((axis==1)?(pole==0?'S':'N'):(pole==0?'D':'U')), auxDepIdx);
#else
#define nanny NULL
#endif
            ocrGuid_t faceOut;
            void * dummy = NULL;
            gasket__ocrDbCreate(&faceOut,  (void **) &dummy, size, __FILE__, __func__, __LINE__, nanny, "faceOut[...]");
            ADD_DEPENDENCE(faceOut, meta->blockClone_Edt, blockClone_Deps_t, aux_Dep[auxDepIdx], DB_MODE_RW, nanny, "faceOut[...]")
            auxDepIdx++;
#ifndef RELAX_DATABLOCK_SEASONING
            meta->cloningState.cloningOpcode = SeasoningOneOrMoreDbCreates;
#endif
         } else if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel - 1) { // Neighbor is coarser.  Our full face provides only a quarter face to it.
            if (control->code == MinimalSends) {                                              // Halo exchange will only need to cover the exact quarter face;  no need to expand to include edges and corners.
               rowCountInGst[pole] = (rowCountInPlaneOfBlk - 2) >> 1;
               colCountInGst[pole] = (colCountInPlaneOfBlk - 2) >> 1;
               firstRowInPlaneOfBlk = 1;
               firstColInPlaneOfBlk = 1;
            } else if (control->code == SendGhosts) {   // TODO
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
            } else if (control->code == SendGhosts_ProcessOnSenderSide) {   // TODO
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
            } else {
               printf ("%s line %d, case not yet known!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
            }

            int size = sizeof_Face_BasePart_t + num_comm * rowCountInGst[pole] * colCountInGst[pole] * sizeof(double);
#ifdef NANNY_ON_STEROIDS
            char nanny[200];
            sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, index is pole=%d, axis=%c, nei=coarser, [auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, pole, (axis==0)?(pole==0?'W':'E'):((axis==1)?(pole==0?'S':'N'):(pole==0?'D':'U')), auxDepIdx);
#else
#define nanny NULL
#endif
            ocrGuid_t faceOut;
            void * dummy = NULL;
            gasket__ocrDbCreate(&faceOut, (void **) &dummy, size, __FILE__, __func__, __LINE__, nanny, "faceOut[...]");
            ADD_DEPENDENCE(faceOut, meta->blockClone_Edt, blockClone_Deps_t, aux_Dep[auxDepIdx], DB_MODE_RW, nanny, "faceOut[...]")
            auxDepIdx++;
#ifndef RELAX_DATABLOCK_SEASONING
            meta->cloningState.cloningOpcode = SeasoningOneOrMoreDbCreates;
#endif
         } else if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel + 1) { // Neighbor is finer.  Our face provides quarter faces to each of four neighbors.
            if (control->code == MinimalSends) {                                              // Halo exchange will only need to cover the exact quarter face;  no need to expand to include edges and corners.
               rowCountInGst[pole] = (rowCountInPlaneOfBlk - 2) >> 1;
               colCountInGst[pole] = (colCountInPlaneOfBlk - 2) >> 1;
               firstRowInPlaneOfBlk = 1;
               firstColInPlaneOfBlk = 1;
            } else if (control->code == SendGhosts) {   // TODO
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
            } else if (control->code == SendGhosts_ProcessOnSenderSide) {   // TODO
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
            } else {
               printf ("%s line %d, case not yet known!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
            }

            int size =  sizeof_Face_BasePart_t + num_comm * rowCountInGst[pole] * colCountInGst[pole] * sizeof(double);
            int qrtr;
            for (qrtr = 0; qrtr < 4; qrtr++) {
#ifdef NANNY_ON_STEROIDS
               char nanny[200];
               sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, index is pole=%d, axis=%c, nei=finer, [auxDepIdx=%d+%d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, pole, (axis==0)?(pole==0?'W':'E'):((axis==1)?(pole==0?'S':'N'):(pole==0?'D':'U')), auxDepIdx, qrtr);
#else
#define nanny NULL
#endif
               ocrGuid_t faceOut;
               void * dummy = NULL;
               gasket__ocrDbCreate(&faceOut, (void **) &dummy, size, __FILE__, __func__, __LINE__, nanny, "faceOut[...]");
               ADD_DEPENDENCE(faceOut, meta->blockClone_Edt, blockClone_Deps_t, aux_Dep[auxDepIdx+qrtr], DB_MODE_RW, nanny, "faceOut[...]")
            }
            auxDepIdx+=4;
#ifndef RELAX_DATABLOCK_SEASONING
            meta->cloningState.cloningOpcode = SeasoningOneOrMoreDbCreates;
#endif
         } else if (meta->neighborRefinementLevel[axis][pole] == NEIGHBOR_IS_OFF_EDGE_OF_MESH) {  // Neighbor is off edge of mesh, i.e. non-existent.  Do nothing now except setup to deal with this case later.
            if (control->code == MinimalSends) {                                                  // Halo replication will only need to cover the exact face;  no need to expand to include edges and corners.
               rowCountInGst[pole] = rowCountInPlaneOfBlk - 2;
               colCountInGst[pole] = colCountInPlaneOfBlk - 2;
               firstRowInPlaneOfBlk = 1;
               firstColInPlaneOfBlk = 1;
            } else if (control->code == SendGhosts) {   // TODO
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
            } else if (control->code == SendGhosts_ProcessOnSenderSide) {   // TODO
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
            } else {
               printf ("%s line %d, case not yet known!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
            }
         } else {
            printf ("%s line %d, error: neighbor is not within one refinement level.\n", __FILE__, __LINE__); fflush(stdout);
            *((int *) 123) = 456;
         }
      }

      // Season the datablocks for the halo data.

#ifndef RELAX_DATABLOCK_SEASONING
      if (meta->cloningState.cloningOpcode == SeasoningOneOrMoreDbCreates) {
         SUSPEND__RESUME_IN_CLONE_EDT(;)
      }
#endif

      // Populate the halo faces

      auxDepIdx = 0;

      for (pole = 0, blkPlane = 1; pole <= 1; pole++, blkPlane = planeCountInBlk) {           // West then East (for axis=0); South then North (for axis=1); Down then Up (for axis=2)
         if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel) {            // Neighbor block is at same refinement level.
            if (control->code != MinimalSends) {                                              // Halo exchange will only need to cover the exact face;  no need to expand to include edges and corners.
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
               // Consider whether the code needs to be different for this case.
            }

            Face_t * faceOut = myDeps->aux_Dep[auxDepIdx++].ptr;
            double * halo = faceOut->cells;
            for (var = 0; var < num_comm; var++) {
               for (row = 0; row < rowCountInGst[pole]; row++) {
                  for (col = 0; col < colCountInGst[pole]; col++) {
                     *halo++ = cp[start+var][(blkPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk) * colPitchInPlaneOfBlk)];
                  }
               }
            }
         } else if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel - 1) { // Neighbor is coarser.  Our full face provides only a quarter face to it.
            if (control->code != MinimalSends) {                                              // Halo exchange will only need to cover the exact face;  no need to expand to include edges and corners.
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
               // Consider whether the code needs to be different for this case.
            }

            Face_t * faceOut = myDeps->aux_Dep[auxDepIdx++].ptr;
            double * halo = faceOut->cells;
            for (var = 0; var < num_comm; var++) {
               for (row = 0; row < rowCountInGst[pole] * 2; row+=2) {
                  for (col = 0; col < colCountInGst[pole] * 2; col+=2) {
                     *halo++ = cp[start+var][(blkPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk    ) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk    ) * colPitchInPlaneOfBlk)] +
                               cp[start+var][(blkPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk    ) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk + 1) * colPitchInPlaneOfBlk)] +
                               cp[start+var][(blkPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk + 1) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk    ) * colPitchInPlaneOfBlk)] +
                               cp[start+var][(blkPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk + 1) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk + 1) * colPitchInPlaneOfBlk)];
                  }
               }
            }
         } else if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel + 1) { // Neighbor is finer.  Our face provides quarter faces to each of four neighbors.
            if (control->code != MinimalSends) {                                              // Halo exchange will only need to cover the exact face;  no need to expand to include edges and corners.
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
               // Consider whether the code needs to be different for this case.
            }
            for (qrtrHaloLeftRight = 0, colOffsetInPlaneOfBlk = 0; qrtrHaloLeftRight <= 1; qrtrHaloLeftRight++, colOffsetInPlaneOfBlk = colCountInGst[pole]) {
               for (qrtrHaloUpDown = 0, rowOffsetInPlaneOfBlk = 0; qrtrHaloUpDown    <= 1; qrtrHaloUpDown++,    rowOffsetInPlaneOfBlk = rowCountInGst[pole]) {
                  Face_t * faceOut = myDeps->aux_Dep[auxDepIdx++].ptr;
                  double * halo = faceOut->cells;
                  for (var = 0; var < num_comm; var++) {
                     for (row = 0; row < rowCountInGst[pole]; row++) {
                        for (col = 0; col < colCountInGst[pole]; col++) {
                           *halo++ = cp[start+var][(blkPlane * planePitchInBlk) +
                                                   ((row + firstRowInPlaneOfBlk + rowOffsetInPlaneOfBlk) * rowPitchInPlaneOfBlk) +
                                                   ((col + firstColInPlaneOfBlk + colOffsetInPlaneOfBlk) * colPitchInPlaneOfBlk)] * 0.25;
                        }
                     }
                  }
               }
            }
         } else if (meta->neighborRefinementLevel[axis][pole] == NEIGHBOR_IS_OFF_EDGE_OF_MESH) {  // Neighbor is off edge of mesh, i.e. non-existent.  Do nothing now; deal with this case later.
            ;
         }
      }

      // Now transmit the halo faces to the neighbors.

      auxDepIdx = 0;

      for (pole = 0, blkPlane = 1; pole <= 1; pole++, blkPlane = planeCountInBlk) {               // West then East (for axis=0); South then North (for axis=1); Down then Up (for axis=2)
         if (meta->neighborRefinementLevel[axis][pole] == NEIGHBOR_IS_OFF_EDGE_OF_MESH) continue; // Neighbor is off edge of mesh, i.e. non-existent.  We do NOT send a halo in that case.
         if (meta->neighborRefinementLevel[axis][pole] <= meta->refinementLevel) {                // Neighbor block is at same refinement level, or one level coarser.  Send ONE halo per axis/pole.
#ifdef NANNY_ON_STEROIDS
            char nanny[NANNYLEN];
            sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, [axis=%d][pole=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole);
#else
#define nanny NULL
#endif
            Face_t * faceOut = myDeps->aux_Dep[auxDepIdx].ptr;
            ocrGuid_t conveyFaceToNeighbor_Event = meta->conveyFaceToNeighbor_Event[axis][pole][0];
            if (ocrGuidIsNull(conveyFaceToNeighbor_Event)) {                                   // First time.  Use labeled guid.
               unsigned long index =                                                           // The first component gets us the index of the base set of 36 applicable for the level: i.e.
                  ((((0111111111111111111111L >> (63L - (meta->refinementLevel*3))) *          // lvl 0: 0;  lvl 1: 1;  lvl 2: 9 (i.e. 1+8);  lvl 2: 73 (i.e. 1+8+8*8); lvl 3: 585 (i.e. 1+8+8*8+8*8*8) ...
                     meta->numBaseResBlocks) +                                                 // ... scaled up by number of blocks in the totally-unrefined mesh.
                    ((((meta->xPos * (control->npy << meta->refinementLevel)) +                // Here we subscript-linearize the x, y, z position of the block within its refinement level and add it
                       meta->yPos) * (control->npz << meta->refinementLevel)) +                // to the above base. ...
                     meta->zPos))  *
                   36) +                                                                       // Then we scale all that by 36 (6 for when nbr coarser; six for when nbr same; 24 for when nbr finer).
                  axis * 2 + pole +                                                            // Now select which of the 36 comm events to use: for nbr coarser, 0=W, 1=E, 2=S, 3=N,  4=D,  5=U, ...
                  (meta->neighborRefinementLevel[axis][pole] < meta->refinementLevel ? 0 : 6); // ... but for nbr at same refinement level:                       6=W, 7=E, 8=S, 9=N, 10=D, 11=U.
               gasket__ocrGuidFromIndex(&conveyFaceToNeighbor_Event, meta->labeledGuidRangeForHaloExchange, index, __FILE__, __func__, __LINE__, nanny, "conveyFaceToNeighbor");
#define DEFAULT_LG_PROPS GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG
               gasket__ocrEventCreate(&conveyFaceToNeighbor_Event, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS, __FILE__, __func__, __LINE__, nanny, "conveyFaceToNeighbor_Event[...][...][0]");  // Create the On Deck Event; record it in our meta.
            }
            faceOut->dbCommHeader.atBat_Event = conveyFaceToNeighbor_Event;                                           // Convey At Bat Event to neighbor so that she can destroy the event.
            gasket__ocrEventCreate(&meta->conveyFaceToNeighbor_Event[axis][pole][0], OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG, __FILE__, __func__, __LINE__, nanny, "conveyFaceToNeighbor_Event[...][...][0]");  // Create the On Deck Event; record it in our meta.
            faceOut->dbCommHeader.onDeck_Event = meta->conveyFaceToNeighbor_Event[axis][pole][0];                     // Convey On Deck Event to neighbor so that she can make her clone depend upon it.
#ifdef NANNY_ON_STEROIDS
            sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d -- [auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, auxDepIdx);
#else
#define nanny NULL
#endif
            gasket__ocrDbRelease(                               myDeps->aux_Dep[auxDepIdx].guid, __FILE__, __func__, __LINE__, nanny, "faceOut[...]");
            gasket__ocrEventSatisfy(conveyFaceToNeighbor_Event, myDeps->aux_Dep[auxDepIdx].guid, __FILE__, __func__, __LINE__, nanny, "faceOut[...] via conveyFaceToNeighbor"); // Satisfy the neighbors's dependence for this face.
            auxDepIdx++;
         } else {                                                                             // Neighbor is finer.  Our face provides quarter faces to each of four neighbors.
            int idep = 0;
            for (qrtrHaloLeftRight = 0; qrtrHaloLeftRight <= 1; qrtrHaloLeftRight++) {        // Left /Right quadrant of quarter-face planes
               for (qrtrHaloUpDown = 0; qrtrHaloUpDown    <= 1; qrtrHaloUpDown++) {           // Upper/Lower quadrant of quarter-face planes
#ifdef NANNY_ON_STEROIDS
                  char nanny[NANNYLEN];
                  sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, [axis=%d][pole=%d][idep=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, idep);
#else
#define nanny NULL
#endif
                  Face_t * faceOut = myDeps->aux_Dep[auxDepIdx+idep].ptr;
                  ocrGuid_t conveyFaceToNeighbor_Event = meta->conveyFaceToNeighbor_Event[axis][pole][idep];
                  if (ocrGuidIsNull(conveyFaceToNeighbor_Event)) {                                   // First time.  Use labeled guid.
                     unsigned long index =                                                           // The first component gets us the index of the base set of 36 applicable for the level: i.e.
                        ((((0111111111111111111111L >> (63L - (meta->refinementLevel*3))) *          // lvl 0: 0;  lvl 1: 1;  lvl 2: 9 (i.e. 1+8);  lvl 2: 73 (i.e. 1+8+8*8); lvl 3: 585 (i.e. 1+8+8*8+8*8*8) ...
                           meta->numBaseResBlocks) +                                                 // ... scaled up by number of blocks in the totally-unrefined mesh.
                          ((((meta->xPos * (control->npy << meta->refinementLevel)) +                // Here we subscript-linearize the x, y, z position of the block within its refinement level and add it
                             meta->yPos) * (control->npz << meta->refinementLevel)) +                // to the above base. ...
                           meta->zPos))  *
                         36) +                                                                       // Then we scale all that by 36 (6 for when nbr coarser; six for when nbr same; 24 for when nbr finer).
                        axis * 8 + pole * 4 + idep + 12;                                             // Now select: 12:15 = W, 16:19 = E, 20:23 = S, 24:27 = N, 28:31 = D, 32:35 = U, neighbor finer.
                     gasket__ocrGuidFromIndex(&conveyFaceToNeighbor_Event, meta->labeledGuidRangeForHaloExchange, index, __FILE__, __func__, __LINE__, nanny, "conveyFaceToNeighbor");
                     gasket__ocrEventCreate(&conveyFaceToNeighbor_Event, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS, __FILE__, __func__, __LINE__, nanny, "conveyFaceToNeighbor_Event[...][...][...]");  // Create the On Deck Event; record it in our meta.
                  }
                  faceOut->dbCommHeader.atBat_Event = conveyFaceToNeighbor_Event;                                                // Convey At Bat Event to neighbor so that she can destroy the event.
                  gasket__ocrEventCreate(&meta->conveyFaceToNeighbor_Event[axis][pole][idep], OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG, __FILE__, __func__, __LINE__, nanny, "conveyFaceToNeighbor_Event[...][...][...]");  // Create the On Deck Event; record it in our meta.
                  faceOut->dbCommHeader.onDeck_Event = meta->conveyFaceToNeighbor_Event[axis][pole][idep];                       // Convey On Deck Event to neighbor so that she can make her clone depend upon it.
#ifdef NANNY_ON_STEROIDS
                  sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, qrtrHaloLeftRight=%d, qrtrHaloUpDown=%d -- [auxDepIdx=%d+%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, qrtrHaloLeftRight, qrtrHaloUpDown, auxDepIdx, idep);
#else
#define nanny NULL
#endif
                  gasket__ocrDbRelease(myDeps->aux_Dep[auxDepIdx+idep].guid,                                __FILE__, __func__, __LINE__, nanny, "faceOut[...]");
                  gasket__ocrEventSatisfy(conveyFaceToNeighbor_Event, myDeps->aux_Dep[auxDepIdx+idep].guid, __FILE__, __func__, __LINE__, nanny, "faceOut[...] via conveyFaceToNeighbor"); // Satisfy the neighbors's dependence for this face.
                  idep++;
               }
            }
            auxDepIdx += 4;
         }
      }

      // Now fill out halo planes at edges of the full mesh.

      for (pole = 0, blkPlane = 1, haloPlane = 0; pole <= 1; pole++, blkPlane = planeCountInBlk, haloPlane = planeCountInBlk+1) {  // W then E (for axis=0); S then N (for axis=1); D then U (for axis=2)
         if (meta->neighborRefinementLevel[axis][pole] == NEIGHBOR_IS_OFF_EDGE_OF_MESH) {     // Neighbor is off edge of mesh.  Deal with it now.
            if (control->code != MinimalSends) {                                              // Halo exchange will only need to cover the exact face;  no need to expand to include edges and corners.
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
               // Consider whether the code needs to be different for this case.
            }

            for (var = 0; var < num_comm; var++) {
               for (row = 0; row < rowCountInGst[pole]; row++) {
                  for (col = 0; col < colCountInGst[pole]; col++) {
                     cp   [start+var][(haloPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk) * colPitchInPlaneOfBlk)] =
                        cp[start+var][(blkPlane  * planePitchInBlk) + ((row + firstRowInPlaneOfBlk) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk) * colPitchInPlaneOfBlk)];
                  }
               }
            }
         }
      }

      // Now set up for our clone to receive halos from neighbors

      if (faceCount != 0) {
#ifdef NANNY_ON_STEROIDS
         char nanny[NANNYLEN];
         sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif

         gasket__ocrEdtCreate(&meta->blockClone_Edt,                     // Guid of the EDT created to continue at function blockContinuaiton_Func.
                              SLOT(blockClone_Deps_t, whoAmI_Dep),
                              myParams->template.blockClone_Template,    // Template for the EDT we are creating.
                              EDT_PARAM_DEF,
                              (u64 *) myParams,
                              countof_blockClone_fixedDeps_t + faceCount,
                              NULL,
                              EDT_PROP_NONE,
                              NULL_HINT,
                              NULL,
                              __FILE__,
                              __func__,
                              __LINE__,
                              nanny,
                              "blockClone (Continuation -- obtain datablocks for halos out)");
      } else {
         meta->blockClone_Edt = NULL_GUID;
      }

      auxDepIdx = 0;

      for (pole = 0, blkPlane = 1; pole <= 1; pole++, blkPlane = planeCountInBlk) { // West then East (for axis=0); South then North (for axis=1); Down then Up (for axis=2)
         if (meta->neighborRefinementLevel[axis][pole] == NEIGHBOR_IS_OFF_EDGE_OF_MESH) continue;  // Neighbor is off edge of mesh, i.e. non-existent.  We do NOT receive a halo in that case.
         meta->cloningState.cloningOpcode = ReceivingACommunication;                // One or both poles does indeed have a neighbor.  Set the cloningOpcode, which we test and act upon after this loop.
         if (meta->neighborRefinementLevel[axis][pole] < meta->refinementLevel) {   // Neighbor block is or one level coarser.  Receive ONE halo per axis/pole.
#ifdef NANNY_ON_STEROIDS
            char nanny[NANNYLEN];
            sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, [auxDepIdx=%d], nei=coarser", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, auxDepIdx);
#else
#define nanny NULL
#endif
            ocrGuid_t faceIn = meta->onDeckToReceiveFace_Event[axis][pole][0];
            meta->onDeckToReceiveFace_Event[axis][pole][0] = NULL_GUID;                       // Assure we don't reuse a stale guid later.
            if (ocrGuidIsNull(faceIn)) {                                                      // First time.  Use labeled guid.
               int nbr_xPos = meta->xPos >> 1;                                                // For starters, our own position at coarser level is just our position scaled down one level.
               int nbr_yPos = meta->yPos >> 1;
               int nbr_zPos = meta->zPos >> 1;
               int qrtrFaceOfNbrToUse = 999;
               if (axis == 0) {                                                               // Then neighbor's position is one away, depending on axis and pole; and we need to note which qrtr-face to use.
                  nbr_xPos += (pole << 1) - 1;                                                // Adjust by -1 or 1.
                  qrtrFaceOfNbrToUse = ((meta->yPos & 1) << 1) + (meta->zPos & 1);
               } else if (axis == 1) {
                  nbr_yPos += (pole << 1) - 1;                                                // Adjust by -1 or 1.
                  qrtrFaceOfNbrToUse = ((meta->xPos & 1) << 1) + (meta->zPos & 1);
               } else {
                  nbr_zPos += (pole << 1) - 1;                                                // Adjust by -1 or 1.
                  qrtrFaceOfNbrToUse = ((meta->xPos & 1) << 1) + (meta->yPos & 1);
               }
               unsigned long index =                                                          // The first component gets us the index of the base set of 36 applicable for the level: i.e.
                  (((((0111111111111111111111L >> (66L - (meta->refinementLevel*3))) *        // lvl 0: 0;  lvl 1: 1;  lvl 2: 9 (i.e. 1+8);  lvl 2: 73 (i.e. 1+8+8*8); lvl 3: 585 (i.e. 1+8+8*8+8*8*8) ...
                      meta->numBaseResBlocks) +                                               // ... scaled up by number of blocks in the totally-unrefined mesh.
                     (((((nbr_xPos) * (control->npy << (meta->refinementLevel-1))) +          // Here we subscript-linearize the x, y, z position of the block within its refinement level and add it
                        (nbr_yPos)) * (control->npz << (meta->refinementLevel-1))) +          // to the above base. ...
                      (nbr_zPos))) *                                                          // ...
                    36) +                                                                     // ... we multiply by 36 (6 for when nbr coarser; six for when nbr same; 24 for when nbr finer).  Then...
                   ((axis * 8 + pole * 4) ^ 4) + qrtrFaceOfNbrToUse + 12);                    // Now select: 12:15 = W, 16:19 = E, 20:23 = S, 24:27 = N, 28:31 = D, 32:35 = U, neighbor finer.
                                                                                              // BUT note that the "^4" operation is to FLIP POLE: my W comms with nbrs E; etc.
               gasket__ocrGuidFromIndex(&faceIn, meta->labeledGuidRangeForHaloExchange, index, __FILE__, __func__, __LINE__, nanny, "faceIn[...]");
               gasket__ocrEventCreate  (&faceIn, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS,         __FILE__, __func__, __LINE__, nanny, "faceIn[...]");
            }
            EVT_DEPENDENCE(faceIn, meta->blockClone_Edt, blockClone_Deps_t, aux_Dep[auxDepIdx], DB_MODE_RO,   nanny, "faceIn[...]")
            auxDepIdx++;
         } else if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel) {             // Neighbor block is at same refinement level.  Receive ONE halo per axis/pole.
#ifdef NANNY_ON_STEROIDS
            char nanny[NANNYLEN];
            sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, [auxDepIdx=%d], nei=sameLvl", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, auxDepIdx);
#else
#define nanny NULL
#endif
            ocrGuid_t faceIn = meta->onDeckToReceiveFace_Event[axis][pole][0];
            meta->onDeckToReceiveFace_Event[axis][pole][0] = NULL_GUID;                       // Assure we don't reuse a stale guid later.
            if (ocrGuidIsNull(faceIn)) {                                                      // First time.  Use labeled guid.
               int nbr_xPos = meta->xPos +                                                    // If NOT doing x-axis halo exchange, neighbor's xPos is the same as ours...
                  ((axis != 0) ? 0 : (pole == 0) ? -1 : 1);                                   // otherwise, it is one off in the direction of the pole being processed.
               int nbr_yPos = meta->yPos +                                                    // If NOT doing y-axis halo exchange, neighbor's yPos is the same as ours...
                  ((axis != 1) ? 0 : (pole == 0) ? -1 : 1);                                   // otherwise, it is one off in the direction of the pole being processed.
               int nbr_zPos = meta->zPos +                                                    // If NOT doing z-axis halo exchange, neighbor's zPos is the same as ours...
                  ((axis != 2) ? 0 : (pole == 0) ? -1 : 1);                                   // otherwise, it is one off in the direction of the pole being processed.
               unsigned long index =                                                          // The first component gets us the index of the base set of 36 applicable for the level: i.e.
                  (((((0111111111111111111111L >> (63L - (meta->refinementLevel*3))) *        // lvl 0: 0;  lvl 1: 1;  lvl 2: 9 (i.e. 1+8);  lvl 2: 73 (i.e. 1+8+8*8); lvl 3: 585 (i.e. 1+8+8*8+8*8*8) ...
                      meta->numBaseResBlocks) +                                               // ... scaled up by number of blocks in the totally-unrefined mesh.
                     (((((nbr_xPos) * (control->npy << (meta->refinementLevel))) +            // Here we subscript-linearize the x, y, z position of the block within its refinement level and add it
                        (nbr_yPos)) * (control->npz << (meta->refinementLevel))) +            // to the above base. ...
                      (nbr_zPos))) *                                                          // ...
                    36) +                                                                     // ... we multiply by 36 (6 for when nbr coarser; six for when nbr same; 24 for when nbr finer).  Then...
                   axis * 2 + pole + 6)                                                       // ... select which of 36 to use: 6=W, 7=E, 8=S, 9=N, 10=D, 11=U; nbr is at same refinement level.
                  ^ 1;                                                                        // BUT FLIP POLE: my W comms with nbrs E; etc.
               gasket__ocrGuidFromIndex(&faceIn, meta->labeledGuidRangeForHaloExchange, index, __FILE__, __func__, __LINE__, nanny, "faceIn[...]");
               gasket__ocrEventCreate  (&faceIn, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS,         __FILE__, __func__, __LINE__, nanny, "faceIn[...]");
            }
            EVT_DEPENDENCE(faceIn, meta->blockClone_Edt, blockClone_Deps_t, aux_Dep[auxDepIdx], DB_MODE_RO,   nanny, "faceIn[...]")
            auxDepIdx++;
         } else {                                                                             // Neighbor is finer.  We receive quarter faces from each of four neighbors.
            int idep = 0;
            for (qrtrHaloLeftRight = 0; qrtrHaloLeftRight <= 1; qrtrHaloLeftRight++) {        // Left /Right quadrant of quarter-face planes
               for (qrtrHaloUpDown = 0; qrtrHaloUpDown    <= 1; qrtrHaloUpDown++) {           // Upper/Lower quadrant of quarter-face planes
#ifdef NANNY_ON_STEROIDS
                  char nanny[NANNYLEN];
                  sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, qrtrLR=%d, qrtrUD=%d, [auxDepIdx=%d+%d], nei=finer", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, qrtrHaloLeftRight, qrtrHaloUpDown, auxDepIdx, idep);
#else
#define nanny NULL
#endif
                  ocrGuid_t faceIn = meta->onDeckToReceiveFace_Event[axis][pole][idep];
                  meta->onDeckToReceiveFace_Event[axis][pole][idep] = NULL_GUID;                    // Assure we don't reuse a stale guid later.
                  if (ocrGuidIsNull(faceIn)) {                                                      // First time.  Use labeled guid.
                     int nbr_xPos = meta->xPos << 1;                                                // For starters, our own position at finer level is just our position scaled up one level.
                     int nbr_yPos = meta->yPos << 1;
                     int nbr_zPos = meta->zPos << 1;
                     if (axis == 0) {
                        nbr_xPos += (pole == 0) ? -1 : 2;
                        nbr_yPos += qrtrHaloLeftRight;
                        nbr_zPos += qrtrHaloUpDown;
                     } else if (axis == 1) {
                        nbr_yPos += (pole == 0) ? -1 : 2;
                        nbr_xPos += qrtrHaloLeftRight;
                        nbr_zPos += qrtrHaloUpDown;
                     } else {
                        nbr_zPos += (pole == 0) ? -1 : 2;
                        nbr_xPos += qrtrHaloLeftRight;
                        nbr_yPos += qrtrHaloUpDown;
                     }
                     unsigned long index =                                                          // The first component gets us the index of the base set of 36 applicable for the level: i.e.
                        (((((0111111111111111111111L >> (60L - (meta->refinementLevel*3))) *        // lvl 0: 0;  lvl 1: 1;  lvl 2: 9 (i.e. 1+8);  lvl 2: 73 (i.e. 1+8+8*8); lvl 3: 585 (i.e. 1+8+8*8+8*8*8) ...
                            meta->numBaseResBlocks) +                                               // ... scaled up by number of blocks in the totally-unrefined mesh.
                           (((((nbr_xPos) * (control->npy << (meta->refinementLevel+1))) +          // Here we subscript-linearize the x, y, z position of the block within its refinement level and add it
                              (nbr_yPos)) * (control->npz << (meta->refinementLevel+1))) +          // to the above base. ...
                            (nbr_zPos))) *                                                          // ...
                          36) +                                                                     // ... we multiply by 36 (6 for when nbr coarser; six for when nbr same; 24 for when nbr finer).  Then...
                         axis * 2 + pole)                                                           // ... select which of 36 to use: 0=W, 1=E, 2=S, 3=N, 4=D, 5=U; nbr's nbr is coarser
                        ^ 1;                                                                        // BUT FLIP POLE: my W comms with nbrs E; etc.
                     gasket__ocrGuidFromIndex(&faceIn, meta->labeledGuidRangeForHaloExchange, index, __FILE__, __func__, __LINE__, nanny, "faceIn[...]");
                     gasket__ocrEventCreate   (&faceIn, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS,        __FILE__, __func__, __LINE__, nanny, "faceIn[...]");
                  }
                  EVT_DEPENDENCE(faceIn, meta->blockClone_Edt, blockClone_Deps_t, aux_Dep[auxDepIdx+idep], DB_MODE_RO,   nanny, "faceIn[...]")
                  idep++;
               }
            }
            auxDepIdx += 4;
         }
      }

      // Now clone, so that our continuation EDT receives the halos

      if (meta->cloningState.cloningOpcode == ReceivingACommunication) {    // If one or both poles does have a neighbor, clone so that we can get the halo(s).
         SUSPEND__RESUME_IN_CLONE_EDT(;)
      }

      // Now move the halo material to its position in the block.  While doing so, also destroy the halo datablocks and the events that brought them to us, and set up the next At-Bat event.

      auxDepIdx = 0;

      for (pole = 0, haloPlane = 0; pole <= 1; pole++, haloPlane = planeCountInBlk + 1) {     // West then East (for axis=0); South then North (for axis=1); Down then Up (for axis=2)
         if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel) {            // Neighbor block is at same refinement level.
            if (control->code != MinimalSends) {                                              // Halo exchange will only need to cover the exact face;  no need to expand to include edges and corners.
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
               // Consider whether the code needs to be different for this case.
            }

            Face_t * faceIn = myDeps->aux_Dep[auxDepIdx].ptr;
            double * halo = faceIn->cells;
            for (var = 0; var < num_comm; var++) {
               for (row = 0; row < rowCountInGst[pole]; row++) {
                  for (col = 0; col < colCountInGst[pole]; col++) {
                     cp[start+var][(haloPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk) * colPitchInPlaneOfBlk)] = *halo++;
                  }
               }
            }
            DbCommHeader_t * faceIn_comm = &faceIn->dbCommHeader;
#ifdef NANNY_ON_STEROIDS
            char nanny[200];
            sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, [auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, auxDepIdx);
#else
#define nanny NULL
#endif
            gasket__ocrEventDestroy(&faceIn_comm->atBat_Event,                                      __FILE__, __func__, __LINE__, nanny, "faceIn[...]"); // Destroy the event that brought us our halo datablock.
            meta->onDeckToReceiveFace_Event[axis][pole][0] = faceIn_comm->onDeck_Event;
            gasket__ocrDbDestroy(&myDeps->aux_Dep[auxDepIdx].guid, &myDeps->aux_Dep[auxDepIdx].ptr, __FILE__, __func__, __LINE__, nanny, "faceIn[...]");
            auxDepIdx++;
         } else if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel - 1) { // Neighbor is coarser.
            if (control->code != MinimalSends) {                                              // Halo exchange will only need to cover the exact face;  no need to expand to include edges and corners.
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
               // Consider whether the code needs to be different for this case.
            }

            // Neighbor provided a qrtr face, which we replicate to our full face.
            Face_t * faceIn = myDeps->aux_Dep[auxDepIdx].ptr;
            double * halo = faceIn->cells;
            for (var = 0; var < num_comm; var++) {
               for (row = 0; row < rowCountInGst[pole] * 2; row+=2) {
                  for (col = 0; col < colCountInGst[pole] * 2; col+=2) {
                     cp[start+var][(haloPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk    ) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk    ) * colPitchInPlaneOfBlk)] = *halo;
                     cp[start+var][(haloPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk    ) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk + 1) * colPitchInPlaneOfBlk)] = *halo;
                     cp[start+var][(haloPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk + 1) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk    ) * colPitchInPlaneOfBlk)] = *halo;
                     cp[start+var][(haloPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk + 1) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk + 1) * colPitchInPlaneOfBlk)] = *halo;
                     halo++;
                  }
               }
            }
            DbCommHeader_t * faceIn_comm = &faceIn->dbCommHeader;
#ifdef NANNY_ON_STEROIDS
            char nanny[200];
            sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, [auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, auxDepIdx);
#else
#define nanny NULL
#endif
            gasket__ocrEventDestroy(&faceIn_comm->atBat_Event,                                      __FILE__, __func__, __LINE__, nanny, "faceIn[...]"); // Destroy the event that brought us our halo datablock.
            meta->onDeckToReceiveFace_Event[axis][pole][0] = faceIn_comm->onDeck_Event;
            gasket__ocrDbDestroy(&myDeps->aux_Dep[auxDepIdx].guid, &myDeps->aux_Dep[auxDepIdx].ptr, __FILE__, __func__, __LINE__, nanny, "faceIn[...]");
            auxDepIdx++;
         } else if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel + 1) { // Neighbor is finer.  We receive four quarter faces.
            if (control->code != MinimalSends) {                                              // Halo exchange will only need to cover the exact face;  no need to expand to include edges and corners.
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
               // Consider whether the code needs to be different for this case.
            }
            int idep = 0;
            for (qrtrHaloLeftRight = 0, colOffsetInPlaneOfBlk = 0; qrtrHaloLeftRight <= 1; qrtrHaloLeftRight++, colOffsetInPlaneOfBlk = colCountInGst[pole]) {
               for (qrtrHaloUpDown = 0, rowOffsetInPlaneOfBlk = 0; qrtrHaloUpDown    <= 1; qrtrHaloUpDown++,    rowOffsetInPlaneOfBlk = rowCountInGst[pole]) {
                  Face_t * faceIn = myDeps->aux_Dep[auxDepIdx+idep].ptr;
                  double * halo = faceIn->cells;
                  for (var = 0; var < num_comm; var++) {
                     for (row = 0; row < rowCountInGst[pole]; row++) {
                        for (col = 0; col < colCountInGst[pole]; col++) {
                           cp[start+var][(haloPlane * planePitchInBlk) +
                                         ((row + firstRowInPlaneOfBlk + rowOffsetInPlaneOfBlk) * rowPitchInPlaneOfBlk) +
                                         ((col + firstColInPlaneOfBlk + colOffsetInPlaneOfBlk) * colPitchInPlaneOfBlk)]  = *halo++;
                        }
                     }
                  }
                  DbCommHeader_t * faceIn_comm = &faceIn->dbCommHeader;
#ifdef NANNY_ON_STEROIDS
                  char nanny[200];
                  sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, qrtrLR=%d, qrtrUD=%d, [auxDepIdx=%d+%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, qrtrHaloLeftRight, qrtrHaloUpDown, auxDepIdx, idep);
#else
#define nanny NULL
#endif
                  gasket__ocrEventDestroy(&faceIn_comm->atBat_Event,                                                __FILE__, __func__, __LINE__, nanny, "faceIn[...]"); // Destroy the event that brought DB.
                  meta->onDeckToReceiveFace_Event[axis][pole][idep] = faceIn_comm->onDeck_Event;
                  gasket__ocrDbDestroy(&myDeps->aux_Dep[auxDepIdx+idep].guid, &myDeps->aux_Dep[auxDepIdx+idep].ptr, __FILE__, __func__, __LINE__, nanny, "faceIn[...]");
                  idep++;
               }
            }
            auxDepIdx += 4;
         }
      }
   }

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE

#undef t1
#undef axisIterator
#undef axis
#undef planeCountInBlk
#undef planePitchInBlk
#undef rowCountInPlaneOfBlk
#undef rowPitchInPlaneOfBlk
#undef rowOffsetInPlaneOfBlk
#undef colCountInPlaneOfBlk
#undef colPitchInPlaneOfBlk
#undef colOffsetInPlaneOfBlk
#undef pole
#undef rowCountInGst
#undef colCountInGst
#undef firstRowInPlaneOfBlk
#undef firstColInPlaneOfBlk
#undef blkPlane
#undef haloPlane
#undef var
#undef row
#undef col
#undef qrtrHaloLeftRight
#undef qrtrHaloUpDown

} // comm
