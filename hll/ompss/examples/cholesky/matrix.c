
#include "util.h"
#include "file.h"
#include "matrix.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>

void copy_to_matrix( const double* src, matrix_t* dst )
{
    const unsigned n = dst->n;
    const unsigned ts = dst->ts;
    const unsigned ntiles = n/ts;

    for( unsigned ti = 0; ti < ntiles; ++ti ) {
        for( unsigned tj = 0; tj < ntiles; ++tj ) {
	        LAPACKE_dlacpy(
	        	LAPACK_COL_MAJOR,
	        	'A',
	        	ts, ts,
	        	&src[ti*ts*n + tj*ts], n,
	        	dst->m[ti*ntiles+tj], ts );
        }
    }
}

void copy_all_from_matrix( const matrix_t* src, double* dst )
{
    const unsigned n = src->n;
    const unsigned ts = src->ts;
    const unsigned ntiles = n/ts;

    for( unsigned ti = 0; ti < ntiles; ++ti ) {
        for( unsigned tj = 0; tj < ntiles; ++tj ) {
	        LAPACKE_dlacpy(
                LAPACK_COL_MAJOR,
                'A',
                ts, ts,
                src->m[ti*ntiles+tj], ts,
                &dst[ti*ts*n + tj*ts], n );
        }
    }
}

void copy_lower_from_matrix( const matrix_t* src, double* dst )
{
    const unsigned n = src->n;
    const unsigned ts = src->ts;
    const unsigned ntiles = n/ts;

    for( unsigned ti = 0; ti < ntiles; ++ti ) {
        for( unsigned tj = 0; tj < ti; ++tj ) {
	        LAPACKE_dlacpy(
	        	LAPACK_COL_MAJOR,
	        	'L',
	        	ts, ts,
	        	src->m[ti*ntiles+tj], ts,
	        	&dst[ti*ts*n + tj*ts], n );
        }
    }
}

void clone_matrix( const matrix_t* src, matrix_t* dst )
{
    const unsigned n = dst->n;
    const unsigned ts = dst->ts;
    const unsigned ntiles = n/ts;

    for( unsigned ti = 0; ti < ntiles*ntiles; ++ti ) {
	    LAPACKE_dlacpy(
	    	LAPACK_COL_MAJOR,
	    	'A',
	    	ts, ts,
	    	src->m[ti], ts,
	    	dst->m[ti], ts );
    }
}

matrix_t* load_matrix( const char* filename, unsigned ts )
{
    unsigned n;
    double* tmp;
    matrix_t* result;

    load_values( filename, &tmp, &n );

    result = new_matrix( n, ts );

    copy_to_matrix( tmp, result );

    free( tmp );

    return result;
}

void store_matrix( const char* input, matrix_t* matrix )
{
    size_t len = matrix->n * matrix->n * sizeof(double);
    double* tmp = malloc( len );
    copy_all_from_matrix( matrix, tmp );

	char filename[1024];
    strncpy( filename, input, 1024 );
    unsigned extension_pos = strlen( input) - 2;
	sprintf( filename+extension_pos, "out" );

    store_values( filename, tmp, matrix->n );
    free( tmp );
}

matrix_t* initialize_matrix( const char* filename, unsigned ts )
{
	// Check if input matrix exists
	if( access( filename, R_OK ) == 0 ) {
		// Load input into matrix
		return load_matrix( filename, ts );
	} else {
        fprintf(stderr,"Must generate file %s first\n", filename);
        abort();
	}
}

