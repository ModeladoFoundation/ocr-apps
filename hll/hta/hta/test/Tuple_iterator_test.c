#include <stdio.h>
#include <assert.h>
#include "Tuple.h"

#ifdef PILHTA
int hta_main(int argc, char** argv)
#else
int main()
#endif
{
    Tuple t[2];
    //t[0] = Tuple_create(2, 5, 3);
    Tuple_init(&t[0], 2, 5, 3);
    t[0].height = 2;
    //t[1] = Tuple_create(2, 3, 7);
    Tuple_init(&t[1], 2, 3, 7);
    t[1].height = 1;

    Tuple flat_size;
    Tuple_init(&flat_size, 2, 1024, 1024);
    Tuple_print(t);

    Tuple iterator[2];
    Tuple_iterator_begin(2, 2, iterator);
    Tuple tile_size;
    Tuple_init_zero(&tile_size, 2);
    int num_leaves = Tuple_count_elements(t, 2);
    int num_elements = 0;
    printf("There are %d leaf tiles\n", num_leaves);
    do
    {
        printf("Leaf tile accessor\n");
        Tuple_print(&iterator[0]);
        Tuple_print(&iterator[1]);
        Tuple_get_leaf_tile_size(&flat_size, t, iterator, &tile_size);
        printf("Leaf tile dimension:\n");
        Tuple_print(&tile_size);
        num_elements += Tuple_product(&tile_size);
    } while(Tuple_iterator_next(t, iterator));

    printf("num_elements = %d\n", num_elements);
    assert(num_elements == 1024*1024);
    return 0;
}
