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

#include "block.h"
#include "proto.h"

_OCR_TASK_FNC_( FNC_comm )
{
    commPRM_t* commPRM = (commPRM_t*) paramv;

    int iAxis = commPRM->iAxis;
    int istart = commPRM->istart;
    int iend = commPRM->iend;
    int ts = commPRM->ts;
    int istage = commPRM->istage;

    s32 _idep, _paramc, _depc;
   int permutations[6][3] = { {0, 1, 2}, {1, 2, 0}, {2, 0, 1},
                              {0, 2, 1}, {1, 0, 2}, {2, 1, 0} };

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    block *bp = &PTR_rankH->blockH;

    int ilevel = PTR_rankH->ilevel;

    ocrHNT_t myDbkAffinityHNT, myEdtAffinityHNT;
    myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    ocrTML_t commHaloNbrsTML = PTR_rankTemplateH->commHaloNbrsTML;
    ocrTML_t TML_comm = PTR_rankTemplateH->TML_comm;

    DEBUG_PRINTF(( "%s ilevel %d id_l %d ts %d istage %d istart %d iend %d\n", __func__, ilevel, PTR_rankH->myRank, ts, istage, istart, iend ));

    int comm_stage = (ts-1)*PTR_cmd->stages_per_ts + istage;

    // Do halo-exchange along one axis
    if( PTR_cmd->permute )
        commPRM->iAxis = permutations[comm_stage%6][iAxis];
    else
        commPRM->iAxis = iAxis;

    ocrDbRelease( DBK_rankH );

    ocrGuid_t commHaloNbrsEDT, commHaloNbrsOEVT, commHaloNbrsOEVTS;

    ocrEdtCreate( &commHaloNbrsEDT, commHaloNbrsTML, //commHaloNbrsEdt
                  EDT_PARAM_DEF, (u64*)commPRM, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &myEdtAffinityHNT, &commHaloNbrsOEVT );

    createEventHelper( &commHaloNbrsOEVTS, 1);
    ocrAddDependence( commHaloNbrsOEVT, commHaloNbrsOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, commHaloNbrsEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( NULL_GUID, commHaloNbrsEDT, _idep++, DB_MODE_NULL );

    iAxis += 1;
    //paramv[1] = iAxis;

    if( iAxis < 3 )
    {
        //set up halo-exchange along the next axis
        commPRM->iAxis = iAxis; //unchaged
        ocrGuid_t commEDT;

        ocrEdtCreate( &commEDT, TML_comm,
                      EDT_PARAM_DEF, (u64*)commPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &myEdtAffinityHNT, NULL);

        _idep = 0;
        ocrAddDependence( DBK_rankH, commEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( commHaloNbrsOEVTS, commEDT, _idep++, DB_MODE_NULL );
    }

    return NULL_GUID;
}

_OCR_TASK_FNC_( commHaloNbrsEdt )
{
    commPRM_t* commPRM = (commPRM_t*) paramv;

    int iAxis = commPRM->iAxis; //maybe permuted direction
    int istart = commPRM->istart;
    int iend = commPRM->iend;
    int ts = commPRM->ts;
    int istage = commPRM->istage;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    block *bp = &PTR_rankH->blockH;

    int ilevel = PTR_rankH->ilevel;

    int nNbrs = blockNnbrs(bp, iAxis);

    ocrDBK_t DBK_array = bp->DBK_array;

    DEBUG_PRINTF(( "%s ilevel %d id_l %d ts %d istage %d istart %d iend %d iAxis %d nNbrs %d\n", __func__, ilevel, PTR_rankH->myRank, ts, istage, istart, iend, iAxis, nNbrs ));

    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);

    doubleBufferedOcrObj_t* PTR_dBufH1 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[0]);
    doubleBufferedOcrObj_t* PTR_dBufH2 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[1]);

    int i, j;
    int comm_stage = (ts-1)*PTR_cmd->stages_per_ts + istage;
    int phase = (comm_stage)%2;

    ocrHNT_t myDbkAffinityHNT, myEdtAffinityHNT;
    myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    ocrTML_t packHalosTML = PTR_rankTemplateH->packHalosTML;
    ocrTML_t unpackHalosTML = PTR_rankTemplateH->unpackHalosTML;

    ocrDBK_t haloCurrSendDBKs[6][2], haloCoarSendDBKs[6][2], haloRefnSendDBKs[6][4][2];
    ocrDBK_t haloCurrRecvEVTs[6], haloCoarRecvEVTs[6], haloRefnRecvEVTs[6][4];

    int nei_level[6];
    int level = bp->level;

    for (i = 2*iAxis; i < 2*(iAxis+1); i++) {
        nei_level[i] = bp->nei_level[i];
        if( nei_level[i] == level ) {
            haloCurrSendDBKs[i][phase] = PTR_dBufH1->haloCurrSendDBKs[i][phase];
            haloCurrRecvEVTs[i] = PTR_dBufH1->haloCurrRecvEVTs[i];
        }
        else if( nei_level[i] == level-1 ) {
            haloCoarSendDBKs[i][phase] = PTR_dBufH1->haloCoarSendDBKs[i][phase];
            haloCoarRecvEVTs[i] = PTR_dBufH1->haloCoarRecvEVTs[i];
        }
        else if( nei_level[i] == level+1 ) {
            for( j = 0; j < 4; j++ ) {
                haloRefnSendDBKs[i][j][phase] = PTR_dBufH1->haloRefnSendDBKs[i][j][phase];
                haloRefnRecvEVTs[i][j] = PTR_dBufH1->haloRefnRecvEVTs[i][j];
            }
        }
    }

    ocrDbRelease(DBK_rankH);

    ocrGuid_t packHalosEDT, packHalosOEVT, packHalosOEVTS;

    ocrEdtCreate( &packHalosEDT, packHalosTML, //packHalosEdt
                  EDT_PARAM_DEF, (u64*)commPRM, nNbrs+2, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &packHalosOEVT );

    createEventHelper( &packHalosOEVTS, 1);
    ocrAddDependence( packHalosOEVT, packHalosOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, packHalosEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_array, packHalosEDT, _idep++, DB_MODE_RW );

    for (i = 2*iAxis; i < 2*(iAxis+1); i++) {
        if( nei_level[i] == level ) {
            ocrAddDependence( haloCurrSendDBKs[i][phase], packHalosEDT, _idep++, DB_MODE_RW );
        }
        else if( nei_level[i] == level-1 ) {
            ocrAddDependence( haloCoarSendDBKs[i][phase], packHalosEDT, _idep++, DB_MODE_RW );
        }
        else if( nei_level[i] == level+1 ) {
            for( j = 0; j < 4; j++ ) {
                ocrAddDependence( haloRefnSendDBKs[i][j][phase], packHalosEDT, _idep++, DB_MODE_RW );
            }
        }
        else if( nei_level[i] == -2 ) { //BOUNDARY
        }
        else {
            PRINTF("SOMETHING WENT WRONG!! UNBALANCED refinement!\n");
        }
    }

    ocrGuid_t unpackHalosEDT;

    ocrEdtCreate( &unpackHalosEDT, unpackHalosTML, //unpackHalosEdt
                  EDT_PARAM_DEF, (u64*)commPRM, nNbrs+3, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL);

    _idep = 0;
    ocrAddDependence( DBK_rankH, unpackHalosEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( DBK_array, unpackHalosEDT, _idep++, DB_MODE_RW );

    for (i = 2*iAxis; i < 2*(iAxis+1); i++) {
        if( nei_level[i] == level ) {
            ocrAddDependence( haloCurrRecvEVTs[i], unpackHalosEDT, _idep++, DB_MODE_RW );
            DEBUG_PRINTF(( "%s face %d dep %d "GUIDF" \n", __func__, i, _idep-1, haloCurrRecvEVTs[i] ));
        }
        else if( nei_level[i] == level-1 ) {
            ocrAddDependence( haloCoarRecvEVTs[i], unpackHalosEDT, _idep++, DB_MODE_RW );
            DEBUG_PRINTF(( "%s face %d dep %d "GUIDF" \n", __func__, i, _idep-1, haloCoarRecvEVTs[i] ));
        }
        else if( nei_level[i] == level+1 ) {
            for( j = 0; j < 4; j++ ) {
                ocrAddDependence( haloRefnRecvEVTs[i][j], unpackHalosEDT, _idep++, DB_MODE_RW );
                DEBUG_PRINTF(( "%s face %d dep %d "GUIDF" \n", __func__, i, _idep-1, haloRefnRecvEVTs[i][j] ));
            }
        }
        else if( nei_level[i] == -2 ) { //BOUNDARY
        }
        else {
            PRINTF("SOMETHING WENT WRONG!! UNBALANCED refinement!\n");
        }
    }
    ocrAddDependence( packHalosOEVTS, unpackHalosEDT, _idep++, DB_MODE_NULL ); //TODO - is this really needed?

    return NULL_GUID;
}

