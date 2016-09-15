#ifndef UNIT_TEST_NEKBONE_BLAS3_C
#define UNIT_TEST_NEKBONE_BLAS3_C

#include <stdio.h>
#include <string.h> //memset
#include <stdlib.h> //malloc & free

#include "blas3.h"

#define Err_t int
#define IFEB if(err) break

Err_t check_transpose2_1()
{
    Err_t err=0;
    while(!err){
        nbb_matrix2_t x={0};
        nbb_matrix2_t y={0};
#       define ROWS 3
#       define COLS 2
#       define N  (ROWS*COLS)

        BLAS_REAL_TYPE bufx[N]={0};
        BLAS_REAL_TYPE bufy[N]={0};

        y.m = &bufy[0];
        y.sz_rows = ROWS;
        y.sz_cols = COLS;

        x.m = &bufx[0];
        x.sz_rows = 10*ROWS; //10* just to fudge the results, as x should be overwritten by y
        x.sz_cols = 10*COLS;

        BLAS_UINT_TYPE i,j;
        for(i=0; i<y.sz_rows; ++i){
            for(j=0; j<y.sz_cols; ++j){
                BLAS_REAL_TYPE q = 1 + i + y.sz_rows * j;
                *nbb_at2(y,i,j) = q;
            }
        }

        err = nbb_transpose2(&x,y); IFEB;

        for(i=0; i<y.sz_rows; ++i){
            for(j=0; j<y.sz_cols; ++j){
                if( nbb_get2(x,j,i) != nbb_get2(y,i,j) ){
                    err = __LINE__;
                    break;
                }
            }
            IFEB;
        }
        IFEB;

        for(i=0; i<x.sz_rows; ++i){
            for(j=0; j<x.sz_cols; ++j){
                if( *nbb_at2(x,i,j) != *nbb_at2(y,j,i) ){
                    err = __LINE__;
                    break;
                }
            }
            IFEB;
        }
        IFEB;

        break; //while(!err){
    }
    return err;
#   undef ROWS
#   undef COLS
#   undef N
}

Err_t check_mxm2()
{
    Err_t err=0;
    while(!err){
        nbb_matrix2_t x={0};
        nbb_matrix2_t y={0};
        nbb_matrix2_t z={0};  //As in Z=X*Y
        nbb_matrix2_t ref={0};
#       define ROWS 2
#       define COLS 5
#       define N  (ROWS*COLS)

        BLAS_REAL_TYPE bufx[N]={0};
        BLAS_REAL_TYPE bufy[N]={0};
        BLAS_REAL_TYPE bufz[ROWS*ROWS]={0};
        BLAS_REAL_TYPE bufref[ROWS*ROWS]={0};

        x.m = &bufx[0];
        x.sz_rows = ROWS;
        x.sz_cols = COLS;

        y.m = &bufy[0];
        y.sz_rows = COLS;
        y.sz_cols = ROWS;

        z.m = &bufz[0];
        z.sz_rows = ROWS;
        z.sz_cols = ROWS;

        ref.m = &bufref[0];
        ref.sz_rows = ROWS;
        ref.sz_cols = ROWS;

        BLAS_UINT_TYPE i,j;
        for(i=0; i<x.sz_rows; ++i){
            for(j=0; j<x.sz_cols; ++j){
                BLAS_REAL_TYPE q = 1 + i + x.sz_rows * j;
                *nbb_at2(x,i,j) = q;
            }
        }

        err = nbb_transpose2(&y,x); IFEB;

        {
            BLAS_UINT_TYPE i,j,k;
            for(i=0; i<x.sz_rows; ++i){
                for(j=0; j<y.sz_cols; ++j){
                    BLAS_REAL_TYPE sum = 0;
                    for(k=0; k<x.sz_cols; ++k){
                        sum += nbb_get2(x,i,k) * nbb_get2(y,k,j);
                    }
                    BLAS_REAL_TYPE * p = nbb_at2(ref,i,j);
                    *p = sum;
                }
            }
        }

        const int do_debug = 0;

        if(do_debug){
            for(i=0; i<x.sz_rows; ++i){
                for(j=0; j<x.sz_cols; ++j){
                    printf("i=%u\tj=%u\tx=%g\n",i,j,nbb_get2(x,i,j));
                }
            }
            for(i=0; i<y.sz_rows; ++i){
                for(j=0; j<y.sz_cols; ++j){
                    printf("i=%u\tj=%u\ty=%g\n",i,j,nbb_get2(y,i,j));
                }
            }
        }

        err = nbb_mxm2(x,y, &z); IFEB;

        if(do_debug){
            for(i=0; i<z.sz_rows; ++i){
                for(j=0; j<z.sz_cols; ++j){
                    printf("i=%u\tj=%u\tz=%g\n",i,j,nbb_get2(z,i,j));
                }
            }
        }

        for(i=0; i<x.sz_rows; ++i){
            for(j=0; j<y.sz_cols; ++j){
                if( nbb_get2(z,i,j) != nbb_get2(ref,i,j) ){
                    err = __LINE__;
                    break;
                }
            }
            IFEB;
        }
        IFEB;

        break; //while(!err){
    }
    return err;
#   undef ROWS
#   undef COLS
#   undef N
}

