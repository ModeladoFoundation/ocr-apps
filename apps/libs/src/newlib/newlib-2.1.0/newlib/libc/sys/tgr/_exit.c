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

