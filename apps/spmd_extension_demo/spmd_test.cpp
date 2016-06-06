/*
 * This file is subject to the license agreement located in the file
 * LICENSE_UNIVIE and cannot be distributed without it. This notice
 * cannot be removed or modified.
 */

#include "spmd.h"
#include <assert.h>

#define USE_NEW_INTERFACE

struct run_args
{
	run_args() : it(0), event_to_destroy(NULL_GUID) {}
	u64 it;
	ocrGuid_t event_to_destroy;
	ocrGuid_t runTML;
	ocrGuid_t run2TML;
	ocrGuid_t run3TML;
};

#define RUN_ARGS_SIZE ((sizeof(run_args)+sizeof(u64)-1)/sizeof(u64))

ocrGuid_t run(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	assert(paramc == RUN_ARGS_SIZE);
	run_args* args = (run_args*)paramv;
	if (!ocrGuidIsNull(args->event_to_destroy)) ocrEventDestroy(args->event_to_destroy);
	PRINTF("%d: continuation %d\n", (int)spmdMyRank(), (int)(args->it));
	//sleep_seconds(1);
	if (args->it == 0)
	{
		PRINTF("Hello from rank %d of %d\n", (int)spmdMyRank(), (int)spmdSize());
		ocrGuid_t continuation_trigger = NULL_GUID;
		if (spmdMyRank() == 0)
		{
			ocrGuid_t trigger, sync;
			//trigger->send1->sync->send2->continuation_trigger
			ocrEventCreate(&trigger, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
			ocrEventCreate(&sync, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
			ocrEventCreate(&continuation_trigger, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
			//send1
			u64 data = 10;
			PRINTF("%d: will send %d to 1\n", (int)spmdMyRank(), (int)data);
			spmdPSend(1, 0, &data, sizeof(u64), trigger, false, sync);
			//send2
			u64* dataPTR;
			ocrGuid_t dataGUID;
			ocrDbCreate(&dataGUID, (void**)&dataPTR, sizeof(u64), 0, NULL_HINT, NO_ALLOC);
			*dataPTR = 20;
			PRINTF("%d: will send %d to 1\n", (int)spmdMyRank(), (int)*dataPTR);
			ocrDbRelease(dataGUID);
			spmdGSend(1, 0, dataGUID, sync, false, continuation_trigger);
			//trigger the trigger
			ocrEventSatisfy(trigger, NULL_GUID);
		}
		else
		{
			ocrEventCreate(&continuation_trigger, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
			spmdRecv(0, 0, continuation_trigger, NULL_GUID, false, NULL_GUID);
		}
		ocrGuid_t runEDT;
		++args->it;
		args->event_to_destroy = continuation_trigger;
		ocrEdtCreate(&runEDT, args->runTML, RUN_ARGS_SIZE, paramv, 1, 0, EDT_PROP_NONE, NULL_HINT, 0);
		ocrAddDependence(continuation_trigger, runEDT, 0, DB_MODE_CONST);
		return NULL_GUID;
	}
	else if (args->it == 1)
	{
		ocrGuid_t continuation_trigger;
		if (spmdMyRank() == 0)
		{
			ocrEventCreate(&continuation_trigger, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
			spmdRecv(1, 0, continuation_trigger, NULL_GUID, false, NULL_GUID);
		}
		else
		{
			PRINTF("%d: received %d\n", (int)spmdMyRank(), (int)(*(u64*)depv[0].ptr));
			ocrDbDestroy(depv[0].guid);

			ocrEventCreate(&continuation_trigger, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
			spmdRecv(0, 0, continuation_trigger, NULL_GUID, false, NULL_GUID);

			ocrGuid_t dataGUID;
			u64* dataPTR;
			ocrDbCreate(&dataGUID, (void**)&dataPTR, sizeof(u64), 0, NULL_HINT, NO_ALLOC);
			*dataPTR = 30;
			PRINTF("%d: will send %d back to 0\n", (int)spmdMyRank(), (int)*dataPTR);
			ocrDbRelease(dataGUID);
			spmdGSend(0, 0, dataGUID, NULL_GUID, false, NULL_GUID);
		}
		ocrGuid_t runEDT;
		++args->it;
		args->event_to_destroy = continuation_trigger;
		ocrEdtCreate(&runEDT, args->runTML, RUN_ARGS_SIZE, paramv, 1, 0, EDT_PROP_NONE, NULL_HINT, 0);
		ocrAddDependence(continuation_trigger, runEDT, 0, DB_MODE_CONST);
		return NULL_GUID;
	}
	else if (args->it == 2)
	{
		PRINTF("%d: received %d\n", (int)spmdMyRank(), (int)(*(u64*)depv[0].ptr));
		ocrDbDestroy(depv[0].guid);

		ocrGuid_t continuation_trigger, destroy_trigger, start_trigger;
		ocrEventCreate(&continuation_trigger, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
		ocrEventCreate(&destroy_trigger, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
		ocrEventCreate(&start_trigger, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
		ocrGuid_t runEDT;
		++args->it;
		args->event_to_destroy = continuation_trigger;
		ocrEdtCreate(&runEDT, args->runTML, RUN_ARGS_SIZE, paramv, 2, 0, EDT_PROP_NONE, NULL_HINT, 0);
		ocrAddDependence(destroy_trigger, runEDT, 1, DB_MODE_CONST);

		double* dataPTR;
		ocrGuid_t dataGUID;
		ocrDbCreate(&dataGUID, (void**)&dataPTR, 2*sizeof(double), 0, NULL_HINT, NO_ALLOC);
		dataPTR[0] = (double)spmdMyRank();
		dataPTR[1] = 10+ (double)spmdMyRank();
		ocrDbRelease(dataGUID);
		spmdGReduce(SPMD_REDUCE_TYPE_DOUBLE, SPMD_REDUCE_OP_SUM, 2, 0, dataGUID, destroy_trigger, continuation_trigger, start_trigger, false, NULL_GUID);
		ocrAddDependence(continuation_trigger, runEDT, 0, DB_MODE_CONST);
		ocrEventSatisfy(start_trigger, NULL_GUID);
		if (spmdMyRank() != 0) ocrEventSatisfy(continuation_trigger, NULL_GUID);
		return NULL_GUID;
	}
	else if (args->it == 3)
	{
		ocrDbDestroy(depv[1].guid);
		if (depv[0].ptr)
		{
			PRINTF("%d: received %f, %f\n", (int)spmdMyRank(), ((double*)depv[0].ptr)[0], ((double*)depv[0].ptr)[1]);
			ocrDbDestroy(depv[0].guid);
		}
		spmdRankFinalize(NULL_GUID, false);
		return NULL_GUID;
	}
	assert(0);
	return NULL_GUID;
}

ocrGuid_t run2(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	assert(paramc == RUN_ARGS_SIZE);
	run_args* args = (run_args*)paramv;
	if (!ocrGuidIsNull(args->event_to_destroy)) ocrEventDestroy(args->event_to_destroy);
	PRINTF("2-%d: continuation %d\n", (int)spmdMyRank(), (int)(args->it));
	//sleep_seconds(1);
	if (args->it == 0)
	{
		u64 root = 2;
		assert(root < spmdSize());
		if (spmdMyRank() == root)
		{
			ocrGuid_t continuation_trigger, start_trigger;
			ocrEventCreate(&continuation_trigger, OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
			ocrEventCreate(&start_trigger, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
			ocrGuid_t runEDT;
			++args->it;
			args->event_to_destroy = continuation_trigger;
			ocrEdtCreate(&runEDT, args->run2TML, RUN_ARGS_SIZE, paramv, 1, 0, EDT_PROP_NONE, NULL_HINT, 0);

			double* dataPTR;
			ocrGuid_t dataGUID;
			ocrDbCreate(&dataGUID, (void**)&dataPTR, 2 * sizeof(double), 0, NULL_HINT, NO_ALLOC);
			dataPTR[0] = (double)spmdMyRank();
			dataPTR[1] = 10 * (double)spmdMyRank();
			ocrDbRelease(dataGUID);

			spmdDReduce(SPMD_REDUCE_TYPE_DOUBLE, SPMD_REDUCE_OP_SUM, 2, root, dataGUID, continuation_trigger, start_trigger, false, NULL_GUID);
			ocrAddDependence(continuation_trigger, runEDT, 0, DB_MODE_CONST);
			ocrEventSatisfy(start_trigger, NULL_GUID);
			if (spmdMyRank() != root) ocrEventSatisfy(continuation_trigger, NULL_GUID);
		}
		else
		{
			double* dataPTR;
			ocrGuid_t dataGUID;
			ocrDbCreate(&dataGUID, (void**)&dataPTR, 2 * sizeof(double), 0, NULL_HINT, NO_ALLOC);
			dataPTR[0] = (double)spmdMyRank();
			dataPTR[1] = 10 * (double)spmdMyRank();
			ocrDbRelease(dataGUID);

			ocrGuid_t continuation_trigger;
			ocrEventCreate(&continuation_trigger, OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG);
			spmdRankFinalize(continuation_trigger, false);
			spmdDReduce(SPMD_REDUCE_TYPE_DOUBLE, SPMD_REDUCE_OP_SUM, 2, root, dataGUID, NULL_GUID, NULL_GUID, false, continuation_trigger);
		}
		return NULL_GUID;
	}
	else if (args->it == 1)
	{
		PRINTF("2-%d: received %f, %f\n", (int)spmdMyRank(), ((double*)depv[0].ptr)[0], ((double*)depv[0].ptr)[1]);
		ocrDbDestroy(depv[0].guid);
		spmdRankFinalize(NULL_GUID, false);
		return NULL_GUID;
	}
	assert(0);
	return NULL_GUID;
}

ocrGuid_t run3(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	assert(paramc == RUN_ARGS_SIZE);
	run_args* args = (run_args*)paramv;
	if (!ocrGuidIsNull(args->event_to_destroy)) ocrEventDestroy(args->event_to_destroy);
	PRINTF("3-%d: continuation %d\n", (int)spmdMyRank(), (int)(args->it));
	//sleep_seconds(1);
	if (args->it == 0)
	{
		if (spmdMyRank() == 0)
		{
			u64 val = 1;
			ocrGuid_t trigger, connect1, connect2, connect3, connect4;
			ocrEventCreate(&trigger, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
			ocrEventCreate(&connect1, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
			ocrEventCreate(&connect2, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
			ocrEventCreate(&connect3, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
			ocrEventCreate(&connect4, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
			spmdPSend(1, 0, &val, sizeof(u64), trigger, false, connect1);
			val = 2;
			spmdPSend(1, 0, &val, sizeof(u64), connect1, false, connect2);
			val = 3;
			spmdPSend(1, 0, &val, sizeof(u64), connect2, false, connect3);
			val = 4;
			spmdPReduce(SPMD_REDUCE_TYPE_DOUBLE, SPMD_REDUCE_OP_SUM, 1, 1, &val, NULL_GUID, connect3, false, connect4);
			spmdRankFinalize(connect4, false);
			ocrEventSatisfy(trigger, NULL_GUID);
			return NULL_GUID;
		}
		else
		{
			assert(spmdMyRank() == 1);
			ocrGuid_t eventsDBK;
			ocrGuid_t* eventsPTR;
			ocrDbCreate(&eventsDBK, (void**)&eventsPTR, 4 * sizeof(ocrGuid_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC);
			ocrGuid_t connect1, connect2;
			ocrGuid_t cont;
			ocrEventCreate(&eventsPTR[0], OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
			ocrEventCreate(&eventsPTR[1], OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
			ocrEventCreate(&eventsPTR[2], OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
			ocrEventCreate(&eventsPTR[3], OCR_EVENT_STICKY_T, EVT_PROP_TAKES_ARG);
			ocrEventCreate(&connect1, OCR_EVENT_STICKY_T, EVT_PROP_NONE);
			ocrEventCreate(&connect2, OCR_EVENT_STICKY_T, EVT_PROP_NONE);
			spmdRecv(0, 0, eventsPTR[0], NULL_GUID, false, connect1);
			spmdRecv(0, 0, eventsPTR[1], connect1, true, connect2);
			spmdRecv(0, 0, eventsPTR[2], connect2, true, NULL_GUID);
			u64 val = 6;
			spmdPReduce(SPMD_REDUCE_TYPE_DOUBLE, SPMD_REDUCE_OP_SUM, 1, 1, &val, eventsPTR[3], NULL_GUID, false, NULL_GUID);
			++args->it;
			ocrEdtCreate(&cont, args->run3TML, RUN_ARGS_SIZE, (u64*)args, 5, 0, EDT_PROP_NONE, NULL_HINT, 0);
			ocrAddDependence(eventsPTR[0], cont, 0, DB_MODE_CONST);
			ocrAddDependence(eventsPTR[1], cont, 1, DB_MODE_CONST);
			ocrAddDependence(eventsPTR[2], cont, 2, DB_MODE_CONST);
			ocrAddDependence(eventsPTR[3], cont, 3, DB_MODE_CONST);
			ocrDbRelease(eventsDBK);
			ocrAddDependence(eventsDBK, cont, 4, DB_MODE_CONST);
			return NULL_GUID;
		}
	}
	if (args->it == 1)
	{
		assert(spmdMyRank() == 1);
		ocrGuid_t* eventsPTR = (ocrGuid_t*)depv[4].ptr;
		PRINTF("3-%d: received %d, %d, %d, %d\n", (int)spmdMyRank(), (int)*(u64*)depv[0].ptr, (int)*(u64*)depv[1].ptr, (int)*(u64*)depv[2].ptr, (int)*(u64*)depv[3].ptr);
		ocrDbDestroy(depv[0].guid);
		ocrDbDestroy(depv[1].guid);
		ocrDbDestroy(depv[2].guid);
		ocrDbDestroy(depv[3].guid);
		ocrEventDestroy(eventsPTR[0]);
		ocrEventDestroy(eventsPTR[1]);
		ocrEventDestroy(eventsPTR[2]);
		ocrEventDestroy(eventsPTR[3]);
		ocrDbDestroy(depv[4].guid);
		spmdRankFinalize(NULL_GUID, false);
		return NULL_GUID;
	}
	assert(0);
	return NULL_GUID;
}

ocrGuid_t down(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	run_args* args = (run_args*)paramv;
	ocrEdtTemplateDestroy(args->runTML);
	ocrEdtTemplateDestroy(args->run2TML);
	ocrEdtTemplateDestroy(args->run3TML);
	ocrShutdown();
	return NULL_GUID;
}

extern "C" ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
	ocrGuid_t downTMP, downEDT, launcher, join1, join2, join3;
	ocrDbDestroy(depv[0].guid);//we don't need the arguments
	run_args args;
	ocrEdtTemplateCreate(&args.runTML, run, EDT_PARAM_UNK, EDT_PARAM_UNK);
	ocrEdtTemplateCreate(&args.run2TML, run2, EDT_PARAM_UNK, EDT_PARAM_UNK);
	ocrEdtTemplateCreate(&args.run3TML, run3, EDT_PARAM_UNK, EDT_PARAM_UNK);
	ocrEdtTemplateCreate(&downTMP, down, EDT_PARAM_UNK, EDT_PARAM_UNK);
	ocrEventCreate(&join1, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
	ocrEventCreate(&join2, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
	ocrEventCreate(&join3, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
	ocrEdtCreate(&downEDT, downTMP, RUN_ARGS_SIZE, (u64*)&args, 1, 0, EDT_PROP_NONE, NULL_HINT, 0);
	ocrAddDependence(join3, downEDT, 0, DB_DEFAULT_MODE);
	ocrEventCreate(&launcher, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
	ocrDbAccessMode_t modes[] = { DB_MODE_NULL };
	ocrGuid_t deps1[] = { launcher };
	spmdEdtSpawn(args.runTML, 2, RUN_ARGS_SIZE, (u64*)&args, 1, deps1, modes, NULL_HINT, join1);
	ocrGuid_t deps2[] = { join1 };
	spmdEdtSpawn(args.run2TML, 14, RUN_ARGS_SIZE, (u64*)&args, 1, deps2, modes, NULL_HINT, join2);
	ocrGuid_t deps3[] = { join2 };
	spmdEdtSpawn(args.run3TML, 2, RUN_ARGS_SIZE, (u64*)&args, 1, deps3, modes, NULL_HINT, join3);
	ocrAddDependence(NULL_GUID, launcher, 0, DB_DEFAULT_MODE);
	ocrEdtTemplateDestroy(downTMP);
	return NULL_GUID;
}
