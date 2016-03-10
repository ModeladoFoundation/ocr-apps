#define ENABLE_EXTENSION_LABELING
#define ENABLE_EXTENSION_CHANNEL_EVT
#define ENABLE_EXTENSION_PARAMS_EVT
#define ENABLE_EXTENSION_COUNTED_EVT
#define ENABLE_EXTENSION_RTITF
#include <ocr.h>
#include <extensions/ocr-labeling.h>
#include <extensions/ocr-runtime-itf.h>

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

	//The following functions have to be called by the application instead of the corresponding OCR functions.
	//They have the same effect as the original calls, but do the extra "magic" required to manage the rank data.
	u8 spmd_ocrEdtTemplateCreate_internal(ocrGuid_t *guid, ocrEdt_t funcPtr, u32 paramc, u32 depc, const char* name);
	u8 spmd_ocrEdtTemplateDestroy(ocrGuid_t guid);
	u8 spmd_ocrEdtCreate(ocrGuid_t * guid, ocrGuid_t templateGuid, u32 paramc, u64* paramv, u32 depc, ocrGuid_t *depv, u16 properties, ocrGuid_t affinity, ocrGuid_t *outputEvent);
	void spmd_ocrShutdown();
	ocrGuid_t spmd_ocrElsUserGet(u8 offset);
	void spmd_ocrElsUserSet(u8 offset, ocrGuid_t data);

	//The library also needs to hijack the mainEdt, therefore the application should define spmd_mainEdt instead.
	//The spmd_mainEdt function will be called by the library after it has initialized it's internal state.
	ocrGuid_t spmd_mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

	//The following functions expose the extra functionality provided by the library.

	//The spmdEdtCreateWithRank is used to create an EDT and set it's rank. All children of that EDT will inherit the
	//rank when they are created using ocrEdtCreate (which is mapped to spmd_ocrEdtCreate).
	//It's assumed (but not enforced) that an EDT with a rank won't create an EDT with a different rank.
	u8 spmdEdtCreateWithRank(ocrGuid_t * guid, ocrGuid_t templateGuid, u32 paramc, u64* paramv, u32 depc, ocrGuid_t *depv, u16 properties, ocrGuid_t affinity, ocrGuid_t *outputEvent, u64 rank);
	//Gets the rank of the current node or (u64)-1 if it doesn't have one
	u64 spmdMyRank();
	//The size of the communicator
	u64 spmdSize();
	//The spmdSend sends a data block to a different rank. The precondition may be NULL_GUID or a event that has to be
	//satisfied before the send is performed. The completion event is either NULL or an output parameter that provides
	//an event that will be satisfied once the data block can be reused. Since the event is "once", the precondition
	//needs to be specified, otherwise the event could get triggered and be destroyed even before spmdSend returns.
	//The take_db_ownership arguments instructs the send not to make a copy but rather directly send the data block
	//to the recipient.
	u8 spmdSend(u64 to, ocrGuid_t data, u64 size, ocrGuid_t precondition, ocrGuid_t* completion_event, bool take_db_ownership);
	//The spmdRecv sets up a receive operation, that should match spmdSend on the other rank. Once the data is received,
	//the completion_event returned by the function will be satisfied with the data block. Since the data block was
	//either newly created by the spmdSend or take_db_ownership was specified by the sender, it is usually the recipient's
	//resposibility to dispose of the data block when it is no longer needed. The completion_event is of such a type,
	//that it is guaranteed not to be destroyed until it is used to set up a dependence, but it is either persistent
	//or destroyed automatically. Therefore, the recipient is reponsible for making exactly one
	//ocrAddDependence(*completion_event, ...) call.
	u8 spmdRecv(u64 from, u64 size, ocrGuid_t* completion_event);


	//The following macros "redirect" the normal OCR functions to their library counterparts.
#ifndef SPMD_IMPLEMENTATION
#define ocrEdtTemplateCreate_internal spmd_ocrEdtTemplateCreate_internal
#define ocrEdtTemplateDestroy spmd_ocrEdtTemplateDestroy
#define ocrEdtCreate spmd_ocrEdtCreate
#define ocrShutdown spmd_ocrShutdown
#define mainEdt spmd_mainEdt
#define ocrElsUserGet spmd_ocrElsUserGet
#define ocrElsUserSet spmd_ocrElsUserSet
#endif

#ifdef __cplusplus
}
#endif