_OCR_TASK_FNC_( packHalosEdt )
{
    commPRM_t* commPRM = (commPRM_t*) paramv;

    int iAxis = commPRM->iAxis; //maybe permuted direction
    int istart = commPRM->istart;
    int iend = commPRM->iend;
    int ts = commPRM->ts;
    int istage = commPRM->istage;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_array = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    double* array      = depv[_idep++].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    block *bp = &PTR_rankH->blockH;

    bp->array = array;

    int ilevel = PTR_rankH->ilevel;

    int nNbrs = blockNnbrs(bp, iAxis);

    DEBUG_PRINTF(( "%s ilevel %d id_l %d ts %d istage %d istart %d iend %d iAxis %d nNbrs %d\n", __func__, ilevel, PTR_rankH->myRank, ts, istage, istart, iend, iAxis, nNbrs ));

    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);

    doubleBufferedOcrObj_t* PTR_dBufH1 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[0]);
    doubleBufferedOcrObj_t* PTR_dBufH2 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[1]);

    int comm_stage = (ts-1)*PTR_cmd->stages_per_ts + istage;
    int phase = comm_stage%2;
    int i, j;
    int fcase;

    double* send_buf;

    int f;
    if (PTR_cmd->stencil == 7)  // add to face case when diags are needed
       f = 0;
    else
       f = 1;

    reset_haloSendPTRs(PTR_rankH, iAxis, phase);

    int num_comm = iend-istart+1;

    for (i = 2*iAxis; i < 2*(iAxis+1); i++) {
        fcase = (i%2)*10;

        if( bp->nei_level[i] == bp->level ) {
            PTR_dBufH1->haloCurrSendDBKs[i][phase] = depv[_idep].guid;
            PTR_dBufH1->haloCurrSendPTRs[i][phase] = (double*) depv[_idep++].ptr;

            send_buf = PTR_dBufH1->haloCurrSendPTRs[i][phase];

            pack_face( PTR_rankH, send_buf, fcase+f, iAxis, istart, num_comm );

            ocrDbRelease( PTR_dBufH1->haloCurrSendDBKs[i][phase] );
            ocrEventSatisfy( PTR_dBufH1->haloCurrSendEVTs[i], PTR_dBufH1->haloCurrSendDBKs[i][phase] );
            DEBUG_PRINTF(( "%s face %d dep %d "GUIDF" \n", __func__, i, _idep-1, PTR_dBufH1->haloCurrSendEVTs[i] ));
        }
        else if( bp->nei_level[i] == bp->level-1 ) {
            PTR_dBufH1->haloCoarSendDBKs[i][phase] = depv[_idep].guid;
            PTR_dBufH1->haloCoarSendPTRs[i][phase] = (double*) depv[_idep++].ptr;

            send_buf = PTR_dBufH1->haloCoarSendPTRs[i][phase];

            pack_face( PTR_rankH, send_buf, fcase+2, iAxis, istart, num_comm ); //face+[2,3,4,5] // case doesn't matter

            ocrDbRelease( PTR_dBufH1->haloCoarSendDBKs[i][phase] );
            ocrEventSatisfy( PTR_dBufH1->haloCoarSendEVTs[i], PTR_dBufH1->haloCoarSendDBKs[i][phase] );
            DEBUG_PRINTF(( "%s face %d dep %d "GUIDF" \n", __func__, i, _idep-1, PTR_dBufH1->haloCoarSendEVTs[i] ));
        }
        else if( bp->nei_level[i] == bp->level+1 ) {
            for( j = 0; j < 4; j++ ) {
                PTR_dBufH1->haloRefnSendDBKs[i][j][phase] = depv[_idep].guid;
                PTR_dBufH1->haloRefnSendPTRs[i][j][phase] = (double*) depv[_idep++].ptr;

                send_buf = PTR_dBufH1->haloRefnSendPTRs[i][j][phase];

                pack_face( PTR_rankH, send_buf, fcase+6+j, iAxis, istart, num_comm );

                ocrDbRelease( PTR_dBufH1->haloRefnSendDBKs[i][j][phase] );
                ocrEventSatisfy( PTR_dBufH1->haloRefnSendEVTs[i][j], PTR_dBufH1->haloRefnSendDBKs[i][j][phase] );
                DEBUG_PRINTF(( "%s face %d dep %d "GUIDF" \n", __func__, i, _idep-1, PTR_dBufH1->haloRefnSendEVTs[i][j] ));
            }
        }
        else if( bp->nei_level[i] == -2 ) { //BOUNDARY
        }
        else {
            PRINTF("SOMETHING WENT WRONG!! UNBALANCED refinement!\n");
        }
    }

    return NULL_GUID;
}

