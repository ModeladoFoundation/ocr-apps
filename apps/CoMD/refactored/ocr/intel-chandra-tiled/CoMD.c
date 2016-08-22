/*
Author Chandra S. Martha
Copywrite Intel Corporation 2015

 This file is subject to the license agreement located in the file ../../../../LICENSE (apps/LICENSE)
 and cannot be distributed without it. This notice cannot be removed or modified.

*/

#define ENABLE_EXTENSION_LABELING
#define ENABLE_EXTENSION_AFFINITY

#include "ocr.h"
#include "extensions/ocr-labeling.h" //currently needed for labeled guids
#include "extensions/ocr-affinity.h" //needed for affinity
#ifdef USE_PROFILER
#include "extensions/ocr-profiler.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <assert.h>

#include "CoMDTypes.h"
//#include "decomposition.h"
//#include "linkCells.h"
//#include "eam.h"
//#include "ljForce.h"
//#include "initAtoms.h"
//#include "memUtils.h"
//#include "yamlOutput.h"
//#include "parallel.h"
//#include "performanceTimers.h"
#include "mycommand.h"
//#include "timestep.h"
#include "constants.h"

#include "SPMDappUtils.h"

#if 0

_OCR_TASK_FNC_( FNC_Lsend )
{
    ocrGuid_t DBK_Lsend = depv[2].guid;

    rankH_t* PTR_rankH = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = &(PTR_rankH->rankParamH);
    double* xIn = (double*) depv[1].ptr;
    double* lsend = (double*) depv[2].ptr;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_x = (s64) PTR_rankParamH->id_x;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;
    s64 np_x = (s64) PTR_rankParamH->np_x;

    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;

    s64 np_y = (s64) PTR_rankParamH->np_y;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

#if FULL_APP==1
    int i, j;
    int kk = 0;
    for( j = jb; j <= je; j++ )
        for( i = ib; i < ib+HALO_RADIUS; i++ )
        {
            lsend[kk++] = IN(i,j);
        }
#endif

    if( id_x != 0 )
    {
        //PRINTF("ID=%d FNC_Lsend lsend guid is %lu\n", id, DBK_Lsend);
        return DBK_Lsend;
    }
    else
    {
        //PRINTF("ID=%d FNC_Lsend lsend guid is NULL\n", id);
        return NULL_GUID;
    }

}

_OCR_TASK_FNC_( FNC_Rsend )
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[1].guid;
    ocrGuid_t DBK_Rsend = depv[2].guid;

    rankH_t* PTR_rankH = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = &(PTR_rankH->rankParamH);
    double* xIn = (double*) depv[1].ptr;
    double* rsend = (double*) depv[2].ptr;

    globalCmdParamH_t* PTR_globalParamH = &(PTR_rankH->globalParamH.cmdParamH);
    s64 NR_X = PTR_globalParamH->NR_X;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_x = (s64) PTR_rankParamH->id_x;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;
    s64 np_x = (s64) PTR_rankParamH->np_x;

    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;

    s64 np_y = (s64) PTR_rankParamH->np_y;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

#if FULL_APP==1
    int i, j;
    int kk = 0;
    for( j = jb; j <= je; j++ )
        for( i = ie-HALO_RADIUS+1; i <= ie; i++ )
        {
            rsend[kk++] = IN(i,j);
        }
#endif

    if( id_x != NR_X - 1 )
    {
        //PRINTF("ID=%d FNC_Rsend rsend guid is %lu\n", id, DBK_Rsend);
        return DBK_Rsend;
    }
    else
    {
        //PRINTF("ID=%d FNC_Lsend lsend guid is NULL\n", id);
        return NULL_GUID;
    }
}

_OCR_TASK_FNC_( FNC_Lrecv )
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[1].guid;
    ocrGuid_t DBK_Rsend = (ocrGuid_t) depv[2].guid;

    rankH_t* PTR_rankH = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = &(PTR_rankH->rankParamH);
    double* xIn = (double*) depv[1].ptr;
    double* rsent = (double*) depv[2].ptr;

    globalCmdParamH_t* PTR_globalParamH = &(PTR_rankH->globalParamH.cmdParamH);
    s64 NR_X = PTR_globalParamH->NR_X;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_x = (s64) PTR_rankParamH->id_x;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;
    s64 np_x = (s64) PTR_rankParamH->np_x;

    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;

    s64 np_y = (s64) PTR_rankParamH->np_y;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

#if FULL_APP==1
    int i, j;
    if( !IS_GUID_NULL(DBK_Rsend) || id_x != 0 )
    {
        int kk = 0;
        for( j = jb; j <= je; j++ )
            for( i = ib-HALO_RADIUS; i < ib; i++ )
            {
                IN(i,j) = rsent[kk++];
            }
        //PRINTF("ID=%d FNC_Lrecv lrecv guid is %lu\n", id, DBK_Rsend);
    }
#endif

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_Rrecv )
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[1].guid;
    ocrGuid_t DBK_Lsend = (ocrGuid_t) depv[2].guid;

    rankH_t* PTR_rankH = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = &(PTR_rankH->rankParamH);
    double* xIn = (double*) depv[1].ptr;
    double* lsent = (double*) depv[2].ptr;

    globalCmdParamH_t* PTR_globalParamH = &(PTR_rankH->globalParamH.cmdParamH);
    s64 NR_X = PTR_globalParamH->NR_X;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_x = (s64) PTR_rankParamH->id_x;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;
    s64 np_x = (s64) PTR_rankParamH->np_x;

    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;

    s64 np_y = (s64) PTR_rankParamH->np_y;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

#if FULL_APP==1
    int i, j;
    if( !IS_GUID_NULL(DBK_Lsend) || id_x != NR_X - 1)
    {
        int kk = 0;
        for( j = jb; j <= je; j++ )
            for( i = ie + 1; i < ie + 1 + HALO_RADIUS; i++ )
            {
                IN(i,j) = lsent[kk++];
            }

        //PRINTF("ID=%d FNC_Rrecv rrecv guid is %lu\n", id, DBK_Lsend);
    }
#endif

    return NULL_GUID;
}

//y-direction
_OCR_TASK_FNC_( FNC_Bsend )
{
    ocrGuid_t DBK_Bsend = (ocrGuid_t) depv[2].guid;

    rankH_t* PTR_rankH = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = &(PTR_rankH->rankParamH);
    double* xIn = (double*) depv[1].ptr;
    double* bsend = (double*) depv[2].ptr;

    globalCmdParamH_t* PTR_globalParamH = &(PTR_rankH->globalParamH.cmdParamH);
    s64 NR_X = PTR_globalParamH->NR_X;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_y = (s64) PTR_rankParamH->id_y;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;
    s64 np_x = (s64) PTR_rankParamH->np_x;
    s64 np_y = (s64) PTR_rankParamH->np_y;

    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

#if FULL_APP==1
    int i, j;
    int kk = 0;
    for( j = jb; j < jb+HALO_RADIUS; j++ )
        for( i = ib; i <= ie; i++ )
        {
            bsend[kk++] = IN(i,j);
        }
#endif

    if( id_y != 0 )
    {
        //PRINTF("ID=%d FNC_Bsend bsend guid is %lu\n", id, DBK_Bsend);
        return DBK_Bsend;
    }
    else
    {
        //PRINTF("ID=%d FNC_Bsend bsend guid is NULL\n", id);
        return NULL_GUID;
    }

}

