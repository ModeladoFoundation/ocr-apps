#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "Debug.h"
#include "HTA.h"
#include "Tuple.h"
#include "Partition.h"
#include "Distribution.h"
#include "test.h"

#define MATRIX_WIDTH (3*5*2)
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
    int i;

    Tuple t0 = Tuple_create(2, 3, 3);
    Tuple t1 = Tuple_create(2, 5, 5);
    //Tuple t2 = Tuple_create(2, 4, 4);
    Tuple flat_size = Tuple_create(2, MATRIX_WIDTH, MATRIX_WIDTH);

    Tuple mesh;
    Tuple_init(&mesh, 1, 8);
    mesh.height = 1;

    Dist dist;
    Dist_init(&dist, DIST_BLOCK, &mesh);
    // create an empty shell
    HTA* h = HTA_create(2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_UINT32, 2, t0, t1);

    // create a 2D matrix
    srand(time(NULL)); /* FIXME: using random to initialize distributed model will fail */
    for(i = 0; i < MATRIX_SIZE; i++)
    {
        M[i] = rand() % 100;
        R1[i] = 0;
    }

    // initialize the HTA using 2D matrix
    HTA_init_with_array(h, M); // copy from process memory to memory allocated for HTAs

    Partition src_part = Partition_create(2, 0, 0);
    Partition offset = Partition_create(2, 1, 1);
    HTA_part(h, &src_part, &offset);

    HTA_to_array(h, R1); // copy from HTA memory to process memory
    printf("Original matrix content\n");
    mat_print(MATRIX_WIDTH, M);
    printf("HTA_part result matrix content\n");
    mat_print(MATRIX_WIDTH, R1);

    Partition rmpart = Partition_create(2, 2, 2);
    HTA_rmpart(h, &rmpart);

    HTA_to_array(h, R1); // copy from HTA memory to process memory
    printf("HTA_rmpart result matrix content\n");
    mat_print(MATRIX_WIDTH, R1);
    // verify the result
    printf("comparing %zd bytes\n", sizeof(R1));
    if(memcmp(M, R1, sizeof(R1)) == 0)
        printf("all results match!\n");
    else {
        printf("incorrect result detected!\n");
        exit(ERR_UNMATCH);
    }

    HTA_destroy(h);

    exit(SUCCESS);
    return 0;
}
