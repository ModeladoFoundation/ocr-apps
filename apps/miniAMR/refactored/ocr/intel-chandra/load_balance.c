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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "proto.h"
#include "timer.h"

_OCR_TASK_FNC_( FNC_loadbalance )
{
    loadbalancePRM_t* loadbalancePRM = (loadbalancePRM_t*) paramv;
    int ts = loadbalancePRM->ts;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;

    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    block *bp = &PTR_rankH->blockH;

    int ilevel = PTR_rankH->ilevel;

    ocrHNT_t myDbkAffinityHNT, myEdtAffinityHNT;
    myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    ocrTML_t TML_redistributeblocks = PTR_rankTemplateH->TML_redistributeblocks;
    ocrTML_t TML_idgather = PTR_rankTemplateH->TML_idgather;

    DEBUG_PRINTF(( "%s ilevel %d id_l %d ts %d\n", __func__, ilevel, PTR_rankH->myRank, PTR_rankH->ts ));

    //Launch blockIDgather reduction
    int r = BLOCKIDGATHER_RED_HANDLE_LB;
    redObjects_t* PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];

    ocrEVT_t redDownOEVT = PTR_redObjects->downOEVT;
    ocrDBK_t DBK_in = PTR_redObjects->DBK_in;

    ocrGuid_t idgatherEDT, idgatherOEVT, idgatherOEVTS;

    ocrEdtCreate( &idgatherEDT, TML_idgather,
                  EDT_PARAM_DEF, NULL, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &idgatherOEVT ); //FNC_idgather
    createEventHelper(&idgatherOEVTS, 1);
    ocrAddDependence( idgatherOEVT, idgatherOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, idgatherEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_in, idgatherEDT, _idep++, DB_MODE_RW );

    redistributeblocksPRM_t redistributeblocksPRM = {ts};
    ocrGuid_t redistributeblocksEDT;
    ocrEdtCreate( &redistributeblocksEDT, TML_redistributeblocks,
                  EDT_PARAM_DEF, (u64*)&redistributeblocksPRM, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL); //FNC_redistributeblocks

    _idep = 0;
    ocrAddDependence( DBK_rankH, redistributeblocksEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( redDownOEVT, redistributeblocksEDT, _idep++, DB_MODE_RO );
    ocrAddDependence( idgatherOEVTS, redistributeblocksEDT, _idep++, DB_MODE_NULL );

    return NULL_GUID;
}

_OCR_TASK_FNC_( FNC_redistributeblocks )
{
    redistributeblocksPRM_t* redistributeblocskPRM = (redistributeblocksPRM_t*) paramv;
    int ts = redistributeblocskPRM->ts;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_out = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    int* blockids_gathered = depv[_idep++].ptr;

    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    block *bp = &PTR_rankH->blockH;

    int ilevel = PTR_rankH->ilevel;

    printGatheredBlockIDs( PTR_rankH, blockids_gathered );

    int seqRank = binary_search_double( PTR_rankH->active_blockcount, PTR_rankH->myRank_g, PTR_rankH->active_blockids);

    PTR_rankH->seqRank = seqRank; //update here

    #ifdef PRINTBLOCKINFO
    PRINTF( "%s ilevel %d id_l %d myRank_g %d seqRank %d ts %d\n", __func__, ilevel, PTR_rankH->myRank, PTR_rankH->myRank_g, seqRank, PTR_rankH->ts );
    #endif

    ocrDbRelease( PTR_rankH->DBK_active_blockids );

    if(PTR_cmd->lb_opt) {
        if(!seqRank) {
            PRINTF("ts %d Doing load balancing\n", PTR_rankH->ts);
        }

        int newPD = mapBlocktoPD( PTR_rankH, PTR_cmd->lb_opt );

        ocrHint_t myEdtAffinityHNT, myDbkAffinityHNT; //New Affinities
        getAffinityHintsForDBandEdtAtPD( &myDbkAffinityHNT, &myEdtAffinityHNT, newPD );

        PTR_rankH->myEdtAffinityHNT = myEdtAffinityHNT;
        PTR_rankH->myDbkAffinityHNT = myDbkAffinityHNT;
    }

    return NULL_GUID;

}

