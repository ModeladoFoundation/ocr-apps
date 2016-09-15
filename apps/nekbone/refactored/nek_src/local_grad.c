#ifndef NEKBONE_LOCAL_GRAD_H
#include "local_grad.h"
#endif

int nbb_local_grad3(nbb_matrix3_t in_u,
                    nbb_matrix2_t in_D,
                    nbb_matrix2_t in_Dt,
                    nbb_matrix3_t * o_ur,
                    nbb_matrix3_t * o_us,
                    nbb_matrix3_t * o_ut
                    )
{
    int erri=0;

    //Original code
    //  real D (0:n,0:n)
    //  real u (0:n,0:n,0:n)
    //  real ur(0:n,0:n,0:n)
    //  m1 = n+1
    //  m2 = m1*m1
    //  call mxm(D ,m1,u,m1,ur,m2)
    //     real a(n1,n2),b(n2,n3),c(n1,n3)

    {
        nbb_matrix2_t u_dup = {0};
        u_dup.sz_rows = in_u.sz_rows;
        u_dup.sz_cols = in_u.sz_cols * in_u.sz_depth;
        u_dup.m = in_u.m;  //A temporary sharing as u_dup is used to change the shape of in_u

        nbb_matrix2_t ur_dup = {0};
        ur_dup.sz_rows = in_D.sz_rows;
        ur_dup.sz_cols = u_dup.sz_cols;
        ur_dup.m = o_ur->m; //A temporary sharing as ur_dup is used to change the shape of o_ur.

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
    for(k=0; k<in_u.sz_depth; ++k){
        nbb_matrix2_t u_dup = {0};
        u_dup.sz_rows = in_u.sz_rows;
        u_dup.sz_cols = in_u.sz_cols;
        u_dup.m = nbb_at3(in_u,0,0,k);

        nbb_matrix2_t us_dup = {0};
        us_dup.sz_rows = in_u.sz_rows;
        us_dup.sz_cols = in_Dt.sz_cols;
        us_dup.m = nbb_at3(*o_us,0,0,k);

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
        u_dup.sz_rows = in_u.sz_rows * in_u.sz_cols;
        u_dup.sz_cols = in_Dt.sz_rows;
        u_dup.m = in_u.m;

        nbb_matrix2_t ut_dup = {0};
        ut_dup.sz_rows = u_dup.sz_rows;
        ut_dup.sz_cols = in_Dt.sz_cols;
        ut_dup.m = o_ut->m;

        erri = nbb_mxm2(u_dup, in_Dt, &ut_dup); if(erri) return __LINE__;
    }
    return erri;
}

int nbb_local_grad3_t(nbb_matrix3_t in_u,
                      nbb_matrix2_t in_D,
                      nbb_matrix2_t in_Dt,
                      nbb_matrix3_t * o_ur,
                      nbb_matrix3_t * o_us,
                      nbb_matrix3_t * o_ut
                     )
{
//    return 0;
//}
    int erri=0;

    return erri;
}



