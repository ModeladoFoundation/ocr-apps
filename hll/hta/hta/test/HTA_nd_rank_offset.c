#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "Debug.h"
#include "HTA.h"
#include "Tuple.h"
#include "Distribution.h"
#include "test.h"

#define MATRIX_WIDTH (10)
#define MATRIX_SIZE (MATRIX_WIDTH*MATRIX_WIDTH)
#ifdef PILHTA
int hta_main(int argc, char** argv)
#else
int main()
#endif
{
    int i;

    Tuple tiling[2];
    tiling[0] = Tuple_create(2, 3, 3);
    tiling[0].height = 2;
    tiling[1] = Tuple_create(2, 2, 2);
    tiling[1].height = 1;
    Tuple flat_size = Tuple_create(2, MATRIX_WIDTH, MATRIX_WIDTH);

    Dist dist;
    Dist_init(&dist, 0);
    // create an empty shell
    HTA* h = HTA_create_with_ts(2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_UINT32, 2, tiling);
 
    Tuple iter[2];
    Tuple nd_offset;
    Tuple_init_zero(&nd_offset, 2);
   
    Tuple_iterator_begin(2, 2, iter);
    do {
        Tuple_get_tile_start_offset(&flat_size, tiling, iter, &nd_offset);
        Tuple_print(&nd_offset);
        HTA *l = HTA_iterator_to_hta(h, iter);
        Tuple_print(&l->nd_element_offset);
        for(i = 0; i < 2; i++)
        {
            if(l->nd_element_offset.values[i] != nd_offset.values[i])
            exit(ERR_UNMATCH);
        }
    } while(Tuple_iterator_next(tiling, iter));

    HTA_destroy(h);

    if(Alloc_count_objects() > 0) {
        printf("Objects left (memory leak) %d\n", Alloc_count_objects());
        exit(ERR_MEMLEAK);
    }
    exit(SUCCESS);
    return 0;
}
