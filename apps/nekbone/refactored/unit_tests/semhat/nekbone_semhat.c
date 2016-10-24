#ifndef UNIT_TEST_NEKBONE_JACG_C
#define UNIT_TEST_NEKBONE_JACG_C

#include <stdio.h>
#include "blas1.h"
#include "blas3.h"
#include "polybasis.h"

#define Err_t int
#define IFEB if(err) break

int main(int argc, char * argv[])
{
    FILE * fout = 0;

    // Here I just define some linear buffers in order to avoid malloc&free.
#   define DOFMAX 100    //This is too big, but close enough.

    NBN_REAL buf_a[DOFMAX*DOFMAX], buf_c[DOFMAX*DOFMAX], buf_d[DOFMAX*DOFMAX];
    NBN_REAL buf_b[DOFMAX], buf_z[DOFMAX];
    NBN_REAL buf_w[2*DOFMAX];

    double buf_work_z[DOFMAX], buf_work_w[2*DOFMAX]; //*2 because this will be used both
                                                     // as a vector and a matrix.

    Err_t err=0;
    while(!err){
        fout = fopen("./z_ccode.out", "w");
        if(!fout) {err=__LINE__; IFEB;}

        // If any of these values changes, make also the change in nekbone_semhat.f90
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

        unsigned int pDOF;
        for(pDOF=firstdegree; pDOF<=lastdegree; ++pDOF){
            const unsigned int N = pDOF -1;

            a.sz_rows = pDOF; a.sz_cols = pDOF;
            c.sz_rows = pDOF; c.sz_cols = pDOF;
            d.sz_rows = pDOF; d.sz_cols = pDOF;

            err = nbb_semhat(a,b,c,d,z,w,pDOF-1, work_w, work_z); IFEB;

            fprintf(fout, "pDOF= %u\n", pDOF);

            unsigned k;
            for(k=0; k<pDOF; ++k){
                fprintf(fout,"%10u  %23.14E  %23.14E\n", k, b[k], z[k]);
            }
            for(k=0; k<=2*N; ++k){
                fprintf(fout,"W(k)=  %10u  %23.14E\n", k, work_w[k]);
            }

            unsigned i,j;
            for(j=0; j<pDOF; ++j){
                for(i=0; i<pDOF; ++i){
                    fprintf(fout,"%10u  %10u  %23.14E  %23.14E  %23.14E\n",
                            i,j, nbb_get2(a,i,j), nbb_get2(c,i,j), nbb_get2(d,i,j)
                            );
                }
            }
        }
        IFEB;

        if(fout){
            fclose(fout); fout=0;
        }

        break; //while(!err)
    }

    if(fout){
        fclose(fout); fout=0;
    }

    if(err){
        printf("ERROR: %lu\n", (unsigned long)err );
        return 1;
    }
    return 0;
}

#endif // UNIT_TEST_NEKBONE_JACG_C
