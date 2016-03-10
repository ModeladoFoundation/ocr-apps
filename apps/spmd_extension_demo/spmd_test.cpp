#include "spmd.h"
#include <assert.h>

ocrGuid_t run(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	assert(paramc > 0);
	if (*paramv == 0)
	{
		PRINTF("Hello from rank %d of %d\n", (int)spmdMyRank(), (int)spmdSize());
		ocrGuid_t continuation_trigger;
		if (spmdMyRank() == 0)
		{
			ocrGuid_t dataGUID,trigger;
			ocrEventCreate(&trigger, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
			u64* dataPTR;
			ocrGuid_t sync;
			ocrDbCreate(&dataGUID, (void**)&dataPTR, sizeof(u64), 0, NULL_GUID, NO_ALLOC);
			*dataPTR = 10;
			PRINTF("Will send %d\n", (int)*dataPTR);
			ocrDbRelease(dataGUID);
			spmdSend(1, dataGUID, sizeof(u64), trigger, &sync, true);
			ocrDbCreate(&dataGUID, (void**)&dataPTR, sizeof(u64), 0, NULL_GUID, NO_ALLOC);
			*dataPTR = 20;
			PRINTF("Will send %d\n", (int)*dataPTR);
			ocrDbRelease(dataGUID);
			spmdSend(1, dataGUID, sizeof(u64), sync, &continuation_trigger, true);
			ocrEventSatisfy(trigger, NULL_GUID);
		}
		else
		{
			spmdRecv(0, sizeof(u64), &continuation_trigger);
		}
		ocrGuid_t runTMP, runEDT;
		u64 params[] = { 1 };
		ocrEdtTemplateCreate(&runTMP, run, EDT_PARAM_UNK, EDT_PARAM_UNK);
		ocrEdtCreate(&runEDT, runTMP, 1, params, 1, 0, EDT_PROP_FINISH, NULL_GUID, 0);
		if (spmdMyRank() == 0)
		{
			ocrAddDependence(continuation_trigger, runEDT, 0, DB_MODE_NULL);
		}
		else
		{
			ocrAddDependence(continuation_trigger, runEDT, 0, DB_MODE_CONST);
		}
		ocrEdtTemplateDestroy(runTMP);
		return NULL_GUID;
	}
	else if (*paramv == 1)
	{
		ocrGuid_t continuation_trigger;
		PRINTF("Continuation %d of rank %d of %d\n", (int)(*paramv), (int)spmdMyRank(), (int)spmdSize());
		if (spmdMyRank() == 0)
		{
			spmdRecv(1, sizeof(u64), &continuation_trigger);
		}
		else
		{
			PRINTF("Received %d\n", (int)(*(u64*)depv[0].ptr));
			ocrDbDestroy(depv[0].guid);
			spmdRecv(0, sizeof(u64), &continuation_trigger);
			ocrGuid_t dataGUID;
			u64* dataPTR;
			ocrDbCreate(&dataGUID, (void**)&dataPTR, sizeof(u64), 0, NULL_GUID, NO_ALLOC);
			*dataPTR = 30;
			PRINTF("Will send %d back\n", (int)*dataPTR);
			ocrDbRelease(dataGUID);
			spmdSend(0, dataGUID, sizeof(u64), 0, 0, true);
		}
		ocrGuid_t runTMP, runEDT;
		u64 params[] = { (*paramv) + 1 };
		ocrEdtTemplateCreate(&runTMP, run, EDT_PARAM_UNK, EDT_PARAM_UNK);
		ocrEdtCreate(&runEDT, runTMP, 1, params, 1, &continuation_trigger, EDT_PROP_FINISH, NULL_GUID, 0);
		ocrEdtTemplateDestroy(runTMP);
		return NULL_GUID;
	}
	else if (*paramv == 2)
	{
		PRINTF("Continuation %d of rank %d of %d\n", (int)(*paramv), (int)spmdMyRank(), (int)spmdSize());
		PRINTF("Received %d\n", (int)(*(u64*)depv[0].ptr));
		ocrDbDestroy(depv[0].guid);
		return NULL_GUID;
	}
	assert(0);
	return NULL_GUID;
}

ocrGuid_t down(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	ocrShutdown();
	return NULL_GUID;
}

extern "C" ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
	ocrGuid_t runTMP, downTMP, downEDT, launcher;
	ocrDbDestroy(depv[0].guid);//we don't need the arguments
	ocrEdtTemplateCreate(&runTMP, run, EDT_PARAM_UNK, EDT_PARAM_UNK);
	ocrEdtTemplateCreate(&downTMP, down, EDT_PARAM_UNK, EDT_PARAM_UNK);
	u64 params[] = { 0 };
	ocrEdtCreate(&downEDT, downTMP, 0, 0, (u32)spmdSize(), 0, EDT_PROP_NONE, NULL_GUID, 0);
	ocrEventCreate(&launcher, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
	for (u64 r = 0; r < spmdSize(); ++r)
	{
		ocrGuid_t runEDT, runEVT;
		spmdEdtCreateWithRank(&runEDT, runTMP, 1, params, 1, 0, EDT_PROP_FINISH, NULL_GUID, &runEVT, r);
		ocrAddDependence(runEVT, downEDT, (u32)r, DB_DEFAULT_MODE);
		ocrAddDependence(launcher, runEDT, 0, DB_DEFAULT_MODE);
	}
	ocrAddDependence(NULL_GUID, launcher, 0, DB_DEFAULT_MODE);
	ocrEdtTemplateDestroy(runTMP);
	ocrEdtTemplateDestroy(downTMP);
	return NULL_GUID;
}
