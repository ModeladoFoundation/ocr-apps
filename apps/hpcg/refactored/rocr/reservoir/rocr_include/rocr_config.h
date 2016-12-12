#ifndef ROCR_CONFIG_H
#define ROCR_CONFIG_H

// Copyright (C) 2015 Reservoir Labs, Inc. All rights reserved.

//
// __________________________ Backend selection _______________________________
//

/**
 * Id of Intel's reference OCR implementation.
 */
#define ROCR_OCR_BACKEND_INTEL 1

/**
 * Id of PNNL's P-OCR implementation.
 */
#define ROCR_OCR_BACKEND_POCR 2

/**
 * Id of the backend used
 */
#define ROCR_OCR_BACKEND ROCR_OCR_BACKEND_INTEL

//
// __________________________ Sizing and limits _______________________________
//

/**
 * Maximum number of task types.
 */
#define ROCR_MAX_EDT_TYPES 64

/**
 * Maximum number of EDT per task type. This is currently only used for POCR.
 * Please keep this value in synch with the value declared in MMToolbox.
 */
#define ROCR_MAX_EDTS_PER_TYPE 32768
//#define ROCR_MAX_EDTS_PER_TYPE 4096

/**
 * Maximum number of parameters transmitted to an EDT.
 */
#define ROCR_MAX_EDT_PARAMS 32

/**
 * Maximum number of datablock types.
 */
#define ROCR_MAX_DB_TYPES 32

/**
 * Maximum number of datablocks per type.
 */
#define ROCR_MAX_DBS_PER_TYPE 4096

/**
 * Maximum number of data dimensions for the datablock coordinates. This
 * includes the DB type (e.g. scalars have 1 dimension).
 */
#define ROCR_MAX_DB_DIMS 4

/**
 * Maximum number of datablocks that can be enumerated by an EDT.
 */
#define ROCR_MAX_DBS_PER_EDT 256

/**
 * Size of the junk datablock returned if a fetched datablock has not been
 * enumerated. (bytes)
 */
#define ROCR_JUNK_DB_SIZE (100*1024*1024)

/**
 * Maximum number of policy domains. There is usually one policy domain per
 * node.
 */
#define ROCR_MAX_PDS 64

//
// __________________________ Data distribution _______________________________
//

/** No datablock distribution: OCR decides where to place data */
#define ROCR_DB_DISTRIB_AUTO 1

//
// ________________________ Datablock Access Mode _____________________________
//

/** dependence with no DB attached */
#define ROCR_DB_NONE DB_MODE_NULL

/** Read only DB */
#define ROCR_DB_RO DB_MODE_RO

/** Written DB */
#define ROCR_DB_RW DB_MODE_EW   /* here we use EW because the OCR memory model
                                   does not allow concurrent distributed writes
                                   on the same cache line */

//
// __________________________ Optional flags __________________________________
//

// The following options were set when compiling R-Stream OCR


#endif
