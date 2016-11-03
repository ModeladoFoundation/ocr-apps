#ifndef UNIT_TEST_NEKBONE_LOCAL_GRAD_C
#define UNIT_TEST_NEKBONE_LOCAL_GRAD_C

#include <stdio.h>
#include <stdlib.h> //malloc,free
#include <string.h> //memset

#include "local_grad.h"
#include "ax.h"
#include "nbn_setup.h"

#define Err_t int
#define IFEB if(err) break
#define SAFEFREE(v) if(v){ free(v); v=0; }
#define SAFEMALLOC(P,COUNT,TYPE)  P = (TYPE *) malloc( COUNT * sizeof(TYPE)); if(!P) {err=__LINE__; IFEB;}

#define MaxN 100 //100 is a very big number, which should never be reached

void prettyPrint_matrix2D(nbb_matrix2_t in_m);  //For debugging
void prettyPrint_matrix3D(nbb_matrix3_t in_m);  //For debugging

Err_t build_variables(unsigned int in_Ecount, unsigned int in_pDOF,
                      NBN_REAL ** o_W, NBN_REAL ** o_U,
                      nbb_matrix2_t *o_D, nbb_matrix2_t *o_Dt,
                      NBN_REAL ** o_g1, NBN_REAL ** o_g4, NBN_REAL ** o_g6,
                      NBN_REAL ** o_wUR, NBN_REAL ** o_wUS, NBN_REAL ** o_wUT,
                      NBN_REAL ** o_temp
                      );

void free_variables( NBN_REAL ** io_W, NBN_REAL ** io_P,
                     nbb_matrix2_t *io_D, nbb_matrix2_t *io_Dt,
                     NBN_REAL ** io_g1, NBN_REAL ** io_g4, NBN_REAL ** io_g6,
                     NBN_REAL ** io_wUR, NBN_REAL ** io_wUS, NBN_REAL ** io_wUT,
                     NBN_REAL ** io_temp);

int main(int argc, char * argv[])
{
    FILE * fout = 0;

    NBN_REAL *W=0, *P=0, *g1=0, *g4=0, *g6=0;
    nbb_matrix2_t D = {0};
    nbb_matrix2_t Dt = {0};

    NBN_REAL *wUR=0, *wUS=0, *wUT=0, *temp;

    Err_t err=0;
    while(!err){
        fout = fopen("./z_ccode.out", "w"); if(!fout) {err=__LINE__; IFEB;}

        unsigned int first_pDOF = 8;  //If (first|last)_pDOF changes, make the same change in nekbone_axi.f90
        unsigned int last_pDOF = 12;
        unsigned int dof, dof3DperR;

        unsigned int first_Ecount = 1;
        unsigned int last_Ecount = 10;
        unsigned int e;

        for(e=first_Ecount; e <= last_Ecount; ++e){
        for(dof=first_pDOF; dof<=last_pDOF; ++dof){
            const unsigned int dof3DperR = dof * dof * dof * e;

            fprintf(fout,"pDOF= %10u Ecount= %10u\n", dof,e);

            err = build_variables(e, dof, &W, &P, &D, &Dt, &g1, &g4, &g6, &wUR, &wUS, &wUT, &temp); IFEB;

            err = nbb_axi_before(e, dof, W, P, D, Dt, g1,g4,g6, wUR,wUS,wUT, temp); IFEB;

            unsigned int i;
            for(i=0; i<dof3DperR; ++i){
                fprintf(fout,"%10u  %23.14E  %23.14E\n", i+1, W[i], P[i]);
            }

            free_variables( &W, &P, &D, &Dt, &g1, &g4, &g6, &wUR, &wUS, &wUT, &temp);
        } IFEB;
        } IFEB;

        fclose(fout); fout=0;
        break; //while(!err){
    }

    if(fout){
        fclose(fout); fout=0;
    }

    free_variables( &W, &P, &D, &Dt, &g1, &g4, &g6, &wUR, &wUS, &wUT, &temp);

    if(err){
        printf("ERROR: %lu\n", (unsigned long)err );
        return 1;
    }
    return 0;
}

