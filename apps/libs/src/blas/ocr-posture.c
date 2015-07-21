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

#include "ocr-posture.h"
#include "ocr-types.h"
#include "ocr.h"
#include <stdio.h>

//static u64 magic = 0xFFFFFFFFFFFFFFFFLL;
//static u64 magic = 0xDad0fFaeDeadBeefLL;
//static u64 magic = 0x2; // FIXME
static u64 magic = 0; // FIXME

#if 0
typedef struct {                      // These are the scalar variables that are passed to the 3d subblock posturing service.
    u64       offset;                 // Byte offset relative to the source (backing-store) datablock's starting address, to the subblock to migrate.
    u64       width;                  // The number of BYTES per row for the subblock to migrate.
    u64       height;                 // The number of rows per plane for the subblock to migrate.
    u64       depth;                  // The number of planes to migrate.
    u64       rowPitch;               // Distance from one row to the next (in bytes).   (I.e. distance from start of one row to start of the next.)
    u64       planePitch;             // Distance from one plane to the next (in bytes). (I.e. distance from start of one plane to start of the next.)
} dma3dBlockToContigParams_t;

typedef struct {                      // These are the dependencies that the dma-inwards portion of the 3D subblock posturing service needs satisfied before it can fire.
    ocrEdtDep_t source;               // The datablock from which the desired subblock is to be migrated closer to the core.
    ocrEdtDep_t dest;                 // The datablock to which the desired subblock is to be migrated closer to the core.
} dma3dBlockToContigDeps_t;
#endif



static bool ocrShouldMigratePostured3dSubblockInward( // Private to OCR, this function does the "magic" to decide whether or not a posturing request should be honored (migrated), or not.
    ocrPosture3dSubblockParams_t     * params,
    ocrPosture3dSubblockInwardDeps_t * deps)
{
//    if (! params->intendToWriteAny) return false; // FIXME
//printf ("considering posturing C.  offset = %ld, height=%ld, width=%ld\n", (u64) params->offset, (u64) params->height, (u64) params->width);
    magic = (magic >> 1) | (magic << 63);
    bool rtnVal = (((s64) magic) < 0);
//    magic &= 0x7FFFFFFFFFFFFFFFLL; // FIXME
    if (params->forceMigration || rtnVal) {
        //printf ("Posturing decision:  migrate the subblock!\n");
        //printf ("Posturing decision:  migrate the subblock!  offset = %ld, height=%ld, width=%ld\n", (u64) params->offset, (u64) params->subblockHeight, (u64) params->subblockWidth);
        return true;
    } else {
        //printf ("Posturing decision:  use the backing store!\n");
        return false;
    }
}



// Application's API into these services:

ocrGuid_t ocrPosture3dSubblockInwardTask (   // This "serivce" EDT is likely to become an OCR primitive, that will "posture" (i.e. ***POTENTIALLY*** migrate) a 3D inputs/outputs
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[])
{
    ocrPosture3dSubblockParams_t     * params = (ocrPosture3dSubblockParams_t     *) paramv;
    ocrPosture3dSubblockInwardDeps_t * deps   = (ocrPosture3dSubblockInwardDeps_t *) depv;

    u64  offset             = params->offset;
    u64  subblockDepth      = params->subblockDepth;
    u64  subblockHeight     = params->subblockHeight;
    u64  subblockWidth      = params->subblockWidth;
    u64  numBytesPerElement = params->numBytesPerElement;
    u64  planePitch         = params->planePitch;
    u64  rowPitch           = params->rowPitch;
    u64  elementPitch       = params->elementPitch;
    u64  estConsumerCount   = params->estConsumerCount;
    u64  estAccessPressure  = params->estAccessPressure;
    bool intendToRead       = params->intendToRead;
    bool intendToWriteAny   = params->intendToWriteAny;
    bool reserveSpaceOnly   = params->reserveSpaceOnly;
    bool forceMigration     = params->forceMigration;

//    printf ("Entered ocrPosture3dSubblockInwardTask. elementLen=%ld width=%ld height=%ld depth=%ld elementPitch=%ld rowPitch=%ld planePitch=%ld\n",
//        (u64) numBytesPerElement, (u64) subblockWidth, (u64) subblockHeight, (u64) subblockDepth, (u64) elementPitch, (u64) rowPitch, (u64) planePitch); fflush(stdout);
    if (! ocrShouldMigratePostured3dSubblockInward(params, deps)) {
//        printf ("        ocrPosture3dSubblockInwardTask exiting without migrating the subblock.\n"); fflush(stdout);
        return deps->backingStore.guid;       // We are NOT migrating the subblock.  Return the backingStore GUID, telling the dependent Edt that it will have to use the subblock's original backing-store.
    }
    if (forceMigration) {
//        printf ("        ocrPosture3dSubblockInwardTask:  migration is being FORCED.\n"); fflush(stdout);
    }

    // We will posture the subblock by migrating it inwards toward the agents that will be using it.  Create the db for the subblock, init it (if necessary), and give the GUID of the db to the dependent EDT..
    ocrGuid_t migratedSubblock; // Guid of the datablock hosting the subblock, if the posturing service decides to migrate it.  Undefined otherwise.
    u8 * destAddr;              // Catch datablock address here.
    ocrDbCreate (&migratedSubblock,                                                   // db:    Return the Guid of the datablock created for the migrated subblock.
                 (void**) &destAddr,                                                  // addr:  Return the address of the datablock created for the migrated subblock.
                 numBytesPerElement * subblockWidth * subblockHeight * subblockDepth, // len:   Amount of storage needed for the subblock.
                 DB_PROP_NONE,                                                        // flags: Flags to create the datablock.
                 NULL_GUID,                                                           // affinity:  GUID to indicate the affinity container of this DB. (TODO)
                 NO_ALLOC);                                                           // allocator: Allocator to use. (TODO)
    if (reserveSpaceOnly) {
//        printf ("        ocrPosture3dSubblockInwardTask exiting;  subblock migrated, but not initialized .\n"); fflush(stdout);
        return migratedSubblock;                                            // No need to initialize the subblock's datablock.
    }

    u8* pIn  = ((u8 *) (deps->backingStore.ptr)) + offset;
    u8* pOut = ((u8 *) (destAddr));
    u64 i, j, k, l;
//printf ("***** Copying from 0x%lx(0x%lx+<0x%lx|%ld>=%f, ht=%ld, wd=%ld, nbpe=%ld, rowPitch=%ld elPitch=%ld\n", (u64) pIn, (u64) (deps->backingStore.ptr), (u64) offset, (u64) offset, *((float*)pIn), (u64) subblockHeight, (u64) subblockWidth, (u64) numBytesPerElement, (u64) rowPitch, (u64) elementPitch);

    for (i = 0; i < subblockDepth; i++) {
        for (j = 0; j < subblockHeight; j++) {
            for (k = 0; k < subblockWidth; k++) {
                for (l = 0; l < numBytesPerElement; l++) {
                    *pOut++ = pIn[(i*planePitch)+(j*rowPitch)+(k*elementPitch)+l];
                }
            }
        }
    }
    return migratedSubblock;                                            // No need to initialize the subblock's datablock.
} // ocrPosture3dSubblockInwardTask

