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

#include <ocr.h>
#include <ocr-std.h>
#include <ocr-guid-functions.h>
//#include <extensions/ocr-labeling.h>
#include "ocrmacs.h"

#include "control.h"
#include "object.h"
#include "chksum.h"
#include "block.h"
#include "parent.h"
#include "refine.h"
//PROFILE:#include "profile.h"
#include "proto.h"
#include "clone.h"

#ifdef NANNY_FUNC_NAMES
#line __LINE__ "block  "
#endif

// block.c         This is a block of the mesh, at some refinement level.
// ***************************************************************************************************************************************************************************************
// ***************************************************************************************************************************************************************************************
// ***************************************************************************************************************************************************************************************
// blockLaunch_Func
//
// This just creates the blockInit EDT and the datablocks that it will need to initialize:  control_dblk, allObjects_dblk.  Data to develop the block's meta datablock is passed as params.
// It then passes control to the blockInit EDT.

ocrGuid_t blockLaunch_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

   //blockLaunch_Deps_t   * myDeps   = (blockLaunch_Deps_t *)   depv;
   blockLaunch_Deps_t   * myDeps;
   ocrGuid_t localDeps;
   ocrDbCreate( &localDeps, (void **)&myDeps, sizeof( ocrEdtDep_t ) * depc, DB_PROP_NONE, NULL_HINT, NO_ALLOC );
   memcpy( myDeps, depv, sizeof( ocrEdtDep_t ) * depc );
   blockLaunch_Params_t * myParams = (blockLaunch_Params_t *) paramv;
   ocrGuid_t          initialControl_dblk    =                (myDeps->control_Dep.guid);
   Control_t  const * initialControl         = ((Control_t *) (myDeps->control_Dep.ptr));
   ocrGuid_t          initialAllObjects_dblk =                (myDeps->allObjects_Dep.guid);
//printf ("Function %36s, File %30s, line %4d, for block at (xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, myParams->xPos, myParams->yPos, myParams->zPos); fflush(stdout);

   ocrGuid_t    meta_dblk;
   ocrGuid_t    control_dblk;
   ocrGuid_t    allObjects_dblk;
   ocrGuid_t    block_dblk;
   void * dummy = NULL;

#ifdef NANNY_ON_STEROIDS
#define NANNYLEN 200
   char nanny[NANNYLEN];
   sprintf(nanny, "xPos=%4d, yPos=%4d, zPos=%4d", myParams->xPos, myParams->yPos, myParams->zPos);
#else
#define nanny NULL
#endif

   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->control_Dep,    blockLaunch_Deps_t, control_Dep,    nanny, "control");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->allObjects_Dep, blockLaunch_Deps_t, allObjects_Dep, nanny, "allObjects");

   gasket__ocrDbCreate(&meta_dblk,         &dummy, sizeof_BlockMeta_t,  __FILE__, __func__, __LINE__, nanny, "meta");
   gasket__ocrDbCreate(&control_dblk,      &dummy, sizeof_Control_t,    __FILE__, __func__, __LINE__, nanny, "control");
#define control initialControl  // In all other contexts, the control_t datablock (which contains num_objects) will be available, but here, it is in initialControl_dblk, so we hack a look-alike.
   gasket__ocrDbCreate(&allObjects_dblk,   &dummy, sizeof_AllObjects_t, __FILE__, __func__, __LINE__, nanny, "allObjects");
   gasket__ocrDbCreate(&block_dblk,        &dummy, sizeof_Block_t,      __FILE__, __func__, __LINE__, nanny, "block");
#undef control            // Clean up the above hack, so that it's effects will not propagate to any code below.
   blockInit_Params_t * blockInit_Params = myParams;

   ocrGuid_t     blockInit_Edt;
#ifdef NANNY_ON_STEROIDS
   sprintf(nanny, "xPos=%4d, yPos=%4d, zPos=%4d", myParams->xPos, myParams->yPos, myParams->zPos);
#endif
   gasket__ocrEdtCreate(&blockInit_Edt,
                        SLOT(blockInit_Deps_t, whoAmI_Dep),
                        myParams->template.blockInit_Template,
                        EDT_PARAM_DEF,
                        (u64 *) blockInit_Params,
                        EDT_PARAM_DEF,
                        NULL,
                        EDT_PROP_NONE,
                        NULL_HINT,
                        NULL,
                        __FILE__,
                        __func__,
                        __LINE__,
                        nanny,
                        "blockInit");

   gasket__ocrDbRelease  (initialControl_dblk, __FILE__, __func__, __LINE__, nanny, "initialControl"); initialControl = NULL;
   ADD_DEPENDENCE(meta_dblk,                blockInit_Edt, blockInit_Deps_t, meta_Dep,               DB_MODE_RW, nanny, "meta")
   ADD_DEPENDENCE(initialControl_dblk,      blockInit_Edt, blockInit_Deps_t, initialControl_Dep,     DB_MODE_RO, nanny, "initialControl")
   ADD_DEPENDENCE(control_dblk,             blockInit_Edt, blockInit_Deps_t, control_Dep,            DB_MODE_RW, nanny, "control")
   ADD_DEPENDENCE(initialAllObjects_dblk,   blockInit_Edt, blockInit_Deps_t, initialAllObjects_Dep,  DB_MODE_RO, nanny, "initialAllObjects")
   ADD_DEPENDENCE(allObjects_dblk,          blockInit_Edt, blockInit_Deps_t, allObjects_Dep,         DB_MODE_RW, nanny, "allObjects")
   ADD_DEPENDENCE(block_dblk,               blockInit_Edt, blockInit_Deps_t, block_Dep,              DB_MODE_RW, nanny, "block")

   REPORT_EDT_DEMISE(myDeps->whoAmI_Dep);
   // Defeat OCR's habit of releasing the input ocrEdtDep_t's, because we have changed many of them to other guids, including some events, which simply must not be released.  This is just a debugging expedient.
   if (1) {
      myDeps->whoAmI_Dep.guid              = NULL_GUID;
      myDeps->control_Dep.guid             = NULL_GUID;
      myDeps->allObjects_Dep.guid          = NULL_GUID;
   }
   return NULL_GUID;
} // blockLaunch_Func


// **************************************************************************************************************************************************************************************************************
// blockInit_Func
//
// This initializes control_dblk, allObjects_dblk, and meta_dblk, then creates all other datablocks that will be needed to process the block, then passes control to the blockClone EDT.
ocrGuid_t blockInit_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

   //blockInit_Deps_t   * myDeps   = (blockInit_Deps_t *)   depv;
   blockInit_Deps_t * myDeps;
   ocrGuid_t localDeps;
   ocrDbCreate( &localDeps, (void **)&myDeps, sizeof( ocrEdtDep_t ) * depc, DB_PROP_NONE, NULL_HINT, NO_ALLOC );
   memcpy( myDeps, depv, sizeof( ocrEdtDep_t ) * depc );
   blockInit_Params_t * myParams = (blockInit_Params_t *) paramv;
   ocrGuid_t            meta_dblk               =                   (myDeps->meta_Dep.guid);
   BlockMeta_t        * meta                    = ((BlockMeta_t *)  (myDeps->meta_Dep.ptr));
   Control_t    const * initialControl          = ((Control_t *)    (myDeps->initialControl_Dep.ptr));
   ocrGuid_t            control_dblk            =                   (myDeps->control_Dep.guid);
   Control_t          * control                 = ((Control_t *)    (myDeps->initialControl_Dep.ptr)); // Purposely set this "wrong" for just a moment.  We need this so that sizeof_Control_t macro will be right.
   AllObjects_t const * initialAllObjects       = ((AllObjects_t *) (myDeps->initialAllObjects_Dep.ptr));
   ocrGuid_t            allObjects_dblk         =                   (myDeps->allObjects_Dep.guid);
   AllObjects_t       * allObjects              = ((AllObjects_t *) (myDeps->allObjects_Dep.ptr));
   ocrGuid_t            block_dblk              =                   (myDeps->block_Dep.guid);

#ifdef NANNY_ON_STEROIDS
   char nanny[NANNYLEN];
   sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", 0, myParams->xPos, myParams->yPos, myParams->zPos, 0);
#else
#define nanny NULL
#endif

   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->meta_Dep,              blockInit_Deps_t,          meta_Dep,              nanny, "meta");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->initialControl_Dep,    blockInit_Deps_t,          initialControl_Dep,    nanny, "initialControl");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->control_Dep,           blockInit_Deps_t,          control_Dep,           nanny, "control");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->initialAllObjects_Dep, blockInit_Deps_t,          initialAllObjects_Dep, nanny, "initialAllObjects");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->allObjects_Dep,        blockInit_Deps_t,          allObjects_Dep,        nanny, "allObjects");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->block_Dep,             blockInit_Deps_t,          block_Dep,             nanny, "block");

// Do this first!   Init our copy of control_db.  Some of the sizeof_... references below expand macros that depend upon having a valid "control" pointer, pointing to valid contents..

   int tmp = sizeof_Control_t;
   control = ((Control_t *)    (myDeps->control_Dep.ptr));        // Fix this pointer that was temporarily hacked to initialControl.
   memcpy (control,    initialControl,    tmp);
   memcpy (allObjects, initialAllObjects, sizeof_AllObjects_t);

   int i;
   blockClone_Deps_t cloneDeps;
   depv = &cloneDeps.firstDependenceSlot;
   DbSize_t   * dbSize                                          = meta->dbSize;

   DbSize(cloneDeps.whoAmI_Dep)                = sizeof(ocrGuid_t);
   DbSize(cloneDeps.meta_Dep)                  = sizeof_BlockMeta_t;
   DbSize(cloneDeps.metaPred_Dep)              = sizeof_BlockMeta_t;
   DbSize(cloneDeps.control_Dep)               = sizeof_Control_t;
   DbSize(cloneDeps.controlPred_Dep)           = sizeof_Control_t;
   DbSize(cloneDeps.block_Dep)                 = sizeof_Block_t;
   DbSize(cloneDeps.blockPred_Dep)             = sizeof_Block_t;
   DbSize(cloneDeps.allObjects_Dep)            = sizeof_AllObjects_t;
   DbSize(cloneDeps.allObjectsPred_Dep)        = sizeof_AllObjects_t;
   DbSize(cloneDeps.upboundRequest_Dep)        = -9999;   // Size context-dependent, i.e. determined when datablock created.  However, it is not needed for crawl-out/crawl-in logic,
                                                          // nor for RefreshRW, Replicate, or Migrate, so just leave it at -9999
   DbSize(cloneDeps.replyFromRequest_Dep)      = sizeof_RefinementDisposition_t;
