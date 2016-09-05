/* stub to allow libpthread to override */

#include <stdio.h>

void __libc_flockfile (FILE *fp)
{
}
weak_alias(__libc_flockfile,flockfile)

void __libc_funlockfile (FILE *fp)
{
}
weak_alias(__libc_funlockfile,funlockfile)
