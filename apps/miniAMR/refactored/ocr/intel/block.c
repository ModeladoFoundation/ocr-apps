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
//#include <mpi.h> TODO

#include <ocr.h>
#include <ocr-std.h>
//#include <extensions/ocr-labeling.h>
#include "ocr-macros_brn.h"

#include "control.h"
#include "meta.h"
#include "object.h"
#include "checksum.h"
#include "block.h"
#include "profile.h"
#include "proto.h"
#include "continuationcloner.h"
#if 0
#include "block.h"
#include "comm.h"
#include "timer.h"
#endif

// block.c         This is a block of the mesh, at some refinement level.
// ***************************************************************************************************************************************************************************************
// ***************************************************************************************************************************************************************************************
// ***************************************************************************************************************************************************************************************
// blockLaunch_Func
//
// This just creates the blockInit EDT and the datablocks that it will need to initialize:  control_dblk, allObjects_dblk, and meta_dblk.  It then passes control to the blockInit EDT.
ocrGuid_t blockLaunch_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

   blockLaunch_Deps_t   * myDeps   = (blockLaunch_Deps_t *)   depv;
   blockLaunch_Params_t * myParams = (blockLaunch_Params_t *) paramv;
   ocrGuid_t          initialControl_dblk    =                (myDeps->control_Dep.guid);
   Control_t  const * initialControl         = ((Control_t *) (myDeps->control_Dep.ptr));
   ocrGuid_t          initialAllObjects_dblk =                (myDeps->allObjects_Dep.guid);
   ocrGuid_t          initialBlock_dblk      =                (myDeps->block_Dep.guid);
printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, myParams->refinementLevel, myParams->xPos, myParams->yPos, myParams->zPos); fflush(stdout);

   ocrGuid_t    meta_dblk;
   ocrGuid_t    control_dblk;
   ocrGuid_t    allObjects_dblk;
   ocrGuid_t    block_dblk;
   void * dummy;
   gasket__ocrDbCreate(&meta_dblk,         &dummy, sizeof_BlockMeta_t,  __FILE__, __func__, __LINE__);
   gasket__ocrDbCreate(&control_dblk,      &dummy, sizeof_Control_t,    __FILE__, __func__, __LINE__);
#define control initialControl  // In all other contexts, the control_t datablock (which contains num_objects) will be available, but here, it is in initialControl_dblk, so we hack a look-alike.
   gasket__ocrDbCreate(&allObjects_dblk,   &dummy, sizeof_AllObjects_t, __FILE__, __func__, __LINE__);
   gasket__ocrDbCreate(&block_dblk,        &dummy, sizeof_Block_t,      __FILE__, __func__, __LINE__);
#undef control            // Clean up the above hack, so that it's effects will not propagate to any code below.
   blockInit_Params_t * blockInit_Params = myParams;

   ocrGuid_t     blockInit_Edt;
   ocrEdtCreate(&blockInit_Edt,
                myParams->template.blockInit_Template,
                EDT_PARAM_DEF,
                (u64 *) blockInit_Params,
                EDT_PARAM_DEF,
                NULL,
                EDT_PROP_NONE,
                NULL_GUID,
                NULL_GUID);

   ocrDbRelease  (initialControl_dblk);     initialControl    = NULL;
   ADD_DEPENDENCE(initialControl_dblk,      blockInit_Edt, blockInit_Deps_t, initialControl_Dep,     DB_MODE_RO);
   ADD_DEPENDENCE(initialAllObjects_dblk,   blockInit_Edt, blockInit_Deps_t, initialAllObjects_Dep,  DB_MODE_RO);
   ADD_DEPENDENCE(initialBlock_dblk,        blockInit_Edt, blockInit_Deps_t, initialBlock_Dep,       initialBlock_dblk==NULL_GUID ? DB_MODE_NULL : DB_MODE_RO);
   ADD_DEPENDENCE(meta_dblk,                blockInit_Edt, blockInit_Deps_t, meta_Dep,               DB_MODE_RW);
   ADD_DEPENDENCE(control_dblk,             blockInit_Edt, blockInit_Deps_t, control_Dep,            DB_MODE_RW);
   ADD_DEPENDENCE(allObjects_dblk,          blockInit_Edt, blockInit_Deps_t, allObjects_Dep,         DB_MODE_RW);
   ADD_DEPENDENCE(block_dblk,               blockInit_Edt, blockInit_Deps_t, block_Dep,              DB_MODE_RW);

   return NULL_GUID;
} // blockLaunch_Func