void reset_haloSendPTRs( rankH_t *PTR_rankH, int iAxis, int phase )
{
    block *bp = &PTR_rankH->blockH;
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);

    doubleBufferedOcrObj_t* PTR_dBufH1 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[0]);
    doubleBufferedOcrObj_t* PTR_dBufH2 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[1]);

    int i, j;

    for (i = 2*iAxis; i < 2*(iAxis+1); i++) {
        PTR_dBufH1->haloCurrSendPTRs[i][phase] = NULL;
        PTR_dBufH1->haloCoarSendPTRs[i][phase] = NULL;

        for( j = 0; j < 4; j++ ) {
            PTR_dBufH1->haloRefnSendPTRs[i][j][phase] = NULL;
        }
    }
}

void reset_haloRecvPTRs( rankH_t *PTR_rankH, int iAxis, int phase )
{
    block *bp = &PTR_rankH->blockH;
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);

    doubleBufferedOcrObj_t* PTR_dBufH1 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[0]);
    doubleBufferedOcrObj_t* PTR_dBufH2 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[1]);

    int i, j;

    for (i = 2*iAxis; i < 2*(iAxis+1); i++) {
        PTR_dBufH1->haloCurrRecvPTRs[i][phase] = NULL;
        PTR_dBufH1->haloCoarRecvPTRs[i][phase] = NULL;
        for( j = 0; j < 4; j++ ) {
            PTR_dBufH1->haloRefnRecvPTRs[i][j][phase] = NULL;
        }
    }
}

_OCR_TASK_FNC_( unpackHalosEdt )
{
    commPRM_t* commPRM = (commPRM_t*) paramv;

    int iAxis = commPRM->iAxis; //maybe permuted direction
    int istart = commPRM->istart;
    int iend = commPRM->iend;
    int ts = commPRM->ts;
    int istage = commPRM->istage;

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;
    ocrDBK_t DBK_array = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    double* array      = depv[_idep++].ptr;

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    block *bp = &PTR_rankH->blockH;

    bp->array = array;

    int ilevel = PTR_rankH->ilevel;

    int nNbrs = blockNnbrs(bp, iAxis);

    DEBUG_PRINTF(( "%s ilevel %d id_l %d ts %d istage %d istart %d iend %d iAxis %d nNbrs %d\n", __func__, ilevel, PTR_rankH->myRank, ts, istage, istart, iend, iAxis, nNbrs ));

    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);

    doubleBufferedOcrObj_t* PTR_dBufH1 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[0]);
    doubleBufferedOcrObj_t* PTR_dBufH2 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[1]);

    int i, j;
    int fcase;

    double* recv_buf;

    int f;
    if (PTR_cmd->stencil == 7)  // add to face case when diags are needed
       f = 0;
    else
       f = 1;

    int num_comm = iend-istart+1;

    int comm_stage = (ts-1)*PTR_cmd->stages_per_ts + istage;
    int phase = comm_stage%2;
    reset_haloRecvPTRs(PTR_rankH, iAxis, phase);

    for (i = 2*iAxis; i < 2*(iAxis+1); i++) {
        fcase = (i%2)*10;

        if( bp->nei_level[i] == bp->level ) {
            PTR_dBufH1->haloCurrRecvPTRs[i][phase] = (double*) depv[_idep++].ptr;

            recv_buf = PTR_dBufH1->haloCurrRecvPTRs[i][phase];

            unpack_face( PTR_rankH, recv_buf, fcase+f, iAxis, istart, num_comm );

            DEBUG_PRINTF(( "%s dep %d "GUIDF" \n", __func__, _idep-1, PTR_dBufH1->haloCurrRecvEVTs[i] ));
        }
        else if( bp->nei_level[i] == bp->level-1 ) {
            PTR_dBufH1->haloCoarRecvPTRs[i][phase] = (double*) depv[_idep++].ptr;

            recv_buf = PTR_dBufH1->haloCoarRecvPTRs[i][phase];

            unpack_face( PTR_rankH, recv_buf, fcase+2, iAxis, istart, num_comm ); //face+[2,3,4,5] // case doesn't matter

            DEBUG_PRINTF(( "%s dep %d "GUIDF" \n", __func__, _idep-1, PTR_dBufH1->haloCoarRecvEVTs[i] ));
        }
        else if( bp->nei_level[i] == bp->level+1 ) {
            for( j = 0; j < 4; j++ ) {
                PTR_dBufH1->haloRefnRecvPTRs[i][j][phase] = (double*) depv[_idep++].ptr;

                recv_buf = PTR_dBufH1->haloRefnRecvPTRs[i][j][phase];

                unpack_face( PTR_rankH, recv_buf, fcase+6+j, iAxis, istart, num_comm );

                DEBUG_PRINTF(( "%s dep %d "GUIDF" \n", __func__, _idep-1, PTR_dBufH1->haloRefnRecvEVTs[i][j] ));
            }
        }
        else if( bp->nei_level[i] == -2 ) { //BOUNDARY
            apply_bc(PTR_rankH, i, istart, num_comm);
        }
        else {
            PRINTF("SOMETHING WENT WRONG!! UNBALANCED refinement!\n");
        }
    }

    return NULL_GUID;
}

