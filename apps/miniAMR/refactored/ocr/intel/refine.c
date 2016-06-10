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

#include "block.h"
#include "plot.h"
#include "proto.h"
//PROFILE:#include "profile.h"
#include "refine.h"
#include <ocr-guid-functions.h>

#ifdef NANNY_FUNC_NAMES
#line __LINE__ "refine "
#endif

#if 0
void dumpComms(ocrEdtDep_t depv[], int callNum, int const ts) {
   blockClone_Deps_t * myDeps       = (blockClone_Deps_t *) depv;
   BlockMeta_t       * meta         = myDeps->meta_Dep.ptr;
   printf ("dmp:%02d:%d,%d, lvl=%d, pos=%d,%d,%d %d:%04x/%04x,%02x/%02x,%02x/%02x,%02x/%02x %d:%04x/%04x,%02x/%02x,%02x/%02x,%02x/%02x %d:%04x/%04x,%02x/%02x,%02x/%02x,%02x/%02x %d:%04x/%04x,%02x/%02x,%02x/%02x,%02x/%02x %d:%04x/%04x,%02x/%02x,%02x/%02x,%02x/%02x %d:%04x/%04x,%02x/%02x,%02x/%02x,%02x/%02x\n", meta->cloningState.cloneNum, callNum, ts,
 meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos,
                        meta->neighborRefinementLevel[0][0] == 240 ? 9 : meta->neighborRefinementLevel[0][0],
 *((unsigned short *) (&meta->conveyFaceToNeighbor_Event[0][0][0])),
 *((unsigned short *) (&meta->onDeckToReceiveFace_Event [0][0][0])),
 *((unsigned char  *) (&meta->conveyFaceToNeighbor_Event[0][0][1])),
 *((unsigned char  *) (&meta->onDeckToReceiveFace_Event [0][0][1])),
 *((unsigned char  *) (&meta->conveyFaceToNeighbor_Event[0][0][2])),
 *((unsigned char  *) (&meta->onDeckToReceiveFace_Event [0][0][2])),
 *((unsigned char  *) (&meta->conveyFaceToNeighbor_Event[0][0][3])),
 *((unsigned char  *) (&meta->onDeckToReceiveFace_Event [0][0][3])),
                        meta->neighborRefinementLevel[0][1] == 240 ? 9 : meta->neighborRefinementLevel[0][1],
 *((unsigned short *) (&meta->conveyFaceToNeighbor_Event[0][1][0])),
 *((unsigned short *) (&meta->onDeckToReceiveFace_Event [0][1][0])),
 *((unsigned char  *) (&meta->conveyFaceToNeighbor_Event[0][1][1])),
 *((unsigned char  *) (&meta->onDeckToReceiveFace_Event [0][1][1])),
 *((unsigned char  *) (&meta->conveyFaceToNeighbor_Event[0][1][2])),
 *((unsigned char  *) (&meta->onDeckToReceiveFace_Event [0][1][2])),
 *((unsigned char  *) (&meta->conveyFaceToNeighbor_Event[0][1][3])),
 *((unsigned char  *) (&meta->onDeckToReceiveFace_Event [0][1][3])),
                        meta->neighborRefinementLevel[1][0] == 240 ? 9 : meta->neighborRefinementLevel[1][0],
 *((unsigned short *) (&meta->conveyFaceToNeighbor_Event[1][0][0])),
 *((unsigned short *) (&meta->onDeckToReceiveFace_Event [1][0][0])),
 *((unsigned char  *) (&meta->conveyFaceToNeighbor_Event[1][0][1])),
 *((unsigned char  *) (&meta->onDeckToReceiveFace_Event [1][0][1])),
 *((unsigned char  *) (&meta->conveyFaceToNeighbor_Event[1][0][2])),
 *((unsigned char  *) (&meta->onDeckToReceiveFace_Event [1][0][2])),
 *((unsigned char  *) (&meta->conveyFaceToNeighbor_Event[1][0][3])),
 *((unsigned char  *) (&meta->onDeckToReceiveFace_Event [1][0][3])),
                        meta->neighborRefinementLevel[1][1] == 240 ? 9 : meta->neighborRefinementLevel[1][1],
 *((unsigned short *) (&meta->conveyFaceToNeighbor_Event[1][1][0])),
 *((unsigned short *) (&meta->onDeckToReceiveFace_Event [1][1][0])),
 *((unsigned char  *) (&meta->conveyFaceToNeighbor_Event[1][1][1])),
 *((unsigned char  *) (&meta->onDeckToReceiveFace_Event [1][1][1])),
 *((unsigned char  *) (&meta->conveyFaceToNeighbor_Event[1][1][2])),
 *((unsigned char  *) (&meta->onDeckToReceiveFace_Event [1][1][2])),
 *((unsigned char  *) (&meta->conveyFaceToNeighbor_Event[1][1][3])),
 *((unsigned char  *) (&meta->onDeckToReceiveFace_Event [1][1][3])),
                        meta->neighborRefinementLevel[2][0] == 240 ? 9 : meta->neighborRefinementLevel[2][0],
 *((unsigned short *) (&meta->conveyFaceToNeighbor_Event[2][0][0])),
 *((unsigned short *) (&meta->onDeckToReceiveFace_Event [2][0][0])),
 *((unsigned char  *) (&meta->conveyFaceToNeighbor_Event[2][0][1])),
 *((unsigned char  *) (&meta->onDeckToReceiveFace_Event [2][0][1])),
 *((unsigned char  *) (&meta->conveyFaceToNeighbor_Event[2][0][2])),
 *((unsigned char  *) (&meta->onDeckToReceiveFace_Event [2][0][2])),
 *((unsigned char  *) (&meta->conveyFaceToNeighbor_Event[2][0][3])),
 *((unsigned char  *) (&meta->onDeckToReceiveFace_Event [2][0][3])),
                        meta->neighborRefinementLevel[2][1] == 240 ? 9 : meta->neighborRefinementLevel[2][1],
 *((unsigned short *) (&meta->conveyFaceToNeighbor_Event[2][1][0])),
 *((unsigned short *) (&meta->onDeckToReceiveFace_Event [2][1][0])),
 *((unsigned char  *) (&meta->conveyFaceToNeighbor_Event[2][1][1])),
 *((unsigned char  *) (&meta->onDeckToReceiveFace_Event [2][1][1])),
 *((unsigned char  *) (&meta->conveyFaceToNeighbor_Event[2][1][2])),
 *((unsigned char  *) (&meta->onDeckToReceiveFace_Event [2][1][2])),
 *((unsigned char  *) (&meta->conveyFaceToNeighbor_Event[2][1][3])),
 *((unsigned char  *) (&meta->onDeckToReceiveFace_Event [2][1][3]))
); fflush(stdout);
}
#else
#define dumpComms(a,b,c)
#endif

