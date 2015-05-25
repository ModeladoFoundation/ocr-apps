#include <stdio.h>
#include <assert.h>
#include "Mapping.h"

int hta_main(int argc, char** argv, int pid)
{
    Tuple t[2];
    Tuple_fill_in_sequence(t, 2, Tuple_create(2, 3, 3), Tuple_create(2, 2, 2));
    Tuple flat_size;
    Tuple_init(&flat_size, 2, 3*2*3, 3*2*3);

    Tuple mesh;
    Tuple_init(&mesh, 1, 8);
    mesh.height = 1;

    Dist dist;
    Dist_init(&dist, DIST_BLOCK, &mesh);
    Mapping *m =  Mapping_create(2, 3, 0, &flat_size, t, &dist, sizeof(double));
    Mapping_print(m);
    Mapping_destroy(m);

    return 0;
}
