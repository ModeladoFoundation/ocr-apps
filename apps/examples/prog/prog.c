#include <stdio.h>
#include <ocr.h>

#define NUM_WORKERS 5
#define WORK 100000
#define ITERS 10
#define BRANCH_AFTER 3

// param 0: event to satisfy
ocrGuid_t workerEdt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	printf("worker\n");
	int i;
	int j = 0;
	for(i = 1; i < WORK; i++) j *= 2;

	ocrGuid_t evt = paramv[0];
	ocrEventSatisfy(evt, NULL_GUID);
	return NULL_GUID;
}


// dep 0: event
// param 0: count
// param 0: 1 if branch, 0 if main
ocrGuid_t syncTask( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	u64 count = paramv[0];
	u64 isMain = paramv[1];
	ocrGuid_t syncEdtTemplate;
	ocrGuid_t workerEdtTemplate;
	ocrGuid_t syncEdt;
	ocrGuid_t workerEdts[NUM_WORKERS];
	u64 workerParamv[NUM_WORKERS];
	ocrGuid_t workerStartEvents[NUM_WORKERS];
	ocrGuid_t workerEvents[NUM_WORKERS];
	int i;

	if(count == ITERS) {
		printf("done \n");
		sleep(3);
		ocrShutdown();
		return NULL_GUID;
	}

	// create sync template
	ocrEdtTemplateCreate(&syncEdtTemplate, syncTask, 2, NUM_WORKERS);

	if(isMain && count % BRANCH_AFTER == BRANCH_AFTER - 1) {
		ocrGuid_t newBranchSyncTemplate;
		ocrGuid_t newBranchEvent;
		ocrGuid_t newBranchSyncEdt;
		ocrEdtTemplateCreate(&newBranchSyncTemplate, syncTask, 2, 1);
		ocrEventCreate(&newBranchEvent, OCR_EVENT_ONCE_T, false);

		u64 branchSyncParamv[2] = {0, 0};
		ocrEdtCreate(&newBranchSyncEdt, newBranchSyncTemplate, EDT_PARAM_DEF, branchSyncParamv,
				 EDT_PARAM_DEF, &newBranchEvent, 0, NULL_GUID, NULL);
		ocrEventSatisfy(newBranchEvent, NULL_GUID);
	}

	// create worker template
	ocrEdtTemplateCreate(&workerEdtTemplate, workerEdt, 1, 1);

	// events for workers to satisfy
	for(i = 0; i < NUM_WORKERS; i++)
		ocrEventCreate(&workerEvents[i], OCR_EVENT_ONCE_T, false);

	// events to start workers
	for(i = 0; i < NUM_WORKERS; i++)
		ocrEventCreate(&workerStartEvents[i], OCR_EVENT_ONCE_T, false);

	// create sync
	u64 syncParamv[2] = {count+1, isMain};
	ocrEdtCreate(&syncEdt, syncEdtTemplate, EDT_PARAM_DEF, syncParamv,
				 EDT_PARAM_DEF, workerEvents, 0, NULL_GUID, NULL);

	// create and start workers
	for(i = 0; i < NUM_WORKERS; i++) {
		workerParamv[i] = workerEvents[i];
		ocrEdtCreate(&workerEdts[i], workerEdtTemplate, EDT_PARAM_DEF, &workerParamv[i],
				 EDT_PARAM_DEF, &workerStartEvents[i], 0, NULL_GUID, NULL);
	}

	for(i = 0; i < NUM_WORKERS; i++) {
		ocrEventSatisfy(workerStartEvents[i], NULL_GUID);
	}

	return NULL_GUID;
}


ocrGuid_t mainEdt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	printf("main \n");
	ocrGuid_t syncEdtTemplate;
	ocrGuid_t syncEdt;

	// create sync template
	ocrEdtTemplateCreate(&syncEdtTemplate, syncTask, 2, 0);

	// create sync
	u64 syncParamv[2] = {0, 1};
	ocrEdtCreate(&syncEdt, syncEdtTemplate, EDT_PARAM_DEF, syncParamv,
				 EDT_PARAM_DEF, NULL, 0, NULL_GUID, NULL);

	return NULL_GUID;
}



