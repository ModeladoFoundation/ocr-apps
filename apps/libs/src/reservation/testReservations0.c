/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

//gcc -DENABLE_EXTENSION_PARAMS_EVT -DENABLE_EXTENSION_CHANNEL_EVT -I${OCR_INSTALL}/include -L${OCR_INSTALL}/lib -locr_${OCR_TYPE} ocr-reservations.c testReservations0.c
//or
//make; gcc -I${OCR_INSTALL}/include -L${PWD} -L${OCR_INSTALL}/lib -locr_${OCR_TYPE}  testReservations0.c -L. -locr-reservations

#include "ocr.h"
#include "ocr-reservations.h"

#define SIZE 10

u64 count = 0;

ocrGuid_t funcEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    ocrAssert(paramc == 1 && depc == 1);
    int i;double s=0;
    u64 temp = count;
    for(i=0;i<100000;i++) s+=i;
    ocrPrintf("id = %d, global count = %d, sum = %f\n",*paramv, temp, s);
    count = temp + 1;
    return NULL_GUID;
}

ocrGuid_t shutEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {
    ocrAssert(paramc == 0);
    ocrPrintf("SHUTDOWN\n");
    ocrShutdown();
    return NULL_GUID;
}

ocrGuid_t mainEdt(u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[]) {

    u64 param_val=0, i;

    ocrGuid_t resGuid = NULL_GUID;
    ocrReservationCreate(&resGuid, NULL);

    ocrGuid_t templGuid;
    ocrEdtTemplateCreate(&templGuid, funcEdt, 1, 1);

    ocrGuid_t depShutEvt[SIZE] = {UNINITIALIZED_GUID};

    for(i=0;i<SIZE;i++)
    {
      ocrGuid_t outResEvt, depRes;
      ocrEventCreate(&depRes, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
      ocrReservationAcquireRequest(resGuid, OCR_RES_EXCL_T, 1, &depRes, &outResEvt);
      ocrGuid_t edtGuid, outEdtGuid;
      //outResEvt = NULL_GUID; //uncomment to remove effect of acquire
      ocrEdtCreate(&edtGuid, templGuid, 1, &param_val, 1, &outResEvt, EDT_PROP_NONE, NULL_HINT, &depShutEvt[i]);
      param_val++;
      ocrReservationReleaseRequest(resGuid, 1, &depShutEvt[i]);
      ocrEventSatisfy(depRes, NULL_GUID);
    }

    ocrGuid_t templShutGuid;
    ocrEdtTemplateCreate(&templShutGuid, shutEdt, 0, SIZE);

    ocrGuid_t edtShutGuid;
    ocrEdtCreate(&edtShutGuid, templShutGuid, 0, NULL, SIZE, depShutEvt, EDT_PROP_NONE, NULL_HINT, NULL);

    return NULL_GUID;
}

