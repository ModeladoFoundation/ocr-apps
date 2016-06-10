/*
Author: Brian R Nickerson
Copyright Intel Corporation 2016

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.
*/

#ifndef __REFINEMENTDISPOSITION_H__
#define __REFINEMENTDISPOSITION_H__

typedef enum { UNREFINE_BLK=-1, LEAVE_BLK_AT_LVL=0, REFINE_BLK=1 } RefinementDecision_t;

// This datablock is used to exchange refinement disposition between neighbors.

typedef struct RefinementDisposition_t RefinementDisposition_t;

#include "commhdr.h"

typedef struct RefinementDisposition_t {
   DbCommHeader_t       dbCommHeader;
   RefinementDecision_t refinementDisposition;

} RefinementDisposition_t;

#define sizeof_RefinementDisposition_t (sizeof(RefinementDisposition_t))

#endif // __REFINEMENTDISPOSITION_H__