//  Understanding refinement decisions:
//
//  for (i = 0; i < num_refine_step; i++) {   // The number of times we try to refine any particular block is unlimited at time=0, and thereafter is a maximum of block_change (from command line arg).
//
//     Step 1: Set up to receive refinement disposition messages from any neighbor currently finer than me.  Simultaneously, assure the existence of datablocks for telling ALL my neighbors my own refinement
//             disposition.  For my finer neighbors, I just reuse the datablocks they sent me;  for other neighbors, I create the datablock; they will season when I clone.  Note that the finest existing
//             blocks won't have any incoming refinement disposition messages from neighbors.
//
//     Step 2: Clone my continuation.  Then examine all incoming refinement disposition messages from finer neighbors.
//             -- If any of them indicate their intent to refine, then change my own disposition to REFINE_BLK, i.e. intent to refine.
//             -- Otherwise, if any of them indicate they are going to stay at the same refinement level that they are presently (one level finer than me), then if I am marked UNREFINE_BLK,
//                change my disposition to LEAVE_BLK_AT_LVL.
//             -- Also destroy the events that brought this information to us, and set up the next At-Bat event.  But keep the datablock, because it will be used to return my disposition to that neighbor.
//
//     Step 3: Tell ALL my neighbors what my own refinement disposition is.  Coarser neighbors will then be able to proceed with their Step 2.  Finer neighbors and those currently at the same refinement
//             level will be able to proceed with Step 4.  Also record the refinement dispositions of my finer neighbors, taking into account the following tricky case:  Where one or more of my finer
//             neighbors indicated it is going finer still, that forced me to go finer, and my eight sibling prongs will have to record the appropriate relationship to the blocks that are now in that
//             direction.  Some of those new neighbors will be finer than the sibling prong; others will be at the same refinement level.
//
//     Step 4: Set up to receive refinement disposition messages from my remaining neighbors (those that are presentley coarser or the same level as me).
//
//     Step 5: Clone my continuation.  Then receive refinement disposition messages from my (presently) coarser and same-level neighbors.  Record their dispositions.  Also destroy the datablocks and the
//             events that brought this information to us, and set up the next At-Bat event.
//
//     Step 6: Blocks that are marked REFINE_BLK now do so, forking to eight children.  They all book-keep as to their new neighbor relationships.
//
//  }
//
//
// This file contains routines that determine which blocks are going to be refined and which are going to be coarsened.
void refine(blockClone_Params_t * myParams, ocrEdtDep_t depv[], int const ts) {

   typedef struct {
      Frame_Header_t myFrame;
      int                  faceCount;
      int                  num_refine_step;
      int                  finerNeighborIsRefining;
      RefinementDecision_t myRefinementDisposition;
      ocrGuid_t            serviceReturnEvent;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__refine_t;

#define faceCount                                       (lcl->faceCount)
#define num_refine_step                                 (lcl->num_refine_step)
#define finerNeighborIsRefining                         (lcl->finerNeighborIsRefining)
#define myRefinementDisposition                         (lcl->myRefinementDisposition)
#define serviceReturnEvent                              (lcl->serviceReturnEvent)

   blockClone_Deps_t * myDeps       = (blockClone_Deps_t *) depv;
   BlockMeta_t       * meta         = myDeps->meta_Dep.ptr;
   Control_t         * control      = myDeps->control_Dep.ptr;

   int axis;
   int pole;
   int qrtrLeftRight;
   int qrtrUpDown;

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__refine_t)

   num_refine_step = ((ts == 0) ? control->num_refine : control->block_change);

   myRefinementDisposition = check_objects(depv);           // Get initial disposition of block, taking into account movement of objects through the mesh over time.

   dumpComms(depv, 0, ts);

   while (num_refine_step-- > 0) {

      // Count how many halo faces and/or quarter faces there are across all faces of our block.

      faceCount = 0;
      for (axis = 0; axis < 3; axis++) {                                                  // For East/West, then South/North, then Down/Up
         for (pole = 0; pole <= 1; pole++) {                                              // West then East (for axis=0); South then North (for axis=1); Down then Up (for axis=2)
            if (meta->neighborRefinementLevel[axis][pole] == NEIGHBOR_IS_OFF_EDGE_OF_MESH) continue; /* neighbor non-existent */
            faceCount += (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel + 1) ?  /* neighbor finer        */ 4 : /* neighbor same or coarser*/ 1;
         }
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
                              "blockClone (Continuation -- obtain datablocks for refinement disposition out)");
      } else {
         meta->blockClone_Edt = NULL_GUID;
      }

      int auxDepIdx = 0;

      // Step 1: Set up to receive refinement disposition messages from any neighbor currently finer than me.  Simultaneously, assure the existence of datablocks for telling ALL my neighbors my own refinement
      //         disposition.  For my finer neighbors, I just reuse the datablocks they sent me;  for other neighbors, I create the datablock; they will season when I clone.  Note that the finest existing
      //         blocks won't have any incoming refinement disposition messages from neighbors.

      for (axis = 0; axis < 3; axis++) {                                                  // For East/West, then South/North, then Down/Up
         for (pole = 0; pole < 2; pole++) {                                               // (East, South, or Down), then (West, North, or Up)
            if (meta->neighborRefinementLevel[axis][pole] == NEIGHBOR_IS_OFF_EDGE_OF_MESH) continue;
            meta->cloningState.cloningOpcode = Special;
            if (meta->neighborRefinementLevel[axis][pole] > meta->refinementLevel) {      // Neighbor is finer.  We need to receive information from her -- i.e. all four quarters of her!
               int idep = 0;
               for (qrtrLeftRight = 0; qrtrLeftRight <= 1; qrtrLeftRight++) {
                  for (qrtrUpDown = 0; qrtrUpDown    <= 1; qrtrUpDown++) {
                     // Set up to hear from finer neighbors what their refinement disposition is:
#ifdef NANNY_ON_STEROIDS
                     char nanny[NANNYLEN];
                     sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, qrtrLR=%d, qrtrUD=%d, nei=finer, [auxDepIdx=%d+%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, qrtrLeftRight, qrtrUpDown, auxDepIdx, idep);
#else
#define nanny NULL
#endif
                     ocrGuid_t dispIn = meta->onDeckToReceiveFace_Event[axis][pole][idep];             // Assure our clone gets the info from the finer nbr.
                     meta->onDeckToReceiveFace_Event[axis][pole][idep] = NULL_GUID;                    // Assure we don't reuse a stale guid later.
                     if (ocrGuidIsNull(dispIn)) {                                                      // First time.  Use labeled guid.
                        int nbr_xPos = meta->xPos << 1;                                                // For starters, our own position at finer level is just our position scaled up one level.
                        int nbr_yPos = meta->yPos << 1;
                        int nbr_zPos = meta->zPos << 1;
                        if (axis == 0) {
                           nbr_xPos += (pole == 0) ? -1 : 2;
                           nbr_yPos += qrtrLeftRight;
                           nbr_zPos += qrtrUpDown;
                        } else if (axis == 1) {
                           nbr_yPos += (pole == 0) ? -1 : 2;
                           nbr_xPos += qrtrLeftRight;
                           nbr_zPos += qrtrUpDown;
                        } else {
                           nbr_zPos += (pole == 0) ? -1 : 2;
                           nbr_xPos += qrtrLeftRight;
                           nbr_yPos += qrtrUpDown;
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
                        gasket__ocrGuidFromIndex(&dispIn, meta->labeledGuidRangeForHaloExchange, index, __FILE__, __func__, __LINE__, nanny, "dispIn[...]");
#define DEFAULT_LG_PROPS GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG
                        gasket__ocrEventCreate  (&dispIn, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS,         __FILE__, __func__, __LINE__, nanny, "dispIn[...]");
                     }
                     EVT_DEPENDENCE(dispIn, meta->blockClone_Edt, blockClone_Deps_t, aux_Dep[auxDepIdx+idep], DB_MODE_RW, nanny, "dispIn[...]");
                     idep++;
                  }
               }
               auxDepIdx += 4;
            } else {   // Create a datablock whereby my clone will tell my currently-coarser-or-same-level neighbor what my refinement disposition becomes after Step 2 has been processed.
#ifdef NANNY_ON_STEROIDS
               char nanny[200];
               sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, nei=coarser-or-same, axis=%c, [auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, (axis==0)?(pole==0?'W':'E'):((axis==1)?(pole==0?'S':'N'):(pole==0?'D':'U')), auxDepIdx);
#else
#define nanny NULL
#endif
               ocrGuid_t dispOut;
               void * dummy;
               gasket__ocrDbCreate(&dispOut, (void **) &dummy, sizeof_RefinementDisposition_t, __FILE__, __func__, __LINE__, nanny, "dispOut[...]");
               ADD_DEPENDENCE(dispOut, meta->blockClone_Edt, blockClone_Deps_t, aux_Dep[auxDepIdx], DB_MODE_RW, nanny, "dispOut[...]");
               auxDepIdx++;
            }
         }
      }

      if (meta->cloningState.cloningOpcode == Special) {     // Only case this would be false is if we have no neighbors at all -- a silly corner-case.

         // Step 2: Clone my continuation.  Then examine all incoming refinement disposition messages from finer neighbors.
         //         -- If any of them indicate their intent to refine, then change my own disposition to REFINE_BLK, i.e. intent to refine.
         //         -- Otherwise, if any of them indicate they are going to stay at the same refinement level that they are presently (one level finer than me), then if I am marked UNREFINE_BLK,
         //            change my disposition to LEAVE_BLK_AT_LVL.
         //         -- Also destroy the events that brought this information to us, and set up the next At-Bat event.  But keep the datablock, because it will be used to return my disposition to that neighbor.

         SUSPEND__RESUME_IN_CLONE_EDT(;)

         auxDepIdx = 0;
         finerNeighborIsRefining = 0;                                                        // Bit vector, set to one for any finer neighbor who is going still finer.  Bit vector ONLY spans finer neighbors.
         int bit = 1;
         for (axis = 0; axis < 3; axis++) {                                                  // For East/West, then South/North, then Down/Up
            for (pole = 0; pole < 2; pole++) {                                               // (East, South, or Down), then (West, North, or Up)
               if (meta->neighborRefinementLevel[axis][pole] == NEIGHBOR_IS_OFF_EDGE_OF_MESH) continue;
               if (meta->neighborRefinementLevel[axis][pole] > meta->refinementLevel) {      // Neighbor is finer.  We need to receive information from her -- i.e. all four quarters of her!
                  for (qrtrLeftRight = 0; qrtrLeftRight <= 1; qrtrLeftRight++) {
                     for (qrtrUpDown = 0; qrtrUpDown    <= 1; qrtrUpDown++) {
                        RefinementDisposition_t * dispIn = myDeps->aux_Dep[auxDepIdx++].ptr;
                        if (dispIn->refinementDisposition == REFINE_BLK) {                   // Neighbor (who is already finer) plans to refine.  I must do likewise!
                           finerNeighborIsRefining |= bit;                                   // Set bit vector of finer neighbor who is going still finer.
                           myRefinementDisposition = REFINE_BLK;
                        } else if (dispIn->refinementDisposition == LEAVE_BLK_AT_LVL &&      // My finer neighbor does NOT plan to refine, but he can't unrefine either.
                                   myRefinementDisposition != REFINE_BLK) {                  // Unless I've been marked for refining, I must be at least at the disposition of LEAVE_BLK_AT_LVL
                           myRefinementDisposition = LEAVE_BLK_AT_LVL;
                        } else {                                                             // My finer neighbor is marked as potentially unrefinable.  My disposition remains unchanged.
                        }
                        bit <<= 1;
                     }
                  }
               } else {
                  auxDepIdx++;   // Skip over coarser or same-level neighbor (for now -- we will write our disposition to them presently, and then hear back from them later).
               }
            }
         }

         auxDepIdx = 0;
         if (myRefinementDisposition == REFINE_BLK) {                                           // Concensus was for me to refine.
            for (axis = 0; axis < 3; axis++) {                                                  // For East/West, then South/North, then Down/Up
               for (pole = 0; pole < 2; pole++) {                                               // (East, South, or Down), then (West, North, or Up)
                  if (meta->neighborRefinementLevel[axis][pole] == NEIGHBOR_IS_OFF_EDGE_OF_MESH) continue;
                  if (meta->neighborRefinementLevel[axis][pole] > meta->refinementLevel) {      // Neighbor is finer.  I need to do my part in severing the channel from her to me.
                     int idep = 0;
                     for (qrtrLeftRight = 0; qrtrLeftRight <= 1; qrtrLeftRight++) {
                        for (qrtrUpDown = 0; qrtrUpDown    <= 1; qrtrUpDown++) {
                           RefinementDisposition_t * dispIn = myDeps->aux_Dep[auxDepIdx++].ptr;
                           if (dispIn->refinementDisposition == REFINE_BLK) {                   // Neighbor (who is already finer) plans to refine.  She should NOT have sent me an onDeck_Event.
                              if (!ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {          // Sanity check:  Finer neighbor is refining.  She should NOT have sent me an onDeck Event!
                                 *((int *) 123) = 456;
                              }
                              if (!ocrGuidIsNull(meta->onDeckToReceiveFace_Event[axis][pole][idep])) {  // Sanity check:  I should have NULL_GUID in onDeck Event, because I nullified it last time I consumed it.
                                 *((int *) 123) = 456;
                              }

                           } else {                                                             // Finer neighbor doesn't plan to refine, but I do.  She sent me an onDeck_Event, but I want to sever the channel.
                                                                                                // When I send her my intention to refine, she will reciprocate the severing, by abandoning the onDeck_Event.
                              if (ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {           // Sanity check:  Finer neighbor is NOT refining.  She should have send me an onDeck Event!
                                 *((int *) 123) = 456;
                              }
                              if (!ocrGuidIsNull(meta->onDeckToReceiveFace_Event[axis][pole][idep])) {  // Sanity check:  I should have NULL_GUID in onDeck Event, because I nullified it last time I consumed it.
                                 *((int *) 123) = 456;
                              }
                           }
#ifdef NANNY_ON_STEROIDS
                           char nanny[200];
                           sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d,pole=%d,qrtrLR=%dqrtrUD=%d,[auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, qrtrLeftRight, qrtrUpDown, auxDepIdx);
#else
#define nanny NULL
#endif
                           gasket__ocrEventDestroy(&dispIn->dbCommHeader.atBat_Event, __FILE__, __func__, __LINE__, nanny, "aux_Dep[...]");        // Destroy event that brought us our disp DB.
                           idep++;
                        }
                     }
                  } else {
                     auxDepIdx++;   // Skip over coarser or same-level neighbor (for now -- we will write our disposition to them presently, and then hear back from them later).
                  }
               }
            }
         } else if (myRefinementDisposition == LEAVE_BLK_AT_LVL) {                              // Concensus was for me to stay at the current level.
            for (axis = 0; axis < 3; axis++) {                                                  // For East/West, then South/North, then Down/Up
               for (pole = 0; pole < 2; pole++) {                                               // (East, South, or Down), then (West, North, or Up)
                  if (meta->neighborRefinementLevel[axis][pole] == NEIGHBOR_IS_OFF_EDGE_OF_MESH) continue;
                  if (meta->neighborRefinementLevel[axis][pole] > meta->refinementLevel) {      // Neighbor is finer.  Handle the channel that she uses to talk to me.
                     int idep = 0;
                     for (qrtrLeftRight = 0; qrtrLeftRight <= 1; qrtrLeftRight++) {
                        for (qrtrUpDown = 0; qrtrUpDown    <= 1; qrtrUpDown++) {
                           RefinementDisposition_t * dispIn = myDeps->aux_Dep[auxDepIdx++].ptr;
                           if (dispIn->refinementDisposition == REFINE_BLK) {                   // Neighbor (who is already finer) plans to refine.  She should NOT have sent me an onDeck_Event.
                              *((int *) 123) = 456;                                             // But we are coarser and NOT refining.  That is not possible!
                           } else {                                                             // Finer neighbor doesn't plan to refine, but I do.  She sent me an onDeck_Event, but I want to sever the channel.
                                                                                                // When I send her my intention to refine, she will reciprocate the severing, by abandoning the onDeck_Event.
                              if (ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {           // Sanity check:  Finer neighbor is NOT refining.  She should have send me an onDeck Event!
                                 *((int *) 123) = 456;
                              }
                              if (!ocrGuidIsNull(meta->onDeckToReceiveFace_Event[axis][pole][idep])) {  // Sanity check:  I should have NULL_GUID in onDeck Event, because I nullified it last time I consumed it.
                                 *((int *) 123) = 456;
                              }
                              meta->onDeckToReceiveFace_Event[axis][pole][idep] = dispIn->dbCommHeader.onDeck_Event;  // Accept my neighbor's onDeck event.
                           }
#ifdef NANNY_ON_STEROIDS
                           char nanny[200];
                           sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d,pole=%d,qrtrLR=%dqrtrUD=%d,[auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, qrtrLeftRight, qrtrUpDown, auxDepIdx);
#else
#define nanny NULL
#endif
                           gasket__ocrEventDestroy(&dispIn->dbCommHeader.atBat_Event, __FILE__, __func__, __LINE__, nanny, "aux_Dep[...]");        // Destroy event that brought us our disp DB.
                           idep++;
                        }
                     }
                  } else {
                     auxDepIdx++;   // Skip over coarser or same-level neighbor (for now -- we will write our disposition to them presently, and then hear back from them later).
                  }
               }
            }
         } else {                                                                               // Concensus is that I could potential unrefine.  Do the same as LEAVE_BLK_AT_LVL, but diagnose bugs separately
            for (axis = 0; axis < 3; axis++) {                                                  // For East/West, then South/North, then Down/Up
               for (pole = 0; pole < 2; pole++) {                                               // (East, South, or Down), then (West, North, or Up)
                  if (meta->neighborRefinementLevel[axis][pole] == NEIGHBOR_IS_OFF_EDGE_OF_MESH) continue;
                  if (meta->neighborRefinementLevel[axis][pole] > meta->refinementLevel) {      // Neighbor is finer.  Handle the channel that she uses to talk to me.
                     int idep = 0;
                     for (qrtrLeftRight = 0; qrtrLeftRight <= 1; qrtrLeftRight++) {
                        for (qrtrUpDown = 0; qrtrUpDown    <= 1; qrtrUpDown++) {
                           RefinementDisposition_t * dispIn = myDeps->aux_Dep[auxDepIdx++].ptr;
                           if (dispIn->refinementDisposition == REFINE_BLK) {                   // Neighbor (who is already finer) plans to refine.  She should NOT have sent me an onDeck_Event.
                              *((int *) 123) = 456;                                             // But we are coarser and NOT refining.  That is not possible!
                           } else {                                                             // Finer neighbor doesn't plan to refine, but I do.  She sent me an onDeck_Event, but I want to sever the channel.
                                                                                                // When I send her my intention to refine, she will reciprocate the severing, by abandoning the onDeck_Event.
                              if (ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {           // Sanity check:  Finer neighbor is NOT refining.  She should have send me an onDeck Event!
                                 *((int *) 123) = 456;
                              }
                              if (!ocrGuidIsNull(meta->onDeckToReceiveFace_Event[axis][pole][idep])) {  // Sanity check:  I should have NULL_GUID in onDeck Event, because I nullified it last time I consumed it.
                                 *((int *) 123) = 456;
                              }
                              meta->onDeckToReceiveFace_Event[axis][pole][idep] = dispIn->dbCommHeader.onDeck_Event;  // Accept my neighbor's onDeck event.
                           }
#ifdef NANNY_ON_STEROIDS
                           char nanny[200];
                           sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d,pole=%d,qrtrLR=%dqrtrUD=%d,[auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, qrtrLeftRight, qrtrUpDown, auxDepIdx);
#else
#define nanny NULL
#endif
                           gasket__ocrEventDestroy(&dispIn->dbCommHeader.atBat_Event, __FILE__, __func__, __LINE__, nanny, "aux_Dep[...]");        // Destroy event that brought us our disp DB.
                           idep++;
                        }
                     }
                  } else {
                     auxDepIdx++;   // Skip over coarser or same-level neighbor (for now -- we will write our disposition to them presently, and then hear back from them later).
                  }
               }
            }
         }


         // Step 3: Tell ALL my neighbors what my own refinement disposition is.  Coarser neighbors will then be able to proceed with their Step 2.  Finer neighbors and those currently at the same refinement
         //         level will be able to proceed with Step 4.  Also record the refinement dispositions of my finer neighbors, taking into account the following tricky case:  Where one or more of my finer
         //         neighbors indicated it is going finer still, that forced me to go finer, and my eight child prongs will have to record the appropriate relationship to the blocks that are now in that
         //         direction.  Some of those new neighbors will be finer than the child prong; others will be at the same refinement level.

         auxDepIdx = 0;
         int auxDepIdx_notFiner = 0;                                                                 // For counting how many of my neighbors are coarser or same level.
         for (axis = 0; axis < 3; axis++) {                                                          // For East/West, then South/North, then Down/Up
            for (pole = 0; pole < 2; pole++) {                                                       // (East, South, or Down), then (West, North, or Up)
               if (meta->neighborRefinementLevel[axis][pole] == NEIGHBOR_IS_OFF_EDGE_OF_MESH) continue;
               if (meta->neighborRefinementLevel[axis][pole] <= meta->refinementLevel) {             // Tell my coarser and same-level neighbors my disposition
                  RefinementDisposition_t * dispOut = myDeps->aux_Dep[auxDepIdx].ptr;
                  dispOut->refinementDisposition = myRefinementDisposition;
                  ocrGuid_t conveyDispToNeighbor_Event = meta->conveyFaceToNeighbor_Event[axis][pole][0];
#ifdef NANNY_ON_STEROIDS
                  char nanny[NANNYLEN];
                  sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, [auxDepIdx=%d], nei=coarser or same", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, auxDepIdx);
#else
#define nanny NULL
#endif
                  if (ocrGuidIsNull(conveyDispToNeighbor_Event)) {                                   // First time.  Use labeled guid.
                     unsigned long index =                                                           // The first component gets us the index of the base set of 36 applicable for the level: i.e.
                        ((((0111111111111111111111L >> (63L - (meta->refinementLevel*3))) *          // lvl 0: 0;  lvl 1: 1;  lvl 2: 9 (i.e. 1+8);  lvl 2: 73 (i.e. 1+8+8*8); lvl 3: 585 (i.e. 1+8+8*8+8*8*8) ...
                           meta->numBaseResBlocks) +                                                 // ... scaled up by number of blocks in the totally-unrefined mesh.
                          ((((meta->xPos * (control->npy << meta->refinementLevel)) +                // Here we subscript-linearize the x, y, z position of the block within its refinement level and add it
                             meta->yPos) * (control->npz << meta->refinementLevel)) +                // to the above base. ...
                           meta->zPos))  *
                         36) +                                                                       // Then we scale all that by 36 (6 for when nbr coarser; six for when nbr same; 24 for when nbr finer).
                        axis * 2 + pole +                                                            // Now select which of the 36 comm events to use: for nbr coarser, 0=W, 1=E, 2=S, 3=N,  4=D,  5=U, ...
                        (meta->neighborRefinementLevel[axis][pole] < meta->refinementLevel ? 0 : 6); // ... but for nbr at same refinement level:                       6=W, 7=E, 8=S, 9=N, 10=D, 11=U.
                     gasket__ocrGuidFromIndex(&conveyDispToNeighbor_Event, meta->labeledGuidRangeForHaloExchange, index, __FILE__, __func__, __LINE__, nanny, "conveyDispToNeighbor");
                     gasket__ocrEventCreate(&conveyDispToNeighbor_Event, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS, __FILE__, __func__, __LINE__, nanny, "conveyFaceToNeighbor_Event[...][...][0]");  // Create the On Deck Event; record it in our meta.
                  }
                  dispOut->dbCommHeader.atBat_Event = conveyDispToNeighbor_Event;                    // Convey At Bat Event to neighbor so that she can destroy the event.
                  if (myRefinementDisposition == REFINE_BLK) {                                       // If I am refining, squash the channel to my coarser or same-level neighbor (and to finer nbrs, below).
                     meta->conveyFaceToNeighbor_Event[axis][pole][0] = NULL_GUID;
                  } else {                                                                           // But if I am NOT refining, then (speculatively) renew the event by which I send stuff to that nbr.
                     gasket__ocrEventCreate(&meta->conveyFaceToNeighbor_Event[axis][pole][0], OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG, __FILE__, __func__, __LINE__, nanny, "conveyDispToNeighbor_Event[...][...][0]");  // Create the On Deck Event; record it in our meta.
                  }
                  dispOut->dbCommHeader.onDeck_Event = meta->conveyFaceToNeighbor_Event[axis][pole][0];  // Convey On Deck Event to neighbor so that she can make her clone depend upon it.
                  gasket__ocrDbRelease(                               myDeps->aux_Dep[auxDepIdx].guid, __FILE__, __func__, __LINE__, nanny, "dispOut[...]");
                  gasket__ocrEventSatisfy(conveyDispToNeighbor_Event, myDeps->aux_Dep[auxDepIdx].guid, __FILE__, __func__, __LINE__, nanny, "dispOut[...] via conveyDispToNeighbor"); // Satisfy the neighbors's dependence for this face.
                  auxDepIdx++;
                  auxDepIdx_notFiner++;
               } else {                                                                              // Tell my finer neighbors my disposition.
                  int idep = 0;
                  for (qrtrLeftRight = 0; qrtrLeftRight <= 1; qrtrLeftRight++) {
                     for (qrtrUpDown = 0; qrtrUpDown    <= 1; qrtrUpDown++) {
#ifdef NANNY_ON_STEROIDS
                        char nanny[NANNYLEN];
                        sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, qrtrLR=%d, qrtrUD=%d, [auxDepIdx=%d+%d], nei=finer", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, qrtrLeftRight, qrtrUpDown, auxDepIdx, idep);
#else
#define nanny NULL
#endif
                        RefinementDisposition_t * dispOut = myDeps->aux_Dep[auxDepIdx+idep].ptr;
                        dispOut->refinementDisposition = myRefinementDisposition;
                        ocrGuid_t conveyDispToNeighbor_Event = meta->conveyFaceToNeighbor_Event[axis][pole][idep];
                        if (ocrGuidIsNull(conveyDispToNeighbor_Event)) {                             // First time.
                           unsigned long index =                                                     // The first component gets us the index of the base set of 36 applicable for the level: i.e.
                              ((((0111111111111111111111L >> (63L - (meta->refinementLevel*3))) *    // lvl 0: 0;  lvl 1: 1;  lvl 2: 9 (i.e. 1+8);  lvl 2: 73 (i.e. 1+8+8*8); lvl 3: 585 (i.e. 1+8+8*8+8*8*8) ...
                                 meta->numBaseResBlocks) +                                           // ... scaled up by number of blocks in the totally-unrefined mesh.
                                ((((meta->xPos * (control->npy << meta->refinementLevel)) +          // Here we subscript-linearize the x, y, z position of the block within its refinement level and add it
                                   meta->yPos) * (control->npz << meta->refinementLevel)) +          // to the above base. ...
                                 meta->zPos))  *
                               36) +                                                                 // Then we scale all that by 36 (6 for when nbr coarser; six for when nbr same; 24 for when nbr finer).
                              axis * 8 + pole *4 + idep + 12;                                        // Now select: 12:15 = W, 16:19 = E, 20:23 = S, 24:27 = N, 28:31 = D, 32:35 = U, neighbor finer.
                           gasket__ocrGuidFromIndex(&conveyDispToNeighbor_Event, meta->labeledGuidRangeForHaloExchange, index, __FILE__, __func__, __LINE__, nanny, "conveyDispToNeighbor");
                           gasket__ocrEventCreate(&conveyDispToNeighbor_Event, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS, __FILE__, __func__, __LINE__, nanny, "conveyDispToNeighbor_Event[...][...][...]");  // Create the On Deck Event; record it in our meta.
                        }
                        dispOut->dbCommHeader.atBat_Event = conveyDispToNeighbor_Event;              // Convey At Bat Event to neighbor so that she can destroy the event.
                        if (myRefinementDisposition == REFINE_BLK) {                                 // If I am refining, squash the channel to my finer neighbor (regardless of whether or not she is refining).
                           meta->conveyFaceToNeighbor_Event[axis][pole][idep] = NULL_GUID;
                        } else {                                                                     // But if neither of us is refining, then renew the event by which I send stuff to that nbr.
                           gasket__ocrEventCreate(&meta->conveyFaceToNeighbor_Event[axis][pole][idep], OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG, __FILE__, __func__, __LINE__, nanny, "conveyFaceToNeighbor_Event[...][...][...]");  // Create the On Deck Event; record it in our meta.
                        }
                        dispOut->dbCommHeader.onDeck_Event = meta->conveyFaceToNeighbor_Event[axis][pole][idep]; // Convey On Deck Event to neighbor so that she can make her clone depend upon it.
                        gasket__ocrDbRelease(                               myDeps->aux_Dep[auxDepIdx+idep].guid, __FILE__, __func__, __LINE__, nanny, "dispOut[...]");
                        gasket__ocrEventSatisfy(conveyDispToNeighbor_Event, myDeps->aux_Dep[auxDepIdx+idep].guid, __FILE__, __func__, __LINE__, nanny, "dispOut[...]");
                        idep++;
                     }
                  }
                  meta->neighborRefinementLevel[axis][pole] = meta->refinementLevel + 1 + ((finerNeighborIsRefining & 0xF) << 4);  // Top four bits indicate which finer neighbors are refining.
                  finerNeighborIsRefining >>= 4;
                  auxDepIdx += 4;
               }
            }
         }


         // Step 4: Set up to receive refinement disposition messages from my remaining neighbors (those that are presently coarser or the same level as me).

         if (auxDepIdx_notFiner != 0) {
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
                                 countof_blockClone_fixedDeps_t + auxDepIdx_notFiner,
                                 NULL,
                                 EDT_PROP_NONE,
                                 NULL_HINT,
                                 NULL,
                                 __FILE__,
                                 __func__,
                                 __LINE__,
                                 nanny,
                                 "blockClone (Continuation -- obtain refinement disposition from coarser and sameLvl neighbors)");

            auxDepIdx = 0;
            meta->cloningState.cloningOpcode = Special;

            for (axis = 0; axis < 3; axis++) {                                                               // For East/West, then South/North, then Down/Up
               for (pole = 0; pole < 2; pole++) {                                                            // (East, South, or Down), then (West, North, or Up)
                  if (meta->neighborRefinementLevel[axis][pole] == NEIGHBOR_IS_OFF_EDGE_OF_MESH) continue;   // Neighbor is off edge of mesh domain, i.e. non-existent.
                  if (meta->neighborRefinementLevel[axis][pole] > meta->refinementLevel)         continue;   // Neighbor is finer.  Already handled earlier.
                  ocrGuid_t dispIn = meta->onDeckToReceiveFace_Event[axis][pole][0];
                  meta->onDeckToReceiveFace_Event[axis][pole][0] = NULL_GUID;                                // Assure we don't reuse a stale guid later.
                  unsigned long index = 9999;
#ifdef NANNY_ON_STEROIDS
                  char nanny[NANNYLEN];
#else
#define nanny NULL
#endif
                  if (meta->neighborRefinementLevel[axis][pole] < meta->refinementLevel) {      // Neighbor is coarser.  Set up to receive the disposition datablock from her.
#ifdef NANNY_ON_STEROIDS
                     sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, [auxDepIdx=%d], nei=coarser", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, auxDepIdx);
#endif
                     if (ocrGuidIsNull(dispIn)) {                                               // First time.  Use labeled guid.
                        int nbr_xPos = meta->xPos >> 1;                                         // For starters, our own position at coarser level is just our position scaled down one level.
                        int nbr_yPos = meta->yPos >> 1;
                        int nbr_zPos = meta->zPos >> 1;
                        int qrtrFaceOfNbrToUse = 999;
                        if (axis == 0) {                                                        // Then neighbor's position is one away, depending on axis and pole; and we need to note which qrtr-face to use.
                           nbr_xPos += (pole << 1) - 1;                                         // Adjust by -1 or 1.
                           qrtrFaceOfNbrToUse = ((meta->yPos & 1) << 1) + (meta->zPos & 1);
                        } else if (axis == 1) {
                           nbr_yPos += (pole << 1) - 1;                                         // Adjust by -1 or 1.
                           qrtrFaceOfNbrToUse = ((meta->xPos & 1) << 1) + (meta->zPos & 1);
                        } else {
                           nbr_zPos += (pole << 1) - 1;                                         // Adjust by -1 or 1.
                           qrtrFaceOfNbrToUse = ((meta->xPos & 1) << 1) + (meta->yPos & 1);
                        }
                        unsigned long index =                                                   // The first component gets us the index of the base set of 36 applicable for the level: i.e.
                           (((((0111111111111111111111L >> (66L - (meta->refinementLevel*3))) * // lvl 0: 0;  lvl 1: 1;  lvl 2: 9 (i.e. 1+8);  lvl 2: 73 (i.e. 1+8+8*8); lvl 3: 585 (i.e. 1+8+8*8+8*8*8) ...
                               meta->numBaseResBlocks) +                                        // ... scaled up by number of blocks in the totally-unrefined mesh.
                              (((((nbr_xPos) * (control->npy << (meta->refinementLevel-1))) +   // Here we subscript-linearize the x, y, z position of the block within its refinement level and add it
                                 (nbr_yPos)) * (control->npz << (meta->refinementLevel-1))) +   // to the above base. ...
                               (nbr_zPos))) *                                                   // ...
                             36) +                                                              // ... we multiply by 36 (6 for when nbr coarser; six for when nbr same; 24 for when nbr finer).  Then...
                            ((axis * 8 + pole * 4) ^ 4) + qrtrFaceOfNbrToUse + 12);             // Now select: 12:15 = W, 16:19 = E, 20:23 = S, 24:27 = N, 28:31 = D, 32:35 = U, neighbor finer.
                                                                                                // BUT note that the "^4" operation is to FLIP POLE: my W comms with nbrs E; etc.
                        gasket__ocrGuidFromIndex(&dispIn, meta->labeledGuidRangeForHaloExchange, index, __FILE__, __func__, __LINE__, nanny, "dispIn[...]");
                        gasket__ocrEventCreate(  &dispIn, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS,         __FILE__, __func__, __LINE__, nanny, "dispIn[...]");
                     }
                  } else {                                                                      // Neighbor is the same level.  Set up to receive the disposition datablock from her.
#ifdef NANNY_ON_STEROIDS
                     sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, [auxDepIdx=%d], nei=sameLvl", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, auxDepIdx);
#endif
                     if (ocrGuidIsNull(dispIn)) {                                               // First time.  Use labeled guid.
                        int nbr_xPos = meta->xPos +                                             // If NOT doing x-axis halo exchange, neighbor's xPos is the same as ours...
                           ((axis != 0) ? 0 : (pole == 0) ? -1 : 1);                            // otherwise, it is one off in the direction of the pole being processed.
                        int nbr_yPos = meta->yPos +                                             // If NOT doing y-axis halo exchange, neighbor's yPos is the same as ours...
                           ((axis != 1) ? 0 : (pole == 0) ? -1 : 1);                            // otherwise, it is one off in the direction of the pole being processed.
                        int nbr_zPos = meta->zPos +                                             // If NOT doing z-axis halo exchange, neighbor's zPos is the same as ours...
                           ((axis != 2) ? 0 : (pole == 0) ? -1 : 1);                            // otherwise, it is one off in the direction of the pole being processed.
                        index =                                                                 // The first component gets us the index of the base set of 36 applicable for the level: i.e.
                           (((((0111111111111111111111L >> (63L - (meta->refinementLevel*3))) * // lvl 0: 0;  lvl 1: 1;  lvl 2: 9 (i.e. 1+8);  lvl 2: 73 (i.e. 1+8+8*8); lvl 3: 585 (i.e. 1+8+8*8+8*8*8) ...
                               meta->numBaseResBlocks) +                                        // ... scaled up by number of blocks in the totally-unrefined mesh.
                              (((((nbr_xPos) * (control->npy << (meta->refinementLevel))) +     // Here we subscript-linearize the x, y, z position of the block within its refinement level and add it
                                 (nbr_yPos)) * (control->npz << (meta->refinementLevel))) +     // to the above base. ...
                               (nbr_zPos))) *                                                   // ...
                             36) +                                                              // ... we multiply by 36 (6 for when nbr coarser; six for when nbr same; 24 for when nbr finer).  Then...
                            axis * 2 + pole + 6)                                                // ... select which of 36 to use: 6=W, 7=E, 8=S, 9=N, 10=D, 11=U; nbr is at same refinement level.
                           ^ 1;                                                                 // BUT FLIP POLE: my W comms with nbrs E; etc.
                        gasket__ocrGuidFromIndex(&dispIn, meta->labeledGuidRangeForHaloExchange, index, __FILE__, __func__, __LINE__, nanny, "dispIn[...]");
                        gasket__ocrEventCreate(  &dispIn, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS,         __FILE__, __func__, __LINE__, nanny, "dispIn[...]");
                     }
                  }
                  EVT_DEPENDENCE(dispIn, meta->blockClone_Edt, blockClone_Deps_t, aux_Dep[auxDepIdx], DB_MODE_RO,   nanny, "dispIn[...]")
                  auxDepIdx++;
               }
            }

            // Step 5: Clone my continuation.  Then receive refinement disposition messages from my (presently) coarser and same-level neighbors.  Record their dispositions.  Also destroy the datablocks and the
            //         events that brought this information to us, and set up the next At-Bat event.

            SUSPEND__RESUME_IN_CLONE_EDT(;)

            auxDepIdx = 0;

            if (myRefinementDisposition == UNREFINE_BLK) {                                                   // If ANY of my same-level neighbors plan to refine, I cannot unrefine!
               for (axis = 0; axis < 3; axis++) {                                                            // For East/West, then South/North, then Down/Up
                  for (pole = 0; pole < 2; pole++) {                                                         // (East, South, or Down), then (West, North, or Up)
                     if (meta->neighborRefinementLevel[axis][pole] == NEIGHBOR_IS_OFF_EDGE_OF_MESH) continue;
                     if (meta->neighborRefinementLevel[axis][pole] > meta->refinementLevel)         continue;// Neighbor is finer.  Already handled earlier.
                     if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel) {
                        RefinementDisposition_t * dispIn = myDeps->aux_Dep[auxDepIdx].ptr;
                        if (dispIn->refinementDisposition == REFINE_BLK) {
                           myRefinementDisposition = LEAVE_BLK_AT_LVL;
                        }
                     }
                     auxDepIdx++;
                  }
               }
            }

            auxDepIdx = 0;

            if (myRefinementDisposition == REFINE_BLK) {                                                     // Consensus was that I am refining.
               for (axis = 0; axis < 3; axis++) {                                                            // For East/West, then South/North, then Down/Up
                  for (pole = 0; pole < 2; pole++) {                                                         // (East, South, or Down), then (West, North, or Up)
#ifdef NANNY_ON_STEROIDS
                     char nanny[200];
                     sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, [auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, auxDepIdx);
#else
#define nanny NULL
#endif
                     if (meta->neighborRefinementLevel[axis][pole] == NEIGHBOR_IS_OFF_EDGE_OF_MESH) continue;
                     if (meta->neighborRefinementLevel[axis][pole] < meta->refinementLevel) {                // Neighbor is coarser.  Receive its disposition datablock.
                        RefinementDisposition_t * dispIn = myDeps->aux_Dep[auxDepIdx].ptr;
                        if (dispIn->refinementDisposition == REFINE_BLK) {                                   // If neighbor is refining...
                           meta->neighborRefinementLevel[axis][pole]++;                                      // ... note her new refinement level.
                           if (!ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][0])) {            // I should have severed the channel.  Crash if not.
                              (*(int *) 123) = 456;
                           }
                           if (!ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {                          // Assert the neighbor has severed her end of the channel.
                              (*(int *) 123) = 456;
                           }
                        } else if (dispIn->refinementDisposition == LEAVE_BLK_AT_LVL) {
                           *((int *) 123) = 456;                                                             // I am finer than my neighor and I am refining, but my coarser neighbor is NOT refining.  Impossible!
                        } else {                                                                             // Nbr is still marked UNREFINE_BLK.  Case is same as LEAVE_BLK_AT_LVL, but bugs diagnosed distinctly.
                           *((int *) 123) = 456;                                                             // I am finer than my neighor and I am refining, but my coarser neighbor is NOT refining.  Impossible!
                        }
                        gasket__ocrEventDestroy(&dispIn->dbCommHeader.atBat_Event, __FILE__, __func__, __LINE__, nanny, "dispIn[...]"); // Destroy the event that brought us our disp DB.
                        gasket__ocrDbDestroy(&myDeps->aux_Dep[auxDepIdx].guid, &myDeps->aux_Dep[auxDepIdx].ptr, __FILE__, __func__, __LINE__, nanny, "dispIn[...]");
                        auxDepIdx++;
                     } else if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel) {        // Nbr is same level.  Receive its disp db.  Case is same as coarser, but bugs diagnosed distinctly.
                        RefinementDisposition_t * dispIn = myDeps->aux_Dep[auxDepIdx].ptr;
                        if (dispIn->refinementDisposition == REFINE_BLK) {                                   // If neighbor is refining...
                           meta->neighborRefinementLevel[axis][pole]++;                                      // ... note her new refinement level.
                           if (!ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][0])) {            // I should have severed the channel.  Crash if not.
                              (*(int *) 123) = 456;
                           }
                           if (!ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {                          // Assert the neighbor has severed her end of the channel.
                              (*(int *) 123) = 456;
                           }
                        } else if (dispIn->refinementDisposition == LEAVE_BLK_AT_LVL) {
                           if (!ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][0])) {            // I should have severed the channel.  Crash if not.
                              (*(int *) 123) = 456;
                           }
                           if (ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {                           // Assert the neighbor has maintained her end of the channel.
                              (*(int *) 123) = 456;
                           }
                           meta->onDeckToReceiveFace_Event[axis][pole][0] = NULL_GUID;                       // Severe connection from neighbor.  She will see my decision to REFINE, and abandon the event.
                        } else {                                                                             // Nbr is still marked UNREFINE_BLK.  Case is same as LEAVE_BLK_AT_LVL, but bugs diagnosed distinctly.
                           if (!ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][0])) {            // I should have severed the channel.  Crash if not.
                              (*(int *) 123) = 456;
                           }
                           if (ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {                           // Assert the neighbor has maintained her end of the channel.
                              (*(int *) 123) = 456;
                           }
                           meta->onDeckToReceiveFace_Event[axis][pole][0] = NULL_GUID;                       // Severe connection from neighbor.  She will see my decision to REFINE, and abandon the event.
                        }
                        gasket__ocrEventDestroy(&dispIn->dbCommHeader.atBat_Event, __FILE__, __func__, __LINE__, nanny, "dispIn[...]"); // Destroy the event that brought us our disp DB.
                        gasket__ocrDbDestroy(&myDeps->aux_Dep[auxDepIdx].guid, &myDeps->aux_Dep[auxDepIdx].ptr, __FILE__, __func__, __LINE__, nanny, "dispIn[...]");
                        auxDepIdx++;
                     }
                  }
               }
            } else if (myRefinementDisposition == LEAVE_BLK_AT_LVL) {                                        // Consensus was that I am staying at the same level.
               for (axis = 0; axis < 3; axis++) {                                                            // For East/West, then South/North, then Down/Up
                  for (pole = 0; pole < 2; pole++) {                                                         // (East, South, or Down), then (West, North, or Up)
#ifdef NANNY_ON_STEROIDS
                     char nanny[200];
                     sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, [auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, auxDepIdx);
#else
#define nanny NULL
#endif
                     if (meta->neighborRefinementLevel[axis][pole] == NEIGHBOR_IS_OFF_EDGE_OF_MESH) continue;
                     if (meta->neighborRefinementLevel[axis][pole] < meta->refinementLevel) {                // Neighbor is coarser.  Receive its disposition datablock.
                        RefinementDisposition_t * dispIn = myDeps->aux_Dep[auxDepIdx].ptr;
                        if (dispIn->refinementDisposition == REFINE_BLK) {                                   // If neighbor is refining...
                           meta->neighborRefinementLevel[axis][pole]++;                                      // ... note her new refinement level.
                           if (ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][0])) {             // I should have (speculatively) perpetuated the channel.  Crash if not.
                              (*(int *) 123) = 456;
                           }
                           if (!ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {                          // Assert the neighbor has severed her end of the channel.
                              (*(int *) 123) = 456;
                           }
                           gasket__ocrEventAbandon(&meta->conveyFaceToNeighbor_Event[axis][pole][0],         // Abandon the event that I had speculatively created, thus doing my part of severing the channel
                              __FILE__, __func__, __LINE__, nanny, "conveyFaceToNeighbor[...][...][0]");
                        } else if (dispIn->refinementDisposition == LEAVE_BLK_AT_LVL) {
                           if (ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][0])) {             // I should have (speculatively) perpetuated the channel.  Crash if not.
                              (*(int *) 123) = 456;
                           }
                           if (ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {                           // Assert the neighbor has maintained her end of the channel.
                              (*(int *) 123) = 456;
                           }
                           meta->onDeckToReceiveFace_Event[axis][pole][0] = dispIn->dbCommHeader.onDeck_Event;
                        } else {                                                                             // Nbr is still marked UNREFINE_BLK.  Case is same as LEAVE_BLK_AT_LVL, but bugs diagnosed distinctly.
                           if (ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][0])) {             // I should have (speculatively) perpetuated the channel.  Crash if not.
                              (*(int *) 123) = 456;
                           }
                           if (ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {                           // Assert the neighbor has maintained her end of the channel.
                              (*(int *) 123) = 456;
                           }
                           meta->onDeckToReceiveFace_Event[axis][pole][0] = dispIn->dbCommHeader.onDeck_Event;
                        }
                        gasket__ocrEventDestroy(&dispIn->dbCommHeader.atBat_Event, __FILE__, __func__, __LINE__, nanny, "dispIn[...]"); // Destroy the event that brought us our disp DB.
                        gasket__ocrDbDestroy(&myDeps->aux_Dep[auxDepIdx].guid, &myDeps->aux_Dep[auxDepIdx].ptr, __FILE__, __func__, __LINE__, nanny, "dispIn[...]");
                        auxDepIdx++;
                     } else if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel) {        // Nbr is same level.  Receive its disp db.  Case is same as coarser, but bugs diagnosed distinctly.
                        RefinementDisposition_t * dispIn = myDeps->aux_Dep[auxDepIdx].ptr;
                        if (dispIn->refinementDisposition == REFINE_BLK) {                                   // If neighbor is refining...
                           meta->neighborRefinementLevel[axis][pole]++;                                      // ... note her new refinement level.
                           if (ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][0])) {             // I should have (speculatively) perpetuated the channel.  Crash if not.
                              (*(int *) 123) = 456;
                           }
                           if (!ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {                          // Assert the neighbor has severed her end of the channel.
                              (*(int *) 123) = 456;
                           }
                           gasket__ocrEventAbandon(&meta->conveyFaceToNeighbor_Event[axis][pole][0],         // Abandon the event that I had speculatively created, thus doing my part of severing the channel
                              __FILE__, __func__, __LINE__, nanny, "conveyFaceToNeighbor[...][...][0]");
                        } else if (dispIn->refinementDisposition == LEAVE_BLK_AT_LVL) {
                           if (ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][0])) {             // I should have (speculatively) perpetuated the channel.  Crash if not.
                              (*(int *) 123) = 456;
                           }
                           if (ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {                           // Assert the neighbor has maintained her end of the channel.
                              (*(int *) 123) = 456;
                           }
                           meta->onDeckToReceiveFace_Event[axis][pole][0] = dispIn->dbCommHeader.onDeck_Event;
                        } else {                                                                             // Nbr is still marked UNREFINE_BLK.  Case is same as LEAVE_BLK_AT_LVL, but bugs diagnosed distinctly.
                           if (ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][0])) {             // I should have (speculatively) perpetuated the channel.  Crash if not.
                              (*(int *) 123) = 456;
                           }
                           if (ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {                           // Assert the neighbor has maintained her end of the channel.
                              (*(int *) 123) = 456;
                           }
                           meta->onDeckToReceiveFace_Event[axis][pole][0] = dispIn->dbCommHeader.onDeck_Event;
                        }
                        gasket__ocrEventDestroy(&dispIn->dbCommHeader.atBat_Event, __FILE__, __func__, __LINE__, nanny, "dispIn[...]"); // Destroy the event that brought us our disp DB.
                        gasket__ocrDbDestroy(&myDeps->aux_Dep[auxDepIdx].guid, &myDeps->aux_Dep[auxDepIdx].ptr, __FILE__, __func__, __LINE__, nanny, "dispIn[...]");
                        auxDepIdx++;
                     }
                  }
               }
            } else {                                                                                         // Consensus was that I might still be able to unrefine.
               for (axis = 0; axis < 3; axis++) {                                                            // For East/West, then South/North, then Down/Up
                  for (pole = 0; pole < 2; pole++) {                                                         // (East, South, or Down), then (West, North, or Up)
#ifdef NANNY_ON_STEROIDS
                     char nanny[200];
                     sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, [auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, auxDepIdx);
#else
#define nanny NULL
#endif
                     if (meta->neighborRefinementLevel[axis][pole] == NEIGHBOR_IS_OFF_EDGE_OF_MESH) continue;
                     if (meta->neighborRefinementLevel[axis][pole] < meta->refinementLevel) {                // Neighbor is coarser.  Receive its disposition datablock.
                        RefinementDisposition_t * dispIn = myDeps->aux_Dep[auxDepIdx].ptr;
                        if (dispIn->refinementDisposition == REFINE_BLK) {                                   // If neighbor is refining...
                           meta->neighborRefinementLevel[axis][pole]++;                                      // ... note her new refinement level.
                           if (ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][0])) {             // I should have (speculatively) perpetuated the channel.  Crash if not.
                              (*(int *) 123) = 456;
                           }
                           if (!ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {                          // Assert the neighbor has severed her end of the channel.
                              (*(int *) 123) = 456;
                           }
                           gasket__ocrEventAbandon(&meta->conveyFaceToNeighbor_Event[axis][pole][0],         // Abandon the event that I had speculatively created, thus doing my part of severing the channel
                              __FILE__, __func__, __LINE__, nanny, "conveyFaceToNeighbor[...][...][0]");
                        } else if (dispIn->refinementDisposition == LEAVE_BLK_AT_LVL) {
                           if (ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][0])) {             // I should have (speculatively) perpetuated the channel.  Crash if not.
                              (*(int *) 123) = 456;
                           }
                           if (ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {                           // Assert the neighbor has maintained her end of the channel.
                              (*(int *) 123) = 456;
                           }
                           meta->onDeckToReceiveFace_Event[axis][pole][0] = dispIn->dbCommHeader.onDeck_Event;
                        } else {                                                                             // Nbr is still marked UNREFINE_BLK. Case is same as LEAVE_BLK_AT_LVL, but bugs diagnosed distinctly.
                           if (ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][0])) {             // I should have (speculatively) perpetuated the channel.  Crash if not.
                              (*(int *) 123) = 456;
                           }
                           if (ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {                           // Assert the neighbor has maintained her end of the channel.
                              (*(int *) 123) = 456;
                           }
                           meta->onDeckToReceiveFace_Event[axis][pole][0] = dispIn->dbCommHeader.onDeck_Event;
                        }
                        gasket__ocrEventDestroy(&dispIn->dbCommHeader.atBat_Event, __FILE__, __func__, __LINE__, nanny, "dispIn[...]"); // Destroy the event that brought us our disp DB.
                        gasket__ocrDbDestroy(&myDeps->aux_Dep[auxDepIdx].guid, &myDeps->aux_Dep[auxDepIdx].ptr, __FILE__, __func__, __LINE__, nanny, "dispIn[...]");
                        auxDepIdx++;
                     } else if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel) {        // Nbr is same level.  Receive its disp db.  Case is same as coarser, but bugs diagnosed distinctly.
                        RefinementDisposition_t * dispIn = myDeps->aux_Dep[auxDepIdx].ptr;
                        if (dispIn->refinementDisposition == REFINE_BLK) {                                   // If neighbor is refining...
                           meta->neighborRefinementLevel[axis][pole]++;                                      // ... note her new refinement level.
                           (*(int *) 123) = 456;                                                             // I can't be marked UNREFINE_BLK if my same-level neighbor is marked REFINE_BLK!
                        } else if (dispIn->refinementDisposition == LEAVE_BLK_AT_LVL) {
                           if (ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][0])) {             // I should have (speculatively) perpetuated the channel.  Crash if not.
                              (*(int *) 123) = 456;
                           }
                           if (ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {                           // Assert the neighbor has maintained her end of the channel.
                              (*(int *) 123) = 456;
                           }
                           meta->onDeckToReceiveFace_Event[axis][pole][0] = dispIn->dbCommHeader.onDeck_Event;
                        } else {                                                                             // Nbr is still marked UNREFINE_BLK. Case is same as LEAVE_BLK_AT_LVL, but bugs diagnosed distinctly.
                           if (ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][0])) {             // I should have (speculatively) perpetuated the channel.  Crash if not.
                              (*(int *) 123) = 456;
                           }
                           if (ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {                           // Assert the neighbor has maintained her end of the channel.
                              (*(int *) 123) = 456;
                           }
                           meta->onDeckToReceiveFace_Event[axis][pole][0] = dispIn->dbCommHeader.onDeck_Event;
                        }
                        gasket__ocrEventDestroy(&dispIn->dbCommHeader.atBat_Event, __FILE__, __func__, __LINE__, nanny, "dispIn[...]"); // Destroy the event that brought us our disp DB.
                        gasket__ocrDbDestroy(&myDeps->aux_Dep[auxDepIdx].guid, &myDeps->aux_Dep[auxDepIdx].ptr, __FILE__, __func__, __LINE__, nanny, "dispIn[...]");
                        auxDepIdx++;
                     }
                  }
               }
            }
         }
      }


      // Step 6: Blocks that are marked REFINE_BLK now do so, forking to eight children.  They all book-keep as to their new neighbor relationships.

      if (myRefinementDisposition == REFINE_BLK) {   // Step 3:  I'm marked to refine.  Time to do so.
 //printf ("**************************** REFINE called, Refining pos=%d,%d%d to level %d at timestep %d.\n", meta->xPos, meta->yPos, meta->zPos, meta->refinementLevel+1, ts); fflush(stdout);

         meta->cloningState.cloningOpcode = Fork;
         SUSPEND__RESUME_IN_CLONE_EDT(;)

         adjustBlockAndMetaRefineOperation(depv, myParams);

         if (meta->refinementLevel == control->num_refine) {
            myRefinementDisposition = LEAVE_BLK_AT_LVL;
         }
      }
   }


