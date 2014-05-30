#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "HTA.h"
#include "HTA_operations.h"
#include "Tuple.h"
#include "Distribution.h"
#include "util.h"
#include "test.h"
#ifdef PILHTA
int hta_main(int argc, char** argv)
#else
int main()
#endif
{
    int i;
    double *M, *R;
    
    int dim_reduc = (argc == 1) ? 0 : atoi(argv[1]);
    
    Tuple flat_size = Tuple_create(2, 8, 8);

    Dist dist;
    // create an empty shell
    HTA* h = HTA_create(2, 3, &flat_size, 0, &dist, HTA_SCALAR_TYPE_DOUBLE, 
            2, Tuple_create(2, 2, 2), Tuple_create(2, 2, 2));

    srand(time(NULL));
    
    // Create a 2D matrix
    int matrix_size = Tuple_product(&flat_size);
    M = (double *) malloc(matrix_size * sizeof(double));
    for(i = 0; i < matrix_size; i++) {
        M[i] = i+1;
    }
    // Initialize the HTA using 2D matrix
    HTA_init_with_array(h, M);
    
    double initval = 0.0;
    // Partial reduction along dim_reduc
    // TODO: HTA_partial_reduction_dim(operation, h, dim_reduc)
    HTA* r = HTA_partial_reduce(REDUCE_MAX, h, dim_reduc, &initval);
    
    assert(r);
    // Print the result
    matrix_size = Tuple_product(&r->flat_size);
    R = (double *) malloc(matrix_size * sizeof(double));
    HTA_to_array(r, R);
    for(i = 0; i < matrix_size; i++) printf("(%d) %lf \n", i, R[i]);
    
    for(i = 0; i < matrix_size; i++)
    {
        double v = (!dim_reduc) ? (56 + i + 1) : (8 * (i+1));
        if( v != R[i] ) {
            printf("incorrect result detected!\n");
            exit(ERR_UNMATCH);
        }
    }
    HTA_destroy(h);
    HTA_destroy(r);

    if(Alloc_count_objects() > 0) {
        printf("Objects left (memory leak) %d\n", Alloc_count_objects());
        exit(ERR_MEMLEAK);
    }
    exit(SUCCESS);
    return 0;
}
