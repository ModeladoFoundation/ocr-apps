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
#include <ocr-std.h>
#include <ocr-guid-functions.h>
#include <extensions/ocr-labeling.h>
#include "ocrmacs.h"

#include "control.h"
#include "object.h"
#include "chksum.h"
#include "block.h"
//PROFILE:#include "profile.h"
#include "proto.h"
#include "clone.h"
#include "refine.h"

#ifdef NANNY_FUNC_NAMES
#line __LINE__ "parent "
#endif

// parent.c        This is the parent of a 2x2x2 set of children blocks.
// ***************************************************************************************************************************************************************************************
// ***************************************************************************************************************************************************************************************
// ***************************************************************************************************************************************************************************************

// **************************************************************************************************************************************************************************************************************
// parentInit_Func
// -- Topology:  Init(initialize contents of datablocks, create children) --> Clone(steady state: perform service requests of children; clone again)
//
// This EDT initializes the parent's meta datablock, and then just CALLS the parentClone_Func.  Thereafter, parentClone_Func is created as a clone EDT, for each service that it is to fulfill past the first one.

ocrGuid_t parentInit_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

   int i, j, k;

   parentClone_Deps_t   * myDeps   = (parentClone_Deps_t *)   depv;
   parentClone_Params_t * myParams = (parentClone_Params_t *) paramv;
   BlockMeta_t  * blockMeta = (BlockMeta_t *)  myDeps->blockMeta_Dep.ptr;

   // Audit incoming dependences.

#ifdef NANNY_ON_STEROIDS
   #define NANNYLEN 200
   char nanny[NANNYLEN];
   sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", blockMeta->refinementLevel, blockMeta->xPos, blockMeta->yPos, blockMeta->zPos, blockMeta->cloningState.cloneNum);
#else
#define nanny NULL
#endif

   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->meta_Dep,              parentClone_Deps_t, meta_Dep,             nanny, "meta");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->blockMeta_Dep,         parentClone_Deps_t, blockMeta_Dep,        nanny, "blockMeta");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->control_Dep,           parentClone_Deps_t, control_Dep,          nanny, "control");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->controlFromBlock_Dep,  parentClone_Deps_t, controlFromBlock_Dep, nanny, "controlFromBlock");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->upboundRequest_Dep,    parentClone_Deps_t, upboundRequest_Dep,   nanny, "upboundRequest");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->serviceRequest_Dep[0], parentClone_Deps_t, serviceRequest_Dep[0],nanny, "serviceRequest[0]");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->serviceRequest_Dep[1], parentClone_Deps_t, serviceRequest_Dep[1],nanny, "serviceRequest[1]");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->serviceRequest_Dep[2], parentClone_Deps_t, serviceRequest_Dep[2],nanny, "serviceRequest[2]");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->serviceRequest_Dep[3], parentClone_Deps_t, serviceRequest_Dep[3],nanny, "serviceRequest[3]");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->serviceRequest_Dep[4], parentClone_Deps_t, serviceRequest_Dep[4],nanny, "serviceRequest[4]");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->serviceRequest_Dep[5], parentClone_Deps_t, serviceRequest_Dep[5],nanny, "serviceRequest[5]");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->serviceRequest_Dep[6], parentClone_Deps_t, serviceRequest_Dep[6],nanny, "serviceRequest[6]");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->serviceRequest_Dep[7], parentClone_Deps_t, serviceRequest_Dep[7],nanny, "serviceRequest[7]");

//printf ("Function %36s, File %30s, line %4d\n", __func__, __FILE__, __LINE__); fflush(stdout);

   ParentMeta_t * meta      = (ParentMeta_t *) myDeps->meta_Dep.ptr;

   DbSize_t * dbSize = meta->dbSize;

   DbSize(myDeps->whoAmI_Dep)               = sizeof(ocrGuid_t);
   DbSize(myDeps->meta_Dep)                 = sizeof_ParentMeta_t;
   DbSize(myDeps->blockMeta_Dep)            = sizeof_BlockMeta_t;
   DbSize(myDeps->control_Dep)              = sizeof_Control_t;
   DbSize(myDeps->controlFromBlock_Dep)     = sizeof_Control_t;
   DbSize(myDeps->upboundRequest_Dep)       = -9999;
   for(i = 0; i < 8; i++) {
      DbSize(myDeps->serviceRequest_Dep[i]) = -9999;
   }

   // Set the parent's block characteristics to those of the unrefined block, not those of any particular child of that block.  These are used for debug messages, and then to convey to join-clone at unrefine.

   meta->conveyServiceRequestToParent_Event           = blockMeta->conveyServiceRequestToParent_Event;
   for (i = 0; i < 8; i++) {
      meta->conveyEighthBlockToJoin_Event[i]          = blockMeta->conveyEighthBlockToJoin_Event[i];
   }
   for (i = 0; i < 3; i++) {
      for (j = 0; j < 2; j++) {
         for (k = 0; k < 4; k++) {
            meta->conveyFaceToNeighbor_Event[i][j][k] = blockMeta->conveyFaceToNeighbor_Event[i][j][k];
            meta->onDeckToReceiveFace_Event[i][j][k]  = blockMeta->onDeckToReceiveFace_Event[i][j][k];
         }
      }
   }
   meta->xPos                                         = blockMeta->xPos;
   meta->yPos                                         = blockMeta->yPos;
   meta->zPos                                         = blockMeta->zPos;
   meta->numBaseResBlocks                             = blockMeta->numBaseResBlocks;
   meta->cen[0]                                       = blockMeta->cen[0];
   meta->cen[1]                                       = blockMeta->cen[1];
   meta->cen[2]                                       = blockMeta->cen[2];
   meta->refinementLevel                              = blockMeta->refinementLevel;

   // Prepare the cloning stack.
   Frame_Header_t * topOfStack  = (Frame_Header_t *) meta->cloningState.stack;
   topOfStack[0].resumption_case_num                  = -9999;                  // Irrelevant for topmost activation record.
   topOfStack[0].my_size                              = sizeof_Frame_Header_t;  // Distance to first callee's frame.
   topOfStack[0].caller_size                          = -9999;                  // Irrelevant for topmost activation record.
   topOfStack[0].validate_callers_prep_for_suspension = -9999;                  // Irrelevant for topmost activation record.
   topOfStack[1].resumption_case_num                  = 0;                      // Prepare for first callee.
   topOfStack[1].my_size                              = -9999;                  // Size of callee's frame is not known by caller.
   topOfStack[1].caller_size                          = sizeof_Frame_Header_t;  // To tell callee how much to pop when it returns.
   meta->cloningState.topPtrAdjRecOffset              = -9999;
   meta->cloningState.numberOfDatablocks              = countof_parentClone_Deps_t;
   meta->cloningState.cloneNum                        = 0;

