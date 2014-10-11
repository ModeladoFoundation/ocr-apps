#include <stdio.h>
#include <assert.h>
#include "Tuple.h"

#ifdef PILHTA
int hta_main(int argc, char** argv)
#else
int main()
#endif
{
    Tuple t[3];
    //t[0] = Tuple_create(2, 5, 3);
    Tuple_init(&t[0], 2, 5, 3);
    t[0].height = 3;
    //t[1] = Tuple_create(2, 11, 22);
    Tuple_init(&t[1], 2, 11, 22);
    t[1].height = 2;
    //t[2] = Tuple_create(4, 33, 44, 55, 22);
    Tuple_init(&t[2], 4, 33, 44, 55, 22);
    t[2].height = 1;
    Tuple_print(&t[0]);
    Tuple_print(&t[1]);
    Tuple_print(&t[2]);
    printf("Level 0: tiles=%d\n", Tuple_count_elements(&t[0], 0));
    printf("Level 1: tiles=%d\n", Tuple_count_elements(&t[0], 1));
    printf("Level 2: tiles=%d\n", Tuple_count_elements(&t[0], 2));
    printf("Level 3: tiles=%d\n", Tuple_count_elements(&t[0], 3));
    printf("Root level tuple\n");

    Tuple tiling[2];
    Tuple_init(&tiling[0], 3, 3, 3, 3);
    tiling[0].height = 2;
    Tuple_init(&tiling[1], 3, 2, 2, 2);
    tiling[1].height = 1;
    Tuple flat_size;
    Tuple_init(&flat_size, 3, 128, 128, 128);
    Tuple nd_offset;
    Tuple iter[2];
    Tuple_iterator_begin(3, 2, iter);
    do {
        Tuple_get_tile_start_offset(&flat_size, tiling, iter, &nd_offset);
        printf("Iterator: (%4d, %4d, %4d)(%4d, %4d, %4d)\n", iter[0].values[0], iter[0].values[1], iter[0].values[2],
                iter[1].values[0], iter[1].values[1], iter[1].values[2]);
        Tuple_print(&nd_offset);
    } while(Tuple_iterator_next(tiling, iter));

    Tuple_print(t);

    return 0;
}
