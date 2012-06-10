#ifndef CACHE_ISA_STUFF
#define CACHE_ISA_STUFF


#define CAC_WB_ADDR(__INP_PTR) \
{ \
    uint64_t __TEMP_CACHE_ADDR = (uint64_t)(__INP_PTR);\
    __asm__ (  \
        "cache.wbinvl %0" \
        : \
        : "r" (__TEMP_CACHE_ADDR) \
        : \
    );\
}
    
#endif
