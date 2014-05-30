#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HTA.h"
#include "Tuple.h"
#include "Distribution.h"
#include "test.h"
#define L1_WIDTH (2)
#define L2_WIDTH (4)
#define L3_WIDTH (2)
#define FLAT_WIDTH (L1_WIDTH*L2_WIDTH*L3_WIDTH)

#ifdef PILHTA
int hta_main(int argc, char** argv)
#else
int main()
#endif
{
    // tuples are linked to form a sequence
    // if the sequence is used in HTA construction,
    // it is implicitly destroyed during HTA destruction
    Tuple flat_size = Tuple_create(2, FLAT_WIDTH, FLAT_WIDTH);

    Dist dist;
    // create an empty shell
    HTA *h = HTA_create(2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_INT32, 
            2, Tuple_create(2, L1_WIDTH, L1_WIDTH), Tuple_create(2, L2_WIDTH, L2_WIDTH));
    // Print recursively
    HTA_print(h, 1);

    // Access a tile
    Tuple a0 = Tuple_create(2, 0, 0);
    Tuple a1 = Tuple_create(2, 1, 1);
    void * acc[2] = {&a0, &a1};

    printf("Select a tile test\n");
    Tuple_print(&a0);
    Tuple_print(&a1);
    HTA *z = HTA_select(h, acc, 2);
    printf("Selected tile\n");
    //HTA_print(z, 1);

    // Access a region
    Tuple x0 = Tuple_create(2, 1, 1);
    Range row;
    Range col;
    Range_init(&row, 0, 3, 1, 0);
    Range_init(&col, 0, 3, 2, 0);
    Region r;
    Region_init(&r, 2, row, col);
    void* acc2[2] = {&x0, &r};

    printf("Select a region test\n");
    Tuple_print(&x0);
    Region_print(&r);
    HTA *mz = HTA_select(h, acc2, 2);
    printf("Selected tile\n");
    HTA_print(mz, 1);

    printf("Destroy h\n");
    HTA_destroy(h); // HTA destruction calls Tuple_destroy_all on its tiling
  
    // Explicit destruction of accessor tuples are needed
    // because they are not used in HTA tiling information
    printf("Destroy z\n");
    HTA_destroy(z); // Tuple_destroy_all on its tiling

    // Explicit destruction of accessor tuples are needed
    // because they are not used in HTA tiling information
    printf("Destroy mz\n");
    HTA_destroy(mz); // Tuple_destroy_all on its tiling

    if(Alloc_count_objects() > 0) {
        printf("Objects left (memory leak) %d\n", Alloc_count_objects());
        exit(ERR_MEMLEAK);
    }
    exit(SUCCESS);
    return 0;
}
