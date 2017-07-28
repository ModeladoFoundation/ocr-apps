/** File: myOcrHelpers.c
 *
 *  Description:
 *    Wrappers for some OCR calls that handle errors and which
 *    set common arguments.
 */

#include "myOcrHelpers.h"

/** Create a DB with no hints.
 *  This function handles errors.
 *
 * @param db   Where to store the GUID that is created.
 * @param len  The number of bytes to allocate in this DB.
 * @return  Pointer to memory or NULL if failure
 */
u8 *myDbCreate(ocrGuid_t *db, u64 len) {
    void *ptr;
    u8 iRet;

    iRet = ocrDbCreate(db, &ptr, len, DB_PROP_NONE, NULL_HINT, NO_ALLOC);
    if (iRet != 0) {
        if (iRet == OCR_ENOMEM) {
            PRINTF("ERROR: mainEdt(): ocrDbCreate() failed to "
                   "allocate %d byte block - OCR_ENOMEM !\n", len);
            return NULL;
        }
        if (iRet == OCR_EINVAL) {
            PRINTF("ERROR: mainEdt(): ocrDbCreate() failed to "
                   "to allocate %d byte block - OCR_EINVAL !\n", len);
            return NULL;
        }
        if (iRet == OCR_EBUSY) {
            PRINTF("ERROR: mainEdt(): ocrDbCreate() failed to "
                   "to allocate %d byte block - OCR_EBUSY !\n", len);
            // Could retry here...
            return NULL;
        }
        PRINTF("ERROR: mainEdt(): ocrDbCreate() failed to "
               "to allocate %d byte block - status = %d !\n", len);
        return NULL;
    }
    return (u8 *)ptr;
 }
