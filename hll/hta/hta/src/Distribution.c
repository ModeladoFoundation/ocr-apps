#include <stdio.h>
#include <stdlib.h>
#include "Distribution.h"
#include "Debug.h"

void Dist_init(Dist *d, DIST_TYPE type, Tuple* mesh) {
    d->type = type;
    d->mesh = *mesh;
    d->num_processes = Tuple_product(mesh);
    d->dimreduc = -1;
    d->projected = 0;
}

void Dist_init_projected(Dist *d, Dist *orig, int dimreduc) {
    *d = *orig;
    ASSERT(dimreduc >= 0 && dimreduc < orig->mesh.dim);
    d->dimreduc = dimreduc;
    d->projected = 1;
}

// binary search
#if 0
int _search_pid(int left, int right, int num_tiles, int tile_idx, int mesh_size) {
    int i = (left + right) / 2;
    int start = (i * num_tiles) / mesh_size;
    int end = ((i+1) * num_tiles) / mesh_size;

    if(start > tile_idx)
        return _search_pid(left, i-1, num_tiles, tile_idx, mesh_size);
    else if (end < tile_idx)
        return _search_pid(i+1, right, num_tiles, tile_idx, mesh_size);
    else // found
        return i;
}
#endif

// for given index to a tile, evaluate to the process ID
int Dist_get_pid(const Dist *dist, const Tuple* nd_idx, const Tuple* tiling) {
    int rank;
    int dim = dist->mesh.dim;
    ASSERT(nd_idx->dim == dim);
    Tuple p_idx; // for process mesh index
    p_idx.dim = dim;

    switch(dist->type) {
        case (DIST_BLOCK):
            for(int i = 0; i < dim; i++) {
                int num_tiles = tiling->values[i];
                int tile_idx = nd_idx->values[i];
                int mesh_size = dist->mesh.values[i];
                if(mesh_size <= num_tiles)
                    p_idx.values[i] = ((mesh_size * (tile_idx +1))-1) / num_tiles; // BLOCK_OWNER macro Quinn's book p 120
                else
                    p_idx.values[i] = tile_idx;
            }
            //rank = Tuple_nd_to_1d_index(&p_idx, &dist->mesh);
            break;
        case (DIST_CYCLIC):
	    //Tuple_print(nd_idx);
	    //Tuple_print(tiling);
	    //Tuple_print(&dist->mesh);
	    //rank = Tuple_nd_to_1d_index(nd_idx, tiling) % 4;
            for(int i = 0; i < dim; i++) {
                int tile_idx = nd_idx->values[i];
                int mesh_size = dist->mesh.values[i];
                p_idx.values[i] = tile_idx % mesh_size;
            }
            break;
        case (DIST_ROW_CYCLIC):
            {
                ASSERT(tiling->dim == 2);
                return nd_idx->values[0] % dist->num_processes;
            }
        case (DIST_COLUMN_CYCLIC):
            {
                ASSERT(tiling->dim == 2);
                return nd_idx->values[1] % dist->num_processes;
            }
        case (DIST_ROW_REFLECTIVE):
            {
                ASSERT(tiling->dim == 2);
                int i = nd_idx->values[0];
                int np = dist->num_processes;
                int isEvenRound = (((i / np) & 0x1) == 0);
                return (isEvenRound) ? (i % np) : (np - 1 - (i % np));
            }
        case (DIST_COL_REFLECTIVE):
            {
                ASSERT(tiling->dim == 2);
                int i = nd_idx->values[1];
                int np = dist->num_processes;
                int isEvenRound = (((i / np) & 0x1) == 0);
                return (isEvenRound) ? (i % np) : (np - 1 - (i % np));
            }
	case (DIST_WAVEFRONT):
	    {
		ASSERT(tiling->dim == 2);
		ASSERT(tiling->values[0] == tiling->values[1]);
		int i = nd_idx->values[0];
		int j = nd_idx->values[1];
		int n = tiling->values[0];
		int np = dist->num_processes;
		int temp, owner;

		if (i < j) { // tile in upper triangle, swap, dist matrix is symmetric
		  temp = i;
		  i = j;
		  j = temp;
		}

		int strip = i + j;

		if (strip < n - 1) {
		  if (strip == 0) {
		    owner = 0;
		  }
		  else {
		    int prev_strip = strip - 1;
		    int num_elems_prev_strip = (prev_strip/2) + 1;
		    if ((prev_strip & 0x01) == 1) temp = 0;
		    else temp = num_elems_prev_strip;
		    int prev_elems = (1+num_elems_prev_strip) * num_elems_prev_strip - temp;
		    int offset = prev_elems + j;
		    owner = offset % np;
		  }
		}
		else if(strip == n - 1) { // Diagonal
		  i = n - i - 1;
		  int group = i / np;
		  if ((group & 0x01) == 0) owner = (np -1) - (i % np);
		  else owner = i % np;
		}
		else {
		  strip = (n - 1) * 2 - strip;
		  if (strip == 0) {
		    owner = 0;
		  }
		  else {
		    int prev_strip = strip - 1;
		    int num_elems_prev_strip = (prev_strip/2) + 1;
		    if ((prev_strip & 0x01) == 1) temp = 0;
		    else temp = num_elems_prev_strip;
		    int prev_elems = (1+num_elems_prev_strip) * num_elems_prev_strip - temp;
		    int offset = prev_elems + (i - (n - strip/2) + 1);
		    owner = offset % np;
		  }
		}
            //printf("(%d,%d)  --> %d\n", nd_idx->values[0], nd_idx->values[1], owner);
	    return owner;
	    }
        default:
            ASSERT(0 && "Unsupported distribution type");
            return -1;
    }
//    return rank;
    return Tuple_nd_to_1d_index(&p_idx, &dist->mesh);
}

void Dist_print(const Dist *d) {
    printf("Dist: unknown\n");
}

