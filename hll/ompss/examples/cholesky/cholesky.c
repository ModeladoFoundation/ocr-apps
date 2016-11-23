/*************************************************************************************/
/*      Copyright 2015 Barcelona Supercomputing Center                               */
/*                                                                                   */
/*      This file is part of the NANOS++ library.                                    */
/*                                                                                   */
/*      NANOS++ is free software: you can redistribute it and/or modify              */
/*      it under the terms of the GNU Lesser General Public License as published by  */
/*      the Free Software Foundation, either version 3 of the License, or            */
/*      (at your option) any later version.                                          */
/*                                                                                   */
/*      NANOS++ is distributed in the hope that it will be useful,                   */
/*      but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/*      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/*      GNU Lesser General Public License for more details.                          */
/*                                                                                   */
/*      You should have received a copy of the GNU Lesser General Public License     */
/*      along with NANOS++.  If not, see <http://www.gnu.org/licenses/>.             */
/*************************************************************************************/

#include "util.h"
#include "matrix.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#pragma omp task inout( A->m[i*A->n/A->ts + j] ) label(potrf)
void omp_potrf(matrix_t* A, int i, int j)
{
    const unsigned ntiles = A->n/A->ts;
	lapack_int error = LAPACKE_dpotrf(
		LAPACK_COL_MAJOR,
		'L',
		A->ts,
		A->m[i*ntiles + j], A->ts );
	assert( error == 0 );
}

#pragma omp task in( A->m[ai*A->n/A->ts + aj] ) inout( B->m[bi*B->n/B->ts + bj] ) label(trsm)
void omp_trsm(matrix_t* A, int ai, int aj, matrix_t* B, int bi, int bj)
{
    const unsigned ntiles = A->n/A->ts;
	const double alpha = 1.0;
	cblas_dtrsm(
		CblasColMajor,
		CblasRight,
		CblasLower,
		CblasTrans,
		CblasNonUnit,
		A->ts, A->ts,
		alpha,
		A->m[ai*ntiles + aj], A->ts,
		B->m[bi*ntiles + bj], B->ts );
}

#pragma omp task in( A->m[ai*A->n/A->ts + aj] ) inout( B->m[bi*B->n/B->ts + bj] ) label(syrk)
void omp_syrk(matrix_t* A, int ai, int aj, matrix_t* B, int bi, int bj)
{
    const unsigned ntiles = A->n/A->ts;
	const double alpha = -1.0;
	const double beta = 1.0;
	cblas_dsyrk(
		CblasColMajor,
		CblasLower,
		CblasNoTrans,
		A->ts, A->ts,
		alpha,
		A->m[ai*ntiles + aj], A->ts,
		beta,
		B->m[bi*ntiles + bj], B->ts );
}

#pragma omp task in( A->m[ai*A->n/A->ts + aj], B->m[bi*B->n/B->ts + bj] ) inout( C->m[ci*C->n/C->ts + cj] ) label(gemm)
void omp_gemm(matrix_t* A, int ai, int aj, matrix_t* B, int bi, int bj, matrix_t* C, int ci, int cj)
{
    const unsigned ntiles = A->n/A->ts;
	const double alpha = -1.0;
	const double beta = 1.0;
	cblas_dgemm(
		CblasColMajor,
		CblasNoTrans,
		CblasTrans,
		A->ts, A->ts, A->ts,
		alpha,
		A->m[ai*ntiles + aj], A->ts,
		B->m[bi*ntiles + bj], B->ts,
		beta,
		C->m[ci*ntiles + cj], C->ts );
}

void cholesky_linear( matrix_t* matrix )
{
    const unsigned ntiles = matrix->n/matrix->ts;
	printf("Executing ...\n");
	for( unsigned k = 0; k < ntiles; ++k ) {
		// Diagonal Block factorization
		omp_potrf( matrix, k, k);

		// Triangular systems
		for( unsigned i = k + 1; i < ntiles; ++i ) {
			omp_trsm (matrix, k, k, matrix, k, i);
		}

		// update trailing matrix
		for( unsigned i = k + 1; i < ntiles; ++i ) {
			for( unsigned j = k + 1; j < i; ++j ) {
				omp_gemm (matrix, k, i, matrix, k, j, matrix, j, i);
			}
			omp_syrk (matrix, k, i, matrix, i, i);
		}
	}
	#pragma omp taskwait
}

//--------------------------- MAIN --------------------
int ompss_user_main(int argc, char* argv[])
{
	if ( argc != 3) {
		printf( "%s block_size input_file\n", argv[0] );
		exit( -1 );
	}
	const unsigned ts = atoi(argv[1]); // tile size
	const char* input = argv[2]; // input file name

	// Init matrix
	matrix_t* matrix = initialize_matrix( input, ts );

	const float t1 = get_time();
	cholesky_linear(matrix);
	const float t2 = get_time();

	const float time = t2 - t1;
    const unsigned n = matrix->n;
	const float gflops = (((1.0 / 3.0) * n * n * n) / ((time) * 1.0e+9));

	printf( "============ CHOLESKY RESULTS ============\n" );
	printf( "  matrix size:          %dx%d\n", n, n);
	printf( "  block size:           %dx%d\n", ts, ts);
	printf( "  time (s):             %f\n", time);
	printf( "  performance (gflops): %f\n", gflops);
	printf( "==========================================\n" );

    store_matrix( input, matrix );
	delete_matrix( matrix );
	return 0;
}