//  Understanding unrefinement decisions:
//
//  After refining, and block that is marked UNREFINE_BLK is a candidate for unrefinement.  Blocks can be unrefined as many levels as works with the block's siblings and neighbors.  In particular,
//  ALL the siblings must agree that unrefinement is possible; and if ANY of the siblings have a non-sibling neighbor that is finer and will NOT unrefine, then that block (and consequently the entire
//  sibling set) cannot unrefine.  Here is how we go about it:
//
//  First of all, understand that myRefinementDisposition might be marked REFINE_BLK, LEAVE_BLK_AT_LVL, or UNREFINE_BLK.  However, from this point on, all refining is done, so all that matters is whether the
//  disposition is or is not UNREFINE_BLK.  We will use a #define to purposely sabotage the other two statuses, so that the compiler would diagnose any errant attempt to use anything but UNREFINE_BLK and
//  ! UNREFINE_BLK.  To set a block away from UNREFINE_BLK, we will set it to DO_NOT_UNREFINE_BLK.

#define DO_NOT_UNREFINE_BLK (UNREFINE_BLK ^ 0x55)                 // Just make DO_NOT_UNREFINE_BLK different than UNREFINE_BLK
#define REFINE_BLK          REFINE_BLK__purposely_sabotaged
#define LEAVE_BLK_AT_LVL    LEAV_BLK_AT_LVL__purposely_sabotaged