// **************************************************************************************************************************************************************************************************************
// blockInit_Func
//
// This initializes control_dblk, allObjects_dblk, and meta_dblk, then creates all other datablocks that will be needed to process the block, then passes control to the blockContinuation EDT.
ocrGuid_t blockInit_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

   int i;

   blockInit_Deps_t   * myDeps   = (blockInit_Deps_t *)   depv;
   blockInit_Params_t * myParams = (blockInit_Params_t *) paramv;
   ocrGuid_t            initialControl_dblk     =                   (myDeps->initialControl_Dep.guid);
   Control_t    const * initialControl          = ((Control_t *)    (myDeps->initialControl_Dep.ptr));
   ocrGuid_t            initialAllObjects_dblk  =                   (myDeps->initialAllObjects_Dep.guid);
   AllObjects_t const * initialAllObjects       = ((AllObjects_t *) (myDeps->initialAllObjects_Dep.ptr));
   ocrGuid_t            meta_dblk               =                   (myDeps->meta_Dep.guid);
   BlockMeta_t        * meta                    = ((BlockMeta_t *)  (myDeps->meta_Dep.ptr));
   ocrGuid_t            control_dblk            =                   (myDeps->control_Dep.guid);
   Control_t          * control                 = ((Control_t *)    (myDeps->control_Dep.ptr));
   ocrGuid_t            allObjects_dblk         =                   (myDeps->allObjects_Dep.guid);
   AllObjects_t       * allObjects              = ((AllObjects_t *) (myDeps->allObjects_Dep.ptr));
   ocrGuid_t            block_dblk              =                   (myDeps->block_Dep.guid);
   Block_t            * block                   = ((Block_t *)      (myDeps->block_Dep.ptr));
printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d)\n", __func__, __FILE__, __LINE__, myParams->refinementLevel, myParams->xPos, myParams->yPos, myParams->zPos); fflush(stdout);

// Init our copy of control_db.  Some of the sizeof_... references below expand macros that depend upon having a valid "control" pointer.

   memcpy (control,    initialControl,    sizeof_Control_t);

// Now init our meta data.
// First, init the Continuation Cloning State.

   Frame_Header_t * topOfStack  = (Frame_Header_t *) meta->cloningState.stack;
   topOfStack[0].resumption_case_num                  = -9999;                  // Irrelevant for topmost activation record.
   topOfStack[0].my_size                              = sizeof_Frame_Header_t;  // Distance to first callee's frame.
   topOfStack[0].caller_size                          = -9999;                  // Irrelevant for topmost activation record.
   topOfStack[0].validate_callers_prep_for_suspension = -9999;                  // Irrelevant for topmost activation record.
   topOfStack[1].resumption_case_num                  = 0;                      // Prepare for first callee.
   topOfStack[1].my_size                              = -9999;                  // Size of callee's frame is not known by caller.
   topOfStack[1].caller_size                          = sizeof_Frame_Header_t;  // To tell callee how much to pop when it returns.
   meta->cloningState.topPtrAdjRecOffset              = -9999;
   meta->cloningState.numberOfDatablocks              = numDatablocksInBlockMetaCatalog;
   meta->cloningState.offsetToCatalogOfDatablocks     = ((unsigned long long) (&(meta->dbCatalog[0]))) - ((unsigned long long) (&(meta->cloningState)));
   meta->cloningState.cloneNum                        = 0;

// Init all the datablocks in the catalog to null.

   for (i = 0; i < meta->cloningState.numberOfDatablocks; i++) {
      meta->dbCatalog[i].dblk = NULL_GUID;      // Default to the datablock NOT existing yet.
      meta->dbCatalog[i].base = NULL;           // Default to its pointer also indicating it does not exist yet.
      meta->dbCatalog[i].size = -9999;          // Default that, when it ultimately is created, its size will have to be filled in at that time, as it is unknowable in advance.
      meta->dbCatalog[i].acMd = DB_MODE_NULL;   // Default to it having no access rights yet.
  }

