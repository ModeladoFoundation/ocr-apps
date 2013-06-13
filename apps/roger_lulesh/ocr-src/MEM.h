#if CILK
extern "C" {
#endif

#if  defined(HAB_C)

#define DRAM_FREE(ptr) free(ptr)
#define DRAM_MALLOC(count,sizeof_type) malloc((count)*(sizeof_type))
#define SPAD_FREE(ptr) free(ptr)
#define SPAD_MALLOC(count,sizeof_type) malloc((count)*(sizeof_type))

#elif defined(CILK) // CILK

#define DRAM_FREE(ptr) free(ptr)
#define DRAM_MALLOC(count,sizeof_type) malloc((count)*(sizeof_type))
#define SPAD_FREE(ptr) free(ptr)
#define SPAD_MALLOC(count,sizeof_type) malloc((count)*(sizeof_type))

#elif defined(UPC) // UPC

#define DRAM_FREE(ptr) upc_free(ptr)
#define DRAM_MALLOC(count,sizeof_type) upc_global_alloc((count),(sizeof_type))
#define SPAD_FREE(ptr) free(ptr)
#define SPAD_MALLOC(count,sizeof_type) malloc((count)*(sizeof_type))

#elif defined(FSIM) // FSIM

#define DRAM_FREE(ptr)                 dram_free(ptr)
#define DRAM_MALLOC(count,sizeof_type) dram_alloc(&domainObject,(count),(sizeof_type))
#define SPAD_FREE(ptr)                 spad_free(ptr)
#define SPAD_MALLOC(count,sizeof_type) spad_alloc((count)*(sizeof_type))

void  dram_free(void *ptr);
void *dram_alloc(struct DomainObject_t *domainObject, size_t count, size_t sizeof_type);
void  spad_free(void *ptr);
void *spad_alloc(size_t len);

#define DOMAIN_CREATE(object,edgeElems,edgeNodes)         domain_create(object,edgeElems,edgeNodes)
#define DOMAIN_DESTROY(object)         domain_destroy(object)

void  domain_create(struct DomainObject_t *domainObject, size_t edgeElems, size_t edgeNodes);
void  domain_destroy(struct DomainObject_t *domainObject);

#elif defined(OCR)  // OCR

void  dram_free(void *ptr);
void *dram_alloc(struct DomainObject_t *domainObject, size_t count, size_t sizeof_type);
void  spad_free(void *ptr);
void *spad_alloc(size_t len);

#define DRAM_FREE(ptr)                 dram_free(ptr)
#define DRAM_MALLOC(count,sizeof_type) dram_alloc(&domainObject,(count),(sizeof_type))
#if       OCR_SPAD_WORKAROUND == 0
#define SPAD_FREE(ptr)                 spad_free(ptr)
#define SPAD_MALLOC(count,sizeof_type) spad_alloc((count)*(sizeof_type))
#else  // OCR_SPAD_WORKAROUND != 0
#define SPAD_FREE(ptr) free(ptr)
#define SPAD_MALLOC(count,sizeof_type) malloc((count)*(sizeof_type))
#endif // OCR_SPAD_WORKAROUND

#define DOMAIN_CREATE(object,edgeElems,edgeNodes)         domain_create(object,edgeElems,edgeNodes)
#define DOMAIN_DESTROY(object)         domain_destroy(object)

void  domain_create(struct DomainObject_t *domainObject, size_t edgeElems, size_t edgeNodes);
void  domain_destroy(struct DomainObject_t *domainObject);

#else // DEFAULT is C99

#define DRAM_FREE(ptr) free(ptr)
#define DRAM_MALLOC(count,sizeof_type) malloc((count)*(sizeof_type))
#define SPAD_FREE(ptr) free(ptr)
#define SPAD_MALLOC(count,sizeof_type) malloc((count)*(sizeof_type))

#endif // HAB_C, CILK, UPC, FSIM, ORC, or C99

#if CILK
} // extern "C"
#endif
