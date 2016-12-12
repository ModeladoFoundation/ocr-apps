/* Copyright (C) 2015 Reservoir Labs, Inc. All rights reserved. */

#ifndef ROCR_EDTS_H
#define ROCR_EDTS_H

#include <stdint.h>

#if (ROCR_OCR_BACKEND == ROCR_OCR_BACKEND_INTEL)
#define ENABLE_EXTENSION_PARAMS_EVT
#endif
#include "ocr.h"

#include "rocr_election.h"

//
// ______________________________ Public types ________________________________
//

/**
 * The signature of a function that counts the number of incoming dependences
 * for an EDT.
 *
 * @param coords coordinates of the EDT whose predecessors are counted.
 */
typedef unsigned long (*rocrPredCounterFn)(long* coords);

/**
 * The signature of a function that enumerates the datablocks of an EDT.
 *
 * @param edtCtx EDT-private runtime context, created by the runtime.
 * @param coords coordinates of the EDT whose datablocks are enumerated.
 */
typedef void (*rocrDBEnumFn)(void *edtCtx, long *coords);

//
// ________________________________ EDT data __________________________________
//

/**
 * Returns a pointer on the problem data specified when creating task.
 *
 * @param depv The depv array as specified in OCR EDTs.
 */
void *rocrArgs(ocrEdtDep_t *depv);

//
// ____________________________ EDT type declaration __________________________
//

/**
 * Declares a new kind of EDTs and the number of EDTs of this kind expected to
 * be created during the program execution.
 *
 * @param ctx The shared context built during the runtime initialization
 * @param fn the function executed by those EDTs.
 * @param typeId a unique ID associated to the EDT type.
 * @param numTasks the number of tasks that will be spawned for this task type
 * @param flags OR-ed flags that apply to the EDTs of this type.
 */
void rocrDeclareType(void *ctx, ocrEdt_t fn, unsigned int typeId,
    unsigned long numTasks, uint32_t flags);

/**
 * EDTs of types declared with this flag will run on the policy domain 0, where
 * the original program data can be accessed in memory.
 */
#define ROCR_EDTTP_ONMASTER (1 << 0)

//
// ______________________________ EDT creation ________________________________
//

/**
 * Immediately execute an EDT function with a single input dependence.
 *
 * @param ctx The shared context built during the runtime initialization
 * @param fn The EDT function to run
 * @param numArgs a data block that contains the numerical arguments to the
 *  task. The datablock is released in this function.
 * @param event A pointer to an event Guid_t that will be satisfied once all
 *  the subtasks will be done. On the contrary to what we used to do in the
 *  runtime, a NULL event is simply ignored here.
 */
void rocrExecute(void *ctx, ocrEdt_t fn, ocrGuid_t numArgs, void *event);

/**
 * Creates the task with the given type and id and set it up so that it can
 * start as soon as all its predecessors are done. If the task has no
 * predecessor, it starts immediately. If the task already exists, its latch
 * input counter is decremented instead.
 *
 * @param ctx The private runtime context for the calling EDT.
 * @param edtTp the EDT type ID
 * @param edtId EDT unique identifier (unique among the EDTs of its type). The
 *  id must be within [0..num_declared_tasks_of_this_type[.
 * @param pbData a datablock that will be transmitted to the EDT. The datablock
 *  is released if the task is created or destroyed if the task already exists.
 * @param predCntFn a function counting the number of predecessor for the EDT
 *  to create
 * @param dbEnumFn a function enumerating the coordinates of all the additional
 *  datablocks that are required by the EDT to create.
 * @param a function electing one particular EDT among all the predecessor EDTs
 *  of the auto-dec'ed EDT.
 * @param nParms How many variadic parameters follow
 * @param ... iterator values required by the counting functions. In some sense
 *  the iterator define the task coordinates. As long values.
 */
void rocrAutoDec(void *ctx, unsigned int edtTp, unsigned long edtId,
    ocrGuid_t pbData, rocrPredCounterFn predCntFn, rocrDBEnumFn dbEnumFn,
    rocrElectionFn elecFn, unsigned int nbParams, ...);

//
// ______________________________ EDT callbacks _______________________________
//


/**
 * This callback must be called in every EDT before calling any other runtime
 * function. It basically setups the runtime state for the new EDT.
 *
 * @param paramc the number of parameters to the EDT
 * @param paramv the EDT parameter values
 * @param depc Number of input dependence slots
 * @param depv Input dependences
 *
 * @return the EDT-private context. Will be null for EDTs that are not regular
 *  EDTs.
 */
void *rocrParseInputArgs(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t *depv);

/**
 * Free all the parameters and internal state associated to an EDT. Must be the
 * last runtime function called for this EDT.
 *
 * @param ctx the EDT-specific context allocated earlier.
 * @param paramc The number of elements in paramv.
 * @param paramv carries the codelet type and taskID
 * @param depc Number of input dependence slots
 * @param depv Input dependences
 */
void rocrFreeInputArgs(void *ctx, u32 paramc, u64* paramv, u32 depc,
    ocrEdtDep_t *depv);

#endif

