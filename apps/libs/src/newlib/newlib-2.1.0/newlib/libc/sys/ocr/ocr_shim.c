#include <stdlib.h>
#include <malloc.h>
#include <stdint.h>
#include <reent.h>
#include <sys/ocr.h>
#include "ocr_shim.h"
/*
 * OCR Shim Methods
 * These methods support translating between the libc notions of open
 * file ids and the associated OCR Guids
 */

#ifdef _OCR_GLIBC
static struct _reent reent;
struct _reent *_get_reent()
{
    return & reent;
}

#endif

//
// Magic GUID definitions for stdin/out/err, for ocr scaffolding only
//
#define STDIN_GUID  ((ocrGuid_t)((0x2L << 60) | 0))
#define STDOUT_GUID ((ocrGuid_t)((0x2L << 60) | 1))
#define STDERR_GUID ((ocrGuid_t)((0x2L << 60) | 2))
/*
 * Allocate memory for and init the fd to GUID mapping table
 * Returns 0 on success
 */
static int ocrLibInit( struct _reent * r )
{
    if( ! r->_ocr.fileGuidMap ) {
        r->_ocr.fileGuidSize = OCR_FILE_MAP_MAX;
        r->_ocr.fileGuidMap = (__int64_t *)malloc( r->_ocr.fileGuidSize *
                                                   sizeof(__int64_t) );
        if( r->_ocr.fileGuidMap ) {
            for( int i = 0 ; i < r->_ocr.fileGuidSize ; i++ )
                r->_ocr.fileGuidMap[i] = NULL_GUID;

            r->_ocr.fileGuidMap[0] = STDIN_GUID;
            r->_ocr.fileGuidMap[1] = STDOUT_GUID;
            r->_ocr.fileGuidMap[2] = STDERR_GUID;
        }
    }
    return r->_ocr.fileGuidMap == NULL;
}

/*
 * Add a GUID to the mapping table and return its associated 'fd'
 * Returns the 'fd' or -1 if no available entries.
 * Possibly we could realloc() the mapping to allow dynamic growth.
 */
int ocrLibAddGuid( struct _reent * r, ocrGuid_t guid )
{
    if( ! ocrLibInit(r) ) {
        for( int i = 0 ; i < r->_ocr.fileGuidSize ; i++ ) {
            if( r->_ocr.fileGuidMap[i] == NULL_GUID ) {
                r->_ocr.fileGuidMap[i] = guid;
                return i;
            }
        }
    }
    return -1;
}

/*
 * Remove an 'fd' mapping from the mapping table
 * Returns a guid, will be NULL_GUID on failure
 */
ocrGuid_t ocrLibRmFd( struct _reent * r, int fd )
{
    if( ocrLibInit(r) || fd < 0 || fd >= r->_ocr.fileGuidSize )
        return NULL_GUID;

    ocrGuid_t guid = r->_ocr.fileGuidMap[fd];
    r->_ocr.fileGuidMap[fd] = NULL_GUID;

    return guid;
}

ocrGuid_t ocrLibFdToGuid( struct _reent * r, int fd )
{
    if( ocrLibInit(r) || fd < 0 || fd >= r->_ocr.fileGuidSize )
        return NULL_GUID;
    return (ocrGuid_t) r->_ocr.fileGuidMap[fd];
}

int ocrLibGuidToFd( struct _reent * r, ocrGuid_t guid )
{
    if( ! ocrLibInit(r) ) {
        for( int i = 0 ; i < r->_ocr.fileGuidSize ; i++ ) {
            if( r->_ocr.fileGuidMap[i] == guid ) {
                return i;
            }
        }
    }
    return -1;
}