//  do {
//
//        Step 1: Look at the neighborRefinementLevel of ALL of my cardinal neighbors.  If ANY of them are finer than me, clone a copy of myself to hear from all such finer cardinal neighbors, to update
//                whether or not they unrefine.  (Note that I only need to hear from the qrtr[0][0] neighbor, since the other quarter-faces in the same direction would give redundant results.)  If not all
//                of them do, mark myself DO_NOT_UNREFINE_BLK.  At the same time, update my record of their refinement levels, and sever my side of the communication channel with any of them that did unrefine.
//                EXCEPTION:  If the finerNeighborNotUnrefiningAndNotTalkingAnyMore flag for the cardinal neighbor is set (in my Step 5 of the previous iteration), do NOT expect to hear from the neighbor.
//                I already know that she is will NOT unrefine, and since she is finer than me, neither can I.
//
//                If my refinement level is 0, i.e. at the coarsest possible level, break out of this loop, because I am done refining, and need not send or receive any other communications with neighbors.
//
//        Step 2: Send my disposition up to my parent.  If my disposition is still UNREFINE_BLK, also send him an event by which he will communicate to me whether my siblings concur with that disposition.
//
//        Step 3: If my disposition is still UNREFINE_BLK, receive a reply from my parent indicating whether my siblings concurred or not, setting my disposition accordingly.
//                (Note:  in the parent code that provides this reply to me, if the consensus reached is DO_NOT_UNREFINE_BLK, he must also send a message to HIS parent, communicating this consensus.
//                The exception is where that parent (which is my grand-parent) would be the root;  there is no reason to propogate this message all the way to the root.)
//
//        Step 4: Send my disposition to all of my cardinal non-sibling neighbors (so that they can update their knowledge of my level, and if it is changing due to unrefining, they can do their part to
//                sever their channels to/from me.  Details:
//                * If my cardinal non-sibling neighbor is finer   than me, then we do NOT need to talk to her, because she already knows that we simply cannot possibly unrefine.
//                * If my cardinal non-sibling neighbor is coarser than me, then I ONLY send a communication to her if I am her qrtr[0][0] neighbor (because all the other quarter faces would just be
//                  sending redundant information).  (She will receive that information in her step 1.)  However, regardless of which quarter I am, if my disposition is to unrefine, then I need to sever
//                  my side of communications with my coarser neighbor.
//                * If my cardinal non-sibling neighbor is the same level as me, then communications I send to her will be handled by her in her Step 5, while mutual information about her disposition will
//                  be received by me in my Step 5.
//
//        Step 5: For any of my non-sibling neighbors that are at the same level, if either she or I are marked UNREFINE_BLK, I need to sever my end of the communication channels to/from her; and ...
//                *  If she is marked UNREFINE_BLK, I need to decrement my record of her neighborRefinementLevel.
//                *  Contrastively, if she is marked DO_NOT_UNREFINE_BLK then I need to set my finerNeighborNotUnrefiningAndNotTalkingAnyMore flag, so that I know not to query her again when I wrap around
//                   to Step 1 of my next iteration.
//
//        Step 6: If I am marked UNREFINE_BLK, perform the Join operation.  My siblings, parent, and I will become one (coarser) block.
//
//  } while (myRefinementDisposition == UNREFINE_BLK);
//
//        Step 7: If I am not at the coarsest level, tell my parent that my disposition is NOT to unrefine any more.
//

   meta->finerNeighborNotUnrefiningAndNotTalkingAnyMore = 0;

   do {

//        Step 1: Look at the neighborRefinementLevel of ALL of my cardinal neighbors.  If ANY of them are finer than me, clone a copy of myself to hear from all such finer cardinal neighbors, to update
//                whether or not they unrefine.  (Note that I only need to hear from the qrtr[0][0] neighbor, since the other quarter-faces in the same direction would give redundant results.)  If not all
//                of them do, mark myself DO_NOT_UNREFINE_BLK.  At the same time, update my record of their refinement levels, and sever my side of the communication channel with any of them that did unrefine.
//                EXCEPTION:  If the FinerNeighborNotUnrefiningAndNotTalkingAnyMore flag for the cardinal neighbor is set (in my Step 5 of the previous iteration), do NOT expect to hear from the neighbor.
//                I already know that she is will NOT unrefine, and since she is finer than me, neither can I.

      dumpComms(depv, 1, ts);

      int auxDepIdx = 0;
      int bit = 1;
      for (axis = 0; axis < 3; axis++) {                                                  // For East/West, then South/North, then Down/Up
         for (pole = 0; pole < 2; pole++) {                                               // (East, South, or Down), then (West, North, or Up)
            if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel+1 &&   // Nbr is finer.  She needs to tell us if she unrefines.  ONLY need info from qrtr[0][0]; others implicitly the same.
                (meta->finerNeighborNotUnrefiningAndNotTalkingAnyMore & bit) == 0) {      // Exception: don't expect to hear from a cardinal neighbor who has set this flag.
               auxDepIdx++;
            }
            bit <<= 1;
         }
      }

      if (meta->finerNeighborNotUnrefiningAndNotTalkingAnyMore) {    // If any of my cardinal neighbors are finer and are NOT talking to me any more (because they already have), I cannot unrefine (any more)
         myRefinementDisposition = DO_NOT_UNREFINE_BLK;
      }


      if (auxDepIdx != 0) {  // If ANY of my cardinal non-sibling neighbors are finer than me, I need to hear from them now (to learn if they decided to unrefine, or not)!
#ifdef NANNY_ON_STEROIDS
         char nanny[NANNYLEN];
         sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d numFinerNeighbors=%d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, auxDepIdx);
#else
#define nanny NULL
#endif
//001 *((int *) 123) = 456; // COVERAGE!
         gasket__ocrEdtCreate(&meta->blockClone_Edt,                     // Guid of the EDT created to continue at function blockContinuaiton_Func.
                              SLOT(blockClone_Deps_t, whoAmI_Dep),
                              myParams->template.blockClone_Template,    // Template for the EDT we are creating.
                              EDT_PARAM_DEF,
                              (u64 *) myParams,
                              countof_blockClone_fixedDeps_t + auxDepIdx,
                              NULL,
                              EDT_PROP_NONE,
                              NULL_HINT,
                              NULL,
                              __FILE__,
                              __func__,
                              __LINE__,
                              nanny,
                              "blockClone (Continuation -- receive unrefinement disposition from finer neighbors.");

         // Set up to hear from finer neighbors (qrtr[0][0] only) what their unrefinement dispositions are:
         auxDepIdx = 0;
         meta->cloningState.cloningOpcode = Special;
         bit = 1;
         for (axis = 0; axis < 3; axis++) {                                                  // For East/West, then South/North, then Down/Up
            for (pole = 0; pole < 2; pole++) {                                               // (East, South, or Down), then (West, North, or Up)
               if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel+1 &&   // Nbr is finer.  She needs to tell us if she unrefines.  ONLY need info from qrtr[0][0]; others implicitly the same.
                   (meta->finerNeighborNotUnrefiningAndNotTalkingAnyMore & bit) == 0) {      // Exception: don't expect to hear from a cardinal neighbor who has set this flag.
//003 *((int *) 123) = 456; // COVERAGE!
               // Nbr is finer.  She needs to tell us if she unrefines.  ONLY need info from qrtr[0][0]; others implicitly the same.
#ifdef NANNY_ON_STEROIDS
                  char nanny[NANNYLEN];
                  sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, qrtr00, nei=finer, [auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, auxDepIdx);
#else
#define nanny NULL
#endif
                  ocrGuid_t dispIn = meta->onDeckToReceiveFace_Event[axis][pole][0];         // Assure our clone gets the info from the finer nbr.
                  meta->onDeckToReceiveFace_Event[axis][pole][0] = NULL_GUID;                // Assure we don't reuse a stale guid later.
                  if (ocrGuidIsNull(dispIn)) {                                               // First time.  Use labeled guid.
                     int nbr_xPos = meta->xPos << 1;                                         // For starters, our own position at finer level is just our position scaled up one level.
                     int nbr_yPos = meta->yPos << 1;
                     int nbr_zPos = meta->zPos << 1;
                     if (axis == 0) {
                        nbr_xPos += (pole == 0) ? -1 : 2;
                     } else if (axis == 1) {
                        nbr_yPos += (pole == 0) ? -1 : 2;
                     } else {
                        nbr_zPos += (pole == 0) ? -1 : 2;
                     }
                     unsigned long index =                                                   // The first component gets us the index of the base set of 36 applicable for the level: i.e.
                        (((((0111111111111111111111L >> (60L - (meta->refinementLevel*3))) * // lvl 0: 0;  lvl 1: 1;  lvl 2: 9 (i.e. 1+8);  lvl 2: 73 (i.e. 1+8+8*8); lvl 3: 585 (i.e. 1+8+8*8+8*8*8) ...
                            meta->numBaseResBlocks) +                                        // ... scaled up by number of blocks in the totally-unrefined mesh.
                           (((((nbr_xPos) * (control->npy << (meta->refinementLevel+1))) +   // Here we subscript-linearize the x, y, z position of the block within its refinement level and add it
                              (nbr_yPos)) * (control->npz << (meta->refinementLevel+1))) +   // to the above base. ...
                            (nbr_zPos))) *                                                   // ...
                          36) +                                                              // ... we multiply by 36 (6 for when nbr coarser; six for when nbr same; 24 for when nbr finer).  Then...
                         axis * 2 + pole)                                                    // ... select which of 36 to use: 0=W, 1=E, 2=S, 3=N, 4=D, 5=U; nbr's nbr is coarser
                        ^ 1;                                                                 // BUT FLIP POLE: my W comms with nbrs E; etc.
                     gasket__ocrGuidFromIndex(&dispIn, meta->labeledGuidRangeForHaloExchange, index, __FILE__, __func__, __LINE__, nanny, "dispIn[...]");
                     gasket__ocrEventCreate  (&dispIn, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS,         __FILE__, __func__, __LINE__, nanny, "dispIn[...]");
                  }
                  EVT_DEPENDENCE(dispIn, meta->blockClone_Edt, blockClone_Deps_t, aux_Dep[auxDepIdx], DB_MODE_RW, nanny, "dispIn[...]");
                  auxDepIdx++;
               } else {
//002 *((int *) 123) = 456; // COVERAGE!
               }
               bit <<= 1;
            }
         }

         // Clone, to hear from ALL finer cardinal neighbors.

         SUSPEND__RESUME_IN_CLONE_EDT(;)

         // Take stock of their replies, telling me which of my finer cardinal neighbors went through with unrefining (and so are now at my refinement level).

         auxDepIdx = 0;
         bit = 1;
         for (axis = 0; axis < 3; axis++) {                                                  // For East/West, then South/North, then Down/Up
            for (pole = 0; pole < 2; pole++) {                                               // (East, South, or Down), then (West, North, or Up)
               if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel+1 &&   // Nbr is finer.  She needs to tell us if she unrefines.  ONLY need info from qrtr[0][0]; others implicitly the same.
                   (meta->finerNeighborNotUnrefiningAndNotTalkingAnyMore & bit) == 0) {      // Exception: don't expect to hear from a cardinal neighbor who has set this flag.
//020 *((int *) 123) = 456; // COVERAGE!
#ifdef NANNY_ON_STEROIDS
                  char nanny[NANNYLEN];
                  sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, qrtr00, nei=finer, [auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, auxDepIdx);
#else
#define nanny NULL
#endif
                  RefinementDisposition_t * dispIn = myDeps->aux_Dep[auxDepIdx].ptr;
                  if (dispIn->refinementDisposition == UNREFINE_BLK) {                       // Neighbor (who is finer) plans to unrefine.
//030 *((int *) 123) = 456; // COVERAGE!
                     meta->neighborRefinementLevel[axis][pole] = meta->refinementLevel;      // Neighbor will become the same level as me.
                     if (!ocrGuidIsNull(meta->onDeckToReceiveFace_Event[axis][pole][0])) {   // This one should have been set null, above.  Comm channels for the other three quarter-faces MIGHT still exist.
                        *((int *) 123) = 456;
                     }
                     meta->onDeckToReceiveFace_Event[axis][pole][1] = NULL_GUID;             // Sever communications from quarter face, if exists.
                     meta->onDeckToReceiveFace_Event[axis][pole][2] = NULL_GUID;             // Sever communications from quarter face, if exists.
                     meta->onDeckToReceiveFace_Event[axis][pole][3] = NULL_GUID;             // Sever communications from quarter face, if exists.

                     if (!ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {                // Assert the neighbor has severed her end of the channel.
                        (*(int *) 123) = 456;
                     }

                     if (!ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][0])) {  // Sever communications to quarter face, if exists.
                           gasket__ocrEventAbandon(&meta->conveyFaceToNeighbor_Event[axis][pole][0],
                              __FILE__, __func__, __LINE__, nanny, "conveyFaceToNeighbor[...][...][0]");
                     }
                     if (!ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][1])) {  // Sever communications to quarter face, if exists.
                           gasket__ocrEventAbandon(&meta->conveyFaceToNeighbor_Event[axis][pole][1],
                              __FILE__, __func__, __LINE__, nanny, "conveyFaceToNeighbor[...][...][1]");
                     }
                     if (!ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][2])) {  // Sever communications to quarter face, if exists.
                           gasket__ocrEventAbandon(&meta->conveyFaceToNeighbor_Event[axis][pole][2],
                              __FILE__, __func__, __LINE__, nanny, "conveyFaceToNeighbor[...][...][2]");
                     }
                     if (!ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][3])) {  // Sever communications to quarter face, if exists.
                           gasket__ocrEventAbandon(&meta->conveyFaceToNeighbor_Event[axis][pole][3],
                              __FILE__, __func__, __LINE__, nanny, "conveyFaceToNeighbor[...][...][3]");
                     }

                  } else {                                                                   // Finer neighbor won't unrefine, so neither will I.
//021 *((int *) 123) = 456; // COVERAGE!
                     myRefinementDisposition = DO_NOT_UNREFINE_BLK;
                     if (!ocrGuidIsNull(meta->onDeckToReceiveFace_Event[axis][pole][0])) {   // This one should have been set null, above.
                        *((int *) 123) = 456;
                     }
                     if (ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {                 // Assert the neighbor has maintained her end of the channel.
                        (*(int *) 123) = 456;
                     }
                     meta->onDeckToReceiveFace_Event[axis][pole][0] = dispIn->dbCommHeader.onDeck_Event;
                  }
                  gasket__ocrEventDestroy(&dispIn->dbCommHeader.atBat_Event, __FILE__, __func__, __LINE__, nanny, "dispIn[...]"); // Destroy the event that brought us our disp DB.
                  gasket__ocrDbDestroy(&myDeps->aux_Dep[auxDepIdx].guid, &myDeps->aux_Dep[auxDepIdx].ptr, __FILE__, __func__, __LINE__, nanny, "dispIn[...]");
                  auxDepIdx++;
               } else {
//019 *((int *) 123) = 456; // COVERAGE!
               }
               bit <<= 1;
            }
         }
      } else {
//004 *((int *) 123) = 456; // COVERAGE!
      }

//                If my refinement level is 0, i.e. at the coarsest possible level, break out of this loop, because I am done refining, and need not send or receive any other communications with neighbors.
      if (meta->refinementLevel == 0) {
//023 *((int *) 123) = 456; // COVERAGE!
         break;
      }


//        Step 2: Send my disposition up to my parent.  If my disposition is still UNREFINE_BLK, also send him an event by which he will communicate to me whether my siblings concur with that disposition.

      dumpComms(depv, 2, ts);

#ifdef NANNY_ON_STEROIDS
      char nanny[NANNYLEN];
      sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
      gasket__ocrDbCreate(&myDeps->upboundRequest_Dep.guid, (void **) &myDeps->upboundRequest_Dep.ptr, sizeof_RefinementDisposition_t, __FILE__, __func__, __LINE__, nanny, "dispOut to parent[...]");
      meta->cloningState.cloningOpcode = SeasoningOneOrMoreDbCreates;
      SUSPEND__RESUME_IN_CLONE_EDT(;)
#ifdef NANNY_ON_STEROIDS
      sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif

      // This block's unrefinement disposition will be passed to the parent as operand, requesting the Aggregate Unrefinement Disposition service.

      ocrGuid_t conveyServiceRequestToParent_Event  = meta->conveyServiceRequestToParent_Event;           // Jot down the At Bat Event.
      RefinementDisposition_t * dispOut = myDeps->upboundRequest_Dep.ptr;
      dispOut->refinementDisposition    = myRefinementDisposition;
      if (myRefinementDisposition == UNREFINE_BLK) {                                                      // If my disposition is to unrefine, I need a reply back from the parent.
//024 *((int *) 123) = 456; // COVERAGE!
         gasket__ocrEventCreate(&myDeps->replyFromRequest_Dep.guid, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG, __FILE__, __func__, __LINE__, nanny, "serviceReturn_Event(From parent, re unrefinement consensus)");  // Create the event by which the parent communicates back to me.
         dispOut->dbCommHeader.serviceReturn_Event = myDeps->replyFromRequest_Dep.guid;
      } else {                                                                                            // But if my disposition is already NOT to unrefine, I don't need a reply from the parent.
//005 *((int *) 123) = 456; // COVERAGE!
         myDeps->replyFromRequest_Dep.guid         = NULL_GUID;
         dispOut->dbCommHeader.serviceReturn_Event = NULL_GUID;
      }
      serviceReturnEvent = dispOut->dbCommHeader.serviceReturn_Event;
      dispOut->dbCommHeader.serviceOpcode = Operation_AggregateUnrefinementDisp;
      dispOut->dbCommHeader.squawk        = 0xFF01;
      dispOut->dbCommHeader.atBat_Event   = conveyServiceRequestToParent_Event;                           // Convey At Bat Event to parent so that he can destroy the event.