// Pack face to send - note different cases for different directions.
void pack_face( rankH_t* PTR_rankH, double *send_buf, int face_case, int dir, int start, int num_comm )
{
   int i, j, k, n, m;
   int is, ie, js, je, ks, ke;

   #if 1

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    block *bp = &PTR_rankH->blockH;

    int x_block_size = PTR_cmd->x_block_size;
    int y_block_size = PTR_cmd->y_block_size;
    int z_block_size = PTR_cmd->z_block_size;

    int x_block_half = PTR_rankH->x_block_half;
    int y_block_half = PTR_rankH->y_block_half;
    int z_block_half = PTR_rankH->z_block_half;

   if (!PTR_cmd->code) {

      if (dir == 0) {        /* X - East, West */

         /* X directions (East and West) sent first, so just send
            the real values and no ghosts
         */
         if (face_case >= 10) { /* +X - East */
            i = x_block_size;
            face_case = face_case - 10;
         } else                 /* -X - West */
            i = 1;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 1; j <= y_block_size; j++)
                  for (k = 1; k <= z_block_size; k++, n++)
                     send_buf[n] = IN(bp,m,i,j,k);
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face - case does not matter */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 1; j < y_block_size; j += 2)
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = IN(bp,m,i,j  ,k  ) +
                                   IN(bp,m,i,j  ,k+1) +
                                   IN(bp,m,i,j+1,k  ) +
                                   IN(bp,m,i,j+1,k+1);
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               js = 1;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size;
            }
            if ((face_case/2)%2 == 1) {
               ks = 1;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = js; j <= je; j++)
                  for (k = ks; k <= ke; k++, n++)
                     send_buf[n] = IN(bp,m,i,j,k)/4.0;
         }

      } else if (dir == 1) { /* Y - North, South */

         /* Y directions (North and South) sent second, so send the real values
         */
         if (face_case >= 10) { /* +Y - North */
            j = y_block_size;
            face_case = face_case - 10;
         } else                 /* -Y - South */
            j = 1;
         if (face_case == 0) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i <= x_block_size; i++)
                  for (k = 1; k <= z_block_size; k++, n++)
                     send_buf[n] = IN(bp,m,i,j,k);
         } else if (face_case == 1) {
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (k = 1; k <= z_block_size; k++, n++)
                     send_buf[n] = IN(bp,m,i,j,k);
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face - case does not matter */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i < x_block_size; i += 2)
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = IN(bp,m,i  ,j,k  ) +
                                   IN(bp,m,i ,j,k+1) +
                                   IN(bp,m,i+1,j,k  ) +
                                   IN(bp,m,i+1,j,k+1);
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               is = 1;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size;
            }
            if ((face_case/2)%2 == 1) {
               ks = 1;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (k = ks; k <= ke; k++, n++)
                     send_buf[n] = IN(bp,m,i,j,k)/4.0;
         }

      } else {               /* Z - Up, Down */

         /* Z directions (Up and Down) sent last
         */
         if (face_case >= 10) { /* +Z - Up */
            k = z_block_size;
            face_case = face_case - 10;
         } else                 /* -Z - Down */
            k = 1;
         if (face_case == 0) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i <= x_block_size; i++)
                  for (j = 1; j <= y_block_size; j++, n++)
                     send_buf[n] = IN(bp,m,i,j,k);
         } else if (face_case == 1) {
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (j = 0; j <= y_block_size+1; j++, n++)
                     send_buf[n] = IN(bp,m,i,j,k);
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face - case does not matter */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i < x_block_size; i += 2)
                  for (j = 1; j < y_block_size; j += 2, n++)
                     send_buf[n] = IN(bp,m,i  ,j  ,k) +
                                   IN(bp,m,i  ,j+1,k) +
                                   IN(bp,m,i+1,j  ,k) +
                                   IN(bp,m,i+1,j+1,k);
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               is = 1;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size;
            }
            if ((face_case/2)%2 == 1) {
               js = 1;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (j = js; j <= je; j++, n++)
                     send_buf[n] = IN(bp,m,i,j,k)/4.0;
         }
      }

   } else if (PTR_cmd->code == 1) { /* send all ghosts */

      if (dir == 0) {        /* X - East, West */

         if (face_case >= 10) { /* +X - East */
            i = x_block_size;
            face_case = face_case - 10;
         } else                 /* -X - West */
            i = 1;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 0; j <= y_block_size+1; j++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     send_buf[n] = IN(bp,m,i,j,k);
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face */
            for (n = 0, m = start; m < start+num_comm; m++) {
               if (face_case%2 == 0) {
                  j = 0;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = IN(bp,m,i,j,k  ) +
                                   IN(bp,m,i,j,k+1);
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
               }
               for (j = 1; j < y_block_size; j += 2) {
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = IN(bp,m,i,j  ,k) +
                                     IN(bp,m,i,j+1,k);
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = IN(bp,m,i,j  ,k  ) +
                                   IN(bp,m,i,j  ,k+1) +
                                   IN(bp,m,i,j+1,k  ) +
                                   IN(bp,m,i,j+1,k+1);
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = IN(bp,m,i,j  ,k) +
                                     IN(bp,m,i,j+1,k);
                  }
               }
               if (face_case%2 == 1) {
                  j = y_block_size + 1;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = IN(bp,m,i,j,k  ) +
                                   IN(bp,m,i,j,k+1);
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
               }
            }
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               js = 0;
               je = y_block_half + 1;
            } else {
               js = y_block_half;
               je = y_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               ks = 0;
               ke = z_block_half + 1;
            } else {
               ks = z_block_half;
               ke = z_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = js; j <= je; j++)
                  for (k = ks; k <= ke; k++, n++)
                     send_buf[n] = IN(bp,m,i,j,k)/4.0;
         }

      } else if (dir == 1) { /* Y - North, South */

         if (face_case >= 10) { /* +Y - North */
            j = y_block_size;
            face_case = face_case - 10;
         } else                 /* -Y - South */
            j = 1;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     send_buf[n] = IN(bp,m,i,j,k);
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face */
            for (n = 0, m = start; m < start+num_comm; m++) {
               if (face_case%2 == 0) {
                  i = 0;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = IN(bp,m,i,j,k  ) +
                                   IN(bp,m,i,j,k+1);
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
               }
               for (i = 1; i < x_block_size; i += 2) {
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = IN(bp,m,i  ,j,k) +
                                     IN(bp,m,i+1,j,k);
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = IN(bp,m,i  ,j,k  ) +
                                   IN(bp,m,i ,j,k+1) +
                                   IN(bp,m,i+1,j,k  ) +
                                   IN(bp,m,i+1,j,k+1);
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = IN(bp,m,i  ,j,k) +
                                     IN(bp,m,i+1,j,k);
                  }
               }
               if (face_case%2 == 1) {
                  i = x_block_size + 1;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = IN(bp,m,i,j,k  ) +
                                   IN(bp,m,i,j,k+1);
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
               }
            }
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               is = 0;
               ie = x_block_half + 1;
            } else {
               is = x_block_half;
               ie = x_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               ks = 0;
               ke = z_block_half + 1;
            } else {
               ks = z_block_half;
               ke = z_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (k = ks; k <= ke; k++, n++)
                     send_buf[n] = IN(bp,m,i,j,k)/4.0;
         }

      } else {               /* Z - Up, Down */

         /* Z directions (Up and Down) sent last
         */
         if (face_case >= 10) { /* +Z - Up */
            k = z_block_size;
            face_case = face_case - 10;
         } else                 /* -Z - Down */
            k = 1;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (j = 0; j <= y_block_size+1; j++, n++)
                     send_buf[n] = IN(bp,m,i,j,k);
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face - case does not matter */
            for (n = 0, m = start; m < start+num_comm; m++) {
               if (face_case%2 == 0) {
                  i = 0;
                  if ((face_case/2)%2 == 1) {
                     j = 0;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
                  for (j = 1; j < y_block_size; j += 2, n++)
                     send_buf[n] = IN(bp,m,i,j  ,k) +
                                   IN(bp,m,i,j+1,k);
                  if ((face_case/2)%2 == 0) {
                     j = y_block_size + 1;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
               }
               for (i = 1; i < x_block_size; i += 2) {
                  if ((face_case/2)%2 == 1) {
                     j = 0;
                     send_buf[n++] = IN(bp,m,i  ,j,k) +
                                     IN(bp,m,i+1,j,k);
                  }
                  for (j = 1; j < y_block_size; j += 2, n++)
                     send_buf[n] = IN(bp,m,i  ,j  ,k) +
                                   IN(bp,m,i  ,j+1,k) +
                                   IN(bp,m,i+1,j  ,k) +
                                   IN(bp,m,i+1,j+1,k);
                  if ((face_case/2)%2 == 0) {
                     j = y_block_size + 1;
                     send_buf[n++] = IN(bp,m,i  ,j,k) +
                                     IN(bp,m,i+1,j,k);
                  }
               }
               if (face_case%2 == 1) {
                  i = x_block_size + 1;
                  if ((face_case/2)%2 == 1) {
                     j = 0;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
                  for (j = 1; j < y_block_size; j += 2, n++)
                     send_buf[n] = IN(bp,m,i,j  ,k) +
                                   IN(bp,m,i,j+1,k);
                  if ((face_case/2)%2 == 0) {
                     j = y_block_size + 1;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
               }
            }
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               is = 0;
               ie = x_block_half + 1;
            } else {
               is = x_block_half;
               ie = x_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               js = 0;
               je = y_block_half + 1;
            } else {
               js = y_block_half;
               je = y_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (j = js; j <= je; j++, n++)
                     send_buf[n] = IN(bp,m,i,j,k)/4.0;
         }
      }

   } else { /* code == 2 send all ghosts and do all processing on send side */

      if (dir == 0) {        /* X - East, West */

         if (face_case >= 10) { /* +X - East */
            i = x_block_size;
            face_case = face_case - 10;
         } else                 /* -X - West */
            i = 1;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 0; j <= y_block_size+1; j++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     send_buf[n] = IN(bp,m,i,j,k);
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face */
            for (n = 0, m = start; m < start+num_comm; m++) {
               if (face_case%2 == 0) {
                  j = 0;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = IN(bp,m,i,j,k  ) +
                                   IN(bp,m,i,j,k+1);
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
               }
               for (j = 1; j < y_block_size; j += 2) {
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = IN(bp,m,i,j  ,k) +
                                     IN(bp,m,i,j+1,k);
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = IN(bp,m,i,j  ,k  ) +
                                   IN(bp,m,i,j  ,k+1) +
                                   IN(bp,m,i,j+1,k  ) +
                                   IN(bp,m,i,j+1,k+1);
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = IN(bp,m,i,j  ,k) +
                                     IN(bp,m,i,j+1,k);
                  }
               }
               if (face_case%2 == 1) {
                  j = y_block_size + 1;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = IN(bp,m,i,j,k  ) +
                                   IN(bp,m,i,j,k+1);
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
               }
            }
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               js = 1;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size;
            }
            if ((face_case/2)%2 == 1) {
               ks = 1;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++) {
               j = js - 1;
               k = ks - 1;
               send_buf[n++] = IN(bp,m,i,j,k)/4.0;
               for (k = ks; k <= ke; k++, n+=2)
                  send_buf[n] = send_buf[n+1] = IN(bp,m,i,j,k)/4.0;
               k = ke + 1;
               send_buf[n++] = IN(bp,m,i,j,k)/4.0;
               for (j = js; j <= je; j++) {
                  k = ks - 1;
                  send_buf[n++] = IN(bp,m,i,j,k)/4.0;
                  for (k = ks; k <= ke; k++, n+=2)
                     send_buf[n] = send_buf[n+1] = IN(bp,m,i,j,k)/4.0;
                  k = ke + 1;
                  send_buf[n++] = IN(bp,m,i,j,k)/4.0;
                  k = ks - 1;
                  send_buf[n++] = IN(bp,m,i,j,k)/4.0;
                  for (k = ks; k <= ke; k++, n+=2)
                     send_buf[n] = send_buf[n+1] = IN(bp,m,i,j,k)/4.0;
                  k = ke + 1;
                  send_buf[n++] = IN(bp,m,i,j,k)/4.0;
               }
               j = je + 1;
               k = ks - 1;
               send_buf[n++] = IN(bp,m,i,j,k)/4.0;
               for (k = ks; k <= ke; k++, n+=2)
                  send_buf[n] = send_buf[n+1] = IN(bp,m,i,j,k)/4.0;
               k = ke + 1;
               send_buf[n++] = IN(bp,m,i,j,k)/4.0;
            }
         }

      } else if (dir == 1) { /* Y - North, South */

         if (face_case >= 10) { /* +Y - North */
            j = y_block_size;
            face_case = face_case - 10;
         } else                 /* -Y - South */
            j = 1;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     send_buf[n] = IN(bp,m,i,j,k);
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face */
            for (n = 0, m = start; m < start+num_comm; m++) {
               if (face_case%2 == 0) {
                  i = 0;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = IN(bp,m,i,j,k  ) +
                                   IN(bp,m,i,j,k+1);
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
               }
               for (i = 1; i < x_block_size; i += 2) {
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = IN(bp,m,i  ,j,k) +
                                     IN(bp,m,i+1,j,k);
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = IN(bp,m,i  ,j,k  ) +
                                   IN(bp,m,i ,j,k+1) +
                                   IN(bp,m,i+1,j,k  ) +
                                   IN(bp,m,i+1,j,k+1);
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = IN(bp,m,i  ,j,k) +
                                     IN(bp,m,i+1,j,k);
                  }
               }
               if (face_case%2 == 1) {
                  i = x_block_size + 1;
                  if ((face_case/2)%2 == 1) {
                     k = 0;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
                  for (k = 1; k < z_block_size; k += 2, n++)
                     send_buf[n] = IN(bp,m,i,j,k  ) +
                                   IN(bp,m,i,j,k+1);
                  if ((face_case/2)%2 == 0) {
                     k = z_block_size + 1;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
               }
            }
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               is = 1;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size;
            }
            if ((face_case/2)%2 == 1) {
               ks = 1;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++) {
               i = is - 1;
               k = ks - 1;
               send_buf[n++] = IN(bp,m,i,j,k)/4.0;
               for (k = ks; k <= ke; k++, n+=2)
                  send_buf[n] = send_buf[n+1] = IN(bp,m,i,j,k)/4.0;
               k = ke + 1;
               send_buf[n++] = IN(bp,m,i,j,k)/4.0;
               for (i = is; i <= ie; i++) {
                  k = ks - 1;
                  send_buf[n++] = IN(bp,m,i,j,k)/4.0;
                  for (k = ks; k <= ke; k++, n+=2)
                     send_buf[n] = send_buf[n+1] = IN(bp,m,i,j,k)/4.0;
                  k = ke + 1;
                  send_buf[n++] = IN(bp,m,i,j,k)/4.0;
                  k = ks - 1;
                  send_buf[n++] = IN(bp,m,i,j,k)/4.0;
                  for (k = ks; k <= ke; k++, n+=2)
                     send_buf[n] = send_buf[n+1] = IN(bp,m,i,j,k)/4.0;
                  k = ke + 1;
                  send_buf[n++] = IN(bp,m,i,j,k)/4.0;
               }
               i = ie + 1;
               k = ks - 1;
               send_buf[n++] = IN(bp,m,i,j,k)/4.0;
               for (k = ks; k <= ke; k++, n+=2)
                  send_buf[n] = send_buf[n+1] = IN(bp,m,i,j,k)/4.0;
               k = ke + 1;
               send_buf[n++] = IN(bp,m,i,j,k)/4.0;
            }
         }

      } else {               /* Z - Up, Down */

         /* Z directions (Up and Down) sent last
         */
         if (face_case >= 10) { /* +Z - Up */
            k = z_block_size;
            face_case = face_case - 10;
         } else                 /* -Z - Down */
            k = 1;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (j = 0; j <= y_block_size+1; j++, n++)
                     send_buf[n] = IN(bp,m,i,j,k);
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face - case does not matter */
            for (n = 0, m = start; m < start+num_comm; m++) {
               if (face_case%2 == 0) {
                  i = 0;
                  if ((face_case/2)%2 == 1) {
                     j = 0;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
                  for (j = 1; j < y_block_size; j += 2, n++)
                     send_buf[n] = IN(bp,m,i,j  ,k) +
                                   IN(bp,m,i,j+1,k);
                  if ((face_case/2)%2 == 0) {
                     j = y_block_size + 1;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
               }
               for (i = 1; i < x_block_size; i += 2) {
                  if ((face_case/2)%2 == 1) {
                     j = 0;
                     send_buf[n++] = IN(bp,m,i  ,j,k) +
                                     IN(bp,m,i+1,j,k);
                  }
                  for (j = 1; j < y_block_size; j += 2, n++)
                     send_buf[n] = IN(bp,m,i  ,j  ,k) +
                                   IN(bp,m,i  ,j+1,k) +
                                   IN(bp,m,i+1,j  ,k) +
                                   IN(bp,m,i+1,j+1,k);
                  if ((face_case/2)%2 == 0) {
                     j = y_block_size + 1;
                     send_buf[n++] = IN(bp,m,i  ,j,k) +
                                     IN(bp,m,i+1,j,k);
                  }
               }
               if (face_case%2 == 1) {
                  i = x_block_size + 1;
                  if ((face_case/2)%2 == 1) {
                     j = 0;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
                  for (j = 1; j < y_block_size; j += 2, n++)
                     send_buf[n] = IN(bp,m,i,j  ,k) +
                                   IN(bp,m,i,j+1,k);
                  if ((face_case/2)%2 == 0) {
                     j = y_block_size + 1;
                     send_buf[n++] = IN(bp,m,i,j,k);
                  }
               }
            }
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to send */
            if (face_case%2 == 0) {
               is = 1;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size;
            }
            if ((face_case/2)%2 == 1) {
               js = 1;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++) {
               i = is - 1;
               j = js - 1;
               send_buf[n++] = IN(bp,m,i,j,k)/4.0;
               for (j = js; j <= je; j++, n+=2)
                  send_buf[n] = send_buf[n+1] = IN(bp,m,i,j,k)/4.0;
               j = je + 1;
               send_buf[n++] = IN(bp,m,i,j,k)/4.0;
               for (i = is; i <= ie; i++) {
                  j = js - 1;
                  send_buf[n++] = IN(bp,m,i,j,k)/4.0;
                  for (j = js; j <= je; j++, n+=2)
                     send_buf[n] = send_buf[n+1] = IN(bp,m,i,j,k)/4.0;
                  j = je + 1;
                  send_buf[n++] = IN(bp,m,i,j,k)/4.0;
                  j = js - 1;
                  send_buf[n++] = IN(bp,m,i,j,k)/4.0;
                  for (j = js; j <= je; j++, n+=2)
                     send_buf[n] = send_buf[n+1] = IN(bp,m,i,j,k)/4.0;
                  j = je + 1;
                  send_buf[n++] = IN(bp,m,i,j,k)/4.0;
               }
               i = ie + 1;
               j = js - 1;
               send_buf[n++] = IN(bp,m,i,j,k)/4.0;
               for (j = js; j <= je; j++, n+=2)
                  send_buf[n] = send_buf[n+1] = IN(bp,m,i,j,k)/4.0;
               j = je + 1;
               send_buf[n++] = IN(bp,m,i,j,k)/4.0;
            }
         }
      }
   }

   #endif
}

