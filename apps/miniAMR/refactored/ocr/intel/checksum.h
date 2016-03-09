// TODO: FIXME:  insert copyright notice

#ifndef __CHECKSUM_H__
#define __CHECKSUM_H__

typedef struct Checksum_t Checksum_t;

#include "dbcommheader.h"
#include "control.h"

typedef struct Checksum_t {
   DbCommHeader_t  dbCommHeader;
   double      sum[0];
} Checksum_t;

#define sizeof_Checksum_t (sizeof(Checksum_t) + (control->num_vars * sizeof(double)))

#endif // __CHECKSUM_H__
