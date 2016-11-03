#ifndef NEKBONE_AX_H
#include "ax.h"
#endif

#include "local_grad.h"

#define IFEB if(err) break

int nbb_ax_e(unsigned int in_pDOF, NBN_REAL * io_W, NBN_REAL * io_U,
             nbb_matrix2_t in_D, nbb_matrix2_t in_Dt,
             NBN_REAL * in_g1, NBN_REAL * in_g4, NBN_REAL * in_g6,
             NBN_REAL * io_wUR, NBN_REAL * io_wUS, NBN_REAL * io_wUT,
             NBN_REAL * io_temp)
{
    int err=0;
    while(!err){
        const unsigned int pDOF3D = in_pDOF * in_pDOF * in_pDOF;

        nbb_matrix3_t w,u, ur,us,ut, temp;

        ur.sz_rows  = in_pDOF;
        ur.sz_cols  = in_pDOF;
        ur.sz_depth = in_pDOF;
        ur.m = io_wUR;

        us.sz_rows  = in_pDOF;
        us.sz_cols  = in_pDOF;
        us.sz_depth = in_pDOF;
        us.m = io_wUS;

        ut.sz_rows  = in_pDOF;
        ut.sz_cols  = in_pDOF;
        ut.sz_depth = in_pDOF;
        ut.m = io_wUT;

        u.sz_rows  = in_pDOF;
        u.sz_cols  = in_pDOF;
        u.sz_depth = in_pDOF;
        u.m = io_U;

        w.sz_rows  = in_pDOF;
        w.sz_cols  = in_pDOF;
        w.sz_depth = in_pDOF;
        w.m = io_W;

        temp.sz_rows  = in_pDOF;
        temp.sz_cols  = in_pDOF;
        temp.sz_depth = in_pDOF;
        temp.m = io_temp;

        //call local_grad3(ur,us,ut,u,n,dxm1,dxtm1)
        err = nbb_local_grad3(in_D, in_Dt, &ur, &us, &ut, &u); IFEB;

        unsigned int i;
        for(i=0; i<pDOF3D; ++i){
            NBN_REAL wr = in_g1[i]*io_wUR[i];
            NBN_REAL ws = in_g4[i]*io_wUS[i];
            NBN_REAL wt = in_g6[i]*io_wUT[i];
            io_wUR[i] = wr;
            io_wUS[i] = ws;
            io_wUT[i] = wt;
        }

        //call local_grad3_t(w,ur,us,ut,n,dxm1,dxtm1)
        err = nbb_local_grad3_t(in_D,in_Dt, &ur, &us, &ut, &w, &temp); IFEB;

        break; //while(!erri)
    }
    return err;
}

int nbb_axi_before(unsigned int in_Ecount, unsigned int in_pDOF,
                   NBN_REAL * io_W, NBN_REAL * io_P,
                   nbb_matrix2_t in_D, nbb_matrix2_t in_Dt,
                   NBN_REAL * in_g1, NBN_REAL * in_g4, NBN_REAL * in_g6,
                   NBN_REAL * io_wUR, NBN_REAL * io_wUS, NBN_REAL * io_wUT,
                   NBN_REAL * io_temp )
{
    int err=0;
    while(!err){
        nbb_matrix2_t w = {0};
        nbb_matrix2_t p = {0};

        w.sz_rows = in_pDOF * in_pDOF * in_pDOF;
        w.sz_cols = in_Ecount;
        w.m = io_W;

        p.sz_rows = in_pDOF * in_pDOF * in_pDOF;
        p.sz_cols = in_Ecount;
        p.m = io_P;

        unsigned int e;
        for(e=0; e<in_Ecount; ++e){
            NBN_REAL * atW = nbb_at2(w,0,e);
            NBN_REAL * atP = nbb_at2(p,0,e);

            err = nbb_ax_e(in_pDOF, atW, atP,
                  in_D, in_Dt, in_g1, in_g4, in_g6,
                  io_wUR, io_wUS, io_wUT, io_temp ); IFEB;

        }

        break; //while(!erri)
    }
    return err;
}





































