#include <stdio.h>
#include <stdint.h>
#include "HTA.h"
#include "Alloc.h"

#define BLK_SIZE 1024
#define NUM_BLOCKS 10
#ifdef PILHTA
int hta_main(int argc, char** argv)
#else
int main()
#endif
{
    void* pointers[NUM_BLOCKS];
    uint64_t block_id[NUM_BLOCKS];
    int i;
    printf("Alloc_test starts, nothing exists = %d\n", Alloc_no_object_exists());
    for(i = 0; i < NUM_BLOCKS; i++)
    {
        pointers[i] = Alloc_acquire_block(-1, BLK_SIZE);
        block_id[i] = Alloc_get_block_id(pointers[i]);
        printf(" block_id:   %lu\n", block_id[i]);
    }
    for(i = 0; i < NUM_BLOCKS; i++)
        Alloc_free_block(pointers[i]);


    printf("Alloc_test finished, all_freed = %d\n", Alloc_no_object_exists());
    return 0;
}
