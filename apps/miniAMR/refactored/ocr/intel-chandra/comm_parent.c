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
//#include <mpi.h>

#include "block.h"
//#include "comm.h"
#include "proto.h"

ocrGuid_t commRefnSibsEdt(EDT_ARGS)
{
    commRefnSibsPRM_t* commRefnSibsPRM = (commRefnSibsPRM_t*) paramv;

    int irefine = commRefnSibsPRM->irefine;
    int flag = commRefnSibsPRM->flag; // 1 is forward
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

    u64 itimestep = (PTR_rankH->ts%PTR_cmd->refine_freq);
    s64 phase = itimestep%2;

    DEBUG_PRINTF(( "%s ilevel %d id_l %d flag %d irefine %d ts %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, flag, irefine, PTR_rankH->ts ));

    block *bp = &PTR_rankH->blockH;

    ocrHint_t myEdtAffinityHNT = PTR_rankH->myEdtAffinityHNT;

    int nSibs = 8;

    ocrGuid_t packRefnBufsSibsTML, packRefnBufsSibsEDT, packRefnBufsSibsOEVT, packRefnBufsSibsOEVTS;

    ocrEdtCreate( &packRefnBufsSibsEDT, PTR_rankTemplateH->packRefnBufsSibsTML, //packRefnBufsSibsEdt
                  EDT_PARAM_DEF, paramv, nSibs+1, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, &packRefnBufsSibsOEVT );

    createEventHelper( &packRefnBufsSibsOEVTS, 1);
    ocrAddDependence( packRefnBufsSibsOEVT, packRefnBufsSibsOEVTS, 0, DB_MODE_NULL );

    _idep = 0;
    ocrAddDependence( DBK_rankH, packRefnBufsSibsEDT, _idep++, DB_MODE_RW );

    int i;


    for (i = 0; i < 8; i++) {
        //if( bp->sib_level[i] == bp->level && bp->level != 0 ) {
        if( bp->level != 0 ) {
            ocrAddDependence( PTR_dBufH1->refnCurrSendSibsDBKs[i], packRefnBufsSibsEDT, _idep++, DB_MODE_RW );
            DEBUG_PRINTF(( "%d "GUIDF" ", i, PTR_dBufH1->refnCurrSendSibsDBKs[i]));
        }
        else {
            ocrAddDependence( NULL_GUID, packRefnBufsSibsEDT, _idep++, DB_MODE_RW );
            DEBUG_PRINTF(( "%d NULL_GUID ", i));
        }
    }
    DEBUG_PRINTF(( "\n" ));

    ocrGuid_t unpackRefnBufsSibsTML, unpackRefnBufsSibsEDT;

    ocrEdtCreate( &unpackRefnBufsSibsEDT, PTR_rankTemplateH->unpackRefnBufsSibsTML, //unpackRefnBufsSibsEdt
                  EDT_PARAM_DEF, paramv, nSibs+2, NULL,
                  EDT_PROP_NONE, &myEdtAffinityHNT, NULL);

    _idep = 0;
    ocrAddDependence( DBK_rankH, unpackRefnBufsSibsEDT, _idep++, DB_MODE_RW );
    for (i = 0; i < 8; i++) {
        //if( bp->sib_level[i] == bp->level && bp->level != 0 ) {
        if( bp->level != 0 ) {
            ocrAddDependence( PTR_dBufH1->haloSiblingsRecvEVTs[i], unpackRefnBufsSibsEDT, _idep++, DB_MODE_RO );
        }
        else {
            ocrAddDependence( NULL_GUID, unpackRefnBufsSibsEDT, _idep++, DB_MODE_RW );
        }
    }
    ocrAddDependence( packRefnBufsSibsOEVTS, unpackRefnBufsSibsEDT, _idep++, DB_MODE_NULL ); //TODO - is this really needed?

    if( PTR_rankH->ilevel >= 1 && PTR_rankH->isibling == 0 ) { // && flag == 11 ) { //parent proxy

        flag = (flag>0?1:-1)*10; //Forward to uncles
        commRefnSibsPRM_t exchangeDataParentSibsPRM = {irefine, flag, (bp->number>=0)?bp->refine:0};
        ocrGuid_t exchangeDataParentSibsEDT;

        ocrEdtCreate( &exchangeDataParentSibsEDT, PTR_rankTemplateH->TML_commRefnSibs, //commRefnSibsEdt
                      EDT_PARAM_DEF, (u64*) &exchangeDataParentSibsPRM, EDT_PARAM_DEF, NULL,
                      EDT_PROP_FINISH, &myEdtAffinityHNT, NULL );

        _idep = 0;
        ocrAddDependence( PTR_sharedOcrObjH->parentRankDBK, exchangeDataParentSibsEDT, _idep++, DB_MODE_RW );
        ocrAddDependence( NULL_GUID, exchangeDataParentSibsEDT, _idep++, DB_MODE_RO );

    }

    return NULL_GUID;
}

ocrGuid_t packRefnBufsSibsEdt(EDT_ARGS)
{
    commRefnSibsPRM_t* commRefnSibsPRM = (commRefnSibsPRM_t*) paramv;

    int irefine = commRefnSibsPRM->irefine;
    int flag = commRefnSibsPRM->flag; // 1 is forward
                                           //-1 is reverse
    s32 _idep;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);

    doubleBufferedOcrObj_t* PTR_dBufH1 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[0]);
    doubleBufferedOcrObj_t* PTR_dBufH2 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[1]);

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);

    u64 itimestep = (PTR_rankH->ts%PTR_cmd->refine_freq);
    s64 phase = itimestep%2;

    DEBUG_PRINTF(( "%s ilevel %d id_l %d flag %d irefine %d ts %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, flag, irefine, PTR_rankH->ts ));

    reset_refnSendSibsPTRs(PTR_rankH);

    block *bp = &PTR_rankH->blockH;

    int i, j;
    int send_int;

    int parent_sib_refine[8];

    if( flag == 10 ) bp->refine = commRefnSibsPRM->child_refine; //parent intention
    if( flag == -10 ) for (i = 0; i < 8; i++) bp->sib_refine[i] = 0; //parent sib_refine

    for (i = 0; i < 8; i++) {
        //if( bp->sib_level[i] == bp->level && bp->level != 0 ) {
        if( bp->level != 0 ) {
            PTR_dBufH1->refnCurrSendSibsDBKs[i] = depv[_idep].guid;
            PTR_dBufH1->refnCurrSendSibsPTRs[i] = (int*) depv[_idep++].ptr;

            if( flag == 1 )
                send_int = bp->refine;
            else if( flag == -1)
                send_int = bp->sib_refine[i];
            else if( flag == 10 )
                send_int = bp->refine;
            else if( flag == -10 )
                send_int = bp->sib_refine[i];

            DEBUG_PRINTF(( "i %d send %d sendEvent "GUIDF" \n", i, send_int, PTR_dBufH1->haloSiblingsSendEVTs[i]));
            PTR_dBufH1->refnCurrSendSibsPTRs[i][0] = send_int;
            ocrDbRelease( PTR_dBufH1->refnCurrSendSibsDBKs[i] );
            ocrEventSatisfy( PTR_dBufH1->haloSiblingsSendEVTs[i], PTR_dBufH1->refnCurrSendSibsDBKs[i] );
        }
        else
            _idep++;
    }

    return NULL_GUID;
}

void reset_refnSendSibsPTRs( rankH_t *PTR_rankH )
{
    block *bp = &PTR_rankH->blockH;
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);

    doubleBufferedOcrObj_t* PTR_dBufH1 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[0]);
    doubleBufferedOcrObj_t* PTR_dBufH2 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[1]);

    int i, j;

    for (i = 0; i < 8; i++) {
        PTR_dBufH1->refnCurrSendSibsPTRs[i] = NULL;
    }
}

