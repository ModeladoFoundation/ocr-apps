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

#include "block.h"
//#include "comm.h"
#include "proto.h"

ocrGuid_t commRefnNbrsEdt(EDT_ARGS)
{
    commRefnNbrsPRM_t* commRefnNbrsPRM = (commRefnNbrsPRM_t*) paramv;

    int irefine = commRefnNbrsPRM->irefine;
    int iAxis = commRefnNbrsPRM->iAxis;
    int flag = commRefnNbrsPRM->flag; // 1 is forward
                                                //-1 is reverse

    s32 _idep, _paramc, _depc;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;

    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);

    ocrHNT_t myDbkAffinityHNT, myEdtAffinityHNT;
    myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    ocrTML_t exchangeDataTML = PTR_rankTemplateH->exchangeDataTML;
    ocrTML_t TML_commRefnNbrs = PTR_rankTemplateH->TML_commRefnNbrs;

    DEBUG_PRINTF(( "%s ilevel %d id_l %d iAxis %d flag %d irefine %d ts %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, iAxis, flag, irefine, PTR_rankH->ts ));

    ocrDbRelease(DBK_rankH);

    // Do halo-exchange along one axis
    ocrGuid_t exchangeDataEDT, exchangeDataOEVT, exchangeDataOEVTS;

    ocrEdtCreate( &exchangeDataEDT, exchangeDataTML, //exchangeDataEdt
                  EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                  EDT_PROP_FINISH, &myEdtAffinityHNT, &exchangeDataOEVT );

    createEventHelper( &exchangeDataOEVTS, 1);
    ocrAddDependence( exchangeDataOEVT, exchangeDataOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, exchangeDataEDT, _idep++, DB_MODE_RW );
    ocrAddDependence( NULL_GUID, exchangeDataEDT, _idep++, DB_MODE_NULL );

    iAxis += 1;
    commRefnNbrsPRM->iAxis = iAxis;

    if( iAxis < 3 )
    {
        //set up halo-exchange along the next axis
        ocrGuid_t haloExchangeEDT;

        ocrEdtCreate( &haloExchangeEDT, TML_commRefnNbrs,
                      EDT_PARAM_DEF, paramv, EDT_PARAM_DEF, NULL,
                      EDT_PROP_NONE, &myEdtAffinityHNT, NULL);

        _idep = 0;
        ocrAddDependence( DBK_rankH, haloExchangeEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( exchangeDataOEVTS, haloExchangeEDT, _idep++, DB_MODE_NULL );
    }

    return NULL_GUID;
}

/// This is the function that does the heavy lifting for the
/// communication of halo data.  It is called once for each axis and
/// sends and receives two message.  Loading and unloading of the
/// buffers is in the hands of the sub-class virtual functions.
///
// This routine uses the communication pattern established for exchanging
// ghost values to exchange information about the refinement level and
// plans for refinement for neighboring blocks.

ocrGuid_t exchangeDataEdt(EDT_ARGS)
{
    commRefnNbrsPRM_t* commRefnNbrsPRM = (commRefnNbrsPRM_t*) paramv;

    int irefine = commRefnNbrsPRM->irefine;
    int iAxis = commRefnNbrsPRM->iAxis;
    int flag = commRefnNbrsPRM->flag; // 1 is forward
                                                //-1 is reverse
    s32 _idep;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;

    rankTemplateH_t* PTR_rankTemplateH = &(PTR_rankH->rankTemplateH);
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);

    doubleBufferedOcrObj_t* PTR_dBufH1 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[0]);
    doubleBufferedOcrObj_t* PTR_dBufH2 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[1]);

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);

    block *bp = &PTR_rankH->blockH;

    ocrHNT_t myDbkAffinityHNT, myEdtAffinityHNT;
    myDbkAffinityHNT = PTR_rankH->myDbkAffinityHNT;
    myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    ocrTML_t packRefnBufsTML = PTR_rankTemplateH->packRefnBufsTML;
    ocrTML_t unpackRefnBufsTML = PTR_rankTemplateH->unpackRefnBufsTML;

    int nNbrs = blockNnbrs(bp, iAxis);

    DEBUG_PRINTF(( "%s ilevel %d id_l %d iAxis %d flag %d nNbrs %d irefine %d ts %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, iAxis, flag, nNbrs, irefine, PTR_rankH->ts ));

    ocrDBK_t refnCurrSendDBKs[6][2], refnCoarSendDBKs[6][2], refnRefnSendDBKs[6][4][2];
    ocrDBK_t haloCurrRecvEVTs[6], haloCoarRecvEVTs[6], haloRefnRecvEVTs[6][4];

    int nei_level[6];
    int level = bp->level;

    int i, j;
    int phase = (flag == 1) ? 0 : 1;

    for (i = 2*iAxis; i < 2*(iAxis+1); i++) {
        nei_level[i] = bp->nei_level[i];
        if( nei_level[i] == level ) {
            refnCurrSendDBKs[i][phase] = PTR_dBufH1->refnCurrSendDBKs[i][phase];
            haloCurrRecvEVTs[i] = PTR_dBufH1->haloCurrRecvEVTs[i];
        }
        else if( nei_level[i] == level-1 ) {
            refnCoarSendDBKs[i][phase] = PTR_dBufH1->refnCoarSendDBKs[i][phase];
            haloCoarRecvEVTs[i] = PTR_dBufH1->haloCoarRecvEVTs[i];
        }
        else if( nei_level[i] == level+1 ) {
            for( j = 0; j < 4; j++ ) {
                refnRefnSendDBKs[i][j][phase] = PTR_dBufH1->refnRefnSendDBKs[i][j][phase];
                haloRefnRecvEVTs[i][j] = PTR_dBufH1->haloRefnRecvEVTs[i][j];
            }
        }
    }

    ocrDbRelease( DBK_rankH );

    ocrGuid_t packRefnBufsEDT, packRefnBufsOEVT, packRefnBufsOEVTS;

    ocrEdtCreate( &packRefnBufsEDT, packRefnBufsTML, //packRefnBufsEdt
                  EDT_PARAM_DEF, paramv, nNbrs+1, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &packRefnBufsOEVT );

    createEventHelper( &packRefnBufsOEVTS, 1);
    ocrAddDependence( packRefnBufsOEVT, packRefnBufsOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, packRefnBufsEDT, _idep++, DB_MODE_RW );

    for (i = 2*iAxis; i < 2*(iAxis+1); i++) {
        if( nei_level[i] == level ) {
            ocrAddDependence( refnCurrSendDBKs[i][phase], packRefnBufsEDT, _idep++, DB_MODE_RW );
        }
        else if( nei_level[i] == level-1 ) {
            ocrAddDependence( refnCoarSendDBKs[i][phase], packRefnBufsEDT, _idep++, DB_MODE_RW );
        }
        else if( nei_level[i] == level+1 ) {
            for( j = 0; j < 4; j++ ) {
                ocrAddDependence( refnRefnSendDBKs[i][j][phase], packRefnBufsEDT, _idep++, DB_MODE_RW );
            }
        }
        else if( nei_level[i] == -2 ) { //BOUNDARY
        }
        else {
            PRINTF("SOMETHING WENT WRONG!! UNBALANCED refinement!\n");
        }
    }

    ocrGuid_t unpackRefnBufsEDT;

    ocrEdtCreate( &unpackRefnBufsEDT, unpackRefnBufsTML, //unpackRefnBufsEdt
                  EDT_PARAM_DEF, paramv, nNbrs+2, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL);

    _idep = 0;
    ocrAddDependence( DBK_rankH, unpackRefnBufsEDT, _idep++, DB_MODE_RW );
    for (i = 2*iAxis; i < 2*(iAxis+1); i++) {
        if( nei_level[i] == level ) {
            ocrAddDependence( haloCurrRecvEVTs[i], unpackRefnBufsEDT, _idep++, DB_MODE_RW );
            DEBUG_PRINTF(( "%s dep %d "GUIDF" \n", __func__, _idep-1, haloCurrRecvEVTs[i] ));
        }
        else if( nei_level[i] == level-1 ) {
            ocrAddDependence( haloCoarRecvEVTs[i], unpackRefnBufsEDT, _idep++, DB_MODE_RW );
            DEBUG_PRINTF(( "%s dep %d "GUIDF" \n", __func__, _idep-1, haloCoarRecvEVTs[i] ));
        }
        else if( nei_level[i] == level+1 ) {
            for( j = 0; j < 4; j++ ) {
                ocrAddDependence( haloRefnRecvEVTs[i][j], unpackRefnBufsEDT, _idep++, DB_MODE_RW );
                DEBUG_PRINTF(( "%s dep %d "GUIDF" \n", __func__, _idep-1, haloRefnRecvEVTs[i][j] ));
            }
        }
        else if( nei_level[i] == -2 ) { //BOUNDARY
        }
        else {
            PRINTF("SOMETHING WENT WRONG!! UNBALANCED refinement!\n");
        }
    }
    ocrAddDependence( packRefnBufsOEVTS, unpackRefnBufsEDT, _idep++, DB_MODE_NULL ); //TODO - is this really needed?

    return NULL_GUID;
}

int blockNnbrs(block *bp, int iAxis)
{
    int nNbrs = 0;
    int i, j;

    for (i = 2*iAxis; i < 2*(iAxis+1); i++) {
        if( bp->nei_level[i] == bp->level )
            ++nNbrs;
        else if( bp->nei_level[i] == bp->level-1 )
            ++nNbrs;
        else if( bp->nei_level[i] == bp->level+1 ) {
            for( j = 0; j < 4; j++ )
                ++nNbrs;
        }
        else if( bp->nei_level[i] == -2 ) { //BOUNDARY
        }
        else {
            PRINTF("SOMETHING WENT WRONG!! UNBALANCED refinement!\n");
        }
    }

    return nNbrs;
}

ocrGuid_t packRefnBufsEdt(EDT_ARGS)
{
    commRefnNbrsPRM_t* commRefnNbrsPRM = (commRefnNbrsPRM_t*) paramv;

    int irefine = commRefnNbrsPRM->irefine;
    int iAxis = commRefnNbrsPRM->iAxis;
    int flag = commRefnNbrsPRM->flag; // 1 is forward
                                                //-1 is reverse

    s32 _idep;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);

    int phase = (flag == 1) ? 0 : 1;

    doubleBufferedOcrObj_t* PTR_dBufH1 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[0]);
    doubleBufferedOcrObj_t* PTR_dBufH2 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[1]);

    reset_refnSendPTRs(PTR_rankH, iAxis, phase);

    DEBUG_PRINTF(( "%s ilevel %d id_l %d iAxis %d flag %d irefine %d ts %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, iAxis, flag, irefine, PTR_rankH->ts ));

    block *bp = &PTR_rankH->blockH;

    int i, j;
    int send_int;

    for (i = 2*iAxis; i < 2*(iAxis+1); i++) {
        if( bp->nei_level[i] == bp->level ) {
            PTR_dBufH1->refnCurrSendDBKs[i][phase] = depv[_idep].guid;
            PTR_dBufH1->refnCurrSendPTRs[i][phase] = (int*) depv[_idep++].ptr;

            if( flag == 1 )
                send_int = bp->refine;
            else if( flag == -1)
                send_int = bp->nei_refine[i];

            PTR_dBufH1->refnCurrSendPTRs[i][phase][0] = send_int;

            ocrDbRelease( PTR_dBufH1->refnCurrSendDBKs[i][phase] );
            ocrEventSatisfy( PTR_dBufH1->haloCurrSendEVTs[i], PTR_dBufH1->refnCurrSendDBKs[i][phase] );
            DEBUG_PRINTF(( "%s dep %d "GUIDF" \n", __func__, _idep-1, PTR_dBufH1->haloCurrSendEVTs[i] ));

            DEBUG_PRINTF(("nei send bp->refine %d nei_refine %d %d %d %d %d %d\n", send_int, bp->nei_refine[0], bp->nei_refine[1], bp->nei_refine[2], bp->nei_refine[3], bp->nei_refine[4], bp->nei_refine[5]));

        }
        else if( bp->nei_level[i] == bp->level-1 ) {
            PTR_dBufH1->refnCoarSendDBKs[i][phase] = depv[_idep].guid;
            PTR_dBufH1->refnCoarSendPTRs[i][phase] = (int*) depv[_idep++].ptr;

            if( flag == 1 )
                send_int = bp->refine;
            else if( flag == -1)
                send_int = bp->nei_refine[i];

            PTR_dBufH1->refnCoarSendPTRs[i][phase][0] = send_int;

            ocrDbRelease( PTR_dBufH1->refnCoarSendDBKs[i][phase] );
            ocrEventSatisfy( PTR_dBufH1->haloCoarSendEVTs[i], PTR_dBufH1->refnCoarSendDBKs[i][phase] );
            DEBUG_PRINTF(( "%s dep %d "GUIDF" \n", __func__, _idep-1, PTR_dBufH1->haloCoarSendEVTs[i] ));
            DEBUG_PRINTF(("nei send bp->refine %d nei_refine %d %d %d %d %d %d\n", send_int, bp->nei_refine[0], bp->nei_refine[1], bp->nei_refine[2], bp->nei_refine[3], bp->nei_refine[4], bp->nei_refine[5]));
        }
        else if( bp->nei_level[i] == bp->level+1 ) {
            for( j = 0; j < 4; j++ ) {
                PTR_dBufH1->refnRefnSendDBKs[i][j][phase] = depv[_idep].guid;
                PTR_dBufH1->refnRefnSendPTRs[i][j][phase] = (int*) depv[_idep++].ptr;

                if( flag == 1 )
                    send_int = bp->refine;
                else if( flag == -1)
                    send_int = bp->nei_refine[i];

                PTR_dBufH1->refnRefnSendPTRs[i][j][phase][0] = send_int;

                ocrDbRelease( PTR_dBufH1->refnRefnSendDBKs[i][j][phase] );
                ocrEventSatisfy( PTR_dBufH1->haloRefnSendEVTs[i][j], PTR_dBufH1->refnRefnSendDBKs[i][j][phase] );
            DEBUG_PRINTF(( "%s dep %d "GUIDF" \n", __func__, _idep-1, PTR_dBufH1->haloRefnSendEVTs[i][j] ));
            DEBUG_PRINTF(("nei send bp->refine %d nei_refine %d %d %d %d %d %d\n", send_int, bp->nei_refine[0], bp->nei_refine[1], bp->nei_refine[2], bp->nei_refine[3], bp->nei_refine[4], bp->nei_refine[5]));

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

void reset_refnSendPTRs( rankH_t *PTR_rankH, int iAxis, int phase )
{
    block *bp = &PTR_rankH->blockH;
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);

    doubleBufferedOcrObj_t* PTR_dBufH1 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[0]);
    doubleBufferedOcrObj_t* PTR_dBufH2 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[1]);

    int i, j;

    for (i = 2*iAxis; i < 2*(iAxis+1); i++) {
        PTR_dBufH1->refnCurrSendPTRs[i][phase] = NULL;
        PTR_dBufH1->refnCoarSendPTRs[i][phase] = NULL;

        for( j = 0; j < 4; j++ ) {
            PTR_dBufH1->refnRefnSendPTRs[i][j][phase] = NULL;
        }
    }
}

void reset_refnRecvPTRs( rankH_t *PTR_rankH, int iAxis, int phase )
{
    block *bp = &PTR_rankH->blockH;
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);

    doubleBufferedOcrObj_t* PTR_dBufH1 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[0]);
    doubleBufferedOcrObj_t* PTR_dBufH2 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[1]);

    int i, j;

    for (i = 2*iAxis; i < 2*(iAxis+1); i++) {
        PTR_dBufH1->refnCurrRecvPTRs[i][phase] = NULL;
        PTR_dBufH1->refnCoarRecvPTRs[i][phase] = NULL;
        for( j = 0; j < 4; j++ ) {
            PTR_dBufH1->refnRefnRecvPTRs[i][j][phase] = NULL;
            //bp->nei_refine_recv[i][j] = STAY;
        }
    }
}

