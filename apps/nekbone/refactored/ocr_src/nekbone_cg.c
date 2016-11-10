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

Err_t init_NEKO_CGtimings(NEKO_CGtimings_t * io)
{
    XMEMSET(io, 0, sizeof(NEKO_CGtimings_t));
    return 0;
}
Err_t destroy_NEKO_CGtimings(NEKO_CGtimings_t * io)
{
    XMEMSET(io, 0, sizeof(NEKO_CGtimings_t));
    return 0;
}
Err_t copy_NEKO_CGtimings(NEKO_CGtimings_t * in_from, NEKO_CGtimings_t * o_target)
{
    XMEMCPY(o_target, in_from, sizeof(NEKO_CGtimings_t));
    return 0;
}
void  print_NEKO_CGtimings(NEKO_CGtimings_t * in)
{
    TIMEPRINT1("NEKO_CGtimings: cumu_tailRecurTransitBEGIN="TIMEF"\n", in->cumu_tailRecurTransitBEGIN);
    TIMEPRINT1("NEKO_CGtimings: cumu_nekCG_solveMi="TIMEF"\n", in->cumu_nekCG_solveMi);
    TIMEPRINT1("NEKO_CGtimings: cumu_nekCG_beta_start="TIMEF"\n", in->cumu_nekCG_beta_start);
    TIMEPRINT1("NEKO_CGtimings: cumu_nekCG_beta_stop="TIMEF"\n", in->cumu_nekCG_beta_stop);
    TIMEPRINT1("NEKO_CGtimings: at_nekCG_beta_start="TIMEF"\n", in->at_nekCG_beta_start);
    TIMEPRINT1("NEKO_CGtimings: cumu_nekCG_beta_transit="TIMEF"\n", in->cumu_nekCG_beta_transit);
    TIMEPRINT1("NEKO_CGtimings: cumu_nekCG_axi_start="TIMEF"\n", in->cumu_nekCG_axi_start);
    TIMEPRINT1("NEKO_CGtimings: cumu_nekCG_axi_stop="TIMEF"\n", in->cumu_nekCG_axi_stop);
    TIMEPRINT1("NEKO_CGtimings: at_nekCG_axi_start="TIMEF"\n", in->at_nekCG_axi_start);
    TIMEPRINT1("NEKO_CGtimings: cumu_nekCG_axi_transit="TIMEF"\n", in->cumu_nekCG_axi_transit);
    TIMEPRINT1("NEKO_CGtimings: cumu_nekCG_alpha_start="TIMEF"\n", in->cumu_nekCG_alpha_start);
    TIMEPRINT1("NEKO_CGtimings: cumu_nekCG_alpha_stop="TIMEF"\n", in->cumu_nekCG_alpha_stop);
    TIMEPRINT1("NEKO_CGtimings: at_nekCG_alpha_start="TIMEF"\n", in->at_nekCG_alpha_start);
    TIMEPRINT1("NEKO_CGtimings: cumu_nekCG_alpha_transit="TIMEF"\n", in->cumu_nekCG_alpha_transit);
    TIMEPRINT1("NEKO_CGtimings: cumu_nekCG_rtr_start="TIMEF"\n", in->cumu_nekCG_rtr_start);
    TIMEPRINT1("NEKO_CGtimings: cumu_nekCG_rtr_stop="TIMEF"\n", in->cumu_nekCG_rtr_stop);
    TIMEPRINT1("NEKO_CGtimings: at_nekCG_rtr_start="TIMEF"\n", in->at_nekCG_rtr_start);
    TIMEPRINT1("NEKO_CGtimings: cumu_nekCG_rtr_transit="TIMEF"\n", in->cumu_nekCG_rtr_transit);
}

Err_t nekbone_setupTailRecusion(NEKOglobals_t * in_NEKOglobals,
                                NEKO_CGscalars_t * in_CGstats,
                                NEKO_CGscalars_t * io_CGstats)
{
    Err_t err=0;
    err = copy_NEKO_CGscalars(in_CGstats, io_CGstats);
    const TimeMark_t t = nekbone_getTime();
    io_CGstats->at_TailRecusionStart = t;
    TIMEPRINT2("NKTIME> rank=%u RankSetup="TIMEF"\n",
               in_NEKOglobals->rankID,
               io_CGstats->at_TailRecusionStart - in_NEKOglobals->startTimeMark);
    return err;
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
                           NEKO_CGscalars_t * io_CGscalars, NEKO_CGtimings_t * io_CGtimes)
{
    Err_t err=0;
    while(!err){
        nekbone_zero_variables(in_NEKOglobals->pDOF3DperR, io_X, io_W, io_P, io_Z);
        err = init_NEKO_CGscalars(io_CGscalars); IFEB;
        err = init_NEKO_CGtimings(io_CGtimes); IFEB;
        break;
    }
    return err;
}

