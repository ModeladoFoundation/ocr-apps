// ************************************************************************
//
// miniAMR: stencil computations with boundary exchange and AMR.
//
// Copyright (2014) Sandia Corporation. Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
// retains certain rights in this software.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
// Questions? Contact Courtenay T. Vaughan (ctvaugh@sandia.gov)
//                    Richard F. Barrett (rfbarre@sandia.gov)
//
// ************************************************************************

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifndef TG_ARCH
#include <sys/time.h>
#include <inttypes.h>
#endif

#include "block.h"
//#include "comm.h"
#include "proto.h"
#include "timer.h"

#define TICK (1.0e-6)

double timer(void)
{
   //return(MPI_Wtime());
#ifndef TG_ARCH
   struct timeval ptime;
   gettimeofday(&ptime, (struct timezone *)NULL);
   return TICK * (double)(((u64)1000000)*(u64)ptime.tv_sec + (u64)ptime.tv_usec);
#else
   return 0;
#endif
}

void ma_malloc_counter( timerH_t* PTR_timerH, void* ptr, size_t size, char *file, int line)
{
   if (ptr == NULL) {
      PRINTF("NULL pointer from ocrDbCreate call in %s at %d\n", file, line);
   }

   PTR_timerH->counter_malloc++;
   PTR_timerH->size_malloc += (double) size;
}

void transferTimer( timerH_t* dst, timerH_t* src )
{
    double *dst_double = (double*) dst;
    double *src_double = (double*) src;

    int i;
    for( i = TIMER_SUM_LB; i <= TIMER_SUM_UB; i++ )
        dst_double[i] += src_double[i];

    //for( i = TIMER_MIN_LB; i <= TIMER_MIN_UB; i++ )
    //    dst_double[i] = MIN(dst_double[i], src_double[i]);

    //for( i = TIMER_MAX_LB; i <= TIMER_MAX_UB; i++ )
    //    dst_double[i] = MAX(dst_double[i], src_double[i]);

    for( i = TIMER_ZERO_LB; i <= TIMER_ZERO_UB; i++ )
        dst_double[i] = 0.;
}

void resetTimer( timerH_t* PTR_timerH )
{
    memset( PTR_timerH, 0, sizeof(timerH_t) );
}

int cmp(const void * a, const void * b)
{
   return ( *(int*)a - *(int*)b );
}


int binary_search_double( int n, int quarry, int * A)
{
    int *p = (int*) bsearch( &quarry, A, n, sizeof(int), cmp );

    if( p != NULL )
        return p-A;
    else {
        PRINTF("Not found! %d is not present in the list.\n", quarry);
        return 0;
    }
}

void sortArray( int n, int *A)
{
    qsort( A, n, sizeof(int), cmp );
}

size_t reductionDBKsize( int reductionHandle, rankH_t* PTR_rankH )
{
    size_t size;
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    int num_refine = PTR_cmd->num_refine;

    if( BLOCKCOUNT_RED_HANDLE_LB <= reductionHandle && reductionHandle <= BLOCKCOUNT_RED_HANDLE_UB ) {
        size = (1+num_refine)*sizeof(int);
    }
    else if( BLOCKIDGATHER_RED_HANDLE_LB <= reductionHandle && reductionHandle <= BLOCKIDGATHER_RED_HANDLE_UB ) {
        int global_max_b = PTR_rankH->num_pes*PTR_cmd->init_block_x*PTR_cmd->init_block_y*PTR_cmd->init_block_z;
        long num = global_max_b*((pow(8.0,PTR_cmd->num_refine+1)-1)/7.);
        size = num*sizeof(int);
    }
    else if( REFINEINTENT_RED_HANDLE_LB <= reductionHandle && reductionHandle <= REFINEINTENT_RED_HANDLE_UB ) {
        size = 1*sizeof(int);
    }
    else if( QUIESCENCE_RED_HANDLE_LB <= reductionHandle && reductionHandle <= QUIESCENCE_RED_HANDLE_UB ) {
        size = 1*sizeof(int);
    }
    else if( COARSEN_RED_HANDLE_LB <= reductionHandle && reductionHandle <= COARSEN_RED_HANDLE_UB ) {
        size = 1*sizeof(int);
    }
    else if( CHECKSUM_RED_HANDLE_LB <= reductionHandle && reductionHandle <= CHECKSUM_RED_HANDLE_UB ) {
        size = 1*sizeof(double);
    }
    else if( FINALIZE_RED_HANDLE_LB <= reductionHandle && reductionHandle <= FINALIZE_RED_HANDLE_UB ) {
        size = 1*sizeof(double);
    }

    return size;
}

