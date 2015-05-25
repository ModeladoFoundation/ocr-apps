#include <stdio.h>
#include <stdint.h>
#include "HTA.h"

int hta_main(int argc, char** argv, int pid)
{
    // 2D tiling on 2D mesh block
    Tuple mesh;
    Tuple_init(&mesh, 2, 4, 4);
    mesh.height = 1;
    Dist dist;
    Dist_init(&dist, DIST_BLOCK, &mesh);

    // 1D tiling on 1D mesh cyclic
    Dist dist_cyclic;
    Dist_init(&dist_cyclic, DIST_CYCLIC, &mesh);

    // 16 tiles
    Tuple tiling;
    Tuple_init(&tiling, 2, 5, 13);
    tiling.height = 1;

    Tuple iter;
    Tuple_iterator_begin(2, 1, &iter);

    printf("Mapping with block distribution\n");
    do {
        int pid = Dist_get_pid(&dist, &iter, &tiling);
        Tuple_print(&iter);
        printf("pid = %d\n", pid);
    } while(Tuple_iterator_next(&tiling, &iter));

    printf("Mapping with cyclic distribution\n");
    Tuple_iterator_begin(2, 1, &iter);
    do {
        int pid = Dist_get_pid(&dist_cyclic, &iter, &tiling);
        Tuple_print(&iter);
        printf("pid = %d\n", pid);
    } while(Tuple_iterator_next(&tiling, &iter));

    return 0;
}
