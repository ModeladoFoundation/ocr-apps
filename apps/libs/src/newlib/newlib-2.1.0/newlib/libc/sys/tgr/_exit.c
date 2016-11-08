#include <limits.h>
#include <_ansi.h>
#include <_syslist.h>
#include <sys/tgr.h>

void
_DEFUN (_exit, (rc),
    int rc)
{
    tgr_exit((s64)rc);
}

// Make sure that errno gets linked into the program because libtgr and libscaffold use it.
extern const int __provide_errno;
const int *const __require_errno = &__provide_errno;