// Now flesh out more details about the datablocks that we know more about.

   meta->metaDb.dblk              = meta_dblk;            // If this looks a bit "self-referential", it is because it unions with an array that holds the meta info about ALL the other
   meta->metaDb.base              = meta;                 // datablocks too, and we use that array to convert between pointers(used in the body of EDTs) and datablock-index-and-offset
   meta->metaDb.size              = sizeof_BlockMeta_t;   // (used to communicate the pointers from one EDT to another, wherein OCR might move the datablock to a new address).
   meta->metaDb.acMd              = DB_MODE_UPDATE;       // We are writing to this datablock here, in this very instance of this EDT, but it will be updated by our successor EDT.

   meta->controlDb.dblk           = control_dblk;         // This datablock is read-only, and shared among all children below this point.
   meta->controlDb.base           = control;
   meta->controlDb.size           = sizeof_Control_t;
   meta->controlDb.acMd           = DB_MODE_RO;           // We are writing to this databock in this very EDT (we did so just above), but hereafter, this one will be read-only to our successor lineage.

   meta->allObjectsDb.dblk        = allObjects_dblk;      // Each child gets a copy of this datablock, and they can manipulate (i.e. track the movement of the objects over time).  The
   meta->allObjectsDb.size        = sizeof_AllObjects_t;  // children all track the movement identically, but doing so is cheaper than having the movement modeled in just one place and
   meta->allObjectsDb.base        = allObjects;           // broadcast around.
   meta->allObjectsDb.acMd        = DB_MODE_RO;           // We write to this datablock here.  OCCASSIONALLY an EDT in the successor lineage will need to modify this, at which time it will change the rights.

   meta->blockDb.dblk             = block_dblk;
   meta->blockDb.size             = sizeof_Block_t;
   meta->blockDb.base             = block;
   meta->blockDb.acMd             = DB_MODE_RW;           // We only create this datablock.  It will be first written (initialized) by our first successor.  After that, it will be updated by every successor.

   void * dummy;
#ifndef ALLOW_DATABLOCK_REWRITES
   // The first successor will surely need to clone metaDb.  Create its datablock for doing same.
   gasket__ocrDbCreate(&meta->metaCloneDb.dblk, (void **) &dummy, sizeof_BlockMeta_t, __FILE__, __func__, __LINE__);
   meta->metaCloneDb.size         = sizeof_BlockMeta_t;
   meta->metaCloneDb.acMd         = DB_MODE_RW;           // Our successor EDT will surely update this datablock.

   meta->blockCloneDb.size        = sizeof_Block_t;
   meta->allObjectsCloneDb.size   = sizeof_AllObjects_t;
   meta->profileCloneDb.size      = sizeof_Profile_t;
#endif

   // The first successor will initialize the profile collection datablock.  We create it here.
   gasket__ocrDbCreate(&meta->profileDb.dblk,   (void **) &dummy,   sizeof_Profile_t, __FILE__, __func__, __LINE__);
   meta->profileDb.size           = sizeof_Profile_t;
   meta->profileDb.acMd           = DB_MODE_RW;

// The catalog of datablocks is complete.  Now init other meta variables.

   int size = control->p2[control->num_refine+1];     // Block size is p2[num_refine+1-level].  Smallest block is size p2[1], so can find its center

   meta->refinementLevel                 = myParams->refinementLevel;
   meta->xPos                            = myParams->xPos;
   meta->yPos                            = myParams->yPos;
   meta->zPos                            = myParams->zPos;
   meta->cen[0]                          = myParams->xPos * size + (size >> 1);
   meta->cen[1]                          = myParams->yPos * size + (size >> 1);
   meta->cen[2]                          = myParams->zPos * size + (size >> 1);
   meta->conveyOperandUpward_Event       = myParams->conveyOperandUpward_Event;   // Event to satisfy dependence at parent, by which he is awaiting Operand_dblk from us.
   meta->labeledGuidRangeForHaloExchange = myParams->labeledGuidRangeForHaloExchange;

   int dir, pole, qrtr;
   for (dir = 0; dir < 3; dir++) {
      for (pole = 0; pole <= 1; pole++) {
//         meta->conveyFaceToFinerNeighbor_Event      [dir][pole]      = NULL_GUID;   // Create event upon first demand, via labeled-guid lookup
//         meta->conveyFaceToSameGrainedNeighbor_Event[dir][pole]      = NULL_GUID;   // Create event upon first demand, via labeled-guid lookup
         for (qrtr = 0; qrtr < 4; qrtr++) {
//            meta->conveyFaceToCoarserNeighbor_Event[dir][pole][qrtr] = NULL_GUID;   // Create event upon first demand, via labeled-guid lookup
            meta->conveyFaceToNeighbor_Event [dir][pole][qrtr] = NULL_GUID; // Create event upon first demand, via labeled-guid lookup
            meta->onDeckToReceiveFace_Event[dir][pole][qrtr]   = NULL_GUID; // Create event upon first demand, via labeled-guid lookup
         }
      }
   }

