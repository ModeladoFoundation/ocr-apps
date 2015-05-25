#ifndef __DISTRIBUTION_H__
#define __DISTRIBUTION_H__
#include "Tuple.h"

typedef enum {
    DIST_BLOCK = 0,
    DIST_CYCLIC = 1,
    DIST_ROW_CYCLIC = 2,
    DIST_COLUMN_CYCLIC = 3,
    DIST_ROW_REFLECTIVE = 4,
    DIST_COL_REFLECTIVE = 5,
    DIST_WAVEFRONT = 6,
    DIST_TYPE_MAX
} DIST_TYPE;

struct distribution {
    DIST_TYPE type;
    Tuple mesh;
    int num_processes;
    // for partial reduction
    int dimreduc;
    int projected;
};

typedef struct distribution Dist;

void Dist_init(Dist *d, DIST_TYPE type, Tuple* mesh);
int Dist_get_pid(const Dist *dist, const Tuple* nd_idx, const Tuple* tiling);
void Dist_init_projected(Dist *d, Dist *orig, int dimreduc);
void Dist_print(const Dist *d);

#endif