#ifdef NANNY_ON_STEROIDS
   sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d  Destroying after child-prongs are all done with it!", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
   gasket__ocrDbDestroy(&myDeps->blockMeta_Dep.guid, &myDeps->blockMeta_Dep.ptr, __FILE__, __func__, __LINE__, nanny, "blockMeta"); // We got what we needed from the blockMeta (of the unrefined block), and we are sure that all child prongs are done with it too.  Time to destroy it.

   memcpy(myDeps->control_Dep.ptr, myDeps->controlFromBlock_Dep.ptr, sizeof_Control_t);
   gasket__ocrDbDestroy(&myDeps->controlFromBlock_Dep.guid, &myDeps->controlFromBlock_Dep.ptr, __FILE__, __func__, __LINE__, nanny, "controlFromBlock"); // We made our own copy of control,  and we are sure that all child prongs are done with it too.  Time to destroy it.
   void * dummy = NULL;
   gasket__ocrDbDestroy(&myParams->allObjects_Dep_ToDestroy, &dummy, __FILE__, __func__, __LINE__, nanny, "allObjects"); // We are sure that all child prongs are done with this.
   gasket__ocrDbDestroy(&myParams->block_Dep_ToDestroy,      &dummy, __FILE__, __func__, __LINE__, nanny, "block"); // We are sure that all child prongs are done with this.

   parentClone_Func(paramc, paramv, depc, depv);  // Just drop into the parentClone_Func for its first service request.

   // Commented out because the above invocation of parentClone_Func is a mere function call, instead of an EDT instantiation.  If uncommented, this causes a SECOND ocrDbDestroy of the whoAmI datablock.
   // REPORT_EDT_DEMISE(myDeps->whoAmI_Dep);
   // Defeat OCR's habit of releasing the input ocrEdtDep_t's, because we have changed many of them to other guids, including some events, which simply must not be released.  This is just a debugging expedient.
   if (0) {
      myDeps->meta_Dep.guid                = NULL_GUID;
      myDeps->whoAmI_Dep.guid              = NULL_GUID;
      myDeps->blockMeta_Dep.guid           = NULL_GUID;
      myDeps->control_Dep.guid             = NULL_GUID;
      myDeps->controlFromBlock_Dep.guid    = NULL_GUID;
      myDeps->upboundRequest_Dep.guid      = NULL_GUID;
      int i;
      for (i=0; i < 8; i++) myDeps->serviceRequest_Dep[i].guid = NULL_GUID;
   }
   return NULL_GUID;

} // parentInit_Func


// **************************************************************************************************************************************************************************************************************
// parentClone_Func
// -- Topology:  Launch(create datablocks) --> Init(initialize contents of datablocks, create children) --> Clone(steady state: perform service requests of children; clone again)
//
// Provide services to the 2x2x2 set of child blocks.

