#include <stdio.h>
#include <assert.h>
#include "RefCount.h"
#include "Region.h"

#ifdef PILHTA
int hta_main(int argc, char** argv)
#else
int main()
#endif
{
    Range r1;
    Range r2;
    Range r3;

    printf("Build a region with 3 ranges:\n");
    Range_init(&r1, 0, 99, 10, 0);
    Range_print(&r1);

    Range_init(&r2, 1, 4, 2, 0);
    Range_print(&r2);

    Range_init(&r3, 1, 8, 1, 0);
    Range_print(&r3);

    Region rgn;
    Region_init(&rgn, 3, r1, r2, r3);
    Region_print(&rgn);

    printf("Select a region within a tile\n");
    Tuple t = Tuple_create(2, 10, 10);
    Range row;
    Range col;
    Range_init(&row, 1, 8, 4, 0);
    Range_init(&col, 0, 9, 3, 0);
    Region x;
    Region_init(&x, 2, row, col);
    Region_print(&x);
    for(int i = 0; i < Region_cardinality(&x); i++) {
        printf("Region linear idx = %d, tile linear idx = %d\n", i, Region_idx_to_tile_idx(&x, &t, i));
    }

    return 0;
}
