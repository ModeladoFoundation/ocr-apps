#include "HTA.h"
#include "HTA_operations.h"
#include "HTAblas.h"
//#define HTABLAS_SEQ

void HTAblas_dgemm (const int order,
            const int transA,
            const int transB,
            const int M,
            const int N,
            const int K,
            const double alpha,
            double *A,
            const int lda,
            double *B,
            const int ldb,
            const double beta,
            double *C,
            const int ldc) {
    if(order != HTAblasColMajor)
    {
        printf("Specified Order not supported\n");
        return;
    }

    if(transA != HTAblasNoTrans || transB != HTAblasTrans)
    {
        printf("Specified Transposition not supported\n");
        return;
    }

    if(M != N || N != K)
    {
        printf("Matrices are not square\n");
        return;
    }

    if(lda != ldb || ldb != lda || lda != M)
    {
        printf("Input matrix sizes not valid\n");
        return;
    }

#ifdef HTABLAS_SEQ
    for(int j = 0; j < N; j++)
    {
        for(int i = 0; i < M; i++)
        {
            if(beta == 0.0)
                C[j*M + i] = 0.0;
            else if(beta != 1.0)
                C[j*M + i] = beta * C[j*M + i];
            else {
                for(int k = 0; k < K; k++)
                {
                    double temp = 0.0;
                    if(B[k*N + j] != 0.0)
                        temp = alpha*B[k*K + j];
                    // A[i,k] * B[j,k] (B is transposed)
                    C[j*M + i] += temp * A[k*M + i];
                }
            }
        }
    }
#else
    int p = 4;
    // TODO: for HTA version
    // Make input matrices into 1D tiled arrays
    // A, C has the same tiling (p tiles). B**T is transposed and non-tiled

    // Since input is column major (assumption made for this case only)
    // (1) A must be transposed since HTA library assumes row-major,
    //     and then it is partitioned into (p) tiles by row
    // (2) B**T can be acquired by accessing array elements in row major order
    //     thus, no transposition is required.
    // (3) The results have to be transposed to get column major format before
    //     writing to C[] and returning to the caller

    Tuple tilingA;

    Tuple flat_size;
    Dist dist;
    Tuple_init(&tilingA, 2, p, 1);
    Tuple_init(&flat_size, 2, M, M);

    HTA *ha = HTA_create_shell(A, 2, 2, &flat_size, ORDER_COL, &dist, HTA_SCALAR_TYPE_DOUBLE, 1, tilingA);
#if 0
    Tuple_init(&tilingA, 2, p, 1);
    Tuple_init(&flat_size, 2, M, M);
    // Create HTA ha for A      (1D partitioned by row)
    HTA *ha = HTA_create(2, 2, &flat_size, 0, &dist, HTA_SCALAR_TYPE_DOUBLE, 1, tilingA);
    // Initialize the tiles with array A
    // FIXME: do it in parallel?
    int row_offset = 0;
    for(int i = 0; i < p; i++) {
        int num_rows = ha->tiles[i]->flat_size.values[0];
        int num_cols = ha->tiles[i]->flat_size.values[1];
        double* ptr = HTA_get_ptr_raw_data(ha->tiles[i]);
        ASSERT(num_cols == M);
        // fill in elements in the tile of ha
        for(int row = 0; row < num_rows; row++) {
            for(int col = 0; col < num_cols; col++) {
                // Read from A in column major order but write to ha tile in row major
                // ptr[row*num_cols + col] = A[col*M + (row_offset+row)];
                *ptr = A[col*M + (row_offset+row)];
                ptr++;
            }
        }
        row_offset += num_rows;
    }
#endif

    HTA *hb = HTA_create_shell(B, 2, 1, &flat_size, ORDER_COL, &dist, HTA_SCALAR_TYPE_DOUBLE, 0);
    HTA *hc = HTA_create_shell(C, 2, 2, &flat_size, ORDER_COL, &dist, HTA_SCALAR_TYPE_DOUBLE, 1, tilingA);
#if 0
    // Create HTA hb for B**T   (Non-tiled)
    HTA *hb = HTA_create(2, 1, &flat_size, 0, &dist, HTA_SCALAR_TYPE_DOUBLE, 0);
    HTA_init_with_array(hb, B);
    // Create HTA hc for C      (1D partitioned by row, same as A)
    HTA *hc = HTA_create(2, 2, &flat_size, 0, &dist, HTA_SCALAR_TYPE_DOUBLE, 1, tilingA);
    row_offset = 0;
    for(int i = 0; i < p; i++) {
        int num_rows = hc->tiles[i]->flat_size.values[0];
        int num_cols = hc->tiles[i]->flat_size.values[1];
        double* ptr = HTA_get_ptr_raw_data(hc->tiles[i]);
        ASSERT(num_cols == M);
        // fill in elements in the tile of ha
        for(int row = 0; row < num_rows; row++) {
            for(int col = 0; col < num_cols; col++) {
                // Read from C in column major order but write to hc tile in row major
                *ptr = C[col*M + (row_offset+row)];
                ptr++;
            }
        }
        row_offset += num_rows;
    }
#endif
    // Call HTA operation which performs matrix-matrix multiplications in parallel
    // to produce results in C
    HTA_tile_to_hta(HTA_LEAF_LEVEL(hc), H3_DGEMM, hc, ha, hb);
#if 0
    row_offset = 0;
    for(int i = 0; i < p; i++) {
        int num_rows = hc->tiles[i]->flat_size.values[0];
        int num_cols = hc->tiles[i]->flat_size.values[1];
        double* ptr = HTA_get_ptr_raw_data(hc->tiles[i]);
        ASSERT(num_cols == M);
        // fill in elements in the tile of ha
        for(int row = 0; row < num_rows; row++) {
            for(int col = 0; col < num_cols; col++) {
                // Traverse hc tile contents in row major and write to C in column major
                C[col*M + (row_offset+row)] = *ptr;
                ptr++;
            }
        }
        row_offset += num_rows;
    }
#endif
    // Write contents of hc to C
    HTA_destroy(ha);
    HTA_destroy(hb);
    HTA_destroy(hc);
#endif
}