void reductionHandle_update( int reductionHandle, rankH_t* PTR_rankH, reductionPrivate_t* PTR_redH)
{
    size_t size;
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    int num_refine = PTR_cmd->num_refine;

    if( BLOCKCOUNT_RED_HANDLE_LB <= reductionHandle && reductionHandle <= BLOCKCOUNT_RED_HANDLE_UB ) {
        PTR_redH->ndata = (1+num_refine);
        PTR_redH->reductionOperator = REDUCTION_U4_ADD;
    }
    else if( BLOCKIDGATHER_RED_HANDLE_LB <= reductionHandle && reductionHandle <= BLOCKIDGATHER_RED_HANDLE_UB ) {
        PTR_redH->ndata = PTR_rankH->max_possible_num_blocks;
        PTR_redH->reductionOperator = REDUCTION_U4_ADD;
    }
    else if( REFINEINTENT_RED_HANDLE_LB <= reductionHandle && reductionHandle <= REFINEINTENT_RED_HANDLE_UB ) {
        PTR_redH->ndata = 1;
        PTR_redH->reductionOperator = REDUCTION_U4_ADD;
    }
    else if( QUIESCENCE_RED_HANDLE_LB <= reductionHandle && reductionHandle <= QUIESCENCE_RED_HANDLE_UB ) {
        PTR_redH->ndata = 1;
        PTR_redH->reductionOperator = REDUCTION_U4_ADD;
    }
    else if( COARSEN_RED_HANDLE_LB <= reductionHandle && reductionHandle <= COARSEN_RED_HANDLE_UB ) {
        PTR_redH->ndata = 1;
        PTR_redH->reductionOperator = REDUCTION_U4_ADD;
        PTR_redH->type = REDUCE; //reduce to just the sibling 0
    }
    else if( CHECKSUM_RED_HANDLE_LB <= reductionHandle && reductionHandle <= CHECKSUM_RED_HANDLE_UB ) {
        PTR_redH->ndata = 1;
        PTR_redH->reductionOperator = REDUCTION_F8_ADD;
    }
    else if( FINALIZE_RED_HANDLE_LB <= reductionHandle && reductionHandle <= FINALIZE_RED_HANDLE_UB ) {
        PTR_redH->ndata = 1;
        PTR_redH->reductionOperator = REDUCTION_F8_MAX;
    }
}

