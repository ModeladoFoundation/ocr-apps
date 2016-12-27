#include "tgr.h"

s8 tgr_gettimeofday(struct timeval *ptimeval, void *ptimezone)
{
    gettimeofday_req req;

    req.timeval_ptr = (uint64_t) ptimeval;
    req.timezone_ptr = (uint64_t) ptimezone;

    int status = send_req(CE_REQTYPE_GETTIMEOFDAY, & req, sizeof(req));

    RETURN(status)
}

s8 tgr_gethostname(char * hostname, size_t len )
{
    gethostname_req req;

    req.hname_ptr = (uint64_t) hostname;
    req.size = len;

    int status = send_req(CE_REQTYPE_GETHOSTNAME, & req, sizeof(req));

    RETURN(status)
}
