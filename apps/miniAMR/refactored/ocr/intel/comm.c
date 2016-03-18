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
//#include <mpi.h>

#include "block.h"
//#include "comm.h"
#include "profile.h"
#include "proto.h"
#include <ocr.h>
#include <ocr-std.h>
#include <extensions/ocr-labeling.h>

// The routines in this file are used in the communication of ghost values between blocks.

// Values of the "code" command-line argument:
#define MinimalSends                   0
#define SendGhosts                     1
#define SendGhosts_ProcessOnSenderSide 2

void dump (BlockMeta_t * meta) {
#if 0


CAREFUL:  "buffer" will overflow if x_block_size is too big

   if (meta->xPos != 0 || meta->yPos != 0 || meta->zPos != 1) return;

   Control_t * control = meta->controlDb.base;
   int i, j, k, l;
   char buffer[200];
   double (* cp4) /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2] =
        (double(*)/*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2]) (meta->blockDb.base->cells);

   for (i = 0; i < 1 /*control->num_vars*/; i++) {
      for (j = 0; j <= control->z_block_size + 1; j++) {
         for (k = 0; k <= control->y_block_size + 1; k++) {
            sprintf (buffer, "XXX var = %d, z = %d, y = %d:", i, j, k);
            for (l = 0; l <= control->x_block_size + 1; l++) {
               sprintf (buffer, "%s  %15f", buffer, cp4[i][l][k][j]);
            }
            printf ("%s\n", buffer); fflush(stdout);
         }
         printf ("\n"); fflush(stdout);
      }
      printf ("\n"); fflush(stdout);
   }
   printf ("\n"); fflush(stdout);
#endif
}

