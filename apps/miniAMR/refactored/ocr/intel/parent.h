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

#ifndef __PARENT_H__
#define __PARENT_H__

typedef struct ParentMeta_t ParentMeta_t;

#include "block.h"
#include "commhdr.h"
#include "clone.h"
#include "ocrmacs.h"



// **************************************************************************************************************************************************************************************************************
// parentInit_Func  -- Instantiated by BlockClone EDT, when a block needs to be refined into 2x2x2 children blocks.
// -- Topology:  Init(initialize contents of datablocks, create children) --> Clone(steady state: perform service requests of children; clone again)
//
// This EDT initializes the parent's meta datablock, and then just CALLS the parentClone_Func.  Thereafter, parentClone_Func is created as a clone EDT, for each service that it is to fulfill past the first one.

ocrGuid_t parentInit_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);

typedef struct {
   ocrEdtDep_t meta_Dep;                            // Parent meta datablock.
   ocrEdtDep_t whoAmI_Dep;                          // Datablock for conveying the guid of the EDT to the EDT, so that it can report about itself in the audit dumps
   ocrEdtDep_t blockMeta_Dep;                       // Block meta datablock (valid at first call, for initializing parent meta datablock.  Then destroyed.
   ocrEdtDep_t control_Dep;
   ocrEdtDep_t controlFromBlock_Dep;
   ocrEdtDep_t upboundRequest_Dep;                  // For example, Storage for accumulating the block's contributiton to a checksum or a plot.
   union {
      ocrEdtDep_t serviceRequest_Dep[8];            // INPUT datablock to the parent EDT for the service being requested by the child.  Initially generic, made specific by the opcode it carries.
      ocrEdtDep_t replyToChild_Dep[8];              // Datablock into which to write a reply to the child.
   };
} parentInit_Deps_t;
#define sizeof_parentInit_Deps_t  (sizeof(parentInit_Deps_t))
#define countof_parentInit_Deps_t (sizeof_parentInit_Deps_t / depsCountDivisor)

typedef struct {
   ocrGuid_t parentInit_Template;
   ocrGuid_t parentClone_Template;
   ocrGuid_t allObjects_Dep_ToDestroy;
   ocrGuid_t block_Dep_ToDestroy;
} parentInit_Params_t;
#define sizeof_parentInit_Params_t  (sizeof(parentInit_Params_t))
#define countof_parentInit_Params_t (sizeof_parentInit_Params_t / paramsCountDivisor)


// **************************************************************************************************************************************************************************************************************
// parentClone_Func
// -- Topology:  Launch(create datablocks) --> Init(initialize contents of datablocks, create children) --> Clone(steady state: perform service requests of children; clone again)
//
// Provide services to the 2x2x2 set of child blocks.

ocrGuid_t parentClone_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);

// The params and dependences for this function are identical to those of the parentInit_func.

#define parentClone_Deps_t parentInit_Deps_t
#define sizeof_parentClone_Deps_t sizeof_parentInit_Deps_t
#define countof_parentClone_Deps_t countof_parentInit_Deps_t

#define parentClone_Params_t parentInit_Params_t
#define sizeof_parentClone_Params_t sizeof_parentInit_Params_t
#define countof_parentClone_Params_t countof_parentInit_Params_t


// **************************************************************************************************************************************************************************************************************
// parent_SoupToNuts
//
// This is the top-level function in the calling topology of the several functions that do the actual processing of the parent's service duties.
//
void parentClone_SoupToNuts(ParentMeta_t * meta, u32 depc, ocrEdtDep_t depv[]);


// **************************************************************************************************************************************************************************************************************
// parent_PropagateShutdownMessage
//
// Just propagate the shutdown message from the children blocks up to the root.  All children are reporting being done.
//
void parent_PropagateShutdownMessage (u32 depc, ocrEdtDep_t depv[]);


// **************************************************************************************************************************************************************************************************************
// parent_ProcessJoinForUnrefine
//
// Send the join-clone (unrefine) the stuff that was stashed for it when the corresponding fork-clone (refine) was done.  Then clean up and die.
//
void parent_ProcessJoinForUnrefine (u32 depc, ocrEdtDep_t depv[]);


// **************************************************************************************************************************************************************************************************************
// **************************************************************************************************************************************************************************************************************
// **************************************************************************************************************************************************************************************************************

typedef struct ParentMeta_t ParentMeta_t;
typedef struct ParentMeta_t {

   DbSize_t  dbSize[countof_parentClone_Deps_t];              // Size of the various datablocks manipulated by blockClone_Func EDT (and catalogued in its clone-handling code).
   Clone_t cloningState;

// These are meta data saved by the parent when it is created (at the time of a fork / refine).  Aside from using some of it in debugging messages, it is passed to the join-clone when we unrefine.

   ocrGuid_t conveyServiceRequestToParent_Event;             // Event that child satisfies with Operand_dblk in order to obtain a service from the parent.
   ocrGuid_t conveyEighthBlockToJoin_Event[8];               // Each child will use its element to send blk for unrefine;  The 000 child will plumb these events to its join-clone.
   ocrGuid_t conveyFaceToNeighbor_Event[3][2][4];            // For each of three cardinal directions, times two poles, we have events to send either one full face or four quarter faces.
   ocrGuid_t onDeckToReceiveFace_Event[3][2][4];             // For each of three cardinal directions, times two poles, the On Deck event whereby we would receive the next furl or qrtr halo face Db.
   int       xPos;
   int       yPos;
   int       zPos;
   int       numBaseResBlocks;                               // Number of blocks at "base resolution", i.e. totally unrefined.  It is npx*npy*npz, and is invariant.  Kept for convenience.
   int       cen[3];                                         // "cen"ter position, as figured in the reference version.
   char      refinementLevel;

// CAREFUL:  See notes above!

//   ocrGuid_t  labeledGuidRangeForHaloExchange;

//   char neighborRefinementLevel[3][2];    // Refinement level of neighbor blocks
//   char refinementLevel;
//   int xPos;
//   int yPos;
//   int zPos;
//   int cen[3];      // "cen"ter position, as figured in the reference version.

//   ocrGuid_t    conveyServiceRequestToParent_Event;                   // Event that child satisfies with datablock in order to obtain a service from the parent (or root).
//   DbSize_t     dbSize[countof_parentClone_Deps_t];

//   ocrGuid_t        conveyFaceToNeighbor_Event[3][2][4];            // For each of three cardinal directions, times two poles, we have events to send either one full face or four quarter faces.
//   ocrGuid_t        onDeckToReceiveFace_Event[3][2][4];             // For each of three cardinal directions, times two poles, the On Deck event whereby we would receive the next furl or qrtr halo face Db.
   //ocrGuid_t        conveyFaceToCoarserNeighbor_Event[3][2];        // For each of six directions, event to send my full face to my coarser-grained neighbor's quarter face.
   //ocrGuid_t        conveyFaceToSameGrainedNeighbor_Event[3][2];    // For each of six directions, event to send my full face to my same-grained neighbor's full face.
   //ocrGuid_t        conveyFaceToFinerNeighbor_Event[3][2][4];       // For each of six directions, event to send my quarter faces to the finer-grained full faces of four different neighbors.


} ParentMeta_t;
#define sizeof_ParentMeta_t                   (sizeof(ParentMeta_t))

#endif // __PARENT_H__