ocrGuid_t parentClone_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]) {

   parentClone_Deps_t   * myDeps    = (parentClone_Deps_t *)   depv;
   parentClone_Params_t * myParams  = (parentClone_Params_t *) paramv;

   ParentMeta_t         * meta      = (ParentMeta_t *) myDeps->meta_Dep.ptr;

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

   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->meta_Dep,              parentClone_Deps_t, meta_Dep,             nanny, "meta");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->blockMeta_Dep,         parentClone_Deps_t, blockMeta_Dep,        nanny, "blockMeta");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->control_Dep,           parentClone_Deps_t, control_Dep,          nanny, "control");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->controlFromBlock_Dep,  parentClone_Deps_t, controlFromBlock_Dep, nanny, "controlFromBlock");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->upboundRequest_Dep,    parentClone_Deps_t, upboundRequest_Dep,   nanny, "upboundRequest");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->serviceRequest_Dep[0], parentClone_Deps_t, serviceRequest_Dep[0],nanny, "serviceRequest[0]");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->serviceRequest_Dep[1], parentClone_Deps_t, serviceRequest_Dep[1],nanny, "serviceRequest[1]");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->serviceRequest_Dep[2], parentClone_Deps_t, serviceRequest_Dep[2],nanny, "serviceRequest[2]");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->serviceRequest_Dep[3], parentClone_Deps_t, serviceRequest_Dep[3],nanny, "serviceRequest[3]");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->serviceRequest_Dep[4], parentClone_Deps_t, serviceRequest_Dep[4],nanny, "serviceRequest[4]");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->serviceRequest_Dep[5], parentClone_Deps_t, serviceRequest_Dep[5],nanny, "serviceRequest[5]");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->serviceRequest_Dep[6], parentClone_Deps_t, serviceRequest_Dep[6],nanny, "serviceRequest[6]");
   INDUCT_DEPENDENCE(myDeps->whoAmI_Dep, myDeps->serviceRequest_Dep[7], parentClone_Deps_t, serviceRequest_Dep[7],nanny, "serviceRequest[7]");

//printf ("Function %36s, File %30s, line %4d, clone=%4d)\n", __func__, __FILE__, __LINE__, meta->cloningState.cloneNum); fflush(stdout);

   meta->cloningState.cloningOpcode = OpcodeUnspecified;
   Frame_Header_t * topOfStack  = (Frame_Header_t *) meta->cloningState.stack;

   topOfStack[1].validate_callers_prep_for_suspension = 1;  // Set "canary trap" for first callee.
   meta->cloningState.returnCanary                    = 1;  // Init the "return canary trap" to "live", to detect a return from a suspendable function via a return stmt instead of the macro.
   parentClone_SoupToNuts(meta, depc, depv);                // Resume (or start, the first time) the algorithm's parentage of 2x2x2 blks.  When we get back, it is either done or needing to clone.
   if (meta->cloningState.returnCanary == 1) {              // If the return canary is "live" the callee must have used a return statement.  We need it to use the macro!
      printf ("ERROR: %s at line %d: Canary trap on RETURN from the callee.  Change callee to return through the NORMAL_RETURN_SEQUENCE macro.\n", __FILE__, __LINE__); fflush(stdout); \
      printf ("ERROR: Other possibility is that callee is not (yet) a SUSPENDABLE function, but it is being identified as such by the caller.\n"); fflush(stdout); \
      *((int *) 123) = 456; \
      ocrShutdown();
   }

   if (topOfStack[1].resumption_case_num == 0) {
      // SoupToNuts just did normal return.  It is time to die.  We just completed the join-clone support for an unrefine operation, or a shutdown propagation.
      if (meta->cloningState.cloningOpcode == Shutdown) {
         int myProngNum = ((meta->xPos & 1)) + ((meta->yPos & 1) << 1) + ((meta->zPos & 1) << 2);
         if ((!ocrGuidIsNull(meta->conveyEighthBlockToJoin_Event[myProngNum]))) {
#ifdef NANNY_ON_STEROIDS
            char nanny[NANNYLEN];
            sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, [prong=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, myProngNum);
#else
#define nanny NULL
#endif
#if 1
            gasket__ocrEventAbandon(&meta->conveyEighthBlockToJoin_Event[myProngNum], __FILE__, __func__, __LINE__, nanny, "conveyEighthBlockToJoin[...]");
#endif
         }
      }
      switch (meta->cloningState.cloningOpcode) {
      case Shutdown:
         gasket__ocrDbDestroy(&myDeps->meta_Dep.guid,     &myDeps->meta_Dep.ptr,     __FILE__, __func__, __LINE__, nanny, "meta");
         gasket__ocrDbDestroy(&myDeps->control_Dep.guid, &myDeps->control_Dep.ptr, __FILE__, __func__, __LINE__, nanny, "control");
   // Defeat OCR's habit of releasing the input ocrEdtDep_t's, because we have changed many of them to other guids, including some events, which simply must not be released.  This is just a debugging expedient.
         REPORT_EDT_DEMISE(myDeps->whoAmI_Dep);
         if (0) {
            myDeps->meta_Dep.guid                = NULL_GUID;
            myDeps->whoAmI_Dep.guid              = NULL_GUID;
            myDeps->blockMeta_Dep.guid           = NULL_GUID;
            myDeps->control_Dep.guid             = NULL_GUID;
            myDeps->controlFromBlock_Dep.guid    = NULL_GUID;
            myDeps->upboundRequest_Dep.guid      = NULL_GUID;
            int i;
            for (i=0; i < 8; i++) myDeps->serviceRequest_Dep[i].guid = NULL_GUID;
         }
         return NULL_GUID;
      case Join:
         gasket__ocrDbDestroy(&myDeps->control_Dep.guid, &myDeps->control_Dep.ptr, __FILE__, __func__, __LINE__, nanny, "control");
   // Defeat OCR's habit of releasing the input ocrEdtDep_t's, because we have changed many of them to other guids, including some events, which simply must not be released.  This is just a debugging expedient.
         REPORT_EDT_DEMISE(myDeps->whoAmI_Dep);
         if (0) {
            myDeps->meta_Dep.guid                = NULL_GUID;
            myDeps->whoAmI_Dep.guid              = NULL_GUID;
            myDeps->blockMeta_Dep.guid           = NULL_GUID;
            myDeps->control_Dep.guid             = NULL_GUID;
            myDeps->controlFromBlock_Dep.guid    = NULL_GUID;
            myDeps->upboundRequest_Dep.guid      = NULL_GUID;
            int i;
            for (i=0; i < 8; i++) myDeps->serviceRequest_Dep[i].guid = NULL_GUID;
         }
         return NULL_GUID;
      default:
         printf ("In Parent, Cloning opcode not yet handled,  opcode = %d / 0x%x\n", meta->cloningState.cloningOpcode, meta->cloningState.cloningOpcode); fflush(stdout);
         *((int *) 123) = 456;
         ocrShutdown();
      }
   }

   // Still more work to do.  Create a clone EDT, cause it to fire, and cause this EDT to terminate.

   switch (meta->cloningState.cloningOpcode) {
   case OpcodeUnspecified:
      printf ("In Parent, Cloning opcode unspecified\n"); fflush(stdout);
      *((int *) 123) = 456;
      ocrShutdown();
      break;
   case SeasoningOneOrMoreDbCreates:
   case ReceivingACommunication:
      // The site that sent us this opcode has put the guid of the events for the remote newResources into the catalog already.  Just drop out of this switch statement and add dependencies of our clone.
      break;
   default:
      printf ("In Parent, Cloning opcode not yet handled,  opcode = %d / 0x%x\n", meta->cloningState.cloningOpcode, meta->cloningState.cloningOpcode); fflush(stdout);
      *((int *) 123) = 456;
      ocrShutdown();
      break;
   }

   meta->cloningState.topPtrAdjRecOffset = ((int) ((unsigned long long) meta->cloningState.topPtrAdjRec) - ((unsigned long long) meta->cloningState.stack));

   ocrGuid_t parentClone_Edt;
