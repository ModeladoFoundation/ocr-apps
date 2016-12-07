
#include "util.h"
#include "file.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

void load_values( const char* filename, double** matrix, unsigned* n )
{
	printf("Loading values from file %s ...\n", filename);
	FILE* f = fopen( filename, "r" );
	assert( f != NULL );

    unsigned N;
	fread( &N, sizeof(unsigned), 1, f );

    // Don't assign a size if it was not requested
    if( n )
        *n = N;

    // Don't load matrix if it was not requested
    if( matrix ) {
        *matrix = (double*) malloc( sizeof(double) * N*N );
	    fread( *matrix, sizeof(double), N*N, f );
    }

	fclose( f );
}

void store_values( const char* filename, double* matrix, unsigned n )
{
	printf("Storing values to file %s ...\n", filename);
	FILE* f = fopen( filename, "w+" );
	assert( f != NULL );

    fwrite( &n, sizeof(int), 1, f );
	fwrite( matrix, sizeof(double), n*n, f );
	fclose( f );
}

void generate_values( const char* filename, double *matrix, unsigned n )
{
	printf("Initializing matrix with random values ...\n");

	lapack_int ISEED[4] = {0,0,0,1};
	lapack_int uniform_distribution = 1;
	LAPACKE_dlarnv( uniform_distribution, ISEED, n*n, matrix );

	for( unsigned i = 0; i < n; i++ ) {
	    for( unsigned j = 0; j < n; j++ ) {
            matrix[i*n+j] += matrix[j*n+i];
            matrix[j*n+i] = matrix[i*n+j];
        }
        matrix[i*n+i] += n;
    }

	// Store values in a file for later use
	store_values( filename, matrix, n );
}

