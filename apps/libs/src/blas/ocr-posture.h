/**
 * @brief Implementation of API that will evolve into an OCR basic service for "posturing" datablocks.
 *
 * Posture (verb):  To position, especially strategically.  (dictionary.com)
 *
 * "Posturing" means making an intelligent decision about whether subblocks of a large "backing-store"
 * datablock should or should not be migrated into closer-to-the-computing-agent memory for faster,
 * cheaper access to inputs/outputs, and then migrated back out to the backing store.  This file
 * models edt-like API for services that will be provided by OCR, allowing the user application and
 * OCR to enter into a cooperative arrangement about subblock placement in the memory hierarchy.
 *
 * A key goal is to make the application as unaware as possible as to what underlying memory resources
 * exist, much less what current availability of those resources are and how they perform relative to
 * other options.  At the same time, a key goal for OCR is to NOT require it to make an immediate
 * decision about whether (and where) to migrate a postured subblock, but to instead allow OCR to defer
 * that decision until it has decided where the EDT(s) that use(s) the subblock will go.
 *
 **/

/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

#ifndef __OCR_POSTURE_H__
#define __OCR_POSTURE_H__

#include "ocr-types.h"
#include "ocr.h"
#include <stdio.h>
typedef struct {                  // Scalar variables to pass to the 3d subblock posturing service.
    u64   offset;                 // Byte offset relative to backing-store datablock's starting address, to the subblock
                                  // of the workload the consumer will process. (Backing-store DB passed as dependency.)
    u64   subblockDepth;          // The number of planes in the subblock.
    u64   subblockHeight;         // The number of rows in the subblock.
    u64   subblockWidth;          // The number of elements per row in the subblock.
    u64   numBytesPerElement;     // The number of contiguous bytes per element in the subblock.
    u64   planePitch;             // Distance from one plane   to the next (in bytes).  (I.e. from plane   start to plane    start.)
    u64   rowPitch;               // Distance from one row     to the next (in bytes).  (I.e. from row     start to row     start.)
    u64   elementPitch;           // Distance from one element to the next (in bytes).  (I.e. from element start to element start.)
    u64   estConsumerCount;       // Estimate of how many significant (i.e. "workload processing") EDTs will utilize
                                  // the postured subblock.  If "many", OCR might posture to farther-out level, e.g.
                                  // at unit level rather than block level, anticipating utilizing all the XE's of a
                                  // unit for workload processing, rather than just the XE's of a single block.)
    u64   estAccessPressure;      // Estimated access pressure is, roughly, the number of reads and write per element,
                                  // per workload that utilizes the subblock.  It portends to whether it is worthwhile
                                  // to migrate the block.  See intendToWriteAny, below.  Also consider, OCR can compare
                                  // this value to the same in other posturing prospects, to get the most bang for the
                                  // buck as to which subblocks win the "better" memory resources.
    bool  intendToRead;           // The subblock will be used as input.  If it is postured by migrating it towards the
                                  // core, OCR needs to copy it in.
    bool  intendToWriteAny;       // Aside from the obvious relevance regarding copying results outward, additional
                                  // relevance is that if this is false and estAccessPressure is large (perhaps just
                                  // more than one), OCR might choose to ignore estConsumerCount, and migrate copise of
                                  // the subblock in to multiple memories.  (e.g, citing the example mentioned at comment
                                  // by estConsumerCount, OCR might migrate a copy of the subblock to all the islands
                                  // that process workloads consuming this subblock.)
    bool  reserveSpaceOnly;       // Set true only if ALL of the subblock will be written, and if any of the elements are
                                  // read, they will be written first.  Saves copying the backing store to the subblock.
    bool  forceMigration;         // Set true if posturing service MUST elect to migrate the subblock.  Helpful to assure
                                  // that the subblock gets migrated at least once, e.g. to assure it gets transposed (with
                                  // transposeIfMigrated is set true).
} ocrPosture3dSubblockParams_t;

typedef struct {                      // Dependencies the 3D subblock posturing service needs satisfied before it can fire.
    ocrEdtDep_t backingStore;         // The datablock from which the desired subblock is to (potentially) be migrated
                                      // closer to the computing agent(s) that will host the EDT(s) that consume it.
    ocrEdtDep_t optionalTriggerEvent; // Optional triggering event, for instance to impose some serialization that can make
                                      // execution order more deterministic which can make debugging easier (sometimes!).
                                      // (Satisfy with NULL_GUID when not needed.)
} ocrPosture3dSubblockInwardDeps_t;

typedef struct {                      // Dpendencies the 3D subblock posturing service needs satisfied before it can fire.
    ocrEdtDep_t postured;             // The datablock from which the desired subblock is to (potentially) be migrated back
                                      // out to the backing store. (If NULL_GUID, there is no subblock to move back.)
    ocrEdtDep_t backingStore;         // The datablock to which the desired subblock is to be migrated back out.
    ocrEdtDep_t doneWithPosturedDb;   // Trigger event, telling outward-migration EDT that the datablock containing the
                                      // migrated subblock should now be postured back to the backing store.
    ocrEdtDep_t optionalTriggerEvent; // Optional triggering event, for instance to impose some serialization that can make
                                      // execution order more deterministic which can make debugging easier (sometimes!).
                                      // (Satisfy with NULL_GUID when not needed.)} ocrPosture3dSubblockOutwardDeps_t;
} ocrPosture3dSubblockOutwardDeps_t;

ocrGuid_t ocrPosture3dSubblockInwardTask (   // This "serivce" EDT is likely to become an OCR primitive, that will
                                             // "posture" (i.e. ***POTENTIALLY*** migrate) a 3D subblock.
                                             // Returns GUID of subblock DB if subblock is migrated, else guid of backingStore.
    u32 paramc,  u64 *paramv,                // Fullfilled with an instance of ocrPosture3dSubblockParams_t.  See above.
    u32 depc,    ocrEdtDep_t depv[]);        // Fullfilled with an instance of ocrPosture3dInwardDeps_t.  See above.

ocrGuid_t ocrPosture3dSubblockOutwardTask (  // This "serivce" EDT is likely to become an OCR primitive, that will "de-
                                             // posture" a previously inwardly-migrated postured subblock.  Does nothing
                                             // of subblock to migrate is NULL_GUID (meaning it wasn't migrated inward).
    u32 paramc,  u64 *paramv,                // Fullfilled with an instance of ocrPosture3dSubblockParams_t.  See above.
    u32 depc,    ocrEdtDep_t depv[]);        // Fullfilled with an instance of ocrPosture3dOutwardDeps_t.  See above.

#endif /*__OCR_POSTURE_H__*/