#ifdef NANNY_ON_STEROIDS
   sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
   gasket__ocrEdtCreate(&parentClone_Edt,                           // Guid of the EDT created to continue at function blockContinuaiton_Func.
                        SLOT(parentClone_Deps_t, whoAmI_Dep),
                        myParams->parentClone_Template,             // Template for the EDT we are creating.
                        EDT_PARAM_DEF,
                        (u64 *) myParams,
                        EDT_PARAM_DEF,
                        NULL,
#ifdef BIG_SYNC
                        EDT_PROP_FINISH,
                        NULL_HINT,
                        meta->bigSync_Event,
#else
                        EDT_PROP_NONE,
                        NULL_HINT,
                        NULL,
#endif
                        __FILE__,
                        __func__,
                        __LINE__,
                        nanny,
                        "parentClone");

   ADD_DEPENDENCE(         NULL_GUID,                          parentClone_Edt, parentClone_Deps_t, blockMeta_Dep,         DB_MODE_NULL,                 nanny, "blockMeta")
   gasket__ocrDbRelease(   myDeps->control_Dep.guid,                                                                       __FILE__, __func__, __LINE__, nanny, "control");
   ADD_DEPENDENCE(         myDeps->control_Dep.guid,           parentClone_Edt, parentClone_Deps_t, control_Dep,           DB_MODE_RW,                   nanny, "control")
   ADD_DEPENDENCE(         NULL_GUID,                          parentClone_Edt, parentClone_Deps_t, controlFromBlock_Dep,  DB_MODE_NULL,                 nanny, "controlFromBlock")
   ADD_DEPENDENCE(         myDeps->upboundRequest_Dep.guid,    parentClone_Edt, parentClone_Deps_t, upboundRequest_Dep,    DB_MODE_RW,                   nanny, "upboundRequest")
   if (meta->cloningState.cloningOpcode == SeasoningOneOrMoreDbCreates) {
      ADD_DEPENDENCE(      myDeps->serviceRequest_Dep[0].guid, parentClone_Edt, parentClone_Deps_t, serviceRequest_Dep[0], DB_MODE_RO,                   nanny, "serviceRequest[0]")
      ADD_DEPENDENCE(      myDeps->serviceRequest_Dep[1].guid, parentClone_Edt, parentClone_Deps_t, serviceRequest_Dep[1], DB_MODE_RO,                   nanny, "serviceRequest[1]")
      ADD_DEPENDENCE(      myDeps->serviceRequest_Dep[2].guid, parentClone_Edt, parentClone_Deps_t, serviceRequest_Dep[2], DB_MODE_RO,                   nanny, "serviceRequest[2]")
      ADD_DEPENDENCE(      myDeps->serviceRequest_Dep[3].guid, parentClone_Edt, parentClone_Deps_t, serviceRequest_Dep[3], DB_MODE_RO,                   nanny, "serviceRequest[3]")
      ADD_DEPENDENCE(      myDeps->serviceRequest_Dep[4].guid, parentClone_Edt, parentClone_Deps_t, serviceRequest_Dep[4], DB_MODE_RO,                   nanny, "serviceRequest[4]")
      ADD_DEPENDENCE(      myDeps->serviceRequest_Dep[5].guid, parentClone_Edt, parentClone_Deps_t, serviceRequest_Dep[5], DB_MODE_RO,                   nanny, "serviceRequest[5]")
      ADD_DEPENDENCE(      myDeps->serviceRequest_Dep[6].guid, parentClone_Edt, parentClone_Deps_t, serviceRequest_Dep[6], DB_MODE_RO,                   nanny, "serviceRequest[6]")
      ADD_DEPENDENCE(      myDeps->serviceRequest_Dep[7].guid, parentClone_Edt, parentClone_Deps_t, serviceRequest_Dep[7], DB_MODE_RO,                   nanny, "serviceRequest[7]")
   } else {
      EVT_DEPENDENCE(      myDeps->serviceRequest_Dep[0].guid, parentClone_Edt, parentClone_Deps_t, serviceRequest_Dep[0], DB_MODE_RO,                   nanny, "serviceRequest[0]")
      EVT_DEPENDENCE(      myDeps->serviceRequest_Dep[1].guid, parentClone_Edt, parentClone_Deps_t, serviceRequest_Dep[1], DB_MODE_RO,                   nanny, "serviceRequest[1]")
      EVT_DEPENDENCE(      myDeps->serviceRequest_Dep[2].guid, parentClone_Edt, parentClone_Deps_t, serviceRequest_Dep[2], DB_MODE_RO,                   nanny, "serviceRequest[2]")
      EVT_DEPENDENCE(      myDeps->serviceRequest_Dep[3].guid, parentClone_Edt, parentClone_Deps_t, serviceRequest_Dep[3], DB_MODE_RO,                   nanny, "serviceRequest[3]")
      EVT_DEPENDENCE(      myDeps->serviceRequest_Dep[4].guid, parentClone_Edt, parentClone_Deps_t, serviceRequest_Dep[4], DB_MODE_RO,                   nanny, "serviceRequest[4]")
      EVT_DEPENDENCE(      myDeps->serviceRequest_Dep[5].guid, parentClone_Edt, parentClone_Deps_t, serviceRequest_Dep[5], DB_MODE_RO,                   nanny, "serviceRequest[5]")
      EVT_DEPENDENCE(      myDeps->serviceRequest_Dep[6].guid, parentClone_Edt, parentClone_Deps_t, serviceRequest_Dep[6], DB_MODE_RO,                   nanny, "serviceRequest[6]")
      EVT_DEPENDENCE(      myDeps->serviceRequest_Dep[7].guid, parentClone_Edt, parentClone_Deps_t, serviceRequest_Dep[7], DB_MODE_RO,                   nanny, "serviceRequest[7]")
      myDeps->serviceRequest_Dep[0].guid = NULL_GUID;
      myDeps->serviceRequest_Dep[1].guid = NULL_GUID;
      myDeps->serviceRequest_Dep[2].guid = NULL_GUID;
      myDeps->serviceRequest_Dep[3].guid = NULL_GUID;
      myDeps->serviceRequest_Dep[4].guid = NULL_GUID;
      myDeps->serviceRequest_Dep[5].guid = NULL_GUID;
      myDeps->serviceRequest_Dep[6].guid = NULL_GUID;
      myDeps->serviceRequest_Dep[7].guid = NULL_GUID;
   }

   gasket__ocrDbRelease(   myDeps->meta_Dep.guid,                                                                          __FILE__, __func__, __LINE__, nanny, "meta");
   ADD_DEPENDENCE(         myDeps->meta_Dep.guid,              parentClone_Edt, parentClone_Deps_t, meta_Dep,              DB_MODE_RW,                   nanny, "meta")

   REPORT_EDT_DEMISE(myDeps->whoAmI_Dep);
   // Defeat OCR's habit of releasing the input ocrEdtDep_t's, because we have changed many of them to other guids, including some events, which simply must not be released.  This is just a debugging expedient.
   if (0) {
      myDeps->meta_Dep.guid                = NULL_GUID;
      myDeps->whoAmI_Dep.guid              = NULL_GUID;
      myDeps->blockMeta_Dep.guid           = NULL_GUID;
      myDeps->control_Dep.guid             = NULL_GUID;
      myDeps->controlFromBlock_Dep.guid    = NULL_GUID;
      myDeps->upboundRequest_Dep.guid      = NULL_GUID;
      int i;
      for (i=0; i < 8; i++) myDeps->serviceRequest_Dep[i].guid = NULL_GUID;
   }
   return NULL_GUID;

} // parentClone_Func


