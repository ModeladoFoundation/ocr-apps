
#include "file.h"
#include "util.h"

#include <stdio.h>
#include <unistd.h>

#include <mkl.h>
#define malloc(x) mkl_malloc(x, 128)
#define free(x)   mkl_free(x)

int main( int argc, char* argv[] )
{
    if( argc < 2 ) {
        fprintf( stderr, "Usage: %s matrix_size\n", argv[0] );
        return -1;
    }
    unsigned n = atoi( argv[1] );

	// Check if input matrix exists
	char filename[1024];
	sprintf( filename, "data/%d.in", n );

    double* matrix = malloc( n*n*sizeof(double) );
    generate_values( filename, matrix, n );
    free( matrix );

    return 0;
}
