#include <stdio.h>
#include <assert.h>
#include "Tuple.h"
#include "RefCount.h"
#ifdef PILHTA
int hta_main(int argc, char** argv)
#else
int main()
#endif
{
    Tuple t0 = Tuple_create(2, 5, 3);
    Tuple t1 = Tuple_create(2, 11, 22);
    Tuple t2 = Tuple_create(5, 33, 44, 55, 22, 22);
    
    Tuple_concat(Tuple_concat(t0, t1), t2);
    Tuple t3 = Tuple_create(2, 9, 9);
    Tuple_attach_with_retain(t3, t0);
    printf("Attach with retain(t3, t0)\n");
    Tuple_print(t3, 1);
    for(int i = 0; i < t3->height; i++) {
        int refcount = RefCount_get_counter(Tuple_at_level(t3, i));
        printf("%dth tuple has reference count %d\n", i, refcount);
    }

    Tuple_retain_all(t3);
    printf("Retain all(t3)\n");
    for(int i = 0; i < t3->height; i++) {
        int refcount = RefCount_get_counter(Tuple_at_level(t3, i));
        printf("%dth tuple has reference count %d\n", i, refcount);
    }

    Tuple_destroy_all(t3);
    printf("Destroy all(t3)\n");
    for(int i = 0; i < t3->height; i++) {
        int refcount = RefCount_get_counter(Tuple_at_level(t3, i));
        printf("%dth tuple has reference count %d\n", i, refcount);
    }

    Tuple_destroy_all(t3);
    printf("Destroy all(t3) again\n");
    for(int i = 0; i < t3->height; i++) {
        int refcount = RefCount_get_counter(Tuple_at_level(t3, i));
        printf("%dth tuple has reference count %d\n", i, refcount);
    }

    Tuple_destroy_all(t0);
    printf("Destroy all(t0)\n");
    printf("%u objects alive\n", RefCount_num_alive());
    assert(RefCount_num_alive() == 0 && "Make sure all memory objects are freed at this point");

    return 0;
}
