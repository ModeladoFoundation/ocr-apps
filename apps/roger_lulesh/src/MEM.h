#if CILK
extern "C" {
#endif

#if  defined(HAB_C)

#define DRAM_FREE(ptr) free(ptr)
#define DRAM_MALLOC(count,sizeof_type) malloc((count)*(sizeof_type))

#elif defined(CILK) // CILK

#define DRAM_FREE(ptr) free(ptr)
#define DRAM_MALLOC(count,sizeof_type) malloc((count)*(sizeof_type))

#elif defined(UPC) // UPC

#define DRAM_FREE(ptr) upc_free(ptr)
#define DRAM_MALLOC(count,sizeof_type) upc_global_alloc((count),(sizeof_type))

#elif defined(FSIM)  || defined (OCR) // FSIM or OCR

void *DRAM_MALLOC(size_t count, size_t sizeof_type);
void  DRAM_FREE(void *ptr);

#else // DEFAULT is C99

#define DRAM_FREE(ptr) free(ptr)
#define DRAM_MALLOC(count,sizeof_type) malloc((count)*(sizeof_type))

#endif // HAB_C, CILK, UPC, FSIM, ORC, or C99

#if CILK
} // extern "C"
#endif