//printf ("Function %36s, File %30s, line %4d, for block at (xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, myParams->xPos, myParams->yPos, myParams->zPos); fflush(stdout);

// Now init our meta data.
// First, init the Cloning State.

   Frame_Header_t * topOfStack  = (Frame_Header_t *) meta->cloningState.stack;
   topOfStack[0].resumption_case_num                  = -9999;                  // Irrelevant for topmost activation record.
   topOfStack[0].my_size                              = sizeof_Frame_Header_t;  // Distance to first callee's frame.
   topOfStack[0].caller_size                          = -9999;                  // Irrelevant for topmost activation record.
   topOfStack[0].validate_callers_prep_for_suspension = -9999;                  // Irrelevant for topmost activation record.
   topOfStack[1].resumption_case_num                  = 0;                      // Prepare for first callee.
   topOfStack[1].my_size                              = -9999;                  // Size of callee's frame is not known by caller.
   topOfStack[1].caller_size                          = sizeof_Frame_Header_t;  // To tell callee how much to pop when it returns.
   meta->cloningState.topPtrAdjRecOffset              = -9999;
   meta->cloningState.numberOfDatablocks              = countof_blockClone_fixedDeps_t;
   meta->cloningState.cloneNum                        = 0;

// Init other meta variables.

   int size = control->p2[control->num_refine+1];     // Block size is p2[num_refine+1-level].  Smallest block is size p2[1], so can find its center

   meta->refinementLevel                    = 0;
   meta->xPos                               = myParams->xPos;
   meta->yPos                               = myParams->yPos;
   meta->zPos                               = myParams->zPos;
   meta->numBaseResBlocks                   = control->npx * control->npy * control->npz;
   meta->cen[0]                             = myParams->xPos * size + (size >> 1);
   meta->cen[1]                             = myParams->yPos * size + (size >> 1);
   meta->cen[2]                             = myParams->zPos * size + (size >> 1);
   meta->conveyServiceRequestToParent_Event = myParams->conveyServiceRequestToParent_Event;   // Event to satisfy dependence at parent, by which he is awaiting a service request datablock from us.
   meta->labeledGuidRangeForHaloExchange    = myParams->labeledGuidRangeForHaloExchange;
//printf ("%s line %d, labeledGuidRanveForHaloExchange:  0x%lx at %p\n", __FILE__, __LINE__, (unsigned long long) meta->labeledGuidRangeForHaloExchange, &meta->labeledGuidRangeForHaloExchange); fflush(stdout);

   int axis, pole, qrtr;
   for (axis = 0; axis < 3; axis++) {
      for (pole = 0; pole <= 1; pole++) {
         for (qrtr = 0; qrtr < 4; qrtr++) {
            meta->conveyFaceToNeighbor_Event[axis][pole][qrtr] = NULL_GUID; // Create event upon first demand, via labeled-guid lookup
            meta->onDeckToReceiveFace_Event [axis][pole][qrtr] = NULL_GUID; // Create event upon first demand, via labeled-guid lookup
         }
      }
   }
   for (i = 0; i < 8; i++) {
      meta->conveyEighthBlockToJoin_Event[i] = NULL_GUID;  // Create event upon fork operation (i.e. refine)
   }

   meta->neighborRefinementLevel[0][0] = meta->xPos == 0                ? NEIGHBOR_IS_OFF_EDGE_OF_MESH : 0;
   meta->neighborRefinementLevel[0][1] = meta->xPos == control->npx - 1 ? NEIGHBOR_IS_OFF_EDGE_OF_MESH : 0;
   meta->neighborRefinementLevel[1][0] = meta->yPos == 0                ? NEIGHBOR_IS_OFF_EDGE_OF_MESH : 0;
   meta->neighborRefinementLevel[1][1] = meta->yPos == control->npy - 1 ? NEIGHBOR_IS_OFF_EDGE_OF_MESH : 0;
   meta->neighborRefinementLevel[2][0] = meta->zPos == 0                ? NEIGHBOR_IS_OFF_EDGE_OF_MESH : 0;
   meta->neighborRefinementLevel[2][1] = meta->zPos == control->npz - 1 ? NEIGHBOR_IS_OFF_EDGE_OF_MESH : 0;

// Pass baton to the first in a series of clones.

   blockClone_Params_t blockClone_Params;
   blockClone_Params.template     = myParams->template;
   blockClone_Params.prongNum     = -1;                              // Indicate NOT continuing into the prong of a fork (which happens only when we refine)
   blockClone_Params.conveyServiceRequestToParent_Event = NULL_GUID; // This field is not applicable now.  It is only applicable when we are doing the Fork, or the original mesh creation from the root)

   ocrGuid_t blockClone_Edt;
#ifdef NANNY_ON_STEROIDS
   sprintf(nanny, "xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
   gasket__ocrEdtCreate(&blockClone_Edt,
                        SLOT(blockClone_Deps_t, whoAmI_Dep),
                        myParams->template.blockClone_Template,
                        EDT_PARAM_DEF,
                        (u64 *) &blockClone_Params,
                        countof_blockClone_fixedDeps_t,
                        NULL,
                        EDT_PROP_NONE,
                        NULL_HINT,
                        NULL,
                        __FILE__,
                        __func__,
                        __LINE__,
                        nanny,
                        "blockClone");

   gasket__ocrDbRelease(meta_dblk,                   __FILE__, __func__, __LINE__,  nanny, "meta");
   gasket__ocrDbRelease(myDeps->control_Dep.guid,    __FILE__, __func__, __LINE__,  nanny, "control");
   gasket__ocrDbRelease(myDeps->allObjects_Dep.guid, __FILE__, __func__, __LINE__,  nanny, "allObjects");

   ADD_DEPENDENCE(meta_dblk,                   blockClone_Edt, blockClone_Deps_t, meta_Dep,             DB_MODE_RW,   nanny, "meta")
   ADD_DEPENDENCE(NULL_GUID,                   blockClone_Edt, blockClone_Deps_t, metaPred_Dep,         DB_MODE_NULL, nanny, "metaPred")
   ADD_DEPENDENCE(control_dblk,                blockClone_Edt, blockClone_Deps_t, control_Dep,          DB_MODE_RO,   nanny, "control")
   ADD_DEPENDENCE(NULL_GUID,                   blockClone_Edt, blockClone_Deps_t, controlPred_Dep,      DB_MODE_NULL, nanny, "controlPred")
   ADD_DEPENDENCE(block_dblk,                  blockClone_Edt, blockClone_Deps_t, block_Dep,            DB_MODE_RW,   nanny, "block")
   ADD_DEPENDENCE(NULL_GUID,                   blockClone_Edt, blockClone_Deps_t, blockPred_Dep,        DB_MODE_NULL, nanny, "blockPred")
   ADD_DEPENDENCE(allObjects_dblk,             blockClone_Edt, blockClone_Deps_t, allObjects_Dep,       DB_MODE_RW,   nanny, "allObjects")
   ADD_DEPENDENCE(NULL_GUID,                   blockClone_Edt, blockClone_Deps_t, allObjectsPred_Dep,   DB_MODE_NULL, nanny, "allObjectsPred_Dep")
   ADD_DEPENDENCE(NULL_GUID,                   blockClone_Edt, blockClone_Deps_t, upboundRequest_Dep,   DB_MODE_NULL, nanny, "upboundRequest")
   ADD_DEPENDENCE(NULL_GUID,                   blockClone_Edt, blockClone_Deps_t, replyFromRequest_Dep, DB_MODE_NULL, nanny, "replyFromRequest")

   REPORT_EDT_DEMISE(myDeps->whoAmI_Dep);
   // Defeat OCR's habit of releasing the input ocrEdtDep_t's, because we have changed many of them to other guids, including some events, which simply must not be released.  This is just a debugging expedient.
   if (0) {
      myDeps->meta_Dep.guid                = NULL_GUID;
      myDeps->whoAmI_Dep.guid              = NULL_GUID;
      myDeps->initialControl_Dep.guid      = NULL_GUID;
      myDeps->control_Dep.guid             = NULL_GUID;
      myDeps->initialAllObjects_Dep.guid   = NULL_GUID;
      myDeps->allObjects_Dep.guid          = NULL_GUID;
      myDeps->block_Dep.guid               = NULL_GUID;
   }
   return NULL_GUID;
} // blockInit_Func


