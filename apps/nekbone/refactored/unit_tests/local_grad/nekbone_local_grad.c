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

void prettyPrint_matrix2D(nbb_matrix2_t in_m);  //For debugging
void prettyPrint_matrix3D(nbb_matrix3_t in_m);  //For debugging

int check_local_grad3(unsigned int in_polyorder, FILE * fout);
int check_local_grad3_t(unsigned int in_polyorder, FILE * fout);

int main(int argc, char * argv[])
{
    FILE * fout = 0;

    Err_t err=0;
    while(!err){

        fout = fopen("./ccode.out", "w");
        if(!fout) {err=__LINE__; IFEB;}

        unsigned int first_polyOrder = 2;
        unsigned int last_polyOrder = 10;

        unsigned int i=0;

        for(i=first_polyOrder; i<= last_polyOrder; ++i){
            fprintf(fout,"%s%10u\n", "LG", i);
            err = check_local_grad3(i,fout); IFEB;
            fprintf(fout,"%s%10u\n", "LGt", i);
            err = check_local_grad3t(i,fout); IFEB;
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

int check_local_grad3(unsigned int in_polyorder, FILE * fout)
{
    const int debug = 0;

    nbb_matrix3_t ur = {0};
    nbb_matrix3_t us = {0};
    nbb_matrix3_t ut = {0};

    nbb_matrix3_t u  = {0};

    nbb_matrix2_t D = {0};
    nbb_matrix2_t Dt = {0};

    unsigned int p2 = in_polyorder * in_polyorder;
    unsigned int p3 = in_polyorder * in_polyorder * in_polyorder;

    unsigned int i,j,k;

    Err_t err=0;
    while(!err){
        ur.sz_rows = in_polyorder;
        ur.sz_cols = in_polyorder;
        ur.sz_depth= in_polyorder;
        ur.m = malloc( p3 * sizeof(BLAS_REAL_TYPE)); if(!ur.m) {err=__LINE__; IFEB;}

        us.sz_rows = in_polyorder;
        us.sz_cols = in_polyorder;
        us.sz_depth= in_polyorder;
        us.m = malloc( p3 * sizeof(BLAS_REAL_TYPE)); if(!us.m) {err=__LINE__; IFEB;}

        ut.sz_rows = in_polyorder;
        ut.sz_cols = in_polyorder;
        ut.sz_depth= in_polyorder;
        ut.m = malloc( p3 * sizeof(BLAS_REAL_TYPE)); if(!ut.m) {err=__LINE__; IFEB;}

        u.sz_rows = in_polyorder;
        u.sz_cols = in_polyorder;
        u.sz_depth= in_polyorder;
        u.m = malloc( p3 * sizeof(BLAS_REAL_TYPE)); if(!u.m) {err=__LINE__; IFEB;}

        D.sz_rows = in_polyorder;
        D.sz_cols = in_polyorder;
        D.m = malloc( p2 * sizeof(BLAS_REAL_TYPE)); if(!D.m) {err=__LINE__; IFEB;}

        Dt.sz_rows = in_polyorder;
        Dt.sz_cols = in_polyorder;
        Dt.m = malloc( p2 * sizeof(BLAS_REAL_TYPE)); if(!Dt.m) {err=__LINE__; IFEB;}

//        do i=1,polyOrder
//            do j=1,polyOrder
//                D(i,j) =1 +(i-1) + (polyOrder)*(j-1)
//                Dt(j,i)=D(i,j)
//            enddo
//        enddo
        for(i=0; i!=in_polyorder; ++i){
            for(j=0; j!=in_polyorder; ++j){
                *nbb_at2(D,i,j) = 1 + i + in_polyorder*j;
                *nbb_at2(Dt,j,i) = 1 + i + in_polyorder*j;
            }
        }

        if(debug){
            fprintf(stdout,"D\n");
            prettyPrint_matrix2D(D); fflush(stdout);
        }

//        do i=1,polyOrder
//            do j=1,polyOrder
//                do k=1,polyOrder
//                    u(i,j,k) = i+2*j+3*k
//                enddo
//            enddo
//        enddo
        for(i=0; i!=in_polyorder; ++i){
            for(j=0; j!=in_polyorder; ++j){
                for(k=0; k!=in_polyorder; ++k){
                    *nbb_at3(u,i,j,k) = (i+1) + 2*(j+1) + 3*(k+1);
                }
            }
        }

        if(debug){
            fprintf(stdout,"u\n");
            prettyPrint_matrix3D(u); fflush(stdout);
        }

//        call local_grad3(ur,us,ut, u, modP, D,Dt)
        err = nbb_local_grad3(u,D,Dt, &ur, &us, &ut); IFEB;

//        do i=1,polyOrder
//            do j=1,polyOrder
//                do k=1,polyOrder
//                    write(10,'(3I10,3ES23.14)') i,j,k, ur(i,j,k), us(i,j,k), ut(i,j,k)
//                enddo
//            enddo
//        enddo
        for(i=0; i!=in_polyorder; ++i){
            for(j=0; j!=in_polyorder; ++j){
                for(k=0; k!=in_polyorder; ++k){
                    fprintf(fout, "%10u%10u%10u%23.14E%23.14E%23.14E\n",
                            i+1,j+1,k+1,    //+1 in order to convert offset to Fortran index
                            nbb_get3(ur,i,j,k),
                            nbb_get3(us,i,j,k),
                            nbb_get3(ut,i,j,k)
                           );
                }
            }
        }

        break;
    }

    SAFEFREE(ur.m);
    SAFEFREE(us.m);
    SAFEFREE(ut.m);
    SAFEFREE( u.m);
    SAFEFREE( D.m);
    SAFEFREE(Dt.m);

    return err;
}
int check_local_grad3t(unsigned int in_polyorder, FILE * fout)
{
    const int debug = 0;

    nbb_matrix3_t ur = {0};
    nbb_matrix3_t us = {0};
    nbb_matrix3_t ut = {0};

    nbb_matrix3_t u  = {0};

    nbb_matrix2_t D = {0};
    nbb_matrix2_t Dt = {0};

    unsigned int p2 = in_polyorder * in_polyorder;
    unsigned int p3 = in_polyorder * in_polyorder * in_polyorder;

    unsigned int i,j,k;

    Err_t err=0;
    while(!err){
        D.sz_rows = in_polyorder;
        D.sz_cols = in_polyorder;
        D.m = malloc( p2 * sizeof(BLAS_REAL_TYPE)); if(!D.m) {err=__LINE__; IFEB;}

        Dt.sz_rows = in_polyorder;
        Dt.sz_cols = in_polyorder;
        Dt.m = malloc( p2 * sizeof(BLAS_REAL_TYPE)); if(!Dt.m) {err=__LINE__; IFEB;}

        //do i=1,polyOrder
        //    do j=1,polyOrder
        //        D(i,j) =1 +(i-1) + (polyOrder)*(j-1)
        //        Dt(j,i)=D(i,j)
        //    enddo
        //enddo
        for(i=0; i!=in_polyorder; ++i){
            for(j=0; j!=in_polyorder; ++j){
                *nbb_at2(D,i,j) = 1 + i + in_polyorder*j;
                *nbb_at2(Dt,j,i) = 1 + i + in_polyorder*j;
            }
        }

        ur.sz_rows = in_polyorder;
        ur.sz_cols = in_polyorder;
        ur.sz_depth= in_polyorder;
        ur.m = malloc( p3 * sizeof(BLAS_REAL_TYPE)); if(!ur.m) {err=__LINE__; IFEB;}

        us.sz_rows = in_polyorder;
        us.sz_cols = in_polyorder;
        us.sz_depth= in_polyorder;
        us.m = malloc( p3 * sizeof(BLAS_REAL_TYPE)); if(!us.m) {err=__LINE__; IFEB;}

        ut.sz_rows = in_polyorder;
        ut.sz_cols = in_polyorder;
        ut.sz_depth= in_polyorder;
        ut.m = malloc( p3 * sizeof(BLAS_REAL_TYPE)); if(!ut.m) {err=__LINE__; IFEB;}

        u.sz_rows = in_polyorder;
        u.sz_cols = in_polyorder;
        u.sz_depth= in_polyorder;
        u.m = malloc( p3 * sizeof(BLAS_REAL_TYPE)); if(!u.m) {err=__LINE__; IFEB;}

        if(debug){
            fprintf(stdout,"D\n");
            prettyPrint_matrix2D(D); fflush(stdout);
        }

        //do i=1,polyOrder
        //    do j=1,polyOrder
        //        do k=1,polyOrder
        //            u(i,j,k) = i+2*j+3*k
        //            ur(i,j,k) = 2*i+j+k
        //            us(i,j,k) = i+j+2*k
        //            ut(i,j,k) = i+2*j+k
        //        enddo
        //    enddo
        //enddo
        for(i=0; i!=in_polyorder; ++i){
            for(j=0; j!=in_polyorder; ++j){
                for(k=0; k!=in_polyorder; ++k){
                    *nbb_at3(u,i,j,k)  = (i+1) + 2*(j+1) + 3*(k+1);
                    *nbb_at3(ur,i,j,k) = 2*(i+1) +   (j+1) +   (k+1);
                    *nbb_at3(us,i,j,k) =   (i+1) +   (j+1) + 2*(k+1);
                    *nbb_at3(ut,i,j,k) =   (i+1) + 2*(j+1) +   (k+1);
                }
            }
        }

        if(debug){
            fprintf(stdout,"u\n");
            prettyPrint_matrix3D(u); fflush(stdout);
        }

        //call local_grad3_t(u, ur,us,ut, modP, D,Dt)
        err = nbb_local_grad3_t(u,D,Dt, &ur, &us, &ut); IFEB;

        //do i=1,polyOrder
        //    do j=1,polyOrder
        //        do k=1,polyOrder
        //            write(10,'(3I10,3ES23.14)') i,j,k, ur(i,j,k), us(i,j,k), ut(i,j,k)
        //        enddo
        //    enddo
        //enddo
        for(i=0; i!=in_polyorder; ++i){
            for(j=0; j!=in_polyorder; ++j){
                for(k=0; k!=in_polyorder; ++k){
                    fprintf(fout, "%10u%10u%10u%23.14E%23.14E%23.14E\n",
                            i+1,j+1,k+1,    //+1 in order to convert offset to Fortran index
                            nbb_get3(ur,i,j,k),
                            nbb_get3(us,i,j,k),
                            nbb_get3(ut,i,j,k)
                           );
                }
            }
        }

        break;
    }

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
