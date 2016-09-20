/**
 * @brief OCR APIs for the "Reservations" synchronization primitive
 **/

/*
 * This file is subject to the license agreement located in the file LICENSE
 * and cannot be distributed without it. This notice cannot be
 * removed or modified.
 */

#ifndef __OCR_RESERVATION_H__
#define __OCR_RESERVATION_H__

#include "ocr-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @defgroup OCRExtReservations Reservations synchronization primitive for OCR
 *  @brief Describes the Reservations synchronization APIs for OCR.
 *
 *  These APIs will enable OCR application developers to perform
 *  synchronization using the Reservation synchronization primitive.
 *  Reservations were first introduced in the Realm runtime.
 *  Reservation mainly provides an acquire and release API for
 *  performing synchronization
 * @{
**/

/**
 * @brief Number of simultaneous acquires supported
 **/
#ifndef RESERVATION_SIZE
#define RESERVATION_SIZE 100
#endif

/**
 * @brief Reservation acquire modes
 *
 * Whether the resevation can be shared or not
 **/
typedef enum {
    OCR_RES_EXCL_T,
    OCR_RES_SHARED_T,
    OCR_RES_NUM_MODE
} ocrReservationMode_t;


/**
 * @brief Create a Reservation
 *
 * On successful creation, the returned GUID can be used
 * to perform synchronization using acquire and release
 *
 * E.g:
 * ...
 * ocrGuid_t res;
 * u8 err = ocrResCreate( &res, NULL );
 * ...
 *
 * @param[out] res       GUID of the new reservation
 * @param[in]  params    additional parameters say for example
 *                       number of simultaneous acquires supported
 *
 * @return a status code
 *     - 0: successful
 *     - !0: error in creating reservation
 **/
u8 ocrReservationCreate(ocrGuid_t *res, void * params);

/**
 * @brief Acquire a reservation
 *
 * Acquire a reservation when all its dependencies are satisfied.
 * This is an asynchronous call which returns immediately with an
 * output event that gets triggered when the reservation is granted.
 * Similar to ocrEdtCreate, use an unsatisfied dependency to prevent
 * immediate granting of acquire, which has the potential for causing
 * a race between the use and destruction of the output event
 *
 * E.g:
 * ..
 * ocrEventCreate(&depRes, OCR_EVENT_ONCE_T, EVT_PROP_NONE);
 * ocrReservationAcquireRequest(res, OCR_RES_EXCL_T, 1, &depRes, &outResEvt);
 * ..
 * ocrEventSatisfy(depRes, NULL_GUID);
 *
 * @param[in] res               The GUID for the reservation
 * @param[in] mode              The access mode, whether exclusive or shared
 * @param[in] depc              Number of dependences to be satisfied before acquire.
 * @param[in] depv              Values for the GUIDs of the dependences (if known). Note
 *                              that all dependences added by this method will be in the
 *                              #DB_DEFAULT_MODE. All dependencies should be initialized.
 *                              These are control dependencies and do not carry data.
 * @param[out] outputEvent      is the event that gets triggered when the reservation is acquired
 *
 * @return a status code
 *     - 0: successful
 *     - !0: error in creating reservation
 **/
u8 ocrReservationAcquireRequest(ocrGuid_t res, ocrReservationMode_t mode, u32 depc, ocrGuid_t *depv, ocrGuid_t *outputEvent);

/**
 * @brief Release a reservation
 *
 * Releasing a reservation triggers the next pending acquire
 *
 * @param[in] res               The GUID for the reservation
 * @param[in] depc              Number of dependences to be satisfied to enable release.
 * @param[in] depv              Values for the GUIDs of the dependences (if known). Note
 *                              that all dependences added by this method will be in the
 *                              #DB_DEFAULT_MODE. All dependencies should be initialized.
 *                              These are control dependencies and do not carry data.
 *
 * @return a status code
 *     - 0: successful
 *     - !0: error in creating reservation
 **/
u8 ocrReservationReleaseRequest(ocrGuid_t res, u32 depc, ocrGuid_t *depv);

/**
 * @brief Destroys a reservation
 *
 * This destroys a reservation and free the internal data structures used.
 * Invoking this API just destroys the Reservation and do not process any pending requests.
 *
 * @param[in] res               The guid of the reservation to be destroyed
 *
 * @return a status code
 *     - 0: successful
 *     - !0: error in creating reservation
 **/
u8 ocrReservationDestroy(ocrGuid_t res);

/**
   @}
**/

#ifdef __cplusplus
}
#endif

#endif /* __OCR_RESERVATION_H__ */
