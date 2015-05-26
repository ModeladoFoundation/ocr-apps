#include <stdio.h>
#include <stdint.h>
#include "HTA.h"
#include "Alloc.h"
#include "test.h"

#define BLK_SIZE 1024
#define NUM_BLOCKS 10
int hta_main(int argc, char** argv, int pid)
{
    void* pointers[NUM_BLOCKS];
    uint64_t block_id[NUM_BLOCKS];
    int i;
    for(i = 0; i < NUM_BLOCKS; i++)
    {
        pointers[i] = Alloc_acquire_block(BLK_SIZE);
        if(pointers[i] == NULL)
            return ERR_ALLOC;
        block_id[i] = Alloc_get_block_id(pointers[i]);
        printf(" block_id:   %lu\n", block_id[i]);
    }
    for(i = 0; i < NUM_BLOCKS; i++)
        Alloc_free_block(pointers[i]);

    return 0;
}
