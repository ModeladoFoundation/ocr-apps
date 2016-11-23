
#ifndef MATRIX_H
#define MATRIX_H

typedef struct {
    unsigned n;
    unsigned ts;
    double* m[];
} matrix_t;

void copy_to_matrix( const double* src, matrix_t* dst );
void copy_all_from_matrix( const matrix_t* src, double* dst );
void copy_lower_from_matrix( const matrix_t* src, double* dst );
void clone_matrix( const matrix_t* src, matrix_t* dst );
matrix_t* initialize_matrix( const char* filename, unsigned ts );
matrix_t* load_matrix( const char* filename, unsigned ts );
void store_matrix( const char* input, matrix_t* matrix );

static inline matrix_t* new_matrix( unsigned n, unsigned ts )
{
	// matrix is actually composed in two:
    const unsigned ntiles = n/ts;
    matrix_t* matrix = malloc( sizeof(matrix_t) + ntiles*ntiles*sizeof(double*) );
	assert(matrix != NULL);

	// - a relative small (n/ts)^2 matrix whose elements
	// point to a single tile contiguous in memory
    matrix->n = n;
    matrix->ts = ts;

	// - the tiles themselves
	for( unsigned i = 0; i < ntiles*ntiles; ++i ) {
        matrix->m[i] = malloc( ts * ts * sizeof(double) );
	    assert(matrix->m[i] != NULL);
    }

    return matrix;
}

static inline void delete_matrix( matrix_t* matrix )
{
    const unsigned ntiles = matrix->n/matrix->ts;
	for( unsigned i = 0; i < ntiles*ntiles; ++i ) {
        free( matrix->m[i] );
    }
    free( matrix );
}

#endif // MATRIX_H

