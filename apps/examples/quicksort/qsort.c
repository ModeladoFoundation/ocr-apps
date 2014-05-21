
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ocr.h>

#define CACHE_LINE_SIZE 64

#define ARRAY_SIZE 1000
#define RANGE 10000

// insertion sort
void sortSerial(u64 *data, u64 low, u64 high)
{
	u64 min, i, j, temp;
	for(i = low; i <= high-1; i++) {
		min = 0xFFFFFFFFFFFFFFFFUL;
		for(j = i; j <=high-1; j++)
			if(data[j] < min)
				min = data[j];

		temp = data[i];
		data[i] = min;
		data[i+1] = temp;
	}
}

// param 0: low index (inclusive)
// param 1: high index (inclusive)
// param 2: qsort edt template
// dep 0: array
ocrGuid_t qsortTask( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	u64 low = paramv[0];
	u64 high = paramv[1];
	u64 size = high - low + 1;
	ocrGuid_t qsortTemplate = paramv[2];
	u64 *data = depv[0].ptr;

	if(size * sizeof(u64) <= CACHE_LINE_SIZE)
		sortSerial(data, low, high);
	else {
		// choose pivot
		u64 pivotIndex = low + rand() % size;
		u64 pivot = data[pivotIndex];

		// partition
		u64 curIndex, swapIndex = low;
		u64 temp;
		data[pivotIndex] = data[high];
		data[high] = pivot;
		for(curIndex = low; curIndex <= high-1; curIndex++) {
			if(data[curIndex] <= pivot) {
				temp = data[swapIndex];
				data[swapIndex] = data[curIndex];
				data[curIndex] = temp;
				swapIndex++;
			}
		}

		data[high] = data[swapIndex];
		data[swapIndex] = pivot;
		pivotIndex = swapIndex;

		// recurse
		ocrGuid_t qsortLowEdt, qsortHighEdt;
		ocrGuid_t qsortLowDataEvt, qsortHighDataEvt;

		ocrEventCreate(&qsortLowDataEvt, OCR_EVENT_ONCE_T, false);
		ocrEventCreate(&qsortHighDataEvt, OCR_EVENT_ONCE_T, false);

		u64 qsortLowParamv[3] = {low, pivotIndex-1, qsortTemplate};
		ocrEdtCreate(&qsortLowEdt, qsortTemplate, EDT_PARAM_DEF, qsortLowParamv,
				 EDT_PARAM_DEF, &qsortLowDataEvt, 0, NULL_GUID, NULL);

		u64 qsortHighParamv[3] = {pivotIndex+1, high, qsortTemplate};
		ocrEdtCreate(&qsortHighEdt, qsortTemplate, EDT_PARAM_DEF, qsortHighParamv,
				 EDT_PARAM_DEF, &qsortHighDataEvt, 0, NULL_GUID, NULL);

		ocrEventSatisfy(qsortLowDataEvt, depv[0].guid);
		ocrEventSatisfy(qsortHighDataEvt, depv[0].guid);
	}

	return depv[0].guid;
}

ocrGuid_t finishTask( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	sleep(15);
	printf("done \n");
	printf("first 100 elements: \n");
	u64 i;
	u64 *data = depv[0].ptr;
	for(i = 0; i < 100; i++)
		printf("%lu \n", data[i]);

	ocrShutdown();
	return NULL_GUID;
}

ocrGuid_t mainEdt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[])
{
	ocrGuid_t qsortTemplate;
	ocrGuid_t qsortEdt;
	ocrGuid_t dataDb;
	ocrGuid_t outEvt;
	u64 *data;
	ocrEdtTemplateCreate(&qsortTemplate, qsortTask, 3, 1);

	ocrDbCreate(&dataDb, (void**)&data, sizeof(u64) * ARRAY_SIZE,
		/*flags=*/0, /*location=*/NULL_GUID, NO_ALLOC);

	srand(time(0));

	u64 i;
	for(i = 0; i < ARRAY_SIZE; i++)
		data[i] = rand() % RANGE;

	u64 qsortParamv[3] = {0, ARRAY_SIZE-1, qsortTemplate};
	ocrEdtCreate(&qsortEdt, qsortTemplate, EDT_PARAM_DEF, qsortParamv,
		EDT_PARAM_DEF, &dataDb, 0, NULL_GUID, &outEvt);

	ocrGuid_t finishTemplate;
	ocrGuid_t finishEdt;
	u64 finishParamv = ARRAY_SIZE;
	u64 finishDepv[2] = {dataDb, outEvt};
	ocrEdtTemplateCreate(&finishTemplate, finishTask, 1, 1);
	ocrEdtCreate(&finishEdt, finishTemplate, EDT_PARAM_DEF, &finishParamv,
		EDT_PARAM_DEF, finishDepv, 0, NULL_GUID, NULL);
}




