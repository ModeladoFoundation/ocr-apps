/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

//gcc -DENABLE_EXTENSION_PARAMS_EVT -DENABLE_EXTENSION_CHANNEL_EVT -I${OCR_INSTALL}/include -L${OCR_INSTALL}/lib -locr_${OCR_TYPE} ocr-reservations.c

#if !defined(ENABLE_EXTENSION_PARAMS_EVT) || !defined(ENABLE_EXTENSION_CHANNEL_EVT)
#error ENABLE_EXTENSION_PARAMS_EVT and ENABLE_EXTENSION_CHANNEL_EVT should be defined
#else

#include "ocr.h"
#include "ocr-reservations.h"

/**
 * @brief Create a Reservation
 **/
u8 ocrReservationCreate(ocrGuid_t *res, void *in_params)
{
    ASSERT(in_params == NULL); //for now
    //reservation is represented using a channel event
    ocrEventParams_t params;
    params.EVENT_CHANNEL.maxGen = RESERVATION_SIZE;
    params.EVENT_CHANNEL.nbSat = 1;
    params.EVENT_CHANNEL.nbDeps = 1;
    u8 ret = ocrEventCreateParams(res, OCR_EVENT_CHANNEL_T, false, &params);
    //This EventSatisfy will help to trigger the first acquire
    //by matching the add dependency done in aqcuire
    ocrEventSatisfy(*res, NULL_GUID);
    return ret;
}

/**
 * @brief Acquire a reservation
 **/
u8 ocrReservationAcquireRequest(ocrGuid_t res, ocrReservationMode_t mode, u32 depc, ocrGuid_t *depv, ocrGuid_t *outputEvent)
{
    ocrEventParams_t params;
    params.EVENT_LATCH.counter = depc+1;
    u8 ret = ocrEventCreateParams(outputEvent, OCR_EVENT_LATCH_T, false, &params);
    int i;

    ocrAddDependence(res, *outputEvent, OCR_EVENT_LATCH_DECR_SLOT, DB_MODE_RO);
    for(i=0; i<depc; i++)
        ocrAddDependence(depv[i], *outputEvent, OCR_EVENT_LATCH_DECR_SLOT, DB_MODE_RO);
    return ret;
}

/**
 * @brief Release a reservation
 *
 * Releasing a reservation triggers the next pending acquire
 **/
u8 ocrReservationReleaseRequest(ocrGuid_t res, u32 depc, ocrGuid_t *depv)
{
    ocrEventParams_t params;
    params.EVENT_LATCH.counter = depc;
    ocrGuid_t latchguid;
    u8 ret = ocrEventCreateParams(&latchguid, OCR_EVENT_LATCH_T, false, &params);
    int i;

    ocrAddDependence(latchguid, res, 0, DB_MODE_RO);
    for(i=0; i<depc; i++)
        ocrAddDependence(depv[i], latchguid, OCR_EVENT_LATCH_DECR_SLOT, DB_MODE_RO);
    return ret;
}

/**
 * @brief Destroys a reservation
 **/
u8 ocrReservationDestroy(ocrGuid_t res)
{
    return ocrEventDestroy(res);
}

#endif
