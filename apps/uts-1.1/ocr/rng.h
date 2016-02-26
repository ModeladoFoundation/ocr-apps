#ifndef _RNG_H
#define _RNG_H

/***********************************************************
 *                                                         *
 *  splitable random number generator to use:              *
 *     (default)  sha1 hash                                *
 *                                                         *
 ***********************************************************/

#if defined(BRG_RNG)
#  include "brg_sha1.h"
#  define RNG_TYPE 0
#else
#  error "No random number generator selected."
#endif

#endif /* _RNG_H */
