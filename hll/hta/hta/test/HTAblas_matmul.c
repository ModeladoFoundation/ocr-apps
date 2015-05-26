#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "HTA.h"
#include "HTAblas.h"
#include "HTA_operations.h"
#include "Tuple.h"
#include "test.h"
#define MATRIX_WIDTH (16)
#define MATRIX_SIZE  (MATRIX_WIDTH*MATRIX_WIDTH)

/* MATRICES ARE ASSUMED TO BE COLUMN MAJOR */

void matmul_direct(int width, double c[], double a[], double b[])
{
    int i, j, k;
    for(i = 0; i < width; i++)
        for(j = 0; j < width; j++)
            for(k = 0; k < width; k++)
                c[j*width + i] -= a[k*width + i] * b[k*width + j]; // C = A*B**T + C
}

void mat_print(int width, double m[])
{
    int i, j;
    for(i = 0; i < width; i++)
    {
        for(j = 0; j < width; j++)
            printf("%.2lf ", m[j * width + i]);
        printf("\n");
    }
}

int main()
{
    double A[MATRIX_SIZE], B[MATRIX_SIZE], C[MATRIX_SIZE];
    double G[MATRIX_SIZE];
    int i/*, j, k*/;
    int cmp_result;

    // Initialize HTA (and PIL)
    hta_init();

    // create a 2D matrix
    srand(time(NULL));

    for(i = 0; i < MATRIX_SIZE; i++)
    {
        A[i] = ((rand() % 100) > 50) ? 0.0 : 1.0;
        B[i] = ((rand() % 100) > 50) ? 0.0 : 1.0;
        C[i] = 0.0;
        G[i] = 0.0;
    }

    printf("The content of matrix A\n");
    mat_print(MATRIX_WIDTH, A);
    printf("The content of matrix B\n");
    mat_print(MATRIX_WIDTH, B);

    HTAblas_dgemm(HTAblasColMajor, HTAblasNoTrans, HTAblasTrans, MATRIX_WIDTH, MATRIX_WIDTH, MATRIX_WIDTH, 1.0,
            A, MATRIX_WIDTH, B, MATRIX_WIDTH, 1.0, C, MATRIX_WIDTH);

    printf("The result of matmul on HTA\n");
    mat_print(MATRIX_WIDTH, C);

    matmul_direct(MATRIX_WIDTH, G, A, B);
    printf("The result of matmul_direct\n");
    mat_print(MATRIX_WIDTH, G);

    printf("Comparing matrix content %lu bytes\n", sizeof(C));
    cmp_result = memcmp(C, G, sizeof(C));
    if(cmp_result == 0)
        printf("** result matches! **\n");
    else {
        printf("** result does not match! **\n");
        exit(ERR_UNMATCH);
    }

    if(Alloc_count_objects() > 0) {
        printf("Objects left (memory leak) %d\n", Alloc_count_objects());
        exit(ERR_MEMLEAK);
    }
    exit(SUCCESS);
    return 0;
}
