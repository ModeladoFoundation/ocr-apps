#ifndef NEKBONE_CG_H
#include "nekbone_cg.h"
#endif

#include "ocr.h" //PRINTF

#include "blas3.h"
#include "ax.h"

#include <math.h> //For sqrt

#define XMEMSET(SRC, CHARC, SZ) {unsigned int xmIT; for(xmIT=0; xmIT<SZ; ++xmIT) *((char*)SRC+xmIT)=CHARC;}
#define XMEMCPY(DEST, SRC, SZ) {unsigned int xmIT; for(xmIT=0; xmIT<SZ; ++xmIT) *((char*)DEST+xmIT)=*((char*)SRC+xmIT);}

Err_t init_NEKO_CGscalars(NEKO_CGscalars_t * io)
{
    XMEMSET(io, 0, sizeof(NEKO_CGscalars_t));
    io->rtz1 = 1;
    return 0;
}
Err_t destroy_NEKO_CGscalars(NEKO_CGscalars_t * io)
{
    XMEMSET(io, 0, sizeof(NEKO_CGscalars_t));
    return 0;
}
Err_t copy_NEKO_CGscalars(NEKO_CGscalars_t * in_from, NEKO_CGscalars_t * o_target)
{
    XMEMCPY(o_target, in_from, sizeof(NEKO_CGscalars_t));
    return 0;
}
void  print_NEKO_CGscalars(NEKO_CGscalars_t * in)
{
    PRINTF("NEKO_CGscalars: currentCGiteration= %ld\n", in->currentCGiteration);
    PRINTF("NEKO_CGscalars: rtz1=%23.14E   rtz2=%23.14E\n", in->rtz1, in->rtz2);
    PRINTF("NEKO_CGscalars: beta=%23.14E   pap=%23.14E\n", in->beta, in->pap);
    PRINTF("NEKO_CGscalars: alpha=%23.14E   alphm=%23.14E\n", in->alpha, in->alphm);
    PRINTF("NEKO_CGscalars: rtr=%23.14E   rnorm=%23.14E\n", in->rtr, in->rnorm);
}

void nekbone_mask(unsigned int in_rankID, NBN_REAL * io)
{
    if(0 == in_rankID){
        io[0] = 0; // suitable for solvability
    }
}

Err_t nekbone_CGstep0_start(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                            NBN_REAL * in_F, NBN_REAL * in_C, NBN_REAL * io_R )
{
    Err_t err=0;
    while(!err){
        unsigned int i;
        for(i=0; i< in_NEKOglobals->pDOF3DperR; ++i){
            io_R[i] = in_F[i];
        }

        if(NEKbone_thread == 0){
            nekbone_mask(in_NEKOglobals->rankID, io_R);
        }

        //TODO: nekbone_CGstep0_start: Calculate administrative function glsc3i(rnorminit,r,c,r,n,find,lind) -> rnorminit = r' * (c.*r)

        break;
    }
    return err;
}

void nekbone_zero_variables(unsigned int in_N, NBN_REAL * io_X, NBN_REAL *io_W,
                            NBN_REAL *io_P, NBN_REAL *io_Z
                            )
{
    const NBN_REAL zero = 0;
    unsigned int i;
    for(i=0; i< in_N; ++i){
        io_X[i] = zero;
        io_W[i] = zero;
        io_P[i] = zero;
        io_Z[i] = zero;
    }
}

Err_t nekbone_CGstep0_stop(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                           NBN_REAL *io_X, NBN_REAL *io_W, NBN_REAL *io_P, NBN_REAL *io_Z,
                           NEKO_CGscalars_t * io_CGscalars)
{
    Err_t err=0;
    while(!err){
        nekbone_zero_variables(in_NEKOglobals->pDOF3DperR, io_X, io_W, io_P, io_Z);
        err = init_NEKO_CGscalars(io_CGscalars); IFEB;
        break;
    }
    return err;
}

Err_t nekbone_tailTransitBegin(long in_currentIteration,
                               NEKO_CGscalars_t * in_CGstats, NEKO_CGscalars_t * io_CGstats)
{
    Err_t err=0;
    while(!err){
        err = copy_NEKO_CGscalars(in_CGstats, io_CGstats); IFEB;
        io_CGstats->currentCGiteration = in_currentIteration;
        break;
    }
    return err;
}

Err_t nekbone_solveMi(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                          NBN_REAL *in_R, NBN_REAL *io_Z)
{
    Err_t err=0;
    while(!err){
        unsigned int i;
        for(i=0; i<in_NEKOglobals->pDOF3DperR; ++i){
            io_Z[i] = in_R[i];
        }
        break;
    }
    return err;
}

