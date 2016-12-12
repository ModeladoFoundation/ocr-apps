/* Copyright (C) 2015 Reservoir Labs, Inc. All rights reserved. */

#ifndef ROCR_SHARED_CONTEXT_H
#define ROCR_SHARED_CONTEXT_H

#include <stdlib.h>

#if (ROCR_OCR_BACKEND == ROCR_OCR_BACKEND_INTEL)
#define ENABLE_EXTENSION_PARAMS_EVT
#endif
#include "ocr.h"

#include "rocr_config.h"
#include "rocr_common.h"

/**
 * Definition of the runtime context shared by all the EDTs.
 *
 * This context is created during the runtime initialization and filled before
 * starting the main EDT task. Afterwards, it is received by all the EDTs but
 * must not be modified. Because it is transmitted in a datablock, this context
 * must be statically allocated.
 *
 * This context is not exposed to the program.
 */

/**
 * Information related to an EDT type.
 */
typedef struct {
#if (ROCR_OCR_BACKEND == ROCR_OCR_BACKEND_POCR)
    /** Reserved GUIDs for the latch events linked to the EDTs of this type */
    ocrGuid_t edts[ROCR_MAX_EDTS_PER_TYPE];
#else
    /** Label map storing the latch event for the EDTs of that type */
    ocrGuid_t edtMap;
#endif

    /** Template for the EDTs of this type */
    ocrGuid_t templ;

#ifdef ROCR_DEBUG
    /** How many EDTs of that kind exist (have been declared) */
    unsigned long numEDTs;
#endif

    /** Flags that apply to all the EDTs of this type */
    uint32_t flags;

} RocrEDTTypeInfo;

/**
 * Information related to a datablock type.
 */
typedef struct {
#if (ROCR_OCR_BACKEND == ROCR_OCR_BACKEND_POCR)
    /** Reserved GUIDs for the events carrying the DBs of this type */
    ocrGuid_t dbs[ROCR_MAX_DBS_PER_TYPE];
#else
    /**
     * Label map storing the DBs of that type. Because of the restrictions of
     * OCR, the map contains sticky events satisfied with a datablock, and not
     * datablocks.
     */
    ocrGuid_t dbMap;
#endif

    /**
     * # of elements in the DB per data dimension. The element at position 0
     * corresponds to the leftmost (largest) C array dimension.
     */
    unsigned long dimSize[ROCR_MAX_DB_DIMS];

    /**
     * # of datablocks per data dimension. The element at position 0
     * corresponds to the leftmost (largest) C array dimension.
     */
    unsigned long dbsPerDim[ROCR_MAX_DB_DIMS];

    /** Actual number of data dimensions for this type. */
    unsigned int dataDims;

} RocrDBTypeInfo;

/**
 * Shared context.
 */
typedef struct {
    /** EDT type description */
    RocrEDTTypeInfo edtTps[ROCR_MAX_EDT_TYPES];

    /** DB type description */
    RocrDBTypeInfo dbTps[ROCR_MAX_DB_TYPES];

    /** Backlink to the GUID of the datablock embedding the context */
    ocrGuid_t guid;

    /** policy domain in which the runtime has been initialized */
    ocrPdDesc_t masterPD;

    /** How many EDT types have been registered so far */
    unsigned int nbEDTTypes;

    /** How many DB types have been registered so far */
    unsigned int nbDBTypes;

    /** Global runtime flags */
    uint32_t flags;

#ifdef ROCR_TRACE
    /** Which mapped function are we currently running? */
    unsigned int generation;
#endif

} RocrSharedCtx;

#endif