#ifdef NANNY_ON_STEROIDS
      sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d disposition=%d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, dispOut->refinementDisposition);
#else
#define nanny NULL
#endif
      gasket__ocrEventCreate(&meta->conveyServiceRequestToParent_Event, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG, __FILE__, __func__, __LINE__, nanny, "conveyServiceRequestToParent(On Deck after AggregateUnrefinementDisp)");  // Create the On Deck Event; record it in our meta.
      dispOut->dbCommHeader.onDeck_Event  = meta->conveyServiceRequestToParent_Event;                     // Convey On Deck Event to parent so that he can make his clone depend upon it.
      gasket__ocrDbRelease(myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "upboundRequest(AggregateUnrefinementDisp)");
      gasket__ocrEventSatisfy(conveyServiceRequestToParent_Event, myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "upboundRequest(AggregateUnrefinementDisp) via conveyServiceRequestToParent");  // Satisfy the parent's serviceRequest datablock dependence.
      myDeps->upboundRequest_Dep.guid      = NULL_GUID;


//        Step 3: If my disposition is still UNREFINE_BLK, receive a reply from my parent indicating whether my siblings concurred or not, setting my disposition accordingly.
//                (Note:  in the parent code that provides this reply to me, if the consensus reached is DO_NOT_UNREFINE_BLK, he must also send a message to HIS parent, communicating this consensus.
//                The exception is where that parent (which is my grand-parent) would be the root;  there is no reason to propogate this message all the way to the root.)

      dumpComms(depv, 3, ts);

      if (myRefinementDisposition == UNREFINE_BLK) {                                                      // If my disposition is to unrefine, I need a reply back from the parent.
//025 *((int *) 123) = 456; // COVERAGE!
         meta->cloningState.cloningOpcode = ReceivingACommunication;
         SUSPEND__RESUME_IN_CLONE_EDT(;)

#ifdef NANNY_ON_STEROIDS
      sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
         RefinementDisposition_t * dispConsensusIn = myDeps->replyFromRequest_Dep.ptr;
         myRefinementDisposition = dispConsensusIn->refinementDisposition;
         gasket__ocrEventDestroy(&serviceReturnEvent, __FILE__, __func__, __LINE__, nanny, "serviceReturn(AggregateUnrefDisp)"); // Destroy event that brought reply from parent.
         gasket__ocrDbDestroy(&myDeps->replyFromRequest_Dep.guid, &myDeps->replyFromRequest_Dep.ptr, __FILE__, __func__, __LINE__, nanny, "replyFromRequest(AggregateUnrefinementDisp"); // Destroy DB.
      } else {
//006 *((int *) 123) = 456; // COVERAGE!
      }

//        Step 4: Send my disposition to all of my cardinal non-sibling neighbors (so that they can update their knowledge of my level, and if it is changing due to unrefining, they can do their part to
//                sever their channels to/from me.  Details:
//                * If my cardinal non-sibling neighbor is finer   than me, then we do NOT need to talk to her, because she already knows that we simply cannot possibly unrefine.
//                * If my cardinal non-sibling neighbor is coarser than me, then I ONLY send a communication to her if I am her qrtr[0][0] neighbor (because all the other quarter faces would just be
//                  sending redundant information).  (She will receive that information in her step 1.)  However, regardless of which quarter I am, if my disposition is to unrefine, then I need to sever
//                  my side of communications with my coarser neighbor.
//                * If my cardinal non-sibling neighbor is the same level as me, then communications I send to her will be handled by her in her Step 5, while mutual information about her disposition will
//                  be received by me in my Step 5.

      dumpComms(depv, 4, ts);
      int axis;
      auxDepIdx = 0;
      ocrGuid_t dbDispOut[3];
      for (axis = 0; axis < 3; axis++) {
         int pole = ((axis == 0) ? (meta->xPos & 1) : ((axis == 1) ? (meta->yPos & 1) : (meta->zPos & 1)));
         if (meta->neighborRefinementLevel[axis][pole] == (meta->refinementLevel+1)) {         // Non-sibling neighbor is finer.  No communication necessary.
//046 *((int *) 123) = 456; // COVERAGE!
         } else if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel) {      // Non-sibling neighbor is at same level.  Send her my disposition.
//032 *((int *) 123) = 456; // COVERAGE!
#ifdef NANNY_ON_STEROIDS
            char nanny[200];
            sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, nei=same, [auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum,  axis, pole, auxDepIdx);
#else
#define nanny NULL
#endif
            void * dummy;
            gasket__ocrDbCreate(&dbDispOut[auxDepIdx], (void **) &dummy, sizeof_RefinementDisposition_t, __FILE__, __func__, __LINE__, nanny, "dbDispOut[...]");
            auxDepIdx++;
         } else if (meta->neighborRefinementLevel[axis][pole] == (meta->refinementLevel-1)) {  // Non-sibling neighbor is coarser.
//007 *((int *) 123) = 456; // COVERAGE!
            int qrtrLR = ((axis == 0) ? (meta->yPos & 1) : ((axis == 1) ? (meta->xPos & 1) : (meta->xPos & 1)));
            int qrtrUD = ((axis == 0) ? (meta->zPos & 1) : ((axis == 1) ? (meta->zPos & 1) : (meta->yPos & 1)));
            if (qrtrLR == 0 && qrtrUD == 0) {
//011 *((int *) 123) = 456; // COVERAGE!
#ifdef NANNY_ON_STEROIDS
               char nanny[200];
               sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, nei=coarser, [auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum,  axis, pole, auxDepIdx);
#else
#define nanny NULL
#endif
               void * dummy;
               gasket__ocrDbCreate(&dbDispOut[auxDepIdx], (void **) &dummy, sizeof_RefinementDisposition_t, __FILE__, __func__, __LINE__, nanny, "dbDispOut[...]");
               auxDepIdx++;
            } else {
//010 *((int *) 123) = 456; // COVERAGE!
               if (!ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][(qrtrLR<<1)+qrtrUD])) {
#ifdef NANNY_ON_STEROIDS
                  char nanny[200];
                  sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, qrtrLR=%d, qrtrUD=%d, nei=coarser", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum,  axis, pole, qrtrLR, qrtrUD);
#else
#define nanny NULL
#endif
                  gasket__ocrEventFlush(&meta->conveyFaceToNeighbor_Event[axis][pole][(qrtrLR<<1)+qrtrUD],  __FILE__, __func__, __LINE__, nanny, "conveyDispToNeighbor_Event[...][...][...]"); // (Other side of comm abandons the event)); // Abandon the event that I had speculatively created, thus doing my part of severing the channel
               }

               if (!ocrGuidIsNull(meta->onDeckToReceiveFace_Event[axis][pole][(qrtrLR<<1)+qrtrUD])) {
*((int *) 123) = 456; // COVERAGE!
*((int *) 123) = 456; // Careful!!! see comment on next line.
                  meta->onDeckToReceiveFace_Event[axis][pole][(qrtrLR<<1)+qrtrUD] = NULL_GUID;                    // If this ever happens, we should figure out if it needs to be abandoned versus flushed!
               } else {
//013 *((int *) 123) = 456; // COVERAGE!
               }
            }
         }
      }

      if (auxDepIdx != 0) {         // If any of my non-sibling neighbors need to hear from me, clone to season the datablocks.

//014 *((int *) 123) = 456; // COVERAGE!
         gasket__ocrEdtCreate(&meta->blockClone_Edt,                     // Guid of the EDT created to continue at function blockContinuaiton_Func.
                              SLOT(blockClone_Deps_t, whoAmI_Dep),
                              myParams->template.blockClone_Template,    // Template for the EDT we are creating.
                              EDT_PARAM_DEF,
                              (u64 *) myParams,
                              countof_blockClone_fixedDeps_t + auxDepIdx,
                              NULL,
                              EDT_PROP_NONE,
                              NULL_HINT,
                              NULL,
                              __FILE__,
                              __func__,
                              __LINE__,
                              nanny,
                              "blockClone (Continuation -- obtain space to write unrefinement disposition to send to non-sibling neighbors that are coarser or same level as me)");
         do {
#ifdef NANNY_ON_STEROIDS
            char nanny[200];
            sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, nei=coarser, [auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, auxDepIdx);
#else
#define nanny NULL
#endif
            auxDepIdx--;
            ADD_DEPENDENCE(dbDispOut[auxDepIdx], meta->blockClone_Edt, blockClone_Deps_t, aux_Dep[auxDepIdx], DB_MODE_RW, nanny, "dbDispOut[...]");
         } while (auxDepIdx > 0);

         meta->cloningState.cloningOpcode = SeasoningOneOrMoreDbCreates;
         SUSPEND__RESUME_IN_CLONE_EDT(;)

         auxDepIdx = 0;
         for (axis = 0; axis < 3; axis++) {
            int pole = ((axis == 0) ? (meta->xPos & 1) : ((axis == 1) ? (meta->yPos & 1) : (meta->zPos & 1)));
            if (meta->neighborRefinementLevel[axis][pole] == (meta->refinementLevel+1)) {         // Non-sibling neighbor is finer.  No communication necessary.
//057 *((int *) 123) = 456; // COVERAGE!
            } else if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel) {      // Non-sibling neighbor is at same level.  Send her my disposition.
//033 *((int *) 123) = 456; // COVERAGE!
#ifdef NANNY_ON_STEROIDS
               char nanny[200];
               sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, nei=same, [auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum,  axis, pole, auxDepIdx);
#else
#define nanny NULL
#endif
               RefinementDisposition_t * dispOut = myDeps->aux_Dep[auxDepIdx].ptr;
               dispOut->refinementDisposition = myRefinementDisposition;
               ocrGuid_t conveyDispToNeighbor_Event = meta->conveyFaceToNeighbor_Event[axis][pole][0];
               if (ocrGuidIsNull(conveyDispToNeighbor_Event)) {                             // First time.  Use labeled guid.
//034 *((int *) 123) = 456; // COVERAGE!
                  unsigned long index =                                                     // The first component gets us the index of the base set of 36 applicable for the level: i.e.
                     ((((0111111111111111111111L >> (63L - (meta->refinementLevel*3))) *    // lvl 0: 0;  lvl 1: 1;  lvl 2: 9 (i.e. 1+8);  lvl 2: 73 (i.e. 1+8+8*8); lvl 3: 585 (i.e. 1+8+8*8+8*8*8) ...
                        meta->numBaseResBlocks) +                                           // ... scaled up by number of blocks in the totally-unrefined mesh.
                       ((((meta->xPos * (control->npy << meta->refinementLevel)) +          // Here we subscript-linearize the x, y, z position of the block within its refinement level and add it
                          meta->yPos) * (control->npz << meta->refinementLevel)) +          // to the above base. ...
                        meta->zPos))  *
                      36) +                                                                 // Then we scale all that by 36 (6 for when nbr coarser; six for when nbr same; 24 for when nbr finer).
                     axis * 2 + pole +                                                      // Now select which of the 36 comm events to use: for nbr coarser, 0=W, 1=E, 2=S, 3=N,  4=D,  5=U, ...
                     6;                                                                     // ... but for nbr at same refinement level:                       6=W, 7=E, 8=S, 9=N, 10=D, 11=U.
                  gasket__ocrGuidFromIndex(&conveyDispToNeighbor_Event, meta->labeledGuidRangeForHaloExchange, index, __FILE__, __func__, __LINE__, nanny, "conveyDispToNeighbor");
                  gasket__ocrEventCreate(&conveyDispToNeighbor_Event, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS, __FILE__, __func__, __LINE__, nanny, "conveyDispToNeighbor_Event[...][...][0]");  // Create the On Deck Event; record it in our meta.
               } else {
//047 *((int *) 123) = 456; // COVERAGE!
               }

               dispOut->dbCommHeader.atBat_Event = conveyDispToNeighbor_Event;              // Convey At Bat Event to neighbor so that she can destroy the event.
               if (myRefinementDisposition == UNREFINE_BLK) {                               // If I am unrefining, sever further communications to neighbor
//048 *((int *) 123) = 456; // COVERAGE!
                  meta->conveyFaceToNeighbor_Event[axis][pole][0] = NULL_GUID;
               } else {                                                                     // But if I am NOT refining, then (speculatively) renew the event by which I send stuff to that nbr.
//035 *((int *) 123) = 456; // COVERAGE!
                  gasket__ocrEventCreate(&meta->conveyFaceToNeighbor_Event[axis][pole][0], OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG, __FILE__, __func__, __LINE__, nanny, "conveyDispToNeighbor_Event[...][...][0]");  // Create the On Deck Event; record it in our meta.
               }

               dispOut->dbCommHeader.onDeck_Event = meta->conveyFaceToNeighbor_Event[axis][pole][0];  // Convey On Deck Event to neighbor so that she can make her clone depend upon it.
               gasket__ocrDbRelease(                               myDeps->aux_Dep[auxDepIdx].guid, __FILE__, __func__, __LINE__, nanny, "dispOut[...]");
               gasket__ocrEventSatisfy(conveyDispToNeighbor_Event, myDeps->aux_Dep[auxDepIdx].guid, __FILE__, __func__, __LINE__, nanny, "dispOut[...] via conveyDispToNeighbor"); // Satisfy the neighbors's dependence for this face.
               auxDepIdx++;
            } else if (meta->neighborRefinementLevel[axis][pole] == (meta->refinementLevel-1)) {  // Non-sibling neighbor is coarser.
               int qrtrLR = ((axis == 0) ? (meta->yPos & 1) : ((axis == 1) ? (meta->xPos & 1) : (meta->xPos & 1)));
               int qrtrUD = ((axis == 0) ? (meta->zPos & 1) : ((axis == 1) ? (meta->zPos & 1) : (meta->yPos & 1)));
               if (qrtrLR == 0 && qrtrUD == 0) {
//015 *((int *) 123) = 456; // COVERAGE!
#ifdef NANNY_ON_STEROIDS
                  char nanny[200];
                  sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, nei=coarser, [auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, auxDepIdx);
#else
#define nanny NULL
#endif
                  RefinementDisposition_t * dispOut = myDeps->aux_Dep[auxDepIdx].ptr;
                  dispOut->refinementDisposition = myRefinementDisposition;
                  ocrGuid_t conveyDispToNeighbor_Event = meta->conveyFaceToNeighbor_Event[axis][pole][0];
                  if (ocrGuidIsNull(conveyDispToNeighbor_Event)) {                             // First time.  Use labeled guid.
//016 *((int *) 123) = 456; // COVERAGE!
                     unsigned long index =                                                     // The first component gets us the index of the base set of 36 applicable for the level: i.e.
                        ((((0111111111111111111111L >> (63L - (meta->refinementLevel*3))) *    // lvl 0: 0;  lvl 1: 1;  lvl 2: 9 (i.e. 1+8);  lvl 2: 73 (i.e. 1+8+8*8); lvl 3: 585 (i.e. 1+8+8*8+8*8*8) ...
                           meta->numBaseResBlocks) +                                           // ... scaled up by number of blocks in the totally-unrefined mesh.
                          ((((meta->xPos * (control->npy << meta->refinementLevel)) +          // Here we subscript-linearize the x, y, z position of the block within its refinement level and add it
                             meta->yPos) * (control->npz << meta->refinementLevel)) +          // to the above base. ...
                           meta->zPos))  *
                         36) +                                                                 // Then we scale all that by 36 (6 for when nbr coarser; six for when nbr same; 24 for when nbr finer).
                        axis * 2 + pole +                                                      // Now select which of the 36 comm events to use: for nbr coarser, 0=W, 1=E, 2=S, 3=N,  4=D,  5=U, ...
                        0;                                                                     // ... but for nbr at same refinement level:                       6=W, 7=E, 8=S, 9=N, 10=D, 11=U.
                  gasket__ocrGuidFromIndex(&conveyDispToNeighbor_Event, meta->labeledGuidRangeForHaloExchange, index, __FILE__, __func__, __LINE__, nanny, "conveyDispToNeighbor");
                  gasket__ocrEventCreate(&conveyDispToNeighbor_Event, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS, __FILE__, __func__, __LINE__, nanny, "conveyDispToNeighbor_Event[...][...][0]");  // Create the On Deck Event; record it in our meta.
                  } else {
//027 *((int *) 123) = 456; // COVERAGE!
                  }

                  dispOut->dbCommHeader.atBat_Event = conveyDispToNeighbor_Event;              // Convey At Bat Event to neighbor so that she can destroy the event.
                  if (myRefinementDisposition == UNREFINE_BLK) {                               // If I am unrefining, sever further communications to neighbor
//029 *((int *) 123) = 456; // COVERAGE!
                     meta->conveyFaceToNeighbor_Event[axis][pole][0] = NULL_GUID;
                  } else {                                                                     // But if I am NOT refining, then (speculatively) renew the event by which I send stuff to that nbr.
//017 *((int *) 123) = 456; // COVERAGE!
                     gasket__ocrEventCreate(&meta->conveyFaceToNeighbor_Event[axis][pole][0], OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG, __FILE__, __func__, __LINE__, nanny, "conveyDispToNeighbor_Event[...][...][0]");  // Create the On Deck Event; record it in our meta.
                  }
                  dispOut->dbCommHeader.onDeck_Event = meta->conveyFaceToNeighbor_Event[axis][pole][0];  // Convey On Deck Event to neighbor so that she can make her clone depend upon it.
                  gasket__ocrDbRelease(                               myDeps->aux_Dep[auxDepIdx].guid, __FILE__, __func__, __LINE__, nanny, "dispOut[...]");
                  gasket__ocrEventSatisfy(conveyDispToNeighbor_Event, myDeps->aux_Dep[auxDepIdx].guid, __FILE__, __func__, __LINE__, nanny, "dispOut[...] via conveyDispToNeighbor"); // Satisfy the neighbors's dependence for this face.
                  auxDepIdx++;
               }
            } else {
//018 *((int *) 123) = 456; // COVERAGE!
            }
         }
      } else {
//008 *((int *) 123) = 456; // COVERAGE!
      }

