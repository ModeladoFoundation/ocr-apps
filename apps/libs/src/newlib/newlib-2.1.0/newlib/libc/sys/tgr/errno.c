/* Supply a definition of errno if one not already provided.  */
#include <errno.h>
#include <reent.h>

// This is a hook to force this file to be linked in (static linking).
// This is required for libtgr / libscaffold.
const int __provide_errno = 0;

int errno;

#ifndef _REENT_ONLY

int *
__errno ()
{
  return &_REENT->_errno;
}

#endif
