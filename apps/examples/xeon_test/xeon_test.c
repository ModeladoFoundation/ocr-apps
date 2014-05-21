
/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "ocr.h"

#define dbSize  100

float timeSec()
{
	struct timeval t;
	gettimeofday(&t, NULL);
	printf("us: %lu \n", t.tv_usec);
	return (float) t.tv_sec + ((float) t.tv_usec) / 1000000.00e0f;
}

struct timeval a;

ocrGuid_t workerEDT(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {

	int low = paramv[0];
	int high = paramv[1];
	int i,sum;
	u64* ptr =depv[0].ptr;

//printf("workerEdt \n");
	for(i=low;i<high;i++){
		sum+= ptr[i];
	}
	return depv[0].guid;
}



ocrGuid_t loopEdt ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {

	int loop_fan = 2;
	int loopCount = 100;

//printf("in loopEdt \n");

	if (paramv[0]== loopCount) {
		ocrDbDestroy(depv[0].guid);
		return NULL_GUID;
	}

	ocrGuid_t worker, workerTemplate;
	ocrEdtTemplateCreate(&workerTemplate , workerEDT, 2, 1);

	ocrGuid_t loopTemplate,finishEvent;
    ocrEdtTemplateCreate(&loopTemplate,loopEdt , 1/*paramc*/, loop_fan/*depc*/);

	ocrGuid_t nextLoopEDT;
	u64 newParamv = paramv[0]++;

	ocrEdtCreate(&nextLoopEDT, loopTemplate, /*paramc=*/EDT_PARAM_DEF,
             /*paramv=*/paramv, EDT_PARAM_DEF, /*depv=*/ NULL,
             /*properties=*/ 0, NULL_GUID, /*outEvent=*/ NULL);

	int i,sum;
	u64* ptr =depv[0].ptr;

	for(i=0;i<loop_fan;i++){
		ocrGuid_t workerEdtGuid;
		ocrGuid_t db = depv[0].guid;
		u64 params[2];
		params[0]=i*dbSize/loop_fan;
		params[1]=(i+1)*dbSize/loop_fan -1;

		ocrEdtCreate(&workerEdtGuid, workerTemplate, /*paramc=*/EDT_PARAM_DEF,
                 /*paramv=*/ params, EDT_PARAM_DEF, /*depv=*/NULL,//&(db) ,
                 /*properties=*/ 0, NULL_GUID, /*outEvent=*/ &finishEvent);

		ocrAddDependence(finishEvent, nextLoopEDT, i , DB_DEFAULT_MODE);
		ocrAddDependence(db,workerEdtGuid, 0,DB_MODE_ITW);
	}
	return depv[0].guid;
}

ocrGuid_t goEdt ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {

	paramv[0]++;
    int go_fan = 50;
	int goCount= 2;

	if(paramv[0] == goCount ) {
		struct timeval b;
		gettimeofday(&b,0);
    	printf("%f\r\n",
        	((b.tv_sec - a.tv_sec)*1000000+(b.tv_usec - a.tv_usec))*1.0/1000000);
		ocrShutdown();
	}

//printf("in goEdt \n");

    ocrGuid_t dbs[go_fan];
    int *data[go_fan];

	ocrGuid_t loopTemplate,finishEvent;
    ocrEdtTemplateCreate(&loopTemplate,loopEdt , 1/*paramc*/, 1/*depc*/);

	ocrGuid_t goEDTTemplate, goEDT;
	ocrEdtTemplateCreate(&goEDTTemplate,goEdt , 1/*paramc*/, go_fan/*depc*/);

    ocrEdtCreate(&goEDT, goEDTTemplate, /*paramc=*/EDT_PARAM_DEF,
                 /*paramv=*/paramv, EDT_PARAM_DEF, /*depv=*/NULL,
                 /*properties=*/0, NULL_GUID, /*outEvent=*/ NULL);

	int i;

	for(i = 0; i < go_fan ; ++i) {

		ocrDbCreate(&dbs[i], (void**)&data[i], sizeof(int)*dbSize, /*flags=*/0,
				    /*location=*/NULL_GUID, NO_ALLOC);

		ocrGuid_t loopEDT;
		u64 startIdx = 0;
		ocrEdtCreate(&loopEDT, loopTemplate, /*paramc=*/EDT_PARAM_DEF,
                /*paramv=*/ &startIdx, EDT_PARAM_DEF, /*depv=*/ NULL,
                 /*properties=*/ EDT_PROP_FINISH, NULL_GUID, /*outEvent=*/ &finishEvent);

		ocrAddDependence(finishEvent, goEDT, i , DB_DEFAULT_MODE);
		ocrAddDependence(dbs[i], loopEDT, 0, DB_MODE_ITW);
	}

	return NULL_GUID;
}


ocrGuid_t mainEdt ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {

	u64 startIdx =0;

	gettimeofday(&a,0);

	ocrGuid_t goEDTTemplate, goEDT;
	ocrEdtTemplateCreate(&goEDTTemplate, goEdt , 1/*paramc*/, 0/*depc*/);

	ocrEdtCreate(&goEDT, goEDTTemplate, /*paramc=*/EDT_PARAM_DEF,
                 /*paramv=*/&startIdx, EDT_PARAM_DEF, /*depv=*/NULL,
                 /*properties=*/0, NULL_GUID, /*outEvent=*/ NULL);

	return NULL_GUID;
}


ocrGuid_t aEdt ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	printf("in aEdt \n");
	return NULL_GUID;
}

ocrGuid_t bEdt ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	printf("in bEdt \n");
	return NULL_GUID;
}

ocrGuid_t testEdt ( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
	ocrGuid_t aTemplateGuid, bTemplateGuid;
	ocrGuid_t aGuid, bGuid;
	ocrGuid_t eventGuid;

	ocrEdtTemplateCreate(&aTemplateGuid, aEdt, 0/*paramc*/, 0/*depc*/);
	ocrEdtTemplateCreate(&bTemplateGuid, bEdt , 0/*paramc*/, 1/*depc*/);

	ocrEdtCreate(&aGuid, aTemplateGuid, /*paramc=*/EDT_PARAM_DEF,
                 /*paramv=*/NULL, EDT_PARAM_DEF, /*depv=*/NULL,
                 /*properties=*/0, NULL_GUID, /*outEvent=*/ &eventGuid);

	sleep(1);

	ocrEdtCreate(&bGuid, bTemplateGuid, /*paramc=*/EDT_PARAM_DEF,
                 /*paramv=*/NULL, EDT_PARAM_DEF, /*depv=*/NULL,
                 /*properties=*/0, NULL_GUID, /*outEvent=*/ NULL);
	ocrAddDependence(eventGuid, bGuid, 0, DB_MODE_ITW);

	return NULL_GUID;
}


