/* Supply a definition of errno if one not already provided.  */
#include <errno.h>
#include <reent.h>

int errno;

#ifndef _REENT_ONLY

int *
__errno ()
{
  return &_REENT->_errno;
}

#endif