_OCR_TASK_FNC_( FNC_reduceAllUp )
{
    reducePRM_t* reducePRM = (reducePRM_t*) paramv;
    int irefine = reducePRM->irefine;
    int ts = reducePRM->ts;
    int phase = reducePRM->phase;
    int r = reducePRM->r;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid;
    ocrGuid_t DBK_in = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    void* PTR_in = depv[_idep++].ptr;

    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    DEBUG_PRINTF(( "%s ilevel %d id_l %d isibling %d irefine %d phase %d r %d ts %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, PTR_rankH->isibling, irefine, phase, r, ts ));

    int ilevel = PTR_rankH->ilevel;

    block *bp = &PTR_rankH->blockH;

    s32 _ichild;

    redObjects_t* PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];

    ocrEVT_t redUpOEVT = PTR_redObjects->upOEVT;
    ocrEVT_t redUpIEVT = PTR_redObjects->upIEVT;
    ocrEVT_t redDownIEVT = PTR_redObjects->downIEVT;

    ocrEventSatisfy( redUpOEVT, DBK_in ); //All active blocks provide partial sums

    ocrDBK_t DBK_out = PTR_redObjects->DBK_out;

    if(ilevel != 0) {

        if( PTR_rankH->isibling == 0 ) { //parent reduction up
            //Create a FNC_reduceAllUp task with parents DBK
            ocrGuid_t reduceAllUpEDT;

            ocrEdtCreate( &reduceAllUpEDT, PTR_rankTemplateH->TML_reduceAllUp, //FNC_reduceAllUp
                          EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                          EDT_PROP_NONE, &PTR_rankH->myEdtAffinityHNT, NULL );

            _idep = 0;
            ocrAddDependence( PTR_sharedOcrObjH->parentRankDBK, reduceAllUpEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( PTR_sharedOcrObjH->parentRedObjects[r].upIEVT, reduceAllUpEDT, _idep++, DB_MODE_RW );

            //Accumulator EDT
            ocrGuid_t accumulatorEDT;

            ocrEdtCreate( &accumulatorEDT, PTR_rankTemplateH->TML_accumulator, //FNC_accumulator
                          EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                          EDT_PROP_NONE, &PTR_rankH->myEdtAffinityHNT, NULL );

            _idep = 0; _ichild = 0;
            ocrAddDependence( PTR_sharedOcrObjH->parentRankDBK, accumulatorEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( PTR_sharedOcrObjH->parentRedObjects[r].DBK_in, accumulatorEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( PTR_sharedOcrObjH->siblingsRedObjects[_ichild++][r].upOEVT, accumulatorEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( PTR_sharedOcrObjH->siblingsRedObjects[_ichild++][r].upOEVT, accumulatorEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( PTR_sharedOcrObjH->siblingsRedObjects[_ichild++][r].upOEVT, accumulatorEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( PTR_sharedOcrObjH->siblingsRedObjects[_ichild++][r].upOEVT, accumulatorEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( PTR_sharedOcrObjH->siblingsRedObjects[_ichild++][r].upOEVT, accumulatorEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( PTR_sharedOcrObjH->siblingsRedObjects[_ichild++][r].upOEVT, accumulatorEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( PTR_sharedOcrObjH->siblingsRedObjects[_ichild++][r].upOEVT, accumulatorEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( PTR_sharedOcrObjH->siblingsRedObjects[_ichild++][r].upOEVT, accumulatorEDT, _idep++, DB_MODE_RW );

        }
    }

    if( ilevel == 0 ) { //root parent or leaf

        //reductionLaunch(); on root
        ocrGuid_t reduceAllRootNodesEDT;

        ocrEdtCreate( &reduceAllRootNodesEDT, PTR_rankTemplateH->TML_reduceAllRootNodes, //FNC_reduceAllRootNodes
                      EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &PTR_rankH->myEdtAffinityHNT, NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, reduceAllRootNodesEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( redUpOEVT, reduceAllRootNodesEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( PTR_rankH->DBK_redUpH[r], reduceAllRootNodesEDT, _idep++, DB_MODE_RW );

        ocrGuid_t reduceAllDownEDT;

        ocrEdtCreate( &reduceAllDownEDT, PTR_rankTemplateH->TML_reduceAllDown, //FNC_reduceAllDown
                      EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &PTR_rankH->myEdtAffinityHNT, NULL );

        _idep = 0;
        ocrAddDependence( DBK_rankH, reduceAllDownEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( redDownIEVT, reduceAllDownEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_out, reduceAllDownEDT, _idep++, DB_MODE_RW );

    }

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_reduceAllDown )
{
    reducePRM_t* reducePRM = (reducePRM_t*) paramv;
    int irefine = reducePRM->irefine;
    int ts = reducePRM->ts;
    int phase = reducePRM->phase;
    int r = reducePRM->r;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid;
    ocrGuid_t DBK_blockCountsOut = depv[_idep++].guid;
    ocrGuid_t DBK_out = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    void* PTR_in = depv[_idep++].ptr;
    void* PTR_out = depv[_idep++].ptr;

    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    DEBUG_PRINTF(( "%s ilevel %d id_l %d isibling %d irefine %d phase %d r %d ts %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, PTR_rankH->isibling, irefine, phase, r, ts ));

     redObjects_t* PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];

    ocrEVT_t redDownOEVT = PTR_redObjects->downOEVT;
    ocrEVT_t redUpIEVT = PTR_redObjects->upIEVT;

    int i;

    int ilevel = PTR_rankH->ilevel;

    block *bp = &PTR_rankH->blockH;

    s32 _ichild;

    size_t size;

    size = reductionDBKsize( r, PTR_rankH );

    memcpy((char*)PTR_out, (char*)PTR_in, size);

    ocrDbRelease(DBK_out);

    if( bp->number >= 0 ) { //active node

        DEBUG_PRINTF(( "redDownOEVT "GUIDF" is satisfied\n", redDownOEVT ));
        DEBUG_PRINTF(( "%s ilevel %d id_l %d irefine %d phase %d ts %d DONE\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, irefine, phase, ts ));

        ocrEventSatisfy( redDownOEVT, DBK_out ); //Only expose to the active blocks

    }

    if( bp->number < 0 ) { // parent

        for( _ichild=0; _ichild < 8; _ichild++ ) {
            //Create a FNC_reduceAllDown task for all children
            ocrGuid_t reduceAllDownEDT;

            ocrEdtCreate( &reduceAllDownEDT, PTR_rankTemplateH->TML_reduceAllDown, //FNC_reduceAllDown
                          EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                          EDT_PROP_NONE, &PTR_rankH->myEdtAffinityHNT, NULL );

            _idep = 0;
            ocrAddDependence( PTR_sharedOcrObjH->childrenRankDBKs[_ichild], reduceAllDownEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( PTR_sharedOcrObjH->childrenRedObjects[_ichild][r].downIEVT, reduceAllDownEDT, _idep++, DB_MODE_RW );
            ocrAddDependence( PTR_sharedOcrObjH->childrenRedObjects[_ichild][r].DBK_out, reduceAllDownEDT, _idep++, DB_MODE_RW );
        }

        //Broadcast to children
        ocrGuid_t bcastEDT;

        ocrEdtCreate( &bcastEDT, PTR_rankTemplateH->TML_bcast, //FNC_bcast
                      EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &PTR_rankH->myEdtAffinityHNT, NULL );

        _idep = 0; _ichild = 0;
        ocrAddDependence( DBK_rankH, bcastEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( DBK_out, bcastEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( PTR_sharedOcrObjH->childrenRedObjects[_ichild++][r].DBK_out, bcastEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( PTR_sharedOcrObjH->childrenRedObjects[_ichild++][r].DBK_out, bcastEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( PTR_sharedOcrObjH->childrenRedObjects[_ichild++][r].DBK_out, bcastEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( PTR_sharedOcrObjH->childrenRedObjects[_ichild++][r].DBK_out, bcastEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( PTR_sharedOcrObjH->childrenRedObjects[_ichild++][r].DBK_out, bcastEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( PTR_sharedOcrObjH->childrenRedObjects[_ichild++][r].DBK_out, bcastEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( PTR_sharedOcrObjH->childrenRedObjects[_ichild++][r].DBK_out, bcastEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( PTR_sharedOcrObjH->childrenRedObjects[_ichild++][r].DBK_out, bcastEDT, _idep++, DB_MODE_RW );
    }

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_bcast )
{
    s32 _idep, _paramc, _depc;
    reducePRM_t* reducePRM = (reducePRM_t*) paramv;
    int irefine = reducePRM->irefine;
    int ts = reducePRM->ts;
    int phase = reducePRM->phase;
    int r = reducePRM->r;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid;
    ocrGuid_t DBK_out = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    void* PTR_out = depv[_idep++].ptr;

    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    DEBUG_PRINTF(( "%s ilevel %d id_l %d irefine %d phase %d r %d ts %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, irefine, phase, r, ts ));

     redObjects_t* PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];

    int i, j;
    size_t size;

    for( j = 0; j <= 7; j++ ) {
        void* PTR_outTmp = (void*) depv[2+j].ptr;

        size = reductionDBKsize( r, PTR_rankH );

        memcpy((char*)PTR_outTmp, (char*)PTR_out, size);

        ocrDbRelease( depv[2+j].guid );
        ocrEventSatisfy( PTR_sharedOcrObjH->childrenRedObjects[j][r].downIEVT, depv[2+j].guid );
    }

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_reduceAllRootNodes )
{
    s32 _idep, _paramc, _depc;
    reducePRM_t* reducePRM = (reducePRM_t*) paramv;
    int irefine = reducePRM->irefine;
    int ts = reducePRM->ts;
    int phase = reducePRM->phase;
    int r = reducePRM->r;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid;
    ocrGuid_t DBK_blockCounts = depv[_idep++].guid;
    ocrDBK_t DBK_redUpH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    void* PTR_in = depv[_idep++].ptr;
    reductionPrivate_t* PTR_redUpH = depv[_idep++].ptr;

    DEBUG_PRINTF(( "%s ilevel %d id_l %d irefine %d phase %d r %d ts %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, irefine, phase, r, ts ));

    reductionLaunch(PTR_redUpH, DBK_redUpH, PTR_in);

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_accumulator )
{
    s32 _idep, _paramc, _depc;
    reducePRM_t* reducePRM = (reducePRM_t*) paramv;
    int irefine = reducePRM->irefine;
    int ts = reducePRM->ts;
    int phase = reducePRM->phase;
    int r = reducePRM->r;

    _idep = 0;
    ocrGuid_t DBK_rankH = depv[_idep++].guid;
    ocrGuid_t DBK_in = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    void* PTR_in_void = depv[_idep++].ptr;

    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);

    DEBUG_PRINTF(( "%s ilevel %d id_l %d irefine %d phase %d r %d ts %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, irefine, phase, r, ts ));

    int i, j;

    size_t size;

    size = reductionDBKsize( r, PTR_rankH );

    if( (BLOCKCOUNT_RED_HANDLE_LB <= r && r <= BLOCKCOUNT_RED_HANDLE_UB) ||
        (BLOCKIDGATHER_RED_HANDLE_LB <= r && r <= BLOCKIDGATHER_RED_HANDLE_UB) ||
        (REFINEINTENT_RED_HANDLE_LB <= r && r <= REFINEINTENT_RED_HANDLE_UB) ||
        (QUIESCENCE_RED_HANDLE_LB <= r && r <= QUIESCENCE_RED_HANDLE_UB ) ||
        (COARSEN_RED_HANDLE_LB <= r && r <= COARSEN_RED_HANDLE_UB ) ) {

        int* PTR_in = (int*) PTR_in_void;
        memset(PTR_in, 0, size);

        for( j = 0; j <= 7; j++ ) {
            int* PTR_inTmp = (int*) depv[j+2].ptr;
            for( i = 0; i < size/sizeof(int); i++ ) {
                PTR_in[i] += PTR_inTmp[i];
            }
        }
    }

    if( CHECKSUM_RED_HANDLE_LB <= r && r <= CHECKSUM_RED_HANDLE_UB ) {
        double* PTR_in = (double*) PTR_in_void;
        memset(PTR_in, 0, size); //default

        for( j = 0; j <= 7; j++ ) {
            double* PTR_inTmp = (double*) depv[j+2].ptr;
            PTR_in[0] += PTR_inTmp[0];
        }
    }
    if( FINALIZE_RED_HANDLE_LB <= r && r <= FINALIZE_RED_HANDLE_UB ) {
        double* PTR_in = (double*) PTR_in_void;
        memset(PTR_in, 0, size); //default

        for( j = 0; j <= 7; j++ ) {
            double* PTR_inTmp = (double*) depv[j+2].ptr;
            PTR_in[0] = MAX(PTR_in[0],PTR_inTmp[0]);
        }
    }

    ocrDbRelease( DBK_in );

    redObjects_t* PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];

    ocrEVT_t redUpIEVT = PTR_redObjects->upIEVT;

    ocrEventSatisfy( redUpIEVT, DBK_in ); //All blocks provide partial sums

    return NULL_GUID;
}