NBN_REAL nekbone_localpart_glsc3i(unsigned int in_length, NBN_REAL *in_A, NBN_REAL *in_B,
                                  NBN_REAL *in_Multi)
{
    NBN_REAL sum = 0;
    unsigned int i;
    for(i=0; i < in_length; ++i){
        sum += in_A[i] * in_B[i] * in_Multi[i];
    }

    NBN_REAL psum = 0;

    if( 1 == NEKbone_find ) psum = 0; //Superflous but it was in the NEkbone baseline.
                                      //It might serve as a marker.
    psum += sum;

    return psum;
}

Err_t nekbone_beta_start(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                         NEKO_CGscalars_t * in_CGstats, NEKO_CGscalars_t * io_CGstats,
                         NBN_REAL *io_R, NBN_REAL *io_C, NBN_REAL *io_Z,
                         NBN_REAL * o_partial_sum_rcz
                         )
{
    NEKO_CGscalars_t * S = io_CGstats; //Just for convenience.

    Err_t err=0;
    while(!err){
        err = copy_NEKO_CGscalars(in_CGstats, S); IFEB;
        S->rtz2 = S->rtz1;

        *o_partial_sum_rcz = 0;
        *o_partial_sum_rcz = nekbone_localpart_glsc3i(in_NEKOglobals->pDOF3DperR, io_R, io_C, io_Z);

        //TODO: nekbone_beta_start: DO the ALL_Reduce: call gop(*o_partial_sum_rcz, work,'+  ',1)

        break;
    }

    S = 0;

    return err;
}

//subroutine add2s1i(a,b,c1,n,start,fin)
void nekbone_add2s1i(unsigned int in_length, NBN_REAL *in_A, NBN_REAL *in_B,
                                  NBN_REAL in_C1, NBN_REAL * io_A)
{
    unsigned int i;
    for(i=0; i < in_length; ++i){
        io_A[i] = in_C1 * in_A[i] + in_B[i];
    }
}

Err_t nekbone_beta_stop(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                        NEKO_CGscalars_t * in_CGstats, NEKO_CGscalars_t * io_CGstats,
                        NBN_REAL * in_sum_rcz,
                        NBN_REAL *in_P, NBN_REAL *io_Z, NBN_REAL *io_P
                        )
{
    //OA.addCustomText(G, nc, '//SKE-NOTE:  This involves add2s1i(p,z,beta,...')
    //OA.addCustomText(G, nc, '//SKE-NOTE:  This involves the loop over all call ax_e')
    NEKO_CGscalars_t * S = io_CGstats; //Just for convenience.

    Err_t err=0;
    while(!err){
        //TODO: nekbone_beta_stop: Complete the ALL_Reduce: call gop(*o_partial_sum_rcz, work,'+  ',1)

        err = copy_NEKO_CGscalars(in_CGstats, S); IFEB;

        //At this point, in_sum_rcz is supposed to be rtz1, as per baseline code.
        S->rtz1 = *in_sum_rcz;
        S->beta = S->rtz1 / S->rtz2;

        if( 0 == S->currentCGiteration) S->beta=0.0;

        //call add2s1i(p,z,beta,n,find,lind)                              ! 2n
        nekbone_add2s1i(in_NEKOglobals->pDOF3DperR, in_P, io_Z, S->beta, io_P);

        break;
    }

    S = 0;

    return err;
}

Err_t nekbone_alpha_start(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                          NBN_REAL *io_W, NBN_REAL *io_C, NBN_REAL *io_P,
                          NBN_REAL * o_partial_sum_pap
                         )
{
    Err_t err=0;
    while(!err){
        //call glsc3i(pap, w,c,p,n,find,lind)
        *o_partial_sum_pap = nekbone_localpart_glsc3i(in_NEKOglobals->pDOF3DperR, io_W, io_C, io_P);

        //TODO: nekbone_alpha_start: DO the ALL_Reduce: call gop(*o_partial_sum_pap, work,'+  ',1)
        break;
    }
    return err;
}

//subroutine add2s2i(a,b,c1,n,start,fin)
void nekbone_add2s2i(unsigned int in_length, NBN_REAL *in_A, NBN_REAL *in_B,
                                  NBN_REAL in_C1, NBN_REAL * io_A)
{
    unsigned int i;
    for(i=0; i < in_length; ++i){
        io_A[i] = in_A[i] + in_C1 * in_B[i];
    }
}

