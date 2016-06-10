#include <ocr.h>
#include <extensions/ocr-labeling.h>
#include <extensions/ocr-runtime-itf.h>
#ifdef ENABLE_EXTENSION_DEBUG
#include <extensions/ocr-debug.h>
#endif
#ifdef ENABLE_EXTENSION_HETEROGENEOUS_FUNCTIONS
#include <extensions/ocr-heterogeneous.h>
#endif

//Simple example of SPMD support implemented in "user-space" as a library on top of the OCR.
//An EDT may be created with a rank. It's children inherit the rank and Send and Recv
//functions can be used to send messages between ranks. These messages use a matrix
//of channel events to transfer data from rank to rank.
//It uses the metadata library to make the implementation cleaner.
//The libry uses two records in the ELS, so enough space needs to be provided. However, the calls
//to ocrElsUserSet and ocrElsUserGet are intercepted by the library, so that the appication
//can still use all indexes starting from zero.

#ifdef __cplusplus
extern "C" {
#endif

	#define SPMD_ANY_SOURCE ((u64)-2)
	#define SPMD_ANY_TAG ((u32)-2)

	//The following functions have to be called by the application instead of the corresponding OCR functions.
	//They have the same effect as the original calls, but do the extra "magic" required to manage the rank data.
	u8 spmd_ocrEdtTemplateCreate_internal(ocrGuid_t *guid, ocrEdt_t funcPtr, u32 paramc, u32 depc, const char* name);
	u8 spmd_ocrEdtTemplateDestroy(ocrGuid_t guid);
	u8 spmd_ocrEdtCreate(ocrGuid_t * guid, ocrGuid_t templateGuid, u32 paramc, u64* paramv, u32 depc, ocrGuid_t *depv, u16 properties, ocrHint_t* hint, ocrGuid_t *outputEvent);
	void spmd_ocrShutdown();
	ocrGuid_t spmd_ocrElsUserGet(u8 offset);
	void spmd_ocrElsUserSet(u8 offset, ocrGuid_t data);
#ifdef ENABLE_EXTENSION_HETEROGENEOUS_FUNCTIONS
	void spmd_registerEdtFunctions();
	u8 spmd_ocrRegisterEdtFuntion(ocrEdt_t funcPtr);
#endif

	//The library also needs to hijack the mainEdt, therefore the application should define spmd_mainEdt instead.
	//The spmd_mainEdt function will be called by the library after it has initialized it's internal state.
	ocrGuid_t spmd_mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

	//The following functions expose the extra functionality provided by the library.

	//Gets the rank of the current node or (u64)-1 if it doesn't have one
	u64 spmdMyRank();
	//The size of the communicator
	u64 spmdSize();

	//Send modes:
	//P - send pointer + size, blocks until data can be reused
	//G - send DB preserving DB's guid on destination, no completion event, DB does not have to be acquired by sender (can released or not held at all). If the DB is still held by the sender, the recipient may not see the chages made by the sender.
	//C - send DB not preserving DB's guid on destination (making a copy), completion event to signal origin can be reused, DB is not acquired by sender (released or not held at all)
	//R - send DB not preserving DB's guid on destination (making a copy), completion event to signal origin can be reused, DB is still acquired, will be released before call returns (it may not need to, but that would be hard to conditionally use); needs the whole ocrEdtDep_t
	//M - move DB in the most efficient way, either moving the original DB or making a copy and destroying the original; needs the whole ocrEdtDep_t
	//D - similar to G, but destroys the source data block; only available with reduce, not with send

	//Parameters:
	//to, from, tag - the usul ones; receive supports SPMD_ANY_SOURCE and SPMD_ANY_TAG
	//ptr, size - size and pointer to the data
	//dbOrEvent - input data; GUID of a data block or an event, which will provide the data block
	//dbGuid - input data; has to be a data block
	//triggerEvent - an optional event, which has to be satisfied before the operation is put to the queue
	//destroyTriggerEvent - a flag indicating, whether the triggerEvent should be automatically destroyed by the SPMD library using ocrEventDestroy
	//continuationEvent - an optional event, which will be satisfied at some point in the future; any operation invoked after the event will be added to the queue after the current operation
	//reuseEvent - an optional event, which will be satisfied once the input data block can be modified or destroyed without affecting the data received on the other end; (tentative) always happens before satisfaction of the continuation event

	u8 spmdPSend(u64 to, u32 tag, void* ptr, u64 size, ocrGuid_t triggerEvent, bool destroyTriggerEvent, ocrGuid_t continuationEvent);
	u8 spmdGSend(u64 to, u32 tag, ocrGuid_t dbOrEvent, ocrGuid_t triggerEvent, bool destroyTriggerEvent, ocrGuid_t continuationEvent);
	u8 spmdCSend(u64 to, u32 tag, ocrGuid_t dbOrEvent, ocrGuid_t reuseEvent, ocrGuid_t triggerEvent, bool destroyTriggerEvent, ocrGuid_t continuationEvent);//not implemented
	u8 spmdRSend(u64 to, u32 tag, ocrGuid_t dbGuid, ocrGuid_t reuseEvent, ocrGuid_t triggerEvent, bool destroyTriggerEvent, ocrGuid_t continuationEvent);//not implemented
	u8 spmdMSend(u64 to, u32 tag, ocrGuid_t dbGuid);//not implemented

	u8 spmdRecv(u64 from, u32 tag, ocrGuid_t destinationEvent, ocrGuid_t triggerEvent, bool destroyTriggerEvent, ocrGuid_t continuationEvent);

	//Parameters:
	//completionEvent - indicates that the barrier has been reached by all ranks; always comes after continuationEvent
	//note that satisfaction of the continuation event does not necessarily mean that the barrier was reached, it only indicates the order among concurrent collective operations
	u8 spmdBarrier(ocrGuid_t completionEvent, ocrGuid_t triggerEvent, bool destroyTriggerEvent, ocrGuid_t continuationEvent);//not implemented

	enum reduceType_t
	{
		SPMD_REDUCE_TYPE_DOUBLE,
	};

	enum reduceOperation_t
	{
		SPMD_REDUCE_OP_SUM,
	};

	//Parameters:
	//type, operation, count, root - the usual ones
	//rootOutputEvent - (root only) event which will be satisfied by a newly created data block with the result
	u8 spmdPReduce(reduceType_t type, reduceOperation_t operation, u64 count, u64 root, void* data, ocrGuid_t rootOutputEvent, ocrGuid_t triggerEvent, bool destroyTriggerEvent, ocrGuid_t continuationEvent);
	u8 spmdGReduce(reduceType_t type, reduceOperation_t operation, u64 count, u64 root, ocrGuid_t dbOrEvent, ocrGuid_t reuseEvent, ocrGuid_t rootOutputEvent, ocrGuid_t triggerEvent, bool destroyTriggerEvent, ocrGuid_t continuationEvent);
	u8 spmdDReduce(reduceType_t type, reduceOperation_t operation, u64 count, u64 root, ocrGuid_t dbOrEvent, ocrGuid_t rootOutputEvent, ocrGuid_t triggerEvent, bool destroyTriggerEvent, ocrGuid_t continuationEvent);//destroys each source data block
	u8 spmdRReduce(reduceType_t type, reduceOperation_t operation, u64 count, u64 root, ocrGuid_t dbGuid, ocrGuid_t localCompletionEvent, ocrGuid_t rootOutputEvent, ocrGuid_t triggerEvent, bool destroyTriggerEvent, ocrGuid_t continuationEvent);//not implemented
	u8 spmdMReduce(reduceType_t type, reduceOperation_t operation, u64 count, u64 root, ocrGuid_t dbGuid, ocrGuid_t rootOutputEvent, ocrGuid_t triggerEvent, bool destroyTriggerEvent, ocrGuid_t continuationEvent);//not implemented

	u8 spmdEdtSpawn(ocrGuid_t templateGuid, u64 count, u32 paramc, u64* paramv, u32 depc, ocrGuid_t* depv, ocrDbAccessMode_t* modes, /*ocrHint_t*  hint*/ u64 ranksPerAffinity, ocrGuid_t finishEvent);
	u8 spmdRankFinalize(ocrGuid_t triggerEvent, bool destroyTriggerEvent);


	//depv has to be specified
	//possble upgrade: if GUID in depv is a range, each dependence is connected from the item at the corresponding index in the map; this is not possible to implement on top of the current API

	//The following macros "redirect" the normal OCR functions to their library counterparts.
#ifndef SPMD_IMPLEMENTATION
#define ocrEdtTemplateCreate_internal spmd_ocrEdtTemplateCreate_internal
#define ocrEdtTemplateDestroy spmd_ocrEdtTemplateDestroy
#define ocrEdtCreate spmd_ocrEdtCreate
#define ocrShutdown spmd_ocrShutdown
#define mainEdt spmd_mainEdt
#define ocrElsUserGet spmd_ocrElsUserGet
#define ocrElsUserSet spmd_ocrElsUserSet
#define ocrElsUserSet spmd_ocrElsUserSet
#ifdef ENABLE_EXTENSION_HETEROGENEOUS_FUNCTIONS
#define registerEdtFunctions spmd_registerEdtFunctions
#define ocrRegisterEdtFuntion spmd_ocrRegisterEdtFuntion

#endif
#endif

#ifdef __cplusplus
}
#endif