void reset_refnRecvSibsPTRs( rankH_t *PTR_rankH, s64 flag )
{
    block *bp = &PTR_rankH->blockH;
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);

    doubleBufferedOcrObj_t* PTR_dBufH1 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[0]);
    doubleBufferedOcrObj_t* PTR_dBufH2 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[1]);

    int i, j;

    for (i = 0; i < 8; i++) {
        if( ABS(flag) == 1 ) bp->sib_refine_recv[i] = 0;
        PTR_dBufH1->refnCurrRecvSibsPTRs[i] = NULL;
    }
}

ocrGuid_t unpackRefnBufsSibsEdt(EDT_ARGS)
{
    commRefnSibsPRM_t* commRefnSibsPRM = (commRefnSibsPRM_t*) paramv;

    int irefine = commRefnSibsPRM->irefine;
    int flag = commRefnSibsPRM->flag; // 1 is forward
                                           //-1 is reverse
    s32 _idep;

    _idep = 0;
    ocrDBK_t DBK_rankH = depv[_idep++].guid;

    _idep = 0;
    rankH_t* PTR_rankH = depv[_idep++].ptr;
    sharedOcrObj_t* PTR_sharedOcrObjH = &(PTR_rankH->sharedOcrObjH);

    doubleBufferedOcrObj_t* PTR_dBufH1 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[0]);
    doubleBufferedOcrObj_t* PTR_dBufH2 = &(PTR_sharedOcrObjH->doubleBufferedOcrObjH[1]);

    Command* PTR_cmd = &(PTR_rankH->globalParamH.cmdParamH);

    u64 itimestep = (PTR_rankH->ts%PTR_cmd->refine_freq);
    s64 phase = itimestep%2;

    DEBUG_PRINTF(( "%s ilevel %d id_l %d flag %d irefine %d ts %d\n", __func__, PTR_rankH->ilevel, PTR_rankH->myRank, flag, irefine, PTR_rankH->ts ));

    reset_refnRecvSibsPTRs(PTR_rankH,flag);

    block *bp = &PTR_rankH->blockH;

    int i, j;
    int refine;

    int* refine_update;

    for (i = 0; i < 8; i++) {
        //if( bp->sib_level[i] == bp->level && bp->level != 0 ) {
        if( bp->level != 0 ) {
            PTR_dBufH1->refnCurrRecvSibsPTRs[i] = (int*) depv[_idep++].ptr;
            refine = PTR_dBufH1->refnCurrRecvSibsPTRs[i][0];

            if( flag == 1 ) {
                refine_update = &(bp->sib_refine[i]);
                bp->sib_refine_recv[i] = refine;

                if( refine > - 1 && bp->sib_refine[i] == - 1)
                    bp->sib_refine[i] = 0;

                DEBUG_PRINTF(( "i %d recv %d\n", i, bp->sib_refine_recv[i] ));
            }
            else if( flag == -1) {
                refine_update = &(bp->refine);
                if( refine > *refine_update )
                        *refine_update = refine;

                DEBUG_PRINTF(( "i %d recv %d\n", i, *refine_update ));
            }
            else if( flag == 10 ) { //parent
                bp->sib_refine_recv[i] = refine;

                bp->sib_level[i] = (PTR_rankH->ilevel+1) + refine;

                DEBUG_PRINTF(( "parent i %d recv %d sib_level %d\n", i, bp->sib_refine_recv[i], bp->sib_level[i] ));
            }
            else if( flag == -10) {
                refine_update = &(bp->refine);
                if( refine > *refine_update )
                        *refine_update = refine;

                DEBUG_PRINTF(( "i %d recv %d\n", i, *refine_update ));
            }
        }
        else
            _idep++;
    }

    DEBUG_PRINTF(( " sib_refine " ));
    for (i = 0; i < 8; i++) {
        DEBUG_PRINTF(( "%d ", bp->sib_refine[i] ));
    }
    DEBUG_PRINTF(( "\n" ));

    return NULL_GUID;
}