#ifndef NEKBONE_CG_H
#include "nekbone_cg.h"
#endif

#include "reduction.h"
#include "app_ocr_util.h" //For debugging reduction

#include "blas3.h"
#include "ax.h"

#ifdef NEK_USE_ADVANCED_FUNCTIONS
#   include <math.h> //For sqrt
#endif

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
#   ifdef NEKO_get_CGLOOP
        io_CGstats->at_TailRecusionStart = t;
#   endif
    TIMEPRINT2("NKTIME> rank=%u RankSetup="TIMEF"\n",
               in_NEKOglobals->rankID,
               t - in_NEKOglobals->startTimeMark);
    return err;
}

void nekbone_mask(unsigned int in_rankID, NBN_REAL * io)
{
    if(0 == in_rankID){
        io[0] = 0; // suitable for solvability
    }
}

Err_t nekbone_CGstep0_start(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                            NBN_REAL * in_F, NBN_REAL * in_C, NBN_REAL * io_R,
                            ocrGuid_t io_reducPrivateGuid, reductionPrivate_t * io_reducPrivate,
                            unsigned int in_destSlot, ocrGuid_t in_destinationGuid
                           )
{
    Err_t err=0;
    while(!err){
        const unsigned int length = in_NEKOglobals->pDOF3DperR;

        unsigned int i;
        for(i=0; i< length; ++i){
            io_R[i] = in_F[i];
        }

        if(NEKbone_thread == 0){
            nekbone_mask(in_NEKOglobals->rankID, io_R);
        }

        ReducSum_t sum = 0;
        for(i=0; i<length; ++i){
            sum += io_R[i] * in_C[i] * io_R[i];
        }
        //PRINTF("DBG> rank=%u> CGstep0_start> rnorminit = %24.14E\n", in_NEKOglobals->rankID, sum);

        //Calculate glsc3i(rnorminit,r,c,r,n,find,lind)
#   ifdef REDUCTION_CGSTEP0
        err = ocrAddDependence(io_reducPrivate->returnEVT, in_destinationGuid, in_destSlot, DB_MODE_RO); IFEB;
        reductionLaunch(io_reducPrivate, io_reducPrivateGuid, &sum);
#   else
        ocrGuid_t gd_sum= NULL_GUID;
        ReducSum_t * o_sum=NULL;
        err = ocrDbCreate( &gd_sum, (void**)&o_sum, 1*sizeof(ReducSum_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC); IFEB;
        *o_sum = sum;
        err = ocrDbRelease(gd_sum); IFEB;
        err = ocrXHookup(OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG, in_destinationGuid, in_destSlot, DB_MODE_RO, gd_sum); IFEB;
        err = ocrDbRelease(io_reducPrivateGuid); IFEB; //This is to imitate the release done by reductionLaunch().
#   endif

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
                           NEKO_CGscalars_t * io_CGscalars, NEKO_CGtimings_t * io_CGtimes,
                           ocrGuid_t in_sum_guid, ReducSum_t * in_sum)
{
    Err_t err=0;
    while(!err){
        nekbone_zero_variables(in_NEKOglobals->pDOF3DperR, io_X, io_W, io_P, io_Z);
        err = init_NEKO_CGscalars(io_CGscalars); IFEB;
        err = init_NEKO_CGtimings(io_CGtimes); IFEB;

        //Taking care of completing glsc3i(rnorminit,r,c,r,n,find,lind)
        double rnorminit = *in_sum;
        //PRINTF("DBG> rank=%u> CGstep0_stop> rnorminit = %24.14E\n", in_NEKOglobals->rankID, rnorminit);

        if( rnorminit < 0) {
            //This should be an error.  But rnorminit is used by no one.
            //So keep this as a warning for now.
            PRINTF("CGstep0> rank=%u> rnorminit is negative\n", in_NEKOglobals->rankID);
            rnorminit = -rnorminit;
        }
#       ifdef NEK_USE_ADVANCED_FUNCTIONS
            rnorminit = sqrt(rnorminit);
            if(in_NEKOglobals->rankID == 0){
                PRINTF("INFO> CGstep0_stop> rnorminit = %24.14E\n", rnorminit);
            }
#       else
            if(in_NEKOglobals->rankID == 0){
                PRINTF("INFO> CGstep0_stop> rnorminit^2 = %24.14E\n", rnorminit);
            }
#       endif
        err = ocrDbDestroy( in_sum_guid ); IFEB;

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
                         NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes,
                         ocrGuid_t io_reducPrivateGuid, reductionPrivate_t * io_reducPrivate,
                         unsigned int in_destSlot, ocrGuid_t in_destinationGuid
                         )
{
    NEKO_CGscalars_t * S = io_CGstats; //Just for convenience.

    Err_t err=0;
    while(!err){
        NEKO_CG_TIMFCN(TimeMark_t t0 = nekbone_getTime();)

        err = copy_NEKO_CGscalars(in_CGstats, S); IFEB;
        S->rtz2 = S->rtz1;

        double sum=0;
        sum = nekbone_localpart_glsc3i(in_NEKOglobals->pDOF3DperR, io_R, io_C, io_Z);

        NEKO_CG_TIMFCN( err = copy_NEKO_CGtimings(in_CGtimes, io_CGtimes); IFEB; )

        NEKO_CG_TIMFCN( io_CGtimes->at_nekCG_beta_start = nekbone_getTime(); )
        //nekbone_beta_start: DO the ALL_Reduce: call gop(sum, work,'+  ',1)
#   ifdef REDUCTION_BETA
        err = ocrAddDependence(io_reducPrivate->returnEVT, in_destinationGuid, in_destSlot, DB_MODE_RO); IFEB;
        reductionLaunch(io_reducPrivate, io_reducPrivateGuid, &sum);
#   else
        ocrGuid_t gd_sum= NULL_GUID;
        ReducSum_t * o_sum=NULL;
        err = ocrDbCreate( &gd_sum, (void**)&o_sum, 1*sizeof(ReducSum_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC); IFEB;
        *o_sum = sum;
        err = ocrDbRelease(gd_sum); IFEB;
        err = ocrXHookup(OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG, in_destinationGuid, in_destSlot, DB_MODE_RO, gd_sum); IFEB;
        err = ocrDbRelease(io_reducPrivateGuid); IFEB; //This is to imitate the release done by reductionLaunch().
#   endif

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
                        NBN_REAL *in_P, NBN_REAL *io_Z, NBN_REAL *io_P,
                        NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes,
                        ocrGuid_t in_sum_guid, ReducSum_t * in_sum
                        )
{
    //OA.addCustomText(G, nc, '//SKE-NOTE:  This involves add2s1i(p,z,beta,...')
    //OA.addCustomText(G, nc, '//SKE-NOTE:  This involves the loop over all call ax_e')
    NEKO_CGscalars_t * S = io_CGstats; //Just for convenience.

    Err_t err=0;
    while(!err){
        NEKO_CG_TIMFCN(TimeMark_t t0 = nekbone_getTime();)
        //Complete the ALL_Reduce: call gop(sum, work,'+  ',1)
        NBN_REAL sum = *in_sum;
        if(in_NEKOglobals->rankID == 0){
            //DBG> PRINTF("INFO> beta_stop> sum = %24.14E\n", sum);
        }
        err = ocrDbDestroy( in_sum_guid ); IFEB;

        err = copy_NEKO_CGscalars(in_CGstats, S); IFEB;

        //At this point, sum is supposed to be rtz1, as per baseline code.
        S->rtz1 = sum;
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
                          NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes,
                          ocrGuid_t io_reducPrivateGuid, reductionPrivate_t * io_reducPrivate,
                          unsigned int in_destSlot, ocrGuid_t in_destinationGuid
                         )
{
    Err_t err=0;
    while(!err){
        NEKO_CG_TIMFCN(TimeMark_t t0 = nekbone_getTime();)

        //call glsc3i(pap, w,c,p,n,find,lind)
        NBN_REAL lsum = nekbone_localpart_glsc3i(in_NEKOglobals->pDOF3DperR, io_W, io_C, io_P);
        ReducSum_t sum = lsum;

        NEKO_CG_TIMFCN( err = copy_NEKO_CGtimings(in_CGtimes, io_CGtimes); IFEB; )

        NEKO_CG_TIMFCN( io_CGtimes->at_nekCG_alpha_start = nekbone_getTime(); )

        //nekbone_alpha_start: DO the ALL_Reduce: call gop(sum, work,'+  ',1)
#   ifdef REDUCTION_ALPHA
        err = ocrAddDependence(io_reducPrivate->returnEVT, in_destinationGuid, in_destSlot, DB_MODE_RO); IFEB;
        reductionLaunch(io_reducPrivate, io_reducPrivateGuid, &sum);
#   else
        ocrGuid_t gd_sum= NULL_GUID;
        ReducSum_t * o_sum=NULL;
        err = ocrDbCreate( &gd_sum, (void**)&o_sum, 1*sizeof(ReducSum_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC); IFEB;
        *o_sum = sum;
        err = ocrDbRelease(gd_sum); IFEB;
        err = ocrXHookup(OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG, in_destinationGuid, in_destSlot, DB_MODE_RO, gd_sum); IFEB;
        err = ocrDbRelease(io_reducPrivateGuid); IFEB; //This is to imitate the release done by reductionLaunch().
#   endif

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
void nekbone_add2s2i_forW(unsigned int in_length, NBN_REAL *in_B,
                          NBN_REAL in_C1, NBN_REAL * io_A)
{
    unsigned int i;
    for(i=0; i < in_length; ++i){
        //Used to be--> io_A[i] = in_A[i] + in_C1 * in_B[i];
        io_A[i] += in_C1 * in_B[i];
    }
}


Err_t nekbone_alpha_stop(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                         NEKO_CGscalars_t * in_CGstats, NEKO_CGscalars_t * io_CGstats,
                         NBN_REAL *in_X, NBN_REAL *in_P, NBN_REAL *io_X,
                         NBN_REAL *in_R, NBN_REAL *in_W, NBN_REAL *io_R,
                         NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes,
                         ocrGuid_t in_sum_guid, ReducSum_t * in_sum
                         )
{
    NEKO_CGscalars_t * S = io_CGstats; //Just for convenience.
    Err_t err=0;
    while(!err){
        NEKO_CG_TIMFCN(TimeMark_t t0 = nekbone_getTime();)

        err = copy_NEKO_CGscalars(in_CGstats, S); IFEB;

        //nekbone_alpha_stop: Post-process the ALL_Reduce: call gop(*o_partial_sum_pap, work,'+  ',1)
        NBN_REAL sum = *in_sum;
        if(in_NEKOglobals->rankID == 0){
            //DBG> PRINTF("INFO> alpha_stop> sum = %24.14E\n", sum);
        }
        err = ocrDbDestroy( in_sum_guid ); IFEB;

        //At this point, sum is supposed to be pap, as per baseline code.
        S->pap = sum;

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
                        NBN_REAL *io_R, NBN_REAL *io_C,
                        NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes,
                        ocrGuid_t io_reducPrivateGuid, reductionPrivate_t * io_reducPrivate,
                        unsigned int in_destSlot, ocrGuid_t in_destinationGuid)
{
    Err_t err=0;
    while(!err){
        NEKO_CG_TIMFCN(TimeMark_t t0 = nekbone_getTime();)

        //call  glsc3i(rtr, r,c,r,n,find,lind)
        NBN_REAL lsum = nekbone_localpart_glsc3i(in_NEKOglobals->pDOF3DperR, io_R, io_C, io_R);
        ReducSum_t sum = lsum;

        NEKO_CG_TIMFCN( err = copy_NEKO_CGtimings(in_CGtimes, io_CGtimes); IFEB; )

        NEKO_CG_TIMFCN( io_CGtimes->at_nekCG_rtr_start = nekbone_getTime(); )
        //nekbone_rtr_start: DO the ALL_Reduce: call gop(sum, work,'+  ',1)
#   ifdef REDUCTION_RTR
        err = ocrAddDependence(io_reducPrivate->returnEVT, in_destinationGuid, in_destSlot, DB_MODE_RO); IFEB;
        reductionLaunch(io_reducPrivate, io_reducPrivateGuid, &sum);
#   else
        ocrGuid_t gd_sum= NULL_GUID;
        ReducSum_t * o_sum=NULL;
        err = ocrDbCreate( &gd_sum, (void**)&o_sum, 1*sizeof(ReducSum_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC); IFEB;
        *o_sum = sum;
        err = ocrDbRelease(gd_sum); IFEB;
        err = ocrXHookup(OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG, in_destinationGuid, in_destSlot, DB_MODE_RO, gd_sum); IFEB;
        err = ocrDbRelease(io_reducPrivateGuid); IFEB; //This is to imitate the release done by reductionLaunch().
#   endif

        NEKO_CG_TIMFCN( TimeMark_t t1 = nekbone_getTime(); )
        NEKO_CG_TIMFCN( io_CGtimes->cumu_nekCG_rtr_start += ((t1-t0<0)?(0):(t1-t0)); )

        break;
    }
    return err;
}

Err_t nekbone_rtr_stop(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                       NEKO_CGscalars_t * in_CGstats, NEKO_CGscalars_t * io_CGstats,
                       NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes,
                       ocrGuid_t in_sum_guid, ReducSum_t * in_sum)
{
    Err_t err=0;
    while(!err){
        NEKO_CG_TIMFCN(TimeMark_t t0 = nekbone_getTime();)

        err = copy_NEKO_CGscalars(in_CGstats, io_CGstats); IFEB;

        //nekbone_rtr_stop: Process the ALL_Reduce: call gop(sum, work,'+  ',1)
        NBN_REAL sum = *in_sum;
        if(in_NEKOglobals->rankID == 0){
            //DBG> PRINTF("INFO> rtr_stop> sum = %24.14E\n", sum);
        }
        err = ocrDbDestroy( in_sum_guid ); IFEB;

        io_CGstats->rtr = sum;
#       ifdef NEK_USE_ADVANCED_FUNCTIONS
            io_CGstats->rnorm = sqrt(sum);
#       else
            io_CGstats->rnorm = sum;
#       endif

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
        //cg.f::axi: call gs_op(gsh,w,1,1,0)  ! Gather-scatter operation  ! w   = QQ  w
        //See neko_halo.h::start_halo_ai and neko_halo.h::stop_halo_ai
        // for how that is handled.

        NEKO_CG_TIMFCN( TimeMark_t t1 = nekbone_getTime(); )
        NEKO_CG_TIMFCN( io_CGtimes->cumu_nekCG_axi_start += ((t1-t0<0)?(0):(t1-t0)); )

        break;
    }
    return err;
}

Err_t nekbone_ai_stop(NEKOstatics_t * in_NEKOstatics, NEKOglobals_t * in_NEKOglobals,
                      NBN_REAL *in_P, NBN_REAL *io_W,
                      NEKO_CGtimings_t * in_CGtimes, NEKO_CGtimings_t * io_CGtimes)
{
   Err_t err=0;
    while(!err){
        NEKO_CG_TIMFCN(TimeMark_t t0 = nekbone_getTime();)
        //cg.f::axi: handle what came out of call gs_op(gsh,w,1,1,0)  ! Gather-scatter operation  ! w   = QQ  w
        //See neko_halo.h::stop_halo_ai for details

        //call add2s2i(w,u,.1,n,find,lind)
        const NBN_REAL fudge = 0.1;  //Yes.  This fudge factor is in Nekbone's baseline code.
        nekbone_add2s2i_forW(in_NEKOglobals->pDOF3DperR, in_P, fudge, io_W);

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

#       ifdef NEKO_get_CGLOOP
            TIMEPRINT3("NKTIME> rank=%u iter=%ld CGloop="TIMEF"\n",
                       in_rankID,
                       in_current_iteration,
                       t - in_CGstats->at_TailRecusionStart);
            io_CGstats->at_TailRecusionStart = t;
#       endif

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



















