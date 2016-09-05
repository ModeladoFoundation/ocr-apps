#include <errno.h>

#include "tgr.h"

#define PAGE_SIZE 4096 // Hardcoded to 4K for now.

s64 tgr_getpagesize(void)
{
    return PAGE_SIZE;
}

s8 tgr_mmap(u64* addr, u64 len)
{
    struct {
        uint64_t   va;      // out
        uint64_t   len;     // in
    } req;

    req.va = 0; //NULL
    req.len = len;

    if (len % PAGE_SIZE) {
        ERRNO = EINVAL;
        return -1;
    }

    int status = send_req(CE_REQTYPE_MEMALLOC, & req, sizeof(req));

    // Check for error.
    if( status ) *addr = (u64)0L;
    else *addr = req.va;

    RETURN(status)
}

s8 tgr_munmap(u64* va, u64 len /* unused, but verified. */)
{
    struct {
        uint64_t   va;      // in
    } req;

    // Even though we do not need it, verify len is sane.
    if (len % PAGE_SIZE) {
        ERRNO = EINVAL;
        return -1;
    }

    req.va = (uint64_t) va;

    int status = send_req(CE_REQTYPE_MEMFREE, & req, sizeof(req));

    RETURN(status)
}