void comm(BlockMeta_t * const meta, int const start, int const num_comm, int const stage) {

   char permutations[6][3] = { {0, 1, 2}, {1, 2, 0}, {2, 0, 1},
                               {0, 2, 1}, {1, 0, 2}, {2, 1, 0} };

   typedef struct {
      Frame_Header_t myFrame;
      double t1;
      int directionIterator;
      int dir;
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
      int pole;
      int rowCountInGst;
      int colCountInGst;
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
#define directionIterator        (lcl->directionIterator)
#define dir                      (lcl->dir)
#define planeCountInBlk          (lcl->planeCountInBlk)
#define planePitchInBlk          (lcl->planePitchInBlk)
#define rowCountInPlaneOfBlk     (lcl->rowCountInPlaneOfBlk)
#define rowPitchInPlaneOfBlk     (lcl->rowPitchInPlaneOfBlk)
#define rowOffsetInPlaneOfBlk    (lcl->rowOffsetInPlaneOfBlk)
#define colCountInPlaneOfBlk     (lcl->colCountInPlaneOfBlk)
#define colPitchInPlaneOfBlk     (lcl->colPitchInPlaneOfBlk)
#define colOffsetInPlaneOfBlk    (lcl->colOffsetInPlaneOfBlk)
#define pole                     (lcl->pole)
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

   Control_t * control = meta->controlDb.base;

   typedef double BlockCells_flat_t /*[control->num_vars]*/[(control->x_block_size+2)*(control->y_block_size+2)*(control->z_block_size+2)];
   BlockCells_flat_t * cp = ((BlockCells_flat_t *) (meta->blockDb.base->cells));

#define checkAddressing
#ifdef checkAddressing
   double (* cp4) /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2] =
        (double(*)/*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2]) (meta->blockDb.base->cells);
#endif

//printf ("num_vars = %d, x/y/z block_size = %d %d %d.  sizeof cp = %d, sizeof cp4 = %d.  cp at %p, cp4 at %p\n", control->num_vars, control->x_block_size, control->y_block_size, control->z_block_size,
//            sizeof(BlockCells_flat_t), sizeof(cp4), cp, cp4); fflush(stdout);

//printf ("<<<<<< at line %d,   zPos = %d,  %p %p\n", __LINE__, meta->zPos, meta->faceDb[0][0][0].base, meta->faceDb[0][0][1].base); fflush(stdout);
   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__comm_t)

//printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);

dump(meta);

   for (directionIterator = 0; directionIterator < 3; directionIterator++) {
//printf ("<<<<<< at line %d,   zPos = %d,  %p %p\n", __LINE__, meta->zPos, meta->faceDb[0][0][0].base, meta->faceDb[0][0][1].base); fflush(stdout);
      if (control->permute)
         dir = permutations[stage%6][directionIterator];
      else
         dir = directionIterator;
      t1 = timer();

      // Calculate the characteristics of the two block faces for this dir.
//printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);

      switch (dir) {
      case 0:   // Doing halo exchange for East/West faces, i.e. in the x-direction, i.e. writing the planes where the first dimension of the block is 1 and x_blocks_size, and reading 0 and x_block_size+1.
         // Planes of x, comprised of rows of y by columns of z.
         planeCountInBlk = (control->x_block_size);                                  // x_block_size, i.e. number of planes in the x-directed stack, not counting the two halo planes.
         planePitchInBlk = (control->y_block_size+2) * (control->z_block_size+2);    // Distance from a row and column in one x-plane to its counterpart in the next.
         rowCountInPlaneOfBlk   = (control->y_block_size+2);                                // Number of rows    (aka length of columns) in the plane, including halos
         colCountInPlaneOfBlk   = (control->z_block_size+2);                                // Number of columns (aka length of rows)   in the plane, including halos
         rowPitchInPlaneOfBlk   = (control->z_block_size+2);                                // Distance from one row    of y in the x-plane to the next row    of y in the same plane.
         colPitchInPlaneOfBlk   = 1;                                                        // Distance from one column of z in the x-plane to the next column of z in the same plane.
#ifdef checkAddressing
         if (&cp4[1][3][5][7] != &cp[1][planePitchInBlk*3+rowPitchInPlaneOfBlk*5+colPitchInPlaneOfBlk*7]) {
            printf ("Addressing mismatch, %s line %d, dir = %d, planeCountInBlk = %d, planePitchInBlk = %d rowCountInPlaneOfBlk = %d, colCountInPlaneOfBlk = %d, rowPitchInPlaneOfBlk = %d, colPitchInPlaneOfBlk = %d\n",
                    __FILE__, __LINE__, dir, planeCountInBlk, planePitchInBlk, rowCountInPlaneOfBlk, rowPitchInPlaneOfBlk, colCountInPlaneOfBlk, colPitchInPlaneOfBlk); fflush(stdout);
            *((int *) 123) = 456;
         }
#endif
         break;
      case 1:   // Doing halo exchange for North/South faces, i.e. in the y-direction, i.e. writing the planes where the second dimension of the block is 1 and y_blocks_size, and reading 0 and y_block_size+1.
         // Planes of y, comprised of rows of x by columns of z.
         planeCountInBlk = (control->y_block_size);                                  // y_block_size, i.e. number of planes in the y-directed stack, not counting the two halo planes.
         planePitchInBlk = (control->z_block_size+2);                                // Distance from a row and column in one y-plane to its counterpart in the next.
         rowCountInPlaneOfBlk   = (control->x_block_size+2);                                // Number of rows    (aka length of columns) in the plane, including halos
         colCountInPlaneOfBlk   = (control->z_block_size+2);                                // Number of columns (aka length of rows)    in the plane, including halos
         rowPitchInPlaneOfBlk   = (control->y_block_size+2) * (control->z_block_size+2);    // Distance from one row of the y-plane to the next.
         colPitchInPlaneOfBlk   = 1;                                                        // Distance from one column of the y-plane to the next.
#ifdef checkAddressing
         if (&cp4[1][3][5][7] != &cp[1][planePitchInBlk*5+rowPitchInPlaneOfBlk*3+colPitchInPlaneOfBlk*7]) {
            printf ("Addressing mismatch, %s line %d, dir = %d, planeCountInBlk = %d, planePitchInBlk = %d rowCountInPlaneOfBlk = %d, rowPitchInPlaneOfBlk = %d, colCountInPlaneOfBlk = %d, colPitchInPlaneOfBlk = %d\n",
                    __FILE__, __LINE__, dir, planeCountInBlk, planePitchInBlk, rowCountInPlaneOfBlk, rowPitchInPlaneOfBlk, colCountInPlaneOfBlk, colPitchInPlaneOfBlk); fflush(stdout);
            *((int *) 123) = 456;
         }
#endif
         break;
      case 2:   // Doing halo exchange for Up/Down faces, i.e. in the z-direction, i.e. writing the planes where the third dimension of the block is 1 and z_blocks_size, and reading 0 and z_block_size+1.
         // Planes of z, comprised of rows of x by columns of y.
         planeCountInBlk = (control->z_block_size);                                  // z_block_size, i.e. number of planes in the z-directed stack, not counting the two halo planes.
         planePitchInBlk = 1;                                                        // Distance from a row and column in one z-plane to its counterpart in the next.
         rowCountInPlaneOfBlk   = (control->x_block_size+2);                                // Number of rows    (aka length of columns) in the plane, including halos
         colCountInPlaneOfBlk   = (control->y_block_size+2);                                // Number of columns (aka length of rows)    in the plane, including halos
         rowPitchInPlaneOfBlk   = (control->y_block_size+2) * (control->z_block_size+2);    // Distance from one row of the y-plane to the next.
         colPitchInPlaneOfBlk   = (control->z_block_size+2);                                // Distance from one column of the y-plane to the next.
#ifdef checkAddressing
         if (&cp4[1][3][5][7] != &cp[1][planePitchInBlk*7+rowPitchInPlaneOfBlk*3+colPitchInPlaneOfBlk*5]) {
            printf ("Addressing mismatch, %s line %d, dir = %d, planeCountInBlk = %d, planePitchInBlk = %d rowCountInPlaneOfBlk = %d, rowPitchInPlaneOfBlk = %d, colCountInPlaneOfBlk = %d, colPitchInPlaneOfBlk = %d\n",
                    __FILE__, __LINE__, dir, planeCountInBlk, planePitchInBlk, rowCountInPlaneOfBlk, rowPitchInPlaneOfBlk, colCountInPlaneOfBlk, colPitchInPlaneOfBlk); fflush(stdout);
            *((int *) 123) = 456;
         }
#endif
         break;
      }

      // Calculate the shape of the halo faces or quarter faces, and allocate datablocks for them.

      for (pole = 0; pole <= 1; pole++) {                                                     // West then East (for dir=0); South then North (for dir=1); Down then Up (for dir=2)
         if (meta->neighborRefinementLevel[dir][pole] == meta->refinementLevel) {             // Neighbor block is at same refinement level.
            if (control->code == MinimalSends) {                                              // Halo exchange will only need to cover the exact face;  no need to expand to include edges and corners.
               rowCountInGst = rowCountInPlaneOfBlk - 2;
               colCountInGst = colCountInPlaneOfBlk - 2;
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

            meta->faceDb[0][0][pole].size = sizeof_Face_BasePart_t + num_comm * rowCountInGst * colCountInGst * sizeof(double);
            meta->faceDb[0][0][pole].acMd = DB_MODE_RW;
            gasket__ocrDbCreate(&meta->faceDb[0][0][pole].dblk,  (void **) &meta->faceDb[0][0][pole].base, meta->faceDb[0][0][pole].size, __FILE__, __func__, __LINE__);
#ifndef RELAX_DATABLOCK_SEASONING
            meta->cloningState.continuationOpcode = SeasoningOneOrMoreDbCreates;
#endif
//printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);
         } else if (meta->neighborRefinementLevel[dir][pole] == meta->refinementLevel - 1) {  // Neighbor is coarser.  Our full face provides only a quarter face to it.
*((int *) 123) = 456;
            if (control->code == MinimalSends) {                                              // Halo exchange will only need to cover the exact face;  no need to expand to include edges and corners.
               rowCountInGst = (rowCountInPlaneOfBlk - 2) >> 1;
               colCountInGst = (colCountInPlaneOfBlk - 2) >> 1;
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

            meta->faceDb[0][0][pole].size = sizeof_Face_BasePart_t + num_comm * rowCountInGst * colCountInGst * sizeof(double);
            meta->faceDb[0][0][pole].acMd = DB_MODE_RW;
            gasket__ocrDbCreate(&meta->faceDb[0][0][pole].dblk,  (void **) &meta->faceDb[0][0][pole].base, meta->faceDb[0][0][pole].size, __FILE__, __func__, __LINE__);
#ifndef RELAX_DATABLOCK_SEASONING
            meta->cloningState.continuationOpcode = SeasoningOneOrMoreDbCreates;
#endif
//printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);
         } else if (meta->neighborRefinementLevel[dir][pole] == meta->refinementLevel + 1) {  // Neighbor is finer.  Our face provides quarter faces to each of four neighbors.
*((int *) 123) = 456;
            if (control->code == MinimalSends) {                                              // Halo exchange will only need to cover the exact face;  no need to expand to include edges and corners.
               rowCountInGst = (rowCountInPlaneOfBlk - 2) >> 1;
               colCountInGst = (colCountInPlaneOfBlk - 2) >> 1;
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

            meta->faceDb[0][0][pole].size = sizeof_Face_BasePart_t + num_comm * rowCountInGst * colCountInGst * sizeof(double);
            meta->faceDb[0][1][pole].size = sizeof_Face_BasePart_t + num_comm * rowCountInGst * colCountInGst * sizeof(double);
            meta->faceDb[1][0][pole].size = sizeof_Face_BasePart_t + num_comm * rowCountInGst * colCountInGst * sizeof(double);
            meta->faceDb[1][1][pole].size = sizeof_Face_BasePart_t + num_comm * rowCountInGst * colCountInGst * sizeof(double);
            meta->faceDb[0][0][pole].acMd = DB_MODE_RW;
            meta->faceDb[0][1][pole].acMd = DB_MODE_RW;
            meta->faceDb[1][0][pole].acMd = DB_MODE_RW;
            meta->faceDb[1][1][pole].acMd = DB_MODE_RW;
            gasket__ocrDbCreate(&meta->faceDb[0][0][pole].dblk,  (void **) &meta->faceDb[0][0][pole].base, meta->faceDb[0][0][pole].size, __FILE__, __func__, __LINE__);
            gasket__ocrDbCreate(&meta->faceDb[0][1][pole].dblk,  (void **) &meta->faceDb[0][1][pole].base, meta->faceDb[0][1][pole].size, __FILE__, __func__, __LINE__);
            gasket__ocrDbCreate(&meta->faceDb[1][0][pole].dblk,  (void **) &meta->faceDb[1][0][pole].base, meta->faceDb[1][0][pole].size, __FILE__, __func__, __LINE__);
            gasket__ocrDbCreate(&meta->faceDb[1][1][pole].dblk,  (void **) &meta->faceDb[1][1][pole].base, meta->faceDb[1][1][pole].size, __FILE__, __func__, __LINE__);
#ifndef RELAX_DATABLOCK_SEASONING
            meta->cloningState.continuationOpcode = SeasoningOneOrMoreDbCreates;
#endif
//printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);
         } else if (meta->neighborRefinementLevel[dir][pole] == -2) {                         // Neighbor is off edge of mesh, i.e. non-existent.  Do nothing now except setup to deal with this case later.
            if (control->code == MinimalSends) {                                              // Halo replication will only need to cover the exact face;  no need to expand to include edges and corners.
               rowCountInGst = rowCountInPlaneOfBlk - 2;
               colCountInGst = colCountInPlaneOfBlk - 2;
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
            printf ("%d line %d, error: neighbor is not within one refinement level.\n"); fflush(stdout);
            *((int *) 123) = 456;
         }
      }

      // Season the datablocks for the halo data.

#ifndef RELAX_DATABLOCK_SEASONING
      if (meta->cloningState.continuationOpcode == SeasoningOneOrMoreDbCreates) {
         SUSPEND__RESUME_IN_CONTINUATION_EDT(;)
      }
#endif

      // Populate the halo faces

      for (pole = 0, blkPlane = 1; pole <= 1; pole++, blkPlane = planeCountInBlk) {           // West then East (for dir=0); South then North (for dir=1); Down then Up (for dir=2)
         if (meta->neighborRefinementLevel[dir][pole] == meta->refinementLevel) {             // Neighbor block is at same refinement level.
            if (control->code != MinimalSends) {                                              // Halo exchange will only need to cover the exact face;  no need to expand to include edges and corners.
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
               // Consider whether the code needs to be different for this case.
            }

            double * halo = meta->faceDb[0][0][pole].base->cells;
//printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);
            for (var = 0; var < num_comm; var++) {
               for (row = 0; row < rowCountInGst; row++) {
                  for (col = 0; col < colCountInGst; col++) {
                     *halo++ = cp[start+var][(blkPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk) * colPitchInPlaneOfBlk)];
                  }
               }
            }
         } else if (meta->neighborRefinementLevel[dir][pole] == meta->refinementLevel - 1) {  // Neighbor is coarser.  Our full face provides only a quarter face to it.
*((int *) 123) = 456;
            if (control->code != MinimalSends) {                                              // Halo exchange will only need to cover the exact face;  no need to expand to include edges and corners.
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
               // Consider whether the code needs to be different for this case.
            }

            double * halo = meta->faceDb[0][0][pole].base->cells;
//printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);
            for (var = 0; var < num_comm; var++) {
               for (row = 0; row < rowCountInGst; row+=2) {
                  for (col = 0; col < colCountInGst; col+=2) {
                     *halo++ = cp[start+var][(blkPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk    ) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk    ) * colPitchInPlaneOfBlk)] +
                               cp[start+var][(blkPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk    ) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk + 1) * colPitchInPlaneOfBlk)] +
                               cp[start+var][(blkPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk + 1) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk    ) * colPitchInPlaneOfBlk)] +
                               cp[start+var][(blkPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk + 1) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk + 1) * colPitchInPlaneOfBlk)];
                  }
               }
            }
         } else if (meta->neighborRefinementLevel[dir][pole] == meta->refinementLevel + 1) {  // Neighbor is finer.  Our face provides quarter faces to each of four neighbors.
*((int *) 123) = 456;
            if (control->code != MinimalSends) {                                              // Halo exchange will only need to cover the exact face;  no need to expand to include edges and corners.
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
               // Consider whether the code needs to be different for this case.
            }
            for (qrtrHaloLeftRight = 0, colOffsetInPlaneOfBlk = 0; qrtrHaloLeftRight <= 1; qrtrHaloLeftRight++, colOffsetInPlaneOfBlk = rowCountInGst) {
               for (qrtrHaloUpDown = 0, rowOffsetInPlaneOfBlk = 0; qrtrHaloUpDown    <= 1; qrtrHaloUpDown++,    rowOffsetInPlaneOfBlk = colCountInGst) {
                  double * halo = meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].base->cells;
//printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);
                  for (var = 0; var < num_comm; var++) {
                     for (row = 0; row < rowCountInGst; row++) {
                        for (col = 0; col < colCountInGst; col++) {
                           *halo++ = cp[start+var][(blkPlane * planePitchInBlk) +
                                                   ((row + firstRowInPlaneOfBlk + rowOffsetInPlaneOfBlk) * rowPitchInPlaneOfBlk) +
                                                   ((col + firstColInPlaneOfBlk + colOffsetInPlaneOfBlk) * colPitchInPlaneOfBlk)] * 0.25;
                        }
                     }
                  }
               }
            }
         } else if (meta->neighborRefinementLevel[dir][pole] == -2) {                         // Neighbor is off edge of mesh, i.e. non-existent.  Do nothing now; deal with this case later.
            ;
         }
      }

      // Now transmit the halo faces to the neighbors.

      for (pole = 0, blkPlane = 1; pole <= 1; pole++, blkPlane = planeCountInBlk) {           // West then East (for dir=0); South then North (for dir=1); Down then Up (for dir=2)
         if (meta->neighborRefinementLevel[dir][pole] == -2) continue;                        // Neighbor is off edge of mesh, i.e. non-existent.  We do NOT send a halo in that case.
         if (meta->neighborRefinementLevel[dir][pole] <= meta->refinementLevel) {             // Neighbor block is at same refinement level, or one level coarser.  Send ONE halo per dir/pole.
            Face_t * halo = meta->faceDb[0][0][pole].base;
//printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);
            ocrGuid_t conveyFaceToNeighbor_Event = meta->conveyFaceToNeighbor_Event[dir][pole][0];
            if (conveyFaceToNeighbor_Event == NULL_GUID) {                                            // First time.  Use labeled guid.
               unsigned long long index =
                  (((01111111111111110LL & (((unsigned long long) meta->refinementLevel) << 3LL)) +   // This component gets the first labeled guid first for the refinement level of this block ...
                     ((((meta->xPos  * (control->npy << meta->refinementLevel)) +                     // ... to which we add the linearized posigion of the block in the mesh.
                         meta->yPos) * (control->npz << meta->refinementLevel)) +
                         meta->zPos)) *
                    36) +                                                                             // We scale all that by 36 (6 for when nbr coarser; six for when nbr same; 24 for when nbr finer).
                  dir * 2 + pole +                                                                    // Now select which of the 36 comm events to use: for nbr coarser, 0=W, 1=E, 2=S, 3=N,  4=D,  5=U, ...
                  (meta->neighborRefinementLevel[dir][pole] < meta->refinementLevel ? 0 : 6);         // ... but for nbr at same refinement level:                       6=W, 7=E, 8=S, 9=N, 10=D, 11=U.
//printf ("***** at line %d, index = %ld / 0x%lx    dir = %d  pole = %d  level = %d %d  pos = %d %d %d  numBlks = %d %d %d\n",
//__LINE__, index, index, dir, pole, meta->refinementLevel, meta->neighborRefinementLevel[dir][pole], meta->xPos, meta->yPos, meta->zPos, control->npx, control->npy, control->npz); fflush(stdout);
//printf ("xPos = %d, yPos = %d, zPos %d, refLvl = %d, sending to %c nbr (%s) via index = %ld / 0x%lx\n", meta->xPos, meta->yPos, meta->zPos, meta->refinementLevel, dir==0?(pole==0?'W':'E'):dir==1?(pole==0?'S':'N'):(pole==0?'D':'U'), meta->neighborRefinementLevel[dir][pole] == meta->refinementLevel ? "sameLvl" : "coarser", index, index); fflush(stdout);
               ocrGuidFromIndex(&conveyFaceToNeighbor_Event, meta->labeledGuidRangeForHaloExchange, index);
#define DEFAULT_LG_PROPS GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG
               ocrEventCreate(&conveyFaceToNeighbor_Event, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS);
            }
            halo->dbCommHeader.atBat_Event = conveyFaceToNeighbor_Event;                                              // Convey At Bat Event to neighbor so that she can destroy the event.
            ocrEventCreate(&meta->conveyFaceToNeighbor_Event[dir][pole][0], OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);  // Create the On Deck Event; record it in our meta.
            halo->dbCommHeader.onDeck_Event = meta->conveyFaceToNeighbor_Event[dir][pole][0];                         // Convey On Deck Event to parent so that she can make her clone depend upon it.
//printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);
            //ocrDbRelease(meta->faceDb[0][0][pole].dblk);
//printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);
            ocrEventSatisfy(conveyFaceToNeighbor_Event, meta->faceDb[0][0][pole].dblk);                               // Satisfy the neighbors's dependence for this face.
//printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);
            meta->faceDb[0][0][pole].base = halo = NULL;
            meta->faceDb[0][0][pole].dblk = NULL_GUID;
            meta->faceDb[0][0][pole].size = -9999;
            meta->faceDb[0][0][pole].acMd = DB_MODE_NULL;
         } else {                                                                             // Neighbor is finer.  Our face provides quarter faces to each of four neighbors.
*((int *) 123) = 456;
            int idep = 0;
            for (qrtrHaloLeftRight = 0; qrtrHaloLeftRight <= 1; qrtrHaloLeftRight++) {        // Left /Right quadrant of quarter-face planes
               for (qrtrHaloUpDown = 0; qrtrHaloUpDown    <= 1; qrtrHaloUpDown++) {           // Upper/Lower quadrant of quarter-face planes
                  Face_t * halo = meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].base;
                  ocrGuid_t conveyFaceToNeighbor_Event = meta->conveyFaceToNeighbor_Event[dir][pole][idep];
                  if (conveyFaceToNeighbor_Event == NULL_GUID) {                                                 // First time.  Use labeled guid.
                     unsigned long long index =
                        (((01111111111111110LL & (((unsigned long long) meta->refinementLevel) << 3LL)) +   // This component gets the first labeled guid first for the refinement level of this block ...
                           ((((meta->xPos  * (control->npy << meta->refinementLevel)) +                     // ... to which we add the linearized posigion of the block in the mesh.
                               meta->yPos) * (control->npz << meta->refinementLevel)) +
                               meta->zPos)) *
                          36) +                                                                             // We scale all that by 36 (6 for when nbr coarser; six for when nbr same; 24 for when nbr finer).
                        dir * 8 + pole *4 + idep + 12;                                                      // Now select: 12:15 = W, 16:19 = E, 20:23 = S, 24:27 = N, 28:31 = D, 32:35 = U, neighbor finer.
//printf ("xPos = %d, yPos = %d, zPos %d, refLvl = %d, sending to %c nbr ( finer ) via index = %ld / 0x%lx\n", meta->xPos, meta->yPos, meta->zPos, meta->refinementLevel, dir==0?(pole==0?'W':'E'):dir==1?(pole==0?'S':'N'):(pole==0?'D':'U'), index, index); fflush(stdout);
                     ocrGuidFromIndex(&conveyFaceToNeighbor_Event, meta->labeledGuidRangeForHaloExchange, index);
                     ocrEventCreate(&conveyFaceToNeighbor_Event, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS);
                  }
                  halo->dbCommHeader.atBat_Event = conveyFaceToNeighbor_Event;                                                // Convey At Bat Event to neighbor so that she can destroy the event.
                  ocrEventCreate(&meta->conveyFaceToNeighbor_Event[dir][pole][idep], OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG); // Create the On Deck Event; record it in our meta.
                  halo->dbCommHeader.onDeck_Event = meta->conveyFaceToNeighbor_Event[dir][pole][idep];                        // Convey On Deck Event to parent so that she can make her clone depend upon it.
                  ocrDbRelease(meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].dblk);
                  ocrEventSatisfy(conveyFaceToNeighbor_Event, meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].dblk);    // Satisfy the neighbors's dependence for this face.
                  meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].base = halo = NULL;
                  meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].dblk = NULL_GUID;
                  meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].size = -9999;
                  meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].acMd = DB_MODE_NULL;
                  idep++;
               }
            }
         }
      }

      // Now fill out halo planes at edges of the full mesh.

      for (pole = 0, blkPlane = 1, haloPlane = 0; pole <= 1; pole++, blkPlane = planeCountInBlk, haloPlane = planeCountInBlk+1) {  // W then E (for dir=0); S then N (for dir=1); D then U (for dir=2)
         if (meta->neighborRefinementLevel[dir][pole] == -2) {                                // Neighbor is off edge of mesh.  Deal with it now.
            if (control->code != MinimalSends) {                                              // Halo exchange will only need to cover the exact face;  no need to expand to include edges and corners.
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
               // Consider whether the code needs to be different for this case.
            }

//printf ("dir = %d, pole = %d, num_comm = %d, rowCountInGst = %d, colCountInGst = %d, start = %d\n", dir, pole, num_comm, rowCountInGst, colCountInGst, start); fflush(stdout);

            for (var = 0; var < num_comm; var++) {
               for (row = 0; row < rowCountInGst; row++) {
                  for (col = 0; col < colCountInGst; col++) {
                     cp   [start+var][(haloPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk) * colPitchInPlaneOfBlk)] =
                        cp[start+var][(blkPlane  * planePitchInBlk) + ((row + firstRowInPlaneOfBlk) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk) * colPitchInPlaneOfBlk)];
                  }
               }
            }
         }
      }

      // Now set up for our clone to receive halos from neighbors

      for (pole = 0, blkPlane = 1; pole <= 1; pole++, blkPlane = planeCountInBlk) { // West then East (for dir=0); South then North (for dir=1); Down then Up (for dir=2)
         if (meta->neighborRefinementLevel[dir][pole] == -2) continue;              // Neighbor is off edge of mesh, i.e. non-existent.  We do NOT receive a halo in that case.
         meta->cloningState.continuationOpcode = ReceivingACommunication;           // One or both poles does indeed have a neighbor.  Set the continuationOpcode, which we test and act upon after this loop.
         if (meta->neighborRefinementLevel[dir][pole] < meta->refinementLevel) {    // Neighbor block is or one level coarser.  Receive ONE halo per dir/pole.
*((int *) 123) = 456;
            meta->faceDb[0][0][pole].size = sizeof_Face_BasePart_t + num_comm * rowCountInGst * colCountInGst * sizeof(double);
            meta->faceDb[0][0][pole].acMd = DB_MODE_RO;
            meta->faceDb[0][0][pole].base = NULL;
            meta->faceDb[0][0][pole].dblk = meta->onDeckToReceiveFace_Event[dir][pole][0];
            if (meta->faceDb[0][0][pole].dblk == NULL_GUID) {                                         // First time.  Use labeled guid.
               unsigned long long index =
                  (((((01111111111111110LL & (((unsigned long long) meta->refinementLevel) << 3LL)) + // This component gets the first labeled guid first for the refinement level of this block ...
                      (((meta->xPos >> 1) * control->npz * control->npy) +                            // ... to which we add the linearized position of our parent block in the mesh, which ...
                       ((meta->yPos >> 1) * control->npz)                +                            // ...
                       ((meta->zPos >> 1)) +                                                          // ...
                       (((pole == 0) ? -1 : 1) *                                                      // ... is here adjusted to the position of the neighbor, which is plus or minus ...
                        ((dir == 2) ? 1 :                                                             // ... 1, i.e.  distanct to next z-block at neighbor's refinement level, or ...
                           ((control->npz >> (meta->refinementLevel-1)) * ((dir == 1) ? 1 :           // ... distance to next y-block at neighbor's refinement level, or ...
                            (control->npy >> (meta->refinementLevel-1)))))))) *                       // ... distance to next x-block at neibhgor's refinement level.  Multiply all of the above by ...
                     36) +                                                                            // ... 36 (6 for when nbr coarser; six for when nbr same; 24 for when nbr finer).  Then...
                    dir * 2 + pole)                                                                   // ... select which of 36 to use: 0=W, 1=E, 2=S, 3=N, 4=D, 5=U; nbr coarser.
                   ^ 1);                                                                              // BUT FLIP POLE: my W comms with nbrs S; etc.
//printf ("***** at line %d,  index = %ld / 0x%lx    dir = %d  pole = %d  level = %d %d  pos = %d %d %d  numBlks = %d %d %d\n",
//__LINE__, index, index, dir, pole, meta->refinementLevel, meta->neighborRefinementLevel[dir][pole], meta->xPos, meta->yPos, meta->zPos, control->npx, control->npy, control->npz); fflush(stdout);
//printf ("xPos = %d, yPos = %d, zPos %d, refLvl = %d, receiving from %c nbr (coarser) via index = %ld / 0x%lx\n", meta->xPos, meta->yPos, meta->zPos, meta->refinementLevel, dir==0?(pole==0?'W':'E'):dir==1?(pole==0?'S':'N'):(pole==0?'D':'U'), index, index); fflush(stdout);
               ocrGuidFromIndex(&meta->faceDb[0][0][pole].dblk, meta->labeledGuidRangeForHaloExchange, index);
               ocrEventCreate(&meta->faceDb[0][0][pole].dblk, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS);
            }
         } else if (meta->neighborRefinementLevel[dir][pole] == meta->refinementLevel) {             // Neighbor block is at same refinement level.  Receive ONE halo per dir/pole.
            meta->faceDb[0][0][pole].size = sizeof_Face_BasePart_t + num_comm * rowCountInGst * colCountInGst * sizeof(double);
            meta->faceDb[0][0][pole].acMd = DB_MODE_RO;
            meta->faceDb[0][0][pole].base = NULL;
            meta->faceDb[0][0][pole].dblk = meta->onDeckToReceiveFace_Event[dir][pole][0];
            if (meta->faceDb[0][0][pole].dblk == NULL_GUID) {                                         // First time.  Use labeled guid.
               unsigned long long index =
                  (((((01111111111111110LL & (((unsigned long long) meta->refinementLevel) << 3LL)) + // This component gets the first labeled guid first for the refinement level of this block ...
                      (((meta->xPos) * control->npz * control->npy) +                                 // ... to which we add the linearized position of our parent block in the mesh, which ...
                       ((meta->yPos) * control->npz)                +                                 // ...
                       ((meta->zPos)) +                                                               // ...
                       (((pole == 0) ? -1 : 1) *                                                      // ... is here adjusted to the position of the neighbor, which is plus or minus ...
                        ((dir == 2) ? 1 :                                                             // ... 1, i.e.  distanct to next z-block at neighbor's refinement level, or ...
                           ((control->npz >> (meta->refinementLevel)) * ((dir == 1) ? 1 :             // ... distance to next y-block at neighbor's refinement level, or ...
                            (control->npy >> (meta->refinementLevel)))))))) *                         // ... distance to next x-block at neibhgor's refinement level.  Multiply all of the above by ...
                     36) +                                                                            // ... 36 (6 for when nbr coarser; six for when nbr same; 24 for when nbr finer).  Then...
                    dir * 2 + pole + 6)                                                               // ... select which of 36 to use: 6=W, 7=E, 8=S, 9=N, 10=D, 11=U; nbr is at same refinement level.
                   ^ 1);                                                                              // BUT FLIP POLE: my W comms with nbrs S; etc.
//printf ("***** at line %d,  index = %ld / 0x%lx    dir = %d  pole = %d  level = %d %d  pos = %d %d %d  numBlks = %d %d %d\n",
//__LINE__, index, index, dir, pole, meta->refinementLevel, meta->neighborRefinementLevel[dir][pole], meta->xPos, meta->yPos, meta->zPos, control->npx, control->npy, control->npz); fflush(stdout);
//printf ("xPos = %d, yPos = %d, zPos %d, refLvl = %d, receiving from %c nbr (sameLvl) via index = %ld / 0x%lx\n", meta->xPos, meta->yPos, meta->zPos, meta->refinementLevel, dir==0?(pole==0?'W':'E'):dir==1?(pole==0?'S':'N'):(pole==0?'D':'U'), index, index); fflush(stdout);
               ocrGuidFromIndex(&meta->faceDb[0][0][pole].dblk, meta->labeledGuidRangeForHaloExchange, index);
//printf ("*********** got event 0x%lx for pole = %d\n", (unsigned long long) meta->faceDb[0][0][pole].dblk, pole); fflush(stdout);
               ocrEventCreate(&meta->faceDb[0][0][pole].dblk, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS);
//printf ("*********** got event 0x%lx for pole = %d\n", (unsigned long long) meta->faceDb[0][0][pole].dblk, pole); fflush(stdout);
            }
         } else {                                                                             // Neighbor is finer.  We receive quarter faces from each of four neighbors.
*((int *) 123) = 456;
            int idep = 0;
            for (qrtrHaloLeftRight = 0; qrtrHaloLeftRight <= 1; qrtrHaloLeftRight++) {        // Left /Right quadrant of quarter-face planes
               for (qrtrHaloUpDown = 0; qrtrHaloUpDown    <= 1; qrtrHaloUpDown++) {           // Upper/Lower quadrant of quarter-face planes
                  meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].size = sizeof_Face_BasePart_t + num_comm * rowCountInGst * colCountInGst * sizeof(double);
                  meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].acMd = DB_MODE_RO;
                  meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].base = NULL;
                  meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].dblk = meta->onDeckToReceiveFace_Event[dir][pole][idep];
                  if (meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].dblk == NULL_GUID) {                     // First time.  Use labeled guid.
                     unsigned long long index =
                        (((((01111111111111110LL & (((unsigned long long) meta->refinementLevel) << 3LL)) + // This component gets the first labeled guid first for the refinement level of this block ...
                            (((meta->xPos << 1) * control->npz * control->npy) +                            // ... to which we add the linearized position of our parent block in the mesh, which ...
                             ((meta->yPos << 1) * control->npz)                +                            // ...
                             ((meta->zPos << 1)) +                                                          // ...
                             (((pole == 0) ? -1 : 1) *                                                      // ... is here adjusted to the position of the neighbor, which is plus or minus ...
                              ((dir == 2) ? 1 :                                                             // ... 1, i.e.  distanct to next z-block at neighbor's refinement level, or ...
                                 ((control->npz >> (meta->refinementLevel+1)) * ((dir == 1) ? 1 :           // ... distance to next y-block at neighbor's refinement level, or ...
                                  (control->npy >> (meta->refinementLevel+1)))))))) *                       // ... distance to next x-block at neibhgor's refinement level.  Multiply all of the above by ...
                           36) +                                                                            // ... 36 (6 for when nbr coarser; six for when nbr same; 24 for when nbr finer).  Then...
                          dir * 8 + pole * 4 + 12)                                                          // ... select which of 36 to use: 12:15=W, 16:19=E, 20:23=S, 24:27=N, 28:31=D, 32:35=U; nbr finer.
                         ^ 4);                                                                              // BUT FLIP POLE: my W comms with nbrs S; etc.
//printf ("xPos = %d, yPos = %d, zPos %d, refLvl = %d, receiving from %c nbr ( finer ) via index = %ld / 0x%lx\n", meta->xPos, meta->yPos, meta->zPos, meta->refinementLevel, dir==0?(pole==0?'W':'E'):dir==1?(pole==0?'S':'N'):(pole==0?'D':'U'), index, index); fflush(stdout);
                     ocrGuidFromIndex(&meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].dblk, meta->labeledGuidRangeForHaloExchange, index);
                     ocrEventCreate(&meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].dblk, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS);
                  }
                  idep++;
               }
            }
         }
      }

      // Now clone, so that our continuation EDT receives the halos

      if (meta->cloningState.continuationOpcode == ReceivingACommunication) {    // If one or both poles does have a neighbor, clone so that we can get the halo(s).
//printf ("SUSPENDING TO GET INCOMING HALOS, dir = %d, zPos = %d  0x%lx 0x%lx\n", dir, meta->zPos, (unsigned long long) meta->faceDb[0][0][0].dblk, (unsigned long long) meta->faceDb[0][0][1].dblk); fflush(stdout);
         meta->cloningState.continuationOpcode = ReceivingACommunication;
         SUSPEND__RESUME_IN_CONTINUATION_EDT(;)
      }

      // Now move the halo material to its position in the block.  While doing so, also destroy the halo datablocks and the events that brought them to us, and set up the next At-Bat event.

      for (pole = 0, haloPlane = 0; pole <= 1; pole++, haloPlane = planeCountInBlk + 1) {     // West then East (for dir=0); South then North (for dir=1); Down then Up (for dir=2)
         if (meta->neighborRefinementLevel[dir][pole] == meta->refinementLevel) {             // Neighbor block is at same refinement level.
            if (control->code != MinimalSends) {                                              // Halo exchange will only need to cover the exact face;  no need to expand to include edges and corners.
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
               // Consider whether the code needs to be different for this case.
            }

            double * halo = meta->faceDb[0][0][pole].base->cells;
//printf (">>>>>>>>> halo = 0x%lx, zPos = %d\n", halo, meta->zPos); fflush(stdout);
            for (var = 0; var < num_comm; var++) {
               for (row = 0; row < rowCountInGst; row++) {
                  for (col = 0; col < colCountInGst; col++) {
//printf ("<<<<<< at line %d,   zPos = %d,  %p %p\n", __LINE__, meta->zPos, meta->faceDb[0][0][0].base, meta->faceDb[0][0][1].base); fflush(stdout);
                     cp[start+var][(haloPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk) * colPitchInPlaneOfBlk)] = *halo++;
                  }
               }
            }
            ocrGuid_t        halo_dblk =   meta->faceDb[0][0][pole].dblk;
            DbCommHeader_t * halo_comm = &(meta->faceDb[0][0][pole].base->dbCommHeader);
            ocrEventDestroy(halo_comm->atBat_Event);     // Destroy the event that brought us our halo datablock.
            meta->onDeckToReceiveFace_Event[dir][pole][0] = halo_comm->onDeck_Event;
            ocrDbDestroy(halo_dblk);
            meta->faceDb[0][0][pole].dblk = NULL_GUID;
            meta->faceDb[0][0][pole].base = NULL;
            meta->faceDb[0][0][pole].size = -9999;
            meta->faceDb[0][0][pole].acMd = DB_MODE_NULL;
         } else if (meta->neighborRefinementLevel[dir][pole] == meta->refinementLevel - 1) {  // Neighbor is coarser.
*((int *) 123) = 456;
            if (control->code != MinimalSends) {                                              // Halo exchange will only need to cover the exact face;  no need to expand to include edges and corners.
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
               // Consider whether the code needs to be different for this case.
            }

            // Neighbor provided a qrtr face, which we replicate to our full face.
            double * halo = meta->faceDb[0][0][pole].base->cells;
            for (var = 0; var < num_comm; var++) {
               for (row = 0; row < rowCountInGst; row+=2) {
                  for (col = 0; col < colCountInGst; col+=2) {
                     cp[start+var][(haloPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk    ) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk    ) * colPitchInPlaneOfBlk)] = *halo;
                     cp[start+var][(haloPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk    ) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk + 1) * colPitchInPlaneOfBlk)] = *halo;
                     cp[start+var][(haloPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk + 1) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk    ) * colPitchInPlaneOfBlk)] = *halo;
                     cp[start+var][(haloPlane * planePitchInBlk) + ((row + firstRowInPlaneOfBlk + 1) * rowPitchInPlaneOfBlk) + ((col + firstColInPlaneOfBlk + 1) * colPitchInPlaneOfBlk)] = *halo;
                     halo++;
                  }
               }
            }
            ocrGuid_t        halo_dblk =   meta->faceDb[0][0][pole].dblk;
            DbCommHeader_t * halo_comm = &(meta->faceDb[0][0][pole].base->dbCommHeader);
            ocrEventDestroy(halo_comm->atBat_Event);     // Destroy the event that brought us our halo datablock.
            meta->onDeckToReceiveFace_Event[dir][pole][0] = halo_comm->onDeck_Event;
            ocrDbDestroy(halo_dblk);
            meta->faceDb[0][0][pole].dblk = NULL_GUID;
            meta->faceDb[0][0][pole].base = NULL;
            meta->faceDb[0][0][pole].size = -9999;
            meta->faceDb[0][0][pole].acMd = DB_MODE_NULL;
         } else if (meta->neighborRefinementLevel[dir][pole] == meta->refinementLevel + 1) {  // Neighbor is finer.  We receive four quarter faces.
*((int *) 123) = 456;
            if (control->code != MinimalSends) {                                              // Halo exchange will only need to cover the exact face;  no need to expand to include edges and corners.
               printf ("%s line %d, case not yet implemented!\n", __FILE__, __LINE__); fflush(stdout);
               *((int *) 123) = 456;
               // Consider whether the code needs to be different for this case.
            }
            int idep = 0;
            for (qrtrHaloLeftRight = 0, colOffsetInPlaneOfBlk = 0; qrtrHaloLeftRight <= 1; qrtrHaloLeftRight++, colOffsetInPlaneOfBlk = rowCountInGst) {
               for (qrtrHaloUpDown = 0, rowOffsetInPlaneOfBlk = 0; qrtrHaloUpDown    <= 1; qrtrHaloUpDown++,    rowOffsetInPlaneOfBlk = colCountInGst) {
                  double * halo = meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].base->cells;
                  for (var = 0; var < num_comm; var++) {
                     for (row = 0; row < rowCountInGst; row++) {
                        for (col = 0; col < colCountInGst; col++) {
                           cp[start+var][(haloPlane * planePitchInBlk) +
                                         ((row + firstRowInPlaneOfBlk + rowOffsetInPlaneOfBlk) * rowPitchInPlaneOfBlk) +
                                         ((col + firstColInPlaneOfBlk + colOffsetInPlaneOfBlk) * colPitchInPlaneOfBlk)]  = *halo++;
                        }
                     }
                  }
                  ocrGuid_t        halo_dblk =   meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].dblk;
                  DbCommHeader_t * halo_comm = &(meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].base->dbCommHeader);
                  ocrEventDestroy(halo_comm->atBat_Event);     // Destroy the event that brought us our halo datablock.
                  meta->onDeckToReceiveFace_Event[dir][pole][idep] = halo_comm->onDeck_Event;
                  ocrDbDestroy(halo_dblk);
                  meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].dblk = NULL_GUID;
                  meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].base = NULL;
                  meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].size = -9999;
                  meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].acMd = DB_MODE_NULL;
                  idep++;
               }
            }
         }
      }
      meta->profileDb.base->timer_comm_dir[dir] += timer() - t1;
dump(meta);
   }

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE

} // comm

#if 0
===
         } else {                                                                             // Neighbor is finer.  Our face provides quarter faces to each of four neighbors.
            int idep = 0;
            for (qrtrHaloLeftRight = 0; qrtrHaloLeftRight <= 1; qrtrHaloLeftRight++) {        // Left /Right quadrant of quarter-face planes
               for (qrtrHaloUpDown = 0; qrtrHaloUpDown    <= 1; qrtrHaloUpDown++) {           // Upper/Lower quadrant of quarter-face planes
                  Face_t * halo = (Face_t *) &meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].base;
                  ocrGuid_t conveyFaceToNeighbor_Event = meta->conveyFaceToNeighbor_Event[dir][pole][idep];
                  if (conveyFaceToNeighbor_Event == NULL_GUID) {                                                              // First time.  Use labeled guid.
                     index = ((01111111111111110LL & ((((unsigned long long) meta->refinementLevel) * 8LL) - 1LL) * 36)) +    // This component gets the first index for this refinement level.
                             dir * 8 + pole *4 + idep + 12;                                                                   // 12:15 = W, 16:19 = E, 20:23 = S, 24:27 = N, 28:31 = D, 32:35 = U, neighbor finer.
                     ocrGuidFromIndex(&conveyFaceToNeighbor_Event, meta->labledGuidRangeForHaloExchange, index);
                     ocrEventCreate(&conveyFaceToNeighbor_Event, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS);
                  }
                  halo->dbCommHeader.atBat_Event = conveyFaceToNeighbor_Event;                                                // Convey At Bat Event to neighbor so that she can destroy the event.
                  ocrEventCreate(&meta->conveyFaceToNeighbor_Event[dir][pole][idep], OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG); // Create the On Deck Event; record it in our meta.
                  halo->dbCommHeader.onDeck_Event = meta->conveyFaceToNeighbor_Event[dir][pole][idep];                        // Convey On Deck Event to parent so that she can make her clone depend upon it.
                  ocrDbRelease(meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].dblk);
                  ocrEventSatisfy(conveyFaceToNeighbor_Event, meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].dblk);    // Satisfy the neighbors's dependence for this face.
                  meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].base = halo = NULL;
                  meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].dblk = NULL_GUID;
                  meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].size = -9999;
                  meta->faceDb[qrtrHaloLeftRight][qrtrHaloUpDown][pole].acMd = DB_MODE_NULL;
                  idep++;
               }
            }
         }
      }


//====
      for (i = 0; i < num_comm_partners[dir]; i++) {
         gasket__mpi_Irecv(glbl, &recv_buff[comm_recv_off[dir][comm_index[dir][i]]],
                   recv_size[dir][i], MPI_DOUBLE,
                   comm_partner[dir][i], type, MPI_COMM_WORLD, -9999, &request[i], __FILE__, __LINE__);
         counter_halo_recv[dir]++;
         size_mesg_recv[dir] += (double) recv_size[dir][i]*sizeof(double);
      }
      timer_comm_recv[dir] += timer() - t1;

/**** the send and recv list can be same if kept ordered (length can be diff)
        would need to expand face case so that:     **** done ****
        0   whole -> whole
        1   whole -> whole   w/ 27 point stencil (send corners)
        2-5 whole -> quarter w/ number indicating which quarter (matters recv)
        6-9 quarter -> whole w/ number indicating which quarter (matters send)
        + 10 for E, N, U
**** one large send buffer -- can pack and send for a neighbor and reuse */
      for (i = 0; i < num_comm_partners[dir]; i++) {
         t2 = timer();
         for (lcl->n = 0; lcl->n < comm_num[dir][i]; lcl->n++) {
            offset = comm_send_off[dir][comm_index[dir][i]+lcl->n];
            if (!nonblocking)
               offset -= comm_send_off[dir][comm_index[dir][i]];
            pack_face(glbl, &send_buff[offset], comm_block[dir][comm_index[dir][i]+lcl->n],
                      comm_face_case[dir][comm_index[dir][i]+lcl->n], dir,
                      start, num_comm);
         }
         counter_face_send[dir] += comm_num[dir][i];
         t3 = timer();
         if (nonblocking) {
            CALL_SUSPENDABLE_CALLEE(1)
            gasket__mpi_Isend(glbl, &send_buff[comm_send_off[dir][comm_index[dir][i]]],
                      send_size[dir][i], MPI_DOUBLE, comm_partner[dir][i],
                      type, MPI_COMM_WORLD, -9999, &s_req[i], __FILE__, __LINE__);
            DEBRIEF_SUSPENDABLE_FUNCTION(;)
         } else {
            CALL_SUSPENDABLE_CALLEE(0)
            gasket__mpi_Send(glbl, send_buff, send_size[dir][i], MPI_DOUBLE,
                     comm_partner[dir][i], type, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
            DEBRIEF_SUSPENDABLE_FUNCTION(;)
         }
         counter_halo_send[dir]++;
         size_mesg_send[dir] += (double) send_size[dir][i]*sizeof(double);
         t4 = timer();
         timer_comm_pack[dir] += t3 - t2;
         timer_comm_send[dir] += t4 - t3;
      }

      // While values are being sent over the mesh, go through and direct
      // blocks to exchange ghost values with other blocks that are on
      // processor.  Also apply boundary conditions for boundary of domain.
      for (in = 0; in < sorted_index[num_refine+1]; in++) {
         lcl->n = sorted_list[in].n;
         bp = &blocks[lcl->n];
         cp = &cells[lcl->n];
         if (bp->number >= 0)
            for (l = dir*2; l < (dir*2 + 2); l++) {
               if (bp->nei_level[l] == bp->level) {
                  t2 = timer();
                  if ((m = bp->nei[l][0][0]) > lcl->n) {
                     on_proc_comm(glbl, lcl->n, m, l, start, num_comm);
                     counter_same[dir] += 2;
                  }
                  timer_comm_same[dir] += timer() - t2;
               } else if (bp->nei_level[l] == (bp->level+1)) {
                  t2 = timer();
                  for (i = 0; i < 2; i++)
                     for (j = 0; j < 2; j++)
                        if ((m = bp->nei[l][i][j]) > lcl->n) {
                           on_proc_comm_diff(glbl, lcl->n, m, l, i, j, start, num_comm);
                           counter_diff[dir] += 2;
                        }
                  timer_comm_diff[dir] += timer() - t2;
               } else if (bp->nei_level[l] == (bp->level-1)) {
                  t2 = timer();
                  if ((m = bp->nei[l][0][0]) > lcl->n) {
                     k = dir*2 + 1 - l%2;
                     for (i = 0; i < 2; i++)
                        for (j = 0; j < 2; j++)
                           if (blocks[m].nei[k][i][j] == lcl->n) {
                              on_proc_comm_diff(glbl, m, lcl->n, k, i, j, start, num_comm);
                              counter_diff[dir] += 2;
                           }
                  }
                  timer_comm_diff[dir] += timer() - t2;
               } else if (bp->nei_level[l] == -2) {
                  t2 = timer();
                  apply_bc(glbl, l, cp, start, num_comm);
                  counter_bc[dir]++;
                  timer_comm_bc[dir] += timer() - t2;
               } else {
                  printf("ERROR: misconnected block\n");
                  exit(-1);
               }
            }
      }

      for (i = 0; i < num_comm_partners[dir]; i++) {
         t2 = timer();
#ifdef BUILD_REFERENCE_VERSION   // Original reference version did this:
         MPI_Waitany(num_comm_partners[dir], request, &which, &status);
#else   // OCR doesn't (presently) support the concept of an EDT triggering when ANY of a list of dependences is satisfied.  Rework
        // to wait specifically for each IRecv.  Also provide stuff OCR version needs to be able to receive the result buffer.
         CALL_SUSPENDABLE_CALLEE(1)
         gasket__mpi_Wait__for_Irecv(glbl, &recv_buff[comm_recv_off[dir][comm_index[dir][i]]], recv_size[dir][i], MPI_DOUBLE, comm_partner[dir][i], MPI_COMM_WORLD, -9999, &request[i], &status, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
         which = i;
#endif
         t3 = timer();
         for (lcl->n = 0; lcl->n < comm_num[dir][which]; lcl->n++)
          unpack_face(glbl, &recv_buff[comm_recv_off[dir][comm_index[dir][which]+lcl->n]],
                      comm_block[dir][comm_index[dir][which]+lcl->n],
                      comm_face_case[dir][comm_index[dir][which]+lcl->n],
                      dir, start, num_comm);
         counter_face_recv[dir] += comm_num[dir][which];
         t4 = timer();
         timer_comm_wait[dir] += t3 - t2;
         timer_comm_unpack[dir] += t4 - t3;
      }
      timer_comm_dir[dir] += timer() - t1;

      if (nonblocking) {
         t2 = timer();
         for (i = 0; i < num_comm_partners[dir]; i++) {
#ifdef BUILD_REFERENCE_VERSION   // Original reference version did this:
            MPI_Waitany(num_comm_partners[dir], s_req, &which, &status);
#else   // OCR doesn't (presently) support the concept of an EDT triggering when ANY of a list of dependences is satisfied.  Rework
        // to wait specifically for each ISend.
            gasket__mpi_Wait__for_Isend(glbl, &s_req[i], &status, __FILE__, __LINE__);
#endif
         }
         t3 = timer();
         timer_comm_wait[dir] += t3 - t2;
      }
   }
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  i
#undef  j
#undef  k
#undef  l
#undef  m
//#undef  n
#undef  dir
#undef  o
#undef  in
#undef  which
#undef  offset
#undef  type
#undef  t1
#undef  t2
#undef  t3
#undef  t4
#undef  status
#undef  bp
#undef  cp
}

// Pack face to send - note different cases for different directions.
void pack_face(Globals_t * const glbl, double *send_buf, int block_num, int face_case, int dir,
               int start, int num_comm)
{
   int i, j, k, n, m;
   int is, ie, js, je, ks, ke;
   OBTAIN_ACCESS_TO_cells
   Cell_t  *cp;
   TRACE

   cp = &cells[block_num];

   if (!code) {

      if (dir == 0) {        /* X - East, West */

         /* X directions (East and West) sent first, so just send
            the real values and no ghosts
         */
         if (face_case >= 10) { /* +X - East */
            i = x_block_size;
            face_case = face_case - 10;
         } else                 /* -X - West */
            i = 1;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 1; j <= y_block_size; j++)
                  for (k = 1; k <= z_block_size; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face - case does not matter */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 1; j < y_block_size; j += 2)
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j  ][k  ] +
                                   (*cp)[m][i][j  ][k+1] +
                                   (*cp)[m][i][j+1][k  ] +
                                   (*cp)[m][i][j+1][k+1];
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               js = 1;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size;
            }
            if ((face_case/2)%2 == 1) {
               ks = 1;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = js; j <= je; j++)
                  for (k = ks; k <= ke; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k]/4.0;
         }

      } else if (dir == 1) { /* Y - North, South */

         /* Y directions (North and South) sent second, so send the real values
         */
         if (face_case >= 10) { /* +Y - North */
            j = y_block_size;
            face_case = face_case - 10;
         } else                 /* -Y - South */
            j = 1;
         if (face_case == 0) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i <= x_block_size; i++)
                  for (k = 1; k <= z_block_size; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case == 1) {
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (k = 1; k <= z_block_size; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face - case does not matter */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i < x_block_size; i += 2)
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i  ][j][k  ] +
                                   (*cp)[m][i  ][j][k+1] +
                                   (*cp)[m][i+1][j][k  ] +
                                   (*cp)[m][i+1][j][k+1];
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               is = 1;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size;
            }
            if ((face_case/2)%2 == 1) {
               ks = 1;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (k = ks; k <= ke; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k]/4.0;
         }

      } else {               /* Z - Up, Down */

         /* Z directions (Up and Down) sent last
         */
         if (face_case >= 10) { /* +Z - Up */
            k = z_block_size;
            face_case = face_case - 10;
         } else                 /* -Z - Down */
            k = 1;
         if (face_case == 0) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i <= x_block_size; i++)
                  for (j = 1; j <= y_block_size; j++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case == 1) {
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (j = 0; j <= y_block_size+1; j++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face - case does not matter */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i < x_block_size; i += 2)
                  for (j = 1; j < y_block_size; j += 2, n++)
                     send_buf[n] = (*cp)[m][i  ][j  ][k] +
                                   (*cp)[m][i  ][j+1][k] +
                                   (*cp)[m][i+1][j  ][k] +
                                   (*cp)[m][i+1][j+1][k];
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               is = 1;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size;
            }
            if ((face_case/2)%2 == 1) {
               js = 1;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (j = js; j <= je; j++, n++)
                     send_buf[n] = (*cp)[m][i][j][k]/4.0;
         }
      }

   } else if (code == 1) { /* send all ghosts */

      if (dir == 0) {        /* X - East, West */

         if (face_case >= 10) { /* +X - East */
            i = x_block_size;
            face_case = face_case - 10;
         } else                 /* -X - West */
            i = 1;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 0; j <= y_block_size+1; j++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face */
            for (n = 0, m = start; m < start+num_comm; m++) {
               if (face_case%2 == 0) {
                  j = 0;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j][k  ] +
                                   (*cp)[m][i][j][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
               for (j = 1; j < y_block_size; j += 2) {
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i][j  ][k] +
                                     (*cp)[m][i][j+1][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j  ][k  ] +
                                   (*cp)[m][i][j  ][k+1] +
                                   (*cp)[m][i][j+1][k  ] +
                                   (*cp)[m][i][j+1][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j  ][k] +
                                     (*cp)[m][i][j+1][k];
                  }
               }
               if (face_case%2 == 1) {
                  j = y_block_size + 1;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j][k  ] +
                                   (*cp)[m][i][j][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
            }
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               js = 0;
               je = y_block_half + 1;
            } else {
               js = y_block_half;
               je = y_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               ks = 0;
               ke = z_block_half + 1;
            } else {
               ks = z_block_half;
               ke = z_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = js; j <= je; j++)
                  for (k = ks; k <= ke; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k]/4.0;
         }

      } else if (dir == 1) { /* Y - North, South */

         if (face_case >= 10) { /* +Y - North */
            j = y_block_size;
            face_case = face_case - 10;
         } else                 /* -Y - South */
            j = 1;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face */
            for (n = 0, m = start; m < start+num_comm; m++) {
               if (face_case%2 == 0) {
                  i = 0;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j][k  ] +
                                   (*cp)[m][i][j][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
               for (i = 1; i < x_block_size; i += 2) {
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i  ][j][k] +
                                     (*cp)[m][i+1][j][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i  ][j][k  ] +
                                   (*cp)[m][i  ][j][k+1] +
                                   (*cp)[m][i+1][j][k  ] +
                                   (*cp)[m][i+1][j][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i  ][j][k] +
                                     (*cp)[m][i+1][j][k];
                  }
               }
               if (face_case%2 == 1) {
                  i = x_block_size + 1;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j][k  ] +
                                   (*cp)[m][i][j][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
            }
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               is = 0;
               ie = x_block_half + 1;
            } else {
               is = x_block_half;
               ie = x_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               ks = 0;
               ke = z_block_half + 1;
            } else {
               ks = z_block_half;
               ke = z_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (k = ks; k <= ke; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k]/4.0;
         }

      } else {               /* Z - Up, Down */

         /* Z directions (Up and Down) sent last
         */
         if (face_case >= 10) { /* +Z - Up */
            k = z_block_size;
            face_case = face_case - 10;
         } else                 /* -Z - Down */
            k = 1;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (j = 0; j <= y_block_size+1; j++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face - case does not matter */
            for (n = 0, m = start; m < start+num_comm; m++) {
               if (face_case%2 == 0) {
                  i = 0;
                  if ((face_case/2)%2 == 1) {
                     j = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (j = 1; j < y_block_size; j += 2, n++)
                     send_buf[n] = (*cp)[m][i][j  ][k] +
                                   (*cp)[m][i][j+1][k];
                  if ((face_case/2)%2 == 0) {
                     j = y_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
               for (i = 1; i < x_block_size; i += 2) {
                  if ((face_case/2)%2 == 1) {
                     j = 0;
                     send_buf[n++] = (*cp)[m][i  ][j][k] +
                                     (*cp)[m][i+1][j][k];
                  }
                  for (j = 1; j < y_block_size; j += 2, n++)
                     send_buf[n] = (*cp)[m][i  ][j  ][k] +
                                   (*cp)[m][i  ][j+1][k] +
                                   (*cp)[m][i+1][j  ][k] +
                                   (*cp)[m][i+1][j+1][k];
                  if ((face_case/2)%2 == 0) {
                     j = y_block_size + 1;
                     send_buf[n++] = (*cp)[m][i  ][j][k] +
                                     (*cp)[m][i+1][j][k];
                  }
               }
               if (face_case%2 == 1) {
                  i = x_block_size + 1;
                  if ((face_case/2)%2 == 1) {
                     j = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (j = 1; j < y_block_size; j += 2, n++)
                     send_buf[n] = (*cp)[m][i][j  ][k] +
                                   (*cp)[m][i][j+1][k];
                  if ((face_case/2)%2 == 0) {
                     j = y_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
            }
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               is = 0;
               ie = x_block_half + 1;
            } else {
               is = x_block_half;
               ie = x_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               js = 0;
               je = y_block_half + 1;
            } else {
               js = y_block_half;
               je = y_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (j = js; j <= je; j++, n++)
                     send_buf[n] = (*cp)[m][i][j][k]/4.0;
         }
      }

   } else { /* code == 2 send all ghosts and do all processing on send side */

      if (dir == 0) {        /* X - East, West */

         if (face_case >= 10) { /* +X - East */
            i = x_block_size;
            face_case = face_case - 10;
         } else                 /* -X - West */
            i = 1;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 0; j <= y_block_size+1; j++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face */
            for (n = 0, m = start; m < start+num_comm; m++) {
               if (face_case%2 == 0) {
                  j = 0;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j][k  ] +
                                   (*cp)[m][i][j][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
               for (j = 1; j < y_block_size; j += 2) {
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i][j  ][k] +
                                     (*cp)[m][i][j+1][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j  ][k  ] +
                                   (*cp)[m][i][j  ][k+1] +
                                   (*cp)[m][i][j+1][k  ] +
                                   (*cp)[m][i][j+1][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j  ][k] +
                                     (*cp)[m][i][j+1][k];
                  }
               }
               if (face_case%2 == 1) {
                  j = y_block_size + 1;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j][k  ] +
                                   (*cp)[m][i][j][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
            }
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               js = 1;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size;
            }
            if ((face_case/2)%2 == 1) {
               ks = 1;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++) {
               j = js - 1;
               k = ks - 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               for (k = ks; k <= ke; k++, n+=2)
                  send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
               k = ke + 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               for (j = js; j <= je; j++) {
                  k = ks - 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
                  for (k = ks; k <= ke; k++, n+=2)
                     send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
                  k = ke + 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
                  k = ks - 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
                  for (k = ks; k <= ke; k++, n+=2)
                     send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
                  k = ke + 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               }
               j = je + 1;
               k = ks - 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               for (k = ks; k <= ke; k++, n+=2)
                  send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
               k = ke + 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
            }
         }

      } else if (dir == 1) { /* Y - North, South */

         if (face_case >= 10) { /* +Y - North */
            j = y_block_size;
            face_case = face_case - 10;
         } else                 /* -Y - South */
            j = 1;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face */
            for (n = 0, m = start; m < start+num_comm; m++) {
               if (face_case%2 == 0) {
                  i = 0;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j][k  ] +
                                   (*cp)[m][i][j][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
               for (i = 1; i < x_block_size; i += 2) {
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i  ][j][k] +
                                     (*cp)[m][i+1][j][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i  ][j][k  ] +
                                   (*cp)[m][i  ][j][k+1] +
                                   (*cp)[m][i+1][j][k  ] +
                                   (*cp)[m][i+1][j][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i  ][j][k] +
                                     (*cp)[m][i+1][j][k];
                  }
               }
               if (face_case%2 == 1) {
                  i = x_block_size + 1;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = (*cp)[m][i][j][k  ] +
                                   (*cp)[m][i][j][k+1];
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
            }
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               is = 1;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size;
            }
            if ((face_case/2)%2 == 1) {
               ks = 1;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++) {
               i = is - 1;
               k = ks - 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               for (k = ks; k <= ke; k++, n+=2)
                  send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
               k = ke + 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               for (i = is; i <= ie; i++) {
                  k = ks - 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
                  for (k = ks; k <= ke; k++, n+=2)
                     send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
                  k = ke + 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
                  k = ks - 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
                  for (k = ks; k <= ke; k++, n+=2)
                     send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
                  k = ke + 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               }
               i = ie + 1;
               k = ks - 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               for (k = ks; k <= ke; k++, n+=2)
                  send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
               k = ke + 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
            }
         }

      } else {               /* Z - Up, Down */

         /* Z directions (Up and Down) sent last
         */
         if (face_case >= 10) { /* +Z - Up */
            k = z_block_size;
            face_case = face_case - 10;
         } else                 /* -Z - Down */
            k = 1;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (j = 0; j <= y_block_size+1; j++, n++)
                     send_buf[n] = (*cp)[m][i][j][k];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face - case does not matter */
            for (n = 0, m = start; m < start+num_comm; m++) {
               if (face_case%2 == 0) {
                  i = 0;
                  if ((face_case/2)%2 == 1) {
                     j = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (j = 1; j < y_block_size; j += 2, n++)
                     send_buf[n] = (*cp)[m][i][j  ][k] +
                                   (*cp)[m][i][j+1][k];
                  if ((face_case/2)%2 == 0) {
                     j = y_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
               for (i = 1; i < x_block_size; i += 2) {
                  if ((face_case/2)%2 == 1) {
                     j = 0;
                     send_buf[n++] = (*cp)[m][i  ][j][k] +
                                     (*cp)[m][i+1][j][k];
                  }
                  for (j = 1; j < y_block_size; j += 2, n++)
                     send_buf[n] = (*cp)[m][i  ][j  ][k] +
                                   (*cp)[m][i  ][j+1][k] +
                                   (*cp)[m][i+1][j  ][k] +
                                   (*cp)[m][i+1][j+1][k];
                  if ((face_case/2)%2 == 0) {
                     j = y_block_size + 1;
                     send_buf[n++] = (*cp)[m][i  ][j][k] +
                                     (*cp)[m][i+1][j][k];
                  }
               }
               if (face_case%2 == 1) {
                  i = x_block_size + 1;
                  if ((face_case/2)%2 == 1) {
                     j = 0;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
                  for (j = 1; j < y_block_size; j += 2, n++)
                     send_buf[n] = (*cp)[m][i][j  ][k] +
                                   (*cp)[m][i][j+1][k];
                  if ((face_case/2)%2 == 0) {
                     j = y_block_size + 1;
                     send_buf[n++] = (*cp)[m][i][j][k];
                  }
               }
            }
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               is = 1;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size;
            }
            if ((face_case/2)%2 == 1) {
               js = 1;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++) {
               i = is - 1;
               j = js - 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               for (j = js; j <= je; j++, n+=2)
                  send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
               j = je + 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               for (i = is; i <= ie; i++) {
                  j = js - 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
                  for (j = js; j <= je; j++, n+=2)
                     send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
                  j = je + 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
                  j = js - 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
                  for (j = js; j <= je; j++, n+=2)
                     send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
                  j = je + 1;
                  send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               }
               i = ie + 1;
               j = js - 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
               for (j = js; j <= je; j++, n+=2)
                  send_buf[n] = send_buf[n+1] = (*cp)[m][i][j][k]/4.0;
               j = je + 1;
               send_buf[n++] = (*cp)[m][i][j][k]/4.0;
            }
         }
      }
   }
}

// Unpack ghost values that have been recieved.
// The sense of the face case is reversed since we are receiving what was sent
void unpack_face(Globals_t * const glbl, double *recv_buf, int block_num, int face_case, int dir,
                 int start, int num_comm)
{
   int i, j, k, n, m;
   int is, ie, js, je, ks, ke;
   OBTAIN_ACCESS_TO_cells
   Cell_t  *cp;
   TRACE

   cp = &cells[block_num];

   if (!code) {

      if (dir == 0) {        /* X - East, West */

         /* X directions (East and West)
            just recv the real values and no ghosts
            face_case based on send - so reverse
         */
         if (face_case >= 10) { /* +X - from East */
            i = x_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -X - from West */
            i = 0;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 1; j <= y_block_size; j++)
                  for (k = 1; k <= z_block_size; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face - one case */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 1; j < y_block_size; j += 2)
                  for (k = 1; k < z_block_size; k += 2, n++)
                     (*cp)[m][i][j  ][k  ] =
                     (*cp)[m][i][j  ][k+1] =
                     (*cp)[m][i][j+1][k  ] =
                     (*cp)[m][i][j+1][k+1] = recv_buf[n];
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to recv */
            if (face_case%2 == 0) {
               js = 1;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size;
            }
            if ((face_case/2)%2 == 1) {
               ks = 1;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = js; j <= je; j++)
                  for (k = ks; k <= ke; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         }

      } else if (dir == 1) { /* Y - North, South */

         /* Y directions (North and South) sent second, so recv the real values
         */
         if (face_case >= 10) { /* +Y - from North */
            j = y_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -Y - from South */
            j = 0;
         if (face_case == 0) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i <= x_block_size; i++)
                  for (k = 1; k <= z_block_size; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else if (face_case == 1) {
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (k = 1; k <= z_block_size; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else if (face_case >= 2 && face_case <= 5) {
            /* one case - recv into 4 cells per cell sent */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i < x_block_size; i += 2)
                  for (k = 1; k < z_block_size; k += 2, n++)
                     (*cp)[m][i  ][j][k  ] =
                     (*cp)[m][i  ][j][k+1] =
                     (*cp)[m][i+1][j][k  ] =
                     (*cp)[m][i+1][j][k+1] = recv_buf[n];
         } else {                     /* quarter face -> whole face */
            /* whole face -> quarter face - determine case */
            if (face_case%2 == 0) {
               is = 1;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size;
            }
            if ((face_case/2)%2 == 1) {
               ks = 1;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (k = ks; k <= ke; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         }

      } else {               /* Z - Up, Down */

         /* Z directions (Up and Down) sent last
         */
         if (face_case >= 10) { /* +Z - from Up */
            k = z_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -Z - from Down */
            k = 0;
         if (face_case == 0) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i <= x_block_size; i++)
                  for (j = 1; j <= y_block_size; j++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else if (face_case == 1) {
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (j = 0; j <= y_block_size+1; j++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else if (face_case >= 2 && face_case <= 5) {
            /* one case - receive into 4 cells */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i < x_block_size; i += 2)
                  for (j = 1; j < y_block_size; j += 2, n++)
                     (*cp)[m][i  ][j  ][k] =
                     (*cp)[m][i  ][j+1][k] =
                     (*cp)[m][i+1][j  ][k] =
                     (*cp)[m][i+1][j+1][k] = recv_buf[n];
         } else {                     /* quarter face -> whole face */
            /* whole face -> quarter face - determine case */
            if (face_case%2 == 0) {
               is = 1;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size;
            }
            if ((face_case/2)%2 == 1) {
               js = 1;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (j = js; j <= je; j++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         }
      }

   } else if (code == 1) {  /* send ghosts */

      if (dir == 0) {        /* X - East, West */

         if (face_case >= 10) { /* +X - from East */
            i = x_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -X - from West */
            i = 0;
         if (face_case < 2) {        /* whole face -> whole */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 0; j <= y_block_size+1; j++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face */
            for (n = 0, m = start; m < start+num_comm; m++) {
               j = 0;
               k = 0;
               (*cp)[m][i][j][k] = recv_buf[n++];
               for (k = 1; k < z_block_size; k += 2, n++)
                  (*cp)[m][i][j][k  ] =
                  (*cp)[m][i][j][k+1] = recv_buf[n];
               k = z_block_size + 1;
               (*cp)[m][i][j][k] = recv_buf[n++];
               for (j = 1; j < y_block_size; j += 2) {
                  k = 0;
                  (*cp)[m][i][j  ][k] =
                  (*cp)[m][i][j+1][k] = recv_buf[n++];
                  for (k = 1; k < z_block_size; k += 2, n++)
                     (*cp)[m][i][j  ][k  ] =
                     (*cp)[m][i][j  ][k+1] =
                     (*cp)[m][i][j+1][k  ] =
                     (*cp)[m][i][j+1][k+1] = recv_buf[n];
                  k = z_block_size + 1;
                  (*cp)[m][i][j  ][k] =
                  (*cp)[m][i][j+1][k] = recv_buf[n++];
               }
               j = y_block_size + 1;
               k = 0;
               (*cp)[m][i][j][k] = recv_buf[n++];
               for (k = 1; k < z_block_size; k += 2, n++)
                  (*cp)[m][i][j][k  ] =
                  (*cp)[m][i][j][k+1] = recv_buf[n];
               k = z_block_size + 1;
               (*cp)[m][i][j][k] = recv_buf[n++];
            }
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to recv */
            if (face_case%2 == 0) {
               js = 0;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               ks = 0;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = js; j <= je; j++)
                  for (k = ks; k <= ke; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         }

      } else if (dir == 1) { /* Y - North, South */

         if (face_case >= 10) { /* +Y - from North */
            j = y_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -Y - from South */
            j = 0;
         if (face_case < 2) {        /* whole face -> whole */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face */
            for (n = 0, m = start; m < start+num_comm; m++) {
               i = 0;
               k = 0;
               (*cp)[m][i][j][k] = recv_buf[n++];
               for (k = 1; k < z_block_size; k += 2, n++)
                  (*cp)[m][i][j][k  ] =
                  (*cp)[m][i][j][k+1] = recv_buf[n];
               k = z_block_size + 1;
               (*cp)[m][i][j][k] = recv_buf[n++];
               for (i = 1; i < x_block_size; i += 2) {
                  k = 0;
                  (*cp)[m][i  ][j][k] =
                  (*cp)[m][i+1][j][k] = recv_buf[n++];
                  for (k = 1; k < z_block_size; k += 2, n++)
                     (*cp)[m][i  ][j][k  ] =
                     (*cp)[m][i  ][j][k+1] =
                     (*cp)[m][i+1][j][k  ] =
                     (*cp)[m][i+1][j][k+1] = recv_buf[n];
                  k = z_block_size + 1;
                  (*cp)[m][i  ][j][k] =
                  (*cp)[m][i+1][j][k] = recv_buf[n++];
               }
               i = x_block_size + 1;
               k = 0;
               (*cp)[m][i][j][k] = recv_buf[n++];
               for (k = 1; k < z_block_size; k += 2, n++)
                  (*cp)[m][i][j][k  ] =
                  (*cp)[m][i][j][k+1] = recv_buf[n];
               k = z_block_size + 1;
               (*cp)[m][i][j][k] = recv_buf[n++];
            }
         } else {                     /* quarter face -> whole face */
            /* whole face -> quarter face - determine case */
            if (face_case%2 == 0) {
               is = 0;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               ks = 0;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (k = ks; k <= ke; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         }

      } else {               /* Z - Up, Down */

         if (face_case >= 10) { /* +Z - from Up */
            k = z_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -Z - from Down */
            k = 0;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (j = 0; j <= y_block_size+1; j++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face */
            for (n = 0, m = start; m < start+num_comm; m++) {
               i = 0;
               j = 0;
               (*cp)[m][i][j][k] = recv_buf[n++];
               for (j = 1; j < y_block_size; j += 2, n++)
                  (*cp)[m][i][j  ][k] =
                  (*cp)[m][i][j+1][k] = recv_buf[n];
               j = y_block_size + 1;
               (*cp)[m][i][j][k] = recv_buf[n++];
               for (i = 1; i < x_block_size; i += 2) {
                  j = 0;
                  (*cp)[m][i  ][j][k] =
                  (*cp)[m][i+1][j][k] = recv_buf[n++];
                  for (j = 1; j < y_block_size; j += 2, n++)
                     (*cp)[m][i  ][j  ][k] =
                     (*cp)[m][i  ][j+1][k] =
                     (*cp)[m][i+1][j  ][k] =
                     (*cp)[m][i+1][j+1][k] = recv_buf[n];
                  j = y_block_size + 1;
                  (*cp)[m][i  ][j][k] =
                  (*cp)[m][i+1][j][k] = recv_buf[n++];
               }
               i = x_block_size + 1;
               j = 0;
               (*cp)[m][i][j][k] = recv_buf[n++];
               for (j = 1; j < y_block_size; j += 2, n++)
                  (*cp)[m][i][j  ][k] =
                  (*cp)[m][i][j+1][k] = recv_buf[n];
               j = y_block_size + 1;
               (*cp)[m][i][j][k] = recv_buf[n++];
            }
         } else {                     /* quarter face -> whole face */
            /* whole face -> quarter face - determine case */
            if (face_case%2 == 0) {
               is = 0;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               js = 0;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (j = js; j <= je; j++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         }
      }

   } else {  /* code == 2 send ghosts and process on send */

      if (dir == 0) {        /* X - East, West */

         if (face_case >= 10) { /* +X - from East */
            i = x_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -X - from West */
            i = 0;
         if (face_case <= 5) {        /* whole face -> whole or quarter face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 0; j <= y_block_size+1; j++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to recv */
            if (face_case%2 == 0) {
               js = 0;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               ks = 0;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = js; j <= je; j++)
                  for (k = ks; k <= ke; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         }

      } else if (dir == 1) { /* Y - North, South */

         if (face_case >= 10) { /* +Y - from North */
            j = y_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -Y - from South */
            j = 0;
         if (face_case <= 5) {        /* whole face -> whole or quarter face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else {                     /* quarter face -> whole face */
            /* whole face -> quarter face - determine case */
            if (face_case%2 == 0) {
               is = 0;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               ks = 0;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (k = ks; k <= ke; k++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         }

      } else {               /* Z - Up, Down */

         if (face_case >= 10) { /* +Z - from Up */
            k = z_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -Z - from Down */
            k = 0;
         if (face_case <= 5) {        /* whole face -> whole or quarter face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (j = 0; j <= y_block_size+1; j++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         } else {                     /* quarter face -> whole face */
            /* whole face -> quarter face - determine case */
            if (face_case%2 == 0) {
               is = 0;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               js = 0;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (j = js; j <= je; j++, n++)
                     (*cp)[m][i][j][k] = recv_buf[n];
         }
      }
   }
}

// Routine that does on processor communication between two blocks that
// are at the same level of refinement.
void on_proc_comm(Globals_t * const glbl, int n, int n1, int l, int start, int num_comm)
{
   int i, j, k, m;
   int is, ie, js, je;
   OBTAIN_ACCESS_TO_cells
   Cell_t  *cp, *cp1;
   TRACE

   /* Determine direction and then exchange data across the face
   */
   if (!code) {
      if ((l/2) == 0) {         /* West, East */
         if ((l%2) == 0) {      /* West */
            cp = &cells[n];
            cp1 = &cells[n1];
         } else {               /* East */
            cp1 = &cells[n];
            cp = &cells[n1];
         }
         for (m = start; m < start+num_comm; m++)
            for (j = 1; j <= y_block_size; j++)
               for (k = 1; k <= z_block_size; k++) {
                  (*cp1)[m][x_block_size+1][j][k] = (*cp)[m][1][j][k];
                  (*cp)[m][0][j][k] = (*cp1)[m][x_block_size][j][k];
               }
      } else if ((l/2) == 1) {  /* South, North */
         if ((l%2) == 0) {      /* South */
            cp = &cells[n];
            cp1 = &cells[n1];
         } else {               /* North */
            cp1 = &cells[n];
            cp = &cells[n1];
         }
         if (stencil == 7) {
           is = 1;
           ie = x_block_size;
         } else {
           is = 0;
           ie = x_block_size + 1;
         }
         for (m = start; m < start+num_comm; m++)
            for (i = is; i <= ie; i++)
               for (k = 1; k <= z_block_size; k++) {
                  (*cp1)[m][i][y_block_size+1][k] = (*cp)[m][i][1][k];
                  (*cp)[m][i][0][k] = (*cp1)[m][i][y_block_size][k];
               }
      } else if ((l/2) == 2) {  /* Down, Up */
         if ((l%2) == 0) {      /* Down */
            cp = &cells[n];
            cp1 = &cells[n1];
         } else {               /* Up */
            cp1 = &cells[n];
            cp = &cells[n1];
         }
         if (stencil == 7) {
           is = 1;
           ie = x_block_size;
           js = 1;
           je = y_block_size;
         } else {
           is = 0;
           ie = x_block_size + 1;
           js = 0;
           je = y_block_size + 1;
         }
         for (m = start; m < start+num_comm; m++)
            for (i = is; i <= ie; i++)
               for (j = js; j <= je; j++) {
                  (*cp1)[m][i][j][z_block_size+1] = (*cp)[m][i][j][1];
                  (*cp)[m][i][j][0] = (*cp1)[m][i][j][z_block_size];
               }
      }
   } else {  /* set all ghosts */
      if ((l/2) == 0) {         /* West, East */
         if ((l%2) == 0) {      /* West */
            cp = &cells[n];
            cp1 = &cells[n1];
         } else {               /* East */
            cp1 = &cells[n];
            cp = &cells[n1];
         }
         for (m = start; m < start+num_comm; m++)
            for (j = 0; j <= y_block_size+1; j++)
               for (k = 0; k <= z_block_size+1; k++) {
                  (*cp1)[m][x_block_size+1][j][k] = (*cp)[m][1][j][k];
                  (*cp)[m][0][j][k] = (*cp1)[m][x_block_size][j][k];
               }
      } else if ((l/2) == 1) {  /* South, North */
         if ((l%2) == 0) {      /* South */
            cp = &cells[n];
            cp1 = &cells[n1];
         } else {               /* North */
            cp1 = &cells[n];
            cp = &cells[n1];
         }
         for (m = start; m < start+num_comm; m++)
            for (i = 0; i <= x_block_size+1; i++)
               for (k = 0; k <= z_block_size+1; k++) {
                  (*cp1)[m][i][y_block_size+1][k] = (*cp)[m][i][1][k];
                  (*cp)[m][i][0][k] = (*cp1)[m][i][y_block_size][k];
               }
      } else if ((l/2) == 2) {  /* Down, Up */
         if ((l%2) == 0) {      /* Down */
            cp = &cells[n];
            cp1 = &cells[n1];
         } else {               /* Up */
            cp1 = &cells[n];
            cp = &cells[n1];
         }
         for (m = start; m < start+num_comm; m++)
            for (i = 0; i <= x_block_size+1; i++)
               for (j = 0; j <= y_block_size+1; j++) {
                  (*cp1)[m][i][j][z_block_size+1] = (*cp)[m][i][j][1];
                  (*cp)[m][i][j][0] = (*cp1)[m][i][j][z_block_size];
               }
      }
   }
}

// Routine that does on processor communication between two blocks that are
// at different levels of refinement.  The order of the blocks that are
// being input determine which block is at a higher level of refinement.
void on_proc_comm_diff(Globals_t * const glbl, int n, int n1, int l, int iq, int jq,
                       int start, int num_comm)
{
   int i, j, k, m;
   int i0, i1, i2, i3, j0, j1, j2, j3, k0, k1, k2, k3;
   OBTAIN_ACCESS_TO_cells
   Cell_t  *cp, *cp1;
   TRACE

   cp = &cells[n];
   cp1 = &cells[n1];

   /* (iq, jq) quarter face on block n to whole face on block n1
   */
   if (!code) {
      /* only have to communicate ghost values - bp is level, bp1 is level+1 -
       * in 2 to 1 case get 0..block_half from one proc and
       *                block_half+1..block_size+1 from another
       * in 1 to 2 case get 0..block_size+1 from 0..block_half+1 or
       *                block_half..block_size+1 with averages
       */
      if ((l/2) == 0) {
         if (l == 0) {             /* West */
            i0 = 0;
            i1 = 1;
            i2 = x_block_size + 1;
            i3 = x_block_size;
         } else {                  /* East */
            i0 = x_block_size + 1;
            i1 = x_block_size;
            i2 = 0;
            i3 = 1;
         }
         j1 = jq*y_block_half;
         k1 = iq*z_block_half;
         for (m = start; m < start+num_comm; m++)
            for (j = 1; j <= y_block_half; j++)
               for (k = 1; k <= z_block_half; k++) {
                  (*cp1)[m][i2][2*j-1][2*k-1] =
                  (*cp1)[m][i2][2*j-1][2*k  ] =
                  (*cp1)[m][i2][2*j  ][2*k-1] =
                  (*cp1)[m][i2][2*j  ][2*k  ] =
                                             (*cp)[m][i1][j+j1][k+k1]/4.0;
                  (*cp)[m][i0][j+j1][k+k1] =
                                             (*cp1)[m][i3][2*j-1][2*k-1] +
                                             (*cp1)[m][i3][2*j-1][2*k  ] +
                                             (*cp1)[m][i3][2*j  ][2*k-1] +
                                             (*cp1)[m][i3][2*j  ][2*k  ];
               }
      } else if ((l/2) == 1) {
         if (l == 2) {             /* South */
            j0 = 0;
            j1 = 1;
            j2 = y_block_size + 1;
            j3 = y_block_size;
         } else {                  /* North */
            j0 = y_block_size + 1;
            j1 = y_block_size;
            j2 = 0;
            j3 = 1;
         }
         i1 = jq*x_block_half;
         k1 = iq*z_block_half;
         for (m = start; m < start+num_comm; m++)
            for (i = 1; i <= x_block_half; i++)
               for (k = 1; k <= z_block_half; k++) {
                  (*cp1)[m][2*i-1][j2][2*k-1] =
                  (*cp1)[m][2*i-1][j2][2*k  ] =
                  (*cp1)[m][2*i  ][j2][2*k-1] =
                  (*cp1)[m][2*i  ][j2][2*k  ] =
                                             (*cp)[m][i+i1][j1][k+k1]/4.0;
                  (*cp)[m][i+i1][j0][k+k1] =
                                             (*cp1)[m][2*i-1][j3][2*k-1] +
                                             (*cp1)[m][2*i-1][j3][2*k  ] +
                                             (*cp1)[m][2*i  ][j3][2*k-1] +
                                             (*cp1)[m][2*i  ][j3][2*k  ];
               }
      } else if ((l/2) == 2) {
         if (l == 4) {             /* Down */
            k0 = 0;
            k1 = 1;
            k2 = z_block_size + 1;
            k3 = z_block_size;
         } else {                  /* Up */
            k0 = z_block_size + 1;
            k1 = z_block_size;
            k2 = 0;
            k3 = 1;
         }
         i1 = jq*x_block_half;
         j1 = iq*y_block_half;
         for (m = start; m < start+num_comm; m++)
            for (i = 1; i <= x_block_half; i++)
               for (j = 1; j <= y_block_half; j++) {
                  (*cp1)[m][2*i-1][2*j-1][k2] =
                  (*cp1)[m][2*i-1][2*j  ][k2] =
                  (*cp1)[m][2*i  ][2*j-1][k2] =
                  (*cp1)[m][2*i  ][2*j  ][k2] =
                                              (*cp)[m][i+i1][j+j1][k1]/4.0;
                  (*cp)[m][i+i1][j+j1][k0] =
                                              (*cp1)[m][2*i-1][2*j-1][k3] +
                                              (*cp1)[m][2*i-1][2*j  ][k3] +
                                              (*cp1)[m][2*i  ][2*j-1][k3] +
                                              (*cp1)[m][2*i  ][2*j  ][k3];
               }
      }
   } else {  /* transfer ghosts */
      if ((l/2) == 0) {
         if (l == 0) {             /* West */
            i0 = 0;
            i1 = 1;
            i2 = x_block_size + 1;
            i3 = x_block_size;
         } else {                  /* East */
            i0 = x_block_size + 1;
            i1 = x_block_size;
            i2 = 0;
            i3 = 1;
         }
         j1 = jq*y_block_half;
         k1 = iq*z_block_half;
         j2 = y_block_size + 1;
         j3 = y_block_half + 1;
         k2 = z_block_size + 1;
         k3 = z_block_half + 1;
         for (m = start; m < start+num_comm; m++) {
            (*cp1)[m][i2][0][0] = (*cp)[m][i1][j1][k1]/4.0;
            for (k = 1; k <= z_block_half; k++)
               (*cp1)[m][i2][0][2*k-1] =
               (*cp1)[m][i2][0][2*k  ] = (*cp)[m][i1][j1][k+k1]/4.0;
            (*cp1)[m][i2][0][k2] = (*cp)[m][i1][j1][k3+k1]/4.0;
            if (jq == 0) {
               if (iq == 0)
                  (*cp)[m][i0][0][0 ] = (*cp1)[m][i3][0][0 ];
               else
                  (*cp)[m][i0][0][k2] = (*cp1)[m][i3][0][k2];
               for (k = 1; k <= z_block_half; k++)
                  (*cp)[m][i0][0][k+k1] = ((*cp1)[m][i3][0][2*k-1] +
                                               (*cp1)[m][i3][0][2*k  ]);
            }
            for (j = 1; j <= y_block_half; j++) {
               (*cp1)[m][i2][2*j-1][0] =
               (*cp1)[m][i2][2*j  ][0] = (*cp)[m][i1][j+j1][k1]/4.0;
               if (iq == 0)
                  (*cp)[m][i0][j+j1][0 ] = ((*cp1)[m][i3][2*j-1][0 ] +
                                                (*cp1)[m][i3][2*j  ][0 ]);
               else
                  (*cp)[m][i0][j+j1][k2] = ((*cp1)[m][i3][2*j-1][k2] +
                                                (*cp1)[m][i3][2*j  ][k2]);
               for (k = 1; k <= z_block_half; k++) {
                  (*cp1)[m][i2][2*j-1][2*k-1] =
                  (*cp1)[m][i2][2*j-1][2*k  ] =
                  (*cp1)[m][i2][2*j  ][2*k-1] =
                  (*cp1)[m][i2][2*j  ][2*k  ] =
                                             (*cp)[m][i1][j+j1][k+k1]/4.0;
                  (*cp)[m][i0][j+j1][k+k1] =
                                             (*cp1)[m][i3][2*j-1][2*k-1] +
                                             (*cp1)[m][i3][2*j-1][2*k  ] +
                                             (*cp1)[m][i3][2*j  ][2*k-1] +
                                             (*cp1)[m][i3][2*j  ][2*k  ];
               }
               (*cp1)[m][i2][2*j-1][k2] =
               (*cp1)[m][i2][2*j  ][k2] = (*cp)[m][i1][j+j1][k3+k1]/4.0;
            }
            (*cp1)[m][i2][j2][0] = (*cp)[m][i1][j3+j1][k1]/4.0;
            for (k = 1; k <= z_block_half; k++)
               (*cp1)[m][i2][j2][2*k-1] =
               (*cp1)[m][i2][j2][2*k  ] = (*cp)[m][i1][j3+j1][k+k1]/4.0;
            (*cp1)[m][i2][j2][k2] = (*cp)[m][i1][j3+j1][k3+k1]/4.0;
            if (jq == 1) {
               if (iq == 0)
                  (*cp)[m][i0][j2][0 ] = (*cp1)[m][i3][j2][0 ];
               else
                  (*cp)[m][i0][j2][k2] = (*cp1)[m][i3][j2][k2];
               for (k = 1; k <= z_block_half; k++)
                  (*cp)[m][i0][j2][k+k1] = ((*cp1)[m][i3][j2][2*k-1] +
                                                (*cp1)[m][i3][j2][2*k  ]);
            }
         }
      } else if ((l/2) == 1) {
         if (l == 2) {             /* South */
            j0 = 0;
            j1 = 1;
            j2 = y_block_size + 1;
            j3 = y_block_size;
         } else {                  /* North */
            j0 = y_block_size + 1;
            j1 = y_block_size;
            j2 = 0;
            j3 = 1;
         }
         i1 = jq*x_block_half;
         k1 = iq*z_block_half;
         i2 = x_block_size + 1;
         i3 = x_block_half + 1;
         k2 = z_block_size + 1;
         k3 = z_block_half + 1;
         for (m = start; m < start+num_comm; m++) {
            (*cp1)[m][0][j2][0 ] = (*cp)[m][i1][j1][k1]/4.0;
            for (k = 1; k <= z_block_half; k++)
               (*cp1)[m][0][j2][2*k-1] =
               (*cp1)[m][0][j2][2*k  ] = (*cp)[m][i1][j1][k+k1]/4.0;
            (*cp1)[m][0][j2][k2] = (*cp)[m][i1][j1][k3+k1]/4.0;
            if (jq == 0) {
               if (iq == 0)
                  (*cp)[m][0][j0][0 ] = (*cp1)[m][0][j3][0 ];
               else
                  (*cp)[m][0][j0][k2] = (*cp1)[m][0][j3][k2];
               for (k = 1; k <= z_block_half; k++)
                  (*cp)[m][0][j0][k+k1] = ((*cp1)[m][0][j3][2*k-1] +
                                               (*cp1)[m][0][j3][2*k  ]);
            }
            for (i = 1; i <= x_block_half; i++) {
               (*cp1)[m][2*i-1][j2][0] =
               (*cp1)[m][2*i  ][j2][0] = (*cp)[m][i+i1][j1][k1]/4.0;
               if (iq == 0)
                  (*cp)[m][i+i1][j0][0 ] = ((*cp1)[m][2*i-1][j3][0 ] +
                                                (*cp1)[m][2*i  ][j3][0 ]);
               else
                  (*cp)[m][i+i1][j0][k2] = ((*cp1)[m][2*i-1][j3][k2] +
                                                (*cp1)[m][2*i  ][j3][k2]);
               for (k = 1; k <= z_block_half; k++) {
                  (*cp1)[m][2*i-1][j2][2*k-1] =
                  (*cp1)[m][2*i-1][j2][2*k  ] =
                  (*cp1)[m][2*i  ][j2][2*k-1] =
                  (*cp1)[m][2*i  ][j2][2*k  ] =
                                             (*cp)[m][i+i1][j1][k+k1]/4.0;
                  (*cp)[m][i+i1][j0][k+k1] =
                                             (*cp1)[m][2*i-1][j3][2*k-1] +
                                             (*cp1)[m][2*i-1][j3][2*k  ] +
                                             (*cp1)[m][2*i  ][j3][2*k-1] +
                                             (*cp1)[m][2*i  ][j3][2*k  ];
               }
               (*cp1)[m][2*i-1][j2][k2] =
               (*cp1)[m][2*i  ][j2][k2] = (*cp)[m][i+i1][j1][k3+k1]/4.0;
            }
            (*cp1)[m][i2][j2][0 ] = (*cp)[m][i3+i1][j1][k1]/4.0;
            for (k = 1; k <= z_block_half; k++)
               (*cp1)[m][i2][j2][2*k-1] =
               (*cp1)[m][i2][j2][2*k  ] = (*cp)[m][i3+i1][j1][k+k1]/4.0;
            (*cp1)[m][i2][j2][k2] = (*cp)[m][i3+i1][j1][k3+k1]/4.0;
            if (jq == 1) {
               if (iq == 0)
                  (*cp)[m][i2][j0][0 ] = (*cp1)[m][i2][j3][0 ];
               else
                  (*cp)[m][i2][j0][k2] = (*cp1)[m][i2][j3][k2];
               for (k = 1; k <= z_block_half; k++)
                  (*cp)[m][i2][j0][k+k1] = ((*cp1)[m][i2][j3][2*k-1] +
                                                (*cp1)[m][i2][j3][2*k  ]);
            }
         }
      } else if ((l/2) == 2) {
         if (l == 4) {             /* Down */
            k0 = 0;
            k1 = 1;
            k2 = z_block_size + 1;
            k3 = z_block_size;
         } else {                  /* Up */
            k0 = z_block_size + 1;
            k1 = z_block_size;
            k2 = 0;
            k3 = 1;
         }
         i1 = jq*x_block_half;
         j1 = iq*y_block_half;
         i2 = x_block_size + 1;
         i3 = x_block_half + 1;
         j2 = y_block_size + 1;
         j3 = y_block_half + 1;
         for (m = start; m < start+num_comm; m++) {
            (*cp1)[m][0][0 ][k2] = (*cp)[m][i1][j1][k1]/4.0;
            for (j = 1; j <= y_block_half; j++)
               (*cp1)[m][0][2*j-1][k2] =
               (*cp1)[m][0][2*j  ][k2] = (*cp)[m][i1][j+j1][k1]/4.0;
            (*cp1)[m][0][j2][k2] = (*cp)[m][i1][j3+j1][k1]/4.0;
            if (jq == 0) {
               if (iq == 0)
                  (*cp)[m][0][0 ][k0] = (*cp1)[m][0][0 ][k3];
               else
                  (*cp)[m][0][j2][k0] = (*cp1)[m][0][j2][k3];
               for (j = 1; j <= y_block_half; j++)
                  (*cp)[m][0][j+j1][k0] = ((*cp1)[m][0][2*j-1][k3] +
                                               (*cp1)[m][0][2*j  ][k3]);
            }
            for (i = 1; i <= x_block_half; i++) {
               (*cp1)[m][2*i-1][0][k2] =
               (*cp1)[m][2*i  ][0][k2] = (*cp)[m][i+i1][j1][k1]/4.0;
               if (iq == 0)
                  (*cp)[m][i+i1][0][k0] = ((*cp1)[m][2*i-1][0][k3] +
                                               (*cp1)[m][2*i  ][0][k3]);
               else
                  (*cp)[m][i+i1][j2][k0] = ((*cp1)[m][2*i-1][j2][k3] +
                                                (*cp1)[m][2*i  ][j2][k3]);
               for (j = 1; j <= y_block_half; j++) {
                  (*cp1)[m][2*i-1][2*j-1][k2] =
                  (*cp1)[m][2*i-1][2*j  ][k2] =
                  (*cp1)[m][2*i  ][2*j-1][k2] =
                  (*cp1)[m][2*i  ][2*j  ][k2] =
                                              (*cp)[m][i+i1][j+j1][k1]/4.0;
                  (*cp)[m][i+i1][j+j1][k0] =
                                              (*cp1)[m][2*i-1][2*j-1][k3] +
                                              (*cp1)[m][2*i-1][2*j  ][k3] +
                                              (*cp1)[m][2*i  ][2*j-1][k3] +
                                              (*cp1)[m][2*i  ][2*j  ][k3];
               }
               (*cp1)[m][2*i-1][j2][k2] =
               (*cp1)[m][2*i  ][j2][k2] = (*cp)[m][i+i1][j3+j1][k1]/4.0;
            }
            (*cp1)[m][i2][0 ][k2] = (*cp)[m][i3+i1][j1][k1]/4.0;
            for (j = 1; j <= y_block_half; j++)
               (*cp1)[m][i2][2*j-1][k2] =
               (*cp1)[m][i2][2*j  ][k2] = (*cp)[m][i3+i1][j+j1][k1]/4.0;
            (*cp1)[m][i2][j2][k2] = (*cp)[m][i3+i1][j3+j1][k1]/4.0;
            if (jq == 1) {
               if (iq == 0)
                  (*cp)[m][i2][0 ][k0] = (*cp1)[m][i2][0 ][k3];
               else
                  (*cp)[m][i2][j2][k0] = (*cp1)[m][i2][j2][k3];
               for (j = 1; j <= y_block_half; j++)
                  (*cp)[m][i2][j+j1][k0] = ((*cp1)[m][i2][2*j-1][k3] +
                                                (*cp1)[m][i2][2*j  ][k3]);
            }
         }
      }
   }
}

// Apply reflective boundary conditions to a face of a block.
void apply_bc(Globals_t * const glbl, int l, void *cp_arg, int start, int num_comm)
{
   int var, i, j, k, f, t;

   OBTAIN_ACCESS_TO_cells
   Cell_t  *cp = (Cell_t *) cp_arg;
   TRACE

   t = 0;
   f = 1;
   if (!code && stencil == 7)
      switch (l) {
         case 1: t = x_block_size + 1;
                 f = x_block_size;
         case 0: for (var = start; var < start+num_comm; var++)
                    for (j = 1; j <= y_block_size; j++)
                       for (k = 1; k <= z_block_size; k++)
                          (*cp)[var][t][j][k] = (*cp)[var][f][j][k];
                 break;
         case 3: t = y_block_size + 1;
                 f = y_block_size;
         case 2: for (var = start; var < start+num_comm; var++)
                    for (i = 1; i <= x_block_size; i++)
                       for (k = 1; k <= z_block_size; k++)
                          (*cp)[var][i][t][k] = (*cp)[var][i][f][k];
                 break;
         case 5: t = z_block_size + 1;
                 f = z_block_size;
         case 4: for (var = start; var < start+num_comm; var++)
                    for (i = 1; i <= x_block_size; i++)
                       for (j = 1; j <= y_block_size; j++)
                          (*cp)[var][i][j][t] = (*cp)[var][i][j][f];
                 break;
      }
   else
      switch (l) {
         case 1: t = x_block_size + 1;
                 f = x_block_size;
         case 0: for (var = start; var < start+num_comm; var++)
                    for (j = 0; j <= y_block_size+1; j++)
                       for (k = 0; k <= z_block_size+1; k++)
                          (*cp)[var][t][j][k] = (*cp)[var][f][j][k];
                 break;
         case 3: t = y_block_size + 1;
                 f = y_block_size;
         case 2: for (var = start; var < start+num_comm; var++)
                    for (i = 0; i <= x_block_size+1; i++)
                       for (k = 0; k <= z_block_size+1; k++)
                          (*cp)[var][i][t][k] = (*cp)[var][i][f][k];
                 break;
         case 5: t = z_block_size + 1;
                 f = z_block_size;
         case 4: for (var = start; var < start+num_comm; var++)
                    for (i = 0; i <= x_block_size+1; i++)
                       for (j = 0; j <= y_block_size+1; j++)
                          (*cp)[var][i][j][t] = (*cp)[var][i][j][f];
                 break;
      }
}
#endif