_OCR_TASK_FNC_( FNC_Tsend )
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[1].guid;
    ocrGuid_t DBK_Tsend = (ocrGuid_t) depv[2].guid;

    rankH_t* PTR_rankH = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = &(PTR_rankH->rankParamH);
    double* xIn = (double*) depv[1].ptr;
    double* tsend = (double*) depv[2].ptr;

    globalCmdParamH_t* PTR_globalParamH = &(PTR_rankH->globalParamH.cmdParamH);
    s64 NR_Y = PTR_globalParamH->NR_Y;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_y = (s64) PTR_rankParamH->id_y;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;
    s64 np_x = (s64) PTR_rankParamH->np_x;
    s64 np_y = (s64) PTR_rankParamH->np_y;

    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

#if FULL_APP==1
    int i, j;
    int kk = 0;
    for( j = je - HALO_RADIUS + 1; j <= je; j++ )
        for( i = ib; i <= ie; i++ )
        {
            tsend[kk++] = IN(i,j);
        }
#endif

    if( id_y != NR_Y - 1 )
    {
        //PRINTF("ID=%d FNC_Tsend tsend guid is %lu\n", id, DBK_Tsend);
        return DBK_Tsend;
    }
    else
    {
        //PRINTF("ID=%d FNC_Bsend bsend guid is NULL\n", id);
        return NULL_GUID;
    }

}

_OCR_TASK_FNC_( FNC_Brecv )
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[1].guid;
    ocrGuid_t DBK_Tsend = (ocrGuid_t) depv[2].guid;

    rankH_t* PTR_rankH = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = &(PTR_rankH->rankParamH);
    double* xIn = (double*) depv[1].ptr;
    double* tsent = (double*) depv[2].ptr;

    globalCmdParamH_t* PTR_globalParamH = &(PTR_rankH->globalParamH.cmdParamH);
    s64 NR_Y = PTR_globalParamH->NR_Y;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_y = (s64) PTR_rankParamH->id_y;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;
    s64 np_x = (s64) PTR_rankParamH->np_x;
    s64 np_y = (s64) PTR_rankParamH->np_y;

    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

#if FULL_APP==1
    int i, j;
    if( !IS_GUID_NULL(DBK_Tsend) || id_y != 0 )
    {
        int kk = 0;
        for( j = jb-HALO_RADIUS; j < jb; j++ )
            for( i = ib; i <= ie; i++ )
            {
                IN(i,j) = tsent[kk++];
            }

        //PRINTF("ID=%d FNC_Brecv lrecv guid is %lu\n", id, DBK_Tsend);
    }
#endif

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_Trecv )
{
    ocrGuid_t DBK_xIn = (ocrGuid_t) depv[1].guid;
    ocrGuid_t DBK_Bsend = (ocrGuid_t) depv[2].guid;

    rankH_t* PTR_rankH = depv[0].ptr;
    rankParamH_t* PTR_rankParamH = &(PTR_rankH->rankParamH);
    double* xIn = (double*) depv[1].ptr;
    double* bsent = (double*) depv[2].ptr;

    globalCmdParamH_t* PTR_globalParamH = &(PTR_rankH->globalParamH.cmdParamH);
    s64 NR_Y = PTR_globalParamH->NR_Y;

    s64 id = (s64) PTR_rankParamH->id;
    s64 id_y = (s64) PTR_rankParamH->id_y;
    s64 itimestep = (s64) PTR_rankParamH->itimestep;
    s64 np_x = (s64) PTR_rankParamH->np_x;
    s64 np_y = (s64) PTR_rankParamH->np_y;

    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);

#if FULL_APP==1
    int i, j;
    if( !IS_GUID_NULL(DBK_Bsend) || id_y != NR_Y - 1)
    {
        int kk = 0;
        for( j = je + 1; j <= je + HALO_RADIUS; j++ )
            for( i = ib; i <= ie; i++ )
            {
                IN(i,j) = bsent[kk++];
            }

        //PRINTF("ID=%d FNC_Trecv rrecv guid is %lu\n", id, DBK_Bsend);
    }
