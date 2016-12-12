/* Copyright (C) 2015 Reservoir Labs, Inc. All rights reserved. */

#ifndef ROCR_DBS_H
#define ROCR_DBS_H

#if (ROCR_OCR_BACKEND == ROCR_OCR_BACKEND_INTEL)
#define ENABLE_EXTENSION_PARAMS_EVT
#endif
#include "ocr.h"

#include "rocr_assert.h"
#include "rocr_election.h"
#include "rocr_macros.h"

// exposes the private context to the program to allow the fetch macro
#include "rocr_private_context.h"

// _____________________________ Low-level API ________________________________
//
//

/**
 * Allocates a data block of size {@code len}. The data block is acquired.
 *
 * @param addr the base address of the block.
 *
 * @return the data block descriptor.
 */
ocrGuid_t rocrAlloc(void ** addr, u64 len);

// ____________________________ High-level API ________________________________
//
//

/**
 * Descriptor of a specific DB element. This is used for fetching and accessing
 * DBs in blackboxes. Warning, in this datablock, the coordinates of the DB are
 * stored in a reverse order (coords[0] is the rightmost array dimension).
 */
typedef struct {
    unsigned int id;        /* Id of the db type to fetch */
    unsigned int ndims;     /* # of index expressions set afterwards (# of data
                               dimensions for this DB */
    size_t sz[ROCR_MAX_DB_DIMS]; /* Size in bytes of the partial array defined
                            at each dimension. The last entry is the size of
                            one array element. */
    unsigned long coords[ROCR_MAX_DB_DIMS];  /* Coordinates of the DB to fetch*/
    unsigned long idx[ROCR_MAX_DB_DIMS];     /* Index of the element in the DB*/
} RocrDBDesc;

/**
 * Declares a new datablock type. This must happen with the EDT type
 * declaration, before starting the main EDT.
 *
 * @param ctx The shared context built during the runtime initialization
 * @param dbTp Unique DB type identifier. Must be zero-based.
 * @param dataDims How many data dimension this DB type has.
 * @param ... The number of element along every data dimension, followed by the
 * number of DBs along every data dimension. As unsigned longs.
 */
void rocrDeclareDBType(void *ctx, unsigned int dbTp, unsigned int dataDims,
    ...);

/**
 * Enumerates a datablock: fetch or create it if it does not already exists.
 * The provided EDT private context is also partially updated with the
 * enumerated datablock guid and coordinates. The enumerated datablock is not
 * acquired: never try to access its content.
 *
 * @param ctx the current EDT-private context
 * @param dbType The enumerated datablock type
 * @param dbSize The size in bytes of a single datablock.
 * @param hint A hint stating the relative importance of the datablock
 *  relatively to the other datablocks
 * @param written true when the datablock will be written
 * @param elecFn Election function that elects one of the datablock users
 *  from the datablock coordinates. Can be used to elect an "owner" for the
 *  datablock.
 * @param ... The coordinates of the enumerated datablock. As unsigned longs.
 */
void rocrEnumDB(void *ctx, unsigned int dbType, unsigned long dbSize, long hint,
    unsigned int written, rocrElectionFn elecFn, ...);

/**
 * Fetches an existing datablock among the EDT input data and return the
 * pointer to the data stored in the datablock.
 *
 * @param pCtx private context of the running EDT.
 * @param ptr (output) The pointer to set with the datablock content pointer.
 * @param dims The number of dimensions used to express the datablock
 *  coordinates. That must be one plus the number of data dimensions of the
 *  data in the datablock.
 * @param ... The coordinates of the enumerated datablock (as unsigned longs).
 *  By convention, the datablock type is expected to be added as the first
 *  coordinate.
 */
#define rocrDBCoordFn(_rocr_dimIdx, _rocr_coordVal)\
    (_rocr_ctx->dbs[_rocr_i].coords[_rocr_dimIdx] == (_rocr_coordVal))

#define rocrFetchDB(pCtx, ptr, dims, ...)\
    do {\
        unsigned int _rocr_i, _rocr_found = 0;\
        RocrPrivateCtx *_rocr_ctx = pCtx;\
        for (_rocr_i = 0; !_rocr_found && _rocr_i < _rocr_ctx->nbDBs; ++_rocr_i) {\
            _rocr_found = BUILDEXP(&&, dims, rocrDBCoordFn, __VA_ARGS__);\
        }\
        if (_rocr_found) {\
            *(ptr) = _rocr_ctx->dbs[_rocr_i - 1].data;\
        } else {\
            dbg("Using JUNK buffer!\n");\
            /**((int*) 0) = 0;*/\
            *(ptr) = junkDBBuffer;\
        }\
    } while(0)