// **************************************************************************************************************************************************************************************************************
// blockClone_Func
//
// This is the code that performs the algorithm on a block at the leaf of the refinement level depth tree.  It advances the algorithm as far as it can, to such point where a communication is needed from
// another EDT (such as a halo exchange from a neighbor, or such as the results of a service requested of the parent), at which time it clones, and the clone continues where the predecessor left off.  This
// particular function just runs the loop that drives the cloning logic.  It calls blockClone_SoupToNuts, which is the top function of the calling topology of functions that do the work.
ocrGuid_t blockClone_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

   int i, j, k;

   blockClone_Deps_t   * myDeps   = (blockClone_Deps_t *)   depv;
   /*blockClone_Deps_t * myDeps;
   ocrGuid_t localDeps;
   ocrDbCreate( &localDeps, (void **)&myDeps, sizeof( ocrEdtDep_t ) * depc, DB_PROP_NONE, NULL_HINT, NO_ALLOC );
   memcpy( myDeps, depv, sizeof( ocrEdtDep_t ) * depc );*/

   blockClone_Params_t * myParams = (blockClone_Params_t *) paramv;
   BlockMeta_t * meta    = (BlockMeta_t *) myDeps->meta_Dep.ptr;
   Control_t   * control = (Control_t   *) myDeps->control_Dep.ptr;

//printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d, clone=%4d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum); fflush(stdout);
   if (!ocrGuidIsNull(myDeps->metaPred_Dep.guid)) { // When metaPred is non-null, it means we are doing a fork for a refine, or a join for an unrefine.
      // In both the fork and the join cases, we make fresh copies of Control_t and AllObjects_t datablocks, to assure improved collocation.
      DbSize_t * dbSize = ((BlockMeta_t *) myDeps->metaPred_Dep.ptr)->dbSize;
      memcpy (myDeps->control_Dep.ptr,    myDeps->controlPred_Dep.ptr,    DbSize(myDeps->controlPred_Dep));     // Destroy Pred version below, ONLY for join case!
      memcpy (myDeps->allObjects_Dep.ptr, myDeps->allObjectsPred_Dep.ptr, DbSize(myDeps->allObjectsPred_Dep));  // Destroy Pred version below, ONLY for join case!

      if (ocrGuidIsNull(myDeps->parentMeta_Dep.guid)) {  // This is NULL_GUID for fork; it is the parentMeta datablock for join.
         // Fork:  We have to copy the metaPred to meta before we can proceed to crawl back into the activation records.  While doing so, we will also copy controlPred to control, and allObjectsPred to
         //        allObjects.  But we won't bother copying blockPred to block, because refine will need to upsample (an eighth of) it anyway.

         memcpy (myDeps->meta_Dep.ptr,       myDeps->metaPred_Dep.ptr,       DbSize(myDeps->metaPred_Dep));
         myDeps->metaPred_Dep.guid       = NULL_GUID; myDeps->metaPred_Dep.ptr       = NULL; // Do NOT destroy; siblings are using asynchronously.  Parent will destroy.
         myDeps->controlPred_Dep.guid    = NULL_GUID; myDeps->controlPred_Dep.ptr    = NULL;
         myDeps->allObjectsPred_Dep.guid = NULL_GUID; myDeps->allObjectsPred_Dep.ptr = NULL;
         // Remaining meta details will be adjusted when we resume this EDT at the crawl-in into the refine function.
      } else {
         // Join: We have to aggregate the meta from parts of the prong000 contribution (metaPred) and other parts from the parent contribution (parentMeta).
         BlockMeta_t  * metaPred    = (BlockMeta_t  *) myDeps->metaPred_Dep.ptr;
         ParentMeta_t * parentMeta  = (ParentMeta_t *) myDeps->parentMeta_Dep.ptr;

         for (i = 0; i < 8; i++) {
#ifdef NANNY_ON_STEROIDS
            char nanny[NANNYLEN];
            sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, [i=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, i);
#else
#define nanny NULL
#endif
            gasket__ocrEventDestroy(&metaPred->conveyEighthBlockToJoin_Event[i], __FILE__, __func__, __LINE__, nanny, "conveyEighthBlockToJoin[...]");
         }

         meta->neighborRefinementLevel[0][0]                = metaPred->neighborRefinementLevel[0][0];
         meta->neighborRefinementLevel[0][1]                = metaPred->neighborRefinementLevel[0][1];
         meta->neighborRefinementLevel[1][0]                = metaPred->neighborRefinementLevel[1][0];
         meta->neighborRefinementLevel[1][1]                = metaPred->neighborRefinementLevel[1][1];
         meta->neighborRefinementLevel[2][0]                = metaPred->neighborRefinementLevel[2][0];
         meta->neighborRefinementLevel[2][1]                = metaPred->neighborRefinementLevel[2][1];
         for (i = 0; i < countof_blockClone_fixedDeps_t; i++) {
            meta->dbSize[i]                                 = metaPred->dbSize[i];
         }
         meta->cloningState                                 = metaPred->cloningState;
         meta->labeledGuidRangeForHaloExchange              = metaPred->labeledGuidRangeForHaloExchange;
         meta->blockClone_Edt                               = metaPred->blockClone_Edt;

         meta->conveyServiceRequestToParent_Event           = parentMeta->conveyServiceRequestToParent_Event;
         for (i = 0; i < 8; i++) {
            meta->conveyEighthBlockToJoin_Event[i]          = parentMeta->conveyEighthBlockToJoin_Event[i];
         }
         for (i = 0; i < 3; i++) {
            for (j = 0; j < 2; j++) {
               for (k = 0; k < 4; k++) {
                  if (!ocrGuidIsNull(parentMeta->conveyFaceToNeighbor_Event[i][j][k])) {
                     *((int *) 123) = 456;
                  }
                  if (!ocrGuidIsNull(parentMeta->onDeckToReceiveFace_Event[i][j][k])) {
                     *((int *) 123) = 456;
                  }
// THESE SHOULD HAVE BEEN SEVERED BEFORE THE CORRESPONDING FORK, SO THEY SHOULD BE NULL_GUIDS!!!  The above forced-crashes will validate that assertion.  If these assertion ever get hit, it is
// probably indicative that one side or other of the communication channel is not living up to the contract -- to maintain the channel, and then to sever when one side or the other refines/unrefines.
                  meta->conveyFaceToNeighbor_Event[i][j][k] = parentMeta->conveyFaceToNeighbor_Event[i][j][k];
// THESE SHOULD HAVE BEEN SEVERED BEFORE THE CORRESPONDING FORK, SO THEY SHOULD BE NULL_GUIDS!!!  Same comment as above.
                  meta->onDeckToReceiveFace_Event[i][j][k]  = parentMeta->onDeckToReceiveFace_Event[i][j][k];
               }
            }
         }
         meta->xPos                                         = parentMeta->xPos;
         meta->yPos                                         = parentMeta->yPos;
         meta->zPos                                         = parentMeta->zPos;
         meta->numBaseResBlocks                             = parentMeta->numBaseResBlocks;
         meta->cen[0]                                       = parentMeta->cen[0];
         meta->cen[1]                                       = parentMeta->cen[1];
         meta->cen[2]                                       = parentMeta->cen[2];
         meta->refinementLevel                              = parentMeta->refinementLevel;

         Block_t * block111 = myDeps->aux_Dep[7].ptr;
         meta->neighborRefinementLevel[0][1] = block111->neighborRefinementLevel_East;   // Obtain prong111 values for non-sibling neighbors.
         meta->neighborRefinementLevel[1][1] = block111->neighborRefinementLevel_North;
         meta->neighborRefinementLevel[2][1] = block111->neighborRefinementLevel_Up;

         meta->finerNeighborNotUnrefiningAndNotTalkingAnyMore = 0;   // Gather up the bit vectors from the eight prongs.
         for (i = 0; i < 8; i++) {
            Block_t * block = myDeps->aux_Dep[i].ptr;
            meta->finerNeighborNotUnrefiningAndNotTalkingAnyMore |= block->finerNeighborNotUnrefiningAndNotTalkingAnyMore;
         }

#ifdef NANNY_ON_STEROIDS
         char nanny[NANNYLEN];
         sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
         gasket__ocrDbDestroy(&myDeps->metaPred_Dep.guid,       &myDeps->metaPred_Dep.ptr,       __FILE__, __func__, __LINE__, nanny, "metaPred");
         gasket__ocrDbDestroy(&myDeps->parentMeta_Dep.guid,     &myDeps->parentMeta_Dep.ptr,     __FILE__, __func__, __LINE__, nanny, "parentMeta");
         gasket__ocrDbDestroy(&myDeps->controlPred_Dep.guid,    &myDeps->controlPred_Dep.ptr,    __FILE__, __func__, __LINE__, nanny, "controlPred");
         gasket__ocrDbDestroy(&myDeps->allObjectsPred_Dep.guid, &myDeps->allObjectsPred_Dep.ptr, __FILE__, __func__, __LINE__, nanny, "allObjectsPred");
         // Remaining meta details will be adjusted when we resume this EDT at the crawl-in into the unrefine function.
     }
  } else {  // We are a simple continuation clone.
      if (!ocrGuidIsNull(myDeps->controlPred_Dep.guid)    ||
          !ocrGuidIsNull(myDeps->blockPred_Dep.guid)      ||
          !ocrGuidIsNull(myDeps->allObjectsPred_Dep.guid)    ) {
         *((int *) 123) = 456;    // ALL four of these should NOT have been provided by the predecessor.
      }
   }

   // Recover pointers in the meta datablock

   meta->cloningState.tos                         = ((char *)                  (((unsigned long long) meta->cloningState.stack)));
   meta->cloningState.topPtrAdjRec                = ((PtrAdjustmentRecord_t *) (((unsigned long long) meta->cloningState.stack) + meta->cloningState.topPtrAdjRecOffset));
   meta->cloningState.cloneNum++;

   // Audit incoming dependences.