Err_t nekbone_alpha_stop(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                         NEKO_CGscalars_t * in_CGstats, NEKO_CGscalars_t * io_CGstats,
                         NBN_REAL * in_sum_pap,
                         NBN_REAL *in_X, NBN_REAL *in_P, NBN_REAL *io_X,
                         NBN_REAL *in_R, NBN_REAL *in_W, NBN_REAL *io_R
                         )
{
    NEKO_CGscalars_t * S = io_CGstats; //Just for convenience.
    Err_t err=0;
    while(!err){
        //TODO: nekbone_alpha_stop: Post-process the ALL_Reduce: call gop(*o_partial_sum_pap, work,'+  ',1)

        err = copy_NEKO_CGscalars(in_CGstats, S); IFEB;

        //At this point, in_sum_pap is supposed to be pap, as per baseline code.
        S->pap = *in_sum_pap;

        //alpha=rtz1/pap  alphm=-alpha
        S->alpha = S->rtz1 / S->pap;
        S->alphm = -(S->alpha);

        //call add2s2i(x,p,alpha,n,find,lind)                             ! 2n
        nekbone_add2s2i(in_NEKOglobals->pDOF3DperR, in_X, in_P, S->alpha, io_X);

        //call add2s2i(r,w,alphm,n,find,lind)                             ! 2n
        nekbone_add2s2i(in_NEKOglobals->pDOF3DperR, in_R, in_W, S->alphm, io_R);

        break;
    }
    S = 0;
    return err;
}

Err_t nekbone_rtr_start(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                        NBN_REAL *io_R, NBN_REAL *io_C, NBN_REAL * o_partial_sum_rtr )
{
    Err_t err=0;
    while(!err){
        //call  glsc3i(rtr, r,c,r,n,find,lind)
        *o_partial_sum_rtr = nekbone_localpart_glsc3i(in_NEKOglobals->pDOF3DperR, io_R, io_C, io_R);

        //TODO: nekbone_rtr_start: DO the ALL_Reduce: call gop(*o_partial_sum_rtr, work,'+  ',1)
        break;
    }
    return err;
}

Err_t nekbone_rtr_stop(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                       NEKO_CGscalars_t * in_CGstats, NEKO_CGscalars_t * io_CGstats,
                       NBN_REAL * in_sum_rtr)
{
    Err_t err=0;
    while(!err){
        //TODO: nekbone_rtr_stop: Process the ALL_Reduce: call gop(*o_partial_sum_rtr, work,'+  ',1)
        err = copy_NEKO_CGscalars(in_CGstats, io_CGstats); IFEB;

        io_CGstats->rtr = *in_sum_rtr;

        io_CGstats->rnorm = sqrt(*in_sum_rtr);

        break;
    }
    return err;
}


Err_t nekbone_ai_start(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                       NBN_REAL *io_W, NBN_REAL *io_P,
                       NBN_REAL * io_UR, NBN_REAL * io_US, NBN_REAL * io_UT,
                       NBN_REAL * in_G1, NBN_REAL * in_G4, NBN_REAL * in_G6,
                       NBN_REAL * in_dxm1, NBN_REAL * in_dxTm1,
                       NBN_REAL * io_AItemp
                      )
{
   Err_t err=0;
    while(!err){
        unsigned int Ecount = in_NEKOstatics->Etotal;
        unsigned int pDOF = in_NEKOglobals->pDOF;

        nbb_matrix2_t D = {0};
        nbb_matrix2_t Dt = {0};

        D.sz_rows = pDOF;
        D.sz_cols = pDOF;
        D.m = in_dxm1;

        Dt.sz_rows = pDOF;
        Dt.sz_cols = pDOF;
        Dt.m = in_dxTm1;

        err = nbb_axi_before(Ecount, pDOF, io_W, io_P, D,Dt,
                             in_G1,in_G4,in_G6, io_UR,io_US,io_UT,
                             io_AItemp); IFEB;
        break;
    }
    return err;
}

Err_t nekbone_ai_stop(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                      NBN_REAL *in_W, NBN_REAL *in_P, NBN_REAL *io_W)
{
   Err_t err=0;
    while(!err){
        //TODO: nekbone_ai_stop: Process Halo Exchange: call gs_op(gsh,w,1,1,0)

        //call add2s2i(w,u,.1,n,find,lind)
        const NBN_REAL fudge = 0.1;  //Yes.  This fudge factor is in Nekbone's baseline code.
        nekbone_add2s2i(in_NEKOglobals->pDOF3DperR, in_W, in_P, fudge, io_W);

        if(1 == NEKbone_find){
            nekbone_mask(in_NEKOglobals->rankID, io_W);
        }
        break;
    }
    return err;
}