// TODO:  Change, when refinement is implemented

   meta->neighborRefinementLevel[0][0] = meta->xPos == 0                ? -2 : 0;
   meta->neighborRefinementLevel[0][1] = meta->xPos == control->npx - 1 ? -2 : 0;
   meta->neighborRefinementLevel[1][0] = meta->yPos == 0                ? -2 : 0;
   meta->neighborRefinementLevel[1][1] = meta->yPos == control->npy - 1 ? -2 : 0;
   meta->neighborRefinementLevel[2][0] = meta->zPos == 0                ? -2 : 0;
   meta->neighborRefinementLevel[2][1] = meta->zPos == control->npz - 1 ? -2 : 0;

// Init other datablocks.

   memcpy (allObjects, initialAllObjects, sizeof_AllObjects_t);

// Pass baton to the first in a series of continuations.

   blockContinuation_Params_t blockContinuation_Params;
   blockContinuation_Params.template     = myParams->template;

   ocrGuid_t blockContinuation_Edt;
   ocrEdtCreate(&blockContinuation_Edt,
                myParams->template.blockContinuation_Template,
                EDT_PARAM_DEF,
                (u64 *) &blockContinuation_Params,
                EDT_PARAM_DEF,
                NULL,
                EDT_PROP_NONE,
                NULL_GUID,
                NULL_GUID);

   for (i = meta->cloningState.numberOfDatablocks-1; i >= 0; i--) {
      if (meta->dbCatalog[i].dblk == NULL_GUID) {
         ADD_DEPENDENCE(NULL_GUID, blockContinuation_Edt, blockContinuation_Deps_t, dependence[i], DB_MODE_NULL);
      } else {
         ocrGuid_t         dblk = meta->dbCatalog[i].dblk;
         ocrDbAccessMode_t acMd = meta->dbCatalog[i].acMd;
         meta->dbCatalog[i].base = NULL;
         ocrDbRelease(dblk);     // Release the guid.  Warning:  in the last loop iteration, this releases meta, which is why we copied dblk and acMd to temps above.
         ADD_DEPENDENCE(dblk, blockContinuation_Edt, blockContinuation_Deps_t, dependence[i], acMd);
      }
   }
} // blockInit_Func


// **************************************************************************************************************************************************************************************************************
// blockContinuation_Func
//
// This is the code that performs the algorithm on a block at the leaf of the refinement level depth tree.  It advances the algorithm as far as it can, to such point where a communication is needed from
// another EDT (such as a halo exchange from a neighbor, or such as the results of a service requested of the parent), at which time it clones, and the clone continues where the predecessor left off.  This
// particular function just runs the loop that drives the continuation cloning logic.  It calls blockContinuation_SoupToNuts, which is the top function of the calling topology of functions that do the work.
ocrGuid_t blockContinuation_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

   int i;

   blockContinuation_Deps_t   * myDeps   = (blockContinuation_Deps_t *)   depv;
   blockContinuation_Params_t * myParams = (blockContinuation_Params_t *) paramv;

   // First prepare meta datablock.

#ifdef ALLOW_DATABLOCK_REWRITES
   ocrGuid_t           meta_dblk        =                      (myDeps->meta_Dep.guid);
   BlockMeta_t       * meta             = ((BlockMeta_t *)     (myDeps->meta_Dep.ptr));
