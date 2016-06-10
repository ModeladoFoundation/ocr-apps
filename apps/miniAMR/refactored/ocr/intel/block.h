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

#ifndef __BLOCK_H__
#define __BLOCK_H__

typedef struct BlockMeta_t BlockMeta_t;
typedef struct Block_t Block_t;


#include "commhdr.h"
#include "clone.h"
#include "ocrmacs.h"


// block.c         The functions in block.c create a launch, init, and a series of clones.  Each instance of same is responsible for processing a single block of the mesh, at some refinement level.
// ***************************************************************************************************************************************************************************************
// ***************************************************************************************************************************************************************************************
// **************************************************************************************************************************************************************************************************************
// blockLaunch_Func
//
// This just creates the blockInit EDT and the datablocks that it will need to initialize:  control_dblk, allObjects_dblk.  Data to develop the block's meta datablock is passed as params.
// It then passes control to the blockInit EDT.
ocrGuid_t blockLaunch_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);

typedef struct {
   ocrEdtDep_t whoAmI_Dep;               // Datablock for conveying the guid of the EDT to the EDT, so that it can report about itself in the audit dumps
   ocrEdtDep_t control_Dep;
   ocrEdtDep_t allObjects_Dep;
} blockLaunch_Deps_t;
#define sizeof_blockLaunch_Deps_t  (sizeof(blockLaunch_Deps_t))
#define countof_blockLaunch_Deps_t (sizeof_blockLaunch_Deps_t / depsCountDivisor)

typedef struct {
   ocrGuid_t  blockLaunch_Template;
   ocrGuid_t  blockInit_Template;
   ocrGuid_t  blockClone_Template;
   ocrGuid_t  parentInit_Template;
   ocrGuid_t  parentClone_Template;
} blockTemplates_t;

typedef struct {
   blockTemplates_t template;
   int              xPos;
   int              yPos;
   int              zPos;
   ocrGuid_t        conveyServiceRequestToParent_Event;  // Event that child satisfies with databloc to obtain a service from the parent. (Applicable first time; thereafter, child provides next event to parent.)
   ocrGuid_t        labeledGuidRangeForHaloExchange;
} blockLaunch_Params_t;
#define sizeof_blockLaunch_Params_t  (sizeof(blockLaunch_Params_t))
#define countof_blockLaunch_Params_t (sizeof_blockLaunch_Params_t / paramsCountDivisor)


// **************************************************************************************************************************************************************************************************************
// blockInit_Func
//
// This initializes control_dblk, allObjects_dblk, and meta_dblk, then creates all other datablocks that will be needed to process the block, then passes control to the blockClone EDT.
ocrGuid_t blockInit_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);

typedef struct {
   ocrEdtDep_t meta_Dep;                // My "meta data" about block location, refinement level, guids and pointers of other datablocks I and my clones will use, and my stack for managing cloning.
   ocrEdtDep_t whoAmI_Dep;              // Datablock for conveying the guid of the EDT to the EDT, so that it can report about itself in the audit dumps
   ocrEdtDep_t initialControl_Dep;      // Root sends us the Control_dblk.  The underlying content is read-only, but we copy it anyway so that we are sure that it resides in our policy domain.
   ocrEdtDep_t control_Dep;             // Storage for my copy of the above.
   ocrEdtDep_t initialAllObjects_Dep;   // Root sends us its AllObjects_dblk.  All siblings will get their own copy, and even though we will track object movement identically, it is best to have our own copy.
   ocrEdtDep_t allObjects_Dep;          // Storage for my copy of the above.
   ocrEdtDep_t block_Dep;               // Space into which one totally-unrefined block of the mesh domain is initialized.
} blockInit_Deps_t;
#define sizeof_blockInit_Deps_t  (sizeof(blockInit_Deps_t))
#define countof_blockInit_Deps_t (sizeof_blockInit_Deps_t / depsCountDivisor)

typedef blockLaunch_Params_t blockInit_Params_t;     // The parameter list for blockInit is identical to that of blockLaunch.
#define sizeof_blockInit_Params_t  sizeof_blockLaunch_Params_t
#define countof_blockInit_Params_t countof_blockLaunch_Params_t



// **************************************************************************************************************************************************************************************************************
// blockClone_Func
//
// This is the code that performs the algorithm on a block at the leaf of the refinement level depth tree.  It advances the algorithm as far as it can, to such point where a communication is needed from
// another EDT (such as a halo exchange from a neighbor, or such as the results of a service requested of the parent), at which time it clones, and the clone continues where the predecessor left off.  This
// particular function just runs the loop that drives the cloning logic.  It calls block_SoupToNuts, which is the top function of the calling topology of functions that do the work.
ocrGuid_t blockClone_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);

