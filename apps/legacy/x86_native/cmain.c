#include <fcntl.h>
#include <malloc.h>
#include <stdint.h>
#include <reent.h>
#include <sys/ocr.h>

static struct _reent reent;
struct _reent *_get_reent()
{
    return & reent;
}

extern int __open( const char * fn, int flags );
extern int __close( int fd );
extern int write( int fd, const char * buf, int count );

extern int mainEdt( int argc, char **argv );

int main( int argc, char **argv )
{
    ocrGuid_t   affinity;
    ocrConfig_t oc;
    //
    // initialize OCR
    //
    ocrParseArgs(argc, (const char **) argv, & oc);
    ocrInit(&_REENT->_ocr.legacyContext, & oc);
    //
    // get the affinity of this EDT
    //
    if (ocrAffinityGetCurrent(&affinity) != 0)
        return 0L;
#if 0
    //
    // funny calls to guarantee we get the ocr-glibc symbols
    //
    struct stat * st = (struct stat *) malloc( sizeof(struct stat) );

    int fd = __open( "/dev/null", O_RDONLY );
    (void) fstat( fd, st );
    (void) write( fd, "/dev/null", 0 );
    (void) __close( fd );
    (void) free( st );
#endif
    //
    // call the application main()
    //
    int status = mainEdt( argc, argv );

    ocrFinalize(_REENT->_ocr.legacyContext);

    return status;
}
