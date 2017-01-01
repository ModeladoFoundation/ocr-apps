#ifndef NEKBONE_SETUP_H
#include "nekbone_setup.h"
#endif

#include "init_mesh.h"
#include "set_vert_box.h"
#include "nbn_setup.h"
#include "set_f.h"

#include "blas3.h"
#include "blas1.h"
#include "polybasis.h"

double sum_vector(unsigned int sz, NBN_REAL * v)
{
    double sum=0;
    unsigned int i;
    for(i=0; i<sz; ++i){
        sum += v[i];
    }
    return sum;
}

Err_t nekbone_init_mesh(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * io_NEKOglobals,
                        unsigned int * io_lglel
                        )
{
    Err_t err=0;
    while(!err){
        unsigned int npx, npy, npz; // processor distribution in x,y,z
        unsigned int mx, my, mz; //local element distribution in x,y,z
        unsigned int nelx, nely, nelz;

        npx = in_NEKOstatics->Rx;
        npy = in_NEKOstatics->Ry;
        npz = in_NEKOstatics->Rz;

        mx = in_NEKOstatics->Ex;
        my = in_NEKOstatics->Ey;
        mz = in_NEKOstatics->Ez;

        err = nbb_init_mesh(NEKbone_ifbrick,
                            &npx, &npy, &npz,
                            &mx, &my, &mz,
                            in_NEKOstatics->Etotal, //in_nelt //The number of element per ranks
                            in_NEKOstatics->Rtotal, //in_np, //The total number of ranks in use
                            io_NEKOglobals->rankID, //in_nid, //The ID for the current rank: 0< nid < np
                            &nelx, &nely, &nelz,
                            io_lglel); IFEB;

        //The expectation is that the static variables would not have to change
        if(npx != in_NEKOstatics->Rx || npy != in_NEKOstatics->Ry || npz != in_NEKOstatics->Rz){
            err = __LINE__; IFEB;
        }
        if(mx != in_NEKOstatics->Ex || my != in_NEKOstatics->Ey || mz != in_NEKOstatics->Ez){
            err = __LINE__; IFEB;
        }

        io_NEKOglobals->ExRx = nelx;
        io_NEKOglobals->EyRy = nely;
        io_NEKOglobals->EzRz = nelz;

        //Sanity check
        if(nelx != npx*mx || nely != npy*my || nelz != npz*mz){
            err = __LINE__; IFEB;
        }

        break;
    }
    return err;
}

Err_t nekbone_set_vert_box(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * io_NEKOglobals,
                           unsigned int * io_lglel, unsigned long * io_glo_num
                          )
{
   Err_t err=0;
    while(!err){
        unsigned int nx1 = io_NEKOglobals->pDOF;
        unsigned int ny1 = io_NEKOglobals->pDOF;
        unsigned int nz1 = io_NEKOglobals->pDOF;

        unsigned int nelx = io_NEKOglobals->ExRx;
        unsigned int nely = io_NEKOglobals->EyRy;
        unsigned int nelz = io_NEKOglobals->EzRz;

        unsigned int nelt = in_NEKOstatics->Etotal;

        err = nbb_set_vert_box(io_lglel, nx1,ny1,nz1, nelx,nely,nelz, nelt, io_glo_num ); IFEB;
        break;
    }
    return err;
}

Err_t nekbone_setup(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * io_NEKOglobals,
                    unsigned int * io_lglel, unsigned long * io_glo_num)
{
   Err_t err=0;
    while(!err){
        err = nekbone_init_mesh(in_NEKOstatics, io_NEKOglobals, io_lglel); IFEB;

        //===== proxy_setupds
        err = nekbone_set_vert_box(in_NEKOstatics, io_NEKOglobals, io_lglel, io_glo_num); IFEB;
        //TODO: proxy_setupds: call gs_setup(gs_handle,glo_num,ntot,nekcomm,mp) ! Initialize gather-scatter

        break;
    }
    return err;
}

