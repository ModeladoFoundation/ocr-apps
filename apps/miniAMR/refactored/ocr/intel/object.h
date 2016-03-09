// TODO: FIXME: insert copyright stuff

#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "dbcommheader.h"

typedef struct {
   int type;
   int bounce;
   double cen[3];
   double orig_cen[3];
   double move[3];
   double orig_move[3];
   double size[3];
   double orig_size[3];
   double inc[3];
} Object_t;

typedef struct {
   DbCommHeader_t dbCommHeader;
   Object_t object[0];
} AllObjects_t;

#define sizeof_AllObjects_t (sizeof(AllObjects_t)+control->num_objects*sizeof(Object_t))


#endif // __OBJECT_H__