// **************************************************************************************************************************************************************************************************************
// parentClone_SoupToNuts
//
// This is the top-level function in the calling topology of the several functions that do the actual processing of the parent's service duties.
//
void parentClone_SoupToNuts(ParentMeta_t * meta, u32 depc, ocrEdtDep_t depv[]) {

   typedef struct {
      Frame_Header_t myFrame;
      //double t1, t2;
      DataExchangeOpcode_t opcode;
      ocrGuid_t nextOnDeck_Event[8];
      ocrGuid_t replyToChild_Event[8];
      RefinementDecision_t consensus;
      struct {
         Control_t * control;
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__parentClone_SoupToNuts_t;

#define control              (lcl->pointers.control)
#define opcode               (lcl->opcode)
#define nextOnDeck_Event     (lcl->nextOnDeck_Event)
#define replyToChild_Event   (lcl->replyToChild_Event)
#define consensus            (lcl->consensus)

   int idep;
   parentClone_Deps_t * myDeps = (parentClone_Deps_t *) depv;

//printf ("Function %36s, File %30s, line %4d, myDeps = %p, meta = %p, depc = %d)\n", __func__, __FILE__, __LINE__, myDeps, meta, depc); fflush(stdout);
   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__parentClone_SoupToNuts_t)

//printf ("Function %36s, File %30s, line %4d, clone=%4d)\n", __func__, __FILE__, __LINE__, meta->cloningState.cloneNum); fflush(stdout);

   control = myDeps->control_Dep.ptr;

// Looping, perform the operations requested by the children.  Keep doing so until the final shutdown request arrives.

   do {

      if (ocrGuidIsNull(myDeps->serviceRequest_Dep[0].guid)) {  // NULL_GUID on the Operand dependency signals final shutdown.
         int idep;
         for (idep = 0; idep < 8; idep++) {
            if (!ocrGuidIsNull(myDeps->serviceRequest_Dep[idep].guid)) {
               printf ("Error!  Inconsistency in unrefinement-done signal provided to parentClone\n"); fflush(stdout);
               *((int *) 123) = 456;
               ocrShutdown();
            }
         }
         meta->cloningState.cloningOpcode = Shutdown;
         SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)    // Top-level will sense normal completion, and just end this EDT (without creating any more clones).
      }

// Process the service requested from children.

      opcode = ((DbCommHeader_t *) (myDeps->serviceRequest_Dep[0].ptr))->serviceOpcode;
      for (idep = 1; idep < 8; idep++) {
         if (((DbCommHeader_t *) (myDeps->serviceRequest_Dep[idep].ptr))->serviceOpcode != opcode) {
            *((int *) 123) = 456;   // Service opcodes from children don't match!
         }
      }

//printf ("%s line %d, Parent xPos=%d, yPos=%d, zPos=%d, lvl=%d perfroming service opcode = %d\n", __FILE__, __LINE__, meta->xPos, meta->yPos, meta->zPos, meta->refinementLevel, opcode); fflush(stdout);

      // Operation_AggregateUnrefinementDisp is different than the others. It replies promptly to some of its input prongs, and it then (but just SOMETIMES) propagates a message to its own parent.
      // Though there are some common parts with how the other cases are handled, nevertheless do all its processing distinctly.

      if (opcode == Operation_AggregateUnrefinementDisp) {
         consensus = UNREFINE_BLK;
         RefinementDisposition_t * contribution;
         // Gather the consensus:  Do ALL sibling agree to unrefine?
         for (idep = 0; idep < 8; idep++) {
            contribution = myDeps->serviceRequest_Dep[idep].ptr;
            if (contribution->refinementDisposition != UNREFINE_BLK) {
#define DO_NOT_UNREFINE_BLK (UNREFINE_BLK ^ 0xFF)
               consensus = DO_NOT_UNREFINE_BLK;
            }
         }
         // Tell ANY sibling who was marked UNREFINE_BLK what the consensus of ALL siblings was.  Clean up our operand event, and either clean up the datablock or send it back to the prong, as appropriate
         int sendConsensus = 0;
         if (consensus != UNREFINE_BLK && meta->refinementLevel != 0) {   // We have to tell this parent's parent that its child (me) is marked DO_NOT_UNREFINE_BLK
            sendConsensus = 1;
#ifdef NANNY_ON_STEROIDS
            char nanny[NANNYLEN];
            sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d  sibling[%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, idep);
#else
#define nanny NULL
#endif
            gasket__ocrDbCreate(&myDeps->upboundRequest_Dep.guid, (void **) &myDeps->upboundRequest_Dep.ptr, sizeof_RefinementDisposition_t,   __FILE__, __func__, __LINE__, nanny, "upboundRequest");
         } else {
            myDeps->upboundRequest_Dep.guid = NULL_GUID;
            myDeps->upboundRequest_Dep.ptr  = NULL;
         }

         for (idep = 0; idep < 8; idep++) {
#ifdef NANNY_ON_STEROIDS
            char nanny[NANNYLEN];
            sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d  sibling[%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, idep);
#else
#define nanny NULL
#endif
            contribution = myDeps->serviceRequest_Dep[idep].ptr;
            gasket__ocrEventDestroy(&contribution->dbCommHeader.atBat_Event, __FILE__, __func__, __LINE__, nanny, "serviceRequest[...]");     // Destroy event that brought us our operand
            nextOnDeck_Event[idep] =  contribution->dbCommHeader.onDeck_Event;                                  // Stash for next clone of this parent (other than possible seasoning clone, below).
            ocrGuid_t replyToRequest_Event = contribution->dbCommHeader.serviceReturn_Event;
            if (contribution->refinementDisposition == UNREFINE_BLK && !ocrGuidIsNull(replyToRequest_Event)) {  // If the sibling prong is a real block and needs to hear the consensus back from me
//055 *((int *) 123) = 456; // COVERAGE!
               sendConsensus = 1;
               replyToChild_Event[idep] = replyToRequest_Event;
               gasket__ocrDbDestroy(&myDeps->serviceRequest_Dep[idep].guid, ((void **) (&myDeps->serviceRequest_Dep[idep].ptr)), __FILE__, __func__, __LINE__, nanny, "serviceRequest[...]");
               gasket__ocrDbCreate(&myDeps->replyToChild_Dep[idep].guid, &myDeps->replyToChild_Dep[idep].ptr, sizeof_RefinementDisposition_t,   __FILE__, __func__, __LINE__, nanny, "replyToChild[...]");
            } else {
//051 *((int *) 123) = 456; // COVERAGE!
               gasket__ocrDbDestroy(&myDeps->serviceRequest_Dep[idep].guid, ((void **) (&myDeps->serviceRequest_Dep[idep].ptr)), __FILE__, __func__, __LINE__, nanny, "serviceRequest[...]");
            }
         }

         if (sendConsensus) {
//052 *((int *) 123) = 456; // COVERAGE!
            meta->cloningState.cloningOpcode = SeasoningOneOrMoreDbCreates;
            SUSPEND__RESUME_IN_CLONE_EDT(;)
            RefinementDisposition_t * propagateToParent = myDeps->upboundRequest_Dep.ptr;
            if (propagateToParent != NULL) {
               propagateToParent->refinementDisposition = DO_NOT_UNREFINE_BLK;
               propagateToParent->dbCommHeader.serviceOpcode = Operation_AggregateUnrefinementDisp;
               propagateToParent->dbCommHeader.squawk        = 0xFF03;
               propagateToParent->dbCommHeader.serviceReturn_Event = NULL_GUID;
               ocrGuid_t conveyServiceRequestToParent_Event  = meta->conveyServiceRequestToParent_Event;                         // Jot down the At Bat Event.
               propagateToParent->dbCommHeader.atBat_Event   = conveyServiceRequestToParent_Event;                               // Convey At Bat Event to parent so that he can destroy the event.
#ifdef NANNY_ON_STEROIDS
               char nanny[NANNYLEN];
               sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
               gasket__ocrEventCreate(&meta->conveyServiceRequestToParent_Event, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG, __FILE__, __func__, __LINE__, nanny, "conveyServiceRequestToParent(On Deck after AggDisp)");  // Create the On Deck Event; record it in our meta.
               propagateToParent->dbCommHeader.onDeck_Event  = meta->conveyServiceRequestToParent_Event;                         // Convey On Deck Event to parent so that he can make his clone depend upon it.
               gasket__ocrDbRelease(myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "upboundRequest(AggDisp)");
               gasket__ocrEventSatisfy(conveyServiceRequestToParent_Event, myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "upboundRequest(AggDisp) via conveyServiceRequestToParent");  // Satisfy the parent's serviceRequest datablock dependence.
               myDeps->upboundRequest_Dep.guid      = NULL_GUID;
            }
            for (idep = 0; idep < 8; idep++) {
               RefinementDisposition_t * child = myDeps->replyToChild_Dep[idep].ptr;
               if (child != NULL) {
//056 *((int *) 123) = 456; // COVERAGE!
#ifdef NANNY_ON_STEROIDS
               char nanny[NANNYLEN];
               sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d, [sibling=%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, idep);
#else
#define nanny NULL
#endif
                  child->refinementDisposition = consensus;
                  child->dbCommHeader.serviceOpcode       = Operation_ConsensusOfSiblingSet;
                  gasket__ocrDbRelease(myDeps->replyToChild_Dep[idep].guid, __FILE__, __func__, __LINE__, nanny, "dispConsenusReplyToChild");
                  gasket__ocrEventSatisfy(replyToChild_Event[idep], myDeps->replyToChild_Dep[idep].guid, __FILE__, __func__, __LINE__, nanny, "replyToChild(AggDisp)");  // Satisfy the child's reply
                  myDeps->replyToChild_Dep[idep].guid = NULL_GUID;
               } else {
//053 *((int *) 123) = 456; // COVERAGE!
               }
            }
         } else {
//054 *((int *) 123) = 456; // COVERAGE!
         }

         // Start a cloning request, to obtain the next service request from this parent's 2x2x2 children.

         for (idep = 0; idep < 8; idep++) {
            myDeps->serviceRequest_Dep[idep].guid = nextOnDeck_Event[idep];
         }

         meta->cloningState.cloningOpcode = ReceivingACommunication;
         SUSPEND__RESUME_IN_CLONE_EDT(;)
         continue;

      } // if (opcode == Operation_AggregateUnrefinementDisp)

      if (opcode == Operation_Checksum) {
         CALL_SUSPENDABLE_CALLEE
         checksum_ParentContribution (depc, depv);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
      } else if (opcode == Operation_Plot) {
         CALL_SUSPENDABLE_CALLEE
         plot_ParentContribution(depc, depv);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
//PROFILE:      } else if (opcode == Operation_Profile) {
//PROFILE:         CALL_SUSPENDABLE_CALLEE
//PROFILE:         propagateProfileResultsUpwardFromParent (meta);
//PROFILE:         DEBRIEF_SUSPENDABLE_FUNCTION(;)
      } else if (opcode == Operation_Shutdown) {
         CALL_SUSPENDABLE_CALLEE
         parent_PropagateShutdownMessage(depc, depv);
         DEBRIEF_SUSPENDABLE_FUNCTION(;)
      } else if (opcode == Operation_Join) {
         parent_ProcessJoinForUnrefine(depc, depv);
      } else {
         printf ("Unrecognized opcode received by parentClone\n"); fflush(stdout);
         *((int *) 123) = 456;
         ocrShutdown();
      } // opcode

      // Now for each block, clean up event that brought THIS operand to us, and put "on deck" event into the catalog so that it will be used to plumb operand dependence next time a service request is sought.
      int idep;
      for (idep = 0; idep < 8; idep++) {
         DbCommHeader_t * operand = ((DbCommHeader_t *) (myDeps->serviceRequest_Dep[idep].ptr));
#ifdef NANNY_ON_STEROIDS
         char nanny[NANNYLEN];
         sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d  index=[%d]", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum, idep);
#else
#define nanny NULL
#endif
         gasket__ocrEventDestroy(&operand->atBat_Event, __FILE__, __func__, __LINE__, nanny, "serviceRequest[...]");     // Destroy event that brought us our operand
         ocrGuid_t onDeck = operand->onDeck_Event;
         gasket__ocrDbDestroy(&myDeps->serviceRequest_Dep[idep].guid, &myDeps->serviceRequest_Dep[idep].ptr, __FILE__, __func__, __LINE__, nanny, "serviceRequest[...]");
         myDeps->serviceRequest_Dep[idep].guid = onDeck;
      }

      if (opcode == Operation_Join) {  // If operation was a join ... die!
         meta->cloningState.cloningOpcode = Join;
         SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
      }

      // Start a cloning request, to obtain the next service request from this parent's 2x2x2 children.

      meta->cloningState.cloningOpcode = ReceivingACommunication;
      SUSPEND__RESUME_IN_CLONE_EDT(;)

   } while (1);

   SUSPENDABLE_FUNCTION_EPILOGUE

#undef  control
#undef  opcode
#undef  nextOnDeck
#undef  replyToChild_Event
#undef  consensus

} // parentClone_SoupToNuts



