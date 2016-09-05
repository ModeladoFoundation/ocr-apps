#include <errno.h>

#include "tgr.h"

s8 tgr_gettimeofday(struct timeval *ptimeval, void *ptimezone)
{
    struct {
        uint64_t timeval_ptr; // in
        uint64_t timezone_ptr; // in
    } req;
    req.timeval_ptr = (uint64_t) ptimeval;
    req.timezone_ptr = (uint64_t) ptimezone;

    int status = send_req(CE_REQTYPE_GETTIMEOFDAY, & req, sizeof(req));

    RETURN(status)
}