/**
 * Fetches an existing datablock among the EDT input data and return the
 * pointer to the data stored in the datablock.
 *
 * @param pCtx private context of the running EDT.
 * @param ptr (output) The pointer to set with the datablock content pointer.
 * @param dbDesc A pointer to a structure describing which DB is fetched.
 */
#define rocrFetchDBDesc(pCtx, ptr, dbDesc)\
    do {\
        unsigned int _rocr_i, _rocr_j, _rocr_found = 0;\
        RocrPrivateCtx *_rocr_ctx = pCtx;\
        for (_rocr_i = 0;\
             !_rocr_found && _rocr_i < _rocr_ctx->nbDBs;\
             ++_rocr_i) {\
            _rocr_found = _rocr_ctx->dbs[_rocr_i].coords[0] == (dbDesc)->id;\
            for (_rocr_j = 0;\
                 _rocr_found && _rocr_j < (dbDesc)->ndims;\
                 ++_rocr_j) {\
                _rocr_found = _rocr_ctx->dbs[_rocr_i].\
                    coords[1 + (dbDesc)->ndims - 1 - _rocr_j] ==\
                    (dbDesc)->coords[_rocr_j];\
            }\
        }\
        if (_rocr_found) {\
            size_t off = 0;\
            for (_rocr_j = 0; _rocr_j < (dbDesc)->ndims; ++_rocr_j) {\
                off += (dbDesc)->idx[_rocr_j] * (dbDesc)->sz[_rocr_j];\
            }\
            *(ptr) = ((uint8_t *) _rocr_ctx->dbs[_rocr_i - 1].data) + off;\
        } else {\
            dbg("Using JUNK buffer!\n");\
            *(ptr) = junkDBBuffer;\
        }\
    } while(0)

extern uint8_t junkDBBuffer[ROCR_JUNK_DB_SIZE];

/**
 * Frees the memory occupied by the designated DB.
 *
 * @param pCtx private context of the running EDT
 * @param dims The number of dimensions used to express the datablock
 *  coordinates. That must be one plus the number of data dimensions of the
 *  data in the datablock.
 * @param ... The coordinates of the enumerated datablock (as unsigned longs).
 *  By convention, the datablock type is expected to be added as the first
 *  coordinate.
 */
#define rocrFreeDB(pCtx, dims, ...)\
    do {\
        unsigned int _rocr_i, _rocr_found = 0;\
        RocrPrivateCtx *_rocr_ctx = pCtx;\
        for (_rocr_i = 0; !_rocr_found && _rocr_i < _rocr_ctx->nbDBs; ++_rocr_i) {\
            _rocr_found = BUILDEXP(&&, dims, rocrDBCoordFn, __VA_ARGS__);\
        }\
        if (_rocr_found) {\
            ocrDbDestroy(_rocr_ctx->dbs[_rocr_i - 1].guid);\
        } else {\
            PRINTF("WARNING: Trying to free a non-enumerated DB!\n");\
        }\
    } while(0)

/**
 * User-defined callbacks to initialize or de-initialize the content of a DB
 *
 * @param data Pointer to the DB content
 * @param size Size of the DB (in bytes)
 * @param dims How many data dimensions exist for this DB
 * @param coord The coordinate (left to right, 0 is leftmost) of the DB along
 * every dimension
 * @param dimSz Number of elements along every data dimension (left to right,
 * 0 is leftmost dimension)
 * @param count The number of datablocks along every dimension (left to right,
 * 0 is leftmost dimension)
 */
typedef void (*userDbCallback)(void *data, unsigned long size,
    unsigned int dims, unsigned long coord[], unsigned long dimSz[],
    unsigned long count[]);

/**
 * Calls a user-defined DB initialization or finalization function.
 *
 * @param ctx private context of the running EDT
 * @param callback The user callback function to call. Can be null.
 * @param dims The number of dimensions used to express the datablock
 * coordinates. That must be one plus the number of data dimensions of the
 * data in the datablock.
 * @param ... The coordinates of the enumerated datablock (as unsigned longs).
 * By convention, the datablock type is expected to be added as the first
 * coordinate.
 */
void rocrUserDBFn(void *ctx, userDbCallback callback, unsigned int dims, ...);

#endif