#ifdef NANNY_ON_STEROIDS
   char nanny[NANNYLEN];
   sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif

   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->meta_Dep,             blockClone_Deps_t,          meta_Dep,             nanny, "meta");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->metaPred_Dep,         blockClone_Deps_t,          metaPred_Dep,         nanny, "metaPred");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->control_Dep,          blockClone_Deps_t,          control_Dep,          nanny, "control");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->controlPred_Dep,      blockClone_Deps_t,          controlPred_Dep,      nanny, "controlPred");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->block_Dep,            blockClone_Deps_t,          block_Dep,            nanny, "block");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->blockPred_Dep,        blockClone_Deps_t,          blockPred_Dep,        nanny, "blockPred");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->allObjects_Dep,       blockClone_Deps_t,          allObjects_Dep,       nanny, "allObjects");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->allObjectsPred_Dep,   blockClone_Deps_t,          allObjectsPred_Dep,   nanny, "allObjectsPred");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->upboundRequest_Dep,   blockClone_Deps_t,          upboundRequest_Dep,   nanny, "upboundRequest");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->replyFromRequest_Dep, blockClone_Deps_t,          replyFromRequest_Dep, nanny, "replyFromRequest");
   for (i = 0; i < depc - ((offsetof(blockClone_Deps_t, aux_Dep[0])) / sizeof(ocrEdtDep_t)) ; i++) {
      INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->aux_Dep[i],        blockClone_Deps_t,          aux_Dep[i],           nanny, "aux_Dep[i]");
   }

   meta->cloningState.cloningOpcode = OpcodeUnspecified;      // Set up to detect any case where code forgets to set the cloningOpcode.
   meta->blockClone_Edt             = NULL_GUID;              // If this is still NULL_GUID when we get back, we have to create the EDT.  Otherwise, the code that instigated the clone did it (i.e. specially).

   Frame_Header_t * topOfStack  = (Frame_Header_t *) meta->cloningState.stack;

   topOfStack[1].validate_callers_prep_for_suspension = 1;    // Set "canary trap" for first callee.
   meta->cloningState.returnCanary                    = 1;    // Init the "return canary trap" to "live", to detect a return from a suspendable function via a return stmt instead of the macro.
   blockClone_SoupToNuts(myParams, depc, (ocrEdtDep_t *)myDeps);               // Resume (or start, the first time) the algorithm.  When we get back, it is either done or needing to clone.
   if (meta->cloningState.returnCanary == 1) {                // If the return canary is "live" the callee must have used a return statement.  We need it to use the macro!
      printf ("ERROR: %s at line %d: Canary trap on RETURN from the callee.  Change callee to return through the NORMAL_RETURN_SEQUENCE macro.\n", __FILE__, __LINE__); fflush(stdout);
      printf ("ERROR: Other possibility is that callee is not (yet) a SUSPENDABLE function, but it is being identified as such by the caller.\n"); fflush(stdout);
      *((int *) 123) = 456;
      ocrShutdown();
   }
   if (topOfStack[1].resumption_case_num != 0) {  // If we are NOT done yet.

      switch (meta->cloningState.cloningOpcode) { // We either need to create a single continuation clone EDT, fork (into eight blocks and a parent), or join.  The cloningOpcode will tell us which.
      case SeasoningOneOrMoreDbCreates:
      case ReceivingACommunication:
      {
         // Simple single continuation case.  Create a continuation EDT, cause it to fire, and cause this EDT has to terminate.

         meta->cloningState.topPtrAdjRecOffset = ((int) ((unsigned long long) meta->cloningState.topPtrAdjRec) - ((unsigned long long) meta->cloningState.stack));

#ifdef NANNY_ON_STEROIDS
         char nanny[NANNYLEN];
         sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif

         ocrGuid_t blockClone_Edt = meta->blockClone_Edt;
         if (ocrGuidIsNull(blockClone_Edt)) {
            gasket__ocrEdtCreate(&blockClone_Edt,                           // Guid of the EDT created to continue at function blockContinuaiton_Func.
                                 SLOT(blockClone_Deps_t, whoAmI_Dep),
                                 myParams->template.blockClone_Template,    // Template for the EDT we are creating.
                                 EDT_PARAM_DEF,
                                 (u64 *) myParams,
                                 countof_blockClone_fixedDeps_t,
                                 NULL,
                                 EDT_PROP_NONE,
                                 NULL_HINT,
                                 NULL,
                                 __FILE__,
                                 __func__,
                                 __LINE__,
                                 nanny,
                                 "blockClone (Continuation)");
         }
         ADD_DEPENDENCE(      myDeps->metaPred_Dep.guid,         blockClone_Edt, blockClone_Deps_t, metaPred_Dep,         DB_MODE_NULL, nanny, "metaPred")
         gasket__ocrDbRelease(myDeps->control_Dep.guid,                                                __FILE__, __func__, __LINE__,    nanny, "control");
         ADD_DEPENDENCE(      myDeps->control_Dep.guid,          blockClone_Edt, blockClone_Deps_t, control_Dep,          DB_MODE_RO,   nanny, "control")
         ADD_DEPENDENCE(      NULL_GUID,                         blockClone_Edt, blockClone_Deps_t, controlPred_Dep,      DB_MODE_NULL, nanny, "controlPred")
         gasket__ocrDbRelease(myDeps->block_Dep.guid,                                                  __FILE__, __func__, __LINE__,    nanny, "block");
         ADD_DEPENDENCE(      myDeps->block_Dep.guid,            blockClone_Edt, blockClone_Deps_t, block_Dep,            DB_MODE_RW,   nanny, "block")
         ADD_DEPENDENCE(      NULL_GUID,                         blockClone_Edt, blockClone_Deps_t, blockPred_Dep,        DB_MODE_NULL, nanny, "blockPred")
         gasket__ocrDbRelease(myDeps->allObjects_Dep.guid,                                             __FILE__, __func__, __LINE__,    nanny, "allObjects");
         ADD_DEPENDENCE(      myDeps->allObjects_Dep.guid,       blockClone_Edt, blockClone_Deps_t, allObjects_Dep,       DB_MODE_RW,   nanny, "allObjects")
         ADD_DEPENDENCE(      NULL_GUID,                         blockClone_Edt, blockClone_Deps_t, allObjectsPred_Dep,   DB_MODE_NULL, nanny, "allObjectsPred")
         ADD_DEPENDENCE(      myDeps->upboundRequest_Dep.guid,   blockClone_Edt, blockClone_Deps_t, upboundRequest_Dep,   DB_MODE_RW,   nanny, "upboundRequest")
         EVT_DEPENDENCE(      myDeps->replyFromRequest_Dep.guid, blockClone_Edt, blockClone_Deps_t, replyFromRequest_Dep, DB_MODE_RW,   nanny, "replyFromRequest")
         myDeps->replyFromRequest_Dep.guid = NULL_GUID;
         gasket__ocrDbRelease(myDeps->meta_Dep.guid,                                                   __FILE__, __func__, __LINE__,    nanny, "meta");
         ADD_DEPENDENCE(      myDeps->meta_Dep.guid,             blockClone_Edt, blockClone_Deps_t, meta_Dep,             DB_MODE_RW,   nanny, "meta")

         break;
      }


      case Fork:
      {
         // Our only "Fork" is to do a block Refine operation.  It means we have to create EIGHT clone EDTs (i.e. a "fork" with a lot of prongs!), one for each of the 2x2x2 refined blocks.  We
         // also have to create a parent (to perform services for the child blocks, e.g. checksum, plot, profile.  And we have to create a collector EDT to fire when the previous nine finish, which will
         // be done if-and-when an unrefine operation happens.

         meta->cloningState.topPtrAdjRecOffset = ((int) ((unsigned long long) meta->cloningState.topPtrAdjRec) - ((unsigned long long) meta->cloningState.stack));

         ocrGuid_t parentInit_Edt;
         parentInit_Params_t parentInit_Params;
         parentInit_Params.parentInit_Template      = myParams->template.parentInit_Template;
         parentInit_Params.parentClone_Template     = myParams->template.parentClone_Template;
         parentInit_Params.allObjects_Dep_ToDestroy = myDeps->allObjects_Dep.guid;
         parentInit_Params.block_Dep_ToDestroy      = myDeps->block_Dep.guid;
#ifdef NANNY_ON_STEROIDS
         char nanny[NANNYLEN];
         sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
         gasket__ocrEdtCreate(&parentInit_Edt,
                              SLOT(parentInit_Deps_t, whoAmI_Dep),
                              myParams->template.parentInit_Template,
                              EDT_PARAM_DEF,
                              (u64 *) &parentInit_Params,
                              EDT_PARAM_DEF,
                              NULL,
                              EDT_PROP_NONE,
                              NULL_HINT,
                              NULL,
                              __FILE__,
                              __func__,
                              __LINE__,
                              nanny,
                              "parentInit");

         ocrGuid_t blockClone_Edt[8];

         // Create the events by which the first service request will be sent from the eight children prong block EDTs to the parent service provider EDT.  These events will only be triggered later
         // by the children prong clones, when they need a service.
         for (myParams->prongNum = 7; myParams->prongNum >= 0; myParams->prongNum--) {
#ifdef NANNY_ON_STEROIDS
            sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d  index=[%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, myParams->prongNum);
#else
#define nanny NULL
#endif
            gasket__ocrEventCreate(&myParams->conveyEighthBlockToJoin_Event[myParams->prongNum], OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG, __FILE__, __func__, __LINE__, nanny,"conveyEighthBlockToJoin_Event[...]");  // Each child will use its element to send blk for unrefine;
         }

         for (myParams->prongNum = 7; myParams->prongNum >= 0; myParams->prongNum--) {
#ifdef NANNY_ON_STEROIDS
            sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d  index=[%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, myParams->prongNum);
#else
#define nanny NULL
#endif
            gasket__ocrEventCreate(&myParams->conveyServiceRequestToParent_Event,                OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG, __FILE__, __func__, __LINE__, nanny,"conveyServiceRequestToParent_Event");
                                                                                                                                   // 000 use ALL to plumb to its clone.
            gasket__ocrEdtCreate(&blockClone_Edt[myParams->prongNum],       // Guid of the EDT created to continue at function blockContinuaiton_Func.
                                 SLOT(blockClone_Deps_t, whoAmI_Dep),
                                 myParams->template.blockClone_Template,    // Template for the EDT we are creating.
                                 EDT_PARAM_DEF,
                                 (u64 *) myParams,
                                 countof_blockClone_fixedDeps_t,
                                 NULL,
                                 EDT_PROP_NONE,
                                 NULL_HINT,
                                 NULL,
                                 __FILE__,
                                 __func__,
                                 __LINE__,
                                 nanny,
                                 "blockClone[...]");
            EVT_DEPENDENCE(myParams->conveyServiceRequestToParent_Event, parentInit_Edt, parentInit_Deps_t, serviceRequest_Dep[myParams->prongNum], DB_MODE_RO, nanny, "conveyServiceRequestToParent_Event")
         }

         // Provision the first clone of the parent service provider with all its other dependences (except do meta last).  Thereafter, the parent provisions its own clones.

         ocrGuid_t parentMeta_dblk;
         ocrGuid_t parentControl_dblk;
         void * dummy = NULL;
#ifdef NANNY_ON_STEROIDS
         sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#endif
         gasket__ocrDbCreate(&parentMeta_dblk,          &dummy, sizeof_ParentMeta_t,                                 __FILE__, __func__, __LINE__, nanny, "meta");
         gasket__ocrDbCreate(&parentControl_dblk,       &dummy, sizeof_Control_t,                                    __FILE__, __func__, __LINE__, nanny, "control");

         ADD_DEPENDENCE(parentMeta_dblk,                parentInit_Edt,    parentInit_Deps_t, meta_Dep,              DB_MODE_RW,                   nanny, "meta")
         ADD_DEPENDENCE(parentControl_dblk,             parentInit_Edt,    parentInit_Deps_t, control_Dep,           DB_MODE_RW,                   nanny, "control")
         ADD_DEPENDENCE(myDeps->control_Dep.guid,       parentInit_Edt,    parentInit_Deps_t, controlFromBlock_Dep,  DB_MODE_RO,                   nanny, "control --> controlFromBlock")
         ADD_DEPENDENCE(NULL_GUID,                      parentInit_Edt,    parentInit_Deps_t, upboundRequest_Dep,    DB_MODE_NULL,                 nanny, "meta")

         // Add dependencies to children prongs.

         // First, give all eight children prongs the datablocks that they will be responsible to copy anew (except postpone meta until last):

         gasket__ocrDbRelease(myDeps->control_Dep.guid,                                                              __FILE__, __func__, __LINE__, nanny, "control");
         gasket__ocrDbRelease(myDeps->block_Dep.guid,                                                                __FILE__, __func__, __LINE__, nanny, "block");
         gasket__ocrDbRelease(myDeps->allObjects_Dep.guid,                                                           __FILE__, __func__, __LINE__, nanny, "allObjects");
         for (i = 0; i < 8; i++) {
            ADD_DEPENDENCE(myDeps->control_Dep.guid,    blockClone_Edt[i], blockClone_Deps_t, controlPred_Dep,       DB_MODE_RO,                   nanny, "control --> controlPred")
            ADD_DEPENDENCE(myDeps->block_Dep.guid,      blockClone_Edt[i], blockClone_Deps_t, blockPred_Dep,         DB_MODE_RO,                   nanny, "block --> blockPred")
            ADD_DEPENDENCE(myDeps->allObjects_Dep.guid, blockClone_Edt[i], blockClone_Deps_t, allObjectsPred_Dep,    DB_MODE_RO,                   nanny, "allObjects --> allObjectsPred")
            ADD_DEPENDENCE(NULL_GUID,                   blockClone_Edt[i], blockClone_Deps_t, replyFromRequest_Dep,  DB_MODE_NULL,                 nanny, "replyFromRequest")
         }

         // Now create new datablocks for the eight chidren prongs to populate anew:

         for (i = 0; i < 8; i++) {
            ocrGuid_t newControl_dblk;
            ocrGuid_t newBlock_dblk;
            ocrGuid_t newAllObjects_dblk;
            gasket__ocrDbCreate(&newControl_dblk,       &dummy,                               sizeof_Control_t,      __FILE__, __func__, __LINE__, nanny, "control");
            gasket__ocrDbCreate(&newBlock_dblk,         &dummy,                               sizeof_Block_t,        __FILE__, __func__, __LINE__, nanny, "block");
            gasket__ocrDbCreate(&newAllObjects_dblk,    &dummy,                               sizeof_AllObjects_t,   __FILE__, __func__, __LINE__, nanny, "allObjects");
            ADD_DEPENDENCE(newControl_dblk,             blockClone_Edt[i], blockClone_Deps_t, control_Dep,           DB_MODE_RW,                   nanny, "control")
            ADD_DEPENDENCE(newBlock_dblk,               blockClone_Edt[i], blockClone_Deps_t, block_Dep,             DB_MODE_RW,                   nanny, "block")
            ADD_DEPENDENCE(newAllObjects_dblk,          blockClone_Edt[i], blockClone_Deps_t, allObjects_Dep,        DB_MODE_RW,                   nanny, "allObjects")
         }

         for (i = 0; i < 8; i++) {
            ADD_DEPENDENCE(NULL_GUID,                   blockClone_Edt[i], blockClone_Deps_t, upboundRequest_Dep,    DB_MODE_NULL,                 nanny, "upboundRequest")
         }

         // Now finish things off by plumbing the meta to the eight prongs, AND to the parent:

         gasket__ocrDbRelease(myDeps->meta_Dep.guid,                                                                 __FILE__, __func__, __LINE__, nanny, "meta");
         for (i = 0; i < 8; i++) {
            ocrGuid_t newMeta_dblk;
            gasket__ocrDbCreate(&newMeta_dblk,          &dummy,                               sizeof_BlockMeta_t,    __FILE__, __func__, __LINE__, nanny, "meta");
            ADD_DEPENDENCE(myDeps->meta_Dep.guid,       blockClone_Edt[i], blockClone_Deps_t, metaPred_Dep,          DB_MODE_RO,                   nanny, "meta --> metaPred")
            ADD_DEPENDENCE(newMeta_dblk,                blockClone_Edt[i], blockClone_Deps_t, meta_Dep,              DB_MODE_RW,                   nanny, "meta")
         }

         ADD_DEPENDENCE(myDeps->meta_Dep.guid,          parentInit_Edt,    parentInit_Deps_t, blockMeta_Dep,         DB_MODE_RO,                   nanny, "meta --> blockMeta")

         break;
      }


      case Join:
      {
         // Our only "Join" is to do a block "Unrefine" operation.  It means we have to coallesce this EDT's block back into the parent level.
         // The code that started the Unrefine, and crawled up the call stack to this point, has already cataloged all the datablock that will be needed to fulfill the dependence of the clone.  Therefore,
         // we can simply drop into the same code that equips our dependences to the clone as applies to the other simple cases.

         int myProngNum = ((meta->xPos & 1)) + ((meta->yPos & 1) << 1) + ((meta->zPos & 1) << 2);
#ifdef NANNY_ON_STEROIDS
         char nanny[NANNYLEN];
         sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d myProngNum=%d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, myProngNum);
#else
#define nanny NULL
#endif
         Block_t * block = myDeps->block_Dep.ptr;
         if (myProngNum == 7) {
            block->neighborRefinementLevel_East   = meta->neighborRefinementLevel[0][1];   // Convey refinement level of East neighbor of prong111 to the aggregation of an unrefine (i.e. join).
            block->neighborRefinementLevel_North  = meta->neighborRefinementLevel[1][1];   //                            North
            block->neighborRefinementLevel_Up     = meta->neighborRefinementLevel[2][1];   //                            Upward
         }
         block->finerNeighborNotUnrefiningAndNotTalkingAnyMore = meta->finerNeighborNotUnrefiningAndNotTalkingAnyMore;   // Bridge this to the join operation.
         gasket__ocrDbRelease(myDeps->block_Dep.guid, __FILE__, __func__, __LINE__, nanny, "block");
         gasket__ocrEventSatisfy(meta->conveyEighthBlockToJoin_Event[myProngNum], myDeps->block_Dep.guid, __FILE__, __func__, __LINE__, nanny, "block via conveyEighthBlockToJoin[...]");            // Feed my block into the proper eighth-block of the join-clone.
         myDeps->block_Dep.guid = NULL_GUID; myDeps->block_Dep.ptr = NULL;

         // Clean up events no longer needed.

         int axis, pole, qrtr;
         for (axis = 0; axis < 3; axis++) {
            for (pole = 0; pole < 2; pole++) {
               for (qrtr = 0; qrtr < 4; qrtr++) {
                  if (!ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][qrtr])) {
#ifdef NANNY_ON_STEROIDS
                     char nanny[NANNYLEN];
                     sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, [axis=%d][pole=%d][qrtr=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, qrtr);
#else
#define nanny NULL
#endif
                     gasket__ocrEventAbandon(&meta->conveyFaceToNeighbor_Event[axis][pole][qrtr], __FILE__, __func__, __LINE__, nanny, "conveyFaceToNeighbor[...][...][...]");
                  }
               }
            }
         }

         if (myProngNum != 0) {
            // Satisfy dependences of parent, agreeing with prong000 that the parent needs to send its parentMeta datablock to the join-clone ... and die.

            DbCommHeader_t * commHdr = myDeps->upboundRequest_Dep.ptr;
            commHdr->serviceOpcode               = Operation_Join;
            commHdr->timeStep                    = -9999;
            commHdr->atBat_Event                 = meta->conveyServiceRequestToParent_Event;               // Convey At Bat Event to parent so that he can destroy the event.
            commHdr->parentMetaToJoinClone_Event = NULL_GUID;                                              // Only prong000 sends a relevant guid through this.
            gasket__ocrDbRelease(myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "upboundRequest(join)");
            gasket__ocrEventSatisfy(meta->conveyServiceRequestToParent_Event, myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "upboundRequest(join) via conveyServiceRequestToParent");  // Satisfy the parent's serviceRequest datablock dependence.
            myDeps->upboundRequest_Dep.guid      = NULL_GUID;

            // Clean up datablocks no longer needed.

            gasket__ocrDbDestroy(&myDeps->control_Dep.guid,    &myDeps->control_Dep.ptr,    __FILE__, __func__, __LINE__, nanny, "control");
            gasket__ocrDbDestroy(&myDeps->allObjects_Dep.guid, &myDeps->allObjects_Dep.ptr, __FILE__, __func__, __LINE__, nanny, "allObjects");
            gasket__ocrDbDestroy(&myDeps->meta_Dep.guid,       &myDeps->meta_Dep.ptr,       __FILE__, __func__, __LINE__, nanny, "meta");

            REPORT_EDT_DEMISE(myDeps->whoAmI_Dep);
   // Defeat OCR's habit of releasing the input ocrEdtDep_t's, because we have changed many of them to other guids, including some events, which simply must not be released.  This is just a debugging expedient.
            if (1) {
               myDeps->meta_Dep.guid                = NULL_GUID;
               myDeps->whoAmI_Dep.guid              = NULL_GUID;
               myDeps->metaPred_Dep.guid            = NULL_GUID;
               myDeps->control_Dep.guid             = NULL_GUID;
               myDeps->controlPred_Dep.guid         = NULL_GUID;
               myDeps->allObjects_Dep.guid          = NULL_GUID;
               myDeps->allObjectsPred_Dep.guid      = NULL_GUID;
               myDeps->upboundRequest_Dep.guid      = NULL_GUID;
               myDeps->replyFromRequest_Dep.guid    = NULL_GUID;
               int i;
               for (i=0; i < depc-countof_blockClone_fixedDeps_t; i++) myDeps->aux_Dep[i].guid = NULL_GUID;
            }
            return NULL_GUID;    // Seven of the eight prongs just die, after plumbing their datablock to the join-clone of the 000 sibling and telling the parent to do its part of the join operation.
         }

         // Join-clone case.  This is the 000 prong; the other seven of the eight prongs just died, above.  Create a clone EDT, cause it to fire, and cause this EDT has to terminate.

         meta->cloningState.topPtrAdjRecOffset = ((int) ((unsigned long long) meta->cloningState.topPtrAdjRec) - ((unsigned long long) meta->cloningState.stack));

         ocrGuid_t blockClone_Edt;
