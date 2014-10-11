#ifndef __ACCESSOR_H__
#define __ACCESSOR_H__

#define ACCESSOR_TUPLE (0)
#define ACCESSOR_REGION (1)

#define ACCESSOR_TYPE(x) (*((int*)x))
#define ACCESSOR_IS_TUPLE(x) (ACCESSOR_TYPE(x) == ACCESSOR_TUPLE)
#define ACCESSOR_IS_REGION(x) (ACCESSOR_TYPE(x) == ACCESSOR_REGION)

#endif

