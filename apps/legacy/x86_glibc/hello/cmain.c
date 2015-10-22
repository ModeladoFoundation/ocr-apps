#include <fcntl.h>
#include <malloc.h>
#include <stdint.h>
#include <ocr.h>
#include <extensions/ocr-legacy.h>
#include <extensions/ocr-affinity.h>
#include <sys/stat.h>

u8 ocrFinalize(ocrGuid_t legacyContext);
void ocrInit(ocrGuid_t *legacyContext, ocrConfig_t * ocrConfig);

extern int __open( const char * fn, int flags );
extern int __close( int fd );
extern int write( int fd, const char * buf, int count );

extern int mainEdt( int argc, char **argv );

int main( int argc, char **argv )
{
    ocrGuid_t   affinity;
    ocrConfig_t oc;
    ocrGuid_t legacyContext;
    //
    // initialize OCR
    //
    ocrParseArgs(argc, (const char **) argv, & oc);
    ocrInit(&legacyContext, & oc);
    //
    // get the affinity of this EDT
    //
    if (ocrAffinityGetCurrent(&affinity) != 0)
        return 0L;
    //
    // funny calls to guarantee we get the ocr-glibc symbols
    //
    struct stat * st = (struct stat *) malloc( sizeof(struct stat) );

    int fd = __open( "/dev/null", O_RDONLY );
    (void) fstat( fd, st );
    (void) write( fd, "/dev/null", 0 );
    (void) __close( fd );
    (void) free( st );
    //
    // call the application main()
    //
    int status = mainEdt( argc, argv );

    ocrFinalize(legacyContext);

    return status;
}