typedef struct {
   union {
      ocrEdtDep_t firstDependenceSlot;
      ocrEdtDep_t meta_Dep;                              // Meta data that tracks and controls progress of the algorithm.
   };
   ocrEdtDep_t whoAmI_Dep;                               // Datablock for conveying the guid of the EDT to the EDT, so that it can report about itself in the audit dumps
   ocrEdtDep_t metaPred_Dep;                             // Meta data of the predecessor clone when doing a fork, or of the [0][0][0] predecessor clone when doing a join.
   ocrEdtDep_t control_Dep;                              // The parsed and derived controls.  This is READ-ONLY to this function, and its subsequent clones, except when doing a fork!
   ocrEdtDep_t controlPred_Dep;                          // Control data of the predecessor clone when doing a fork.
   ocrEdtDep_t block_Dep;                                // The block proper.
   ocrEdtDep_t blockPred_Dep;                            // The block proper of the predecessor when doing a fork.
   ocrEdtDep_t allObjects_Dep;                           // The object(s) being modeled as they migrate through the mesh domain.
   ocrEdtDep_t allObjectsPred_Dep;                       // allObjects data of the predecessor clone when doing a fork.
   union {
      ocrEdtDep_t upboundRequest_Dep;                    // For example, Storage for accumulating the block's contributiton to a checksum or a plot.
      ocrEdtDep_t parentMeta_Dep;                        // At a join-clone (for unrefine), there is no upboundRequest_Dep, so alias here.  This is where parent sends stuff it stashed at fork (refine).
   };
   ocrEdtDep_t replyFromRequest_Dep;                     // Reply from an upbound service request.  Doesn't happen much.  Presently receives reply about sibling's concensus as to whether to unrefine.
   ocrEdtDep_t aux_Dep[0];                               // Auxiliary dependences, the number of which depends on the context.
} blockClone_Deps_t;

// Implicit is that, when using neighborDispositionIn_Dep, the above struct is extended by 16 more elements.
#define  sizeof_blockClone_Deps_t          (sizeof(blockClone_Deps_t))
#define countof_blockClone_fixedDeps_t     (sizeof_blockClone_Deps_t / depsCountDivisor)

typedef struct {
   blockTemplates_t template;
   int              prongNum;                            // -1 when NOT forking;  0:7 when forking in order to effect the Refine operation, wherein one block becomes 2x2x2 child blocks.
   ocrGuid_t        conveyServiceRequestToParent_Event;  // Event that child satisfies with datablock to obtain a service from the parent. (This field is applicable when we are doing the Fork; else undefined.)
   ocrGuid_t        conveyEighthBlockToJoin_Event[8];    // Each child will use its element to send blk for unrefine;  The 000 child will plumb these events to its join-clone.
   ocrGuid_t        parentMetaToJoinClone_Event;         // Event that carries parentMeta to a join-clone.  Sent as a param so that clone can destroy the event.  Undefined for other kinds of clones.
} blockClone_Params_t;
#define sizeof_blockClone_Params_t  (sizeof(blockClone_Params_t))
#define countof_blockClone_Params_t (sizeof_blockClone_Params_t / paramsCountDivisor)


// **************************************************************************************************************************************************************************************************************
// blockClone_SoupToNuts
//
// This is the top-level function in the calling topology of the several functions that do the actual processing of a block.
//
void blockClone_SoupToNuts (blockClone_Params_t * myParams, u32 depc, ocrEdtDep_t depv[]);


// **************************************************************************************************************************************************************************************************************
// **************************************************************************************************************************************************************************************************************
// **************************************************************************************************************************************************************************************************************
// **************************************************************************************************************************************************************************************************************
// **************************************************************************************************************************************************************************************************************

// blockMetaDb is the "meta" datablock applicable to the functions which process a "block" of the problem domain.  (This is as contrasted to the different kinds of meta datablock needed for the RootService
// functions and the ParentService functions.)  The meta datablock is comprised of the meta-data that evolves over the course of the program run.  This is as opposed to that which is invariant after
// the command-line has been processed, which are in controlDb.