#ifdef NANNY_ON_STEROIDS
         sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
         gasket__ocrEventCreate(&myParams->parentMetaToJoinClone_Event, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG,     __FILE__, __func__, __LINE__, nanny, "parentMetaToJoinClone");
         gasket__ocrEdtCreate(&blockClone_Edt,                           // Guid of the EDT created to continue at function blockContinuaiton_Func.
                              SLOT(blockClone_Deps_t, whoAmI_Dep),
                              myParams->template.blockClone_Template,    // Template for the EDT we are creating.
                              EDT_PARAM_DEF,
                              (u64 *) myParams,
                              countof_blockClone_fixedDeps_t + 8,
                              NULL,
                              EDT_PROP_NONE,
                              NULL_HINT,
                              NULL,
                              __FILE__,
                              __func__,
                              __LINE__,
                              nanny,
                              "blockClone (Join 000)");

         ocrGuid_t            joinBlock_dblk;
         ocrGuid_t            newMeta_dblk;
         ocrGuid_t            newControl_dblk;
         ocrGuid_t            newAllObjects_dblk;
         void               * dummy = NULL;
         gasket__ocrDbCreate(    &joinBlock_dblk,        &dummy,                          sizeof_Block_t,            __FILE__, __func__, __LINE__, nanny, "join block");
         gasket__ocrDbCreate(    &newMeta_dblk,          &dummy,                          sizeof_BlockMeta_t,        __FILE__, __func__, __LINE__, nanny, "meta");
         gasket__ocrDbCreate(    &newControl_dblk,       &dummy,                          sizeof_Control_t,          __FILE__, __func__, __LINE__, nanny, "control");
         gasket__ocrDbCreate(    &newAllObjects_dblk,    &dummy,                          sizeof_AllObjects_t,       __FILE__, __func__, __LINE__, nanny, "allObjects");

         gasket__ocrDbRelease(    myDeps->control_Dep.guid,                                                          __FILE__, __func__, __LINE__, nanny, "control");
         ADD_DEPENDENCE(          newControl_dblk,                        blockClone_Edt, blockClone_Deps_t, control_Dep,            DB_MODE_RW,   nanny, "control")
         ADD_DEPENDENCE(          myDeps->control_Dep.guid,               blockClone_Edt, blockClone_Deps_t, controlPred_Dep,        DB_MODE_RO,   nanny, "controlPred")
         ADD_DEPENDENCE(          joinBlock_dblk,                         blockClone_Edt, blockClone_Deps_t, block_Dep,              DB_MODE_RW,   nanny, "join block")
         ADD_DEPENDENCE(          NULL_GUID,                              blockClone_Edt, blockClone_Deps_t, blockPred_Dep,          DB_MODE_NULL, nanny, "blockPred")
         gasket__ocrDbRelease(    myDeps->allObjects_Dep.guid,                                                       __FILE__, __func__, __LINE__, nanny, "allObjects");
         ADD_DEPENDENCE(          newAllObjects_dblk,                     blockClone_Edt, blockClone_Deps_t, allObjects_Dep,         DB_MODE_RW,   nanny, "allObjects")
         ADD_DEPENDENCE(          myDeps->allObjects_Dep.guid,            blockClone_Edt, blockClone_Deps_t, allObjectsPred_Dep,     DB_MODE_RO,   nanny, "allObjectsPred")
         ADD_DEPENDENCE(          NULL_GUID,                              blockClone_Edt, blockClone_Deps_t, replyFromRequest_Dep,   DB_MODE_NULL, nanny, "replyFromRequest")

         EVT_DEPENDENCE(myParams->parentMetaToJoinClone_Event,            blockClone_Edt, blockClone_Deps_t, parentMeta_Dep,         DB_MODE_RO,   nanny, "parentMeta")

         EVT_DEPENDENCE(          meta->conveyEighthBlockToJoin_Event[0], blockClone_Edt, blockClone_Deps_t, aux_Dep[0],             DB_MODE_RO,   nanny, "joinBlock_Dep[0][0][0]")
         EVT_DEPENDENCE(          meta->conveyEighthBlockToJoin_Event[1], blockClone_Edt, blockClone_Deps_t, aux_Dep[1],             DB_MODE_RO,   nanny, "joinBlock_Dep[0][0][1]")
         EVT_DEPENDENCE(          meta->conveyEighthBlockToJoin_Event[2], blockClone_Edt, blockClone_Deps_t, aux_Dep[2],             DB_MODE_RO,   nanny, "joinBlock_Dep[0][1][0]")
         EVT_DEPENDENCE(          meta->conveyEighthBlockToJoin_Event[3], blockClone_Edt, blockClone_Deps_t, aux_Dep[3],             DB_MODE_RO,   nanny, "joinBlock_Dep[0][1][1]")
         EVT_DEPENDENCE(          meta->conveyEighthBlockToJoin_Event[4], blockClone_Edt, blockClone_Deps_t, aux_Dep[4],             DB_MODE_RO,   nanny, "joinBlock_Dep[1][0][0]")
         EVT_DEPENDENCE(          meta->conveyEighthBlockToJoin_Event[5], blockClone_Edt, blockClone_Deps_t, aux_Dep[5],             DB_MODE_RO,   nanny, "joinBlock_Dep[1][0][1]")
         EVT_DEPENDENCE(          meta->conveyEighthBlockToJoin_Event[6], blockClone_Edt, blockClone_Deps_t, aux_Dep[6],             DB_MODE_RO,   nanny, "joinBlock_Dep[1][1][0]")
         EVT_DEPENDENCE(          meta->conveyEighthBlockToJoin_Event[7], blockClone_Edt, blockClone_Deps_t, aux_Dep[7],             DB_MODE_RO,   nanny, "joinBlock_Dep[1][1][1]")

         // Tell the parent to do its part for the join operation ... and die.

         DbCommHeader_t * commHdr = myDeps->upboundRequest_Dep.ptr;
         commHdr->serviceOpcode               = Operation_Join;
         commHdr->timeStep                    = -9999;
         commHdr->atBat_Event                 = meta->conveyServiceRequestToParent_Event;               // Convey At Bat Event to parent so that he can destroy the event.
         commHdr->parentMetaToJoinClone_Event = myParams->parentMetaToJoinClone_Event;                  // Tell parent how to send its parentMeta datablock to the join-clone.
         gasket__ocrDbRelease(myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "upboundRequest(join)");
         gasket__ocrEventSatisfy(meta->conveyServiceRequestToParent_Event, myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "upboundRequest(join) via conveyServiceRequestToParent");  // Satisfy the parent's serviceRequest datablock dependence.
         myDeps->upboundRequest_Dep.guid      = NULL_GUID;

         // Join-clone will set its blockMeta data to some of the material from this EDT, and some from the parent that it stashed when the block was originally forked (refiend).  New storage is allocated
         // for the clone's blockMeta, to improve the odds that it will be collocated closer to the policy domain of that EDT.  We plumb our meta_Dep to clone's metaPred_Dep, supply a new datablock into
         // the clone's meta_Dep; and the parent plumbs its (parentMeta_t) meta into the clone's parentMeta_Dep (which aliases with the slot for upboundRequest_Dep, for other kinds of clones).

         gasket__ocrDbRelease(myDeps->meta_Dep.guid,                                                           __FILE__, __func__, __LINE__,  nanny, "meta");
         ADD_DEPENDENCE(      newMeta_dblk,                           blockClone_Edt, blockClone_Deps_t, meta_Dep,           DB_MODE_RW,   nanny, "meta")
         ADD_DEPENDENCE(      myDeps->meta_Dep.guid,                  blockClone_Edt, blockClone_Deps_t, metaPred_Dep,       DB_MODE_RO,   nanny, "meta --> metaPred")

         break;
      }

      case Special:
      {
         // Simple single continuation case, but for a special case, namely the exchange of datablocks related to refinement disposition.  The special parts of this are
         // that it needs more dependences than all the other cases; and that some of the dependences are for RO access, others for RW.  The special parts are performed
         // by the code that invoked this case.  TODO:  This can probably just use the Seasoning... case, above.  Caveat is that blockClone_Edt MUST have been created (by the code requesting the clone).

         meta->cloningState.topPtrAdjRecOffset = ((int) ((unsigned long long) meta->cloningState.topPtrAdjRec) - ((unsigned long long) meta->cloningState.stack));

         ocrGuid_t blockClone_Edt = meta->blockClone_Edt;
#ifdef NANNY_ON_STEROIDS
         char nanny[NANNYLEN];
         sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif

         ADD_DEPENDENCE(      NULL_GUID,                       blockClone_Edt, blockClone_Deps_t, metaPred_Dep,         DB_MODE_NULL, nanny, "metaPred")
         gasket__ocrDbRelease(myDeps->control_Dep.guid,                                              __FILE__, __func__, __LINE__,    nanny, "control");
         ADD_DEPENDENCE(      myDeps->control_Dep.guid,        blockClone_Edt, blockClone_Deps_t, control_Dep,          DB_MODE_RO,   nanny, "control")
         ADD_DEPENDENCE(      NULL_GUID,                       blockClone_Edt, blockClone_Deps_t, controlPred_Dep,      DB_MODE_NULL, nanny, "controlPred")
         gasket__ocrDbRelease(myDeps->block_Dep.guid,                                                __FILE__, __func__, __LINE__,    nanny, "block");
         ADD_DEPENDENCE(      myDeps->block_Dep.guid,          blockClone_Edt, blockClone_Deps_t, block_Dep,            DB_MODE_RW,   nanny, "block")
         ADD_DEPENDENCE(      NULL_GUID,                       blockClone_Edt, blockClone_Deps_t, blockPred_Dep,        DB_MODE_NULL, nanny, "blockPred")
         gasket__ocrDbRelease(myDeps->allObjects_Dep.guid,                                           __FILE__, __func__, __LINE__,    nanny, "allObjects");
         ADD_DEPENDENCE(      myDeps->allObjects_Dep.guid,     blockClone_Edt, blockClone_Deps_t, allObjects_Dep,       DB_MODE_RW,   nanny, "allObjects")
         ADD_DEPENDENCE(      NULL_GUID,                       blockClone_Edt, blockClone_Deps_t, allObjectsPred_Dep,   DB_MODE_NULL, nanny, "allObjectsPred")
         ADD_DEPENDENCE(      NULL_GUID,                       blockClone_Edt, blockClone_Deps_t, upboundRequest_Dep,   DB_MODE_NULL, nanny, "upboundRequest")
         ADD_DEPENDENCE(      NULL_GUID,                       blockClone_Edt, blockClone_Deps_t, replyFromRequest_Dep, DB_MODE_NULL, nanny, "replyFromRequest")
         gasket__ocrDbRelease(myDeps->meta_Dep.guid,                                                 __FILE__, __func__, __LINE__,    nanny, "meta");
         ADD_DEPENDENCE(      myDeps->meta_Dep.guid,           blockClone_Edt, blockClone_Deps_t, meta_Dep,             DB_MODE_RW,   nanny, "meta")

         break;
      }


      case OpcodeUnspecified:
      {
         printf ("Cloning opcode unspecified\n"); fflush(stdout);
         *((int *) 0) = 123;
         ocrShutdown();
         break;
      }

      default:
      {
         printf ("Cloning opcode not yet handled\n"); fflush(stdout);
         *((int *) 0) = 123;
         ocrShutdown();
         break;
      }
      }

   } else {  // (topOfStack[1].resumption_case_num != 0)    If that was the very last iteration.  SoupToNuts did the clean-up, so just die.
      int myProngNum = ((meta->xPos & 1)) + ((meta->yPos & 1) << 1) + ((meta->zPos & 1) << 2);
      if ((!ocrGuidIsNull(meta->conveyEighthBlockToJoin_Event[myProngNum]))) {
         gasket__ocrEventAbandon(&meta->conveyEighthBlockToJoin_Event[myProngNum], __FILE__, __func__, __LINE__, nanny, "conveyEighthBlockToJoin[...]");
      }
      gasket__ocrDbDestroy(&myDeps->meta_Dep.guid,       &myDeps->meta_Dep.ptr,       __FILE__, __func__, __LINE__, nanny, "meta");
   }
   REPORT_EDT_DEMISE(myDeps->whoAmI_Dep);
   // Defeat OCR's habit of releasing the input ocrEdtDep_t's, because we have changed many of them to other guids, including some events, which simply must not be released.  This is just a debugging expedient.
   if (0) {
      myDeps->meta_Dep.guid                = NULL_GUID;
      myDeps->whoAmI_Dep.guid              = NULL_GUID;
      myDeps->metaPred_Dep.guid            = NULL_GUID;
      myDeps->control_Dep.guid             = NULL_GUID;
      myDeps->controlPred_Dep.guid         = NULL_GUID;
      myDeps->allObjects_Dep.guid          = NULL_GUID;
      myDeps->allObjectsPred_Dep.guid      = NULL_GUID;
      myDeps->upboundRequest_Dep.guid      = NULL_GUID;
      myDeps->replyFromRequest_Dep.guid    = NULL_GUID;
      int i;
      for (i=0; i < depc-countof_blockClone_fixedDeps_t; i++) myDeps->aux_Dep[i].guid = NULL_GUID;
   }
   return NULL_GUID;

} // blockClone_Func


