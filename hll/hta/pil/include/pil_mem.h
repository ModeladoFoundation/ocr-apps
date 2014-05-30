
#ifndef PIL_MEM_H
#define PIL_MEM_H

#include "pil.h"
#include <stddef.h> // needed by rstream for size_t

void pil_alloc(gpp_t *, size_t);
void *pil_mem(void *id);
void pil_free(gpp_t);
void *pil_realloc(void *, size_t);
void pil_release(void *id);

#endif
