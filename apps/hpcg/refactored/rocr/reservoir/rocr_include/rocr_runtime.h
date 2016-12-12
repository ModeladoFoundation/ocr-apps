/* Copyright (C) 2015 Reservoir Labs, Inc. All rights reserved. */

#ifndef ROCR_RUNTIME_H
#define ROCR_RUNTIME_H

#include <stdint.h>

/**
 * Set when the main EDT only creates predecessor-less EDTs. When the flag is
 * missing, the runtime layer will assume that all the EDTs are created/auto-
 * dec'ed by the main codelet.
 */
#define ROCR_FLAG_REDUCED_EDT_CREATE (1u << 0)

/**
 * Set when the program relies on the election of a predecessor EDT to create
 * the EDTs and DBs. This is typically the case for POCR but not for the Intel
 * backend.
 */
#define ROCR_FLAG_ELECTIVE (1u << 1)

/**
 * Initializes the runtime. Must be called before anything else in the runtime.
 *
 * @param nbEDTTypes How many EDT types will be declared
 * @param nbDBTypes How many DB types will be declared
 * @param flags Some options to use in this context. This must be an or-ed
 * combination of ROCR_FLAG* constants.
 *
 * @return a new empty shared context.
 */
void *rocrInit(unsigned int nbEDTTypes, unsigned int nbDBTypes,
    uint32_t flags);

/**
 * Cleans up the R-Stream OCR Runtime layer. You can safely call this function
 * once you have started the main EDT, without having to wait for all the
 * tasks to end, the runtime will handle that properly.
 *
 * @param the shared context returned by rocrInit.
 */
void rocrExit(void *ctx);

/**
 * Checks whether the runtime layer was compiled for the given OCR
 * implementation. The program aborts if the runtime was not compiled for the
 * provided implementation.
 *
 * @param ocr The OCR implementation expected by the program. Must be one of the
 * constants starting by OCR_BACKEND_.
 */
void rocrCheckOCRVersion(int ocr);

/**
 * Registers a malloc'ed address to be freed at the end of the program execution
 *
 * @param addr The address to be freed when the program ends.
 */
void rocrAsyncFree(void *addr);

#endif

