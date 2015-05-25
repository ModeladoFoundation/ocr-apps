#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "Debug.h"
#include "Comm.h"
#include "HTA_operations.h"
#include "HTA.h"
#include "Tuple.h"
#include "Partition.h"
#include "Distribution.h"
#include "test.h"

#define MATRIX_WIDTH (15)
#define MATRIX_SIZE (MATRIX_WIDTH*MATRIX_WIDTH)

void mat_print(int width, uint32_t m[])
{
    int i, j;
    for(i = 0; i < width; i++)
    {
        for(j = 0; j < width; j++)
            printf("%u ", m[i * width + j]);
        printf("\n");
    }
}
int hta_main(int argc, char** argv, int pid)
{
    uint32_t M[MATRIX_SIZE];
    uint32_t R1[MATRIX_SIZE];
    int i, err, all_err=SUCCESS;

    // create a 2D matrix
    srand(time(NULL)); /* FIXME: using random to initialize distributed model will fail */
    for(i = 0; i < MATRIX_SIZE; i++)
    {
        M[i] = rand() % 100;
        R1[i] = 0;
    }

    Partition parts[2];
    parts[0] = Partition_create(1, 7);
    parts[1] = Partition_create(4, 5, 9, 12, 14);
    Tuple matrix_size = Tuple_create(2, MATRIX_WIDTH, MATRIX_WIDTH);

    // initialize the HTA using 2D matrix
    HTA* h = HTA_part_matrix(2, M, &matrix_size, HTA_SCALAR_TYPE_UINT32, parts);

    HTA_to_array(h, R1); // copy from HTA memory to process memory
    //HTA_flatten(R1, NULL, NULL, h);

    printf("Original matrix content\n");
    mat_print(MATRIX_WIDTH, M);
    printf("HTA_part result matrix content\n");
    mat_print(MATRIX_WIDTH, R1);

    // verify the result
    printf("thread(%d) comparing %zd bytes\n", pid, sizeof(uint32_t) * MATRIX_SIZE);
    if(memcmp(M, R1, sizeof(uint32_t) * MATRIX_SIZE) == 0) {
        printf("thread (%d) all results match!\n",pid);
        err = SUCCESS;
    }
    else {
        printf("thread (%d) incorrect result detected!\n", pid);
        err = ERR_UNMATCH;
    }

    HTA_destroy(h);

    all_err = SUCCESS;
    comm_allreduce(pid, REDUCE_MAX, &err, &all_err, HTA_SCALAR_TYPE_INT32);
    assert(all_err==SUCCESS);
    return 0;
}