// Unpack ghost values that have been recieved.
// The sense of the face case is reversed since we are receiving what was sent
void unpack_face( rankH_t* PTR_rankH, double *recv_buf, int face_case, int dir, int start, int num_comm )
{
   int i, j, k, n, m;
   int is, ie, js, je, ks, ke;

#if 1

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    block *bp = &PTR_rankH->blockH;

    int x_block_size = PTR_cmd->x_block_size;
    int y_block_size = PTR_cmd->y_block_size;
    int z_block_size = PTR_cmd->z_block_size;

    int x_block_half = PTR_rankH->x_block_half;
    int y_block_half = PTR_rankH->y_block_half;
    int z_block_half = PTR_rankH->z_block_half;

   if (!PTR_cmd->code) {

      if (dir == 0) {        /* X - East, West */

         /* X directions (East and West)
            just recv the real values and no ghosts
            face_case based on send - so reverse
         */
         if (face_case >= 10) { /* +X - from East */
            i = x_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -X - from West */
            i = 0;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 1; j <= y_block_size; j++)
                  for (k = 1; k <= z_block_size; k++, n++)
                     IN(bp,m,i,j,k) = recv_buf[n];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face - one case */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 1; j < y_block_size; j += 2)
                  for (k = 1; k < z_block_size; k += 2, n++)
                     IN(bp,m,i,j  ,k  ) =
                     IN(bp,m,i,j  ,k+1) =
                     IN(bp,m,i,j+1,k  ) =
                     IN(bp,m,i,j+1,k+1) = recv_buf[n];
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to recv */
            if (face_case%2 == 0) {
               js = 1;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size;
            }
            if ((face_case/2)%2 == 1) {
               ks = 1;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = js; j <= je; j++)
                  for (k = ks; k <= ke; k++, n++)
                     IN(bp,m,i,j,k) = recv_buf[n];
         }

      } else if (dir == 1) { /* Y - North, South */

         /* Y directions (North and South) sent second, so recv the real values
         */
         if (face_case >= 10) { /* +Y - from North */
            j = y_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -Y - from South */
            j = 0;
         if (face_case == 0) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i <= x_block_size; i++)
                  for (k = 1; k <= z_block_size; k++, n++)
                     IN(bp,m,i,j,k) = recv_buf[n];
         } else if (face_case == 1) {
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (k = 1; k <= z_block_size; k++, n++)
                     IN(bp,m,i,j,k) = recv_buf[n];
         } else if (face_case >= 2 && face_case <= 5) {
            /* one case - recv into 4 cells per cell sent */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i < x_block_size; i += 2)
                  for (k = 1; k < z_block_size; k += 2, n++)
                     IN(bp,m,i  ,j,k  ) =
                     IN(bp,m,i ,j,k+1) =
                     IN(bp,m,i+1,j,k  ) =
                     IN(bp,m,i+1,j,k+1) = recv_buf[n];
         } else {                     /* quarter face -> whole face */
            /* whole face -> quarter face - determine case */
            if (face_case%2 == 0) {
               is = 1;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size;
            }
            if ((face_case/2)%2 == 1) {
               ks = 1;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (k = ks; k <= ke; k++, n++)
                     IN(bp,m,i,j,k) = recv_buf[n];
         }

      } else {               /* Z - Up, Down */

         /* Z directions (Up and Down) sent last
         */
         if (face_case >= 10) { /* +Z - from Up */
            k = z_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -Z - from Down */
            k = 0;
         if (face_case == 0) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i <= x_block_size; i++)
                  for (j = 1; j <= y_block_size; j++, n++)
                     IN(bp,m,i,j,k) = recv_buf[n];
         } else if (face_case == 1) {
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (j = 0; j <= y_block_size+1; j++, n++)
                     IN(bp,m,i,j,k) = recv_buf[n];
         } else if (face_case >= 2 && face_case <= 5) {
            /* one case - receive into 4 cells */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 1; i < x_block_size; i += 2)
                  for (j = 1; j < y_block_size; j += 2, n++)
                     IN(bp,m,i  ,j  ,k) =
                     IN(bp,m,i  ,j+1,k) =
                     IN(bp,m,i+1,j  ,k) =
                     IN(bp,m,i+1,j+1,k) = recv_buf[n];
         } else {                     /* quarter face -> whole face */
            /* whole face -> quarter face - determine case */
            if (face_case%2 == 0) {
               is = 1;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size;
            }
            if ((face_case/2)%2 == 1) {
               js = 1;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (j = js; j <= je; j++, n++)
                     IN(bp,m,i,j,k) = recv_buf[n];
         }
      }

   } else if (PTR_cmd->code == 1) {  /* send ghosts */

      if (dir == 0) {        /* X - East, West */

         if (face_case >= 10) { /* +X - from East */
            i = x_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -X - from West */
            i = 0;
         if (face_case < 2) {        /* whole face -> whole */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 0; j <= y_block_size+1; j++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     IN(bp,m,i,j,k) = recv_buf[n];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face */
            for (n = 0, m = start; m < start+num_comm; m++) {
               j = 0;
               k = 0;
               IN(bp,m,i,j,k) = recv_buf[n++];
               for (k = 1; k < z_block_size; k += 2, n++)
                  IN(bp,m,i,j,k  ) =
                  IN(bp,m,i,j,k+1) = recv_buf[n];
               k = z_block_size + 1;
               IN(bp,m,i,j,k) = recv_buf[n++];
               for (j = 1; j < y_block_size; j += 2) {
                  k = 0;
                  IN(bp,m,i,j  ,k) =
                  IN(bp,m,i,j+1,k) = recv_buf[n++];
                  for (k = 1; k < z_block_size; k += 2, n++)
                     IN(bp,m,i,j  ,k  ) =
                     IN(bp,m,i,j  ,k+1) =
                     IN(bp,m,i,j+1,k  ) =
                     IN(bp,m,i,j+1,k+1) = recv_buf[n];
                  k = z_block_size + 1;
                  IN(bp,m,i,j  ,k) =
                  IN(bp,m,i,j+1,k) = recv_buf[n++];
               }
               j = y_block_size + 1;
               k = 0;
               IN(bp,m,i,j,k) = recv_buf[n++];
               for (k = 1; k < z_block_size; k += 2, n++)
                  IN(bp,m,i,j,k  ) =
                  IN(bp,m,i,j,k+1) = recv_buf[n];
               k = z_block_size + 1;
               IN(bp,m,i,j,k) = recv_buf[n++];
            }
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to recv */
            if (face_case%2 == 0) {
               js = 0;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               ks = 0;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = js; j <= je; j++)
                  for (k = ks; k <= ke; k++, n++)
                     IN(bp,m,i,j,k) = recv_buf[n];
         }

      } else if (dir == 1) { /* Y - North, South */

         if (face_case >= 10) { /* +Y - from North */
            j = y_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -Y - from South */
            j = 0;
         if (face_case < 2) {        /* whole face -> whole */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     IN(bp,m,i,j,k) = recv_buf[n];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face */
            for (n = 0, m = start; m < start+num_comm; m++) {
               i = 0;
               k = 0;
               IN(bp,m,i,j,k) = recv_buf[n++];
               for (k = 1; k < z_block_size; k += 2, n++)
                  IN(bp,m,i,j,k  ) =
                  IN(bp,m,i,j,k+1) = recv_buf[n];
               k = z_block_size + 1;
               IN(bp,m,i,j,k) = recv_buf[n++];
               for (i = 1; i < x_block_size; i += 2) {
                  k = 0;
                  IN(bp,m,i  ,j,k) =
                  IN(bp,m,i+1,j,k) = recv_buf[n++];
                  for (k = 1; k < z_block_size; k += 2, n++)
                     IN(bp,m,i  ,j,k  ) =
                     IN(bp,m,i ,j,k+1) =
                     IN(bp,m,i+1,j,k  ) =
                     IN(bp,m,i+1,j,k+1) = recv_buf[n];
                  k = z_block_size + 1;
                  IN(bp,m,i  ,j,k) =
                  IN(bp,m,i+1,j,k) = recv_buf[n++];
               }
               i = x_block_size + 1;
               k = 0;
               IN(bp,m,i,j,k) = recv_buf[n++];
               for (k = 1; k < z_block_size; k += 2, n++)
                  IN(bp,m,i,j,k  ) =
                  IN(bp,m,i,j,k+1) = recv_buf[n];
               k = z_block_size + 1;
               IN(bp,m,i,j,k) = recv_buf[n++];
            }
         } else {                     /* quarter face -> whole face */
            /* whole face -> quarter face - determine case */
            if (face_case%2 == 0) {
               is = 0;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               ks = 0;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (k = ks; k <= ke; k++, n++)
                     IN(bp,m,i,j,k) = recv_buf[n];
         }

      } else {               /* Z - Up, Down */

         if (face_case >= 10) { /* +Z - from Up */
            k = z_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -Z - from Down */
            k = 0;
         if (face_case < 2) {        /* whole face -> whole face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (j = 0; j <= y_block_size+1; j++, n++)
                     IN(bp,m,i,j,k) = recv_buf[n];
         } else if (face_case >= 2 && face_case <= 5) {
            /* whole face -> quarter face */
            for (n = 0, m = start; m < start+num_comm; m++) {
               i = 0;
               j = 0;
               IN(bp,m,i,j,k) = recv_buf[n++];
               for (j = 1; j < y_block_size; j += 2, n++)
                  IN(bp,m,i,j  ,k) =
                  IN(bp,m,i,j+1,k) = recv_buf[n];
               j = y_block_size + 1;
               IN(bp,m,i,j,k) = recv_buf[n++];
               for (i = 1; i < x_block_size; i += 2) {
                  j = 0;
                  IN(bp,m,i  ,j,k) =
                  IN(bp,m,i+1,j,k) = recv_buf[n++];
                  for (j = 1; j < y_block_size; j += 2, n++)
                     IN(bp,m,i  ,j  ,k) =
                     IN(bp,m,i  ,j+1,k) =
                     IN(bp,m,i+1,j  ,k) =
                     IN(bp,m,i+1,j+1,k) = recv_buf[n];
                  j = y_block_size + 1;
                  IN(bp,m,i  ,j,k) =
                  IN(bp,m,i+1,j,k) = recv_buf[n++];
               }
               i = x_block_size + 1;
               j = 0;
               IN(bp,m,i,j,k) = recv_buf[n++];
               for (j = 1; j < y_block_size; j += 2, n++)
                  IN(bp,m,i,j  ,k) =
                  IN(bp,m,i,j+1,k) = recv_buf[n];
               j = y_block_size + 1;
               IN(bp,m,i,j,k) = recv_buf[n++];
            }
         } else {                     /* quarter face -> whole face */
            /* whole face -> quarter face - determine case */
            if (face_case%2 == 0) {
               is = 0;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               js = 0;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (j = js; j <= je; j++, n++)
                     IN(bp,m,i,j,k) = recv_buf[n];
         }
      }

   } else {  /* code == 2 send ghosts and process on send */

      if (dir == 0) {        /* X - East, West */

         if (face_case >= 10) { /* +X - from East */
            i = x_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -X - from West */
            i = 0;
         if (face_case <= 5) {        /* whole face -> whole or quarter face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = 0; j <= y_block_size+1; j++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     IN(bp,m,i,j,k) = recv_buf[n];
         } else {                     /* quarter face -> whole face */
            /* four cases - figure out which quarter of face to recv */
            if (face_case%2 == 0) {
               js = 0;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               ks = 0;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (j = js; j <= je; j++)
                  for (k = ks; k <= ke; k++, n++)
                     IN(bp,m,i,j,k) = recv_buf[n];
         }

      } else if (dir == 1) { /* Y - North, South */

         if (face_case >= 10) { /* +Y - from North */
            j = y_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -Y - from South */
            j = 0;
         if (face_case <= 5) {        /* whole face -> whole or quarter face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (k = 0; k <= z_block_size+1; k++, n++)
                     IN(bp,m,i,j,k) = recv_buf[n];
         } else {                     /* quarter face -> whole face */
            /* whole face -> quarter face - determine case */
            if (face_case%2 == 0) {
               is = 0;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               ks = 0;
               ke = z_block_half;
            } else {
               ks = z_block_half + 1;
               ke = z_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (k = ks; k <= ke; k++, n++)
                     IN(bp,m,i,j,k) = recv_buf[n];
         }

      } else {               /* Z - Up, Down */

         if (face_case >= 10) { /* +Z - from Up */
            k = z_block_size + 1;
            face_case = face_case - 10;
         } else                 /* -Z - from Down */
            k = 0;
         if (face_case <= 5) {        /* whole face -> whole or quarter face */
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = 0; i <= x_block_size+1; i++)
                  for (j = 0; j <= y_block_size+1; j++, n++)
                     IN(bp,m,i,j,k) = recv_buf[n];
         } else {                     /* quarter face -> whole face */
            /* whole face -> quarter face - determine case */
            if (face_case%2 == 0) {
               is = 0;
               ie = x_block_half;
            } else {
               is = x_block_half + 1;
               ie = x_block_size + 1;
            }
            if ((face_case/2)%2 == 1) {
               js = 0;
               je = y_block_half;
            } else {
               js = y_block_half + 1;
               je = y_block_size + 1;
            }
            for (n = 0, m = start; m < start+num_comm; m++)
               for (i = is; i <= ie; i++)
                  for (j = js; j <= je; j++, n++)
                     IN(bp,m,i,j,k) = recv_buf[n];
         }
      }
   }

#endif
}

// Apply reflective boundary conditions to a face of a block.
void apply_bc( rankH_t* PTR_rankH, int l, int start, int num_comm )
{
   int var, i, j, k, f, t;

#if 1

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);
    block *bp = &PTR_rankH->blockH;

    int x_block_size = PTR_cmd->x_block_size;
    int y_block_size = PTR_cmd->y_block_size;
    int z_block_size = PTR_cmd->z_block_size;

    int x_block_half = PTR_rankH->x_block_half;
    int y_block_half = PTR_rankH->y_block_half;
    int z_block_half = PTR_rankH->z_block_half;

   t = 0;
   f = 1;
   if (!PTR_cmd->code && PTR_cmd->stencil == 7)
      switch (l) {
         case 1: t = x_block_size + 1;
                 f = x_block_size;
         case 0: for (var = start; var < start+num_comm; var++)
                    for (j = 1; j <= y_block_size; j++)
                       for (k = 1; k <= z_block_size; k++)
                          IN(bp,var,t,j,k) = IN(bp,var,f,j,k);
                 break;
         case 3: t = y_block_size + 1;
                 f = y_block_size;
         case 2: for (var = start; var < start+num_comm; var++)
                    for (i = 1; i <= x_block_size; i++)
                       for (k = 1; k <= z_block_size; k++)
                          IN(bp,var,i,t,k) = IN(bp,var,i,f,k);
                 break;
         case 5: t = z_block_size + 1;
                 f = z_block_size;
         case 4: for (var = start; var < start+num_comm; var++)
                    for (i = 1; i <= x_block_size; i++)
                       for (j = 1; j <= y_block_size; j++)
                          IN(bp,var,i,j,t) = IN(bp,var,i,j,f);
                 break;
      }
   else
      switch (l) {
         case 1: t = x_block_size + 1;
                 f = x_block_size;
         case 0: for (var = start; var < start+num_comm; var++)
                    for (j = 0; j <= y_block_size+1; j++)
                       for (k = 0; k <= z_block_size+1; k++)
                          IN(bp,var,t,j,k) = IN(bp,var,f,j,k);
                 break;
         case 3: t = y_block_size + 1;
                 f = y_block_size;
         case 2: for (var = start; var < start+num_comm; var++)
                    for (i = 0; i <= x_block_size+1; i++)
                       for (k = 0; k <= z_block_size+1; k++)
                          IN(bp,var,i,t,k) = IN(bp,var,i,f,k);
                 break;
         case 5: t = z_block_size + 1;
                 f = z_block_size;
         case 4: for (var = start; var < start+num_comm; var++)
                    for (i = 0; i <= x_block_size+1; i++)
                       for (j = 0; j <= y_block_size+1; j++)
                          IN(bp,var,i,j,t) = IN(bp,var,i,j,f);
                 break;
      }

#endif
}