// **************************************************************************************************************************************************************************************************************
// blockClone_SoupToNuts
//
// This is the top-level function in the calling topology of the several functions that do the actual processing of a block.
//
void blockClone_SoupToNuts (blockClone_Params_t * myParams, u32 depc, ocrEdtDep_t depv[]) {

   typedef struct {
      Frame_Header_t myFrame;
      double t1, t2, t3, t4;
      int ts, var, start, number, stage, comm_stage;
      struct {
         Control_t * control;
//PROFILE:         Profile_t * profile;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__blockClone_SoupToNuts_t;

#define t1         (lcl->t1)
#define t2         (lcl->t2)
#define t3         (lcl->t3)
#define t4         (lcl->t4)
#define ts         (lcl->ts)
#define var        (lcl->var)
#define start      (lcl->start)
#define number     (lcl->number)
#define stage      (lcl->stage)
#define comm_stage (lcl->comm_stage)
#define control    (lcl->pointers.control)
//PROFILE:#define profile    (lcl->pointers.profile)

   blockClone_Deps_t * myDeps = (blockClone_Deps_t *)   depv;
   BlockMeta_t  * meta                                            = (BlockMeta_t *) myDeps->meta_Dep.ptr;
   DbSize_t     * dbSize                                          = meta->dbSize;

//printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d, clone=%4d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum); fflush(stdout);
   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__blockClone_SoupToNuts_t)
   control = myDeps->control_Dep.ptr;
//PROFILE:   profile = meta->profileDb.base;

   if (meta->refinementLevel == 0) {
      // Initialize the unrefined mesh block to random cell contents.
      CALL_SUSPENDABLE_CALLEE
      init(myParams, depv);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
   }
   CALL_SUSPENDABLE_CALLEE
   checksum_BlockContribution (myParams, depv, 0);
   DEBRIEF_SUSPENDABLE_FUNCTION(;)

//PROFILE:   init_profile(meta);    // Initialize the performance metrics information for this block.

   t1 = timer();

   if (control->num_refine != 0 || control->uniform_refine) {
      CALL_SUSPENDABLE_CALLEE
      refine(myParams, depv, 0);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
//printf (">>>> %s line %d, lvl=%d, pos=%d,%d,%d, neiLvl=%d,%d,%d,%d,%d,%d\n", __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->neighborRefinementLevel[0][0],meta->neighborRefinementLevel[0][1],meta->neighborRefinementLevel[1][0],meta->neighborRefinementLevel[1][1],meta->neighborRefinementLevel[2][0],meta->neighborRefinementLevel[2][1]); fflush(stdout);
   }

   t2 = timer();
//PROFILE:   profile->timer_refine_all += t2 - t1;

   if (control->plot_freq) {
      CALL_SUSPENDABLE_CALLEE
      plot_BlockContribution(myParams, depv, 0);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
   }
   t3 = timer();
//PROFILE:   profile->timer_plot += t3 - t2;

//===
//TODO:   nb_min = nb_max = global_active;

   for (comm_stage = 0, ts = 1; ts <= control->num_tsteps; ts++) {
      for (stage = 0; stage < control->stages_per_ts; stage++, comm_stage++) {


//printf ("======= %s line %d, ts=%d, comm_stage=%d, stage=%d, lvl=%d, pos=%d,%d,%d\n", __func__, __LINE__, ts, comm_stage, stage, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);
//TODO:         total_blocks += global_active;
//TODO:         if (global_active < nb_min)
//TODO:            nb_min = global_active;
//TODO:         if (global_active > nb_max)
//TODO:            nb_max = global_active;
         for (start = 0; start < control->num_vars; start += control->comm_vars) {
            number = ((start + control->comm_vars) > control->num_vars) ? (control->num_vars - start) : control->comm_vars;
            t3 = timer();
//printf ("comm_stage = %d, ts = %d, num_tsteps = %d, stage = %d, stages_per_ts = %d, start = %d, num_vars = %d, comm_vars = %d\n",
//comm_stage, ts, control->num_tsteps, stage, control->stages_per_ts, start, control->num_vars, control->comm_vars); fflush(stdout);
            CALL_SUSPENDABLE_CALLEE
            comm (myParams, depv, start, number, comm_stage);
            DEBRIEF_SUSPENDABLE_FUNCTION(;)
            t4 = timer();
//PROFILE:            profile->timer_comm_all += t4 - t3;
            for (var = start; var < (start+number); var++) {
               stencil_calc(depc, depv, var);
               t3 = timer();
//PROFILE:               profile->timer_calc_all += t3 - t4;
            }
         }

         // Reference code does the following if-stmt inside the above for-loop, i.e. does checksums comm_vars at a time.  OCR version does all at once.
         if (control->checksum_freq && !(stage%control->checksum_freq)) {
            t3 = timer();
            CALL_SUSPENDABLE_CALLEE
            checksum_BlockContribution (myParams, depv, ts);
            DEBRIEF_SUSPENDABLE_FUNCTION(;)
            t4 = timer();
//PROFILE:            profile->timer_cs_all += t4 - t3;
         }
      }
//printf ("======= %s line %d, ts=%d, comm_stage=%d, stage=%d, lvl=%d, pos=%d,%d,%d\n", __func__, __LINE__, ts, comm_stage, stage, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);

      if (control->num_refine && !control->uniform_refine) {
         move(depv);
         if (!(ts%control->refine_freq)) {
            CALL_SUSPENDABLE_CALLEE
            refine(myParams, depv, ts);
            DEBRIEF_SUSPENDABLE_FUNCTION(;)
//printf (">>>> %s line %d, lvl=%d, pos=%d,%d,%d, neiLvl=%d,%d,%d,%d,%d,%d\n", __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->neighborRefinementLevel[0][0],meta->neighborRefinementLevel[0][1],meta->neighborRefinementLevel[1][0],meta->neighborRefinementLevel[1][1],meta->neighborRefinementLevel[2][0],meta->neighborRefinementLevel[2][1]); fflush(stdout);
         }
      }
//printf ("======= %s line %d, ts=%d, comm_stage=%d, stage=%d, lvl=%d, pos=%d,%d,%d\n", __func__, __LINE__, ts, comm_stage, stage, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);
      t2 = timer();
//PROFILE:      profile->timer_refine_all += t2 - t4;

      t3 = timer();
      if (control->plot_freq && ((ts % control->plot_freq) == 0)) {
         CALL_SUSPENDABLE_CALLEE
         plot_BlockContribution(myParams, depv, ts);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
      }
//PROFILE:      profile->timer_plot += timer() - t3;
   }
//printf ("======= %s line %d, lvl=%d, pos=%d,%d,%d\n", __func__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos); fflush(stdout);
//PROFILE:   profile->timer_all = timer() - t1;


   // We are done iterating the time steps.  Finishing the algorithm does NOT necessarily require the refined blocks to have been coalesced back into their unrefined parents.  No real need to do all that,
   // even though it implies we will be leaving a lot of dead datablocks lying around.  (One reason to want to clean those up by performing the unrefine operations is to make it easier to find real
   // memory leaks -- the kind that could cause the whole algorithm to choke on longer runs.  But I set that aside, assuming that it is easy enough to contrive to make all the blocks unrefine anyway,
   // by moving all the objects out of the mesh's spatial domain so that they unrefine "naturally".  Then a trip through valgrind would find those pesky leaks.)
   //
   // Just pass our profile datablock upwards, and then a shutdown service request. We can then end this EDT.
   //

//PROFILE:   ocrGuid_t conveyOperandUpward_Event  = meta->conveyOperandUpward_Event;                                     // Jot down the At Bat Event.
//PROFILE:   profile->dbCommHeader.serviceOpcode  = Operation_Profile;
//PROFILE:   profile->dbCommHeader.timeStep       = ts;
//PROFILE:   profile->dbCommHeader.atBat_Event    = conveyOperandUpward_Event;                                           // Convey At Bat Event to parent so that he can destroy the event.
//PROFILE:   ocrEventCreate(                       &meta->conveyOperandUpward_Event, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);  // Create the On Deck Event; record it in our meta.
//PROFILE:   profile->dbCommHeader.onDeck_Event   = meta->conveyOperandUpward_Event;                                           // Convey On Deck Event to parent so that he can make his clone depend upon it.
//PROFILE:   ocrDbRelease(meta->profileDb.dblk);
//PROFILE:   ocrEventSatisfy(conveyOperandUpward_Event, meta->profileDb.dblk);                                           // Satisfy the parent's operand1 dependence.
//PROFILE:   meta->profileDb.base = profile = NULL;
//PROFILE:   meta->profileDb.dblk = NULL_GUID;
//PROFILE:   meta->profileDb.size = -9999;
//PROFILE:   meta->profileDb.acMd = DB_MODE_NULL;

#ifdef NANNY_ON_STEROIDS
   char nanny[NANNYLEN];
   sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
   gasket__ocrDbCreate(&myDeps->upboundRequest_Dep.guid, (void **) &myDeps->upboundRequest_Dep.ptr, sizeof_DbCommHeader_t,   __FILE__, __func__, __LINE__, nanny, "upboundRequest");
   DbSize(myDeps->upboundRequest_Dep) = sizeof_DbCommHeader_t;
   meta->cloningState.cloningOpcode = SeasoningOneOrMoreDbCreates;
   SUSPEND__RESUME_IN_CLONE_EDT(;)
   DbCommHeader_t * dbCommHeader = (DbCommHeader_t *) myDeps->upboundRequest_Dep.ptr;
   ocrGuid_t conveyServiceRequestToParent_Event= meta->conveyServiceRequestToParent_Event;             // Jot down the At Bat Event.
   dbCommHeader->serviceOpcode = Operation_Shutdown;
   dbCommHeader->timeStep      = ts;
   dbCommHeader->atBat_Event   = conveyServiceRequestToParent_Event;                                   // Convey At Bat Event to parent so that he can destroy the event.
   meta->conveyServiceRequestToParent_Event = NULL_GUID;                                               // Cease creating an On Deck Event for the next time;  there is no next time.
   dbCommHeader->onDeck_Event               = NULL_GUID;                                               // Convey that NULL_GUID to parent, too.
#ifdef NANNY_ON_STEROIDS
   sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif

   // Before we finish sending the service request up the line, let's do our own clean-up.
   // Reset the pathway between this block and its neighbors.  It will be reestablished (by way of labeled guid lookup) the first time we try to communicate with the (new) neighbor.

   int axis, pole, qrtr;
   for (axis = 0; axis < 3; axis++) {
      for (pole = 0; pole < 2; pole++) {
         for (qrtr = 0; qrtr < 4; qrtr++) {
            if (!ocrGuidIsNull(meta->conveyFaceToNeighbor_Event[axis][pole][qrtr])) {
#ifdef NANNY_ON_STEROIDS
               sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, [axis=%d][pole=%d][qrtr=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, axis, pole, qrtr);
#else
#define nanny NULL
#endif
               gasket__ocrEventAbandon(&meta->conveyFaceToNeighbor_Event[axis][pole][qrtr], __FILE__, __func__, __LINE__, nanny, "conveyFaceToNeighbor[...][...][...]");
            }
            meta->onDeckToReceiveFace_Event [axis][pole][qrtr] = NULL_GUID;
         }
      }
   }

   int myProngNum = ((meta->xPos & 1)) + ((meta->yPos & 1) << 1) + ((meta->zPos & 1) << 2);
   if ((!ocrGuidIsNull(meta->conveyEighthBlockToJoin_Event[myProngNum]))) {
      gasket__ocrEventAbandon(&meta->conveyEighthBlockToJoin_Event[myProngNum], __FILE__, __func__, __LINE__, nanny, "conveyEighthBlockToJoin[...]");
   }

   gasket__ocrDbDestroy(&myDeps->block_Dep.guid,      &myDeps->block_Dep.ptr,      __FILE__, __func__, __LINE__, nanny, "block");
   gasket__ocrDbDestroy(&myDeps->allObjects_Dep.guid, &myDeps->allObjects_Dep.ptr, __FILE__, __func__, __LINE__, nanny, "allObjects");
   gasket__ocrDbDestroy(&myDeps->control_Dep.guid,    &myDeps->control_Dep.ptr,    __FILE__, __func__, __LINE__, nanny, "control");
   // MOVED TO CALLER, BECAUSE CLONING STACK IS IN META!  gasket__ocrDbDestroy(&myDeps->meta_Dep.guid,       &myDeps->meta_Dep.ptr,       __FILE__, __func__, __LINE__, nanny, "meta");

   // Okay.  Finish sending the shutdown request up the line.

   gasket__ocrDbRelease(myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "upboundRequest(Shutdown)");
   gasket__ocrEventSatisfy(conveyServiceRequestToParent_Event, myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "upboundRequest(Shutdown) via conveyServiceRequestToParent");  // Satisfy the parent's serviceRequest datablock dependence.
   myDeps->upboundRequest_Dep.ptr  = NULL;
   myDeps->upboundRequest_Dep.guid = NULL_GUID;

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)    // Top-level will sense normal completion, and just end this EDT (without creating any more clones).
   SUSPENDABLE_FUNCTION_EPILOGUE
#undef t1
#undef t2
#undef t3
#undef t4
#undef ts
#undef var
#undef start
#undef number
#undef stage
#undef comm_stage
#undef control
//PROFILE:#undef profile

} // blockClone_SoupToNuts
