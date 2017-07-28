/** File: myOcrHelpers.h
 *
 *  Description:
 *    Wrappers for some OCR calls that handle errors and which
 *    set common arguments.
 */


#include "ocr.h"

/** Create a DB with no hints.
 *  This function handles errors.
 *
 * @param db   Where to store the GUID that is created.
 * @param len  The number of bytes to allocate in this DB.
 * @return  pointer to data or NULL on failure
 */
u8 *myDbCreate(ocrGuid_t *db, u64 len);
