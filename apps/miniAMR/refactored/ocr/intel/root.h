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

#ifndef __ROOT_H__
#define __ROOT_H__

typedef struct RootMeta_t RootMeta_t;

// Keep MAX_NUM_UNREFINED_BLOCKS reasonable.  Each unrefined block requires a depencence in the rootClone EDT.  Also, the DbSize and DbEquip for the root has MAX_NUM_UNREFINED_BLOCKS entries.
// 1000 is plenty!  It supports 10x10x10 unrefined blocks.  If you need more, you can always get the same effect by adding more refinement levels.  Remember, you need to compile OCR to support
// ceiling(MAX_NUM_UNREFINED_BLOCKS+6)/64 slot bit vectors.

#define MAX_NUM_UNREFINED_BLOCKS 1000

#include "commhdr.h"
#include "clone.h"
#include "ocrmacs.h"


// root.c          This is the "root progenitor", i.e. the "Adam"-level parent of the entire mesh.
// **************************************************************************************************************************************************************************************************************
// ***************************************************************************************************************************************************************************************
// ***************************************************************************************************************************************************************************************
// mainEdt
//
// This is the EDT that the OCR startup instantiates.  It creates only ONE instance of this EDT.  The name "mainEdt" is essentially just a generic name made necessary by the fact that
// the OCR startup doesn't know the context of what will be done at the very start.  To make this more clear, we will simply make this function call a function whose name imlies what we
// need to do first:  rootLaunch_Func.

ocrGuid_t mainEdt (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);


// **************************************************************************************************************************************************************************************************************
// rootLaunch_Func  -- "Thunked" to by mainEdt, i.e. called as a function directly from mainEdt, NOT created as an EDT therefrom.
// -- Topology:  mainEdt --> rootLaunch(create datablocks) --> rootInit(initialize contents of datablocks, create children) --> rootClone(steady state: perform service requests of children; clone again)
//
// rootLaunch_Func does the following:
//   * Process the command-line arguments only sufficiently to figure out how many objects will be modeled to move through the mesh.  Creates a "scratch" datablock for same.
//   * Creates a "control" datablock for an instance of the shared struct of parsed command line arguments and derived control variables.
//   * Passes control to the EDT that starts at rootInit_Func.

ocrGuid_t rootLaunch_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);

typedef struct {
   ocrEdtDep_t argv_Dep;                    // OCR startup sends us a datablock comprised of the argc argument count followed by the arguments themselves.
} rootLaunch_Deps_t;
#define sizeof_rootLaunch_Deps_t (sizeof(rootLaunch_Deps_t))

// rootLaunch_Func receives a vacuous parameter list.  Don't bother to define a rootLaunch_Params_t struct typedef.


// **************************************************************************************************************************************************************************************************************
// rootInit_Func
// -- Topology:  mainEdt --> rootLaunch(create datablocks) --> rootInit(initialize contents of datablocks, create children) --> rootClone(steady state: perform service requests of children; clone again)
//
// This EDT does the following:
//   * Parses the command line into control_t and allScratchObjects_t.
//   * Creates one instance of blockLaunch for each unrefined block in the problem mesh, of which there are npx*npy*npz of them.  These are the "children" of the root.  Identify position of each by
//     parameters thereto.
//   * Creates the first in the series of rootClone EDTs, to await the first service request from the children.

ocrGuid_t rootInit_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);

typedef struct {
   ocrEdtDep_t whoAmI_Dep;              // Datablock for conveying the guid of the EDT to the EDT, so that it can report about itself in the audit dumps
   ocrEdtDep_t argv_Dep;                // Passed in from mainEdt, OCR startup sends us a datablock comprised of the argc argument count followed by the arguments themselves.
   ocrEdtDep_t meta_Dep;                // My "meta data" mostly comprised of the cloning stack and related support.
   ocrEdtDep_t control_Dep;             // mainEdt also provides the space into which to write the parsed arguments and derived control parameters.  These become READ-ONLY during driveblock processing.
   ocrEdtDep_t scratchAllObjects_Dep;   // mainEdt provides scratch space for writing the descriptions of the object(s) listed in the command line.
} rootInit_Deps_t;
#define sizeof_rootInit_Deps_t  (sizeof(rootInit_Deps_t))
#define countof_rootInit_Deps_t (sizeof_rootInit_Deps_t / depsCountDivisor)