#else
   ocrGuid_t           oldMeta_dblk     =                      (myDeps->meta_Dep.guid);
   BlockMeta_t const * oldMeta          = ((BlockMeta_t *)     (myDeps->meta_Dep.ptr));
   ocrGuid_t           meta_dblk        =                      (myDeps->metaClone_Dep.guid);
   BlockMeta_t       * meta             = ((BlockMeta_t *)     (myDeps->metaClone_Dep.ptr));
   memcpy (meta, oldMeta, sizeof_BlockMeta_t);
   ocrDbDestroy(oldMeta_dblk);  oldMeta_dblk = NULL_GUID; oldMeta = NULL;
   meta->metaCloneDb.dblk                 = NULL_GUID;
   meta->metaCloneDb.base                 = NULL;
#endif

   meta->metaDb.dblk                              = meta_dblk;
   meta->metaDb.base                              = meta;
   meta->cloningState.tos                         = ((char *)                  (((unsigned long long) meta->cloningState.stack)));
   meta->cloningState.topPtrAdjRec                = ((PtrAdjustmentRecord_t *) (((unsigned long long) meta->cloningState.stack) + meta->cloningState.topPtrAdjRecOffset));

   // Now prepare profile datablock.

#ifdef ALLOW_DATABLOCK_REWRITES
   meta->profileDb.dblk                  =                      (myDeps->profile_Dep.guid);
   meta->profileDb.base                  = ((Profile_t *)       (myDeps->profile_Dep.ptr));
#else
   meta->profileDb.dblk                  =                      (myDeps->profileClone_Dep.guid);
   meta->profileDb.base                  = ((Profile_t *)       (myDeps->profileClone_Dep.ptr));
   if (meta->profileDb.dblk == NULL_GUID) {                                                      // If we are NOT cloning the profile (i.e. if this is the version of this EDT created by blockInit_Func)
      meta->profileDb.dblk               =                      (myDeps->profile_Dep.guid);
      meta->profileDb.base               = ((Profile_t *)       (myDeps->profile_Dep.ptr));
   } else {
      ocrGuid_t       oldProfile_dblk    =                      (myDeps->profile_Dep.guid);
      Profile_t     * oldProfile         = ((Profile_t *)       (myDeps->profile_Dep.ptr));
      memcpy (meta->profileDb.base, oldProfile, sizeof_Profile_t);
      ocrDbDestroy (oldProfile_dblk);
      meta->profileCloneDb.dblk          = NULL_GUID;
      meta->profileCloneDb.base          = NULL;
      meta->profileCloneDb.acMd          = DB_MODE_NULL;
   }
#endif

   // Now prepare all other datablocks.

   for (i = 0; i < meta->cloningState.numberOfDatablocks; i++) {
      if (&meta->dbCatalog[i].dblk == &meta->metaDb.dblk)         continue;    // meta already prepared.
      if (&meta->dbCatalog[i].dblk == &meta->profileDb.dblk)      continue;    // profile already prepared.
#ifndef ALLOW_DATABLOCK_REWRITES
      if (&meta->dbCatalog[i].dblk == &meta->metaCloneDb.dblk)    continue;    // already done with metaClone, above.
      if (&meta->dbCatalog[i].dblk == &meta->profileCloneDb.dblk) continue;    // already done with profileClone, above.
#endif
      meta->dbCatalog[i].dblk = (myDeps->dependence[i].guid);
      meta->dbCatalog[i].base = (myDeps->dependence[i].ptr);
   }
   ocrGuid_t          control_dblk = meta->controlDb.dblk;
   Control_t  const * control      = meta->controlDb.base;

   meta->cloningState.cloneNum++;
//printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d, clone=%4d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum); fflush(stdout);

   meta->cloningState.continuationOpcode = ContinuationOpcodeUnspecified;
   Frame_Header_t * topOfStack  = (Frame_Header_t *) meta->cloningState.stack;

   topOfStack[1].validate_callers_prep_for_suspension = 1;           // Set "canary trap" for first callee.
   meta->cloningState.returnCanary                    = 1;                      // Init the "return canary trap" to "live", to detect a return from a suspendable function via a return stmt instead of the macro.
   blockContinuation_SoupToNuts(meta);                               // Resume (or start, the first time) the algorithm.  When we get back, it is either done or needing to clone.
   if (meta->cloningState.returnCanary == 1) {  // If the return canary is "live" the callee must have used a return statement.  We need it to use the macro!
      printf ("ERROR: %s at line %d: Canary trap on RETURN from the callee.  Change callee to return through the NORMAL_RETURN_SEQUENCE macro.\n", __FILE__, __LINE__); fflush(stdout); \
      printf ("ERROR: Other possibility is that callee is not (yet) a SUSPENDABLE function, but it is being identified as such by the caller.\n"); fflush(stdout); \
      *((int *) 123) = 456; \
      ocrShutdown();
   }
   if (topOfStack[1].resumption_case_num != 0) {                     // Create a continuation EDT, cause it to fire, and cause this EDT has to terminate.

      meta->cloningState.topPtrAdjRecOffset = ((int) ((unsigned long long) meta->cloningState.topPtrAdjRec) - ((unsigned long long) meta->cloningState.stack));

      ocrGuid_t blockContinuation_Edt;
      ocrEdtCreate(&blockContinuation_Edt,                           // Guid of the EDT created to continue at function blockContinuaiton_Func.
                   myParams->template.blockContinuation_Template,    // Template for the EDT we are creating.
                   EDT_PARAM_DEF,
                   (u64 *) myParams,
                   EDT_PARAM_DEF,
                   NULL,
                   EDT_PROP_NONE,
                   NULL_GUID,
                   NULL_GUID);

#ifndef ALLOW_DATABLOCK_REWRITES
      gasket__ocrDbCreate(&meta->metaCloneDb.dblk,    (void **) &meta->metaCloneDb.base,    sizeof_BlockMeta_t, __FILE__, __func__, __LINE__);
      meta->metaCloneDb.base       = NULL;
      gasket__ocrDbCreate(&meta->profileCloneDb.dblk, (void **) &meta->profileCloneDb.base, sizeof_Profile_t,   __FILE__, __func__, __LINE__);
      meta->profileCloneDb.base    = NULL;
      meta->profileCloneDb.acMd    = DB_MODE_RW;
#endif

      switch (meta->cloningState.continuationOpcode) {
      case ContinuationOpcodeUnspecified :
         printf ("Continuation opcode unspecified\n"); fflush(stdout);
         ocrShutdown();
         *((int *) 0) = 123;
         break;
      case SeasoningOneOrMoreDbCreates:
         // TODO:  If a newResource SLOT exists, we will need to satisfy it with NULL_GUID for this opcode:  ADD_DEPENDENCE(NULL_GUID, blockContinuation_Edt, blockContinuation_Deps_t, dep_newResource, RO);
         break;
      case ReceivingACommunication:
#if 0
         ocrGuidFromIndex(&resource, shared->labeledGuidRange, (((my_pe*num_pes+continuationDetail)*GUID_ROUND_ROBIN_SPAN)+guid_toggle__recv[continuationDetail]));
         guid_toggle__recv[continuationDetail] = (guid_toggle__recv[continuationDetail] + 1) & (GUID_ROUND_ROBIN_SPAN - 1);
#define DEFAULT_LG_PROPS GUID_PROP_IS_LABELED | GUID_PROP_CHECK | EVT_PROP_TAKES_ARG
         ocrEventCreate(&resource, OCR_EVENT_STICKY_T, DEFAULT_LG_PROPS);
         ADD_DEPENDENCE(resource, continuation, referenceVersionsMain_Deps_t, dep_newResource, RO);
         break;
      case MassParallelismDone:
         printf ("pe %d is regaining control after utilizing massive OpenMP-like parallelism, such as for the stencil operation\n", my_pe); fflush(stdout);
         *((int *) 0) = 123;
         ADD_DEPENDENCE(NULL_GUID, continuation, referenceVersionsMain_Deps_t, dep_newResource, RO);
#endif
         break;
      default:
         printf ("Continuation opcode not yet handled\n"); fflush(stdout);
         ocrShutdown();
         *((int *) 0) = 123;
         break;
      }

      for (i = meta->cloningState.numberOfDatablocks-1; i >= 0; i--) {
         if (meta->dbCatalog[i].dblk == NULL_GUID) {
            ADD_DEPENDENCE(NULL_GUID, blockContinuation_Edt, blockContinuation_Deps_t, dependence[i], DB_MODE_NULL);
         } else {
            ocrGuid_t         dblk = meta->dbCatalog[i].dblk;
            ocrDbAccessMode_t acMd = meta->dbCatalog[i].acMd;
//printf ("zPos = %d, Adding dependence of i = %d, dblk = 0x%lx, base = 0x%lx, size = %d\n", meta->zPos, i, (unsigned long long) dblk, (unsigned long long) meta->dbCatalog[i].base, meta->dbCatalog[i].size); fflush(stdout);
            meta->dbCatalog[i].dblk = NULL_GUID;
            meta->dbCatalog[i].base = NULL;
            //ocrDbRelease(dblk);     // Release the guid.  Warning:  in the last loop iteration, this releases meta, which is why we copied dblk and acMd to temps above.
            ADD_DEPENDENCE(dblk, blockContinuation_Edt, blockContinuation_Deps_t, dependence[i], acMd);
            //meta->dbCatalog[i].acMd = DB_MODE_NULL;
         }
      }

   } else {  // (topOfStack[1].resumption_case_num != 0)    If that was the very last iteration, clean up and shut down.
      int i;
      for (i = 1; i < meta->cloningState.numberOfDatablocks; i++) {     // Skip over meta_t (until after this loop).
         if (meta->dbCatalog[i].dblk != NULL_GUID) {
            ocrDbDestroy(meta->dbCatalog[i].dblk);
            meta->dbCatalog[i].dblk = NULL_GUID;
            meta->dbCatalog[i].base = NULL;
         }
      }
      ocrDbDestroy(meta_dblk);
   }
   return NULL_GUID;

} // blockContinuation_Func