void parent_PropagateShutdownMessage (u32 depc, ocrEdtDep_t depv[]) {
   typedef struct {
      Frame_Header_t myFrame;
      struct {
      } pointers;
      Frame_Header_t calleeFrame;
   } Frame__parent_PropagateShutdownMessage_t;

   parentClone_Deps_t * myDeps  = (parentClone_Deps_t *) depv;
   ParentMeta_t       * meta    = myDeps->meta_Dep.ptr;

   SUSPENDABLE_FUNCTION_PROLOGUE(meta, Frame__parent_PropagateShutdownMessage_t)

#ifdef NANNY_ON_STEROIDS
   char nanny[NANNYLEN];
   sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
   gasket__ocrDbCreate(&myDeps->upboundRequest_Dep.guid, (void **) &myDeps->upboundRequest_Dep.ptr, sizeof_DbCommHeader_t,   __FILE__, __func__, __LINE__, nanny, "upboundRequest");
   meta->cloningState.cloningOpcode = SeasoningOneOrMoreDbCreates;
   SUSPEND__RESUME_IN_CLONE_EDT(;)
   DbCommHeader_t * commHdr = myDeps->upboundRequest_Dep.ptr;

   ocrGuid_t conveyServiceRequestToParent_Event= meta->conveyServiceRequestToParent_Event;                      // Jot down the At Bat Event.
   commHdr->serviceOpcode = Operation_Shutdown;
   commHdr->timeStep      = -9999;
   commHdr->atBat_Event   = conveyServiceRequestToParent_Event;                                                 // Convey At Bat Event to parent so that he can destroy the event.
   meta->conveyServiceRequestToParent_Event = NULL_GUID;                                                        // No new on deck event.  We are done with this parent; he is dying.
   commHdr->onDeck_Event  = meta->conveyServiceRequestToParent_Event;                                           // Convey On Deck Event to parent so that he can make his clone depend upon it.
   gasket__ocrDbRelease(myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "conveyServiceRequestToParent(Shutdown)");
   gasket__ocrEventSatisfy(conveyServiceRequestToParent_Event, myDeps->upboundRequest_Dep.guid, __FILE__, __func__, __LINE__, nanny, "upboundRequest(Shutdown) via conveyServiceRequestToParent");  // Satisfy the parent's serviceRequest datablock dependence.
   myDeps->upboundRequest_Dep.guid = NULL_GUID;
   meta->cloningState.cloningOpcode = Shutdown;

   SUSPENDABLE_FUNCTION_NORMAL_RETURN_SEQUENCE(;)
   SUSPENDABLE_FUNCTION_EPILOGUE

} // parent_PropagateShutdownMessage


void parent_ProcessJoinForUnrefine (u32 depc, ocrEdtDep_t depv[]) {

   parentClone_Deps_t * myDeps  = (parentClone_Deps_t *) depv;
   DbCommHeader_t * commHdr = myDeps->serviceRequest_Dep[0].ptr;
#ifdef NANNY_ON_STEROIDS
   ParentMeta_t       * meta    = myDeps->meta_Dep.ptr;
   char nanny[NANNYLEN];
   sprintf(nanny, "lvl=%2d, xPos=%4d, yPos=%4d, zPos=%4d, clone=%5d", meta->refinementLevel, meta->xPos, meta->yPos, meta->zPos, meta->cloningState.cloneNum);
#else
#define nanny NULL
#endif
   gasket__ocrEventSatisfy(commHdr->parentMetaToJoinClone_Event, myDeps->meta_Dep.guid, __FILE__, __func__, __LINE__, nanny, "upboundRequest(Join) via conveyServiceRequestToParent"); // Satisfy the join-clone's parentMeta_t datablock dependence.
} // parent_ProcessJoinForUnrefine
