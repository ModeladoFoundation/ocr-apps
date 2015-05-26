#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "Debug.h"
#include "HTA.h"
#include "HTA_operations.h"
#include "Region.h"
#include "test.h"
#define NUM_LEAF_TILES_X (3*5)
#define NUM_LEAF_TILES_Y (3*5)
#define MATRIX_WIDTH (3*5*4)
#define MATRIX_SIZE (3*3*5*5*4*4)

void mat_print(int width, int32_t m[])
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
    int32_t M[MATRIX_SIZE];
    int32_t G[MATRIX_SIZE];
    int lhs_sel[NUM_LEAF_TILES_X * NUM_LEAF_TILES_Y];
    int rhs_sel[NUM_LEAF_TILES_X * NUM_LEAF_TILES_Y];
    Region lhs_elem_sel, rhs_elem_sel;
    Range lhs_elem_range_x, lhs_elem_range_y, rhs_elem_range_x, rhs_elem_range_y;
    int i, j, err;
    int cmp_result;

    Tuple t0 = Tuple_create(2, 3, 3);
    Tuple t1 = Tuple_create(2, 5, 5);
    Tuple flat_size = Tuple_create(2, MATRIX_WIDTH, MATRIX_WIDTH);

    Tuple mesh = HTA_get_vp_mesh(2);
    Tuple_print(&mesh);

    Dist dist;
    Dist_init(&dist, DIST_BLOCK, &mesh);
    // create an empty shell
    HTA *h = HTA_create_with_pid(pid, 2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_INT32,
            2, t0, t1);
    HTA *h2 = HTA_create_with_pid(pid, 2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_INT32,
            2, t0, t1);

    // create a 2D matrix
    for(i = 0; i < MATRIX_SIZE; i++) {
        M[i] = i%4+1;
	G[i] = M[i];
    }
    if (pid == 0 || pid == -1) {
        printf("Original matrix:\n");
        mat_print(MATRIX_WIDTH, M);
    }

    // initialize the HTA using 2D matrix
    HTA_init_with_array(h, M);

    // tile selections
    for(i = 0; i < NUM_LEAF_TILES_X; i++)
        for(j = 0; j < NUM_LEAF_TILES_Y; j++) {
            lhs_sel[i * NUM_LEAF_TILES_Y + j] = ((j == 0) ? 1 : 0); // select the 0th cloumn
            rhs_sel[i * NUM_LEAF_TILES_Y + j] = ((j == 14) ? 1 : 0); // select the last cloumn
        }

    if (pid == 0 || pid == -1) {
        printf("LHS selection:\n");
        mat_print(NUM_LEAF_TILES_Y, lhs_sel);
        printf("RHS selection:\n");
        mat_print(NUM_LEAF_TILES_Y, rhs_sel);
    }

    // element region selections
    Range_init(&lhs_elem_range_x, 0, 3, 1, 0); // [0:3],
    Range_init(&lhs_elem_range_y, -1, -1, 1, 0);   // [3]
    Region_init(&lhs_elem_sel, 2, lhs_elem_range_x, lhs_elem_range_y);
    Range_init(&rhs_elem_range_x, 0, 3, 1, 0); // [0:3],
    Range_init(&rhs_elem_range_y, 0, 0, 1, 0);   // [0]
    Region_init(&rhs_elem_sel, 2, rhs_elem_range_x, rhs_elem_range_y);

    HTA_assign(lhs_sel, &lhs_elem_sel, h, rhs_sel, &rhs_elem_sel, h);

    for(i = 0; i < MATRIX_SIZE; i++) {
        M[i] = 0;
    }
    // dump HTA data
    HTA_flatten(M, NULL, NULL, h);
    if (pid == 0 || pid == -1) {
        printf("Result matrix:\n");
        mat_print(MATRIX_WIDTH, M);
    }

    //printf("thread(%d) comparing %zd bytes\n", pid, sizeof(M));
    //cmp_result = memcmp(M, G, sizeof(M));
    //if(cmp_result == 0) {
    //    printf("thread(%d) map_h2 all results match!\n", pid);
    //    err = SUCCESS;
    //}
    //else {
    //    printf("thread(%d) map_h2 results mismatch!\n", pid);
    //    err = ERR_UNMATCH;
    //}

    // dump HTA data
    HTA_destroy(h);

    return 0;
}