Err_t nekbone_tailRecursionELSE(NEKO_CGtimings_t * io_CGtimes)
{
    Err_t err=0;
    while(!err){
        print_NEKO_CGtimings(io_CGtimes);
        err = destroy_NEKO_CGtimings(io_CGtimes); IFEB;
        break;
    }
    return err;
}

Err_t nekbone_tailTransitBegin(long in_currentIteration,
                               NEKO_CGscalars_t * in_CGstats, NEKO_CGscalars_t * io_CGstats,
                               NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes)
{
    Err_t err=0;
    while(!err){
        NEKO_CG_TIMFCN(TimeMark_t t0 = nekbone_getTime();)

        err = copy_NEKO_CGscalars(in_CGstats, io_CGstats); IFEB;
        io_CGstats->currentCGiteration = in_currentIteration;

        NEKO_CG_TIMFCN( err = copy_NEKO_CGtimings(in_CGtimes, io_CGtimes); IFEB; )
        NEKO_CG_TIMFCN( TimeMark_t t1 = nekbone_getTime() -t0; )
        NEKO_CG_TIMFCN( io_CGtimes->cumu_tailRecurTransitBEGIN += ((t1<0)?(0):(t1)); )

        break;
    }
    return err;
}

Err_t nekbone_solveMi(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                      NBN_REAL *in_R, NBN_REAL *io_Z,
                      NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes)
{
    Err_t err=0;
    while(!err){
        NEKO_CG_TIMFCN(TimeMark_t t0 = nekbone_getTime();)

        unsigned int i;
        for(i=0; i<in_NEKOglobals->pDOF3DperR; ++i){
            io_Z[i] = in_R[i];
        }

        NEKO_CG_TIMFCN( err = copy_NEKO_CGtimings(in_CGtimes, io_CGtimes); IFEB; )
        NEKO_CG_TIMFCN( TimeMark_t t1 = nekbone_getTime() -t0; )
        NEKO_CG_TIMFCN( io_CGtimes->cumu_nekCG_solveMi += ((t1<0)?(0):(t1)); )
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
                         NBN_REAL * o_partial_sum_rcz,
                         NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes
                         )
{
    NEKO_CGscalars_t * S = io_CGstats; //Just for convenience.

    Err_t err=0;
    while(!err){
        NEKO_CG_TIMFCN(TimeMark_t t0 = nekbone_getTime();)

        err = copy_NEKO_CGscalars(in_CGstats, S); IFEB;
        S->rtz2 = S->rtz1;

        *o_partial_sum_rcz = 0;
        *o_partial_sum_rcz = nekbone_localpart_glsc3i(in_NEKOglobals->pDOF3DperR, io_R, io_C, io_Z);

        NEKO_CG_TIMFCN( err = copy_NEKO_CGtimings(in_CGtimes, io_CGtimes); IFEB; )

        NEKO_CG_TIMFCN( io_CGtimes->at_nekCG_beta_start = nekbone_getTime(); )
        //TODO: nekbone_beta_start: DO the ALL_Reduce: call gop(*o_partial_sum_rcz, work,'+  ',1)

        NEKO_CG_TIMFCN( TimeMark_t t1 = nekbone_getTime(); )
        NEKO_CG_TIMFCN( io_CGtimes->cumu_nekCG_beta_start += ((t1-t0<0)?(0):(t1-t0)); )

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
                        NBN_REAL *in_P, NBN_REAL *io_Z, NBN_REAL *io_P,
                        NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes
                        )
{
    //OA.addCustomText(G, nc, '//SKE-NOTE:  This involves add2s1i(p,z,beta,...')
    //OA.addCustomText(G, nc, '//SKE-NOTE:  This involves the loop over all call ax_e')
    NEKO_CGscalars_t * S = io_CGstats; //Just for convenience.

    Err_t err=0;
    while(!err){
        NEKO_CG_TIMFCN(TimeMark_t t0 = nekbone_getTime();)
        //TODO: nekbone_beta_stop: Complete the ALL_Reduce: call gop(*o_partial_sum_rcz, work,'+  ',1)

        err = copy_NEKO_CGscalars(in_CGstats, S); IFEB;

        //At this point, in_sum_rcz is supposed to be rtz1, as per baseline code.
        S->rtz1 = *in_sum_rcz;
        S->beta = S->rtz1 / S->rtz2;

        if( 0 == S->currentCGiteration) S->beta=0.0;

        //call add2s1i(p,z,beta,n,find,lind)                              ! 2n
        nekbone_add2s1i(in_NEKOglobals->pDOF3DperR, in_P, io_Z, S->beta, io_P);

        NEKO_CG_TIMFCN( err = copy_NEKO_CGtimings(in_CGtimes, io_CGtimes); IFEB; )
        NEKO_CG_TIMFCN( TimeMark_t t1 = nekbone_getTime(); )
        NEKO_CG_TIMFCN( io_CGtimes->cumu_nekCG_beta_stop += ((t1-t0<0)?(0):(t1-t0)); )
        NEKO_CG_TIMFCN(t0 -= io_CGtimes->at_nekCG_beta_start;)
        NEKO_CG_TIMFCN( io_CGtimes->cumu_nekCG_beta_transit += ((t0<0)?(0):(t0)); )

        break;
    }

    S = 0;

    return err;
}

Err_t nekbone_alpha_start(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                          NBN_REAL *io_W, NBN_REAL *io_C, NBN_REAL *io_P,
                          NBN_REAL * o_partial_sum_pap,
                          NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes
                         )
{
    Err_t err=0;
    while(!err){
        NEKO_CG_TIMFCN(TimeMark_t t0 = nekbone_getTime();)

        //call glsc3i(pap, w,c,p,n,find,lind)
        *o_partial_sum_pap = nekbone_localpart_glsc3i(in_NEKOglobals->pDOF3DperR, io_W, io_C, io_P);

        NEKO_CG_TIMFCN( err = copy_NEKO_CGtimings(in_CGtimes, io_CGtimes); IFEB; )

        NEKO_CG_TIMFCN( io_CGtimes->at_nekCG_alpha_start = nekbone_getTime(); )
        //TODO: nekbone_alpha_start: DO the ALL_Reduce: call gop(*o_partial_sum_pap, work,'+  ',1)

        NEKO_CG_TIMFCN( TimeMark_t t1 = nekbone_getTime(); )
        NEKO_CG_TIMFCN( io_CGtimes->cumu_nekCG_alpha_start += ((t1-t0<0)?(0):(t1-t0)); )

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
                         NBN_REAL *in_R, NBN_REAL *in_W, NBN_REAL *io_R,
                         NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes
                         )
{
    NEKO_CGscalars_t * S = io_CGstats; //Just for convenience.
    Err_t err=0;
    while(!err){
        NEKO_CG_TIMFCN(TimeMark_t t0 = nekbone_getTime();)
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

        NEKO_CG_TIMFCN( err = copy_NEKO_CGtimings(in_CGtimes, io_CGtimes); IFEB; )
        NEKO_CG_TIMFCN( TimeMark_t t1 = nekbone_getTime(); )
        NEKO_CG_TIMFCN( io_CGtimes->cumu_nekCG_alpha_stop += ((t1-t0<0)?(0):(t1-t0)); )
        NEKO_CG_TIMFCN(t0 -= io_CGtimes->at_nekCG_alpha_start;)
        NEKO_CG_TIMFCN( io_CGtimes->cumu_nekCG_alpha_transit += ((t0<0)?(0):(t0)); )

        break;
    }
    S = 0;
    return err;
}

Err_t nekbone_rtr_start(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                        NBN_REAL *io_R, NBN_REAL *io_C, NBN_REAL * o_partial_sum_rtr,
                        NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes )
{
    Err_t err=0;
    while(!err){
        NEKO_CG_TIMFCN(TimeMark_t t0 = nekbone_getTime();)

        //call  glsc3i(rtr, r,c,r,n,find,lind)
        *o_partial_sum_rtr = nekbone_localpart_glsc3i(in_NEKOglobals->pDOF3DperR, io_R, io_C, io_R);

        NEKO_CG_TIMFCN( err = copy_NEKO_CGtimings(in_CGtimes, io_CGtimes); IFEB; )

        NEKO_CG_TIMFCN( io_CGtimes->at_nekCG_rtr_start = nekbone_getTime(); )
        //TODO: nekbone_rtr_start: DO the ALL_Reduce: call gop(*o_partial_sum_rtr, work,'+  ',1)

        NEKO_CG_TIMFCN( TimeMark_t t1 = nekbone_getTime(); )
        NEKO_CG_TIMFCN( io_CGtimes->cumu_nekCG_rtr_start += ((t1-t0<0)?(0):(t1-t0)); )

        break;
    }
    return err;
}

Err_t nekbone_rtr_stop(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                       NEKO_CGscalars_t * in_CGstats, NEKO_CGscalars_t * io_CGstats,
                       NBN_REAL * in_sum_rtr,
                       NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes)
{
    Err_t err=0;
    while(!err){
        NEKO_CG_TIMFCN(TimeMark_t t0 = nekbone_getTime();)

        //TODO: nekbone_rtr_stop: Process the ALL_Reduce: call gop(*o_partial_sum_rtr, work,'+  ',1)
        err = copy_NEKO_CGscalars(in_CGstats, io_CGstats); IFEB;

        io_CGstats->rtr = *in_sum_rtr;

        io_CGstats->rnorm = sqrt(*in_sum_rtr);

        NEKO_CG_TIMFCN( err = copy_NEKO_CGtimings(in_CGtimes, io_CGtimes); IFEB; )
        NEKO_CG_TIMFCN( TimeMark_t t1 = nekbone_getTime(); )
        NEKO_CG_TIMFCN( io_CGtimes->cumu_nekCG_rtr_stop += ((t1-t0<0)?(0):(t1-t0)); )
        NEKO_CG_TIMFCN(t0 -= io_CGtimes->at_nekCG_rtr_start;)
        NEKO_CG_TIMFCN( io_CGtimes->cumu_nekCG_rtr_transit += ((t0<0)?(0):(t0)); )

        break;
    }
    return err;
}

Err_t nekbone_ai_start(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                       NBN_REAL *io_W, NBN_REAL *io_P,
                       NBN_REAL * io_UR, NBN_REAL * io_US, NBN_REAL * io_UT,
                       NBN_REAL * in_G1, NBN_REAL * in_G4, NBN_REAL * in_G6,
                       NBN_REAL * in_dxm1, NBN_REAL * in_dxTm1,
                       NBN_REAL * io_AItemp,
                       NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes
                      )
{
   Err_t err=0;
    while(!err){
        NEKO_CG_TIMFCN(TimeMark_t t0 = nekbone_getTime();)

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

        NEKO_CG_TIMFCN( err = copy_NEKO_CGtimings(in_CGtimes, io_CGtimes); IFEB; )

        NEKO_CG_TIMFCN( io_CGtimes->at_nekCG_axi_start = nekbone_getTime(); )
        //TODO: nekbone_ai_start: Process Halo Exchange: call gs_op(gsh,w,1,1,0)

        NEKO_CG_TIMFCN( TimeMark_t t1 = nekbone_getTime(); )
        NEKO_CG_TIMFCN( io_CGtimes->cumu_nekCG_axi_start += ((t1-t0<0)?(0):(t1-t0)); )

        break;
    }
    return err;
}

Err_t nekbone_ai_stop(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                      NBN_REAL *in_W, NBN_REAL *in_P, NBN_REAL *io_W,
                      NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes)
{
   Err_t err=0;
    while(!err){
        NEKO_CG_TIMFCN(TimeMark_t t0 = nekbone_getTime();)
        //TODO: nekbone_ai_stop: Process Halo Exchange: call gs_op(gsh,w,1,1,0)

        //call add2s2i(w,u,.1,n,find,lind)
        const NBN_REAL fudge = 0.1;  //Yes.  This fudge factor is in Nekbone's baseline code.
        nekbone_add2s2i(in_NEKOglobals->pDOF3DperR, in_W, in_P, fudge, io_W);

        if(1 == NEKbone_find){
            nekbone_mask(in_NEKOglobals->rankID, io_W);
        }

        NEKO_CG_TIMFCN( err = copy_NEKO_CGtimings(in_CGtimes, io_CGtimes); IFEB; )
        NEKO_CG_TIMFCN( TimeMark_t t1 = nekbone_getTime(); )
        NEKO_CG_TIMFCN( io_CGtimes->cumu_nekCG_axi_stop += ((t1-t0<0)?(0):(t1-t0)); )
        NEKO_CG_TIMFCN(t0 -= io_CGtimes->at_nekCG_axi_start;)
        NEKO_CG_TIMFCN( io_CGtimes->cumu_nekCG_axi_transit += ((t0<0)?(0):(t0)); )

        break;
    }
    return err;
}

Err_t nekbone_tailRecurTransitEND(long in_current_iteration,
                                  unsigned int in_rankID,
                                  NEKO_CGscalars_t * in_CGstats,
                                  NEKO_CGscalars_t * io_CGstats)
{
    Err_t err=0;
    while(!err){
        err = copy_NEKO_CGscalars(in_CGstats, io_CGstats); IFEB;

        TimeMark_t t = nekbone_getTime();

        TIMEPRINT3("NKTIME> rank=%u iter=%ld SPDM_fork="TIMEF"\n",
                   in_rankID,
                   in_current_iteration,
                   t - in_CGstats->at_TailRecusionStart);

        io_CGstats->at_TailRecusionStart = t;

        break;
    }
    return err;
}

Err_t nekbone_BtForkTransition_Stop(NEKOglobals_t * in_NEKOglobals)
{
    Err_t err=0;
    while(!err){
        TimeMark_t t = nekbone_getTime();

        TIMEPRINT2("NKTIME> rank=%u TotalRank="TIMEF"\n",
                   in_NEKOglobals->rankID,
                   t - in_NEKOglobals->startTimeMark);
        break;
    }
    return err;
}



