//        Step 5: For any of my non-sibling neighbors that are at the same level, if either she or I are marked UNREFINE_BLK, I need to sever my end of the communication channels to/from her; and ...
//                *  If she is marked UNREFINE_BLK, I need to decrement my record of her neighborRefinementLevel.
//                *  Contrastively, if she is marked DO_NOT_UNREFINE_BLK then I need to set my FinerNeighborNotUnrefiningAndNotTalkingAnyMore flag, so that I know not to query her again when I wrap around
//                   to Step 1 of my next iteration.
//

      dumpComms(depv, 5, ts);
      auxDepIdx = 0;
      for (axis = 0; axis < 3; axis++) {                                                            // For East/West, then South/North, then Down/Up
         pole = (axis == 0) ? (meta->xPos & 1) : (axis == 1) ? (meta->yPos & 1) : (meta->zPos & 1); // (East, South, or Down), then (West, North, or Up)
         if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel) {                  // Nbr is same level.  She needs to tell us if she unrefines.
//036 *((int *) 123) = 456; // COVERAGE!
            auxDepIdx++;
            meta->cloningState.cloningOpcode = Special;
         } else {
//009 *((int *) 123) = 456; // COVERAGE!
         }
      }

      if (auxDepIdx != 0) {
//037 *((int *) 123) = 456; // COVERAGE!

#ifdef NANNY_ON_STEROIDS
                  char nanny[200];
                  sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, nei=same lvl, [auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, auxDepIdx);
#else
#define nanny NULL
#endif
         gasket__ocrEdtCreate(&meta->blockClone_Edt,                     // Guid of the EDT created to continue at function blockContinuaiton_Func.
                              SLOT(blockClone_Deps_t, whoAmI_Dep),
                              myParams->template.blockClone_Template,    // Template for the EDT we are creating.
                              EDT_PARAM_DEF,
                              (u64 *) myParams,
                              countof_blockClone_fixedDeps_t + auxDepIdx,
                              NULL,
                              EDT_PROP_NONE,
                              NULL_HINT,
                              NULL,
                              __FILE__,
                              __func__,
                              __LINE__,
                              nanny,
                              "blockClone (Continuation -- receive unrefinement disposition from neighbors at same level.");
         auxDepIdx = 0;
         for (axis = 0; axis < 3; axis++) {                                                            // For East/West, then South/North, then Down/Up
            pole = (axis == 0) ? (meta->xPos & 1) : (axis == 1) ? (meta->yPos & 1) : (meta->zPos & 1); // (East, South, or Down), then (West, North, or Up)
            if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel) {                  // Nbr is same level.  She needs to tell us if she unrefines.

//039 *((int *) 123) = 456; // COVERAGE!
               // Set up to hear from finer neighbor (qrtr[0][0] only) what her unrefinement disposition is:
#ifdef NANNY_ON_STEROIDS
               char nanny[NANNYLEN];
               sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, qrtr00, nei=same lvl, [auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, auxDepIdx);
#else
#define nanny NULL
#endif
               ocrGuid_t dispIn = meta->onDeckToReceiveFace_Event[axis][pole][0];             // Assure my clone gets the info from the neighbor, who is at the same refinement level as me.
               meta->onDeckToReceiveFace_Event[axis][pole][0] = NULL_GUID;                    // Assure I don't reuse a stale guid later.
               if (ocrGuidIsNull(dispIn)) {                                                   // First time.  Use labeled guid.
//040 *((int *) 123) = 456; // COVERAGE!
                  int nbr_xPos = meta->xPos +                                                 // If NOT doing x-axis disposition exchange, neighbor's xPos is the same as ours...
                     ((axis != 0) ? 0 : (pole == 0) ? -1 : 1);                                // otherwise, it is one off in the direction of the pole being processed.
                  int nbr_yPos = meta->yPos +                                                 // If NOT doing y-axis disposition exchange, neighbor's yPos is the same as ours...
                     ((axis != 1) ? 0 : (pole == 0) ? -1 : 1);                                // otherwise, it is one off in the direction of the pole being processed.
                  int nbr_zPos = meta->zPos +                                                 // If NOT doing z-axisdispositionhalo exchange, neighbor's zPos is the same as ours...
                     ((axis != 2) ? 0 : (pole == 0) ? -1 : 1);                                // otherwise, it is one off in the direction of the pole being processed.
                  unsigned long index =                                                       // The first component gets us the index of the base set of 36 applicable for the level: i.e.
                     (((((0111111111111111111111L >> (63L - (meta->refinementLevel*3))) *     // lvl 0: 0;  lvl 1: 1;  lvl 2: 9 (i.e. 1+8);  lvl 2: 73 (i.e. 1+8+8*8); lvl 3: 585 (i.e. 1+8+8*8+8*8*8) ...
                         meta->numBaseResBlocks) +                                            // ... scaled up by number of blocks in the totally-unrefined mesh.
                        (((((nbr_xPos) * (control->npy << (meta->refinementLevel))) +         // Here we subscript-linearize the x, y, z position of the block within its refinement level and add it
                           (nbr_yPos)) * (control->npz << (meta->refinementLevel))) +         // to the above base. ...
                         (nbr_zPos))) *                                                       // ...
                       36) +                                                                  // ... we multiply by 36 (6 for when nbr coarser; six for when nbr same; 24 for when nbr finer).  Then...
                      axis * 2 + pole + 6)                                                    // ... select which of 36 to use: 6=W, 7=E, 8=S, 9=N, 10=D, 11=U; nbr's nbr is same level
                     ^ 1;                                                                     // BUT FLIP POLE: my W comms with nbrs E; etc.
                  gasket__ocrGuidFromIndex(&dispIn, meta->labeledGuidRangeForHaloExchange, index, __FILE__, __func__, __LINE__, nanny, "dispIn[...]");
                  gasket__ocrEventCreate  (&dispIn, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS,         __FILE__, __func__, __LINE__, nanny, "dispIn[...]");
               } else {
//049 *((int *) 123) = 456; // COVERAGE!
               }
               EVT_DEPENDENCE(dispIn, meta->blockClone_Edt, blockClone_Deps_t, aux_Dep[auxDepIdx], DB_MODE_RW, nanny, "dispIn[...]");
               auxDepIdx++;
            } else {
//038 *((int *) 123) = 456; // COVERAGE!
            }
         }

//041 *((int *) 123) = 456; // COVERAGE!

//        Step 5, continuation:
//        Step 5: For any of my non-sibling neighbors that are at the same level, if either she or I are marked UNREFINE_BLK, I need to sever my end of the communication channels to/from her; and ...
//                *  If she is marked UNREFINE_BLK, I need to decrement my record of her neighborRefinementLevel.
//                *  Contrastively, if she is marked DO_NOT_UNREFINE_BLK then I need to set my FinerNeighborNotUnrefiningAndNotTalkingAnyMore flag, so that I know not to query her again when I wrap around
//                   to Step 1 of my next iteration.
//

         SUSPEND__RESUME_IN_CLONE_EDT(;)

         auxDepIdx = 0;
         for (axis = 0; axis < 3; axis++) {                                                            // For East/West, then South/North, then Down/Up
            pole = (axis == 0) ? (meta->xPos & 1) : (axis == 1) ? (meta->yPos & 1) : (meta->zPos & 1); // (East, South, or Down), then (West, North, or Up)
            if (meta->neighborRefinementLevel[axis][pole] == meta->refinementLevel) {                  // Nbr is same level.  She needs to tell us if she unrefines.
#ifdef NANNY_ON_STEROIDS
               char nanny[NANNYLEN];
               sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, axis=%d, pole=%d, qrtr00, nei=same lvl, [auxDepIdx=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, auxDepIdx);
#else
#define nanny NULL
#endif
               RefinementDisposition_t * dispIn = myDeps->aux_Dep[auxDepIdx].ptr;
               if (dispIn->refinementDisposition == UNREFINE_BLK) {                       // Neighbor (who is same level) plans to unrefine.
//050 *((int *) 123) = 456; // COVERAGE!
                  meta->neighborRefinementLevel[axis][pole]--;                            // Neighbor will become the same level as me (if I am unrefining too) or one level coarser (if I am not).
                  if (!ocrGuidIsNull(meta->onDeckToReceiveFace_Event[axis][pole][0])) {   // This one should have been set null, above.  Comm channels for the other three quarter-faces MIGHT still exist.
                     *((int *) 123) = 456;
                  }

                  if (!ocrGuidIsNull(dispIn->dbCommHeader.onDeck_Event)) {                // Assert the neighbor has severed her end of the channel.
                     (*(int *) 123) = 456;
                  }

                  if (!ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][0])) {  // Sever communications to full face, if exists.
                        gasket__ocrEventAbandon(&meta->conveyFaceToNeighbor_Event[axis][pole][0],
                           __FILE__, __func__, __LINE__, nanny, "conveyFaceToNeighbor[...][...][0]");
                  }
               } else if (myRefinementDisposition == UNREFINE_BLK) {                      // Neighbor won't unrefine, but I plan to, so we need to sever communications anyway.
                  meta->finerNeighborNotUnrefiningAndNotTalkingAnyMore |= (1<<((axis<<1)+pole));// Tell my next iteration of step 1 NOT to expect to hear from this neighbor any more.  She is NOT unrefining!
//058 *((int *) 123) = 456; // COVERAGE!
               } else {                                                                   // Neither I nor my same-level neighbor intend to unrefine.  We can keep the channel open.
//042 *((int *) 123) = 456; // COVERAGE!
                  meta->onDeckToReceiveFace_Event[axis][pole][0] = dispIn->dbCommHeader.onDeck_Event;
               }
               gasket__ocrEventDestroy(&dispIn->dbCommHeader.atBat_Event, __FILE__, __func__, __LINE__, nanny, "dispIn[...]"); // Destroy the event that brought us our disp DB.
               gasket__ocrDbDestroy(&myDeps->aux_Dep[auxDepIdx].guid, &myDeps->aux_Dep[auxDepIdx].ptr, __FILE__, __func__, __LINE__, nanny, "dispIn[...]");
               auxDepIdx++;
            }
         }
      } else {
//012 *((int *) 123) = 456; // COVERAGE!
      }

//        Step 6: If I am marked UNREFINE_BLK, perform the Join operation.  My siblings, parent, and I will become one (coarser) block.

      dumpComms(depv, 6, ts);

      if (myRefinementDisposition != UNREFINE_BLK) continue;

      // Unrefine the block.

#ifdef NANNY_ON_STEROIDS
      sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
      if (!ocrGuidIsNull(       meta->conveyFaceToNeighbor_Event[0][0][0])) {
         gasket__ocrEventFlush(&meta->conveyFaceToNeighbor_Event[0][0][0], __FILE__, __func__, __LINE__, nanny, "conveyDispToNeighbor_Event[0][0][0]"); // (Other side of comm abandons the event)
      }
      if (!ocrGuidIsNull(       meta->conveyFaceToNeighbor_Event[0][1][0])) {
         gasket__ocrEventFlush(&meta->conveyFaceToNeighbor_Event[0][1][0], __FILE__, __func__, __LINE__, nanny, "conveyDispToNeighbor_Event[0][1][0]"); // (Other side of comm abandons the event)
      }
      if (!ocrGuidIsNull(       meta->conveyFaceToNeighbor_Event[1][0][0])) {
         gasket__ocrEventFlush(&meta->conveyFaceToNeighbor_Event[1][0][0], __FILE__, __func__, __LINE__, nanny, "conveyDispToNeighbor_Event[1][0][0]"); // (Other side of comm abandons the event)
      }
      if (!ocrGuidIsNull(       meta->conveyFaceToNeighbor_Event[1][1][0])) {
         gasket__ocrEventFlush(&meta->conveyFaceToNeighbor_Event[1][1][0], __FILE__, __func__, __LINE__, nanny, "conveyDispToNeighbor_Event[1][1][0]"); // (Other side of comm abandons the event)
      }
      if (!ocrGuidIsNull(       meta->conveyFaceToNeighbor_Event[2][0][0])) {
         gasket__ocrEventFlush(&meta->conveyFaceToNeighbor_Event[2][0][0], __FILE__, __func__, __LINE__, nanny, "conveyDispToNeighbor_Event[2][0][0]"); // (Other side of comm abandons the event)
      }
      if (!ocrGuidIsNull(       meta->conveyFaceToNeighbor_Event[2][1][0])) {
         gasket__ocrEventFlush(&meta->conveyFaceToNeighbor_Event[2][1][0], __FILE__, __func__, __LINE__, nanny, "conveyDispToNeighbor_Event[2][1][0]"); // (Other side of comm abandons the event)
      }
      if (!ocrGuidIsNull(       meta->onDeckToReceiveFace_Event[0][0][0])) {
         gasket__ocrEventFlush(&meta->onDeckToReceiveFace_Event[0][0][0], __FILE__, __func__, __LINE__, nanny, "onDeckToReceiveFace_Event[0][0][0]"); // (Other side of comm abandons the event)
      }
      if (!ocrGuidIsNull(       meta->onDeckToReceiveFace_Event[0][1][0])) {
         gasket__ocrEventFlush(&meta->onDeckToReceiveFace_Event[0][1][0], __FILE__, __func__, __LINE__, nanny, "onDeckToReceiveFace_Event[0][1][0]"); // (Other side of comm abandons the event)
      }
      if (!ocrGuidIsNull(       meta->onDeckToReceiveFace_Event[1][0][0])) {
         gasket__ocrEventFlush(&meta->onDeckToReceiveFace_Event[1][0][0], __FILE__, __func__, __LINE__, nanny, "onDeckToReceiveFace_Event[1][0][0]"); // (Other side of comm abandons the event)
      }
      if (!ocrGuidIsNull(       meta->onDeckToReceiveFace_Event[1][1][0])) {
         gasket__ocrEventFlush(&meta->onDeckToReceiveFace_Event[1][1][0], __FILE__, __func__, __LINE__, nanny, "onDeckToReceiveFace_Event[1][1][0]"); // (Other side of comm abandons the event)
      }
      if (!ocrGuidIsNull(       meta->onDeckToReceiveFace_Event[2][0][0])) {
         gasket__ocrEventFlush(&meta->onDeckToReceiveFace_Event[2][0][0], __FILE__, __func__, __LINE__, nanny, "onDeckToReceiveFace_Event[2][0][0]"); // (Other side of comm abandons the event)
      }
      if (!ocrGuidIsNull(       meta->onDeckToReceiveFace_Event[2][1][0])) {
         gasket__ocrEventFlush(&meta->onDeckToReceiveFace_Event[2][1][0], __FILE__, __func__, __LINE__, nanny, "onDeckToReceiveFace_Event[2][1][0]"); // (Other side of comm abandons the event)
      }
//028 *((int *) 123) = 456; // COVERAGE!

//printf ("**************************** REFINE called, Unrefining pos=%d,%d,%d from level %d to level %d at timestep %d.\n", meta->xPos, meta->yPos, meta->zPos, meta->refinementLevel, meta->refinementLevel-1, ts); fflush(stdout);
      dumpComms(depv, 7, ts);

      // Make a commHeader by which a Join service request can be delivered to the parent.

      gasket__ocrDbCreate(&myDeps->upboundRequest_Dep.guid, (void **) &myDeps->upboundRequest_Dep.ptr, sizeof_DbCommHeader_t, __FILE__, __func__, __LINE__, nanny, "upboundRequest");
      meta->cloningState.cloningOpcode = SeasoningOneOrMoreDbCreates;
      SUSPEND__RESUME_IN_CLONE_EDT(;)

      // Now do the join-clone operation.
      meta->cloningState.cloningOpcode = Join;
      SUSPEND__RESUME_IN_CLONE_EDT(;)

      // Only the 000 prong cloned to this join; the other seven prongs fed their blocks to the appropriate dependences of the join-fork and just died.

//031 *((int *) 123) = 456; // COVERAGE!

      // Now downsample the eight input blocks to get the aggregated block.

      Block_t           * joinBlock000 = myDeps->aux_Dep[0].ptr;
      Block_t           * joinBlock001 = myDeps->aux_Dep[1].ptr;
      Block_t           * joinBlock010 = myDeps->aux_Dep[2].ptr;
      Block_t           * joinBlock011 = myDeps->aux_Dep[3].ptr;
      Block_t           * joinBlock100 = myDeps->aux_Dep[4].ptr;
      Block_t           * joinBlock101 = myDeps->aux_Dep[5].ptr;
      Block_t           * joinBlock110 = myDeps->aux_Dep[6].ptr;
      Block_t           * joinBlock111 = myDeps->aux_Dep[7].ptr;
      Block_t           * block        = myDeps->block_Dep.ptr;
      double (* pCells)         /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2] =
              (double(*)        /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2]) (block->cells);        // Ptr to new, finer or coarser blk.
      double (* p8thCells)      /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2];
      double (* pCells_Join000) /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2] =
              (double(*)        /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2]) (joinBlock000->cells); // Ptr to block[0][0][0] contrib to new coarser blk.
      double (* pCells_Join001) /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2] =
              (double(*)        /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2]) (joinBlock001->cells); // Ptr to block[0][0][1] contrib to new coarser blk.
      double (* pCells_Join010) /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2] =
              (double(*)        /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2]) (joinBlock010->cells); // Ptr to block[0][1][0] contrib to new coarser blk.
      double (* pCells_Join011) /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2] =
              (double(*)        /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2]) (joinBlock011->cells); // Ptr to block[0][1][1] contrib to new coarser blk.
      double (* pCells_Join100) /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2] =
              (double(*)        /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2]) (joinBlock100->cells); // Ptr to block[1][0][0] contrib to new coarser blk.
      double (* pCells_Join101) /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2] =
              (double(*)        /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2]) (joinBlock101->cells); // Ptr to block[1][0][1] contrib to new coarser blk.
      double (* pCells_Join110) /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2] =
              (double(*)        /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2]) (joinBlock110->cells); // Ptr to block[1][1][0] contrib to new coarser blk.
      double (* pCells_Join111) /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2] =
              (double(*)        /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2]) (joinBlock111->cells); // Ptr to block[1][1][1] contrib to new coarser blk.
      void * pCells_Join[2][2][2] = {{{pCells_Join000, pCells_Join001}, {pCells_Join010, pCells_Join011}},{{pCells_Join100, pCells_Join101}, {pCells_Join110, pCells_Join111}}};

      int i, j, k, xStart, yStart, zStart, var, x, y, z;
      for (      i = 0, xStart = 0; i < 2; i++, xStart = (control->x_block_size >> 1)) {
         for (   j = 0, yStart = 0; j < 2; j++, yStart = (control->y_block_size >> 1)) {
            for (k = 0, zStart = 0; k < 2; k++, zStart = (control->z_block_size >> 1)) {
               p8thCells = pCells_Join[i][j][k];
               for (var = 0; var < control->num_vars; var++) {
                  for (      x = 1; x <= control->x_block_size >> 1; x++) {
                     for (   y = 1; y <= control->y_block_size >> 1; y++) {
                        for (z = 1; z <= control->z_block_size >> 1; z++) {
                           pCells[var][xStart+x][yStart+y][zStart+z] =
                              p8thCells[var][x+x-1][y+y-1][z+z-1] +
                              p8thCells[var][x+x-1][y+y-1][z+z  ] +
                              p8thCells[var][x+x-1][y+y  ][z+z-1] +
                              p8thCells[var][x+x-1][y+y  ][z+z  ] +
                              p8thCells[var][x+x  ][y+y-1][z+z-1] +
                              p8thCells[var][x+x  ][y+y-1][z+z  ] +
                              p8thCells[var][x+x  ][y+y  ][z+z-1] +
                              p8thCells[var][x+x  ][y+y  ][z+z  ];
                        }
                     }
                  }
               }
            }
         }
      }

      // Destroy the input blocks.

#ifdef NANNY_ON_STEROIDS
      sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
      gasket__ocrDbDestroy(&myDeps->aux_Dep[0].guid,       &myDeps->aux_Dep[0].ptr,       __FILE__, __func__, __LINE__, nanny, "joinBlock[0][0][0]");
      gasket__ocrDbDestroy(&myDeps->aux_Dep[1].guid,       &myDeps->aux_Dep[1].ptr,       __FILE__, __func__, __LINE__, nanny, "joinBlock[0][0][1]");
      gasket__ocrDbDestroy(&myDeps->aux_Dep[2].guid,       &myDeps->aux_Dep[2].ptr,       __FILE__, __func__, __LINE__, nanny, "joinBlock[0][1][0]");
      gasket__ocrDbDestroy(&myDeps->aux_Dep[3].guid,       &myDeps->aux_Dep[3].ptr,       __FILE__, __func__, __LINE__, nanny, "joinBlock[0][1][1]");
      gasket__ocrDbDestroy(&myDeps->aux_Dep[4].guid,       &myDeps->aux_Dep[4].ptr,       __FILE__, __func__, __LINE__, nanny, "joinBlock[1][0][0]");
      gasket__ocrDbDestroy(&myDeps->aux_Dep[5].guid,       &myDeps->aux_Dep[5].ptr,       __FILE__, __func__, __LINE__, nanny, "joinBlock[1][0][1]");
      gasket__ocrDbDestroy(&myDeps->aux_Dep[6].guid,       &myDeps->aux_Dep[6].ptr,       __FILE__, __func__, __LINE__, nanny, "joinBlock[1][1][0]");
      gasket__ocrDbDestroy(&myDeps->aux_Dep[7].guid,       &myDeps->aux_Dep[7].ptr,       __FILE__, __func__, __LINE__, nanny, "joinBlock[1][1][1]");

      dumpComms(depv, 8, ts);

      // Destroy the event that brought us the parentMeta

      gasket__ocrEventDestroy(&myParams->parentMetaToJoinClone_Event, __FILE__, __func__, __LINE__, nanny, "parentMetaToJoinClone");

   } while (myRefinementDisposition == UNREFINE_BLK);

