#ifndef __LEA_H__
#define __LEA_H__

// In x86 all addresses valid for one thread are valid for any other thread in that process

#define XSTG_LEA(ptr_in, ptr_out) ptr_out = ptr_in
static inline void * xstg_lea(void * ptr) {
    return ptr;
}

#endif