Err_t check_local_grad3()
{
    Err_t err=0;
    while(!err){
        nbb_matrix2_t x={0};
        nbb_matrix2_t y={0};
        nbb_matrix2_t z={0};  //As in Z=X*Y
        nbb_matrix2_t ref={0};
#       define ROWS 2
#       define COLS 5
#       define N  (ROWS*COLS)

        BLAS_REAL_TYPE bufx[N]={0};
        BLAS_REAL_TYPE bufy[N]={0};
        BLAS_REAL_TYPE bufz[ROWS*ROWS]={0};
        BLAS_REAL_TYPE bufref[ROWS*ROWS]={0};

        x.m = &bufx[0];
        x.sz_rows = ROWS;
        x.sz_cols = COLS;

        y.m = &bufy[0];
        y.sz_rows = COLS;
        y.sz_cols = ROWS;

        z.m = &bufz[0];
        z.sz_rows = ROWS;
        z.sz_cols = ROWS;

        ref.m = &bufref[0];
        ref.sz_rows = ROWS;
        ref.sz_cols = ROWS;

        BLAS_UINT_TYPE i,j;
        for(i=0; i<x.sz_rows; ++i){
            for(j=0; j<x.sz_cols; ++j){
                BLAS_REAL_TYPE q = 1 + i + x.sz_rows * j;
                *nbb_at2(x,i,j) = q;
            }
        }

        err = nbb_transpose2(&y,x); IFEB;

        {
            BLAS_UINT_TYPE i,j,k;
            for(i=0; i<x.sz_rows; ++i){
                for(j=0; j<y.sz_cols; ++j){
                    BLAS_REAL_TYPE sum = 0;
                    for(k=0; k<x.sz_cols; ++k){
                        sum += nbb_get2(x,i,k) * nbb_get2(y,k,j);
                    }
                    BLAS_REAL_TYPE * p = nbb_at2(ref,i,j);
                    *p = sum;
                }
            }
        }

        const int do_debug = 0;

        if(do_debug){
            for(i=0; i<x.sz_rows; ++i){
                for(j=0; j<x.sz_cols; ++j){
                    printf("i=%u\tj=%u\tx=%g\n",i,j,nbb_get2(x,i,j));
                }
            }
            for(i=0; i<y.sz_rows; ++i){
                for(j=0; j<y.sz_cols; ++j){
                    printf("i=%u\tj=%u\ty=%g\n",i,j,nbb_get2(y,i,j));
                }
            }
        }

        err = nbb_mxm2(x,y, &z); IFEB;

        if(do_debug){
            for(i=0; i<z.sz_rows; ++i){
                for(j=0; j<z.sz_cols; ++j){
                    printf("i=%u\tj=%u\tz=%g\n",i,j,nbb_get2(z,i,j));
                }
            }
        }

        for(i=0; i<x.sz_rows; ++i){
            for(j=0; j<y.sz_cols; ++j){
                if( nbb_get2(z,i,j) != nbb_get2(ref,i,j) ){
                    err = __LINE__;
                    break;
                }
            }
            IFEB;
        }
        IFEB;

        break; //while(!err){
    }
    return err;
#   undef ROWS
#   undef COLS
#   undef N
}

int main(int argc, char * argv[])
{
    Err_t err=0;
    while(!err){
        err = check_transpose2_1(); IFEB;
        err = check_mxm2(); IFEB;
        err = check_local_grad3(); IFEB;

        break; //while(!err){
    }

    if(err){
        printf("ERROR: %lu\n", (unsigned long)err );
        return 1;
    }
    return 0;
}

#endif // UNIT_TEST_NEKBONE_BLAS3_C
