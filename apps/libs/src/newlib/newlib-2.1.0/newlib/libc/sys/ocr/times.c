#include <_ansi.h>
#include <_syslist.h>
#include <sys/times.h>
#include <reent.h>
#include <errno.h>
#include <sys/ocr.h>
#include <time.h>

int
_DEFUN (_nanosleep_r, (reent, req, rem),
        struct _reent *reent _AND
        const struct timespec *req _AND
        struct timespec *rem)
{
    if (!req || req->tv_nsec < 0 || req->tv_nsec > 999999999 || req->tv_sec < 0) {
        reent->_errno = EINVAL;
        return -1;
    }
    // We never have any remaining time.
    if (rem) {
        rem->tv_sec = 0;
        rem->tv_nsec = 0;
    }

    // Just busy loop for a while. The magic number below is just made up.
    unsigned long i = req->tv_sec * 10000 + req->tv_nsec;
    while (i--)
        ;

    return 0;
}

int
_DEFUN (_nanosleep, (req, rem),
        const struct timespec *req _AND
        struct timespec *rem)
{
    return _nanosleep_r( _REENT, req, rem );
}

weak_alias(_nanosleep, nanosleep)
