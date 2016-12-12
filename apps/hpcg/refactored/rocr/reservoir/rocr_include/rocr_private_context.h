/* Copyright (C) 2015 Reservoir Labs, Inc. All rights reserved. */

#ifndef ROCR_PRIVATE_CONTEXT_H
#define ROCR_PRIVATE_CONTEXT_H

#if (ROCR_OCR_BACKEND == ROCR_OCR_BACKEND_INTEL)
#define ENABLE_EXTENSION_PARAMS_EVT
#endif
#include "ocr.h"

#include "rocr_config.h"
#include "rocr_common.h"
#include "rocr_shared_context.h"

/**
 * Defines all the possible kinds of EDT. By convention, we always send the EDT
 * kind as the first parameter in all the created EDTs.
 */
typedef enum {
    EDTK_MAIN_EDT,  /* the first EDT initiating the computation */
    EDTK_REG_EDT,   /* regular computation EDT created during the computation */
    EDTK_EXIT_EDT   /* sink EDT called at the end of the program */
} RocrEDTKind;

/**
 * Definition of the runtime context created for each EDT.
 *
 * This context has the same lifespan as its associated EDT and is never
 * shared. However, its size must be statically defined to ease its use in
 * a datablock.
 *
 * Moreover, to allow an efficient implementation, this context is exposed
 * to the program so that it can directly manipulate it: only use valid C code
 * here.
 */

/**
 * Tuple (DB coordinates, data pointer) plus some extra information.
 */
typedef struct {
    /**
     * Coordinates of the datablock. By convention, we add the DB type as the
     * first coordinate in this array. This is performed during the DB
     * enumeration.
     */
    unsigned long coords[ROCR_MAX_DB_DIMS];

    /**
     * Guid associated to this datablock.
     *
     * From the datablock enumeration to the argument parsing in the EDT related
     * to this context, this is actually the guid of the event satisfied with
     * the datablock.
     *
     * After the argument parsing in the EDT, this guid is set to the actual
     * datablock guid.
     */
    ocrGuid_t guid;

    /**
     * Policy domain on which this datablock is placed. This is an affinity
     * GUID in Intel OCR. In POCR, this field holds the id of the node hosting
     * the datablock.
     */
    ocrPdDesc_t pd;

#if defined(ROCR_TRACE)
    /** Id of the enumerated DB */
    unsigned long id;
#endif

    /**
     * Corresponding data pointer. This is filled during the runtime
     * initialization performed when the EDT starts.
     */
    void *data;

    /** Relative importance of the DB compared to the other DBs */
    long hint;

    /** Size of the enumerated DB (in bytes) */
    unsigned long size;

    /** Whether the enumerated DB will be written */
    unsigned int written;

} RocrPrivateDBDesc;

/**
 * Full EDT-private context.
 */
typedef struct {
    /**
     * All the datablocks enumerated for the EDT. The datablocks appear in the
     * array in the order they have been enumerated by the datablock enumeration
     * function.
     */
    RocrPrivateDBDesc dbs[ROCR_MAX_DBS_PER_EDT];

    /** Backlink to the global shared context */
    RocrSharedCtx *sCtx;

    /* kind of EDT currently running */
    RocrEDTKind myKind;

    /**
     * How many datablocks have been enumerated for this EDT.
     */
    unsigned long nbDBs;

    /**
     * True when the computation is done and the datablocks have been released.
     */
    bool dbReleased;

#if defined(ROCR_TRACE) || (ROCR_OCR_BACKEND == ROCR_OCR_BACKEND_POCR)
    /** Type of the currently running EDT */
    unsigned int myEdtTp;

    /** Id of the currently running EDT */
    unsigned long myEdtId;
#endif

} RocrPrivateCtx;

#endif

