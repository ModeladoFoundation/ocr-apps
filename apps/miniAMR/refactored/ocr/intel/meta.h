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

#ifndef __META_H__
#define __META_H__

typedef struct Meta_t Meta_t;

#include "control.h"
#include "object.h"
#include "checksum.h"
#include "profile.h"
#include "block.h"
#include <ocr.h>
#include "continuationcloner.h"
#include "dbcommheader.h"



typedef struct RootProgenitorMeta_t RootProgenitorMeta_t;
typedef struct RootProgenitorMeta_t {

   ContinuationCloning_t cloningState;

// Put the datablock catalog last in the meta struct.  (This catalog has an annex whose size is only known after parsing the command line options.)

   union {
      genericDbCatalogEntry_t      dbCatalog[0];
      struct {
         struct {                        // MUST BE FIRST in array of datablocks.
                                         // Consider that THIS STRUCT will be the payload of the meta_dblk datablock.  So the cataloguing of its GUID here is a bit "self-referential".  It is done,
                                         // though, for the benefit of contexts where we would prefer to riffle through the array of datablocks, rather than naming them all in tedious fashion.
            ocrGuid_t              dblk;
            RootProgenitorMeta_t * base;
            int                    size;
            ocrDbAccessMode_t      acMd;
         } metaDb;
         struct {                        // MUST BE SECOND.  Special:  read-only once initialized by blockInit_Func.
            ocrGuid_t              dblk;
            Control_t            * base;
            int                    size;
            ocrDbAccessMode_t      acMd;
         } controlDb;
         struct {                        // This is the biggie, the storage for all the cells of a block in the mesh, and for each cell, all its variables; and it includes space for the halo cells.
            ocrGuid_t              dblk;
            Checksum_t           * base;
            int                    size;
            ocrDbAccessMode_t      acMd;
         } goldenChecksumDb;
         struct {                        // This models the "objects" that move through the mesh.  Every block has a copy of this datablock, though they all update it identically to model the moving objects.
            ocrGuid_t              dblk;
            Checksum_t           * base;
            int                    size;
            ocrDbAccessMode_t      acMd;
         } scratchChecksumDb;
#ifndef ALLOW_DATABLOCK_REWRITES
         struct {                        // If doing single assignment, this will always be provided to an EDT's clone so that meta can migrate thereto and then be updated by the new EDT.
                                         // The mere act of needing to be able to use the Continuation Cloning managed stack means that ALL continuation EDTs will need to manipualate the stack,
                                         // and hence the meta datablock will always need to support updates.  There are also lots of other things in meta, such as timer and counter accumulations.
            ocrGuid_t              dblk;
            RootProgenitorMeta_t * base;
            int                    size;
            ocrDbAccessMode_t      acMd;
         } metaCloneDb;
#endif
         // All remaining datablocks are created only upon demand, and then conveyed immediately to a remote EDT, i.e. to a neighboring block, or to a parent for requesting a service (e.g. checksum).
         struct {
            struct {
               ocrGuid_t           dblk;
               void              * base;
               int                 size;
               ocrDbAccessMode_t   acMd;
            } serviceRequestOperand;
         } annexDb[0];               // Size of the annex struct determined at run time as npx*npy*npz, and is invariant after initial determination.
         // CAUTION:  These must agree in number and order with the dependencies in roogProgenitorContinuation_Deps_t, declared in proto.h
      };
   };

} RootProgenitorMeta_t;
#define sizeof_RootProgenitorMeta_t              (sizeof(RootProgenitorMeta_t)+sizeof(genericDbCatalogEntry_t)*control->npx*control->npy*control->npz)
#define numDatablocksInRootProgenitorMetaCatalog ((sizeof_RootProgenitorMeta_t - offsetof(RootProgenitorMeta_t, dbCatalog[0])) / sizeof(genericDbCatalogEntry_t))



