#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "Debug.h"
#include "HTA.h"
#include "Tuple.h"
#include "Distribution.h"
#include "test.h"

#define MATRIX_WIDTH (3*5*4)
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

    Tuple iter[2];
    int count = 0;
    HTA* t = HTA_iterator_begin(h, iter);
    do {
        printf("%dth tile (rank=%d):\n", count++, t->rank);
        Tuple_print(iter);
        Tuple_print(iter+1);
    }while((t = HTA_iterator_next(h, iter)));

    // verify the result
    //printf("comparing %zd bytes\n", sizeof(R1));
    //if(memcmp(M, R1, sizeof(R1)) == 0)
    //    printf("all results match!\n");
    //else {
    //    printf("incorrect result detected!\n");
    //    exit(ERR_UNMATCH);
    //}

    HTA_destroy(h);

    exit(SUCCESS);
    return 0;
}
