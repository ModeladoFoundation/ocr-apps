/* Copyright (C) 2014 Reservoir Labs, Inc. All rights reserved. */


#ifndef __RSTREAM_SOCR_H__
#define __RSTREAM_SOCR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ocr.h>
#include <stdarg.h>

#define cdlTypeId_t int
#define taskId_t u32
#define AUTO_TASK_ID 2147483647u

/** The signature of a function that counts the number of incoming
 * dependences
 * @param args iterator values are passed as the variable number of arguments
 */
typedef unsigned long (*DepCounterFunction)(va_list args);

// ________________________________ Data blocks ________________________________


/**
 * Allocates a data block of size {@code len}. The data block is acquired.
 * @param addr the base address of the block.
 * @return the data block descriptor.
 */
ocrGuid_t rsocrAlloc(void ** addr, u64 len);


// _____________________________ EDT types _____________________________________
//
//

/**
 * Associates a descriptor to an EDT function, making it a worker EDT.
 * @param cdlPtr the function
 * @param typeId the ID assigned to the EDT type.
 * @param numTasks the number of tasks that will be spawned for this task type
 * @param isMain whether the declared codelet is the main task (which spawns the
 * worker tasks).
 * @param depCounter function pointer for the function that counts the number
 * of incoming dependences for the EDT of type cdlPtr
 */
void rsocrDeclareType(ocrEdt_t cdlPtr, int typeId, taskId_t numTasks,
		     int isMain, DepCounterFunction depCounter);


// __________________________________ EDTs ___________________________________
//
//

/**
 * Declares a task to be run.
 * @param cdlType the ID of its type (the function to be run)
 * @param numArgs a data block that contains the numerical arguments to the
 * task.
 * @param taskId an ID given to the task (unrelated to its future guid).
 * @param ... variable number of arguments: iterator values (task coordinates)
 */

void rsocrCreateTask(int cdlType, ocrGuid_t numArgs, taskId_t taskId, ...);

/**
 * Executes the main EDT.
 * @param synchronous True if the task must be executed synchronously.
 * @param cdlType the ID of its type (the function to be run)
 * @param numArgs a data block that contains the numerical arguments to the
 * task.
 */
void rsocrExecute(bool synchronous, int typeId, ocrGuid_t numArgs);


// ________Initialization and Finalization functions_________________________
//
//


/**
 * Initializes the runtime. Codelet types have to be declared already.
 */
void rsocrInit();

#ifndef ROCR_ASYNCHRONOUS
/**
 * Cleans up the R-Stream OCR Runtime layer.
 */
void rsocrExit();
#endif

// ______________________________ Helper functions ____________________________
//
//

/**
 * Gets a void * pointer to the structure that contains the current EDT's
 * numerical arguments.
 */
void * rsocrArgs(ocrEdtDep_t inputs[]);

/**
 * Asynchronous / deferred free. Frees the pointer in rsocrExit().
 */
void rsocrAsyncFree(void * ptr);

/**  Asycnhronous / deferred DB destroy.
 * @param db the GUID of the data block to be destroyed
 */
void rsocrAsyncDbDestroy(ocrGuid_t db);

/**
 * This callback must be called in every EDT before calling any other runtime
 * function.
 *
 * @param paramac the number of parameters to the EDT
 * @param paramv the EDT parameter values
 * @param nbInputs Number of input dependence slots
 * @param inputs Input dependences
 */
void rsocrParseInputArgs(unsigned int paramc, unsigned long* paramv,
		       unsigned int nbInputs, ocrEdtDep_t inputs[]);

/**
 * @param paramac the number of parameters to the EDT
 * @param paramv character string containing parameters
 * passed to the EDT
 * @param nbInputs Number of input dependence slots
 * @param inputs Input dependences
 */
void rsocrFreeInputArgs(unsigned int paramc, unsigned long* paramv,
		       u32 nbInputs, ocrEdtDep_t inputs[]);


// ______________________________ AutoDec functions ______________________________

/**
 * @param cdlType the EDT type ID
 * @param numArgs the data block that contains the numerical arguments
 * to the task
 * @param taskId the EDT ID
 * @param ... variable number of arguments: iterator values (task coordinates)
 */
void rsocrAutoDec(int cdlType, ocrGuid_t numArgs, taskId_t taskId, ...);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // __RSTREAM_SOCR_H__
