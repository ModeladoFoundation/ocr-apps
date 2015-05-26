#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>
#include <assert.h>

#ifndef DBG_LVL
#define DBG_LVL (0)
#endif

#ifdef DEBUG
#define DBG(level, format, ...) ((level <= DBG_LVL) ? fprintf(stderr, format, ##__VA_ARGS__) : (void) 0)
#ifndef ASSERT
#define ASSERT(x) (assert(x))
#endif
#else
#define DBG(level, format, ...) ((void) 0)
#ifndef ASSERT
#define ASSERT(x) ((void) 0)
#endif
#endif

#define UNUSED(x) ((void) x)
#endif
