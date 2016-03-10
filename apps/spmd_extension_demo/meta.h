#define ENABLE_EXTENSION_LABELING
#define ENABLE_EXTENSION_CHANNEL_EVT
#define ENABLE_EXTENSION_PARAMS_EVT
#define ENABLE_EXTENSION_COUNTED_EVT
#define ENABLE_EXTENSION_RTITF
#include <ocr.h>
#include <extensions/ocr-labeling.h>
#include <extensions/ocr-runtime-itf.h>

//Support for storing user's metadata alongside OCR objects. Supports only EDT templates and is very inefficient.
//To use, include the meta.h header instead of the normal OCR headers.
//The libry uses two records in the ELS, so enough space needs to be provided. However, the calls
//to ocrElsUserSet and ocrElsUserGet are intercepted by the library, so that the appication
//can still use all indexes starting from zero.

#ifdef __cplusplus
extern "C" {
#endif

	//The following functions have to be called by the application instead of the corresponding OCR functions.
	//They have the same effect as the original calls, but do the extra "magic" required to provide the metadata.
	u8 meta_ocrEdtTemplateCreate_internal(ocrGuid_t *guid, ocrEdt_t funcPtr, u32 paramc, u32 depc, const char* name);
	u8 meta_ocrEdtTemplateDestroy(ocrGuid_t guid);
	u8 meta_ocrEdtCreate(ocrGuid_t *guid, ocrGuid_t templateGuid, u32 paramc, u64* paramv, u32 depc, ocrGuid_t *depv, u16 properties, ocrGuid_t affinity, ocrGuid_t *outputEvent);
	void meta_ocrShutdown();
	ocrGuid_t meta_ocrElsUserGet(u8 offset);
	void meta_ocrElsUserSet(u8 offset, ocrGuid_t data);

	//The library also needs to hijack the mainEdt, therefore the application should define meta_mainEdt instead.
	//The meta_mainEdt function will be called by the library after it has initialized it's internal state.
	ocrGuid_t meta_mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]);

	//The following functions expose the extra functionality provided by the library.
	u8 metaCreateKey(ocrGuid_t *guid, ocrGuidUserKind kind, u64 max_data_size);
	u8 metaAssignData(ocrGuid_t object, ocrGuid_t key, void* data, u64 actual_data_size);
	u8 metaGetData(ocrGuid_t object, ocrGuid_t key, void** data);

	//The following macros "redirect" the normal OCR functions to their library counterparts.
#ifndef META_IMPLEMENTATION
#define ocrEdtTemplateCreate_internal meta_ocrEdtTemplateCreate_internal
#define ocrEdtTemplateDestroy meta_ocrEdtTemplateDestroy
#define ocrEdtCreate meta_ocrEdtCreate
#define mainEdt meta_mainEdt
#define ocrShutdown meta_ocrShutdown
#define ocrElsUserGet meta_ocrElsUserGet
#define ocrElsUserSet meta_ocrElsUserSet
#endif

#ifdef __cplusplus
}
#endif