ocrGuid_t unpackRefnBufsEdt(EDT_ARGS)
{
    commRefnNbrsPRM_t* commRefnNbrsPRM = (commRefnNbrsPRM_t*) paramv;

    int irefine = commRefnNbrsPRM->irefine;
    int iAxis = commRefnNbrsPRM->iAxis;
    int flag = commRefnNbrsPRM->flag; // 1 is forward
                                                //-1 is reverse

    s32 _idep;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);
    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);

    int phase = (flag == 1) ? 0 : 1;

    doubleBufferedOcrObj_t* PTR_dBufH1 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[0]);
    doubleBufferedOcrObj_t* PTR_dBufH2 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[1]);

    reset_refnRecvPTRs(PTR_rankH, iAxis, phase);

    block *bp = &PTR_rankH->blockH;

    DEBUG_PRINTF(( "%s ilevel %d id_l %d iAxis %d flag %d irefine %d ts %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, iAxis, flag, irefine, PTR_rankH->ts ));

    int i, j;
    int refine;

    int* refine_update;

    for (i = 2*iAxis; i < 2*(iAxis+1); i++) {
        if( bp->nei_level[i] == bp->level ) {
            PTR_dBufH1->refnCurrRecvPTRs[i][phase] = (int*) depv[_idep++].ptr;
            refine = PTR_dBufH1->refnCurrRecvPTRs[i][phase][0];

            if( flag == 1 ) {
                bp->nei_refine_recv[i][0] = refine;
                if( refine == REFINE )
                    bp->nei_refine[i] = REFINE;
                else if( refine >= STAY && bp->nei_refine[i] == COARSEN  )
                    bp->nei_refine[i] = STAY;

                DEBUG_PRINTF(( "%s dep %d "GUIDF" \n", __func__, _idep-1, PTR_dBufH1->haloCurrRecvEVTs[i] ));
                DEBUG_PRINTF(("nei recv %d nei_refine %d %d %d %d %d %d\n", refine, bp->nei_refine[0], bp->nei_refine[1], bp->nei_refine[2], bp->nei_refine[3], bp->nei_refine[4], bp->nei_refine[5]));
            }
            else if( flag == -1) {
                if( refine > bp->refine )
                        bp->refine = refine;
                DEBUG_PRINTF(( "%s dep %d "GUIDF" \n", __func__, _idep-1, PTR_dBufH1->haloCurrRecvEVTs[i] ));
                DEBUG_PRINTF(("nei recv bp->refine %d nei_refine %d %d %d %d %d %d\n", refine, bp->nei_refine[0], bp->nei_refine[1], bp->nei_refine[2], bp->nei_refine[3], bp->nei_refine[4], bp->nei_refine[5]));
            }

        }
        else if( bp->nei_level[i] == bp->level-1 ) {
            PTR_dBufH1->refnCoarRecvPTRs[i][phase] = (int*) depv[_idep++].ptr;
            refine = PTR_dBufH1->refnCoarRecvPTRs[i][phase][0];

            if( flag == 1 ) {
                bp->nei_refine_recv[i][0] = refine;
                if( refine == REFINE )
                    bp->nei_refine[i] = REFINE;
                else if( refine >= STAY && bp->nei_refine[i] == COARSEN  )
                    bp->nei_refine[i] = STAY;
                DEBUG_PRINTF(( "%s dep %d "GUIDF" \n", __func__, _idep-1, PTR_dBufH1->haloCoarRecvEVTs[i] ));
                DEBUG_PRINTF(("nei recv %d nei_refine %d %d %d %d %d %d\n", refine, bp->nei_refine[0], bp->nei_refine[1], bp->nei_refine[2], bp->nei_refine[3], bp->nei_refine[4], bp->nei_refine[5]));
            }
            else if( flag == -1) {
                if( refine > bp->refine )
                        bp->refine = refine;
                DEBUG_PRINTF(( "%s dep %d "GUIDF" \n", __func__, _idep-1, PTR_dBufH1->haloCoarRecvEVTs[i] ));
                DEBUG_PRINTF(("nei recv bp->refine %d nei_refine %d %d %d %d %d %d\n", refine, bp->nei_refine[0], bp->nei_refine[1], bp->nei_refine[2], bp->nei_refine[3], bp->nei_refine[4], bp->nei_refine[5]));
            }
        }
        else if( bp->nei_level[i] == bp->level+1 ) {
            for( j = 0; j < 4; j++ ) {
                PTR_dBufH1->refnRefnRecvPTRs[i][j][phase] = (int*) depv[_idep++].ptr;
                refine = PTR_dBufH1->refnRefnRecvPTRs[i][j][phase][0];

                if( flag == 1 ) {
                    bp->nei_refine_recv[i][j] = refine;

                    if( refine >= STAY && bp->nei_refine[i] == COARSEN  )
                        bp->nei_refine[i] = STAY;
                    DEBUG_PRINTF(( "%s dep %d "GUIDF" \n", __func__, _idep-1, PTR_dBufH1->haloRefnRecvEVTs[i][j] ));
                    DEBUG_PRINTF(("nei recv %d nei_refine %d %d %d %d %d %d\n", refine, bp->nei_refine[0], bp->nei_refine[1], bp->nei_refine[2], bp->nei_refine[3], bp->nei_refine[4], bp->nei_refine[5]));
                }
                else if( flag == - 1) {
                    if( refine > bp->refine )
                            bp->refine = refine;
                    DEBUG_PRINTF(( "%s dep %d "GUIDF" \n", __func__, _idep-1, PTR_dBufH1->haloRefnRecvEVTs[i][j] ));
                    DEBUG_PRINTF(("nei recv bp->refine %d nei_refine %d %d %d %d %d %d\n", refine, bp->nei_refine[0], bp->nei_refine[1], bp->nei_refine[2], bp->nei_refine[3], bp->nei_refine[4], bp->nei_refine[5]));
                }
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
