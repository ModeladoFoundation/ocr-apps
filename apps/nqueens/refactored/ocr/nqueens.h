
#ifndef NQUEENS_H
#define NQUEENS_H

#include <stdatomic.h>
#include <ocr.h>

struct nqueens_args
{
    u32       all;
    u32       ldiag;
    u32       cols;
    u32       rdiag;
};

// Computes Hamming-weight for an arbitrary integer
static inline u32 NumberOfSetBits( u32 i )
{
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

extern atomic_uint solutions;

static inline void solution_found()
{
    atomic_fetch_add( &solutions, 1 );
}

static inline u32 get_solution_number()
{
    return atomic_load( &solutions );
}

#endif

