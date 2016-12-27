#include "tgr.h"

#define PAGE_SIZE   8   // we're guaranteed 8 byte alignment

s64 tgr_getpagesize(void)
{
    return PAGE_SIZE;
}

s8 tgr_mmap( void** addr, size_t len)
{
    memalloc_req req;

	req.region = MREQ_ANY;	// ANY auto-promotes XXX add interface to choose locality?
    req.va = 0; 			// 0 = let CE choose
    req.len = len;          // will be rounded up by the CE
	req.private = 0;		// XE private or shared ? (0 = shared)
	req.promote = 0;		// choose from 'farther' if doesn't fit choice?

    int status = send_req(CE_REQTYPE_MEMALLOC, & req, sizeof(req));

    // Check for error.
    if( status )
		*addr = NULL;
    else
		*addr = (void *) req.va;

    RETURN(status)
}

s8 tgr_munmap(void * va, size_t len /* unused, but verified. */)
{
    memfree_req req;

    req.va = (uint64_t) va;

    int status = send_req(CE_REQTYPE_MEMFREE, & req, sizeof(req));

    RETURN(status)
}
