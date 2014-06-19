/* Copyright (C) 2012 Reservoir Labs, Inc. All rights reserved. */

// PERFECT rights 

#ifndef __RSTREAM_OCR_H__
#define __RSTREAM_OCR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ocr.h>

#define taskId_t u32
#define AUTO_TASK_ID 2147483647u

// ________________________________ Data blocks ________________________________


/** 
 * Allocates a data block of size {@code len}. The data block is acquired. 
 * @param addr the base address of the block. 
 * @return the data block descriptor. 
 */
ocrGuid_t rocrAlloc(void ** addr, u64 len);


// _____________________________ EDT types _____________________________________
//
//

/**
 * Associates a descriptor to an EDT function, making it an EDT "type". 
 * @param cdlPtr the function
 * @param nbArrayInputs the number of global array base addresses used by the
 * function.
 * @param typeId the ID assigned to the EDT type. 
 * @param numTasks the number of tasks that will be spawned for this task
 * @param isMain whether the declared codelet is the main task (which spawns the
 * slave tasks).
 */
void rocrDeclareType(ocrEdt_t cdlPtr, int typeId, taskId_t numTasks, int isMain);

// __________________________________ EDTs ___________________________________
//
//

/** 
 * Declares a task to be run. 
 * @param cdlType the ID of its type (the function to be run)
 * @param numArgs a data block that contains the numerical arguments to the
 * task.
 * @param taskIndex an ID given to the task (unrelated to its future guid).
 */
void rocrDeclareTask(int cdlType, ocrGuid_t numArgs, taskId_t taskIndex);

/** 
 * Executes the finish EDT. 
 * @param cdlType the ID of its type (the function to be run)
 * @param numArgs a data block that contains the numerical arguments to the
 * task.
 */
void rocrExecute(int typeId, ocrGuid_t numArgs);

// ___________________________ Dependences/Events ____________________________
//
//

/** 
 * Declares that there exists a dependence between tasks {@code taskId1} of type
 * {@code type1} and {@code taskId2} of type {@code type2}.
 */
void rocrDeclareDependence(int type1, taskId_t taskId1, int type2, taskId_t taskId2);


// _________________________ Putting it all together _________________________
// 
// 

/** 
 * Runs the graph declared with {@link rocrDeclareTask}, {@link
 * rocrDeclareTask} and {@link rocrDeclareDependence}.
 */
void rocrScheduleAll();

/**
 * Initializes the runtime. Codelet types have to be declared already. 
 */
void rocrInit();

/** 
 * Cleans up the R-Stream OCR Runtime layer. 
 */
void rocrExit();

// ______________________________ Slave functions ____________________________
//
//

/** 
 * Satisfies this task's successor events. 
 * @param nbInputs the number of input datablocks+pointers given to the EDT 
 *        (its "depc" parameter). 
 * @param inputs the set of input datablocks+pointers to the EDT. 
 */
void rocrSlaveCodeTerm(u32 nbInputs, ocrEdtDep_t * inputs);

/**
 * Gets a void * pointer to the structure that contains the current EDT's
 * numerical arguments.
 */
void * rocrArgs(ocrEdtDep_t inputs[]);

/** 
 * Asynchronous / deferred free. Frees the pointer in rocrExit().
 */
void rocrAsyncFree(void * ptr);

/**  Asycnhronous / deferred DB destroy. */
void rocrAsyncDbDestroy(ocrGuid_t db);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __RSTREAM_OCR_H__
