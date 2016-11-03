#ifndef NEKBONE_LOCAL_GRAD_H
#include "local_grad.h"
#endif

int nbb_local_grad3(nbb_matrix2_t in_D, nbb_matrix2_t in_Dt,
                    nbb_matrix3_t * io_ur, nbb_matrix3_t * io_us,
                    nbb_matrix3_t * io_ut, nbb_matrix3_t * io_u)
{
    int erri=0;

    //Original code
    //  real D (0:n,0:n)
    //  real u (0:n,0:n,0:n)
    //  real ur(0:n,0:n,0:n)
    //  m1 = n+1
    //  m2 = m1*m1
    //  call mxm(D ,m1,u,m1,ur,m2)
    //      subroutine mxm(a,n1,b,n2,c,n3)
    //          real a(n1,n2),b(n2,n3),c(n1,n3)

    {
        nbb_matrix2_t u_dup = {0};
        u_dup.sz_rows = io_u->sz_rows;
        u_dup.sz_cols = io_u->sz_cols * io_u->sz_depth;
        u_dup.m = io_u->m;  //A temporary sharing as u_dup is used to change the shape of in_u

        nbb_matrix2_t ur_dup = {0};
        ur_dup.sz_rows = in_D.sz_rows;
        ur_dup.sz_cols = u_dup.sz_cols;
        ur_dup.m = io_ur->m; //A temporary sharing as ur_dup is used to change the shape of o_ur.

        erri = nbb_mxm2(in_D, u_dup, &ur_dup); if(erri) return __LINE__;
    }

    //Original code
    //  real Dt(0:n,0:n)
    //  real u (0:n,0:n,0:n)
    //  real us(0:n,0:n,0:n)
    //      do k=0,n
    //         call mxm(u(0,0,k),m1,Dt,m1,us(0,0,k),m1)
    //      enddo

    BLAS_UINT_TYPE k=0;
    for(k=0; k < io_u->sz_depth; ++k){
        nbb_matrix2_t u_dup = {0};
        u_dup.sz_rows = io_u->sz_rows;
        u_dup.sz_cols = io_u->sz_cols;
        u_dup.m = nbb_at3(*io_u,0,0,k);

        nbb_matrix2_t us_dup = {0};
        us_dup.sz_rows = io_u->sz_rows;
        us_dup.sz_cols = in_Dt.sz_cols;
        us_dup.m = nbb_at3(*io_us,0,0,k);

        erri = nbb_mxm2(u_dup, in_Dt, &us_dup); if(erri) return __LINE__;
    }

    //Original code
    //  real D (0:n,0:n)
    //  real u (0:n,0:n,0:n)
    //  real ur(0:n,0:n,0:n)
    //  m1 = n+1
    //  m2 = m1*m1
    //  call mxm(u,m2,Dt,m1,ut,m1)
    //     real a(n1,n2),b(n2,n3),c(n1,n3)
    {
        nbb_matrix2_t u_dup = {0};
        u_dup.sz_rows = io_u->sz_rows * io_u->sz_cols;
        u_dup.sz_cols = in_Dt.sz_rows;
        u_dup.m = io_u->m;

        nbb_matrix2_t ut_dup = {0};
        ut_dup.sz_rows = u_dup.sz_rows;
        ut_dup.sz_cols = in_Dt.sz_cols;
        ut_dup.m = io_ut->m;

        erri = nbb_mxm2(u_dup, in_Dt, &ut_dup); if(erri) return __LINE__;
    }
    return erri;
}

int nbb_local_grad3_t(nbb_matrix2_t in_D, nbb_matrix2_t in_Dt,
                      nbb_matrix3_t * io_ur, nbb_matrix3_t * io_us,
                      nbb_matrix3_t * io_ut, nbb_matrix3_t * io_u,
                      nbb_matrix3_t * io_temp)
{
    int erri=0;

    //Original code
    //real u (0:N,0:N,0:N)
    //real ur(0:N,0:N,0:N),us(0:N,0:N,0:N),ut(0:N,0:N,0:N)
    //real D (0:N,0:N),Dt(0:N,0:N)
    //real w (0:N,0:N,0:N)
    //integer e
    //
    //m1 = N+1
    //m2 = m1*m1
    //m3 = m1*m1*m1
    //
    //call mxm(Dt,m1,ur,m1,u,m2)
    //      subroutine mxm(a,n1,b,n2,c,n3)
    //          real a(n1,n2),b(n2,n3),c(n1,n3)

   {
        nbb_matrix2_t ur_dup = {0};
        ur_dup.sz_rows = in_D.sz_rows;
        ur_dup.sz_cols = io_ur->sz_cols * io_ur->sz_depth;
        ur_dup.m = io_ur->m; //A temporary sharing as ur_dup is used to change the shape of io_ur.

        nbb_matrix2_t u_dup = {0};
        u_dup.sz_rows = in_D.sz_rows;
        u_dup.sz_cols = ur_dup.sz_cols;
        u_dup.m = io_u->m;  //A temporary sharing as u_dup is used to change the shape of io_u

        erri = nbb_mxm2(in_Dt, ur_dup, &u_dup); if(erri) return __LINE__;
    }

    //do k=0,N
    //  call mxm(us(0,0,k),m1,D ,m1,w(0,0,k),m1)
    //enddo

    BLAS_UINT_TYPE k=0;
    for(k=0; k < io_u->sz_depth; ++k){
        nbb_matrix2_t us_dup = {0};
        us_dup.sz_rows = io_us->sz_rows;
        us_dup.sz_cols = in_D.sz_cols;
        us_dup.m = nbb_at3(*io_us,0,0,k);

        nbb_matrix2_t w = {0};
        w.sz_rows = us_dup.sz_rows;
        w.sz_cols = us_dup.sz_cols;
        w.m = nbb_at3(*io_temp,0,0,k);

        erri = nbb_mxm2(us_dup, in_D, &w); if(erri) return __LINE__;
    }

    const BLAS_UINT_TYPE m3 = io_u->sz_rows * io_u->sz_cols * io_u->sz_depth;
    //call add2(u,w,m3)
    for(k=0; k < m3; ++k){
        io_u->m[k] += io_temp->m[k];
    }

    //call mxm(ut,m2,D ,m1,w,m1)
    //      subroutine mxm(a,n1,b,n2,c,n3)
    //          real a(n1,n2),b(n2,n3),c(n1,n3)
   {
        nbb_matrix2_t ut_dup = {0};
        ut_dup.sz_rows = io_ut->sz_rows * io_ut->sz_cols;
        ut_dup.sz_cols = io_ut->sz_depth;
        ut_dup.m = io_ut->m; //A temporary sharing as ut_dup is used to change the shape of io_ut.

        nbb_matrix2_t w = {0};
        w.sz_rows = ut_dup.sz_rows;
        w.sz_cols = in_D.sz_cols;
        w.m = io_temp->m;

        erri = nbb_mxm2(ut_dup, in_D, &w); if(erri) return __LINE__;
    }

    //call add2(u,w,m3)
    for(k=0; k < m3; ++k){
        io_u->m[k] += io_temp->m[k];
    }

    return erri;
}



