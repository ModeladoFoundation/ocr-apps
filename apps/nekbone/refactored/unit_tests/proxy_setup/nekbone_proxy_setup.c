#ifndef UNIT_TEST_NEKBONE_JACG_C
#define UNIT_TEST_NEKBONE_JACG_C

#include <stdio.h>
#include <stdlib.h> //malloc & free

#include "blas1.h"
#include "blas3.h"
#include "polybasis.h"

#define Err_t int
#define IFEB if(err) break

void printout_matrices(unsigned int in_pDOF,
                       NBN_REAL * in_g1, NBN_REAL * in_g4, NBN_REAL * in_g6,
                       nbb_matrix2_t in_dxm1, nbb_matrix2_t in_dxtm1);

int main(int argc, char * argv[])
{
    // Here I just define some linear buffers in order to avoid malloc&free.
#   define DOFMAX 100    //This is too big, but close enough.

    NBN_REAL buf_a[DOFMAX*DOFMAX], buf_c[DOFMAX*DOFMAX], buf_d[DOFMAX*DOFMAX];
    NBN_REAL buf_b[DOFMAX], buf_z[DOFMAX];
    NBN_REAL buf_w[2*DOFMAX];

    double buf_work_z[DOFMAX], buf_work_w[2*DOFMAX]; //*2 because this will be used both
                                                     // as a vector and a matrix.

    NBN_REAL * buf_g1 = NULL;
    NBN_REAL * buf_g4 = NULL;
    NBN_REAL * buf_g6 = NULL;

    BLAS_REAL_TYPE buf_D[DOFMAX*DOFMAX], buf_Dt[DOFMAX*DOFMAX];

    Err_t err=0;
    while(!err){
        // If any of these values changes, make also the change in nekbone_proxy_setup.f90
        //2016Oct20: After experimentations, these ranges [firstdegree, lastdegree] had
        // the following corroletion with the C code:
        //     [ 2,  5] -> tol = 1e-14
        //     [ 5, 15] -> tol = 1e-13
        //     [15, 25] -> tol = 5e-13
        const unsigned int firstdegree = 2;
        const unsigned int lastdegree  = 25;

        const unsigned int N = 84;  //Largest value allowed in Nekbone's speclib.f::ZWGJ
        if(lastdegree >= N){
            printf("ERROR: test_jacg: lastdegree must be smaller than 100.\n");
            err=__LINE__; IFEB;
        }

        //Setup the variables
        buf_g1 = (NBN_REAL*) malloc(sizeof(NBN_REAL) * DOFMAX * DOFMAX * DOFMAX); if(!buf_g1){err=__LINE__;IFEB;}
        buf_g4 = (NBN_REAL*) malloc(sizeof(NBN_REAL) * DOFMAX * DOFMAX * DOFMAX); if(!buf_g4){err=__LINE__;IFEB;}
        buf_g6 = (NBN_REAL*) malloc(sizeof(NBN_REAL) * DOFMAX * DOFMAX * DOFMAX); if(!buf_g6){err=__LINE__;IFEB;}

        NBN_REAL *b, *w, *z;
        b = buf_b;
        w = buf_w;
        z = buf_z;

        double *work_z, *work_w;
        work_w = buf_work_w;
        work_z = buf_work_z;

        nbb_matrix2_t a,c,d;
        a.m = buf_a;
        c.m = buf_c;
        d.m = buf_d;

        NBN_REAL *g1, *g4, *g6;
        g1 = buf_g1;
        g4 = buf_g4;
        g6 = buf_g6;

        nbb_matrix2_t dxm1, dxtm1;
        dxm1.m = buf_D;
        dxtm1.m = buf_Dt;

        //Do the calculations
        unsigned int pDOF;
        for(pDOF=firstdegree; pDOF<=lastdegree; ++pDOF){

            a.sz_rows = pDOF; a.sz_cols = pDOF;
            c.sz_rows = pDOF; c.sz_cols = pDOF;
            d.sz_rows = pDOF; d.sz_cols = pDOF;

            dxm1.sz_rows = pDOF;  dxm1.sz_cols = pDOF;
            dxtm1.sz_rows = pDOF; dxtm1.sz_cols = pDOF;

            err = nbb_proxy_setup(a,b,c,d,z,w, pDOF,work_w,work_z, g1,g4,g6, dxm1,dxtm1); IFEB;

            printout_matrices(pDOF, g1,g4,g6, dxm1, dxtm1);
        }
        IFEB;

        break; //while(!err)
    }

    if(buf_g1) { free(buf_g1); buf_g1=NULL;}
    if(buf_g4) { free(buf_g4); buf_g4=NULL;}
    if(buf_g6) { free(buf_g6); buf_g6=NULL;}

    if(err){
        printf("ERROR: %lu\n", (unsigned long)err );
        return 1;
    }
    return 0;
}

void printout_matrices(unsigned int in_pDOF,
                       NBN_REAL * in_g1, NBN_REAL * in_g4, NBN_REAL * in_g6,
                       nbb_matrix2_t in_dxm1, nbb_matrix2_t in_dxtm1)
{

    unsigned i,j,k;

    fprintf(stdout, "DXM1_&_DXtM1  pDOF= %u\n", in_pDOF);
    for(j=0; j<in_pDOF; ++j){
        for(i=0; i<in_pDOF; ++i){
            double dxm1  = nbb_get2(in_dxm1,i,j);
            double dx1m1 = nbb_get2(in_dxtm1,i,j);
            fprintf(stdout, "%10u  %10u  %23.14E  %23.14E\n", i,j, dxm1, dx1m1 );
        }
    }

    const unsigned int nx1 = in_pDOF;
    const unsigned int ny1 = in_pDOF;
    const unsigned int nz1 = in_pDOF;

    fprintf(stdout, "Gauss Weights  pDOF= %u\n", in_pDOF);
    for(k=0; k<nz1; ++k){
        for(j=0; j<ny1; ++j){
            for(i=0; i<nx1; ++i){
                unsigned long h = hash_columnMajor3(0, nx1,ny1,nz1, i,j,k);
                NBN_REAL g1 = in_g1[h];
                NBN_REAL g4 = in_g4[h];
                NBN_REAL g6 = in_g6[h];
                fprintf(stdout, "%10u  %10u  %10u  %23.14E  %23.14E  %23.14E  %23.14E  %23.14E  %23.14E\n",
                        i+1,j+1,k+1, g1, 0.0, 0.0, g4, 0.0, g6);
            }
        }
    }
    fprintf(stdout, "-----\n");
}

#endif // UNIT_TEST_NEKBONE_JACG_C
