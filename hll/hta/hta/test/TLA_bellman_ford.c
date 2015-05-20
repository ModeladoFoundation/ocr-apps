#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "HTA.h"

#define MIN(a, b) ((a<=b)?a:b)
#define INF (INFINITY)
int N = 8;   // N is number of vertices
int P = 4;   // P for the amount of parallelism

// Saeed's example
double adjacency_matrix[64] = { INF, INF, INF, 3.0, INF, INF, INF, INF,
                                1.0, INF, INF, INF, INF, INF, INF, INF,
                                INF, 6.0, INF, INF, INF, INF, INF, INF,
                                INF, INF, 7.0, INF, 3.0, INF, INF, INF,
                                INF, 8.0, INF, INF, INF, 2.0, INF, INF,
                                INF, INF, INF, INF, INF, INF, 1.0, INF,
                                INF, INF, INF, INF, INF, INF, INF, 2.0,
                                INF, 5.0, INF, INF, INF, 4.0, INF, INF
};
double init_d[8] = { INF, INF, 0.0, INF, INF, INF, INF, INF };
double inf = INF;
double result_d[8];

void tropical(HTA* d_tmp, HTA *M, HTA *d) {
    double *M_ptr = (double*) HTA_get_ptr_raw_data(M);
    double *d_ptr = (double*) HTA_get_ptr_raw_data(d);
    double *d_tmp_ptr = (double*) HTA_get_ptr_raw_data(d_tmp);

    int N = M->flat_size.values[0];
    int NP = M->flat_size.values[1];

    for(int i = 0; i < N; i++) {
        double min = d_tmp_ptr[i];
        for(int j = 0; j < NP; j++) {
           min = MIN(min, d_ptr[j] + M_ptr[j]);
        }
        d_tmp_ptr[i] = min;
        M_ptr += NP;
    }
}

void reduce_scatter(HTA* d, HTA *d_tmp) {
    int NP = d->flat_size.values[1];
    double *d_ptr = (double*) HTA_get_ptr_raw_data(d);
    int offset = d->nd_element_offset.values[1];

    for(int i = 0; i < P; i++) {
        double *d_tmp_ptr = (double*) HTA_get_ptr_raw_data(d_tmp->tiles[i]);
        for(int j = 0; j < NP; j++) {
           d_ptr[j] = MIN(d_ptr[j], d_tmp_ptr[offset + j]);
        }
    }
}

int hta_main(int argc, char** argv, int pid)
{
    if(argc > 2) {
        P = atoi(argv[1]);
    }
    if(argc > 3) {
        N = atoi(argv[2]);
    }

    // Construct adjacency matrix, distance vector and the vector for temporary
    HTA *M, *d, *d_tmp;
    Tuple mesh;
    Tuple_init(&mesh, 1, 8);
    mesh.height = 1;

    Dist dist;
    Dist_init(&dist, DIST_BLOCK, &mesh);

    Tuple t_M = Tuple_create(2, 1, P); // partitioned evenly by columns
    Tuple flat_size_M = Tuple_create(2, N, N);
    Tuple t_d = Tuple_create(2, 1, P); // d^T partitioned evenly by columns
    Tuple flat_size_d = Tuple_create(2, 1, N);
    Tuple t_d_tmp = Tuple_create(2, 1, P); // d reflected on all processes
    Tuple flat_size_d_tmp = Tuple_create(2, N, P);

    // FIXME: These should be sparse
    M = HTA_create(2, 2, &flat_size_M, 0, &dist, HTA_SCALAR_TYPE_DOUBLE, 1, t_M);
    d = HTA_create(2, 2, &flat_size_d, 0, &dist, HTA_SCALAR_TYPE_DOUBLE, 1, t_d);
    d_tmp = HTA_create(2, 2, &flat_size_d_tmp, 0, &dist, HTA_SCALAR_TYPE_DOUBLE, 1, t_d_tmp);

    // TODO: Initialization
    // Transpose the adjacency matrix
    for(int i = 0; i < N; i++)
        for (int j = i+1; j < N; j++) {
                double tmp = adjacency_matrix[i*N + j];
                adjacency_matrix[i*N + j] = adjacency_matrix[j*N + i];
                adjacency_matrix[j*N + i] = tmp;
        }
    HTA_init_with_array(M, adjacency_matrix);
    HTA_init_with_array(d, init_d);
    HTA_init_all_scalars(d_tmp, &inf);

    for(int i = 0; i < N-1; i++) {
        // d <- d + M*d
        HTA_map_h3(HTA_LEAF_LEVEL(d_tmp), tropical, d_tmp, M, d); // result is kept in d_tmp instead of d

        // d += d
        HTA_tile_to_hta2(HTA_LEAF_LEVEL(d), reduce_scatter, d, d_tmp);
        HTA_to_array(d, result_d);

        printf("Final result of d: ");
        for(int j = 0; j < N; j++) {
            printf("%.1lf, ", result_d[j]);
        }
        printf("\n");

    }

    HTA_destroy(d);
    HTA_destroy(d_tmp);
    HTA_destroy(M);

    return 0;
}