typedef struct BlockMeta_t {

// CAREFUL:  There are TWO sections to this material: those values that are captured by the parent at the time of the fork (refine) and propagated to the join (unrefine);  and those that evolve during
//           the lifetime of the refined block, and are passed by prong000 to the join-clone.  When adding variables, be circumspect about which kind they are.  Be sure to examine the code that propagates
//           them, in the parentInit_Func, and in the blockClone_Func where joins are processes.  It would be preferable to encapsulate these two sections in structs of their own, and then union a named
//           instance (for the above propagations) and an anonymous instance (for general usage without having to fully qualify).  But anonymous structs (unlike anonymous unions) are NOT supported
//           ubiquitously in current C compilers.

// See above!  These are the "ElementsConveyedFromProng000ToMergedBlockAtJoint"
   unsigned char neighborRefinementLevel[3][2];              // Refinement level of neighbor blocks:
#define NEIGHBOR_IS_OFF_EDGE_OF_MESH (0xF0)                  //     Normally, this value is from 0 to 15 inclusive, indicating the true refinement level of the block.
                                                             //     Exception is that a non-existent neighbor (because it is off the edge of the mesh domain) is indicated by NEIGHBOR_IS_OFF_EDGE_OF_MESH
                                                             //     Special case:  During refinement, we use the upper four bits as a special bit vector.  Consider this scenario:
                                                             //     --  I am a block at level=5.  Off my East edge, the data is finer, i.e. it is four blocks at level=6.  (If it is finer, that is the only
                                                             //         possible case.)
                                                             //     --  Now consider that one or more of those four neighbors are slated to be refined, i.e. to level=7.  That means I MUST refine (to level=6).
                                                             //         I will become eight children, four of whom are at the East edge, facing these four neighbors.  The thing is, SOME of those four neighbors
                                                             //         MIGHT be refining, but not necessarily ALL of them.  (It could be NONE of them.  Consider that I might be getting forced to refine because
                                                             //         this situation I am desribing is actually being driven by refinement decisions of my finer neighbors to the North.)
                                                             //     --  In just this case, I therefore use the upper four bits of neighborRefinementLevel to record which of the four finer neighbors are going
                                                             //         to refine (to level=7), versus which are going to remain at their current refinement (level=6).  When I do the Fork Clone (i.e. the
                                                             //         refinement split of myself into eight finer blocks), I use the appropriate bit to adjust the refinement level I have recorded for each
                                                             //         prong-child's neighbor(s).
                                                             //     Note that there is NEVER a case where these high-order bits might be ALL set AND the low order bits indicate a refinement level=0.  Therefore,
                                                             //     there is no conflict between this encoding and the value of NEIGHBOR_IS_OFF_EDGE_OF_MESH.
                                                             //     Note also that if, for some reason, 16 refinement levels isn't enough, this char would need to be enlarged and reorganized, and then the
                                                             //     upper limit would be 21 levels, because 8^21 == 2^63, approaching the limits of 64-bit integers for cataloguing block identities, and the
                                                             //     capacity of labeled guids used in this OCR version.
   unsigned char finerNeighborNotUnrefiningAndNotTalkingAnyMore; // Bit mask of non-sibling neighbors that have already told me they are finer than me and won't unrefine.  See step 1 and 5 of unrefine code in
                                                                 // refine.c
   DbSize_t  dbSize[countof_blockClone_fixedDeps_t];         // Size of the various datablocks manipulated by blockClone_Func EDT (and catalogued in its clone-handling code).
   Clone_t   cloningState;
   ocrGuid_t labeledGuidRangeForHaloExchange;
   ocrGuid_t blockClone_Edt;                                 // Guid of the EDT created to clone the current EDT, for continuation, fork, or join.

// See above!  These are the "ElementsPreservedByParentAtForkAndSuppliedToMergedBlockAtJoin_Macro"
   ocrGuid_t conveyServiceRequestToParent_Event;             // Event that child satisfies with Operand_dblk in order to obtain a service from the parent.
   ocrGuid_t conveyEighthBlockToJoin_Event[8];               // Each child will use its element to send blk for unrefine;  The 000 child will plumb these events to its join-clone.
   ocrGuid_t conveyFaceToNeighbor_Event[3][2][4];            // For each of three cardinal directions, times two poles, we have events to send either one full face or four quarter faces.
   ocrGuid_t onDeckToReceiveFace_Event[3][2][4];             // For each of three cardinal directions, times two poles, the On Deck event whereby we would receive the next furl or qrtr halo face Db.
   int       xPos;
   int       yPos;
   int       zPos;
   int       numBaseResBlocks;                               // Number of blocks at "base resolution", i.e. totally unrefined.  It is npx*npy*npz, and is invariant.  Kept for convenience.
   int       cen[3];                                         // "cen"ter position, as figured in the reference version.
   unsigned char refinementLevel;
} BlockMeta_t;
#define sizeof_BlockMeta_t                   (sizeof(BlockMeta_t))


// Datablock in which a "block" of the mesh domain is represented.  This is comprised of the cells of all the variables of the block, including halo cells.

typedef struct Block_t {
   double          cells[0];
   unsigned char   neighborRefinementLevel_East;   // Used to convey refinement level of East neighbor of prong111 to the aggregation of an unrefine (i.e. join).
   unsigned char   neighborRefinementLevel_North;  //                                    North
   unsigned char   neighborRefinementLevel_Up;     //                                    Upward
   unsigned char   finerNeighborNotUnrefiningAndNotTalkingAnyMore; // Bit mask of non-sibling neighbors that have already told me they are finer than me and won't unrefine.  See step 1 and 5 of unrefine code in
} Block_t;

#define sizeof_Block_t (sizeof(Block_t) + ((control->x_block_size + 2) * (control->y_block_size + 2) * (control->z_block_size + 2) * control->num_vars * sizeof(double)))

typedef struct Face_t {
   DbCommHeader_t  dbCommHeader;
   double          cells[0];
} Face_t;

#define sizeof_Face_BasePart_t (sizeof(Face_t))

#endif // __BLOCK_H__