//        Step 7: If I am not at the coarsest level, tell my parent that my disposition is NOT to unrefine any more.

   if (meta->refinementLevel != 0) {
#ifdef NANNY_ON_STEROIDS
      char nanny[NANNYLEN];
      sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
      gasket__ocrDbCreate(&myDeps->upboundRequest_Dep.guid, (void **) &myDeps->upboundRequest_Dep.ptr, sizeof_RefinementDisposition_t, __FILE__, __func__, __LINE__, nanny, "dispOut to parent[...]");
      meta->cloningState.cloningOpcode = SeasoningOneOrMoreDbCreates;
      SUSPEND__RESUME_IN_CLONE_EDT(;)
#ifdef NANNY_ON_STEROIDS
      sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif

      // This block's unrefinement disposition (which is DO_NOT_UNREFINE_BLK) will be passed to the parent as operand, requesting the Aggregate Unrefinement Disposition service.

      ocrGuid_t conveyServiceRequestToParent_Event  = meta->conveyServiceRequestToParent_Event;           // Jot down the At Bat Event.
      RefinementDisposition_t * dispOut = myDeps->upboundRequest_Dep.ptr;
      dispOut->refinementDisposition    = DO_NOT_UNREFINE_BLK;
      myDeps->replyFromRequest_Dep.guid         = NULL_GUID;
      dispOut->dbCommHeader.serviceReturn_Event = NULL_GUID;
      serviceReturnEvent = dispOut->dbCommHeader.serviceReturn_Event;
      dispOut->dbCommHeader.serviceOpcode = Operation_AggregateUnrefinementDisp;
      dispOut->dbCommHeader.squawk        = 0xFF02;
      dispOut->dbCommHeader.atBat_Event   = conveyServiceRequestToParent_Event;                           // Convey At Bat Event to parent so that he can destroy the event.
#ifdef NANNY_ON_STEROIDS
      sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d disposition=%d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, dispOut->refinementDisposition);
#else
#define nanny NULL
#endif
      gasket__ocrEventCreate(&meta->conveyServiceRequestToParent_Event, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG, __FILE__, __func__, __LINE__, nanny, "conveyServiceRequestToParent(On Deck after AggregateUnrefinementDisp)");  // Create the On Deck Event; record it in our meta.
      dispOut->dbCommHeader.onDeck_Event  = meta->conveyServiceRequestToParent_Event;                     // Convey On Deck Event to parent so that he can make his clone depend upon it.
      gasket__ocrDbRelease(myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "upboundRequest(AggregateUnrefinementDisp)");
      gasket__ocrEventSatisfy(conveyServiceRequestToParent_Event, myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "upboundRequest(AggregateUnrefinementDisp) via conveyServiceRequestToParent");  // Satisfy the parent's serviceRequest datablock dependence.
      myDeps->upboundRequest_Dep.guid      = NULL_GUID;
   }

   dumpComms(depv, 9, ts);

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  faceCount
#undef  num_refine_step
#undef  finerNeighborIsRefining
#undef  myRefinementDisposition
#undef  serviceReturnEvent
} // refine

void adjustBlockAndMetaRefineOperation(ocrEdtDep_t depv[], blockClone_Params_t * myParams) {

   blockClone_Deps_t * myDeps       = (blockClone_Deps_t *) depv;
   BlockMeta_t       * meta         = myDeps->meta_Dep.ptr;
   Control_t         * control      = myDeps->control_Dep.ptr;

   Block_t           * block        = myDeps->block_Dep.ptr;
   Block_t           * blockPred    = myDeps->blockPred_Dep.ptr;
   double (* pCells)         /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2] =
           (double(*)        /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2]) (block->cells);         // Ptr to new, finer or coarser blk.
   double (* p8thCells)      /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2] =
           (double(*)        /*[control->num_vars]*/ [control->x_block_size+2] [control->y_block_size+2] [control->z_block_size+2]) (blockPred->cells);     // For fork: Ptr to 8th blk [0][0][0].  Adjust below.

   // We are now one of eight children of the block that just forked.  We need to adjust our meta information, and write the upsampled eighth block of our parent into our own block.

   int poleEW = ((myParams->prongNum & 1)     );
   int poleSN = ((myParams->prongNum & 2) >> 1);
   int poleDU = ((myParams->prongNum    ) >> 2);

   meta->refinementLevel++;
   meta->xPos                          = (meta->xPos << 1) + poleEW;
   meta->yPos                          = (meta->yPos << 1) + poleSN;
   meta->zPos                          = (meta->zPos << 1) + poleDU;
   meta->cen[0]                        = meta->cen[0] + (((poleEW << 1) - 1) * control->p2[control->num_refine - meta->refinementLevel - 1]);
   meta->cen[1]                        = meta->cen[1] + (((poleSN << 1) - 1) * control->p2[control->num_refine - meta->refinementLevel - 1]);
   meta->cen[2]                        = meta->cen[2] + (((poleDU << 1) - 1) * control->p2[control->num_refine - meta->refinementLevel - 1]);

   // Fix neighbor refinement level.
   // For neighbors that are sibling prongs, they need to be reset to the current refinement level of this entire sibling set.

   meta->neighborRefinementLevel[0][1-poleEW] = meta->refinementLevel;        // East's  neighbor to the West,  and West's  neighbor to the East  is a sibling, and so is at the same refinement level as me.
   meta->neighborRefinementLevel[1][1-poleSN] = meta->refinementLevel;        // South's neighbor to the North, and North's neighbor to the South is a sibling, and so is at the same refinement level as me.
   meta->neighborRefinementLevel[2][1-poleDU] = meta->refinementLevel;        // Down's  neighbor Above,        and Up's    neighbor Below        is a sibling, and so is at the same refinement level as me.

   // For neighbors of this prong that are NOT siblings, the neighbor's refinement level is inherited from the metadata of the predecessor of the fork.  However, in the case where that neighbor was finer
   // than our predecessor, it may now be the SAME refinement level as us, or one finer, and that can be different for each prong.  We have to adjust appropriately, using an indicative bit vector provided
   // by the predecessor in the high order four bits of the neighborRefinementLevel.

   if (meta->neighborRefinementLevel[0][poleEW] != NEIGHBOR_IS_OFF_EDGE_OF_MESH) {
      int adjustment = (meta->neighborRefinementLevel[0][poleEW] >> (4 + (poleSN*2) + poleDU)) & 1;
      meta->neighborRefinementLevel[0][poleEW] &= 0xF;
      meta->neighborRefinementLevel[0][poleEW] += adjustment;
   }
   if (meta->neighborRefinementLevel[1][poleSN] != NEIGHBOR_IS_OFF_EDGE_OF_MESH) {
      int adjustment = (meta->neighborRefinementLevel[1][poleSN] >> (4 + (poleEW*2) + poleDU)) & 1;
      meta->neighborRefinementLevel[1][poleSN] &= 0xF;
      meta->neighborRefinementLevel[1][poleSN] += adjustment;
   }
   if (meta->neighborRefinementLevel[2][poleDU] != NEIGHBOR_IS_OFF_EDGE_OF_MESH) {
      int adjustment = (meta->neighborRefinementLevel[2][poleDU] >> (4 + (poleEW*2) + poleSN)) & 1;
      meta->neighborRefinementLevel[2][poleDU] &= 0xF;
      meta->neighborRefinementLevel[2][poleDU] += adjustment;
   }
   meta->conveyServiceRequestToParent_Event = myParams->conveyServiceRequestToParent_Event;   // Event to satisfy dependence at parent, by which he is awaiting serviceRequest_dblk from us.

//printf ("%s line %d, lvl=%d, zPos=%d, yPos=%d, xPos=%d, conveyEights: 0x%lx 0x%lx 0x%lx 0x%lx 0x%lx 0x%lx 0x%lx 0x%lx\n", __FILE__, __LINE__, meta->refinementLevel, meta->zPos, meta->yPos, meta->xPos, (unsigned long) meta->conveyEighthBlockToJoin_Event[0], (unsigned long) meta->conveyEighthBlockToJoin_Event[1], (unsigned long) meta->conveyEighthBlockToJoin_Event[2], (unsigned long) meta->conveyEighthBlockToJoin_Event[3], (unsigned long) meta->conveyEighthBlockToJoin_Event[4], (unsigned long) meta->conveyEighthBlockToJoin_Event[5], (unsigned long) meta->conveyEighthBlockToJoin_Event[6], (unsigned long) meta->conveyEighthBlockToJoin_Event[7]); fflush(stdout);
   int i;
   for (i = 0; i < 8; i++) {
      meta->conveyEighthBlockToJoin_Event[i] = myParams->conveyEighthBlockToJoin_Event[i];
   }
   myParams->prongNum     = -1;                                                               // Reset:  Indicate NOT continuing into the prong of a fork (which happens only when we refine)
   myParams->conveyServiceRequestToParent_Event = NULL_GUID;                                  // Reset:  This field is not applicable now.  (It is only applicable when we are doing the Fork)

   // Now upsample the appropriate one eighth of the input block to a full block.

   int xStart = ((control->x_block_size) >> 1) * (meta->xPos & 1);
   int yStart = ((control->y_block_size) >> 1) * (meta->yPos & 1);
   int zStart = ((control->z_block_size) >> 1) * (meta->zPos & 1);

   int var, x, y, z;
   for (var = 0; var < control->num_vars; var++) {
      for (x = 0; x <= control->x_block_size+1; x++) {
         for (y = 0; y <= control->y_block_size+1; y++) {
            for (z = 0; z <= control->z_block_size+1; z++) {
               pCells[var][x][y][z] = -8888.00;
            }
         }
      }
      for (x = 1; x <= control->x_block_size >> 1; x++) {
         for (y = 1; y <= control->y_block_size >> 1; y++) {
            for (z = 1; z <= control->z_block_size >> 1; z++) {
               double val = p8thCells[var][xStart+x][yStart+y][zStart+z] * 0.125;
               pCells[var][x+x-1][y+y-1][z+z-1] = val;
               pCells[var][x+x-1][y+y-1][z+z  ] = val;
               pCells[var][x+x-1][y+y  ][z+z-1] = val;
               pCells[var][x+x-1][y+y  ][z+z  ] = val;
               pCells[var][x+x  ][y+y-1][z+z-1] = val;
               pCells[var][x+x  ][y+y-1][z+z  ] = val;
               pCells[var][x+x  ][y+y  ][z+z-1] = val;
               pCells[var][x+x  ][y+y  ][z+z  ] = val;
            }
         }
      }
   }

   // We are done with the predecessor's coarser block.  We cannot destroy it, because there are seven siblings who are asyncrhonously using it to upsample their workload.  Instead, we leave it lying
   // around until it is convenient to destroy it, which happens when the parent receives its first service request.

   myDeps->blockPred_Dep.guid = NULL_GUID;
   myDeps->blockPred_Dep.ptr  = NULL;

} // adjustBlockAndMetaRefineOperation

#if 0
===


                     myDeps->neiRefDisp_Dep[axis][pole][qrtrLeftRight][qrtrUpDown].ptr  = NULL;
                     myDeps->neiRefDisp_Dep[axis][pole][qrtrLeftRight][qrtrUpDown].guid = meta->onDeckToReceiveFace_Event[axis][pole][idep];
                     if (ocrGuidIsNull(myDeps->faceIn_Dep[qrtrHaloLeftRight][qrtrHaloUpDown][pole].guid)) {  // First time.
                        (*(int *) 123) = 456;  // Throw seg-fault.  This should NOT be the first time we've talked with that neighbor!  We already did some stencil halo exchanges with her.
                     }
                     idep++;
                  }
               }
            } else if (meta->neighborRefinementLevel[axis][pole] < meta->refinementLevel) {          // While doing Step 1, also allocate and season datablocks for telling our coarser neighbors our disposition
               meta->cloningState.cloningOpcode = Special;
#ifdef NANNY_ON_STEROIDS
              char nanny[200];
              sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, index is [axis=%d][pole=%d], nei=coarser, axis=%c", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, (axis==0)?(pole==0?'W':'E'):((axis==1)?(pole==0?'S':'N'):(pole==0?'D':'U')));
#else
#define nanny NULL
#endif
               gasket__ocrDbCreate(&myDeps->neiRefDispXDep[axis][pole][0][0].guid, (void **) &myDeps->neiRefDispXDep[axis][pole][0][0].ptr, sizeof_RefinementDisposition_t, __FILE__, __func__, __LINE__, nanny, "neiRefDisp[...][...][0][0]");
               meta->readWriteDetailForRefinementComm |= bit;                             // Indicate that this dependence is for a datablock where the continuation clone needs RW access.
            }
            bit <<= 4;
         }
      }



      for (j = control->num_refine; j >= 0; j--) {
         if (blockCount->blkCnt[j]) {
            cur_max_level = j;
            break;
         }
      }