// **************************************************************************************************************************************************************************************************************
// blockContinuation_SoupToNuts
//
// This is the top-level function in the calling topology of the several functions that do the actual processing of a block.
//
void blockContinuation_SoupToNuts (BlockMeta_t * meta) {

   typedef struct {
      Frame_Header_t myFrame;
      double t1, t2, t3, t4;
      int ts, var, start, number, stage, comm_stage;
      struct {
         Control_t * control;
         Profile_t * profile;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__blockContinuation_SoupToNuts_t;

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
#define profile    (lcl->pointers.profile)

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__blockContinuation_SoupToNuts_t)
printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d, clone=%4d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum); fflush(stdout);
   control = meta->controlDb.base;
   profile = meta->profileDb.base;

   if (meta->refinementLevel == 0) {
      // Initialize the unrefined mesh block to random cell contents.
      CALL_SUSPENDABLE_CALLEE
      init(meta);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
   } else {
      // TODO: refine octant of parent's block into this block.
   }
   CALL_SUSPENDABLE_CALLEE
   transmitBlockContributionForChecksum(meta, 0);
   DEBRIEF_SUSPENDABLE_FUNCTION(;)

   init_profile(meta);    // Initialize the performance metrics information for this block.

   t1 = timer();

   if (control->num_refine != 0 || control->uniform_refine) {
      CALL_SUSPENDABLE_CALLEE
      refine(meta, 0);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
   }

   t2 = timer();
   profile->timer_refine_all += t2 - t1;

   if (control->plot_freq) {
      CALL_SUSPENDABLE_CALLEE
      transmitBlockContributionForPlot(meta, 0);
      DEBRIEF_SUSPENDABLE_FUNCTION(;)
   }
   t3 = timer();
   profile->timer_plot += t3 - t2;

//===
//TODO:   nb_min = nb_max = global_active;

   for (comm_stage = 0, ts = 1; ts <= control->num_tsteps; ts++) {
      for (stage = 0; stage < control->stages_per_ts; stage++, comm_stage++) {
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
            comm (meta, start, number, comm_stage);
            DEBRIEF_SUSPENDABLE_FUNCTION(;)
            t4 = timer();
            profile->timer_comm_all += t4 - t3;
            for (var = start; var < (start+number); var++) {
               stencil_calc(meta, var);
               t3 = timer();
               profile->timer_calc_all += t3 - t4;
               t4 = t3;
            }

            // Reference code does the following if-stmt inside the above for-loop, i.e. does checksums comm_vars at a time.  OCR version does all at once.
            if (control->checksum_freq && !(stage%control->checksum_freq)) {
               CALL_SUSPENDABLE_CALLEE
               transmitBlockContributionForChecksum(meta, ts);
               DEBRIEF_SUSPENDABLE_FUNCTION(;)
#if 0
Move to rootProgenitor:
               if (report_diffusion && !my_pe) {
                  printf("%d var %d sum %lf old %lf diff %lf tol %lf\n", ts, var, sum, grid_sum[var], (fabs(sum - grid_sum[var])/grid_sum[var]), tol);
               }
               if (fabs(sum - grid_sum[var])/grid_sum[var] > tol) {
                  if (!my_pe)
                     printf("Time step %d sum %lf (old %lf) variable %d difference too large\n", ts, sum, grid_sum[var], var);
                  SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
               }
               grid_sum[var] = sum;
#endif
            }
            t4 = timer();
            profile->timer_cs_all += t4 - t3;
         }
      }

      if (control->num_refine && !control->uniform_refine) {
         CALL_SUSPENDABLE_CALLEE
         move(meta);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
         if (!(ts%control->refine_freq)) {
            CALL_SUSPENDABLE_CALLEE
            refine(meta, ts);
            DEBRIEF_SUSPENDABLE_FUNCTION(;)
         }
      }
      t2 = timer();
      profile->timer_refine_all += t2 - t4;

      t3 = timer();
      if (control->plot_freq && ((ts % control->plot_freq) == 0)) {
         CALL_SUSPENDABLE_CALLEE
         transmitBlockContributionForPlot(meta, ts);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
      }
      profile->timer_plot += timer() - t3;
   }
   profile->timer_all = timer() - t1;


   // We are done with this block.  If we are at a refinementLevel > 0, we need to give our block back to our parent to consolidate it with our siblings and produce an unrefined aggregate.  We will also
   // need to send of profile datablock up to the parent for aggregation into the total.
   //
   // If we are at refinementLevel == 0 (i.e. totally unrefined), then we are ready to shutdown the application, but that is a duty of our parent, the rootProgenitor.  Just pass him our profile datablock
   // and know that we will never hear from him again.

   if (meta->refinementLevel == 0) {
      ocrGuid_t conveyOperandUpward_Event  = meta->conveyOperandUpward_Event;                                     // Jot down the At Bat Event.
      profile->dbCommHeader.serviceOpcode  = Operation_Profile;
      profile->dbCommHeader.timeStep       = ts;
      profile->dbCommHeader.atBat_Event    = conveyOperandUpward_Event;                                           // Convey At Bat Event to parent so that he can destroy the event.
      meta->conveyOperandUpward_Event      = NULL_GUID;                                                           // Cease creating an On Deck Event for the next time;  there is no next time.
      profile->dbCommHeader.onDeck_Event   = NULL_GUID;                                                           // Convey that NULL_GUID to parent, too.
      ocrDbRelease(meta->profileDb.dblk);
      ocrEventSatisfy(conveyOperandUpward_Event, meta->profileDb.dblk);                                           // Satisfy the parent's operand1 dependence.
      meta->profileDb.base = profile = NULL;
      meta->profileDb.dblk = NULL_GUID;
      meta->profileDb.size = -9999;
      meta->profileDb.acMd = DB_MODE_NULL;
      SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)    // Top-level will sense normal completion, and just end this EDT (without creating any more clones).
   } else {
//printf ("Function %36s, File %30s, line %4d, for block at (lvl=%d, xPos=%d, yPos=%d, zPos=%d, clone=%4d)\n", __func__, __FILE__, __LINE__, meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum); fflush(stdout);
      *((int *) (123)) = 456;   // TODO  FIXME   crash the application until we implement unrefinement.
      SUSPEND__RESUME_IN_CONTINUATION_EDT(;)   // TODO:  Is this needed?
      SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   }
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
#undef profile

} // blockContinuation_SoupToNuts