Err_t nekbone_set_multiplicity_start(NEKOglobals_t * in_NEKOglobals, NBN_REAL * io_C)
{
   Err_t err=0;
    while(!err){
        const unsigned int N = in_NEKOglobals->pDOF3DperR;
        const double one = 1;
        unsigned int i;
        for(i=0; i < N; ++i){
            io_C[i] = one;
        }

        //driver.f::set_multiplicity: call gs_op(gsh,c,1,1,0)  ! Gather-scatter operation  ! w   = QQ  w
        //See neko_halo.h::start_halo_multiplicity and neko_halo.h::stop_halo_multiplicity
        // for how that is handled.

        break;
    }
    return err;
}

Err_t nekbone_set_multiplicity_stop(NEKOglobals_t * in_NEKOglobals, NBN_REAL * io_C)
{
   Err_t err=0;
    while(!err){
        const unsigned int N = in_NEKOglobals->pDOF3DperR;
        const double one = 1;
        unsigned int i;

        //driver.f::set_multiplicity: handle what came out of call gs_op(gsh,c,1,1,0)  ! Gather-scatter operation  ! w   = QQ  w
        //See neko_halo.h::stop_halo_multiplicity for details

        for(i=0; i < N; ++i){
            io_C[i] = one/io_C[i];
        }

        break;
    }
    return err;
}

Err_t nekbone_proxy_setup(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                          NBN_REAL *io_wA, NBN_REAL *io_wC, NBN_REAL *io_wD,
                          NBN_REAL *io_wB, NBN_REAL *io_wZ, NBN_REAL *io_wW,
                          double * io_wZd, double * io_wWd,
                          NBN_REAL * io_G1, NBN_REAL * io_G4, NBN_REAL * io_G6,
                          BLAS_REAL_TYPE * io_dxm1, BLAS_REAL_TYPE * io_dxTm1
                          )
{
   Err_t err=0;
    while(!err){
        const unsigned int pDOF = in_NEKOglobals->pDOF;

        nbb_matrix2_t a,c,d;
        a.m = io_wA;
        c.m = io_wC;
        d.m = io_wD;
        a.sz_rows = pDOF; a.sz_cols = pDOF;
        c.sz_rows = pDOF; c.sz_cols = pDOF;
        d.sz_rows = pDOF; d.sz_cols = pDOF;

        NBN_REAL *b, *w, *z;
        b = io_wB;
        w = io_wW;
        z = io_wZ;

        double *work_z, *work_w;
        work_z = io_wZd;
        work_w = io_wWd;

        NBN_REAL *g1, *g4, *g6;
        g1 = io_G1;
        g4 = io_G4;
        g6 = io_G6;

        nbb_matrix2_t dxm1, dxtm1;
        dxm1.m = io_dxm1;
        dxtm1.m = io_dxTm1;
        dxm1.sz_rows = pDOF;  dxm1.sz_cols = pDOF;
        dxtm1.sz_rows = pDOF; dxtm1.sz_cols = pDOF;

        err = nbb_proxy_setup(a,b,c,d,z,w, pDOF,work_w,work_z, g1,g4,g6, dxm1,dxtm1); IFEB;

        break;
    }
    return err;
}

Err_t nekbone_set_f_start(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                          NBN_REAL * io_f)
{
   Err_t err=0;
    while(!err){
        const unsigned int N = in_NEKOglobals->pDOF3DperR;
        //driver.f::set_f: call gs_op(gsh,f,1,1,0)  ! Gather-scatter operation  ! w   = QQ  w
        //See neko_halo.h::start_halo_setf and neko_halo.h::stop_halo_setf
        // for how that is handled.

        err = nbb_set_f(N, io_f); IFEB;
        break;
    }
    return err;
}

Err_t nekbone_set_f_stop(NEKOglobals_t * in_NEKOglobals,
                         NBN_REAL * in_C, NBN_REAL * io_f)
{
   Err_t err=0;
    while(!err){
        const unsigned int N = in_NEKOglobals->pDOF3DperR;

        //driver.f::set_f: handle what came out of call gs_op(gsh,c,1,1,0)  ! Gather-scatter operation  ! w   = QQ  w
        //See neko_halo.h::stop_halo_setf for details

        //Now to do the original Fortran code: call col2 (f,c,n)
        unsigned int i;
        for(i=0; i < N; ++i){
            io_f[i] *= in_C[i];
        }

        break;
    }
    return err;
}


