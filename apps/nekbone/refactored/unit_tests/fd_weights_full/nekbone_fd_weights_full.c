#ifndef UNIT_TEST_NEKBONE_GAMMAF_C
#define UNIT_TEST_NEKBONE_GAMMAF_C

#include <stdio.h>
#include <stdlib.h> //malloc, free
#include "polybasis.h"

#define Err_t int
#define IFEB if(err) break

int main(int argc, char * argv[])
{
    FILE * fout = 0;

    nbb_matrix2_t w;
    w.m = NULL;

    Err_t err=0;
    while(!err){
        fout = fopen("./z_ccode.out", "w");
        if(!fout) {err=__LINE__; IFEB;}

        //Porder and Ntry have to be the same as in their Fortran counterpart.

        const int Porder = 10; //Porder = (Nb of DOF for given polynomial) - 1
        const int pDOF = Porder +1;

        const int Ntry = 5;

        double z[pDOF];

        int m;
        int i,j,k;
        double po;

        w.sz_rows = pDOF;
        w.sz_cols = pDOF;
        w.m = (BLAS_REAL_TYPE*) malloc(sizeof(BLAS_REAL_TYPE)*(w.sz_cols*w.sz_rows));
        if( ! w.m){err=__LINE__; IFEB;}

        m = 1; // That is all that the Nekbone Fortran code uses.
        po = Porder;

        for(k=1; k<=Ntry; ++k){ // 1 to 5 chosen quasi-randomly, just to stress the function
            for(i=0; i <= Porder; ++i){
                z[i] = (i+k)/(po);
                for(j=0; j <= Porder; ++j){
                    *nbb_at2(w,i,j) = 0;
                }
            }
            for(i=0; i <= Porder; ++i){
                nbb_fd_weights_full(z[i],z,Porder,m,&w);
                fprintf(fout, "%10d%10d%23.14E\n", k, i,z[i]);
                for(j=0; j <= Porder; ++j){
                    BLAS_REAL_TYPE ww = nbb_get2(w,i,j);
                    fprintf(fout, "%10d%10d%10d%23.14E\n", k, i, j, ww);
                }
            }

        }

        if(fout){
            fclose(fout); fout=0;
        }

        break; //while(!err){
    }

    if(fout){
        fclose(fout); fout=0;
    }

    if(!w.m){
        free(w.m);
        w.m = NULL;
    }

    if(err){
        printf("ERROR: %lu\n", (unsigned long)err );
        return 1;
    }
    return 0;
}

#endif // UNIT_TEST_NEKBONE_GAMMAF_C