Err_t build_variables(unsigned int in_Ecount, unsigned int in_pDOF,
                      NBN_REAL ** o_W, NBN_REAL ** o_P,
                      nbb_matrix2_t *o_D, nbb_matrix2_t *o_Dt,
                      NBN_REAL ** o_g1, NBN_REAL ** o_g4, NBN_REAL ** o_g6,
                      NBN_REAL ** o_wUR, NBN_REAL ** o_wUS, NBN_REAL ** o_wUT,
                      NBN_REAL ** o_temp
                      )
{
    Err_t err=0;
    while(!err){
        const unsigned int pDOFmax   = MaxN;
        const unsigned int pDOF2Dmax = MaxN * MaxN;
        const unsigned int pDOF3Dmax = MaxN * MaxN * MaxN;
        const unsigned int pDOF3DmaxperR = MaxN * MaxN * MaxN * in_Ecount;

        if( sizeof(NBN_REAL) != sizeof(BLAS_REAL_TYPE)){
            err = __LINE__; IFEB;
        }

        SAFEMALLOC(*o_W, pDOF3DmaxperR, NBN_REAL);
        SAFEMALLOC(*o_P, pDOF3DmaxperR, NBN_REAL);
        //TODO: Consider removing memoization from the Gaussian Weight G
        SAFEMALLOC(*o_g1, pDOF3Dmax, NBN_REAL);
        SAFEMALLOC(*o_g4, pDOF3Dmax, NBN_REAL);
        SAFEMALLOC(*o_g6, pDOF3Dmax, NBN_REAL);
        SAFEMALLOC(*o_wUR, pDOF3Dmax, NBN_REAL);
        SAFEMALLOC(*o_wUS, pDOF3Dmax, NBN_REAL);
        SAFEMALLOC(*o_wUT, pDOF3Dmax, NBN_REAL);
        SAFEMALLOC(*o_temp, pDOF3Dmax, NBN_REAL);

        o_D->sz_rows = in_pDOF;
        o_D->sz_cols = in_pDOF;
        SAFEMALLOC(o_D->m, pDOF2Dmax, BLAS_REAL_TYPE);

        o_Dt->sz_rows = in_pDOF;
        o_Dt->sz_cols = in_pDOF;
        SAFEMALLOC(o_Dt->m, pDOF2Dmax, BLAS_REAL_TYPE);

        unsigned int i,j;
        for(i=0; i<in_pDOF; ++i){
            for(j=0; j<in_pDOF; ++j){
                double a = 1 + i;
                a /= in_pDOF;
                a += j;
                *nbb_at2(*o_D,i,j) = a;
                *nbb_at2(*o_Dt,j,i) = nbb_get2(*o_D,i,j);
            }
        }

        const unsigned int pDOF3D = in_pDOF * in_pDOF * in_pDOF;
        for(j=0; j<pDOF3D; ++j){
            double y = j+1;
            y /= pDOF3D;
            (*o_g1)[j] = y + (1-1);
            (*o_g4)[j] = y + (4-1);
            (*o_g6)[j] = y + (6-1);
        }

        const unsigned int pDOF3DperR = pDOF3D * in_Ecount;
        NBN_REAL x;
        for(i=0; i<pDOF3DperR; ++i){
            x = (i+1);
            x = x / pDOF3DperR;
            (*o_W)[i] = x;
            x = (pDOF3DperR - (i+1) +1);
            x = x / pDOF3DperR;
            (*o_P)[i] = x;
        }

        break; //while(!err)
    }
    return err;
}

void free_variables( NBN_REAL ** io_W, NBN_REAL ** io_U,
                    nbb_matrix2_t * io_D, nbb_matrix2_t * io_Dt,
                    NBN_REAL ** io_g1, NBN_REAL ** io_g4, NBN_REAL ** io_g6,
                    NBN_REAL ** io_wUR, NBN_REAL ** io_wUS, NBN_REAL ** io_wUT,
                    NBN_REAL ** io_temp)
{
    SAFEFREE(*io_W);
    SAFEFREE(*io_U);
    SAFEFREE(*io_g1);
    SAFEFREE(*io_g4);
    SAFEFREE(*io_g6);
    SAFEFREE(io_D->m); io_D->sz_rows = 0; io_D->sz_cols = 0;
    SAFEFREE(io_Dt->m); io_Dt->sz_rows = 0; io_Dt->sz_cols = 0;
    SAFEFREE(*io_wUR);
    SAFEFREE(*io_wUS);
    SAFEFREE(*io_wUT);
    SAFEFREE(*io_temp);
}

#endif // UNIT_TEST_NEKBONE_LOCAL_GRAD_C
