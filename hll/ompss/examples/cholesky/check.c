
#include "file.h"
#include "util.h"

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

// ------------------------------------------------------------------------
// Robust Check the factorization of the result matrix
// ------------------------------------------------------------------------
void check_factorization( const double *restrict original, const double *restrict result, unsigned n, double eps )
{
	printf("Checking the Cholesky Factorization \n");

	const unsigned len = n*n*sizeof(double);
	double *Residual = malloc( len );
	double *L1       = malloc( len );
	double *L2       = malloc( len );
	memset( L1, 0, len );
	memset( L2, 0, len );

    LAPACKE_dlacpy(
        LAPACK_COL_MAJOR,
        'A',
        n, n,
        original, n,
        Residual, n );

    LAPACKE_dlacpy(
        LAPACK_COL_MAJOR,
        'L',
        n, n,
        result, n,
        L1, n );

    LAPACKE_dlacpy(
        LAPACK_COL_MAJOR,
        'L',
        n, n,
        result, n,
        L2, n );

	const double alpha= 1.0;
	cblas_dtrmm(
		CblasColMajor,
		CblasRight,
		CblasLower,
		CblasTrans,
		CblasNonUnit,
		n, n,
		alpha,
		L1, n,
		L2, n);

	const double Anorm =
		LAPACKE_dlange(
			LAPACK_COL_MAJOR,
			'I',
			n, n,
			Residual, n);

	// Compute the Residual A - L'*L
	// daxpy: y := a*x + y  (with alpha = -1, x:= L'*L
	cblas_daxpy( n*n, -1.0, L2, 1, Residual, 1 );

	const double Rnorm =
		LAPACKE_dlange(
			LAPACK_COL_MAJOR,
			'I',
			n, n,
			Residual, n );

	printf("-- ||L'L-A||_oo/(||A||_oo.n.eps) = %e \n",Rnorm/(Anorm*n*eps));

	const int info_factorization = isnan(Rnorm/(Anorm*n*eps)) ||
				   isinf(Rnorm/(Anorm*n*eps)) ||
				   (Rnorm/(Anorm*n*eps) > 60.0);

	if ( info_factorization) {
		printf("\n-- Factorization is wrong ! \n\n");
	} else{
		printf("\n-- Factorization is CORRECT ! \n\n");
	}

	free(Residual);
	free(L1);
	free(L2);
}

int main( int argc, char* argv[] )
{
    unsigned n;
    double* input;
    double* output;
	const double eps = BLAS_dfpinfo( blas_eps );

    if( argc < 2 ) {
        fprintf( stderr, "Usage: %s output_file\n", argv[0] );
        return -1;
    }
	const char* filename = argv[1];

	// Check if output matrix exists
	if( access( filename, R_OK ) != 0 ) {
        fprintf( stderr, "File %s not found.\n", filename );
        return -1;
    }
    load_values( filename, &output, &n );

	// Check if input matrix exists
	char ifile[1024];
	snprintf( ifile, n, "data/%u.in", n );
	if( access( ifile, R_OK ) != 0 ) {
        fprintf( stderr, "File %s not found.\n", ifile );
        return -1;
    }
    load_values( ifile, &input, NULL );

    check_factorization( input, output, n, eps );

    free( input );
    free( output );

    return 0;
}