====
      reset_all(glbl);
      if (uniform_refine) {
         for (in = 0; in < sorted_index[num_refine+1]; in++) {
            lcl->n = sorted_list[in].n;
            bp = &blocks[lcl->n];
            if (bp->number >= 0)
               if (bp->level < num_refine)
                  bp->refine = 1;
               else
                  bp->refine = 0;
         }
      } else {
         t2 = timer();
         check_objects(glbl);
         timer_refine_co += timer() - t2;
         t4 += timer() - t2;
      }

      t2 = timer();
      CALL_SUSPENDABLE_CALLEE(1)
      comm_refine(glbl);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      CALL_SUSPENDABLE_CALLEE(1)
      comm_parent(glbl);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      CALL_SUSPENDABLE_CALLEE(1)
      comm_parent_reverse(glbl);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      timer_refine_c1 += timer() - t2;
      t4 += timer() - t2;

      t2 = timer();
      CALL_SUSPENDABLE_CALLEE(1)
      num_split = refine_level(glbl);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      t5 = timer();
      timer_refine_mr += t5 - t2;
      t4 += t5 - t2;

      t2 = timer();
      sum_b = num_active + 7*num_split + 1;
      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Allreduce(glbl, &sum_b, &max_b, 1, MPI_INTEGER, MPI_MAX, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      sum_b = num_parents + num_split;
      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Allreduce(glbl, &sum_b, &min_b, 1, MPI_INTEGER, MPI_MAX, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      if (max_b > ((int) (0.75*((double) max_num_blocks))) ||
          min_b >= (max_num_parents-1)) {
         CALL_SUSPENDABLE_CALLEE(1)
         redistribute_blocks(glbl, &tp1, &tm1, &tu1, &t3, &nm_r, num_split);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
         timer_rs_ca += t3;
         nrrs++;
      }
      t5 = timer();
      timer_rs_all += t5 - t2;
      t4 += t5 - t2;

      t2 = timer();
      CALL_SUSPENDABLE_CALLEE(1)
      split_blocks(glbl);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      t5 = timer();
      timer_refine_sb += t5 - t2;
      t4 += t5 - t2;

      t2 = timer();
      reset_neighbors(glbl);
      CALL_SUSPENDABLE_CALLEE(1)
      comm_parent(glbl);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      CALL_SUSPENDABLE_CALLEE(1)
      comm_parent_reverse(glbl);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      CALL_SUSPENDABLE_CALLEE(1)
      comm_refine(glbl);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      timer_refine_c2 += timer() - t2;
      t4 += timer() - t2;

      t2 = timer();
      CALL_SUSPENDABLE_CALLEE(1)
      redistribute_blocks(glbl, &tp, &tm, &tu, &t3, &nm_c, 0);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      t3 = timer() - t3;
      CALL_SUSPENDABLE_CALLEE(1)
      consolidate_blocks(glbl);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      t5 = timer();
      timer_cb_cb += t5 - t3;
      timer_cb_all += t5 - t2;
      t4 += t5 - t2;
      CALL_SUSPENDABLE_CALLEE(1)
      check_buff_size(glbl);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)

      t2 = timer();
      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Allreduce(glbl, local_num_blocks, num_blocks, (num_refine+1), MPI_INTEGER,
                    MPI_SUM, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      timer_refine_sy += timer() - t2;
      t4 += timer() - t2;
      if (lb_opt == 2) {
         t2 = timer();
         CALL_SUSPENDABLE_CALLEE(0)
         gasket__mpi_Allreduce(glbl, &num_active, &min_b, 1, MPI_INTEGER, MPI_MIN,
                       MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
         CALL_SUSPENDABLE_CALLEE(0)
         gasket__mpi_Allreduce(glbl, &num_active, &max_b, 1, MPI_INTEGER, MPI_MAX,
                       MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
         CALL_SUSPENDABLE_CALLEE(0)
         gasket__mpi_Allreduce(glbl, &num_active, &sum_b, 1, MPI_INTEGER, MPI_SUM,
                       MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
         if (max_b > global_max_b) global_max_b = max_b;
         t4 += timer() - t2;
         ratio = ((double) (max_b - min_b)*num_pes)/((double) sum_b);
         if (!uniform_refine && max_b > (min_b + 1) &&
             ratio > ((double) inbalance/100.0)) {
            nlbs++;
            t2 = timer();
            CALL_SUSPENDABLE_CALLEE(0)
            load_balance(glbl);
            DEBRIEF_SUSPENDABLE_FUNCTION(;)
            t5 = timer();
            timer_lb_all += t5 - t2;
            t4 += t5 - t2;

            t2 = timer();
            CALL_SUSPENDABLE_CALLEE(0)
            gasket__mpi_Allreduce(glbl, local_num_blocks, num_blocks, (num_refine+1),
                          MPI_INTEGER, MPI_SUM, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
            DEBRIEF_SUSPENDABLE_FUNCTION(;)
            timer_refine_sy += timer() - t2;
            t4 += timer() - t2;
         }
      }
   }
   timer_rs_pa += tp1;
   timer_rs_mv += tm1;
   timer_rs_un += tu1;
   timer_cb_pa += tp;
   timer_cb_mv += tm;
   timer_cb_un += tu;

   if (target_active || target_max || target_min) {
      if (!my_pe) {
         for (j = 0; j <= num_refine; j++)
            printf("Number of blocks at level %d before timestep %d is %d\n",
                   j, ts, num_blocks[j]);
         printf("\n");
      }
      t2 = timer();
      global_active = num_blocks[0];
      for (i = 1; i <= num_refine; i++)
         global_active += num_blocks[i];
      // Will not be able to get to target in all cases, but can get to
      // a range of target +- 3 since can add or subtract in units of
      // 7 blocks.
      if ((target_active && global_active > num_pes*target_active + 3) ||
          (target_max && global_active > num_pes*target_max))
         nm_t += reduce_blocks(glbl);
      else if ((target_active && global_active < num_pes*target_active - 3) ||
               (target_min && global_active < num_pes*target_min))
         add_blocks(glbl);
      CALL_SUSPENDABLE_CALLEE(0)
      check_buff_size(glbl);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      t5 = timer();
      timer_target_all += t5 - t2;
      t4 += t5 - t2;
   }
   t2 = timer();
   CALL_SUSPENDABLE_CALLEE(1)
   gasket__mpi_Allreduce(glbl, &num_active, &min_b, 1, MPI_INTEGER, MPI_MIN, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
   DEBRIEF_SUSPENDABLE_FUNCTION(;)
   CALL_SUSPENDABLE_CALLEE(1)
   gasket__mpi_Allreduce(glbl, &num_active, &max_b, 1, MPI_INTEGER, MPI_MAX, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
   DEBRIEF_SUSPENDABLE_FUNCTION(;)
   CALL_SUSPENDABLE_CALLEE(1)
   gasket__mpi_Allreduce(glbl, &num_active, &sum_b, 1, MPI_INTEGER, MPI_SUM, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
   DEBRIEF_SUSPENDABLE_FUNCTION(;)
   i = nm_r + nm_c + nm_t;
   CALL_SUSPENDABLE_CALLEE(1)
   gasket__mpi_Allreduce(glbl, &i, &num_split, 1, MPI_INTEGER, MPI_SUM, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
   DEBRIEF_SUSPENDABLE_FUNCTION(;)
   if (max_b > global_max_b) global_max_b = max_b;
   for (j = 0; j <= num_refine; j++) {
      if (!j)
         global_active = num_blocks[0];
      else
         global_active += num_blocks[j];
      if (!my_pe && report_perf & 8)
         printf("Number of blocks at level %d at timestep %d is %d\n",
                j, ts, num_blocks[j]);
   }
   if (!my_pe && report_perf & 8) printf("\n");
   timer_refine_sy += timer() - t2;
   t4 += timer() - t2;

   if (lb_opt) {
      ratio = ((double) (max_b - min_b)*num_pes)/((double) sum_b);
      if (!uniform_refine &&
          (max_b > (min_b + 1) && ratio > ((double) inbalance/100.0))) {
         nlbs++;
         t2 = timer();
         CALL_SUSPENDABLE_CALLEE(1)
         load_balance(glbl);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
         t5 = timer();
         timer_lb_all += t5 - t2;
         t4 += t5 - t2;

         t2 = timer();
         CALL_SUSPENDABLE_CALLEE(1)
         gasket__mpi_Allreduce(glbl, local_num_blocks, num_blocks, (num_refine+1),
                       MPI_INTEGER, MPI_SUM, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
         timer_refine_sy += timer() - t2;
         t4 += timer() - t2;
      }
   }
   num_moved_rs += nm_r;
   num_moved_coarsen += nm_c;
   num_moved_reduce += nm_t;
   CALL_SUSPENDABLE_CALLEE(1)
   check_buff_size(glbl);
   DEBRIEF_SUSPENDABLE_FUNCTION(;)
   t5 = timer();
   timer_refine_cc += t5 - t1 - t4;

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE

#undef i
#undef j
//#undef n
#undef in
#undef min_b
#undef max_b
#undef sum_b
#undef num_refine_step
#undef num_split
#undef nm_r
#undef nm_c
#undef nm_t
#undef ratio
#undef tp
#undef tm
#undef tu
#undef tp1
#undef tm1
#undef tu1
#undef t1
#undef t2
#undef t3
#undef t4
#undef t5
#undef bp

} // refine

int refine_level(Globals_t * const glbl)
{
   OBTAIN_ACCESS_TO_blocks
   OBTAIN_ACCESS_TO_parents
   OBTAIN_ACCESS_TO_sorted_list

   typedef struct {
      Frame_Header_t myFrame;
      int level, nei, n, i, j, b, c, c1, change, lchange, unrefine, sib, p, in;
      struct {
         Block_t *bp, *bp1;
         Parent_t *pp;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__refine_level_t;

//#define level             (lcl->level) // Overloaded symbol.  Use fully-qualified name at point of reference
//#define nei               (lcl->nei)   // Overloaded symbol.  Use fully-qualified name at point of reference
//#define n                 (lcl->n)     // Overloaded symbol.  Use fully-qualified name at point of reference
#define i                 (lcl->i)
#define j                 (lcl->j)
#define b                 (lcl->b)
#define c                 (lcl->c)
#define c1                (lcl->c1)
#define change            (lcl->change)
#define lchange           (lcl->lchange)
#define unrefine          (lcl->unrefine)
#define sib               (lcl->sip)
#define p                 (lcl->p)
#define in                (lcl->in)
#define bp                (lcl->pointers.bp)
#define bp1               (lcl->pointers.bp1)
#define pp                (lcl->pointers.pp)

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__refine_level_t)
   TRACE

   /* block states:
    * 1 block should be refined
    * -1 block could be unrefined
    * 0 block at level 0 and can not be unrefined or
    *         at max level and can not be refined
    */

// get list of neighbor blocks (indirect links in from blocks)

   for (lcl->level = cur_max_level; lcl->level >= 0; lcl->level--) {
      /* check for blocks at this level that will refine
         their neighbors at this level can not unrefine
         their neighbors at a lower level must refine
      */
      do {
         lchange = 0;
         for (in = sorted_index[lcl->level]; in < sorted_index[lcl->level+1]; in++) {
            lcl->n = sorted_list[in].n;
            bp = &blocks[lcl->n];
            if (bp->number >= 0 && bp->level == lcl->level) {
               if (bp->refine == 1) {
                  if (bp->parent != -1 && bp->parent_node == my_pe) {
                     pp = &parents[bp->parent];
                     if (pp->refine == -1)
                        pp->refine = 0;
                     for (b = 0; b < 8; b++)
                        if (pp->child_node[b] == my_pe && pp->child[b] >= 0)
                           if (blocks[pp->child[b]].refine == -1) {
                              blocks[pp->child[b]].refine = 0;
                              lchange++;
                           }
                  }
                  for (i = 0; i < 6; i++)
                     /* neighbors in level above taken care of already */
                     /* neighbors in this level can not unrefine */
                     if (bp->nei_level[i] == lcl->level)
                        if ((lcl->nei = bp->nei[i][0][0]) >= 0) { /* on core */
                           if (blocks[lcl->nei].refine == -1) {
                              blocks[lcl->nei].refine = 0;
                              lchange++;
                              if ((p = blocks[lcl->nei].parent) != -1 &&
                                    blocks[lcl->nei].parent_node == my_pe) {
                                 if ((pp = &parents[p])->refine == -1)
                                    pp->refine = 0;
                                 for (b = 0; b < 8; b++)
                                    if (pp->child_node[b] == my_pe &&
                                        pp->child[b] >= 0)
                                       if (blocks[pp->child[b]].refine == -1) {
                                          blocks[pp->child[b]].refine = 0;
                                          lchange++;
                                       }
                              }
                           }
                        } else { /* off core */
                           if (bp->nei_refine[i] == -1) {
                              bp->nei_refine[i] = 0;
                              lchange++;
                           }
                        }
                     /* neighbors in level below must refine */
                     else if (bp->nei_level[i] == lcl->level-1)
                        if ((lcl->nei = bp->nei[i][0][0]) >= 0) {
                           if (blocks[lcl->nei].refine != 1) {
                              blocks[lcl->nei].refine = 1;
                              lchange++;
                           }
                        } else
                           if (bp->nei_refine[i] != 1) {
                              bp->nei_refine[i] = 1;
                              lchange++;
                           }
               } else if (bp->refine == -1) {
                  // check if block can be unrefined
                  for (i = 0; i < 6; i++)
                     if (bp->nei_level[i] == lcl->level+1) {
                        bp->refine = 0;
                        lchange++;
                        if ((p = bp->parent) != -1 &&
                            bp->parent_node == my_pe) {
                           if ((pp = &parents[p])->refine == -1)
                              pp->refine = 0;
                           for (b = 0; b < 8; b++)
                              if (pp->child_node[b] == my_pe &&
                                  pp->child[b] >= 0 &&
                                  blocks[pp->child[b]].refine == -1)
                                 blocks[pp->child[b]].refine = 0;
                        }
                     }
               }
            }
         }

         CALL_SUSPENDABLE_CALLEE(1)
         gasket__mpi_Allreduce(glbl, &lchange, &change, 1, MPI_INTEGER, MPI_SUM,
                       MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)

         // Communicate these changes if any made
         if (change) {
            CALL_SUSPENDABLE_CALLEE(1)
            comm_reverse_refine(glbl);
            DEBRIEF_SUSPENDABLE_FUNCTION(-9999)
            // Communicate any changes of which blocks will refine
            CALL_SUSPENDABLE_CALLEE(1)
            comm_refine(glbl);
            DEBRIEF_SUSPENDABLE_FUNCTION(-9999)
            CALL_SUSPENDABLE_CALLEE(1)
            comm_parent_reverse(glbl);
            DEBRIEF_SUSPENDABLE_FUNCTION(-9999)
            CALL_SUSPENDABLE_CALLEE(1)
            comm_parent(glbl);
            DEBRIEF_SUSPENDABLE_FUNCTION(-9999)
         }
      } while (change);

      /* Check for blocks at this level that will remain at this level
         their neighbors at a lower level can not unrefine
      */
      do {
         lchange = 0;
         for (in = sorted_index[lcl->level]; in < sorted_index[lcl->level+1]; in++) {
            lcl->n = sorted_list[in].n;
            bp = &blocks[lcl->n];
            if (bp->number >= 0)
               if (bp->level == lcl->level && bp->refine == 0)
                  for (c = 0; c < 6; c++)
                     if (bp->nei_level[c] == lcl->level-1) {
                        if ((lcl->nei = bp->nei[c][0][0]) >= 0) {
                           if (blocks[lcl->nei].refine == -1) {
                              blocks[lcl->nei].refine = 0;
                              lchange++;
                              if ((p = blocks[lcl->nei].parent) != -1 &&
                                    blocks[lcl->nei].parent_node == my_pe)
                                 if ((pp = &parents[p])->refine == -1) {
                                    pp->refine = 0;
                                    for (b = 0; b < 8; b++)
                                       if (pp->child_node[b] == my_pe &&
                                           pp->child[b] >= 0 &&
                                           blocks[pp->child[b]].refine == -1)
                                          blocks[pp->child[b]].refine = 0;
                                 }
                           }
                        } else
                           if (bp->nei_refine[c] == -1) {
                              bp->nei_refine[c] = 0;
                              lchange++;
                           }
                     } else if (bp->nei_level[c] == lcl->level) {
                        if ((lcl->nei = bp->nei[c][0][0]) >= 0)
                           blocks[lcl->nei].nei_refine[(c/2)*2+(c+1)%2] = 0;
                     } else if (bp->nei_level[c] == lcl->level+1) {
                        c1 = (c/2)*2 + (c+1)%2;
                        for (i = 0; i < 2; i++)
                           for (j = 0; j < 2; j++)
                              if ((lcl->nei = bp->nei[c][i][j]) >= 0)
                                 blocks[lcl->nei].nei_refine[c1] = 0;
                     }
         }

         CALL_SUSPENDABLE_CALLEE(1)
         gasket__mpi_Allreduce(glbl, &lchange, &change, 1, MPI_INTEGER, MPI_SUM,
                       MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)

         // Communicate these changes of any parent that can not refine
         if (change) {
            CALL_SUSPENDABLE_CALLEE(1)
            comm_reverse_refine(glbl);
            DEBRIEF_SUSPENDABLE_FUNCTION(-9999)
            CALL_SUSPENDABLE_CALLEE(1)
            comm_refine(glbl);
            DEBRIEF_SUSPENDABLE_FUNCTION(-9999)
            CALL_SUSPENDABLE_CALLEE(1)
            comm_parent(glbl);
            DEBRIEF_SUSPENDABLE_FUNCTION(-9999)
            // Communicate any changes of which blocks can not unrefine
            CALL_SUSPENDABLE_CALLEE(1)
            comm_parent_reverse(glbl);
            DEBRIEF_SUSPENDABLE_FUNCTION(-9999)
         }
      } while (change);
   }

   for (i = in = 0; in < sorted_index[num_refine+1]; in++)
     if (blocks[sorted_list[in].n].refine == 1)
        i++;

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(i)
   SUSPENDABLE_FUNCTION_EPILOGUE
//#undef  level
//#undef  nei
//#undef  n
#undef  i
#undef  j
#undef  b
#undef  c
#undef  c1
#undef  change
#undef  lchange
#undef  unrefine
#undef  sib
#undef  p
#undef  in
#undef  bp
#undef  bp1
#undef  pp
}

// Reset the neighbor lists on blocks so that matching them against objects
// can set those which can be refined.
void reset_all(Globals_t * const glbl)
{
   int n, c, in;
   Block_t *bp;
   Parent_t *pp;
   OBTAIN_ACCESS_TO_blocks
   OBTAIN_ACCESS_TO_parents
   OBTAIN_ACCESS_TO_sorted_list
   TRACE

   for (in = 0; in < sorted_index[num_refine+1]; in++) {
      n = sorted_list[in].n;
      if ((bp= &blocks[n])->number >= 0) {
         bp->refine = -1;
         for (c = 0; c < 6; c++)
            if (bp->nei_level[c] >= 0)
               bp->nei_refine[c] = -1;
      }
   }

   for (n = 0; n < max_active_parent; n++)
      if ((pp = &parents[n])->number >= 0) {
         pp->refine = -1;
         for (c = 0; c < 8; c++)
            if (pp->child[c] < 0)
               pp->refine = 0;
         if (pp->refine == 0)
            for (c = 0; c < 8; c++)
               if (pp->child_node[c] == my_pe && pp->child[c] >= 0)
                  if (blocks[pp->child[c]].refine == -1)
                     blocks[pp->child[c]].refine = 0;
      }
}

// Reset neighbor lists on blocks since those lists are incorrect on blocks
// that have just been split.
void reset_neighbors(Globals_t * const glbl)
{
   int n, c, in;
   Block_t *bp;
   OBTAIN_ACCESS_TO_blocks
   OBTAIN_ACCESS_TO_from
   OBTAIN_ACCESS_TO_sorted_list
   TRACE

   for (in = 0; in < sorted_index[num_refine+1]; in++) {
      n = sorted_list[in].n;
      if ((bp= &blocks[n])->number >= 0)
         for (c = 0; c < 6; c++)
            if (bp->nei_level[c] >= 0 && bp->nei[c][0][0] < 0)
               bp->nei_refine[c] = -1;
   }
}

// Redistribute blocks so that the number of blocks will not exceed the
// number of available blocks on processors during refinement and coarsening
void redistribute_blocks(Globals_t * const glbl, double * const tp, double * const tm, double * const tu, double * const time, int * const num_moved, int const num_split)
{
   OBTAIN_ACCESS_TO_blocks
   OBTAIN_ACCESS_TO_parents
   OBTAIN_ACCESS_TO_to
   OBTAIN_ACCESS_TO_from
   OBTAIN_ACCESS_TO_sorted_list

   typedef struct {
      Frame_Header_t myFrame;
      int i, in, m, n, p, need, excess, my_excess, target, rem, sum, my_active,
          space[num_pes], use[num_pes];
      double t1;
      struct {
         Block_t *bp;
         Parent_t *pp;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__redistribute_blocks_t;

#define i                 (lcl->i)
#define in                (lcl->in)
#define m                 (lcl->m)
// #define n                 (lcl->n)  // Overloaded name ("n" appears as a member of a struct elsewhere.  Just use the fully qualified name, lcl->n).
#define p                 (lcl->p)
#define need              (lcl->need)
#define excess            (lcl->excess)
#define my_excess         (lcl->my_excess)
#define target            (lcl->target)
#define rem               (lcl->rem)
#define sum               (lcl->sum)
#define my_active         (lcl->my_active)
#define space             (lcl->space)
#define use               (lcl->use)
#define t1                (lcl->t1)
#define bp                (lcl->pointers.bp)
#define pp                (lcl->pointers.pp)

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__redistribute_blocks_t)
   TRACE

   t1 = timer();

   for (i = 0; i < num_pes; i++)
      bin[i] = 0;
   bin[my_pe] = num_split;

   CALL_SUSPENDABLE_CALLEE(1)
   gasket__mpi_Allreduce(glbl, bin, gbin, num_pes, MPI_INTEGER, MPI_SUM, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
   DEBRIEF_SUSPENDABLE_FUNCTION(;)

   for (sum = i = 0; i < num_pes; i++) {
      from[i] = 0;
      sum += gbin[i];
   }

   for (i = 0; i < num_pes; i++)
      bin[i] = 0;
   bin[my_pe] = max_num_parents - num_parents - 1 - num_split;

   CALL_SUSPENDABLE_CALLEE(1)
   gasket__mpi_Allreduce(glbl, bin, space, num_pes, MPI_INTEGER, MPI_SUM, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
   DEBRIEF_SUSPENDABLE_FUNCTION(;)

   for (in = 0; in < sorted_index[num_refine+1]; in++)
      blocks[sorted_list[in].n].new_proc = -1;

   target = sum/num_pes;
   rem = sum - target*num_pes;

   for (excess = i = 0; i < num_pes; i++) {
      need = target + (i < rem);
      if (need > space[i]) {
         use[i] = space[i];
         excess += need - space[i];
      } else
         use[i] = need;
   }
   // loop while there is blocks to be moved and progress is being made
   // if there are blocks to move and no progress, the code will fail later
   while (excess && sum)
      for (sum = i = 0; i < num_pes && excess; i++)
         if (space[i] > use[i]) {
            use[i]++;
            excess--;
            sum++;
         }

   m = in = 0;
   if (num_split > use[my_pe]) {  // have blocks to give
      my_excess = num_split - use[my_pe];
      my_active = num_active - my_excess + 7*use[my_pe] + 1;
      (*num_moved) += my_excess;
      for (excess = i = 0; i < my_pe; i++)
         if (gbin[i] > use[i])
            excess += gbin[i] - use[i];
      for (need = i = 0; i < num_pes && my_excess; i++)
         if (gbin[i] < use[i]) {
            need += use[i] - gbin[i];
            if (need > excess)
               for ( ; in < sorted_index[num_refine+1] && need > excess &&
                       my_excess; in++)
                  if ((bp = &blocks[sorted_list[in].n])->refine == 1) {
                     from[i]++;
                     bp->new_proc = i;
                     need--;
                     my_excess--;
                     m++;
                  }
         }
   } else  // getting blocks
      my_active = num_active + 7*use[my_pe] + 1;

   for (in = 0; in < sorted_index[num_refine+1]; in++) {
      lcl->n = sorted_list[in].n;
      if ((bp = &blocks[lcl->n])->number >= 0)
         if (bp->refine == -1 && bp->parent_node != my_pe) {
            bp->new_proc = bp->parent_node;
            from[bp->parent_node]++;
            my_active--;
            m++;
            (*num_moved)++;
         }
   }
   for (p = 0; p < max_active_parent; p++)
      if ((pp = &parents[p])->number >= 0 && pp->refine == -1)
         for (i = 0; i < 8; i++)
            if (pp->child_node[i] != my_pe)
               my_active++;
            else
               blocks[pp->child[i]].new_proc = my_pe;

   CALL_SUSPENDABLE_CALLEE(1)
   gasket__mpi_Allreduce(glbl, &m, &lcl->n, 1, MPI_INTEGER, MPI_SUM, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
   DEBRIEF_SUSPENDABLE_FUNCTION(;)

   if (lcl->n) {
      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Allreduce(glbl, &my_active, &sum, 1, MPI_INTEGER, MPI_MAX, MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)

      if (sum > ((int) (0.75*((double) max_num_blocks)))) {
         // even up the expected number of blocks per processor
         for (i = 0; i < num_pes; i++)
            bin[i] = 0;
         bin[my_pe] = my_active;

         CALL_SUSPENDABLE_CALLEE(1)
         gasket__mpi_Allreduce(glbl, bin, gbin, num_pes, MPI_INTEGER, MPI_SUM,
                       MPI_COMM_WORLD, -9999, __FILE__, __LINE__);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)

         for (sum = i = 0; i < num_pes; i++)
            sum += gbin[i];

         target = sum/num_pes;
         rem = sum - target*num_pes;

         in = sorted_index[num_refine+1] - 1;  // don't want to move big blocks
         if (my_active > (target + (my_pe < rem))) {  // have blocks to give
            my_excess = my_active - (target + (my_pe < rem));
            (*num_moved) += my_excess;
            for (excess = i = 0; i < my_pe; i++)
               if (gbin[i] > (target + (i < rem)))
                  excess += gbin[i] - (target + (i < rem));
            for (need = i = 0; i < num_pes && my_excess; i++)
               if (gbin[i] < (target + (i < rem))) {
                  need += (target + (i < rem)) - gbin[i];
                  if (need > excess)
                     for ( ; in >= 0 && need > excess && my_excess; in--)
                        if ((bp = &blocks[sorted_list[in].n])->new_proc == -1){
                           from[i]++;
                           bp->new_proc = i;
                           need--;
                           my_excess--;
                           m++;
                        }
               }
         }
      } else
         in = sorted_index[num_refine+1] - 1;

      // keep the rest of the blocks on this processor
      for ( ; in >= 0; in--)
         if (blocks[sorted_list[in].n].new_proc == -1)
            blocks[sorted_list[in].n].new_proc = my_pe;

      *time = timer() - t1;

      CALL_SUSPENDABLE_CALLEE(1)
      gasket__mpi_Alltoall(glbl, from, 1, MPI_INTEGER, to, 1, MPI_INTEGER, MPI_COMM_WORLD, __FILE__, __LINE__);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
      CALL_SUSPENDABLE_CALLEE(1)
      move_blocks(glbl, tp, tm, tu);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
   } else
      *time = timer() - t1;
   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef  i
#undef  in
#undef  m
// #undef  n
#undef  p
#undef  need
#undef  excess
#undef  my_excess
#undef  target
#undef  rem
#undef  sum
#undef  my_active
#undef  space
#undef  use
#undef  t1
#undef  bp
#undef  pp

}

#endif