int mapBlocktoPD( rankH_t* PTR_rankH, int lb_opt )
{
    u64 affinityCount = 1;
    ocrAffinityCount( AFFINITY_PD, &affinityCount );
    u64 PDS = affinityCount;

    int pd;

    if( lb_opt == 1 ) { //3D
        u64 edtGridDims[3];
        u64 pdGridDims[3];

        splitDimension_Cart3D(PTR_rankH->active_blockcount, &edtGridDims[0], &edtGridDims[1], &edtGridDims[2]);
        splitDimension_Cart3D(PDS, &pdGridDims[0], &pdGridDims[1], &pdGridDims[2] );

        pd = getPolicyDomainID_Cart3D( PTR_rankH->seqRank, edtGridDims, pdGridDims );

        #ifdef PRINTBLOCKINFO
        PRINTF( "EDT grid %dx%dx%d, PD grid %dx%dx%d: Block %d is at location %d to be ASSIGNED PD %d\n", edtGridDims[0], edtGridDims[1], edtGridDims[2], pdGridDims[0], pdGridDims[1], pdGridDims[2], PTR_rankH->myRank_g, PTR_rankH-.seqRank, pd );
        #endif
    }
    else { //1D
        u64 pdGridDims[1] = { PDS };
        u64 edtGridDims[1] = { PTR_rankH->active_blockcount };
        #ifdef PRINTBLOCKINFO
        pd = getPolicyDomainID_Cart1D( PTR_rankH->seqRank, edtGridDims, pdGridDims );
        PRINTF( "EDT grid %dx%dx%d, PD grid %dx%dx%d: Block %d is at location %d to be ASSIGNED PD %d\n", edtGridDims[0], 1, 1, pdGridDims[0], 1, 1, PTR_rankH->myRank_g, PTR_rankH->seqRank, pd );
        #endif
    }

    return pd;
}

void printGatheredBlockIDs( rankH_t* PTR_rankH, int* blockids_gathered )
{
    int i, j;
    PTR_rankH->active_blockcount = 0;

    #ifdef PRINTBLOCKINFO
    PRINTF("Active block ids:");
    #endif
    for( i = 0; i < PTR_rankH->max_possible_num_blocks; i++ ) {
        if( blockids_gathered[i] != 0 ){
    #ifdef PRINTBLOCKINFO
            PRINTF("%d ", blockids_gathered[i]-1);
    #endif
            PTR_rankH->active_blockcount++;
        }
    }
    #ifdef PRINTBLOCKINFO
    PRINTF("\n");
    #endif

    ocrDbCreate( &(PTR_rankH->DBK_active_blockids), (void **) &PTR_rankH->active_blockids,
                 PTR_rankH->active_blockcount*sizeof(int),
                 DB_PROP_NONE, &PTR_rankH->myDbkAffinityHNT, NO_ALLOC );

    for( i = 0, j = 0; i < PTR_rankH->max_possible_num_blocks; i++ ) {
        if( blockids_gathered[i] != 0 ){
            blockids_gathered[i] -= 1; //remove the offset
            PTR_rankH->active_blockids[j++] = blockids_gathered[i];
        }
    }
}

_OCR_TASK_FNC_( FNC_idgather )
{
    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_in = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    int* in = depv[_idep++].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    block *bp = &PTR_rankH->blockH;

    ocrTML_t TML_reduceAllUp = PTR_rankTemplateH->TML_reduceAllUp;
    ocrHNT_t myDbkAffinityHNT, myEdtAffinityHNT;
    myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    int ilevel = PTR_rankH->ilevel;
    int ts = PTR_rankH->ts;

    int r = BLOCKIDGATHER_RED_HANDLE_LB;
    redObjects_t* PTR_redObjects = &PTR_sharedOcrObjH->blockRedObjects[r];

    ocrEVT_t redDownOEVT = PTR_redObjects->downOEVT;
    ocrEVT_t redUpIEVT = PTR_redObjects->upIEVT;

    memset(in, 0, PTR_rankH->max_possible_num_blocks*sizeof(int));

    in[PTR_rankH->myRank_g] = (PTR_rankH->myRank_g+1); //ofset by 1

    DEBUG_PRINTF(( "%s ilevel %d id_l %d ts %d in %d\n", __func__, ilevel, PTR_rankH->myRank, PTR_rankH->ts, in[PTR_rankH->myRank_g]));

    ocrDbRelease( DBK_in );
    ocrEventSatisfy( redUpIEVT, DBK_in ); //All blocks provide partial sums

    ocrDbRelease( DBK_rankH );

    int phase = -22;
    reducePRM_t reducePRM = {-1, ts, phase, r};
    ocrGuid_t reduceAllUpEDT;

    ocrEdtCreate( &reduceAllUpEDT, TML_reduceAllUp, //FNC_reduceAllUp
                  EDT_PARAM_DEF, (u64*)&reducePRM, EDT_PARAM_DEF, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, reduceAllUpEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( redUpIEVT, reduceAllUpEDT, _idep++, DB_MODE_RW );

    return NULL_GUID;
}
