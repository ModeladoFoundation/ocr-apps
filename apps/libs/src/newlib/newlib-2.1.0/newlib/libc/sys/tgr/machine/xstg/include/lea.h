#ifndef __LEA_H__
#define __LEA_H__

// Take the pointer and do the "load effective address" instruction
// on it. The upshot of this is that any XE will be able to use this
// pointer then.
#define XSTG_LEA(ptr_in, ptr_out) \
__asm__ ("lea %0, %1" : "=r"(ptr_out) : "r"(ptr_in))

static inline void * xstg_lea(void * ptr)
{
    void * out;
    XSTG_LEA(ptr, out);
    return out;
}

#endif