typedef struct {
   u64 dummy_Prm;                             // No parameters to pass to rootInit_Func.
} rootInit_Params_t;
#define sizeof_rootInit_Params_t  (sizeof(rootInit_Params_t))
#define countof_rootInit_Params_t (sizeof_rootInit_Params_t / paramsCountDivisor)


// **************************************************************************************************************************************************************************************************************
// rootClone_Func
// -- Topology:  mainEdt --> rPLaunch(create datablocks) --> rPInit(initialize contents of datablocks, create children) --> rPClone(steady state: perform service requests of children; clone again)
//
// Provide services to the top-level blocks (i.e. the blocks that are totally unrefined, or the parents of refined blocks descended from them.)

ocrGuid_t rootClone_Func (u32 paramc, u64 * paramv, u32 depc, ocrEdtDep_t depv[]);

typedef struct {
   union {
      ocrEdtDep_t firstDependenceSlot;
      ocrEdtDep_t meta_Dep;
   };
   ocrEdtDep_t whoAmI_Dep;              // Datablock for conveying the guid of the EDT to the EDT, so that it can report about itself in the audit dumps
   ocrEdtDep_t control_Dep;
   ocrEdtDep_t goldenChecksum_Dep;
   ocrEdtDep_t scratchChecksum_Dep;
   ocrEdtDep_t serviceRequest_Dep[0];   // INPUT db to the rootClone EDT for the service being requested by the child.  Size determined at run time as npx*npy*npz, and is invariant after initial determination.
} rootClone_Deps_t;
#define sizeof_rootClone_DepsAnnex_t       (sizeof(ocrEdtDep_t)*control->npx*control->npy*control->npz)
#define sizeof_rootClone_Deps_t            (sizeof(rootClone_Deps_t)+sizeof_rootClone_DepsAnnex_t)
#define countof_rootClone_Deps_t           (sizeof_rootClone_Deps_t / depsCountDivisor)
#define countof_rootClone_AnnexDeps_t      (control->npx*control->npy*control->npz)

typedef struct {
   ocrGuid_t rootClone_Template;
   ocrGuid_t scratchAllObjects_dblk;            // Provided as a paramater, so the first instance of this function can clean it up.
   int       isFirstChecksum;                   // True until we record the "golden" checksum, to which all subsequent checksums are compared.
} rootClone_Params_t;
#define sizeof_rootClone_Params_t  (sizeof(rootClone_Params_t))
#define countof_rootClone_Params_t (sizeof_rootClone_Params_t / paramsCountDivisor)

//
// **************************************************************************************************************************************************************************************************************
// rootClone_SoupToNuts
//
// This is the top-level function in the calling topology of the several functions that do the actual processing of the root's service duties.
//
void rootClone_SoupToNuts(rootClone_Params_t * myParams, u32 depc, ocrEdtDep_t depv[], DbSize_t * dbSize);



// ***************************************************************************************************************************************************************************************
// root.c   Other functions.
int check_input(Control_t * control);
void print_help_message();


typedef struct RootMeta_t RootMeta_t;
typedef struct RootMeta_t {

   Clone_t       cloningState;
   DbSize_t      dbSize[MAX_NUM_UNREFINED_BLOCKS];
   unsigned int  satisfySlotInCloneWithDefault;                  // Bit vector of which slots in the clone EDT need fulfillment with the default contents.

} RootMeta_t;
#define sizeof_RootMeta_t              (sizeof(RootMeta_t))

#endif // __ROOT_H__