#endif

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_update )
{
#ifdef USE_PROFILER
    START_PROFILE( app_FNC_update );
#endif
    s32 itimestep = paramv[0];
    u32 _paramc, _depc, _idep;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid;
    ocrGuid_t DBK_xIn = depv[_idep++].guid;
    ocrGuid_t DBK_xOut = depv[_idep++].guid;
    ocrGuid_t DBK_weight = depv[_idep++].guid;
    ocrGuid_t DBK_refNorm = depv[_idep++].guid;
    ocrGuid_t DBK_timers = depv[_idep++].guid;
    ocrGuid_t DBK_norm_reductionH = depv[_idep++].guid;
    ocrGuid_t DBK_timer_reductionH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    double *restrict xIn = depv[_idep++].ptr;
    double *restrict xOut = depv[_idep++].ptr;
    double *restrict weight = depv[_idep++].ptr;
    double *refNorm = depv[_idep++].ptr;
    timer* PTR_timers = depv[_idep++].ptr;
    reductionPrivate_t* PTR_norm_reductionH = depv[_idep++].ptr;
    reductionPrivate_t* PTR_timer_reductionH = depv[_idep++].ptr;

    rankParamH_t* PTR_rankParamH = &(PTR_rankH->rankParamH);
    globalCmdParamH_t* PTR_globalParamH = &(PTR_rankH->globalParamH.cmdParamH);
    globalOcrParamH_t* PTR_globalOcrParamH = &(PTR_rankH->globalParamH.ocrParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    s64 NR = PTR_globalParamH->NR;
    s64 NP_X = PTR_globalParamH->NP_X;

    s64 id = (s64) PTR_rankParamH->id;
    s64 np_x = (s64) PTR_rankParamH->np_x;
    s64 ib = (s64) PTR_rankParamH->ib;
    s64 ie = (s64) PTR_rankParamH->ie;

    s64 NP_Y = PTR_globalParamH->NP_Y;
    s64 np_y = (s64) PTR_rankParamH->np_y;
    s64 jb = (s64) PTR_rankParamH->jb;
    s64 je = (s64) PTR_rankParamH->je;

    //PRINTF("ID:%d %s timestep %d\n", id, __func__, itimestep);
    //PRINTF("ID: %d ib %d ie %d jb %d je %d\n", id, ib, ie, jb, je);

    /* Apply the stencil operator */
#if FULL_APP==1
    //HALO_RADIUS is hard-coded for better compiler optimzation here
    int i, j, ii, jj;
    for (j=MAX(jb,HALO_RADIUS); j<=MIN(NP_Y-HALO_RADIUS-1,je); j++)
    {
        for (i=MAX(ib,HALO_RADIUS); i<=MIN(NP_X-HALO_RADIUS-1,ie); i++)
        {
            for (jj=-HALO_RADIUS; jj<=HALO_RADIUS; jj++) {
                OUT(i,j) += WEIGHT(0,jj)*IN(i,j+jj);
            }
            for (ii=-HALO_RADIUS; ii<0; ii++) {
                OUT(i,j) += WEIGHT(ii,0)*IN(i+ii,j);
            }
            for (ii=1; ii<=HALO_RADIUS; ii++) {
                OUT(i,j) += WEIGHT(ii,0)*IN(i+ii,j);
            }
        }
    }

    /* add constant to solution to force refresh of neighbor data, if any */
    for (j=jb; j<=je; j++) for (i=ib; i<=ie; i++) IN(i,j)+= 1.0;
#endif

    //PTR_rankParamH->itimestep = itimestep + 1; TODO

    if( itimestep == PTR_globalParamH->NT )
    {
#if FULL_APP==1

        for (j=MAX(jb,HALO_RADIUS); j<=MIN(NP_Y-HALO_RADIUS-1,je); j++)
            for (i=MAX(ib,HALO_RADIUS); i<=MIN(NP_X-HALO_RADIUS-1,ie); i++)
                refNorm[0] += (double) ABS(OUT(i,j));
#endif
        ocrDbRelease(DBK_refNorm);

        profile_stop( total_timer, PTR_timers );
        ocrDbRelease(DBK_timers);

        reductionLaunch(PTR_norm_reductionH, DBK_norm_reductionH, refNorm);

        double stencil_time = get_elapsed_time( total_timer, PTR_timers );
        //PRINTF("time %f %f\n", stencil_time, refNorm[0]);
        reductionLaunch(PTR_timer_reductionH, DBK_timer_reductionH, &stencil_time);
    }

#ifdef USE_PROFILER
    RETURN_PROFILE(NULL_GUID);
#endif

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_summary )
{
    ocrGuid_t DBK_norm = depv[2].guid;

    rankH_t *PTR_rankH = depv[0].ptr;

    globalCmdParamH_t* PTR_globalParamH = &(PTR_rankH->globalParamH.cmdParamH);
    double* stencil_time = depv[1].ptr;
    double* PTR_norm = depv[2].ptr;

    s64 NT = PTR_globalParamH->NT;
    s64 NR = PTR_globalParamH->NR;
    s64 NP_X = PTR_globalParamH->NP_X;

    s64 NP_Y = PTR_globalParamH->NP_Y;
    double f_active_points = (double) ( (NP_X - 2*HALO_RADIUS) * (NP_Y - 2*HALO_RADIUS) );
    double reference_norm = (double) (NT+1)*2;
    int stencil_size = 4*HALO_RADIUS + 1;

#if FULL_APP==1
    PTR_norm[0] /= f_active_points;

    if( ABS( PTR_norm[0] - reference_norm ) > EPSILON )
        PRINTF( "ERROR: L1 norm = %f, Reference L1 norm = %f\n", PTR_norm[0], reference_norm);
    else
        PRINTF( "Solution validates\n" );
#endif

    double avgtime = stencil_time[0]/(double)NT;

    double flops = (double) (2*stencil_size+1) * f_active_points;
    PRINTF("Rate (MFlops/s): %f  Avg time (s): %f\n",
           1.0E-06 * flops/avgtime, avgtime);

    ocrShutdown();

    return NULL_GUID;
}

_OCR_TASK_FNC_( timestepEdt )
{
    s32 itimestep = paramv[0];

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid;
    ocrGuid_t DBK_xIn = depv[_idep++].guid;
    ocrGuid_t DBK_xOut = depv[_idep++].guid;
    ocrGuid_t DBK_weight = depv[_idep++].guid;
    ocrGuid_t DBK_refNorm = depv[_idep++].guid;
    ocrGuid_t DBK_timers = depv[_idep++].guid;
    ocrGuid_t DBK_norm_reductionH = depv[_idep++].guid;
    ocrGuid_t DBK_timer_reductionH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    double *xIn = depv[_idep++].ptr;
    double *xOut = depv[_idep++].ptr;
    double *weight = depv[_idep++].ptr;
    double *refNorm = depv[_idep++].ptr;
    timer* PTR_timers = depv[_idep++].ptr;
    reductionPrivate_t* PTR_norm_reductionH = depv[_idep++].ptr;
    reductionPrivate_t* PTR_timer_reductionH = depv[_idep++].ptr;

    rankParamH_t* PTR_rankParamH = &(PTR_rankH->rankParamH);
    globalCmdParamH_t* PTR_globalParamH = &(PTR_rankH->globalParamH.cmdParamH);
    globalOcrParamH_t* PTR_globalOcrParamH = &(PTR_rankH->globalParamH.ocrParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    s64 id_x = PTR_rankParamH->id_x;
    s64 NR_X = PTR_globalParamH->NR_X;

    s64 id_y = PTR_rankParamH->id_y;
    s64 NR_Y = PTR_globalParamH->NR_Y;

    s64 NT = PTR_globalParamH->NT;

    s64 id = PTR_rankParamH->id;

    s64 phase = itimestep%2;

    //PRINTF("%s id %d x %d y %d\n", __func__, id, id_x, id_y);

    if(itimestep==1) //Do not time iteration 0
    {
        profile_start( total_timer, PTR_timers );
        //PRINTF("Starting profile\n");
        ocrDbRelease( DBK_timers );
    }

    ocrHint_t myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;
    ocrHint_t myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;

    MyOcrTaskStruct_t TS_Lsend; _paramc = 0; _depc = 3;
    ocrGuid_t EVT_Lsend_fin;
    ocrEventCreate( &EVT_Lsend_fin, OCR_EVENT_STICKY_T, false );

    TS_Lsend.TML = PTR_rankTemplateH->TML_FNC_Lsend;
    ocrEdtCreate( &TS_Lsend.EDT, TS_Lsend.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &TS_Lsend.OET);

    if( id_x!=0 ) ocrAddDependence( TS_Lsend.OET, PTR_rankH->haloSendEVTs[0], 0, DB_MODE_RO );
    ocrAddDependence( TS_Lsend.OET, EVT_Lsend_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, TS_Lsend.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_xIn, TS_Lsend.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankH->DBK_LsendBufs[phase], TS_Lsend.EDT, _idep++, DB_MODE_RW );

    MyOcrTaskStruct_t TS_Rsend; _paramc = 0; _depc = 3;
    ocrGuid_t EVT_Rsend_fin;
    ocrEventCreate( &EVT_Rsend_fin, OCR_EVENT_STICKY_T, false );

    TS_Rsend.TML = PTR_rankTemplateH->TML_FNC_Rsend;
    ocrEdtCreate( &TS_Rsend.EDT, TS_Rsend.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &TS_Rsend.OET);

    if( id_x != NR_X - 1 ) ocrAddDependence( TS_Rsend.OET, PTR_rankH->haloSendEVTs[1], 0, DB_MODE_RO );
    ocrAddDependence( TS_Rsend.OET, EVT_Rsend_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, TS_Rsend.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_xIn, TS_Rsend.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankH->DBK_RsendBufs[phase], TS_Rsend.EDT, _idep++, DB_MODE_RW );

    MyOcrTaskStruct_t TS_Lrecv; _paramc = 0; _depc = 3;

    TS_Lrecv.TML = PTR_rankTemplateH->TML_FNC_Lrecv;
    ocrEdtCreate( &TS_Lrecv.EDT, TS_Lrecv.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &TS_Lrecv.OET);

    ocrGuid_t EVT_Lrecv_fin;
    ocrEventCreate( &EVT_Lrecv_fin, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( TS_Lrecv.OET, EVT_Lrecv_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, TS_Lrecv.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_xIn, TS_Lrecv.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( (id_x!=0)?PTR_rankH->haloRecvEVTs[0]:NULL_GUID, TS_Lrecv.EDT, _idep++, DB_MODE_RO );

    MyOcrTaskStruct_t TS_Rrecv; _paramc = 0; _depc = 3;

    TS_Rrecv.TML = PTR_rankTemplateH->TML_FNC_Rrecv;
    ocrEdtCreate( &TS_Rrecv.EDT, TS_Rrecv.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &TS_Rrecv.OET);

    ocrGuid_t EVT_Rrecv_fin;
    ocrEventCreate( &EVT_Rrecv_fin, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( TS_Rrecv.OET, EVT_Rrecv_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, TS_Rrecv.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_xIn, TS_Rrecv.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( (id_x!=NR_X-1)?PTR_rankH->haloRecvEVTs[1]:NULL_GUID, TS_Rrecv.EDT, _idep++, DB_MODE_RO );

    MyOcrTaskStruct_t TS_Bsend; _paramc = 0; _depc = 3;
    ocrGuid_t EVT_Bsend_fin;
    ocrEventCreate( &EVT_Bsend_fin, OCR_EVENT_STICKY_T, false );

    TS_Bsend.TML = PTR_rankTemplateH->TML_FNC_Bsend;
    ocrEdtCreate( &TS_Bsend.EDT, TS_Bsend.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &TS_Bsend.OET);

    if( id_y!=0 ) ocrAddDependence( TS_Bsend.OET, PTR_rankH->haloSendEVTs[2], 0, DB_MODE_RO );
    ocrAddDependence( TS_Bsend.OET, EVT_Bsend_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, TS_Bsend.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_xIn, TS_Bsend.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankH->DBK_BsendBufs[phase], TS_Bsend.EDT, _idep++, DB_MODE_RW );

    MyOcrTaskStruct_t TS_Tsend; _paramc = 0; _depc = 3;
    ocrGuid_t EVT_Tsend_fin;
    ocrEventCreate( &EVT_Tsend_fin, OCR_EVENT_STICKY_T, false );

    TS_Tsend.TML = PTR_rankTemplateH->TML_FNC_Tsend;
    ocrEdtCreate( &TS_Tsend.EDT, TS_Tsend.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &TS_Tsend.OET);

    if( id_y != NR_Y - 1 ) ocrAddDependence( TS_Tsend.OET, PTR_rankH->haloSendEVTs[3], 0, DB_MODE_RO );
    ocrAddDependence( TS_Tsend.OET, EVT_Tsend_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, TS_Tsend.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_xIn, TS_Tsend.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( PTR_rankH->DBK_TsendBufs[phase], TS_Tsend.EDT, _idep++, DB_MODE_RW );

    MyOcrTaskStruct_t TS_Brecv; _paramc = 0; _depc = 3;

    TS_Brecv.TML = PTR_rankTemplateH->TML_FNC_Brecv;
    ocrEdtCreate( &TS_Brecv.EDT, TS_Brecv.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &TS_Brecv.OET);

    ocrGuid_t EVT_Brecv_fin;
    ocrEventCreate( &EVT_Brecv_fin, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( TS_Brecv.OET, EVT_Brecv_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, TS_Brecv.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_xIn, TS_Brecv.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( (id_y!=0)?PTR_rankH->haloRecvEVTs[2]:NULL_GUID, TS_Brecv.EDT, _idep++, DB_MODE_RO );

    MyOcrTaskStruct_t TS_Trecv; _paramc = 0; _depc = 3;

    TS_Trecv.TML = PTR_rankTemplateH->TML_FNC_Trecv;
    ocrEdtCreate( &TS_Trecv.EDT, TS_Trecv.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &TS_Trecv.OET);

    ocrGuid_t EVT_Trecv_fin;
    ocrEventCreate( &EVT_Trecv_fin, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( TS_Trecv.OET, EVT_Trecv_fin, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, TS_Trecv.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_xIn, TS_Trecv.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( (id_y!=NR_Y-1)?PTR_rankH->haloRecvEVTs[3]:NULL_GUID, TS_Trecv.EDT, _idep++, DB_MODE_RO );

    ocrGuid_t TS_update_OET;
    ocrEventCreate( &TS_update_OET, OCR_EVENT_STICKY_T, false );

    MyOcrTaskStruct_t TS_update; //FNC_update

    TS_update.TML = PTR_rankTemplateH->TML_FNC_update;
    ocrEdtCreate( &TS_update.EDT, TS_update.TML,
                  EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &TS_update.OET );

    ocrAddDependence( TS_update.OET, TS_update_OET, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, TS_update.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_xIn, TS_update.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_xOut, TS_update.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_weight, TS_update.EDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_refNorm, TS_update.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_timers, TS_update.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_norm_reductionH, TS_update.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_timer_reductionH, TS_update.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( EVT_Lsend_fin, TS_update.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence( EVT_Rsend_fin, TS_update.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence( EVT_Lrecv_fin, TS_update.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence( EVT_Rrecv_fin, TS_update.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence( EVT_Bsend_fin, TS_update.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence( EVT_Tsend_fin, TS_update.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence( EVT_Brecv_fin, TS_update.EDT, _idep++, DB_MODE_NULL );
    ocrAddDependence( EVT_Trecv_fin, TS_update.EDT, _idep++, DB_MODE_NULL );

    s32 ntimesteps = PTR_globalParamH->NT;
    if( itimestep == ntimesteps && id == 0 )
    {
        //Trigger wrapup EVT
        MyOcrTaskStruct_t TS_summary; _paramc = 0; _depc = 4;

        TS_summary.FNC = FNC_summary;
        ocrEdtTemplateCreate( &TS_summary.TML, TS_summary.FNC, _paramc, _depc );

        ocrEdtCreate( &TS_summary.EDT, TS_summary.TML,
                      EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &myEdtAffinityHNT, NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, TS_summary.EDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( PTR_globalOcrParamH->EVT_OUT_timer_reduction, TS_summary.EDT, _idep++, DB_MODE_RW );
        ocrAddDependence( PTR_globalOcrParamH->EVT_OUT_norm_reduction, TS_summary.EDT, _idep++, DB_MODE_RW );
        ocrAddDependence( TS_update_OET, TS_summary.EDT, _idep++, DB_MODE_NULL );
    }

    return NULL_GUID;
}

_OCR_TASK_FNC_( timestepLoopEdt )
{
    s32 itimestep = paramv[0];

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid;
    ocrGuid_t DBK_xIn = depv[_idep++].guid;
    ocrGuid_t DBK_xOut = depv[_idep++].guid;
    ocrGuid_t DBK_weight = depv[_idep++].guid;
    ocrGuid_t DBK_refNorm = depv[_idep++].guid;
    ocrGuid_t DBK_timers = depv[_idep++].guid;
    ocrGuid_t DBK_norm_reductionH = depv[_idep++].guid;
    ocrGuid_t DBK_timer_reductionH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    double *xIn = depv[_idep++].ptr;
    double *xOut = depv[_idep++].ptr;
    double *weight = depv[_idep++].ptr;
    double *refNorm = depv[_idep++].ptr;
    timer* PTR_timers = depv[_idep++].ptr;
    reductionPrivate_t* PTR_norm_reductionH = depv[_idep++].ptr;
    reductionPrivate_t* PTR_timer_reductionH = depv[_idep++].ptr;

    rankParamH_t* PTR_rankParamH = &(PTR_rankH->rankParamH);
    globalCmdParamH_t* PTR_globalParamH = &(PTR_rankH->globalParamH.cmdParamH);
    globalOcrParamH_t* PTR_globalOcrParamH = &(PTR_rankH->globalParamH.ocrParamH);

    s64 id = PTR_rankParamH->id;

    // Do one timestep
    ocrGuid_t timestepTML, timestepEDT, timestepOEVT, timestepOEVTS;

    ocrEdtTemplateCreate( &timestepTML, timestepEdt, 1, 8 );

    ocrEdtCreate( &timestepEDT, timestepTML,
                  EDT_PARAM_DEF, (u64*) &itimestep, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &PTR_rankH->myEdtAffinityHNT, &timestepOEVT );

    ocrEventCreate( &timestepOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( timestepOEVT, timestepOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, timestepEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_xIn, timestepEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_xOut, timestepEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_weight, timestepEDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_refNorm, timestepEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_timers, timestepEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_norm_reductionH, timestepEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_timer_reductionH, timestepEDT, _idep++, DB_MODE_RW );

    itimestep += 1;

    s32 ntimesteps = PTR_globalParamH->NT;

    if( itimestep <= ntimesteps )
    {
    //start next timestep
        ocrGuid_t timestepLoopTML, timestepLoopEDT, timestepLoopOEVT, timestepLoopOEVTS;

        ocrEdtTemplateCreate( &timestepLoopTML, timestepLoopEdt, 1, 9 );

        ocrEdtCreate( &timestepLoopEDT, timestepLoopTML,
                      EDT_PARAM_DEF, (u64*)&itimestep, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &PTR_rankH->myEdtAffinityHNT, &timestepLoopOEVT );

        ocrEventCreate( &timestepLoopOEVTS, OCR_EVENT_STICKY_T, false );
        ocrAddDependence( timestepLoopOEVT, timestepLoopOEVTS, 0, DB_MODE_NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, timestepLoopEDT, _idep++, DB_MODE_RO );
        ocrAddDependence( DBK_xIn, timestepLoopEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_xOut, timestepLoopEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_weight, timestepLoopEDT, _idep++, DB_MODE_CONST );
        ocrAddDependence( DBK_refNorm, timestepLoopEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_timers, timestepLoopEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_norm_reductionH, timestepLoopEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_timer_reductionH, timestepLoopEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( timestepOEVTS, timestepLoopEDT, _idep++, DB_MODE_NULL );
    }

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_initialize )
{
    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid;
    ocrGuid_t DBK_xIn = depv[_idep++].guid;
    ocrGuid_t DBK_xOut = depv[_idep++].guid;
    ocrGuid_t DBK_weight = depv[_idep++].guid;
    ocrGuid_t DBK_refNorm = depv[_idep++].guid;
    ocrGuid_t DBK_timers = depv[_idep++].guid;
    ocrGuid_t DBK_norm_reductionH = depv[_idep++].guid;
    ocrGuid_t DBK_timer_reductionH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    double *xIn = depv[_idep++].ptr;
    double *xOut = depv[_idep++].ptr;
    double *weight = depv[_idep++].ptr;
    double *refNorm = depv[_idep++].ptr;
    timer* PTR_timers = depv[_idep++].ptr;
    reductionPrivate_t* PTR_norm_reductionH = depv[_idep++].ptr;
    reductionPrivate_t* PTR_timer_reductionH = depv[_idep++].ptr;

    rankParamH_t* PTR_rankParamH = &(PTR_rankH->rankParamH);
    globalCmdParamH_t* PTR_globalParamH = &(PTR_rankH->globalParamH.cmdParamH);
    globalOcrParamH_t* PTR_globalOcrParamH = &(PTR_rankH->globalParamH.ocrParamH);

    s64 np_x = PTR_rankParamH->np_x;
    s64 id_x = PTR_rankParamH->id_x;
    s64 ib = PTR_rankParamH->ib;
    s64 ie = PTR_rankParamH->ie;

    s64 NR_X = PTR_globalParamH->NR_X;

    s64 np_y = PTR_rankParamH->np_y;
    s64 id_y = PTR_rankParamH->id_y;
    s64 jb = PTR_rankParamH->jb;
    s64 je = PTR_rankParamH->je;

    s64 NR_Y = PTR_globalParamH->NR_Y;

    s64 id = PTR_rankParamH->id;

    s64 i, j;
    int ii, jj;

    for( j = jb; j <= je; j++ )
        for( i = ib; i <= ie; i++ )
        {
            IN(i,j) = (double) ( i ) + ( j );
            OUT(i,j) = 0.;
        }

    /* fill the stencil weights to reflect a discrete divergence operator         */
    for (jj=-HALO_RADIUS; jj<=HALO_RADIUS; jj++) for (ii=-HALO_RADIUS; ii<=HALO_RADIUS; ii++)
        WEIGHT(ii,jj) = (double) 0.0;

    //stencil_size = 4*HALO_RADIUS+1;
    for (ii=1; ii<=HALO_RADIUS; ii++) {
        WEIGHT(0, ii) = WEIGHT( ii,0) =  (double) (1.0/(2.0*ii*HALO_RADIUS));
        WEIGHT(0,-ii) = WEIGHT(-ii,0) = -(double) (1.0/(2.0*ii*HALO_RADIUS));
    }

    for( i = 0; i < number_of_timers; i++ )
    {
        PTR_timers[i].start = 0;
        PTR_timers[i].total = 0;
        PTR_timers[i].count = 0;
        PTR_timers[i].elapsed = 0;
    }

    refNorm[0] = 0.0;

    PTR_norm_reductionH->nrank = PTR_globalParamH->NR;
    PTR_norm_reductionH->myrank = id;
    PTR_norm_reductionH->ndata = 1;
    PTR_norm_reductionH->reductionOperator = REDUCTION_F8_ADD;
    PTR_norm_reductionH->rangeGUID = PTR_globalOcrParamH->normReductionRangeGUID;
    PTR_norm_reductionH->reductionTML = NULL_GUID;
    PTR_norm_reductionH->new = 1;  //first time
    PTR_norm_reductionH->all = 0;  //go up and down (ALL_REDUCE)
    if(id==0) PTR_norm_reductionH->returnEVT = PTR_globalOcrParamH->EVT_OUT_norm_reduction;

    PTR_timer_reductionH->nrank = PTR_globalParamH->NR;
    PTR_timer_reductionH->myrank = id;
    PTR_timer_reductionH->ndata = 1;
    PTR_timer_reductionH->reductionOperator = REDUCTION_F8_MAX;
    PTR_timer_reductionH->rangeGUID = PTR_globalOcrParamH->timerReductionRangeGUID;
    PTR_timer_reductionH->reductionTML = NULL_GUID;
    PTR_timer_reductionH->new = 1;  //first time
    PTR_timer_reductionH->all = 0;  //go up and down (ALL_REDUCE)
    if(id==0) PTR_timer_reductionH->returnEVT = PTR_globalOcrParamH->EVT_OUT_timer_reduction;

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_stencil )
{
    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid;
    ocrGuid_t DBK_xIn = depv[_idep++].guid;
    ocrGuid_t DBK_xOut = depv[_idep++].guid;
    ocrGuid_t DBK_weight = depv[_idep++].guid;
    ocrGuid_t DBK_refNorm = depv[_idep++].guid;
    ocrGuid_t DBK_timers = depv[_idep++].guid;
    ocrGuid_t DBK_norm_reductionH = depv[_idep++].guid;
    ocrGuid_t DBK_timer_reductionH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    double *xIn = depv[_idep++].ptr;
    double *xOut = depv[_idep++].ptr;
    double *weight = depv[_idep++].ptr;
    double *refNorm = depv[_idep++].ptr;
    timer* PTR_timers = depv[_idep++].ptr;
    reductionPrivate_t* PTR_norm_reductionH = depv[_idep++].ptr;
    reductionPrivate_t* PTR_timer_reductionH = depv[_idep++].ptr;

    ocrGuid_t initializeDataTML, initializeDataEDT, initializeDataOEVT, initializeDataOEVTS;

    ocrEdtTemplateCreate( &initializeDataTML, FNC_initialize, 0, 8 );

    ocrEdtCreate( &initializeDataEDT, initializeDataTML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &PTR_rankH->myEdtAffinityHNT, &initializeDataOEVT );

    ocrEventCreate( &initializeDataOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( initializeDataOEVT, initializeDataOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, initializeDataEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_xIn, initializeDataEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_xOut, initializeDataEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_weight, initializeDataEDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_refNorm, initializeDataEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_timers, initializeDataEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_norm_reductionH, initializeDataEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_timer_reductionH, initializeDataEDT, _idep++, DB_MODE_RW );

    u64 itimestep = 0;
    ocrGuid_t timestepLoopTML, timestepLoopEDT, timestepLoopOEVT, timestepLoopOEVTS;

    ocrEdtTemplateCreate( &timestepLoopTML, timestepLoopEdt, 1, 9 );

    ocrEdtCreate( &timestepLoopEDT, timestepLoopTML,
                  EDT_PARAM_DEF, (u64*) &itimestep, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &PTR_rankH->myEdtAffinityHNT, &timestepLoopOEVT );

    ocrEventCreate( &timestepLoopOEVTS, OCR_EVENT_STICKY_T, false );
    ocrAddDependence( timestepLoopOEVT, timestepLoopOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, timestepLoopEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_xIn, timestepLoopEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_xOut, timestepLoopEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_weight, timestepLoopEDT, _idep++, DB_MODE_CONST );
    ocrAddDependence( DBK_refNorm, timestepLoopEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_timers, timestepLoopEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_norm_reductionH, timestepLoopEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_timer_reductionH, timestepLoopEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( initializeDataOEVTS, timestepLoopEDT, _idep++, DB_MODE_NULL );

    return NULL_GUID;
}

SimFlat* initSimulation(Command cmd)
{
   //SimFlat* sim = comdMalloc(sizeof(SimFlat));
   sim->nSteps = cmd.nSteps;
   sim->printRate = cmd.printRate;
   sim->dt = cmd.dt;
   sim->domain = NULL;
   sim->boxes = NULL;
   sim->atoms = NULL;
   sim->ePotential = 0.0;
   sim->eKinetic = 0.0;
   sim->atomExchange = NULL;

   sim->pot = initPotential(cmd.doeam, cmd.potDir, cmd.potName, cmd.potType);
   real_t latticeConstant = cmd.lat;
   if (cmd.lat < 0.0)
      latticeConstant = sim->pot->lat;

   // ensure input parameters make sense.
   sanityChecks(cmd, sim->pot->cutoff, latticeConstant, sim->pot->latticeType);

   sim->species = initSpecies(sim->pot);

   real3 globalExtent;
   globalExtent[0] = cmd.nx * latticeConstant;
   globalExtent[1] = cmd.ny * latticeConstant;
   globalExtent[2] = cmd.nz * latticeConstant;

   sim->domain = initDecomposition(
      cmd.xproc, cmd.yproc, cmd.zproc, globalExtent);

   sim->boxes = initLinkCells(sim->domain, sim->pot->cutoff);
   sim->atoms = initAtoms(sim->boxes);

   // create lattice with desired temperature and displacement.
   createFccLattice(cmd.nx, cmd.ny, cmd.nz, latticeConstant, sim);
   setTemperature(sim, cmd.temperature);
   randomDisplacements(sim, cmd.initialDelta);

   sim->atomExchange = initAtomHaloExchange(sim->domain, sim->boxes);

   // Forces must be computed before we call the time stepper.
   startTimer(redistributeTimer);
   redistributeAtoms(sim);
   stopTimer(redistributeTimer);

   startTimer(computeForceTimer);
   computeForce(sim);
   stopTimer(computeForceTimer);

   kineticEnergy(sim);

   return sim;
}
#endif

void initOcrTemplates( rankTemplateH_t* PTR_rankTemplateH )
{
    s32 _idep, _paramc, _depc;

    //_paramc = 0; _depc = 3;
    //ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_Lsend), FNC_Lsend, _paramc, _depc );
    //ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_Rsend), FNC_Rsend, _paramc, _depc );
    //ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_Lrecv), FNC_Lrecv, _paramc, _depc );
    //ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_Rrecv), FNC_Rrecv, _paramc,_depc );

    //_paramc = 0; _depc = 3;
    //ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_Bsend), FNC_Bsend, _paramc, _depc );
    //ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_Tsend), FNC_Tsend, _paramc, _depc );
    //ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_Brecv), FNC_Brecv, _paramc, _depc );
    //ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_Trecv), FNC_Trecv, _paramc,_depc );

    //_paramc = 1; _depc = 16;

    //ocrEdtTemplateCreate( &(PTR_rankTemplateH->TML_FNC_update), FNC_update, _paramc, _depc );

}

_OCR_TASK_FNC_( FNC_initSimulation )
{
    s32 _idep, _paramc, _depc;

    ocrGuid_t DBK_rankH = depv[0].guid;

    rankH_t* PTR_rankH = depv[0].ptr;

    //rankParamH_t* PTR_rankParamH = &(PTR_rankH->rankParamH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    initOcrTemplates( PTR_rankTemplateH );

    //initSimulation( PTR_sim, PTR_cmd );

    return NULL_GUID;
}

_OCR_TASK_FNC_( channelSetupEdt )
{
    s32 _idep, _paramc, _depc;

    #if 0

    u64 id = paramv[0];

    ocrGuid_t DBK_rankH = depv[0].guid;

    rankH_t* PTR_rankH = depv[0].ptr;

    rankParamH_t* PTR_rankParamH = &(PTR_rankH->rankParamH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    PTR_rankParamH->id = id;

    globalCoordsFromRank_Cart3D( id, PTR_cmd->xprocs, PTR_cmd->yprocs, PTR_cmd->zprocs,
                                    &PTR_rankParamH->id_x, &PTR_rankParamH->id_y, &PTR_rankParamH->id_z  );

    u64 i;

    //Capture all the events needed for inter-rank depedencies
    for(i=0;i<6;i++)
    {
        ocrGuid_t* eventsPTR = depv[1+i].ptr;
        PTR_rankH->haloRecvEVTs[i] = eventsPTR[0];
        //PRINTF("Recv rank %d %d "GUIDF" \n", id, i, PTR_rankH->haloRecvEVTs[i]);
    }

    ocrGuid_t currentAffinity = NULL_GUID;
    ocrHint_t myEdtAffinityHNT;
    ocrHintInit( &myEdtAffinityHNT, OCR_HINT_EDT_T );
    ocrHint_t myDbkAffinityHNT;
    ocrHintInit( &myDbkAffinityHNT, OCR_HINT_DB_T );

    ocrAffinityGetCurrent(&currentAffinity);
    ocrSetHintValue( &myEdtAffinityHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(currentAffinity) );
    ocrSetHintValue( &myDbkAffinityHNT, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(currentAffinity) );

    PTR_rankH->myEdtAffinityHNT = myEdtAffinityHNT;
    PTR_rankH->myDbkAffinityHNT = myDbkAffinityHNT;

    SimFlat* sim;

    ocrDbCreate( &(PTR_rankH->DBK_sim), (void **) &sim, sizeof(SimFlat*),
                 DB_PROP_NONE, &myDbkAffinityHNT, NO_ALLOC );

    MyOcrTaskStruct_t TS_initSimulation; _paramc = 0; _depc = 8;

    TS_initSimulation.FNC = FNC_initSimulation;
    ocrEdtTemplateCreate( &TS_initSimulation.TML, TS_initSimulation.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_initSimulation.EDT, TS_initSimulation.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, TS_initSimulation.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_sim, TS_initSimulation.EDT, _idep++, DB_MODE_RW );

    MyOcrTaskStruct_t TS_init_rankH; _paramc = 0; _depc = 8;

    TS_init_rankH.FNC = FNC_stencil;
    ocrEdtTemplateCreate( &TS_init_rankH.TML, TS_init_rankH.FNC, _paramc, _depc );

    ocrEdtCreate( &TS_init_rankH.EDT, TS_init_rankH.TML,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, TS_init_rankH.EDT, _idep++, DB_MODE_RO );
    ocrAddDependence( DBK_xIn, TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_xOut, TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_weight, TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_refNorm, TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_timers, TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_norm_reductionH, TS_init_rankH.EDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_timer_reductionH, TS_init_rankH.EDT, _idep++, DB_MODE_RW );

    #endif

    return NULL_GUID;
}

_OCR_TASK_FNC_( initEdt )
{
    PRM_initEdt_t* PTR_PRM_initEdt = (PRM_initEdt_t*) paramv;

    u64 id = PTR_PRM_initEdt->id;

    ocrGuid_t DBK_cmdLineArgs = depv[0].guid;
    ocrGuid_t DBK_globalParamH = depv[1].guid;

    void *PTR_cmdLineArgs = depv[0].ptr;
    globalParamH_t *PTR_globalParamH = (globalParamH_t *) depv[1].ptr;

    ocrGuid_t DBK_rankH;
    rankH_t *PTR_rankH;

    //DB creation calls
    ocrDbCreate( &DBK_rankH, (void**) &PTR_rankH, sizeof(rankH_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );

    memcpy( &(PTR_rankH->globalParamH), PTR_globalParamH, sizeof(globalParamH_t) );

    ocrGuid_t currentAffinity = NULL_GUID;
    ocrHint_t myEdtAffinityHNT;
    ocrHintInit( &myEdtAffinityHNT, OCR_HINT_EDT_T );
    ocrHint_t myDbkAffinityHNT;
    ocrHintInit( &myDbkAffinityHNT, OCR_HINT_DB_T );

    ocrAffinityGetCurrent(&currentAffinity);
    ocrSetHintValue( &myEdtAffinityHNT, OCR_HINT_EDT_AFFINITY, ocrAffinityToHintValue(currentAffinity) );
    ocrSetHintValue( &myDbkAffinityHNT, OCR_HINT_DB_AFFINITY, ocrAffinityToHintValue(currentAffinity) );

    s32 gx = PTR_PRM_initEdt->edtGridDims[0];
    s32 gy = PTR_PRM_initEdt->edtGridDims[1];
    s32 gz = PTR_PRM_initEdt->edtGridDims[2];

    //TODO
    //PTR_rankH->rankParamH.id = id;
    //PTR_rankH->rankParamH.id_x = id % gx;
    //PTR_rankH->rankParamH.id_y = id / gx;

    //PTR_rankH->myEdtAffinityHNT = myEdtAffinityHNT;
    //PTR_rankH->myDbkAffinityHNT = myDbkAffinityHNT;

    int ix0, iy0, iz0, ix, iy, iz;

    globalCoordsFromRank_Cart3D( id, gx, gy, gz, &ix0, &iy0, &iz0 );

    ocrGuid_t channelSetupTML, channelSetupEDT, stickyEVT;

    ocrEventParams_t params;
    params.EVENT_CHANNEL.maxGen = 2;
    params.EVENT_CHANNEL.nbSat = 1;
    params.EVENT_CHANNEL.nbDeps = 1;

    ocrEdtTemplateCreate( &channelSetupTML, channelSetupEdt, 1, 7 );
    ocrEdtCreate( &channelSetupEDT, channelSetupTML, EDT_PARAM_DEF, (u64*)&id, EDT_PARAM_DEF, NULL, EDT_PROP_NONE,
                    &myEdtAffinityHNT, NULL );

    int nbr, i, j, k;
    int nbrImage;

    //6 neighbors: -x, +x, -y, +y, -z, +z
    for( nbr = 0; nbr < 6; nbr++ )
    {
        i = j = k = 0;

        switch(nbr)
        {
            case 0:
                i = -1;                 // neighbor offset
                nbrImage = 1;           // this is the tag the same nbr uses to refer to the current "rank"
                break;
            case 1:
                i = 1;
                nbrImage = 0;
                break;
            case 2:
                j = -1;
                nbrImage = 3;
                break;
            case 3:
                j = 1;
                nbrImage = 2;
                break;
            case 4:
                k = -1;
                nbrImage = 5;
                break;
            case 5:
                k = 1;
                nbrImage = 4;
                break;
        }

        ix = MOD(ix0+i, gx); //periodic index
        iy = MOD(iy0+j, gy);
        iz = MOD(iz0+k, gz);
        u32 nbrRank = globalRankFromCoords_Cart3D(ix, iy, iz, gx, gy, gz);

        //Collective event create for sends
        ocrGuidFromIndex(&(stickyEVT), PTR_rankH->globalParamH.ocrParamH.haloRangeGUID, 6*id + nbr);//send
        ocrEventCreate( &stickyEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG );

        ocrEventCreateParams( &(PTR_rankH->haloSendEVTs[nbr]), OCR_EVENT_CHANNEL_T, false, &params );

        ocrGuid_t* eventsPTR;
        ocrGuid_t eventsDBK;
        ocrDbCreate( &eventsDBK, (void**) &eventsPTR, 1*sizeof(ocrGuid_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );

        eventsPTR[0] = PTR_rankH->haloSendEVTs[nbr]; //channel event of the send operation from rank i to nbrRank

        ocrDbRelease( eventsDBK );

        ocrEventSatisfy( stickyEVT, eventsDBK );

        //Map the sends to receive events
        //receive: (id, nbr) will be the send event from the (nbrRank,nbrImage)
        ocrGuidFromIndex( &(stickyEVT), PTR_rankH->globalParamH.ocrParamH.haloRangeGUID, 6*nbrRank + nbrImage );
        ocrEventCreate( &stickyEVT, OCR_EVENT_STICKY_T, GUID_PROP_CHECK | EVT_PROP_TAKES_ARG );

        //PRINTF("s %d r %d se %d re %d s(%d %d) r(%d %d)\n", id, nbrRank, nbr, nbrImage, ix0, iy0, ix, iy );
        //PRINTF("Send rank %d %d "GUIDF" \n", id, nbr, PTR_rankH->haloSendEVTs[nbr]);

        ocrAddDependence( stickyEVT, channelSetupEDT, 1+nbr, DB_MODE_RW ); //TODO
    }

    ocrDbRelease(DBK_rankH);

    ocrAddDependence( DBK_rankH, channelSetupEDT, 0, DB_MODE_RW );

    return NULL_GUID;
}

ocrGuid_t mainEdt( u32 paramc, u64* paramv, u32 depc, ocrEdtDep_t depv[] )
{
    u32 _paramc, _depc, _idep;

    ocrGuid_t DBK_cmdLineArgs = depv[0].guid;

    void * PTR_cmdLineArgs = depv[0].ptr;
    u32 argc = getArgc( PTR_cmdLineArgs );

    //Pack the PTR_cmdLineArgs into the "cannonical" char** argv
    ocrGuid_t argv_g;
    char** argv;
    ocrDbCreate( &argv_g, (void**)&argv, sizeof(char*)*argc, DB_PROP_NONE, PICK_1_1(NULL_HINT,NULL_GUID), NO_ALLOC );
    for( u32 a = 0; a < argc; ++a )
       argv[a] = getArgv( PTR_cmdLineArgs, a );

    Command cmd = parseCommandLine(argc, argv);
    printCmdYaml(screenOut, &cmd);

    u64 nRanks = (cmd.xproc)*(cmd.yproc)*(cmd.zproc);

    //Create OCR objects to pass down to the child EDTs
    //for collective operatations among the child EDTs
    globalOcrParamH_t ocrParamH;
    ocrGuidRangeCreate(&(ocrParamH.haloRangeGUID), 6*nRanks, GUID_USER_EVENT_STICKY);
    //TODO: Either have 6 nbrs or 27 nbrs depending on the halo-exchange implementation
    ocrGuidRangeCreate(&(ocrParamH.normReductionRangeGUID), nRanks, GUID_USER_EVENT_STICKY);
    ocrGuidRangeCreate(&(ocrParamH.timerReductionRangeGUID),nRanks, GUID_USER_EVENT_STICKY);
    ocrEventCreate( &(ocrParamH.EVT_OUT_norm_reduction), OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG );
    ocrEventCreate( &(ocrParamH.EVT_OUT_timer_reduction), OCR_EVENT_ONCE_T, EVT_PROP_TAKES_ARG );

    //A datablock to store the commandline and the OCR objectes created above
    ocrGuid_t DBK_globalParamH;
    globalParamH_t* PTR_globalParamH;
    ocrDbCreate( &DBK_globalParamH, (void**) &PTR_globalParamH, sizeof(globalParamH_t), DB_PROP_NONE, NULL_HINT, NO_ALLOC );

    memcpy(&(PTR_globalParamH->cmdParamH), &cmd, sizeof(Command) );
    memcpy(&(PTR_globalParamH->ocrParamH), &ocrParamH, sizeof(globalOcrParamH_t) );

    //Release the changes so they are visible for any dependent EDTs below
    ocrDbRelease(DBK_globalParamH);

    s64 edtGridDims[3] = { cmd.xproc, cmd.yproc, cmd.zproc };

    //All SPMD EDTs depend on the following dependencies
    ocrGuid_t spmdDepv[2] = {DBK_cmdLineArgs, DBK_globalParamH};

    //2-D Cartesian grid of SPMD EDTs get mapped to a 2-D Cartesian grid of PDs
#ifdef USE_STATIC_SCHEDULER
    PRINTF("Using STATIC scheduler\n");
    forkSpmdEdts_staticScheduler_Cart3D( initEdt, edtGridDims, spmdDepv );
#else
    PRINTF("NOT Using STATIC scheduler\n");
    forkSpmdEdts_Cart3D( initEdt, edtGridDims, spmdDepv );
#endif

    return NULL_GUID;
}