ocrGuid_t ocrPosture3dSubblockOutwardTask (   // This "serivce" EDT is likely to become an OCR primitive, that will "de-posture" a previously inwardly-migrated postured subblock.
    u32 paramc,
    u64 *paramv,
    u32 depc,
    ocrEdtDep_t depv[])
{
//    printf ("Entered ocrPosture3dSubblockOutwardTask.\n"); fflush(stdout);
    ocrPosture3dSubblockParams_t      * params = (ocrPosture3dSubblockParams_t      *) paramv;
    ocrPosture3dSubblockOutwardDeps_t * deps   = (ocrPosture3dSubblockOutwardDeps_t *) depv;

    u64  offset             = params->offset;
    u64  subblockDepth      = params->subblockDepth;
    u64  subblockHeight     = params->subblockHeight;
    u64  subblockWidth      = params->subblockWidth;
    u64  numBytesPerElement = params->numBytesPerElement;
    u64  planePitch         = params->planePitch;
    u64  rowPitch           = params->rowPitch;
    u64  elementPitch       = params->elementPitch;
    u64  estConsumerCount   = params->estConsumerCount;
    u64  estAccessPressure  = params->estAccessPressure;
    bool intendToRead       = params->intendToRead;
    bool intendToWriteAny   = params->intendToWriteAny;
    bool reserveSpaceOnly   = params->reserveSpaceOnly;
    bool forceMigration     = params->forceMigration;

    if (deps->postured.guid == deps->backingStore.guid) {
//        printf ("        ocrPosture3dSubblockOutwardTask exiting;  Postured subblock was NOT migrated, so nothing to do here.\n"); fflush(stdout);
        return NULL_GUID;
    }
    if (intendToWriteAny) {
//        printf ("        ocrPosture3dSubblockOutwardTask           writing migrated datablock contents to backing store.\n"); fflush(stdout);
        u8* pIn  = ((u8 *) (deps->postured.ptr));
        u8* pOut = ((u8 *) (deps->backingStore.ptr)) + offset;
        u64 i, j, k, l;
        for (i = 0; i < subblockDepth; i++) {
            for (j = 0; j < subblockHeight; j++) {
                for (k = 0; k < subblockWidth; k++) {
                    for (l = 0; l < numBytesPerElement; l++) {
                        pOut[(i*planePitch)+(j*rowPitch)+(k*elementPitch)+l] = *pIn++;
                    }
                }
            }
        }
    }
    ocrDbFree(deps->postured.guid, deps->postured.ptr);
//    printf ("        ocrPosture3dSubblockOutwardTask exiting;  migrated subblock has been freed.\n"); fflush(stdout);
    return NULL_GUID;
} // ocrPosture3dSubblockOutwardTask
