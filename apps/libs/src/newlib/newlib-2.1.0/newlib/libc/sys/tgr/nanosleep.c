#include <_ansi.h>
#include <_syslist.h>
#include <time.h>
#include <sys/times.h>
#include <reent.h>
#include <errno.h>
#include <sys/tgr.h>

int
_DEFUN (_nanosleep_r, (reent, req, rem),
        struct _reent *reent _AND
        const struct timespec *req _AND
        struct timespec *rem)
{
    struct timeval tv;
    int ret = tgr_gettimeofday(&tv, NULL);
    if (ret) {
        return ret;
    }

    struct timespec ts;
    ts.tv_sec = tv.tv_sec + req->tv_sec;
    ts.tv_nsec = tv.tv_usec / 1000 + req->tv_nsec;

    ret = tgr_suspend(&ts);
    if (! ret) {
        // We didn't time out. We must have gotten a resume signal.
        if (rem) {
            struct timeval tv2;
            int ret = tgr_gettimeofday(&tv2, NULL);
            if (ret) {
                return ret;
            }

            rem->tv_sec = ts.tv_sec - tv.tv_sec;
            rem->tv_nsec = ts.tv_nsec - tv.tv_usec / 1000;
        }

        return EINTR;
    }
    return ret;
}

int
_DEFUN (_nanosleep, (req, rem),
        const struct timespec *req _AND
        struct timespec *rem)
{
    return _nanosleep_r( _REENT, req, rem );
}

weak_alias(_nanosleep, nanosleep);