typedef struct BlockMeta_t BlockMeta_t;
typedef struct BlockMeta_t {

   ContinuationCloning_t cloningState;

   ocrGuid_t  labeledGuidRangeForHaloExchange;

   char neighborRefinementLevel[3][2];    // Refinement level of neighbor blocks
   char refinementLevel;
   int xPos;
   int yPos;
   int zPos;
   int cen[3];      // "cen"ter position, as figured in the reference version.

   ocrGuid_t        conveyOperandUpward_Event;                      // Event that child satisfies with Operand_dblk in order to obtain a service from the parent.
   ocrGuid_t        conveyFaceToNeighbor_Event[3][2][4];            // For each of three cardinal directions, times two poles, we have events to send either one full face or four quarter faces.
   ocrGuid_t        onDeckToReceiveFace_Event[3][2][4];             // For each of three cardinal directions, times two poles, the On Deck event whereby we would receive the next furl or qrtr halo face Db.
   //ocrGuid_t        conveyFaceToCoarserNeighbor_Event[3][2];        // For each of six directions, event to send my full face to my coarser-grained neighbor's quarter face.
   //ocrGuid_t        conveyFaceToSameGrainedNeighbor_Event[3][2];    // For each of six directions, event to send my full face to my same-grained neighbor's full face.
   //ocrGuid_t        conveyFaceToFinerNeighbor_Event[3][2][4];       // For each of six directions, event to send my quarter faces to the finer-grained full faces of four different neighbors.


// Put the datablock catalog LAST in the meta struct.

   union {
      genericDbCatalogEntry_t      dbCatalog[0];
      struct {
         struct {                        // MUST BE FIRST in array of datablocks.
                                         // Consider that THIS STRUCT will be the payload of the meta_dblk datablock.  So the cataloguing of its GUID here is a bit "self-referential".  It is done,
                                         // though, for the benefit of contexts where we would prefer to riffle through the array of datablocks, rather than naming them all in tedious fashion.
            ocrGuid_t              dblk;
            BlockMeta_t          * base;
            int                    size;
            ocrDbAccessMode_t      acMd;
         } metaDb;
         struct {                        // MUST BE SECOND.  Special:  read-only once initialized by blockInit_Func.
            ocrGuid_t              dblk;
            Control_t            * base;
            int                    size;
            ocrDbAccessMode_t      acMd;
         } controlDb;
         struct {                        // This is the biggie, the storage for all the cells of a block in the mesh, and for each cell, all its variables; and it includes space for the halo cells.
            ocrGuid_t              dblk;
            Block_t              * base;
            int                    size;
            ocrDbAccessMode_t      acMd;
         } blockDb;
         struct {                        // This models the "objects" that move through the mesh.  Every block has a copy of this datablock, though they all update it identically to model the moving objects.
            ocrGuid_t              dblk;
            AllObjects_t         * base;
            int                    size;
            ocrDbAccessMode_t      acMd;
         } allObjectsDb;
         struct {                        // Performance metrics.
            ocrGuid_t              dblk;
            Profile_t            * base;
            int                    size;
            ocrDbAccessMode_t      acMd;
         } profileDb;
#ifndef ALLOW_DATABLOCK_REWRITES
         struct {                        // If doing single assignment, this will always be provided to an EDT's clone so that meta can migrate thereto and then be updated by the new EDT.
                                         // The mere act of needing to be able to use the Continuation Cloning managed stack means that ALL continuation EDTs will need to manipualate the stack,
                                         // and hence the meta datablock will always need to support updates.  There are also lots of other things in meta, such as timer and counter accumulations.
            ocrGuid_t              dblk;
            Meta_t               * base;
            int                    size;
            ocrDbAccessMode_t      acMd;
         } metaCloneDb;
         struct {                        // If doing single assignment, new storage into which to clone the block_dblk datablock will often be needed (though NOT in all contexts).
            ocrGuid_t              dblk;
            Block_t              * base;
            int                    size;
            ocrDbAccessMode_t      acMd;
         } blockCloneDb;
         struct {                        // At various times, the position of the object(s) needs to be updated, so we need a place to copy the old values so that we can update.
            ocrGuid_t              dblk;
            AllObjects_t         * base;
            int                    size;
            ocrDbAccessMode_t      acMd;
         } allObjectsCloneDb;
         struct {                        // Performance metrics.
            ocrGuid_t              dblk;
            Profile_t            * base;
            int                    size;
            ocrDbAccessMode_t      acMd;
         } profileCloneDb;
#endif
         // All remaining datablocks are created only upon demand, and then conveyed immediately to a remote EDT, i.e. to a neighboring block, or to a parent for requesting a service (e.g. checksum).
         struct {
            ocrGuid_t              dblk;
            void                 * base;
            int                    size;
            ocrDbAccessMode_t      acMd;
         } upboundOperandDb;
         struct {
            ocrGuid_t              dblk;
            Face_t               * base;
            int                    size;
            ocrDbAccessMode_t      acMd;
         } faceDb[2][2][2];          // First dimension: row of quarter face; Second dimension: column of quarter face;  Third dimension: plane (0=0; 1=block_size+1).  For full face, use [0][0][0:1]
         // CAUTION:  These must agree in number and order with the dependencies in blockContinuation_Deps_t, declared in proto.h
      };
   };

} BlockMeta_t;
#define sizeof_BlockMeta_t                   (sizeof(BlockMeta_t))
#define numDatablocksInBlockMetaCatalog      ((sizeof_BlockMeta_t - offsetof(BlockMeta_t, dbCatalog[0])) / sizeof(genericDbCatalogEntry_t))

#endif // __META_H__

