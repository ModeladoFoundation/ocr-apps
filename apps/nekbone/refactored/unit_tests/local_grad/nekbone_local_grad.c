#ifndef UNIT_TEST_NEKBONE_LOCAL_GRAD_C
#define UNIT_TEST_NEKBONE_LOCAL_GRAD_C

#include <stdio.h>
#include <stdlib.h> //malloc,free
#include <string.h> //memset

#include "blas.h"
#include "blas3.h"

#define Err_t int
#define IFEB if(err) break
#define SAFEFREE(v) if(v){ free(v); v=0; }
#define SAFEMALLOC(P,COUNT,TYPE)  P = (TYPE *) malloc( COUNT * sizeof(TYPE)); if(!P) {err=__LINE__; IFEB;}

void prettyPrint_matrix2D(nbb_matrix2_t in_m);  //For debugging
void prettyPrint_matrix3D(nbb_matrix3_t in_m);  //For debugging

Err_t check_local_grad3(unsigned int in_pDOF, FILE * fout);
Err_t check_local_grad3_t(unsigned int in_pDOF, FILE * fout);

int main(int argc, char * argv[])
{
    FILE * fout = 0;

    Err_t err=0;
    while(!err){

        fout = fopen("./z_ccode.out", "w");
        if(!fout) {err=__LINE__; IFEB;}

        unsigned int first_pDOF = 2;
        unsigned int last_pDOF = 10;

        unsigned int pDOF;
        for(pDOF=first_pDOF; pDOF<= last_pDOF; ++pDOF){
            err = check_local_grad3(pDOF,fout); IFEB;
            err = check_local_grad3t(pDOF,fout); IFEB;
        }
        IFEB;

        fclose(fout); fout=0;
        break; //while(!err){
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

Err_t build_data(unsigned int in_pDOF, nbb_matrix3_t * ur, nbb_matrix3_t *us, nbb_matrix3_t *ut,
                 nbb_matrix3_t *u, nbb_matrix3_t *temp, nbb_matrix2_t *D, nbb_matrix2_t *Dt)
{
    Err_t err=0;
    while(!err){
        const unsigned int pDOF2D = in_pDOF * in_pDOF;
        const unsigned int pDOF3D = in_pDOF * in_pDOF * in_pDOF;

        ur->sz_rows = in_pDOF;
        ur->sz_cols = in_pDOF;
        ur->sz_depth= in_pDOF;
        SAFEMALLOC(ur->m, pDOF3D, BLAS_REAL_TYPE);

        us->sz_rows = in_pDOF;
        us->sz_cols = in_pDOF;
        us->sz_depth= in_pDOF;
        SAFEMALLOC(us->m, pDOF3D, BLAS_REAL_TYPE);

        ut->sz_rows = in_pDOF;
        ut->sz_cols = in_pDOF;
        ut->sz_depth= in_pDOF;
        SAFEMALLOC(ut->m, pDOF3D, BLAS_REAL_TYPE);

        u->sz_rows = in_pDOF;
        u->sz_cols = in_pDOF;
        u->sz_depth= in_pDOF;
        SAFEMALLOC(u->m, pDOF3D, BLAS_REAL_TYPE);

        temp->sz_rows = in_pDOF;
        temp->sz_cols = in_pDOF;
        temp->sz_depth= in_pDOF;
        SAFEMALLOC(temp->m, pDOF3D, BLAS_REAL_TYPE);

        D->sz_rows = in_pDOF;
        D->sz_cols = in_pDOF;
        SAFEMALLOC(D->m, pDOF2D, BLAS_REAL_TYPE);

        Dt->sz_rows = in_pDOF;
        Dt->sz_cols = in_pDOF;
        SAFEMALLOC(Dt->m, pDOF2D, BLAS_REAL_TYPE);

        unsigned int i,j,k;

//        do i=1,pDOF
//            do j=1,pDOF
//                D(i,j) =1 +(i-1) + (pDOF)*(j-1)
//                Dt(j,i)=D(i,j)
//            enddo
//        enddo

        for(i=0; i < in_pDOF; ++i){
            for(j=0; j < in_pDOF; ++j){
                *nbb_at2(*D, i,j) = 1 + i + in_pDOF * j;
                *nbb_at2(*Dt, j,i) = nbb_get2(*D,i,j);
            }
        }

//        do i=1,pDOF
//            do j=1,pDOF
//                do k=1,pDOF
//                    u(i,j,k) = i+5*j+7*k
//                    ur(i,j,k) = 1+(i-1)
//                    us(i,j,k) = 2+(j-1)
//                    ut(i,j,k) = 3+(k-1)
//                enddo
//            enddo
//        enddo
        for(i=0; i < in_pDOF; ++i){
            for(j=0; j < in_pDOF; ++j){
                for(k=0; k < in_pDOF; ++k){
                    *nbb_at3(*u, i,j,k) = (i+1) +5*(j+1) +7*(k+1);
                    *nbb_at3(*ur, i,j,k) = 1 + i;
                    *nbb_at3(*us, i,j,k) = 2 + j;
                    *nbb_at3(*ut, i,j,k) = 3 + k;
                    *nbb_at3(*temp, i,j,k) = 0;
                }
            }
        }


        break; //while(!err){
    }
    return err;
}

Err_t destroy_data(nbb_matrix3_t * ur, nbb_matrix3_t *us, nbb_matrix3_t *ut,
                   nbb_matrix3_t *u, nbb_matrix3_t *temp, nbb_matrix2_t *D, nbb_matrix2_t *Dt)
{
    Err_t err=0;
    while(!err){
        SAFEFREE(ur->m);
        SAFEFREE(us->m);
        SAFEFREE(ut->m);
        SAFEFREE(u->m);
        SAFEFREE(temp->m);

        ur->sz_cols = 0;
        us->sz_cols = 0;
        ut->sz_cols = 0;
        u->sz_cols = 0;
        temp->sz_cols = 0;

        ur->sz_rows = 0;
        us->sz_rows = 0;
        ut->sz_rows = 0;
        u->sz_rows = 0;
        temp->sz_rows = 0;

        ur->sz_depth = 0;
        us->sz_depth = 0;
        ut->sz_depth = 0;
        u->sz_depth = 0;
        temp->sz_depth = 0;

        SAFEFREE(D->m);
        SAFEFREE(Dt->m);

        D->sz_cols = 0;
        Dt->sz_cols = 0;

        D->sz_rows = 0;
        Dt->sz_rows = 0;

        break; //while(!err){
    }
    return err;
}

Err_t check_local_grad3(unsigned int in_pDOF, FILE * fout)
{
    const int debug = 0;

    nbb_matrix3_t ur = {0};
    nbb_matrix3_t us = {0};
    nbb_matrix3_t ut = {0};

    nbb_matrix3_t u  = {0};

    nbb_matrix3_t temp  = {0}; //Not in use here. See check_local_grad3_t.

    nbb_matrix2_t D = {0};
    nbb_matrix2_t Dt = {0};

    Err_t err=0;
    while(!err){
        err = build_data(in_pDOF, &ur, &us, &ut, &u, &temp, &D, &Dt); IFEB;

        err = nbb_local_grad3(D,Dt, &ur, &us, &ut, &u); IFEB;

        fprintf(fout,"%s%10u\n", "LG_pDOF= ", in_pDOF);

        unsigned int i,j,k;
        for(i=0; i!=in_pDOF; ++i){
            for(j=0; j!=in_pDOF; ++j){
                for(k=0; k!=in_pDOF; ++k){
                    fprintf(fout, "%10u %10u %10u %23.14E %23.14E %23.14E %23.14E\n",
                            i+1,j+1,k+1,    //+1 in order to convert offset to Fortran index
                            nbb_get3(ur,i,j,k),
                            nbb_get3(us,i,j,k),
                            nbb_get3(ut,i,j,k),
                            nbb_get3(u,i,j,k)
                           );
                }
            }
        }

        err = destroy_data(&ur, &us, &ut, &u, &temp, &D, &Dt); IFEB;
        break;
    }

    destroy_data(&ur, &us, &ut, &u, &temp, &D, &Dt);

    return err;
}
int check_local_grad3t(unsigned int in_pDOF, FILE * fout)
{
    const int debug = 0;

    nbb_matrix3_t ur = {0};
    nbb_matrix3_t us = {0};
    nbb_matrix3_t ut = {0};

    nbb_matrix3_t u  = {0};

    nbb_matrix3_t temp  = {0};

    nbb_matrix2_t D = {0};
    nbb_matrix2_t Dt = {0};

    const unsigned int pDOF2D = in_pDOF * in_pDOF;
    const unsigned int pDOF3D = in_pDOF * in_pDOF * in_pDOF;

    Err_t err=0;
    while(!err){
        err = build_data(in_pDOF, &ur, &us, &ut, &u, &temp, &D, &Dt); IFEB;

        err = nbb_local_grad3_t(D,Dt, &ur, &us, &ut, &u, &temp); IFEB;

        fprintf(fout,"%s%10u\n", "LGt_pDOF= ", in_pDOF);

        unsigned int i,j,k;
        for(i=0; i!=in_pDOF; ++i){
            for(j=0; j!=in_pDOF; ++j){
                for(k=0; k!=in_pDOF; ++k){
                    fprintf(fout, "%10u %10u %10u %23.14E %23.14E %23.14E %23.14E\n",
                            i+1,j+1,k+1,    //+1 in order to convert offset to Fortran index
                            nbb_get3(ur,i,j,k),
                            nbb_get3(us,i,j,k),
                            nbb_get3(ut,i,j,k),
                            nbb_get3(u,i,j,k)
                           );
                }
            }
        }

        err = destroy_data(&ur, &us, &ut, &u, &temp, &D, &Dt); IFEB;

        break;
    }

    destroy_data(&ur, &us, &ut, &u, &temp, &D, &Dt);

    return err;
}

void prettyPrint_matrix2D(nbb_matrix2_t in_m)
{
    if(!in_m.m) return;
    unsigned int i,j;
    for(i=0; i!=in_m.sz_rows; ++i){
        for(j=0; j!=in_m.sz_cols; ++j){
            //write(*,*) "  M2(", i,", ", j,")=",M(i,j)
            fprintf(stdout,"  M2(%u, %u)=%g\n", i,j, nbb_get2(in_m,i,j));
        }
    }
}

void prettyPrint_matrix3D(nbb_matrix3_t in_m)
{
    if(!in_m.m) return;
    unsigned int i,j,k;
    for(i=0; i!=in_m.sz_rows; ++i){
        for(j=0; j!=in_m.sz_cols; ++j){
            for(k=0; k!=in_m.sz_depth; ++k){
                //write(*,*) "  M3(", i,", ", j,", ", k,")=",M(i,j,k)
                fprintf(stdout,"  M3(%u, %u, %u)=%g\n", i,j,k, nbb_get3(in_m,i,j,k));
            }
        }
    }
}

#endif // UNIT_TEST_NEKBONE_LOCAL_GRAD_C
