#include <stdio.h>
#include <assert.h>
#include "Mapping.h"

#ifdef PILHTA
int hta_main(int argc, char** argv)
#else
int main()
#endif
{
    Tuple t[2];
    Tuple_fill_in_sequence(t, 2, Tuple_create(2, 3, 3), Tuple_create(2, 2, 2));
    Tuple flat_size;
    Tuple_init(&flat_size, 2, 3*2*3, 3*2*3);

    Dist dist;
    Dist_init(&dist, 0);
    Mapping *m =  Mapping_create(2, 3, 0, &flat_size, t, &dist, sizeof(double));
    Mapping_print(m);
    Mapping_destroy(m);

    printf("Objects left (memory leak) %d\n", Alloc_count_objects());
    return 0;
}
